// MTG Token Counter — main firmware
// Display: 3461BS-1 common-anode 4-digit 7-segment (see display_pinout.md)
// Buttons: 4 tactile switches, active LOW (INPUT_PULLUP)

// ── Display pins ──────────────────────────────────────────────────────────────
const int SEG[]  = {2, 3, 4, 5, 6, 8, 13}; // A F B E D C G  (indices 0–6)
const int DIG[]  = {9, 10, 11, 12};          // Digit 1-4 (left→right)
const int DP_PIN = 7;                         // Decimal point (used as separator)

// common-anode: 0 = segment ON, 1 = OFF
const byte PATTERNS[10][7] = {
  {0,0,0,0,0,0,1}, // 0
  {1,1,0,1,1,0,1}, // 1
  {0,1,0,0,0,1,0}, // 2
  {0,1,0,1,0,0,0}, // 3
  {1,0,0,1,1,0,0}, // 4
  {0,0,1,1,0,0,0}, // 5
  {0,0,1,0,0,0,0}, // 6
  {0,1,0,1,1,0,1}, // 7
  {0,0,0,0,0,0,0}, // 8
  {0,0,0,1,1,0,0}, // 9
};

// ── Button pins ───────────────────────────────────────────────────────────────
const int BTN_TOP    = A0;
const int BTN_BOTTOM = A1;
const int BTN_LEFT   = A2;
const int BTN_RIGHT  = A3;
const int BUTTONS[4] = {BTN_TOP, BTN_BOTTOM, BTN_LEFT, BTN_RIGHT};

// ── Timing constants ──────────────────────────────────────────────────────────
const unsigned long DEBOUNCE_MS   = 20;
const unsigned long LONG_PRESS_MS = 500;
const unsigned long MUX_STEP_US   = 2500; // ~2.5 ms per digit → ~100 Hz refresh
const unsigned long ANIM_STEP_MS  = 80;   // ms per animation frame (~1s per revolution)

// ── Startup animation ─────────────────────────────────────────────────────────
//
// The outer perimeter of 4 digits forms a 12-segment clockwise loop:
//
//   [A0][A1][A2][A3]
//    F0              B3
//    E0              C3
//   [D0][D1][D2][D3]
//
// Each entry is {digit index, seg array index}
// SEG array order: A=0, F=1, B=2, E=3, D=4, C=5, G=6
//
const byte PERIM[12][2] = {
  {0, 0}, // A of digit 0  (top-left)
  {1, 0}, // A of digit 1
  {2, 0}, // A of digit 2
  {3, 0}, // A of digit 3  (top-right)
  {3, 2}, // B of digit 3  (right-top)
  {3, 5}, // C of digit 3  (right-bottom)
  {3, 4}, // D of digit 3  (bottom-right)
  {2, 4}, // D of digit 2
  {1, 4}, // D of digit 1
  {0, 4}, // D of digit 0  (bottom-left)
  {0, 3}, // E of digit 0  (left-bottom)
  {0, 1}, // F of digit 0  (left-top)
};

bool animating = true;
int  animPos   = 0;
byte animSegs[4]; // per-digit segment bitmask during animation (bit N = SEG[N] on)

// Build animSegs for the current frame: light 2 consecutive perimeter segments
void buildAnimFrame() {
  memset(animSegs, 0, sizeof(animSegs));
  for (int i = 0; i < 2; i++) {
    int p = (animPos + i) % 12;
    animSegs[PERIM[p][0]] |= (1 << PERIM[p][1]);
  }
}

// ── Button state ──────────────────────────────────────────────────────────────
int  lastRaw[4]      = {HIGH, HIGH, HIGH, HIGH};
int  stable[4]       = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastChange[4]  = {0, 0, 0, 0};
unsigned long pressStart[4]  = {0, 0, 0, 0};
bool longFired[4]    = {false, false, false, false};
bool wasPressed[4]   = {false, false, false, false};

// ── Konami sequence (arrow-only: ↑↑↓↓←→←→) ───────────────────────────────────
// Button indices: TOP=0, BOTTOM=1, LEFT=2, RIGHT=3
const byte KONAMI[8] = {0, 0, 1, 1, 2, 3, 2, 3};
byte konamiBuffer[8] = {255, 255, 255, 255, 255, 255, 255, 255};
int  konamiPos = 0;

void konamiRecord(int btn) {
  konamiBuffer[konamiPos] = btn;
  konamiPos = (konamiPos + 1) % 8;
  // Check if the last 8 presses match the sequence
  for (int i = 0; i < 8; i++) {
    if (konamiBuffer[(konamiPos + i) % 8] != KONAMI[i]) return;
  }
  // Match — trigger animation and wipe buffer
  animating = true;
  animPos   = 0;
  buildAnimFrame();
  memset(konamiBuffer, 255, sizeof(konamiBuffer));
}

