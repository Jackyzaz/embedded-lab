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
      Serial.print("Mode ");
      Serial.print(d);
      Serial.print(" is found in EEPROM location[");
      Serial.print(i);
      Serial.println("]");     
    }
    
  }
  if (number_data_found > 1)
    Serial.println("Error!, found more than one data in EEPROM");
  else if(number_data_found ==0)
    Serial.println("no data found in EEPROM");
}

//คำอธิบาย ฟังก์ชันนี้รับค่าพารามิเตอร์ 2 ค่า คือ ตำแหน่งหน่วยความจำที่ต้องการเขียนและค่าที่ต้องการเขียน
void EEPROM_Erase_and_Write1B(uint16_t addr, uint8_t data)
{
  while(EECR & (1<<EEPE)) ; //วนซ้ำจนกว่าบิต EEPE จะเปลี่ยนเป็นตรรกะต่ำ
  EECR = 0x00;      //mode erase and write
  EEAR = addr;      //คัดลอกพารามิเตอร์รับเข้า addr สู่เรจิสเตอร์ EEAR
  EEDR = data;      //คัดลอกพารามิเตอร์รับเข้า data สู่เรจิสเตอร์ EEDR
  char backupSREG;    //ตัวแปรสำหรับเก็บค่าสถานะของเรจิสเตอร์ตัวบ่งชี้
  backupSREG = SREG;    //คัดลอกค่าในเรจิสเตอร์ตัวบ่งชี้ใส่ในตัวแปรที่ตั้งไว้
  cli();        //ปิดทางการขัดจังหวะส่วนกลางของตัวประมวลผล
  EECR |= (1<<EEMPE);   //สั่งให้บิต EEMPE ใน EECR เป็นตรรกะสูง
  EECR |= (1<<EEPE);    //สั่งให้บิต EEPE ใน EECR เป็นตรรกะสูง
  SREG = backupSREG;    //คืนค่าเรจิสเตอร์ตัวบ่งชี้กลับสู่สถานะเดิม
}



void EEPROM_Erase_only(uint16_t addr)
{  
  while(EECR & (1<<EEPE)) ; //วนซ้ำจนกว่าบิต EEPE จะเปลี่ยนเป็นตรรกะต่ำ
  EECR = 0b01 << EEPM0;   //ลบอย่างเดียว 
  EEAR = addr;      //คัดลอกพารามิเตอร์รับเข้า addr สู่เรจิสเตอร์ EEAR
  EEDR = 0xFF;      //คัดลอกพารามิเตอร์รับเข้า data สู่เรจิสเตอร์ EEDR
  char backupSREG;    //ตัวแปรสำหรับเก็บค่าสถานะของเรจิสเตอร์ตัวบ่งชี้
  backupSREG = SREG;    //คัดลอกค่าในเรจิสเตอร์ตัวบ่งชี้ใส่ในตัวแปรที่ตั้งไว้
  cli();        //ปิดทางการขัดจังหวะส่วนกลางของตัวประมวลผล
  EECR |= (1<<EEMPE);   //สั่งให้บิต EEMPE ใน EECR เป็นตรรกะสูง
  EECR |= (1<<EEPE);    //สั่งให้บิต EEPE ใน EECR เป็นตรรกะสูง
  SREG = backupSREG;    //คืนค่าเรจิสเตอร์ตัวบ่งชี้กลับสู่สถานะเดิม
}


void EEPROM_Write_to_Empty_location(uint16_t addr, uint8_t data)
{
  while(EECR & (1<<EEPE)) ; //วนซ้ำจนกว่าบิต EEPE จะเปลี่ยนเป็นตรรกะต่ำ
  EECR = 0b10 << EEPM0;    //เขียนบนตำแหน่งที่ลบเสร็จแล้วก่อนหน้านี้
  EEAR = addr;      //คัดลอกพารามิเตอร์รับเข้า addr สู่เรจิสเตอร์ EEAR
  EEDR = data;      //คัดลอกพารามิเตอร์รับเข้า data สู่เรจิสเตอร์ EEDR
  char backupSREG;    //ตัวแปรสำหรับเก็บค่าสถานะของเรจิสเตอร์ตัวบ่งชี้
  backupSREG = SREG;    //คัดลอกค่าในเรจิสเตอร์ตัวบ่งชี้ใส่ในตัวแปรที่ตั้งไว้
  cli();        //ปิดทางการขัดจังหวะส่วนกลางของตัวประมวลผล
  EECR |= (1<<EEMPE);   //สั่งให้บิต EEMPE ใน EECR เป็นตรรกะสูง
  EECR |= (1<<EEPE);    //สั่งให้บิต EEPE ใน EECR เป็นตรรกะสูง
  SREG = backupSREG;    //คืนค่าเรจิสเตอร์ตัวบ่งชี้กลับสู่สถานะเดิม
}




unsigned char EEPROM_read1byte(uint16_t addr)  //ชื่อฟังก์ชันและพารามิเตอร์รับเข้า
{       
  while (EECR & (1<<EEPE)) ;    //วนซ้ำจนกว่าบิต EEPE จะเปลี่ยนเป็นตรรกะต่ำ
  EEAR = addr;        //คัดลอกพารามิเตอร์รับเข้าสู่เรจิสเตอร์ EEAR
  EECR |= (1<<EERE);      //สั่งให้บิต EERE เป็นตรรกะสูงเพื่อเริ่มการอ่าน
  return EEDR;        //นำค่าที่ปรากฏใน EEDR ไปใช้งาน
}



unsigned long measure_time_read_all_data_in_EEPROM()
{
  unsigned long start_t, stop_t, duration, sum;
  uint8_t c;
  start_t = millis();
  for (int i=0;i<1024;i++)
  {
    c = EEPROM_read1byte(i);
    sum +=c;
  }
  stop_t = millis();
  
  duration= stop_t - start_t;
  Serial.print(stop_t);
  Serial.print("-");
  Serial.print(start_t);
  Serial.print("time (ms) for reading all the data in EEPROM=");
  Serial.println(duration);
  return sum;
}

void display_all_data_in_EEPROM()
{
  Serial.println("Data in the EEPROM");
  int i;
  uint8_t c;
  for (i=0;i<1024;i++)
  {
    c = EEPROM_read1byte(i);
    if( (i>0) && (i%16==0) )
        Serial.println("");
    Serial.print(c);
    Serial.print(" ");
  }
  Serial.println("\n\r--------------------------");  
}


int find_first_location_of_data(void)
{
  uint8_t d;
  int i;
  uint16_t loc;
  for (i=0;i<1024;i++)
  {
    d = EEPROM_read1byte(i);
    if(d != 0xFF)
    {
      return i;
    }
  }
  return -1;
}

void insert_data_to_EEPROM(void)
{
  int x;
  x = find_first_location_of_data();
  if(x == -1)
  {
    EEPROM_Write_to_Empty_location(1023, 00);
  }
  else if(x<1024)
  { 
    EEPROM_Erase_only(x);
    EEPROM_Write_to_Empty_location(x-1,00);
  }
}



void setup() 
{
  Serial.begin(38400);
  display_all_data_in_EEPROM();
  //measure_time_read_all_data_in_EEPROM();
  Serial.println("Begin Scan...");
  scan_EEPROM();
  Serial.println("Finish Scan");
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
