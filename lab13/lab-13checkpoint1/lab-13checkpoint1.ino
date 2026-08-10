#include <avr/io.h>           //เรียกใช้คลังโปรแกรม io.h
#include <avr/interrupt.h>    //เรียกใช้คลังโปรแกรมสำหรับขัดจังหวะ
#include "my_twi.h"           //เรียกใช้คลังโปรแกรมจัดการมอดูลทีดับเบิลยูไอ
#include "my_pcf8574.h"       //เรียกใช้คลังโปรแกรมจัดการไอซี PCF8574
                              //---ส่วนบิตอื่นที่เหลือเป็นเอาต์พุต
uint8_t count;                //ตั้งค่าตัวแปรส่วนกลาง (global variable)
unsigned long last_time;

uint8_t prepare_data(uint8_t d)
{
  uint8_t tmp;
  tmp = ~d;                   //กลับทุกบิตเป็นตรงข้ามเนื่องจาก LED เป็น Common A
  tmp |= 0x80;                //เขียนค่ารหัสขับแอลอีดีโดยมิให้มีผล                               
  return tmp;                 //--กระทบต่อบิต MSB ที่ใช้เป็นอินพุต 
}

void setup()
{
  last_time =0;  
  Serial.begin(38400);
  uint8_t tmp, i;             //ประกาศตัวแปร tmp และตัวแปร i ขนาด 8 บิต
  init_twi_module();          //สั่งให้มอดูลทีดับเบิลยูไอเริ่มทำงาน
  count = 0;                  //สั่งให้ตัวแปร count มีค่าเริ่มต้นเท่ากับศูนย์  
  tmp = prepare_data(count);  
  PCF8574_write(0, tmp);      
}                             //จบการทำงานของฟังก์ชันหลัก

void loop(){
  
  uint8_t tmp;

  tmp = PCF8574_read(0);
  tmp &= 0x80;
  if(!tmp)
  {
    if( millis() - last_time > 20) 
    {
      last_time = millis(); 
      tmp = PCF8574_read(0);
      tmp &= 0x80;
      if (!tmp)
      {
        
        count++;
        // reset count
        if (count > 127) {
          count = 0;
        }
        Serial.println("Keypressed detected");
        Serial.print("Count: ");
        Serial.println(count);
      }
      do
      {
        tmp = PCF8574_read(0);  
        tmp &= 0x80;
      }
      while(!tmp);    
    }
  }

  // display
  PCF8574_write(0, prepare_data(count));      
}
