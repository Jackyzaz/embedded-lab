#include <avr/io.h>           //เรียกใช้คลังโปรแกรม io.h
#include <avr/interrupt.h>    //เรียกใช้คลังโปรแกรมสำหรับขัดจังหวะ
#include "my_twi.h"           //เรียกใช้คลังโปรแกรมจัดการมอดูลทีดับเบิลยูไอ
#include "my_pcf8574.h"       //เรียกใช้คลังโปรแกรมจัดการไอซี PCF8574
                              //---ส่วนบิตอื่นที่เหลือเป็นเอาต์พุต

uint8_t count;                //ตั้งค่าตัวแปรส่วนกลาง (global variable)
uint8_t timer = 0;
unsigned long last_time = 0;      // เวลาสำหรับการ debounce ปุ่มสั่งงาน (P7)
unsigned long last_mode_time = 0; // เวลาสำหรับการ debounce ปุ่มเปลี่ยนโหมด (P6)
bool pressed = 0;

uint8_t mode = 1;             // start at mode 1


uint8_t prepare_data(uint8_t d)
{
  uint8_t tmp;
  tmp = ~d;                   //กลับทุกบิตเป็นตรงข้ามเนื่องจาก LED เป็น Common A
  tmp |= 0xC0;                //เขียนค่า 1 ให้บิต P7 (0x80) และ P6 (0x40) เพื่อใช้เป็นอินพุต 
  return tmp;                 
}

void setup()
{
  last_time = 0;  
  last_mode_time = 0;
  Serial.begin(38400);
  uint8_t tmp; 
  init_twi_module();          //สั่งให้มอดูลทีดับเบิลยูไอเริ่มทำงาน
  count = 0;                  //สั่งให้ตัวแปร count มีค่าเริ่มต้นเท่ากับศูนย์  
  tmp = prepare_data(count);  
  PCF8574_write(0, tmp);      
}                             //จบการทำงานของฟังก์ชันหลัก

void loop(){  
  uint8_t tmp;

  // --- ตรวจจับการกดปุ่ม Mode (ขาสัญญาณ P6 ของ PCF8574) ---
  tmp = PCF8574_read(0);
  tmp &= 0x40; // เช็คเฉพาะบิต P6
  if (!tmp)
  {
    if (millis() - last_mode_time > 20) 
    {
      last_mode_time = millis(); 
      tmp = PCF8574_read(0);
      tmp &= 0x40;
      if (!tmp)
      {
        mode++;
        if (mode > 4) mode = 1;
        
        if (mode == 1) {
          count = 0;
          timer = 0;
          PCF8574_write(0, prepare_data(timer));
        } else if (mode == 2) {
          count = 0x01;
          PCF8574_write(0, prepare_data(count));
        } else if (mode == 3) {
          count = 0x20;
          PCF8574_write(0, prepare_data(count));
        } else if (mode == 4) {
          count = 0x00;
          PCF8574_write(0, prepare_data(count));
        }

        Serial.print("Mode Change to ");
        Serial.println(mode);
      }
      do
      {
        tmp = PCF8574_read(0);  
        tmp &= 0x40;
      }
      while (!tmp);    
    }
  }

  // --- ตรวจจับการกดปุ่มสั่งงาน Keypressed (ขาสัญญาณ P7 ของ PCF8574) ---
  tmp = PCF8574_read(0);
  tmp &= 0x80; // เช็คเฉพาะบิต P7
  if(!tmp)
  {
    if( millis() - last_time > 20) 
    {
      last_time = millis(); 
      tmp = PCF8574_read(0);
      tmp &= 0x80;
      if (!tmp)
      {
        pressed = 1;
        Serial.println("Keypressed detected");
      }
      do
      {
        tmp = PCF8574_read(0);  
        tmp &= 0x80;
      }
      while(!tmp);    
    }
  }

  if (mode == 1) {
    delay(100);
    count++;
    if (count >= 10) {
      timer++; 
      if (timer > 0x3F) timer = 0; // นับ 6 บิต (0-63 / 0x00-0x3F) สำหรับ P0-P5
      count = 0;
      PCF8574_write(0, prepare_data(timer));      
    }
  }
  else if (mode == 2) {
    if (pressed) {
      count = count << 1;
      if (count > 0x20 || count == 0) 
        count = 0x01;
      pressed = 0;
      PCF8574_write(0, prepare_data(count));      
    }
  }
  else if (mode == 3) {
    if (pressed) {
      count = count >> 1;
      if (count == 0) 
        count = 0x20;
      pressed = 0;
      PCF8574_write(0, prepare_data(count));     
    }
  }
  else if (mode == 4) {
    if (pressed) {
      count = ~count & 0x3F;
      pressed = 0;
      PCF8574_write(0, prepare_data(count));     
    }
  }
}
