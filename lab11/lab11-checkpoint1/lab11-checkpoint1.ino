#include <avr/interrupt.h>
#include <avr/io.h>

// TM1638 connection
const int strobe = 7;
const int clock = 9;
const int data = 8;

// TCNT1 100ms N = 1024 (16MHz)
const uint16_t HUNDRED_MILLI_SEC = 63973;

// shared var
uint8_t timer = 0;
uint8_t second = 58;
uint8_t minute = 59;
uint8_t hour = 0;


ISR(TIMER1_OVF_vect) {
  TCNT1 = HUNDRED_MILLI_SEC;
  timer++;
  if (timer == 10) {
    timer = 0;
    second++;
    if (second == 60) {
      second = 0;         
      minute++;
      if (minute == 60) {
        minute = 0; 
        hour++;
        if (hour == 24) {
          hour = 0; 
        }
      }
    }
  }
}

void setup() {
  timer = 0;
  // ตั้งค่า Timer 1
  TIMSK1 |= 0x01; // Enable Timer1 Overflow Interrupt
  TCCR1A = 0x00;  // Normal Mode
  TCCR1B = 0x05;  // CS1[2:0] = 101 -> เลือก Prescaler /1024 
  TCCR1C = 0x00;
  TCNT1 = HUNDRED_MILLI_SEC;
  // กำหนดทิศทางของพินเป็น OUTPUT
  pinMode(strobe, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);
  sendCommand(0x8f);  // เปิดการทำงานหน้าจอแสดงผล (ความสว่างสูงสุด)
  reset();            // เคลียร์ค่าตัวเลขในจอแสดงผล
  sei();              // เปิดใช้งาน Interrupt ทั่วไป
}

void loop() {
  display_clock();    
}

void sendCommand(uint8_t value)
{
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}
  
void reset()
{
  sendCommand(0x40); // กำหนดโหมดเขียนแบบเลื่อนตำแหน่ง Address อัตโนมัติ (Auto increment)
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc0);   // กำหนดตำแหน่งเริ่มต้น Address ที่ 0 (0xc0)
  for(uint8_t i = 0; i < 16; i++)
  {
    shiftOut(data, clock, LSBFIRST, 0x00); // ล้างข้อมูลหลักแสดงผลทั้ง 16 บล็อก
  }
  digitalWrite(strobe, HIGH);
}

void display_clock()
{
  static uint8_t digits[] = { 0x3f, 0x06, 0x5b, 0x4f, 
  0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };

  sendCommand(0x44);  // Fixed address

  uint8_t seperator = 0x40; // รหัส 7-segment สำหรับตัวขีดกลาง '-'
  
  // คำนวณหาหลักสิบและหลักหน่วยของเวลาปัจจุบัน
  uint8_t hour_low = hour % 10;
  uint8_t hour_high = hour / 10;

  uint8_t minute_low = minute % 10;
  uint8_t minute_high = minute / 10;

  uint8_t second_low = second % 10;
  uint8_t second_high = second / 10;

  // ส่งข้อมูลไปอัปเดตที่บอร์ด TM1638 ทีละตำแหน่ง (มีขีดคั่นกลาง)
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc0); // หลักที่ 1: ชั่วโมงหลักสิบ
  shiftOut(data, clock, LSBFIRST, digits[hour_high]); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc2); // หลักที่ 2: ชั่วโมงหลักหน่วย
  shiftOut(data, clock, LSBFIRST, digits[hour_low]); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc4); // หลักที่ 3: ขีดคั่นกลาง '-'
  shiftOut(data, clock, LSBFIRST, seperator); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc6); // หลักที่ 4: นาทีหลักสิบ
  shiftOut(data, clock, LSBFIRST, digits[minute_high]); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc8); // หลักที่ 5: นาทีหลักหน่วย
  shiftOut(data, clock, LSBFIRST, digits[minute_low]); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xca); // หลักที่ 6: ขีดคั่นกลาง '-'
  shiftOut(data, clock, LSBFIRST, seperator); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xcc); // หลักที่ 7: วินาทีหลักสิบ
  shiftOut(data, clock, LSBFIRST, digits[second_high]); 
  digitalWrite(strobe, HIGH);
  
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xce); // หลักที่ 8: วินาทีหลักหน่วย
  shiftOut(data, clock, LSBFIRST, digits[second_low]); 
  digitalWrite(strobe, HIGH);
}
