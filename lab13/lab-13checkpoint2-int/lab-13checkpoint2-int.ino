#include <avr/io.h>           // เรียกใช้คลังโปรแกรม io.h
#include <avr/interrupt.h>    // เรียกใช้คลังโปรแกรมสำหรับขัดจังหวะ
#include "my_twi.h"           // เรียกใช้คลังโปรแกรมจัดการมอดูล TWI
#include "my_pcf8574.h"       // เรียกใช้คลังโปรแกรมจัดการไอซี PCF8574

// --- ตัวแปร Global ---
uint8_t count;                // ตัวแปรสำหรับเก็บสถานะนับ/เลื่อนบิต
uint8_t timer = 0;            // ตัวแปรนับเวลาใน Mode 1
unsigned long last_time = 0;  // เวลาสำหรับ debounce ปุ่มกด
bool pressed = 0;             // สถานะการกดปุ่มสั่งงาน (ถ้ามี)

uint8_t mode = 1;            // ตัวแปรโหมด
bool pcf_int_flag = false;   // Flag สัญญาณขัดจังหวะจากขา INT ของ PCF8574

uint8_t prepare_data(uint8_t d) // ฟังชังก์กลับค่า logic ของ LED CA
{
  uint8_t tmp;
  tmp = ~d;
  tmp |= 0x80;
  return tmp;
}

// ISR รับสัญญาณขัดจังหวะ
// เมื่อสถานะขา Input บน PCF8574 เปลี่ยนแปลงจะขัดจังหวะ CPU
ISR(INT0_vect)
{
  pcf_int_flag = true; // set flag แล้วค่อยจัดการใน loop
}

void setup()
{
  last_time = 0;
  Serial.begin(38400);

  // ตั้งค่าขา D2 (INT0 / PD2) เป็น Input
  // (ขา INT ของ PCF8574 เป็น Open-Drain Output จึงต้องเปิด Pull-up บน Arduino)
  DDRD &= ~(1 << DDD2);        // กำหนดให้ขา PD2 เป็น Input
  EICRA |= (1 << ISC01);       // ตั้งค่า External Interrupt 0 (INT0)
  EICRA &= ~(1 << ISC00);      // ให้ทำงานเมื่อเกิดสัญญาณขอบขาลง (Falling Edge)
  EIMSK |= (1 << INT0);        // เปิดใช้งาน Interrupt INT0
  sei();                       // เปิดการขัดจังหวะระบบ Global Interrupt

  init_twi_module();           // เริ่มต้นการทำงานของมอดูล TWI/I2C
  count = 0;
  PCF8574_write(0, prepare_data(count));
}

void loop()
{
  uint8_t tmp;
  if (pcf_int_flag)             // เมื่อพบการขัดจังหวะ
  {
    pcf_int_flag = false;       // ล้าง Flag Interrupt
    tmp = PCF8574_read(0);      // อ่านข้อมูลจาก PCF8574
    if (!(tmp & 0x80))           // ตรวจสอบว่าบิต P7 (ปุ่มเปลี่ยนโหมด) ถูกกดหรือไม่
    {
      if (millis() - last_time > 200) // Debounce 200ms`
      {
        last_time = millis();
        mode++;                 // เปลี่ยนโหมด
        if (mode > 4) mode = 1; // วนโหมด 1->2->3->4->1
        if (mode == 1) {
          count = 0;
          timer = 0;
          PCF8574_write(0, prepare_data(timer));
        } else if (mode == 2) {
          count = 0x40;            // เริ่มจากขวา P6 (0x40)
          PCF8574_write(0, prepare_data(count));
        } else if (mode == 3) {
          count = 0x01;            // เริ่มจากซ้าย P0 (0x01)
          PCF8574_write(0, prepare_data(count));
        } else if (mode == 4) {
          count = 0x00;
          PCF8574_write(0, prepare_data(count));
        }
        Serial.print("Mode Change via PCF8574 INT to ");
        Serial.println(mode);
      }
    }
  }

  // ควบคุมการแสดงผล
  if (mode == 1) {                  // M1 โหมดนับขึ้น 7 bit
    delay(100);
    count++;
    if (count >= 10) {
      timer++;
      if (timer > 0x7F) timer = 0;  // นับ 7 บิต (0-127 / 0x00-0x7F) 
                                    // สำหรับ LED P0-P6
      count = 0;
      PCF8574_write(0, prepare_data(timer));
    }
  }
  else if (mode == 2) {             // M2 โหมดเลื่อนไฟไปทางขวา
    delay(100);
    count = count >> 1;             // right shift
    if (count == 0)
      count = 0x40;                 // วนกลับไปที่ P6 (0x40)
    PCF8574_write(0, prepare_data(count));
  }
  else if (mode == 3) {             // M3 โหมดเลื่อนไฟไปทางซ้าย
    delay(100);
    count = count << 1;             // left shift
    if (count > 0x40 || count == 0) // ขอบเขตสูงสุดอยู่ที่ P6 (0x40)
      count = 0x01;
    PCF8574_write(0, prepare_data(count));
  }
  else if (mode == 4) {             // M4 ติดดับ สลับกัน
    delay(200);
    count = ~count & 0x7F;          // สลับสถานะ LED 7 ดวง (P0-P6)
    PCF8574_write(0, prepare_data(count));
  }
}
