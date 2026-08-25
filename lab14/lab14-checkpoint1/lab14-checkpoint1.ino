// ตั้งค่า Library
#include <LiquidCrystal_I2C.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

// พบ pcf8574 ที่ตำแหน่ง 0x38
#define I2CADDR 0x38

const byte ROWS = 4;          // กำหนดจำนวน row
const byte COLS = 4;          // กำหนดจำนวน column

char hexaKeys[ROWS][COLS] = { // mappig key
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {7, 6, 5, 4}; // mapping pcf8574 pin
byte colPins[COLS] = {3, 2, 1, 0}; // row และ คอลัมม์

unsigned long last_time;      // ตัวแปรนับเวลาเพื่อ refresh จอ

LiquidCrystal_I2C lcd(0x27, 16, 2);       // ตั้งค่า lcd
Keypad_I2C keypad( makeKeymap(hexaKeys),  // ตั้งค่า keypad
 rowPins, colPins, ROWS, COLS, I2CADDR);
tmElements_t tm;                          //for DS1307

void print2digitsLCD(int number) {        // helper funciton 
  if (number >= 0 && number < 10) {       // สำหรับเขียนเลข 2 หลักลง lcd
    lcd.write('0');
  }
  lcd.print(number);
}
void print2digits(int number) {           // helper function
  if (number >= 0 && number < 10) {       // สำหรับเขียนเลข 2 หลักลง
    Serial.write('0');                    // Serial Monitor
  }
  Serial.print(number);
}

void setup()
{
  Serial.begin(38400);
  while (!Serial) ;       // wait for Arduino Serial Monitor
  delay(200);
  lcd.init();
  lcd.backlight();        // เปิด backlight
  lcd.home();

  refresh_display();      // เรียกการแสดง วันที่ และเวลา
  
  
}
void loop()
{
  if (millis() - last_time > 1000) {  
    last_time = millis(); // refresh หน้าจอทุก ๆ 1 วินาที
    refresh_display();
  }
}

void refresh_display() {
  if (RTC.read(tm)) { // อ่านค่าจาก ds1307
    Serial.print("Ok, Time = ");      // แสดงค่าบน Serial
    print2digits(tm.Hour);            // แสดง ชั่วโมง
    Serial.write(':');    
    print2digits(tm.Minute);          // แสดง นาที
    Serial.write(':');
    print2digits(tm.Second);          // แสดง วินาที 
                                      // (เพิ่มเติม เพื่อให้เห็นการตอบสนอง)
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);             // แสดง วันที่
    Serial.write('/');
    Serial.print(tm.Month);           // แสดง เดือน
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year)); // แสดงปี
    Serial.println();
    lcd.setCursor(0, 0);              // lcd แถวบน
    lcd.print("D/M/Y= ");             // แสดงวันที่
    lcd.print(tm.Day);                // วันที่
    lcd.print("/");
    lcd.print(tm.Month);              // เดือน
    lcd.print("/");
    lcd.print(tmYearToCalendar(tm.Year)); // ปี
    lcd.setCursor(0, 1);              // lcd แถวล่าง
    lcd.print("Time = ");             // แสดงเวลา
    print2digitsLCD(tm.Hour);         // ขั่วโมง
    lcd.print(":");
    print2digitsLCD(tm.Minute);       // นาทั
    lcd.print(":");
    print2digitsLCD(tm.Second);       // วินาที
  } else {                            // กรณีอ่านค่าไม่สำเร็จ
    if (RTC.chipPresent()) {          // แสดงข้อความแจ้งเตือนออกทาง Serial
      Serial.println("The DS1307 is stopped. Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error! Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
  } 
}
