#include "my_EEPROM.h"

void scan_EEPROM(void)
{
  int i, data_location, number_data_found;
  uint8_t d;
  number_data_found = 0;
  Serial.println("Performing EEPROM scanning...");
  for(i=0;i<1024;i++)
  {
    d = EEPROM_read1byte(i);
    if (d != 0xFF)
    {
      data_location = i;
      number_data_found++;
      if (d < 0x10) Serial.print("0");
      Serial.print(d, HEX);
      Serial.print(" is found in EEPROM location[");
      Serial.print(i);
      Serial.println("]");
//      EEPROM_Erase_only(i);     
    }
    
  }
  if (number_data_found > 1)
    Serial.println("More than one data are found in EEPROM");
  else if(number_data_found == 0)
    Serial.println("no data found in EEPROM");
}

void setup() 
{
  Serial.begin(38400);
  display_all_data_in_EEPROM();
    Serial.println("Begin Scan...");

  scan_EEPROM();
    Serial.println("Finish Scan");

}

void loop() {
  // put your main code here, to run repeatedly:

}
