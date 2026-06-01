int seg[] = {2, 3, 4, 5, 6, 8, 13};
int dig[] = {9, 10, 11, 12};

byte digits[10][7] = {
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

int number[4] = {0, 0, 0, 0};
unsigned long lastUpdate = 0;
int count = 0;

void displayNumber(int n) {
  number[0] = n / 1000;
  number[1] = (n / 100) % 10;
  number[2] = (n / 10) % 10;
  number[3] = n % 10;
}

void setup() {
  for (int i = 0; i < 7; i++) {
    pinMode(seg[i], OUTPUT);
    digitalWrite(seg[i], HIGH);
  }
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  for (int i = 0; i < 4; i++) {
    pinMode(dig[i], OUTPUT);
    digitalWrite(dig[i], LOW);
  }
}

void loop() {
  if (millis() - lastUpdate > 100) {
    lastUpdate = millis();
    count++;
    if (count > 9999) count = 0;
    displayNumber(count);
  }

  for (int d = 0; d < 4; d++) {
    digitalWrite(dig[d], HIGH);
    for (int s = 0; s < 7; s++) {
      digitalWrite(seg[s], digits[number[d]][s]);
    }
    delay(5);
    for (int s = 0; s < 7; s++) digitalWrite(seg[s], HIGH);
    digitalWrite(dig[d], LOW);
  }
}