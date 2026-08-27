// -------------------------------------------------------------
// Blynk Credentials & Template Info
// -------------------------------------------------------------
#define BLYNK_TEMPLATE_ID "TMPL6LF86qGfa"
#define BLYNK_TEMPLATE_NAME "Checkpoint3"
#define BLYNK_AUTH_TOKEN "xlNQW7y7m7Q3EtesImkPrpHmbltR93BB"
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
int PWMRANGE = 1023;
// WiFi credentials
char ssid[] = "Jack>/~";
char pass[] = "nmcu1234";

// -------------------------------------------------------------
// Pin Definitions
// -------------------------------------------------------------
// LEDs บนบอร์ด NodeMCU
#define R_LED 15   // GPIO15 (D8) - แอลอีดีสีแดง
#define G_LED 2    // GPIO2  (D4) - แอลอีดีสีเขียว
#define Y_LED 0    // GPIO0  (D3) - แอลอีดีสีเหลือง

// DIP Switch Bit 5 - 8 (Mode Control บนบอร์ด)
#define SW_BIT5 12 // GPIO12 (D6) - ดิปสวิตช์บิตที่ 5
#define SW_BIT6 13 // GPIO13 (D7) - ดิปสวิตช์บิตที่ 6
#define SW_BIT7 5  // GPIO5  (D1) - ดิปสวิตช์บิตที่ 7
#define SW_BIT8 16 // GPIO16 (D0) - ดิปสวิตช์บิตที่ 8

// DIP Switch Bit 1 - 2 (จำลอง Sensor สองบิตล่าง)
#define SW_DOOR_FRONT 4  // GPIO4  (D2) - บิตที่ 1: ประตูหน้าบ้าน
#define SW_DOOR_BACK  14 // GPIO14 (D5) - บิตที่ 2: ประตูหลังบ้าน

// -------------------------------------------------------------
// Blynk Virtual Pins Mapping 
// 1. Button Widgets (บนโทรศัพท์มือถือ):
//    - V4: ปุ่มควบคุม RED LED
//    - V5: ปุ่มควบคุม GREEN LED
//    - V6: ปุ่มควบคุม YELLOW LED
// 2. LED Widgets (แสดงสถานะเซนเซอร์ 2 ตัว):
//    - V0: LED Widget "ประตูหน้าบ้านถูกปิดแล้ว" (DIP Switch บิตที่ 1)
//    - V1: LED Widget "ประตูหลังบ้านถูกปิดแล้ว" (DIP Switch บิตที่ 2)
// -------------------------------------------------------------
int blynk_btn_r = 0;
int blynk_btn_g = 0;
int blynk_btn_y = 0;

BlynkTimer timer;

// ฟังก์ชันรับค่าจาก Button Widgets บนแอป Blynk (V4, V5, V6)
BLYNK_WRITE(V4) {
  blynk_btn_r = param.asInt();
}

BLYNK_WRITE(V5) {
  blynk_btn_g = param.asInt();
}

BLYNK_WRITE(V6) {
  blynk_btn_y = param.asInt();
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V4, V5, V6);
}

// -------------------------------------------------------------
// ฟังก์ชันปิด LED ทั้งหมด
// -------------------------------------------------------------
void turnOffAllLEDs() {
  analogWrite(R_LED, 0);
  analogWrite(G_LED, 0);
  analogWrite(Y_LED, 0);
}

