#include <Wire.h>

#define SDA_PIN 4 
#define SCL_PIN 5 
#define EEPROM_ADDR 0x50

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=================================");
  Serial.println("      EEPROM FULL DEBUG SCRIPT    ");
  Serial.println("=================================");

  Wire.begin(SDA_PIN, SCL_PIN);

  // 1. สแกนหาอุปกรณ์ I2C
  Serial.println("\n[1] Scanning I2C Bus...");
  int devices = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("-> Found device at 0x");
      Serial.println(address, HEX);
      devices++;
    }
  }
  if (devices == 0) Serial.println("-> NO I2C DEVICES FOUND!");

  // 2. ทดสอบแบบ 16-bit Addressing (สำหรับชิป AT24C32 หรือ AT24C64)
  Serial.println("\n[2] Testing 16-bit Addressing (AT24C32)");
  String testStr16 = "hello16";
  Serial.print("-> Writing: "); Serial.println(testStr16);
  for(int i=0; i<8; i++) {
    byte b = (i < testStr16.length()) ? testStr16[i] : 0;
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write(0); // MSB
    Wire.write(i); // LSB
    Wire.write(b);
    byte err = Wire.endTransmission();
    Serial.print("   Wrote '"); Serial.print((char)b); 
    Serial.print("' ("); Serial.print(b, HEX); 
    Serial.print(") to addr 0x000"); Serial.print(i); 
    Serial.print(" | ErrorCode="); Serial.println(err);
    delay(20);
  }

  Serial.print("-> Reading back 16-bit (HEX): ");
  for(int i=0; i<8; i++) {
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write(0); // MSB
    Wire.write(i); // LSB
    Wire.endTransmission();
    Wire.requestFrom(EEPROM_ADDR, 1);
    byte b = Wire.available() ? Wire.read() : 0xFF;
    if (b < 16) Serial.print("0");
    Serial.print(b, HEX); Serial.print(" ");
  }
  Serial.println();


  // 3. ทดสอบแบบ 8-bit Addressing (สำหรับชิปตัวเล็ก เช่น AT24C02, AT24C04)
  Serial.println("\n[3] Testing 8-bit Addressing (AT24C02)");
  String testStr8 = "world8!";
  Serial.print("-> Writing: "); Serial.println(testStr8);
  for(int i=0; i<8; i++) {
    byte b = (i < testStr8.length()) ? testStr8[i] : 0;
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write(i); // 8-bit Address
    Wire.write(b);
    byte err = Wire.endTransmission();
    Serial.print("   Wrote '"); Serial.print((char)b); 
    Serial.print("' ("); Serial.print(b, HEX); 
    Serial.print(") to addr 0x0"); Serial.print(i); 
    Serial.print(" | ErrorCode="); Serial.println(err);
    delay(20);
  }

  Serial.print("-> Reading back 8-bit (HEX):  ");
  for(int i=0; i<8; i++) {
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write(i); // 8-bit Address
    Wire.endTransmission();
    Wire.requestFrom(EEPROM_ADDR, 1);
    byte b = Wire.available() ? Wire.read() : 0xFF;
    if (b < 16) Serial.print("0");
    Serial.print(b, HEX); Serial.print(" ");
  }
  Serial.println("\n\n=================================");
  Serial.println("          DEBUG FINISHED         ");
  Serial.println("=================================");
}

void loop() {
}
