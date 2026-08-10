#include <avr/interrupt.h>
#include <avr/io.h>
// กำหนดพินสำหรับเชื่อมต่อโมดูล TM1638
const int strobe = 7;
const int clock = 9;
const int data = 8;
// ค่าโหลดสำหรับนับเวลาให้ได้ 100ms ด้วย Prescaler 1024 (บอร์ด 16MHz)
const uint16_t HUNDRED_MILLI_SEC = 63973;
uint8_t timer = 0;
uint8_t second = 0;
uint8_t minute = 0;
uint8_t hour = 0;
// ตัวแปรสำหรับตรวจจับขอบขาขึ้น (Edge Detection) ป้องกันการกดค้างแล้วค่าวิ่งรัว
uint8_t lastButtons = 0;
void setup() {
  timer = 0;
  // ตั้งค่า Timer 1
  TIMSK1 |= 0x01; // Enable Timer1 Overflow Interrupt
  TCCR1A = 0x00;  // Normal Mode
  TCCR1B = 0x05;  // CS1[2:0] = 101 -> เลือก Prescaler /1024 
  TCCR1C = 0x00;
  TCNT1 = HUNDRED_MILLI_SEC;
  pinMode(strobe, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);
  sendCommand(0x8f);  
  reset();           
  sei();              
}

void loop() {
  uint8_t buttons = readTM1638_Buttons();\
  uint8_t pressed = buttons & ~lastButtons;
  lastButtons = buttons;
  if (pressed) {
    cli(); 
    // แยกหลักสิบและหลักหน่วยเพื่อความแม่นยำในการคำนวณ
    uint8_t h_tens = hour / 10;
    uint8_t h_units = hour % 10;
    uint8_t m_tens = minute / 10;
    uint8_t m_units = minute % 10;
    // S1 เพิ่มค่าชั่วโมงหลักสิบ (0 -> 1 -> 2 -> 0) 
    if (pressed & 0x01) {
      h_tens = (h_tens + 1) % 3; 
      
      if (h_tens == 2 && h_units > 3) {
        h_units = 0; 
      }
      hour = (h_tens * 10) + h_units;
    } 
    // S2 เพิ่มค่าชั่วโมงหลักหน่วย (0-9 หรือ 0-3 ขึ้นอยู่กับหลักสิบ) 
    else if (pressed & 0x02) {
      if (h_tens == 2) {
        h_units = (h_units + 1) % 4; 
      } else {
        h_units = (h_units + 1) % 10; 
      }
      hour = (h_tens * 10) + h_units;
    } 
    // S4 เพิ่มค่านาทีหลักสิบ (0 -> 1 -> ... -> 5 -> 0) 
    else if (pressed & 0x08) {
      m_tens = (m_tens + 1) % 6; 
      minute = (m_tens * 10) + m_units;
    } 
    // S5 เพิ่มค่านาทีหลักหน่วย (0 -> 1 -> ... -> 9 -> 0) 
    else if (pressed & 0x10) {
      m_units = (m_units + 1) % 10; 
      minute = (m_tens * 10) + m_units;
    } 
    // S7 ตั้งให้ค่าวินาที = 30
    else if (pressed & 0x40) {
      second = 30;
      timer = 0; 
    } 
    // S8 ตั้งให้วินาที = 0
    else if (pressed & 0x80) {
      second = 0;
      timer = 0; 
    } 
    sei(); 
  }
  display_clock();    
}

// ฟังก์ชันขัดจังหวะเพื่อการนับเวลา (ทุก ๆ 100ms)
ISR(TIMER1_OVF_vect) {
  TCNT1 = HUNDRED_MILLI_SEC;
  timer++;
  if (timer >= 10) {
    timer = 0;
    second++;
    if (second >= 60) {
      second = 0;
      minute++;
      if (minute >= 60) {
        minute = 0; 
        hour++;
        if (hour >= 24) {
          hour = 0; 
        }
      }
    }
  }
}








void sendCommand(uint8_t value)
{
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}
  
void reset()
{
  sendCommand(0x40); 
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc0);   
  for(uint8_t i = 0; i < 16; i++)
  {
    shiftOut(data, clock, LSBFIRST, 0x00);
  }
  digitalWrite(strobe, HIGH);
}

uint8_t readTM1638_Buttons(void)
{
  uint8_t buttons = 0;
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0x42);

  pinMode(data, INPUT);

  for (uint8_t i = 0; i < 4; i++)
  {
    uint8_t v = shiftIn(data, clock, LSBFIRST) << i;
    buttons |= v;
  }

  pinMode(data, OUTPUT);
  digitalWrite(strobe, HIGH);
  return buttons;
}

void setTM1638LED(uint8_t value, uint8_t position)
{
  pinMode(data, OUTPUT);
  sendCommand(0x44); 
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xC1 + (position << 1));
  shiftOut(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}

void display_clock()
{
  static uint8_t digits[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };

  sendCommand(0x44); 
  uint8_t seperator = 0x40; // รหัส 7-segment สำหรับตัวขีดกลาง '-'
  
  uint8_t hour_low = hour % 10;
  uint8_t hour_high = hour / 10;

  uint8_t minute_low = minute % 10;
  uint8_t minute_high = minute / 10;

  uint8_t second_low = second % 10;
  uint8_t second_high = second / 10;

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc0); 
  shiftOut(data, clock, LSBFIRST, digits[hour_high]); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc2); 
  shiftOut(data, clock, LSBFIRST, digits[hour_low]); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc4); 
  shiftOut(data, clock, LSBFIRST, seperator); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc6); 
  shiftOut(data, clock, LSBFIRST, digits[minute_high]); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc8); 
  shiftOut(data, clock, LSBFIRST, digits[minute_low]); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xca); 
  shiftOut(data, clock, LSBFIRST, seperator); 
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xcc); 
  shiftOut(data, clock, LSBFIRST, digits[second_high]); 
  digitalWrite(strobe, HIGH);
  
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xce); 
  shiftOut(data, clock, LSBFIRST, digits[second_low]); 
  digitalWrite(strobe, HIGH);
}
