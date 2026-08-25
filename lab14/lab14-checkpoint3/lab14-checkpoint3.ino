#include <LiquidCrystal_I2C.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#define I2CADDR 0x38

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {7, 6, 5, 4};
byte colPins[COLS] = {3, 2, 1, 0};


unsigned long last_time;  // ตัวแปรจับเวลา
int mode = 0;             // 0 = run, 1 = set time, 2 = set date
int select_time = 0;      // 0 = hour, 1 = minute
int select_date = 0;      // 0 = day, 1 = month, 2 = year
int hourDisplay;          // ตัวแปรแทน tm.hour
int minuteDisplay;        // ตัวแปรแทน tm.minute
int secondDisplay;        // ตัวแปรแทน tm.second
int dayDisplay;           // ตัวแปรแทน tm.day
int monthDisplay;         // ตัวแปรแทน tm.month
int yearDisplay;          // ตัวแปรแทน tm.year
bool isBlink = 0;         // ตัวแปรจัดการ การกระพริบ

LiquidCrystal_I2C lcd(0x27, 16, 2);
Keypad_I2C keypad( makeKeymap(hexaKeys), rowPins,
                   colPins, ROWS, COLS, I2CADDR);
tmElements_t tm;          //for DS1307

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void print2digitsLCD(int number) {
  if (number >= 0 && number < 10) {
    lcd.write('0');
  }
  lcd.print(number);
}

void setup()
{
  Serial.begin(38400);
  while (!Serial) ; // wait for Arduino Serial Monitor
  delay(200);
  lcd.init();
  lcd.backlight(); // เปิด backlight
  lcd.home();
  Wire.begin();
  keypad.begin();
  refresh_display();
}

