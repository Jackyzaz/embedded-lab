// กำหนดขา LED
#define LED_R 15
#define LED_Y 0
#define LED_G 2

// กำหนดขา DIP Switch 
#define SW_16 16  // S8
#define SW_12 12  // S5
#define SW_13 13  // S13
#define SW_5  5   // S7

// Helper function
void setLED(bool r, bool y, bool g) { 
  digitalWrite(LED_R, r);         
  digitalWrite(LED_Y, y);
  digitalWrite(LED_G, g);
}

void setup() {
  pinMode(LED_R, OUTPUT);   // ตั้งค่า Output LED
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);

  pinMode(SW_16, INPUT);    // ตั้งค่า Dip Switch
  pinMode(SW_12, INPUT);
  pinMode(SW_13, INPUT);
  pinMode(SW_5, INPUT);

  Serial.begin(115200);     // ตั้งค่า Serial Communication
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
    setLED(0, 0, 0);  // LED ดับทุกดวง
  } 
  else if (s16 == 0 && s5 == 0 && s13 == 0 && s12 == 1) {
    setLED(1, 1, 0);  // LED แดงและเขียวติด
  } 
  else if (s16 == 0 && s5 == 0 && s13 == 1 && s12 == 0) {
    setLED(1, 0, 1);  // LED แดงและเหลืองติด
  } 
  else if (s16 == 0 && s5 == 0 && s13 == 1 && s12 == 1) {
    setLED(0, 1, 1);  // LED เขียวและเหลืองติด
  } 
  else if (s16 == 0 && s5 == 1 && s13 == 0 && s12 == 0) {
    setLED(1, 1, 1);  // LED ทุกดวงติดสลับดับ
    delay(1000);
    setLED(0, 0, 0);
    delay(1000);
  }
}
