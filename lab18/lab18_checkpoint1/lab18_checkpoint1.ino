// กำหนดขา LED
#define LED_R 15
#define LED_Y 0
#define LED_G 2

// กำหนดขา DIP Switch 
#define SW_16 16  // S8
#define SW_12 12  // S5
#define SW_13 13  // S13
#define SW_5  5   // S7

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);

  pinMode(SW_16, INPUT);
  pinMode(SW_12, INPUT);
  pinMode(SW_13, INPUT);
  pinMode(SW_5, INPUT);

  Serial.begin(115200);
}

void setLED(bool r, bool y, bool g) {
  digitalWrite(LED_R, r );
  digitalWrite(LED_Y, y );
  digitalWrite(LED_G, g );
}

void loop() {
  int s16 = digitalRead(SW_16);
  int s12 = digitalRead(SW_12);
  int s13 = digitalRead(SW_13);
  int s5  = digitalRead(SW_5);
  
  Serial.print("s16, s5, s13, s12: ");
  Serial.print(s16);
  Serial.print(s5);
  Serial.print(s13);
  Serial.println(s12);

  if (s16 == 1) {
    // red blink
    setLED(1, 0, 0);
    delay(1000);
    setLED(0, 0, 0);
    delay(1000);
  } 
  else if (s16 == 0 && s5 == 0 && s13 == 0 && s12 == 0) {
    // all off
    setLED(0, 0, 0);
  } 
  else if (s16 == 0 && s5 == 0 && s13 == 0 && s12 == 1) {
    // red green
    setLED(1, 1, 0);
  } 
  else if (s16 == 0 && s5 == 0 && s13 == 1 && s12 == 0) {
    // red yellow
    setLED(1, 0, 1);
  } 
  else if (s16 == 0 && s5 == 0 && s13 == 1 && s12 == 1) {
    // green yellow
    setLED(0, 1, 1);
  } 
  else if (s16 == 0 && s5 == 1 && s13 == 0 && s12 == 0) {
    // all blink
    setLED(1, 1, 1);
    delay(1000);
    setLED(0, 0, 0);
    delay(1000);
  }
}