// ── Counter state ─────────────────────────────────────────────────────────────
int untapped = 0;
int tapped   = 0;

// ── Display buffer ────────────────────────────────────────────────────────────
int digits[4];

void updateDisplay() {
  digits[0] = (untapped / 10) % 10;
  digits[1] = untapped % 10;
  digits[2] = (tapped / 10) % 10;
  digits[3] = tapped % 10;
}

// ── Action handlers ───────────────────────────────────────────────────────────
void handleShort(int btn) {
  switch (btn) {
    case 0: untapped = min(untapped + 1, 99); break;          // TOP    — +1 untapped
    case 1: untapped = max(untapped - 1, 0);  break;          // BOTTOM — -1 untapped
    case 2: if (tapped   > 0) { tapped--;   untapped++; } break; // LEFT   — untap 1
    case 3: if (untapped > 0) { untapped--; tapped++;   } break; // RIGHT  — tap 1
  }
}

void handleLong(int btn) {
  switch (btn) {
    case 0: untapped = min(untapped + 2, 99); break; // TOP hold    — +2 untapped
    case 1: untapped = 0; tapped = 0;         break; // BOTTOM hold — reset
    case 2: untapped += tapped; tapped = 0;   break; // LEFT hold   — untap all
    case 3: tapped += untapped; untapped = 0; break; // RIGHT hold  — tap all
  }
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  for (int i = 0; i < 7; i++) { pinMode(SEG[i], OUTPUT); digitalWrite(SEG[i], HIGH); }
  for (int i = 0; i < 4; i++) { pinMode(DIG[i], OUTPUT); digitalWrite(DIG[i], LOW); }
  pinMode(DP_PIN, OUTPUT);
  digitalWrite(DP_PIN, HIGH);

  for (int i = 0; i < 4; i++) pinMode(BUTTONS[i], INPUT_PULLUP);

  buildAnimFrame();
  updateDisplay();
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // — Button polling —
  for (int i = 0; i < 4; i++) {
    int raw = digitalRead(BUTTONS[i]);

    if (raw != lastRaw[i]) lastChange[i] = now;
    lastRaw[i] = raw;

    if (now - lastChange[i] < DEBOUNCE_MS) continue;

    if (raw == LOW && !wasPressed[i]) {
      wasPressed[i] = true;
      pressStart[i] = now;
      longFired[i]  = false;

      // Any press dismisses the animation immediately
      if (animating) {
        animating = false;
        updateDisplay();
      }
    }

    // Skip long-press / short-press logic while animating
    if (animating) { if (raw != stable[i]) stable[i] = raw; continue; }

    if (raw == LOW && wasPressed[i] && !longFired[i]) {
      if (now - pressStart[i] >= LONG_PRESS_MS) {
        handleLong(i);
        updateDisplay();
        longFired[i] = true;
      }
    }

    if (raw == HIGH && wasPressed[i]) {
      if (!longFired[i]) { handleShort(i); updateDisplay(); konamiRecord(i); }
      wasPressed[i] = false;
    }

    if (raw != stable[i]) stable[i] = raw;
  }

  // — Animation step —
  if (animating) {
    static unsigned long lastAnimStep = 0;
    if (now - lastAnimStep >= ANIM_STEP_MS) {
      lastAnimStep = now;
      animPos = (animPos + 1) % 12;
      buildAnimFrame();
    }
  }

  // — Display multiplexing —
  static int curDigit = 0;
  static unsigned long lastMux = 0;

  if (micros() - lastMux >= MUX_STEP_US) {
    lastMux = micros();

    // blank previous digit
    digitalWrite(DIG[(curDigit + 3) % 4], LOW);

    if (animating) {
      // Drive segments from animation bitmask; DP always off during animation
      digitalWrite(DP_PIN, HIGH);
      for (int s = 0; s < 7; s++) {
        digitalWrite(SEG[s], (animSegs[curDigit] & (1 << s)) ? LOW : HIGH);
      }
    } else {
      // Normal counter display
      digitalWrite(DP_PIN, curDigit == 1 ? LOW : HIGH);
      for (int s = 0; s < 7; s++) {
        digitalWrite(SEG[s], PATTERNS[digits[curDigit]][s]);
      }
    }

    digitalWrite(DIG[curDigit], HIGH);
    curDigit = (curDigit + 1) % 4;
  }
}