void loop() {
  char key = keypad.getKey();
  if (mode == 0) {                        // running mode
    if (millis() - last_time > 1000) {    // ตอบสนองทุก 1 วินาที
      if (RTC.read(tm)) {
        last_time = millis();
        hourDisplay = tm.Hour;
        minuteDisplay = tm.Minute;
        secondDisplay = tm.Second;
        refresh_display();
      } else {
        if (RTC.chipPresent()) {
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
    char key = keypad.getKey();           // อ่านค่าจาก keypad
    if (key == 'B') {                     // เปลี่ยน mode ตั้งเวลา เมื่อกด B
      mode = 1;
    }
    if (key == 'A') {                     // เปลี่ยน mode ตั้งวันที่ เมื่อกด A
      mode = 2;
      select_date = 0;
    }
  }
  else if (mode == 1) {                   // mode set เวลา
    char key = keypad.getKey();
    if (key != NO_KEY) {
      if (key == 'B') {                   // เลือกระหว่าง ชั่วโมง หรือ นาที
        select_time = !select_time;       // กลับโหมด 0->1->0 เมื่อกด B
      }
      if (key == 'C') {                   // กด C เป็นการเพิ่มค่า
        if (select_time) {                // สำหรับ นาที
          minuteDisplay++;
          if (minuteDisplay == 60)        // วน 0->59->0
            minuteDisplay = 0;
        } else {                          // สำหรับชั่วโมง
          hourDisplay++;
          if (hourDisplay == 24)          // วัน 0->23->0
            hourDisplay = 0;
        }
        refresh_display();                // เรียก refresh_display() เสมอ
      }
      if (key == 'D') {                   // กด D เพื่อลดค่า
        if (select_time) {                // สำหรับนาที
          minuteDisplay--;
          if (minuteDisplay == -1)        // วนลดค่า 59->0->59
            minuteDisplay = 59;
        } else {                          // สำหรับชั่วโมง
          hourDisplay--;
          if (hourDisplay == -1)          // วนลดค่า 23->0->23
            hourDisplay = 23;
        }
        refresh_display();                // เรียก refresh_display() เสมอ
      }
      if (key == '#') {                   // กด # จะ save ค่าลง DS 1307 แล้วนับต่อ
        tm.Minute = minuteDisplay;
        tm.Hour = hourDisplay;
        if (RTC.write(tm)) {
          Serial.println("[/] Write New Time Success");
          mode = 0;
        } else {                          // Handle กรณีมีข้อผิดพลาด
          Serial.println("[!] Write New Time Failed");
        }
      }

      if (millis() - last_time > 500) {     // ควบคุมการกระพิบหลักขณะที่มีการ set ค่าเวลา
        last_time = millis();               // แบบ non-blocking function
        if (select_time) {                  // สำหรับนาที
          if (isBlink) {
            lcd.setCursor(10, 1);           // ตำแหน่งของนาที คือแถวล่าง ช่อง 10-11
            lcd.print("  ");
          } else {
            lcd.setCursor(10, 1);
            print2digitsLCD(minuteDisplay); // ใช้ helper function แสดงค่า 2 หลัก
          }
          isBlink = !isBlink;
        } else {                            // สำหรัั่วโมง
          if (isBlink) {
            lcd.setCursor(7, 1);            // ตำแหน่งของนาที คือแถวบน ช่อง 7-8
            lcd.print("  ");
          } else {
            lcd.setCursor(7, 1);
            print2digitsLCD(hourDisplay);   // ใช้ helper function แสดงค่า 2 หลัก
          }
          isBlink = !isBlink;               // กลับค่า logic เพื่อให้กระพริบ
        }
      }
    }
  } else if (mode == 2) {                   // mode set วันที่
    if (key != NO_KEY) {
      if (key == 'A') {                     // เลือกระหว่างวันที่, เดือน, ปี
        refresh_display();
        select_date = (select_date + 1) % 3;    // วนระหว่าง วันที่ -> เดือน -> ปี
      }
      if (key == 'C') {                     // กด C เป็นการเพิ่มค่า
        if (select_date == 0) {             // day
          dayDisplay++;
          if (dayDisplay > 31)              // วน 0->31->0
            dayDisplay = 1;
        } else if (select_date == 1) {      // month
          monthDisplay++;                   // วน 0->12->0
          if (monthDisplay > 12) 
            monthDisplay = 1;
        } else if (select_date == 2) {      // year
          yearDisplay;
        }
        refresh_display();
      }
      if (key == 'D') {                      // กด D เป็นการลดค่า
        if (select_date == 0) {              // day
          dayDisplay--;
          if (dayDisplay < 1)                // วน 31->0->31
            dayDisplay = 31;
        } else if (select_date == 1) {       // month
          monthDisplay--;
          if (monthDisplay < 1)              // วน 12->0->12
            monthDisplay = 12;
        } else if (select_date == 2) {       // year
          yearDisplay--;
        }
        refresh_display();
      }
      if (key == '#') {              // กด # จะ save ค่าลง DS 1307 แล้วนับต่อ
        tm.Day = dayDisplay;
        tm.Month = monthDisplay;
        tm.Year = CalendarYrToTm(yearDisplay);
        if (RTC.write(tm)) {
          Serial.println("[/] Write New Date Success");
          mode = 0;
        } else {    // Handle กรณีมีข้อผิดพลาด
          Serial.println("[!] Write New Date Failed");
        }
      }
    }
    if (millis() - last_time > 500) { // ควบคุมการกระพิบหลักขณะที่มีการ set ค่าวันที่
      last_time = millis();           // แบบ non-blocking function
      if (select_date == 0) {         // Day
        lcd.setCursor(6, 0);          // ตำแหน่งของวันที่ คือแถวบน ช่อง 6-7
        if (isBlink) 
          lcd.print("  ");
        else print2digitsLCD(dayDisplay);
      } else if (select_date == 1) {  // Month
        lcd.setCursor(9, 0);          // ตำแหน่งของวันที่ คือแถวบน ช่อง 9-10
        if (isBlink) 
          lcd.print("  ");
        else print2digitsLCD(monthDisplay);
      } else if (select_date == 2) {  // Year
        lcd.setCursor(12, 0);         // ตำแหน่งของวันที่ คือแถวบน ช่อง 12-15
        if (isBlink) 
          lcd.print("    ");
        else lcd.print(yearDisplay);
      }
      isBlink = !isBlink;
    }
  }
}

void refresh_display() {
  Serial.print("Ok, Time = ");
  print2digits(tm.Hour);
  Serial.write(':');
  print2digits(tm.Minute);
  Serial.write(':');
  print2digits(tm.Second);
  Serial.print(", Date (D/M/Y) = ");
  Serial.print(tm.Day);
  Serial.write('/');
  Serial.print(tm.Month);
  Serial.write('/');
  Serial.print(tmYearToCalendar(tm.Year));
  Serial.println();
  lcd.setCursor(0, 0);
  lcd.setCursor(0, 0);
  lcd.print("Date: ");
  print2digitsLCD(dayDisplay);
  lcd.print("/");
  print2digitsLCD(monthDisplay);
  lcd.print("/");
  lcd.print(yearDisplay);

  lcd.setCursor(0, 1);
  lcd.print("Time = ");
  print2digitsLCD(hourDisplay);
  lcd.print(":");
  print2digitsLCD(minuteDisplay);
  lcd.print(":");
  print2digitsLCD(secondDisplay);
}
