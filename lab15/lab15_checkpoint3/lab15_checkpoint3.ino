#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

// กำหนดขาเชื่อมต่อกับ TM1638 ตามรูปที่ 15.3
const int strobe = A0; // STB ต่อกับ A0
const int clock  = A1; // CLK ต่อกับ A1
const int data   = A2; // DIO ต่อกับ A2

// ตารางรหัส 7-segment ตัวเลข 0 - 9
const uint8_t digits[] = {
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 
  0x6D, 0x7D, 0x07, 0x7F, 0x6F  
};

// ตัวอักษรพิเศษ
const uint8_t CHAR_D = 0x5E; // ตัว 'd'
const uint8_t CHAR_N = 0x54; // ตัว 'n'
const uint8_t CHAR_DASH = 0x40; // ขีดกลาง '-'
const uint8_t CHAR_BLANK = 0x00; // ปิดดับ

// --- ฟังก์ชันพื้นฐานควบคุม TM1638 ---
void sendCommand(uint8_t value) {
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}

void reset() {
  sendCommand(0x40); // Auto-increment address
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xC0);
  for (uint8_t i = 0; i < 16; i++) {
    shiftOut(data, clock, LSBFIRST, 0x00);
  }
  digitalWrite(strobe, HIGH);
}

uint8_t readTM1638_Buttons(void) {
  uint8_t buttons = 0;
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0x42); // คำสั่งอ่านคีย์บอร์ด

  pinMode(data, INPUT);
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t v = shiftIn(data, clock, LSBFIRST) << i;
    buttons |= v;
  }
  pinMode(data, OUTPUT);
  digitalWrite(strobe, HIGH);
  return buttons;
}

// อัปเดตข้อมูล 7-segment ทั้ง 8 หลักรวดเดียวด้วยโหมด Auto-increment
void update_display_raw(const uint8_t raw[8]) {
  sendCommand(0x40); // Auto increment
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xC0);
  for (uint8_t i = 0; i < 8; i++) {
    shiftOut(data, clock, LSBFIRST, raw[i]); // ข้อมูล 7-segment
    shiftOut(data, clock, LSBFIRST, 0x00);   // ข้อมูล LED ประจำหลัก (ปิด)
  }
  digitalWrite(strobe, HIGH);
}

// แสดงผล "dn  12-00"
void display_initial() {
  uint8_t raw[8] = {
    CHAR_D,         // หลักที่ 1: 'd'
    CHAR_N,         // หลักที่ 2: 'n'
    CHAR_BLANK,     // หลักที่ 3: ว่าง
    digits[1],      // หลักที่ 4: '1'
    digits[2],      // หลักที่ 5: '2'
    CHAR_DASH,      // หลักที่ 6: '-'
    digits[0],      // หลักที่ 7: '0'
    digits[0]       // หลักที่ 8: '0'
  };
  update_display_raw(raw);
}

// อัปเดตเวลาที่เหลือนับถอยหลัง (หน่วย 1/100 วินาที)
void display_time(int remaining_cs) {
  uint8_t sec = remaining_cs / 100;
  uint8_t cs  = remaining_cs % 100;

  uint8_t raw[8] = {
    CHAR_D,
    CHAR_N,
    CHAR_BLANK,
    digits[sec / 10],
    digits[sec % 10],
    CHAR_DASH,
    digits[cs / 10],
    digits[cs % 10]
  };
  update_display_raw(raw);
}

// แสดงผล "   00-00" (3 ตัวแรกดับ)
void display_zero(bool on) {
  if (!on) {
    reset();
    return;
  }
  uint8_t raw[8] = {
    CHAR_BLANK,
    CHAR_BLANK,
    CHAR_BLANK,
    digits[0],
    digits[0],
    CHAR_DASH,
    digits[0],
    digits[0]
  };
  update_display_raw(raw);
}

void enter_sleep() {
  // 1. ดับหน้าจอและล้างรีจิสเตอร์ของ TM1638 ก่อนเพื่อลดการกินกระแส
  sendCommand(0x80); // Display OFF
  reset();
  // 2. ปิดและตั้งค่า sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_bod_disable(); // ปิด Brown-Out Detector เพื่อประหยัดพลังงานสูงสุด
  sleep_cpu();         // เข้าสู่ภาวะหลับทันที
  sleep_disable();
}

// ฟังก์ชันนับถอยหลัง 12 วินาที (เปลี่ยนค่าทุก 1/100 วินาที = 10ms)
void countdown() {
  int remaining_cs = 1200; // 12.00 วินาที = 12000 ms  
  display_time(remaining_cs);

  unsigned long last_tick = millis();

  while (remaining_cs > 0) {
    if (millis() - last_tick >= 10) {
      last_tick += 10;
      remaining_cs--;
      display_time(remaining_cs);
    }
  }

  // หลังจากถึง 00-00 แล้ว ให้กระพริบ 2 ครั้ง
  for (int i = 0; i < 2; i++) {
    display_zero(true);
    _delay_ms(1000);
    display_zero(false);
    _delay_ms(1000);
  }
}

void setup() {
  pinMode(strobe, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);
  // เริ่มต้นเปิดจอ TM1638 และเคลียร์หน้าจอ
  sendCommand(0x8F); // Display ON, Brightness สูงสุด
  reset();
  // แสดงผลเริ่มต้น "dn  12-00"
  display_initial();
  // รอการกดสวิตช์ S8 หรือรอจนหมดเวลา 5 วินาที
  unsigned long start_time = millis();
  bool s8_pressed = false;
  while (millis() - start_time < 5000) {
    uint8_t buttons = readTM1638_Buttons();
    if (buttons & 0x80) { // สวิตช์ S8 คือบิต 7 (0x80)
      s8_pressed = true;
      break;
    }
    _delay_ms(20); // ดีเลย์เล็กน้อยเพื่อลดความถี่ในการสแกนปุ่ม
  }
  if (s8_pressed) {
    // รอจนปล่อยปุ่ม S8 ป้องกันการสั่งงานซ้ำ
    while (readTM1638_Buttons() & 0x80);
    _delay_ms(20);
    // ดำเนินการนับถอยหลัง 12 วินาที
    countdown();
  }
  // หากไม่มีการกดปุ่มใน 5 วินาที หรือนับถอยหลังและกระพริบเสร็จแล้ว ให้เข้าสู่ภาวะหลับ
  enter_sleep();
}

void loop() {
  enter_sleep();
}
