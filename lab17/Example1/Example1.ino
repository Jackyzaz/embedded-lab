#include "my_EEPROM.h"

uint16_t scan_EEPROM(void)
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
      Serial.print(d);
      Serial.print(" is found in EEPROM location[");
      Serial.print(i);
      Serial.println("]");     
    }
    
  }
  if (number_data_found > 1)
    Serial.println("More than one data are found in EEPROM");
  else if(number_data_found ==0)
    Serial.println("no data found in EEPROM");
}

void setup() 
{
  Serial.begin(38400);
  display_all_data_in_EEPROM();
  scan_EEPROM();
}

void loop() {
  // put your main code here, to run repeatedly:

}
