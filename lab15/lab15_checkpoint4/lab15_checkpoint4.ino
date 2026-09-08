#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "my_wdt.h"

const int strobe = A0; 
const int clock  = A1; 
const int data   = A2; 

const uint8_t digits[] = {
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 
  0x6D, 0x7D, 0x07, 0x7F, 0x6F  
};

// ตัวอักษรพิเศษ
const uint8_t CHAR_D    = 0x5E; // 'd'
const uint8_t CHAR_N    = 0x54; // 'n'
const uint8_t CHAR_U    = 0x1C; // 'u'
const uint8_t CHAR_P    = 0x73; // 'P'
const uint8_t CHAR_DASH = 0x40; // ขีดกลาง '-'
const uint8_t CHAR_BLANK= 0x00; // ปิดดับ

#define MODE_DOWN 0
#define MODE_UP   1

volatile uint8_t current_mode = MODE_DOWN; // บันทึกโหมดล่าสุด
volatile bool woken_up = false;            // แฟล็กสำหรับระบุว่าตื่นจากการกดปุ่ม
unsigned long last_activity_time = 0;
uint8_t last_buttons = 0;

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
  shiftOut(data, clock, LSBFIRST, 0x42); // คำสั่งอ่านสถานะคีย์บอร์ด

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
    shiftOut(data, clock, LSBFIRST, 0x00);   // ปิด LED
  }
  digitalWrite(strobe, HIGH);
}

// อัปเดตเวลาตามโหมดปัจจุบัน (หน่วย 1/100 วินาที)
void display_time(int current_cs) {
  uint8_t sec = current_cs / 100;
  uint8_t cs  = current_cs % 100;

  uint8_t raw[8];
  if (current_mode == MODE_DOWN) {
    raw[0] = CHAR_D;
    raw[1] = CHAR_N;
  } else {
    raw[0] = CHAR_U;
    raw[1] = CHAR_P;
  }
  raw[2] = CHAR_BLANK;
  raw[3] = digits[sec / 10];
  raw[4] = digits[sec % 10];
  raw[5] = CHAR_DASH;
  raw[6] = digits[cs / 10];
  raw[7] = digits[cs % 10];

  update_display_raw(raw);
}

// แสดงผลหน้าจอเริ่มต้นของโหมดปัจจุบัน
void show_current_mode() {
  if (current_mode == MODE_DOWN) {
    display_time(1200); // แสดง "dn  12-00" 
  } else {
    display_time(0);    // แสดง "uP 00-00"
  }
}

// กระพริบค่าสุดท้าย 2 ครั้ง
void display_blink_final(bool on) {
  if (!on) {
    reset();
    return;
  }
  uint8_t raw[8] = { CHAR_BLANK, CHAR_BLANK, CHAR_BLANK };
  if (current_mode == MODE_DOWN) {
    // รูป 15.5 (b): "   00-00"
    raw[3] = digits[0];
    raw[4] = digits[0];
    raw[5] = CHAR_DASH;
    raw[6] = digits[0];
    raw[7] = digits[0];
  } else {
    // รูป 15.5 (d): "   12-00"
    raw[3] = digits[1];
    raw[4] = digits[2];
    raw[5] = CHAR_DASH;
    raw[6] = digits[0];
    raw[7] = digits[0];
  }
  update_display_raw(raw);
}

// Watchdog (ทำงานทุก ๆ 4 วินาที ขณะ CPU หลับ)
ISR(WDT_vect) {
  // อ่านค่าคีย์ของบอร์ด TM1638 เพื่อตรวจสอบว่ามีการกดปุ่มใด ๆ หรือไม่
  uint8_t buttons = readTM1638_Buttons();
  if (buttons != 0) {
    woken_up = true; // มีการกดปุ่ม -> อนุญาตให้ตื่นจากภาวะหลับ
  }
}

