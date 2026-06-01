// MTG Token Counter — main firmware
// Display: 3461BS-1 common-anode 4-digit 7-segment (see display_pinout.md)
// Buttons: 4 tactile switches, active LOW (INPUT_PULLUP)

// ── Display pins ──────────────────────────────────────────────────────────────
const int SEG[]  = {2, 3, 4, 5, 6, 8, 13}; // A F B E D C G
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
const unsigned long DEBOUNCE_MS  = 20;
const unsigned long LONG_PRESS_MS = 500;
const unsigned long MUX_STEP_US  = 2500; // ~2.5 ms per digit → ~100 Hz refresh

// ── Button state ──────────────────────────────────────────────────────────────
int  lastRaw[4]       = {HIGH, HIGH, HIGH, HIGH};
int  stable[4]        = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastChange[4]  = {0, 0, 0, 0};
unsigned long pressStart[4]  = {0, 0, 0, 0};
bool longFired[4]     = {false, false, false, false};
bool wasPressed[4]    = {false, false, false, false};

// ── Counter state ─────────────────────────────────────────────────────────────
int untapped = 0;
int tapped   = 0;

// ── Display buffer ────────────────────────────────────────────────────────────
int digits[4]; // digit values to show

void updateDisplay() {
  digits[0] = (untapped / 10) % 10;
  digits[1] = untapped % 10;
  digits[2] = (tapped / 10) % 10;
  digits[3] = tapped % 10;
}

// ── Action handlers ───────────────────────────────────────────────────────────
void handleShort(int btn) {
  switch (btn) {
    case 0: // TOP — +1 untapped
      untapped = min(untapped + 1, 99);
      break;
    case 1: // BOTTOM — -1 untapped
      untapped = max(untapped - 1, 0);
      break;
    case 2: // LEFT — untap 1
      if (tapped > 0) { tapped--; untapped++; }
      break;
    case 3: // RIGHT — tap 1
      if (untapped > 0) { untapped--; tapped++; }
      break;
  }
}

void handleLong(int btn) {
  switch (btn) {
    case 0: // TOP hold — +2 untapped
      untapped = min(untapped + 2, 99);
      break;
    case 1: // BOTTOM hold — reset
      untapped = 0;
      tapped   = 0;
      break;
    case 2: // LEFT hold — untap all
      untapped += tapped;
      tapped    = 0;
      break;
    case 3: // RIGHT hold — tap all
      tapped  += untapped;
      untapped = 0;
      break;
  }
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  for (int i = 0; i < 7; i++) { pinMode(SEG[i], OUTPUT); digitalWrite(SEG[i], HIGH); }
  for (int i = 0; i < 4; i++) { pinMode(DIG[i], OUTPUT); digitalWrite(DIG[i], LOW); }
  pinMode(DP_PIN, OUTPUT);
  digitalWrite(DP_PIN, HIGH); // DP off by default

  for (int i = 0; i < 4; i++) pinMode(BUTTONS[i], INPUT_PULLUP);

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
      // just pressed
      wasPressed[i]  = true;
      pressStart[i]  = now;
      longFired[i]   = false;
    }

    if (raw == LOW && wasPressed[i] && !longFired[i]) {
      if (now - pressStart[i] >= LONG_PRESS_MS) {
        handleLong(i);
        updateDisplay();
        longFired[i] = true;
      }
    }

    if (raw == HIGH && wasPressed[i]) {
      // released
      if (!longFired[i]) {
        handleShort(i);
        updateDisplay();
      }
      wasPressed[i] = false;
    }

    if (raw != stable[i]) stable[i] = raw;
  }

  // — Display multiplexing (non-blocking, one digit per loop pass) —
  static int curDigit = 0;
  static unsigned long lastMux = 0;

  if (micros() - lastMux >= MUX_STEP_US) {
    lastMux = micros();

    // blank previous digit
    digitalWrite(DIG[(curDigit + 3) % 4], LOW);

    // enable decimal point between digit 2 and 3 as separator
    digitalWrite(DP_PIN, curDigit == 1 ? LOW : HIGH);

    // write segments for current digit
    for (int s = 0; s < 7; s++) digitalWrite(SEG[s], PATTERNS[digits[curDigit]][s]);

    // enable digit
    digitalWrite(DIG[curDigit], HIGH);

    curDigit = (curDigit + 1) % 4;
  }
}
