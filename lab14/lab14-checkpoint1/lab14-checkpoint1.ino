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

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

byte rowPins[ROWS] = {7, 6, 5, 4};
byte colPins[COLS] = {3, 2, 1, 0};
unsigned long last_time;
bool blink_txt;

LiquidCrystal_I2C lcd(0x27, 16, 2); //Module IIC/I2C Interface บางรุFนอาจจะใชN 0x3f
Keypad_I2C keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, I2CADDR);
tmElements_t tm; //for DS1307



bool getTime(const char *str)
{
  int Hour, Min, Sec;
  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}
bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;
  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}
void print2digitsLCD(int number) {
  if (number >= 0 && number < 10) {
    lcd.write('0');
  }
  lcd.print(number);
}
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void setup()
{
//  bool parsed = false;
//  bool configured = false;
//  // get the date and time the compiler was run
//  if (getDate(__DATE__) && getTime(__TIME__)) {
//    parsed = true;
//    // and configure the RTC with this info
//    if (RTC.write(tm)) {
//      configured = true;
//    }
//  }
  Serial.begin(38400);
  while (!Serial) ; // wait for Arduino Serial Monitor
  delay(200);
//  if (parsed && configured) {
//    Serial.print("DS1307 configured Time=");
//    Serial.print(__TIME__);
//    Serial.print(", Date=");
//    Serial.println(__DATE__);
//  } else if (parsed) {
//    Serial.println("DS1307 Communication Error :-{");
//    Serial.println("Please check your circuitry");
//  } else {
//    Serial.print("Could not parse info from the compiler, Time=\"");
//    Serial.print(__TIME__);
//    Serial.print("\", Date=\"");
//    Serial.print(__DATE__);
//    Serial.println("\"");
//  }
  lcd.init();
  lcd.backlight(); // เปิด backlight
  lcd.home();

  refresh_display();
  
  
}
void loop()
{
  if (millis() - last_time > 1000) {
    last_time = millis();
    refresh_display();
  }
}

void refresh_display() {
  if (RTC.read(tm)) {
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
    lcd.print("D/M/Y= ");
    lcd.print(tm.Day);
    lcd.print("/");
    lcd.print(tm.Month);
    lcd.print("/");
    lcd.print(tmYearToCalendar(tm.Year));

    lcd.setCursor(0, 1);
    lcd.print("Time = ");
    print2digitsLCD(tm.Hour);
    lcd.print(":");
    print2digitsLCD(tm.Minute);
    lcd.print(":");
    print2digitsLCD(tm.Second);
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
