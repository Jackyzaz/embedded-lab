// -------------------------------------------------------------
// Blynk Credentials & Template Info
// -------------------------------------------------------------
#define BLYNK_TEMPLATE_ID "TMPL6n4U-bFxw"
#define BLYNK_TEMPLATE_NAME "Checkpoint4"
#define BLYNK_AUTH_TOKEN "qPyisR4Cq87sfRpSp3Zuiwd8Qq0YIzG2"
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
char ssid[] = "Jack>/~";
char pass[] = "nmcu1234";

// -------------------------------------------------------------
// Hardware Configuration
// -------------------------------------------------------------
// I2C Pins สำหรับ ESP8266 (NodeMCU)
// ใช้ D2 (GPIO4) เป็น SDA และ D1 (GPIO5) เป็น SCL
#define SDA_PIN 4 
#define SCL_PIN 5 

// Serial EEPROM Config (AT24C32 บนโมดูล DS1307)
#define EEPROM_ADDR 0x50       // I2C Address ของ AT24C32 ปกติจะเป็น 0x50
#define EEPROM_START_ADDR 600 // ตำแหน่งแอดเดรสเริ่มต้นในการอ่านและเขียน

// LCD Config
// Address อาจจะเป็น 0x27 หรือ 0x3F ขึ้นอยู่กับโมดูล I2C ที่ใช้
LiquidCrystal_I2C lcd(0x25, 16, 2); 

BlynkTimer timer;
String currentMessage = "";

// -------------------------------------------------------------
// EEPROM Functions
// -------------------------------------------------------------
// ฟังก์ชันเขียนข้อมูล 1 ไบต์ ลง Serial EEPROM
void writeEEPROM(unsigned int eeaddress, byte data) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();
  delay(20); // สำคัญ: รอ 20ms ต่อ 1 ตัวอักษร เพื่อให้ชิปมีเวลาเซฟ
}

// ฟังก์ชันอ่านข้อความความยาว 8 ตัวอักษร จาก EEPROM (แบบ Sequential Read)
String readMessageFromEEPROM() {
  String msg = "";
  
  // 1. ตั้งค่า Address Pointer ไปที่ 0x00
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((int)(EEPROM_START_ADDR >> 8));   // MSB
  Wire.write((int)(EEPROM_START_ADDR & 0xFF)); // LSB
  Wire.endTransmission();
  
  delay(10); // รอให้บัสว่าง

  // 2. ขออ่านรวดเดียว 8 ไบต์
  Wire.requestFrom((uint8_t)EEPROM_ADDR, (uint8_t)8);
  
  Serial.print("Raw Read (Seq): ");
  for (int i = 0; i < 8; i++) {
    if (Wire.available()) {
      byte b = Wire.read();
      Serial.print(b, HEX);
      Serial.print(" ");
      
      // แปลงเป็นตัวอักษรถ้าไม่ใช่ค่าว่าง
      if (b != 0xFF && b != 0x00) {
        msg += (char)b;
      }
    } else {
      Serial.print("ERR "); // ถ้าอ่านไม่ได้
    }
  }
  Serial.println();
  
  return msg;
}

// ฟังก์ชันเขียนข้อความลง EEPROM (บันทึกทีละไบต์)
void writeMessageToEEPROM(String msg) {
  Serial.print("Raw Write: ");
  for (int i = 0; i < 8; i++) {
    byte b;
    if (i < msg.length()) {
      b = msg[i];
    } else {
      b = '\0'; // เติม NULL ถ้าความยาวไม่ถึง 8
    }
    Serial.print(b, HEX);
    Serial.print(" ");
    
    writeEEPROM(EEPROM_START_ADDR + i, b);
    yield(); // ป้องกัน Watchdog Timer รีเซ็ตบอร์ดระหว่างหน่วงเวลาเขียน
  }
  Serial.println();
}

// -------------------------------------------------------------
// LCD Display Function
// -------------------------------------------------------------
// ฟังก์ชันแสดงผลข้อความลงจอ LCD
void updateLCD(String msg) {
  Serial.println("LCD Log: Updating display with message: [" + msg + "]");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("EEPROM Msg:");
  lcd.setCursor(0, 1);
  lcd.print(msg);
}

// -------------------------------------------------------------
// Blynk Event: รอรับค่าจากแอป
// -------------------------------------------------------------
// ใช้ Text Input Widget หรือ Terminal กำหนดเป็น Virtual Pin V7
BLYNK_WRITE(V7) {
  String newMsg = param.asStr();
  
  // ตัดข้อความให้เหลือแค่ 8 ตัวอักษรตาม Requirement
  if (newMsg.length() > 8) {
    newMsg = newMsg.substring(0, 8);
  }
  
  Serial.println("Received from Blynk: " + newMsg);
  
  // บันทึกข้อความใหม่ลง EEPROM
  writeMessageToEEPROM(newMsg);
  
  // อัปเดตตัวแปรปัจจุบันและแสดงผลบนจอ LCD
  currentMessage = newMsg;
  updateLCD(currentMessage);
}

// ฟังก์ชันสแกนหา I2C Slave ทั้งหมด (1 - 127)
void scanI2C() {
  Serial.println("\n--- Scanning all I2C Addresses (1 - 127) ---");
  int devicesFound = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("-> Found I2C Device at address: 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devicesFound++;
    }
  }
  if (devicesFound == 0) {
    Serial.println("-> NO I2C DEVICES FOUND!");
  }
  Serial.println("--------------------------------------------\n");
}

void setup() {
  Serial.begin(115200);
  delay(500); // รอให้ Serial Monitor เปิดทัน
  Serial.println("\n\n--- Checkpoint 4 System Starting ---");

  // กำหนดค่าเริ่มต้น I2C (SDA, SCL)
  Serial.println("Step 1: Initializing I2C (SDA=" + String(SDA_PIN) + ", SCL=" + String(SCL_PIN) + ")");
  Wire.begin(SDA_PIN, SCL_PIN);
  
  scanI2C(); // สแกนหา I2C Address ของจอ LCD
  
  // ตั้งค่าจอ LCD
  Serial.println("Step 2: Initializing LCD...");
  lcd.init();      
  lcd.backlight();
  
  Serial.println("Step 3: Printing initial text to LCD...");
  lcd.setCursor(0, 0);
  lcd.print("Init EEPROM...");
  delay(1000);

  // อ่านข้อความเดิมที่ถูกเก็บไว้ใน EEPROM
  Serial.println("Step 4: Reading from EEPROM...");
  currentMessage = readMessageFromEEPROM();
  Serial.println("-> Read result: [" + currentMessage + "]");
  
  // แสดงผลบนจอ LCD 
  updateLCD(currentMessage);

  // เริ่มการเชื่อมต่อ WiFi และ Blynk
  Serial.println("Step 5: Connecting to WiFi & Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("-> Blynk connected successfully!");
}

void loop() {
  Blynk.run();
  timer.run();
}