void enter_sleep() {
  // 1. ดับหน้าจอและล้างค่าใน TM1638 เพื่อประหยัดพลังงาน
  sendCommand(0x80); // Display OFF
  reset();

  // 2. เปิด Watchdog Interrupt ทุก 4 วินาที
  woken_up = false;
  WDT_interrupt_enable(wdt_timeout_4sec);

  // 3. วนลูปเข้าสู่ภาวะหลับจนกว่าจะมีปุ่มกดใน WDT ISR
  while (!woken_up) {
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_bod_disable(); // ปิด BOD
    sleep_cpu();         // หลับลึก (CPU หยุดทำงาน)
    sleep_disable();
  }

  // 4. เมื่อตื่นจากการกดปุ่ม ให้ปิด WDT
  WDT__disable();

  // 5. เปิดจอ TM1638 และแสดงผลโหมดล่าสุดทันทีที่ตื่น (ไม่ต้องรอปล่อยปุ่ม)
  sendCommand(0x8F); // Display ON, Brightness สูงสุด
  reset();
  show_current_mode();

  // 6. บันทึกสถานะปุ่มที่กดค้างอยู่เข้า last_buttons
    // เพื่อป้องกันไม่ให้กลายเป็นจังหวะกดใหม่ (Rising Edge) ใน loop()
  last_buttons = readTM1638_Buttons();
}

// ฟังก์ชันนับถอยหลัง 12 วินาที (MODE_DOWN)
void countdown() {
  int remaining_cs = 1200;
  display_time(remaining_cs);

  unsigned long last_tick = millis();

  while (remaining_cs > 0) {
    if (millis() - last_tick >= 10) {
      last_tick += 10;
      remaining_cs--;
      display_time(remaining_cs);
    }
  }

  // กระพริบค่า 00-00 สองครั้ง
  for (int i = 0; i < 3; i++) {
    display_blink_final(true);
    _delay_ms(1000);
    display_blink_final(false);
    _delay_ms(1000);
  }
}

void countup() {
  int current_cs = 0;
  display_time(current_cs);

  unsigned long last_tick = millis();

  while (current_cs < 1200) {
    if (millis() - last_tick >= 10) {
      last_tick += 10;
      current_cs++;
      display_time(current_cs);
    }
  }

  // กระพริบค่า 12-00 สองครั้ง
  for (int i = 0; i < 2; i++) {
    display_blink_final(true);
    _delay_ms(300);
    display_blink_final(false);
    _delay_ms(300);
  }
}

void setup() {
  pinMode(strobe, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);

  // เริ่มต้นจอ TM1638
  sendCommand(0x8F);
  reset();

  // แสดงผลเริ่มต้นในโหมดนับถอยหลัง "dn  12-00" (รูป 15.5 a)
  current_mode = MODE_DOWN;
  show_current_mode();

  last_activity_time = millis();
  sei(); // เปิดใช้งาน Interrupt
}

void loop() {
  uint8_t buttons = readTM1638_Buttons();
  uint8_t pressed = buttons & ~last_buttons; // ตรวจจับขอบขาขึ้น (Rising Edge)
  last_buttons = buttons;

  // 1. กดสวิตช์ S1 เพื่อสลับโหมด (DOWN <-> UP)
  if (pressed & 0x01) {
    current_mode = (current_mode == MODE_DOWN) ? MODE_UP : MODE_DOWN;
    show_current_mode();
    last_activity_time = millis(); // รีเซ็ตเวลา 5 วินาที
  }

  // 2. กดสวิตช์ S8 เพื่อเริ่มนับ
  else if (pressed & 0x80) {
    if (current_mode == MODE_DOWN) {
      countdown();
      enter_sleep();
    } else {
      countup();
      sendCommand(0x80);
      reset();
      // รอตรวจว่าไม่มีการกดสวิตช์ใดเลยเป็นเวลา 5 วินาที ก่อนเข้าสู่ภาวะหลับ
      unsigned long wait_start = millis();
      bool key_pressed = false;
      while (millis() - wait_start < 5000) {
        if (readTM1638_Buttons() != 0) {
          key_pressed = true;
          break;
        }
        _delay_ms(20);
      }
      if (!key_pressed) {
        enter_sleep();
      } else {
        sendCommand(0x8F);  // wake up
        show_current_mode();
      }
    }
    last_activity_time = millis();
  }

  // 3. หากไม่มีการกดสวิตช์ใด ๆ เป็นเวลา 5 วินาที ให้ดับจอและเข้าสู่ภาวะหลับ
  if (millis() - last_activity_time >= 5000) {
    enter_sleep();
    last_activity_time = millis();
  }

  _delay_ms(20);
}
