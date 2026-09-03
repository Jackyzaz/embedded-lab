#include "my_EEPROM.h"
int max_addr = -1;
int scan_EEPROM(void)
{
  int i, number_data_found = 0;
  uint8_t d;
  max_addr = -1; // รีเซ็ตก่อนสแกนทุกครั้ง
  Serial.println("Performing EEPROM scanning...");
  for(i=0; i<1024; i++)
  {
    d = EEPROM_read1byte(i);
    if (d != 0xFF)
    {
      max_addr = i;
      number_data_found++;
      if (d < 0x10) Seria l.print("0");
      Serial.print(d, HEX);
      Serial.print(" is found in EEPROM location[");
      Serial.print(i);
      Serial.println("]");     
    }
  }
  if (number_data_found > 1)
    Serial.println("More than one data are found in EEPROM");
  else if(number_data_found == 0)
    Serial.println("no data found in EEPROM");
  return number_data_found;
}

void setup() 
{
  Serial.begin(38400);
  display_all_data_in_EEPROM();
  Serial.println("Begin Scan...");

  int found = scan_EEPROM();
  Serial.println("Finish Scan");
  Serial.print("max_addr = ");
  Serial.println(max_addr);

   if (found == 0) {
    // ครั้งแรกที่ยังไม่มีข้อมูล เขียนที่ 1021-1023
    EEPROM_Write_to_Empty_location(1021, 0xFA);
    EEPROM_Write_to_Empty_location(1022, 0xCE);
    EEPROM_Write_to_Empty_location(1023, 0x0F);
  } else {
    // 1. ลบ 3 ไบต์เดิมทิ้งเสมอ
    EEPROM_Erase_only(max_addr - 2);
    EEPROM_Erase_only(max_addr - 1);
    EEPROM_Erase_only(max_addr);

    // 2. ถ้าถึง 3 2 1 หมดแล้วจะต้องวนไป 1023 1022 1021 ใหม่
    if (max_addr - 5 < 0) {
      EEPROM_Write_to_Empty_location(1021, 0xFA);
      EEPROM_Write_to_Empty_location(1022, 0xCE);
      EEPROM_Write_to_Empty_location(1023, 0x0F);
    } else {
      // ถ้ายังไม่หลุดขอบ ให้ถอยหลัง 3 ช่องตามปกติ
      EEPROM_Write_to_Empty_location(max_addr - 5, 0xFA);
      EEPROM_Write_to_Empty_location(max_addr - 4, 0xCE);
      EEPROM_Write_to_Empty_location(max_addr - 3, 0x0F);
    }
  }

}

void loop() {
  // empty loop
}
