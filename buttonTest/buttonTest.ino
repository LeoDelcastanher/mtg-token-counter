const int BUTTON_PIN = 2;
const int LED_PIN = 13;

int lastState = HIGH;
int stableState = HIGH;
unsigned long lastChangeTime = 0;
const unsigned long DEBOUNCE_DELAY = 20;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastState) {
    lastChangeTime = millis();
  }

  if (millis() - lastChangeTime > DEBOUNCE_DELAY) {
    if (reading != stableState) {
      stableState = reading;
      if (stableState == LOW) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("Pressed");
      } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("Released");
      }
    }
  }

  lastState = reading;
}