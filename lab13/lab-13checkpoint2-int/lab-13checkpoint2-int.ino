#include <avr/io.h>           // เรียกใช้คลังโปรแกรม io.h
#include <avr/interrupt.h>    // เรียกใช้คลังโปรแกรมสำหรับขัดจังหวะ
#include "my_twi.h"           // เรียกใช้คลังโปรแกรมจัดการมอดูล TWI
#include "my_pcf8574.h"       // เรียกใช้คลังโปรแกรมจัดการไอซี PCF8574

// --- ตัวแปร Global ---
uint8_t count;                // ตัวแปรสำหรับเก็บสถานะนับ/เลื่อนบิต
uint8_t timer = 0;            // ตัวแปรนับเวลาใน Mode 1
unsigned long last_time = 0;  // เวลาสำหรับ debounce ปุ่มกด
bool pressed = 0;             // สถานะการกดปุ่มสั่งงาน (ถ้ามี)

volatile uint8_t mode = 1;            // ตัวแปรโหมด
volatile bool pcf_int_flag = false;   // Flag สัญญาณขัดจังหวะจากขา INT ของ PCF8574

// ฟังก์ชันเตรียมข้อมูลส่งให้ PCF8574 
// (P0-P6 เป็น LED Common Anode แบบ Active LOW, P7 เป็น Input ปุ่มกด)
uint8_t prepare_data(uint8_t d)
{
  uint8_t tmp;
  tmp = ~d;                   // กลับบิตเป็นตรงข้ามเนื่องจาก LED เป็น Common Anode
  tmp |= 0x80;                // เขียน 1 ให้บิต P7 (0x80) เป็น Input ส่วน P0-P6 (0x7F) เป็น Output ขับ LED 7 ดวง
  return tmp;                 
}

// --- ISR รับสัญญาณขัดจังหวะจากขา INT (Pin 13) ของ PCF8574 ต่อเข้ากับขา D2 (INT0) ของ Arduino ---
ISR(INT0_vect)
{
  // เมื่อสถานะขา Input บน PCF8574 เปลี่ยนแปลง ขา INT ของ PCF8574 จะส่งสัญญาณ LOW มาขัดจังหวะ Arduino
  pcf_int_flag = true;
}

void setup()
{
  last_time = 0;  
  Serial.begin(38400);

  // --- ตั้งค่าขา D2 (INT0 / PD2) เป็น Input แบบ Pull-up สำหรับรับขาสัญญาณ INT จาก PCF8574 ---
  // (ขา INT ของ PCF8574 เป็น Open-Drain Output จึงต้องเปิด Pull-up บน Arduino)
  DDRD &= ~(1 << DDD2);        // กำหนดให้ขา PD2 เป็น Input
  PORTD |= (1 << PORTD2);      // เปิดใช้งาน Internal Pull-up resistor บนขา PD2
  
  // --- ตั้งค่า External Interrupt 0 (INT0) ให้ทำงานเมื่อเกิดสัญญาณขอบขาลง (Falling Edge) ---
  EICRA |= (1 << ISC01);
  EICRA &= ~(1 << ISC00);
  EIMSK |= (1 << INT0);        // เปิดใช้งาน Interrupt INT0
  sei();                       // เปิดการขัดจังหวะระบบ Global Interrupt

  init_twi_module();           // เริ่มต้นการทำงานของมอดูล TWI/I2C
  count = 0;                  
  PCF8574_write(0, prepare_data(count));      
}

void loop()
{  
  uint8_t tmp;

  // --- เมื่อขา INT ของ PCF8574 ส่งสัญญาณขัดจังหวะเข้ามา ---
  if (pcf_int_flag)
  {
    pcf_int_flag = false;      // ล้าง Flag Interrupt

    // อ่านค่าข้อมูลจาก PCF8574 เพื่อตรวจสอบว่าบิตใดเปลี่ยนแปลง
    // (หมายเหตุ: การอ่านค่า PCF8574_read() จะทำหน้าที่เคลียร์ขา INT ของ PCF8574 กลับเป็น HIGH โดยอัตโนมัติ)
    tmp = PCF8574_read(0);

    // ตรวจสอบว่าบิต P7 (ปุ่มเปลี่ยนโหมด) ถูกกดหรือไม่ (Active LOW -> บิต P7 เป็น 0)
    if (!(tmp & 0x80))
    {
      if (millis() - last_time > 200) // Debounce 200ms
      {
        last_time = millis();
        
        mode++;
        if (mode > 4) mode = 1;

        if (mode == 1) {
          count = 0;
          timer = 0;
          PCF8574_write(0, prepare_data(timer));
        } else if (mode == 2) {
          count = 0x01;            // เริ่มจาก P0 (0x01)
          PCF8574_write(0, prepare_data(count));
        } else if (mode == 3) {
          count = 0x40;            // เริ่มจาก P6 (0x40)
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

  // --- การทำงานตามแต่ละ Mode (รองรับ LED 7 ดวง: P0-P6) ---
  if (mode == 1) {
    delay(100);
    count++;
    if (count >= 10) {
      timer++; 
      if (timer > 0x7F) timer = 0; // นับ 7 บิต (0-127 / 0x00-0x7F) สำหรับ LED P0-P6
      count = 0;
      PCF8574_write(0, prepare_data(timer));      
    }
  }
  else if (mode == 2) {
    // ใน Mode 2-4: หากมีการกดปุ่ม จะสลับโหมดผ่าน PCF8574 INT อัตโนมัติ หรือประมวลผลตามโหมด
    delay(100);
    count = count << 1;
    if (count > 0x40 || count == 0) // ขอบเขตสูงสุดอยู่ที่ P6 (0x40)
      count = 0x01;
    PCF8574_write(0, prepare_data(count));      
  }
  else if (mode == 3) {
    delay(100);
    count = count >> 1;
    if (count == 0) 
      count = 0x40;                 // วนกลับไปที่ P6 (0x40)
    PCF8574_write(0, prepare_data(count));     
  }
  else if (mode == 4) {
    delay(200);
    count = ~count & 0x7F;           // สลับสถานะ LED 7 ดวง (P0-P6)
    PCF8574_write(0, prepare_data(count));     
  }
}