// -------------------------------------------------------------
// Task หลัก: อ่านค่า Switch, คำนวณ Mode ตามตารางที่ 2 และส่งค่า Sensor ไปยัง Blynk
// -------------------------------------------------------------
void updateControl() {
  // 1. อ่านค่า DIP Switch Bit 5-8 (GPIO16, 5, 13, 12)
  int s16 = digitalRead(SW_BIT8);
  int s5  = digitalRead(SW_BIT7);
  int s13 = digitalRead(SW_BIT6);
  int s12 = digitalRead(SW_BIT5);

  // คำนวณรูปแบบ 4 บิต (MSB -> LSB: s16, s5, s13, s12)
  int mode = (s16 << 3) | (s5 << 2) | (s13 << 1) | s12;

  // ตรวจสอบรอบการกระพริบ 1 วินาทีแบบ Non-blocking (ติด 1s / ดับ 1s รวม 2s)
  bool blinkState = ((millis() / 1000) % 2) == 0;

  // คำนวณค่า PWM Duty Cycle ตามความสว่างที่กำหนด (PWMRANGE = 1023)
  int pwm20 = (PWMRANGE * 20) / 100; // ~204 (20%)
  int pwm50 = (PWMRANGE * 50) / 100; // ~511 (50%)
  int pwm70 = (PWMRANGE * 70) / 100; // ~716 (70%)

  // 2. ควบคุม LED ทั้งสามดวงตามโหมดในตารางที่ 2
  switch (mode) {
    case 0b1000: // 1000: แอลอีดีทุกดวงกระพริบติดดับพร้อมกันทุก ๆ 1 วินาที
      if (blinkState) {
        digitalWrite(R_LED, HIGH);
        digitalWrite(G_LED, HIGH);
        digitalWrite(Y_LED, HIGH);
      } else {
        turnOffAllLEDs();
      }
      break;

    case 0b1001: // 1001: แอลอีดีสีแดงถูกควบคุมโดย Button บนมือถือ (V4), ดวงอื่นดับตลอดเวลา
      digitalWrite(R_LED, blynk_btn_r );
      digitalWrite(G_LED, LOW);
      digitalWrite(Y_LED, LOW);
      break;

    case 0b1010: // 1010: แอลอีดีสีเขียวถูกควบคุมโดย Button บนมือถือ (V5), ดวงอื่นดับตลอดเวลา
      digitalWrite(R_LED, LOW);
      digitalWrite(G_LED, blynk_btn_g );
      digitalWrite(Y_LED, LOW);
      break;

    case 0b1011: // 1011: แอลอีดีสีเหลืองถูกควบคุมโดย Button บนมือถือ (V6), ดวงอื่นดับตลอดเวลา
      digitalWrite(R_LED, LOW);
      digitalWrite(G_LED, LOW);
      digitalWrite(Y_LED, blynk_btn_y );
      break;

    case 0b0100: // 0100: แอลอีดีทั้งสามดวงถูกควบคุมโดย Button บนโทรศัพท์มือถือ (V4, V5, V6)
      digitalWrite(R_LED, blynk_btn_r );
      digitalWrite(G_LED, blynk_btn_g );
      digitalWrite(Y_LED, blynk_btn_y );
      break;

    case 0b0101: // 0101: แอลอีดีสีแดงกระพริบติดสลับดับทุก ๆ 1 วินาที ความสว่าง 20%
      if (blinkState) {
        analogWrite(R_LED, pwm20);
      } else {
        analogWrite(R_LED, 0);
      }
      digitalWrite(G_LED, LOW);
      digitalWrite(Y_LED, LOW);
      break;

    case 0b0110: // 0110: แอลอีดีสีเขียวกระพริบติดสลับดับทุก ๆ 1 วินาที ความสว่าง 50%
      if (blinkState) {
        analogWrite(G_LED, pwm50);
      } else {
        analogWrite(G_LED, 0);
      }
      digitalWrite(R_LED, LOW);
      digitalWrite(Y_LED, LOW);
      break;

    case 0b0111: // 0111: แอลอีดีสีเหลืองกระพริบติดสลับดับทุก ๆ 1 วินาที ความสว่าง 70%
      if (blinkState) {
        analogWrite(Y_LED, pwm70);
      } else {
        analogWrite(Y_LED, 0);
      }
      digitalWrite(R_LED, LOW);
      digitalWrite(G_LED, LOW);
      break;

    default: // รูปแบบอื่น ๆ ให้ดับ LED ทั้งหมดเพื่อความปลอดภัย
      turnOffAllLEDs();
      break;
  }

  // 3. อ่านค่า DIP Switch สองบิตล่าง (Bit 1 และ Bit 2) ส่งไปยัง Blynk LED Widget
  int doorFrontStatus = digitalRead(SW_DOOR_FRONT); // 1 = สวิตช์เปิด/ปิดประตูหน้าบ้าน
  int doorBackStatus  = digitalRead(SW_DOOR_BACK);  // 1 = สวิตช์เปิด/ปิดประตูหลังบ้าน

  // ส่งค่าสถานะไปยัง LED Widget บน Blynk (255 = ติดสว่าง, 0 = ดับ)
  Blynk.virtualWrite(V0, doorFrontStatus ? 255 : 0);
  Blynk.virtualWrite(V1, doorBackStatus ? 255 : 0);
}

void setup() {
  Serial.begin(115200);

  // กำหนดโหมดขา LED
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(Y_LED, OUTPUT);
  turnOffAllLEDs();

  // กำหนดโหมดขา Mode DIP Switch (Bit 5 - Bit 8)
  pinMode(SW_BIT5, INPUT);
  pinMode(SW_BIT6, INPUT);
  pinMode(SW_BIT7, INPUT);
  pinMode(SW_BIT8, INPUT);

  // กำหนดโหมดขา Sensor DIP Switch (Bit 1 - Bit 2)
  pinMode(SW_DOOR_FRONT, INPUT);
  pinMode(SW_DOOR_BACK, INPUT);

  // เริ่มเชื่อมต่อ Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // ตั้งเวลาเรียกฟังก์ชัน updateControl ทุกๆ 100 ms (Non-blocking)
  timer.setInterval(100L, updateControl);
}

void loop() {
  Blynk.run();
  timer.run();
}
