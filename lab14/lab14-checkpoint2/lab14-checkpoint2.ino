#include <LiquidCrystal_I2C.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#define I2CADDR 0x20

const byte ROWS = 4;
const byte COLS = 4;

int hourDisplay;
int minuteDisplay;

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

int mode = 0; // 0 = run, 1 = set;
int select_time = 0; // 0 = hour, 1 = minute
int hour_display;
int minute_display;
int secondDisplay;
bool isBlink = 0;

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
  bool parse = false;
  bool config = false;
  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
    parse = true;
    // and configure the RTC with this info
    if (RTC.write(tm)) {
      config = true;
    }
  }
  Serial.begin(38400);
  while (!Serial) ; // wait for Arduino Serial Monitor
  delay(200);
  if (parse && config) {
    Serial.print("DS1307 configured Time=");
    Serial.print(__TIME__);
    Serial.print(", Date=");
    Serial.println(__DATE__);
  } else if (parse) {
    Serial.println("DS1307 Communication Error :-{");
    Serial.println("Please check your circuitry");
  } else {
    Serial.print("Could not parse info from the compiler, Time=\"");
    Serial.print(__TIME__);
    Serial.print("\", Date=\"");
    Serial.print(__DATE__);
    Serial.println("\"");
  }
  lcd.init();
  lcd.backlight(); // เปิด backlight
  lcd.home();

  refresh_display();
}
void loop()
{
  if (mode == 0) {
    if (millis() - last_time > 1000) {
      if (RTC.read(tm)) {
        last_time = millis();
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

    char key = keypad.getKey();
    if (key == 'B') {
      mode = 1;
    }
  }

  if (mode == 1) {
    // no reading from DS1307
    char key = keypad.getKey();
    if (key != NO_KEY) {
      if (key == 'B') {
        select_time = !select_time;
      }

      if (key == 'C') {
        if (select_time) { // minute
          minuteDisplay++;
          if (minuteDisplay == 60) minuteDisplay = 0;
        } else { //hour
          hourDisplay++;
          if (hourDisplay == 24) hourDisplay = 0;
        }
        refresh_display();
      }

      if (key == 'D') {
        if (select_time) { // minute
          minuteDisplay--;
          if (minuteDisplay == -1) minuteDisplay = 59;
        } else { // hour
          hourDisplay--;
          if (hourDisplay == -1) hourDisplay = 23;
        }
        refresh_display();
      }

      if (key == '#') {
        tm.Minute = minuteDisplay;
        tm.Hour = hourDisplay;
        if (RTC.write(tm)) {
          Serial.println("[/] Write New Time Success");
          mode = 0;
        } else {
          Serial.println("[!] Write New Time Failed");
        }
      }
    }

    // Blink Display
    if (millis() - last_time > 500) {
      last_time = millis();
      if (select_time) { // minute
        if (isBlink) {
          lcd.setCursor(10, 1);
          lcd.print("  ");
        } else {
          lcd.setCursor(10, 1);
          print2digitsLCD(minuteDisplay);
        }
        isBlink = !isBlink;
      } else { // hour
        if (isBlink) {
          lcd.setCursor(7, 1);
          lcd.print("  ");
        } else {
          lcd.setCursor(7, 1);
          print2digitsLCD(hourDisplay);
        }
        isBlink = !isBlink;
      } 
    }
  }
}

void refresh_display() {
  lcd.setCursor(0, 0);
  lcd.print("D/M/Y= ");
  lcd.print(tm.Day);
  lcd.print("/");
  lcd.print(tm.Month);
  lcd.print("/");
  lcd.print(tmYearToCalendar(tm.Year));

  lcd.setCursor(0, 1);
  lcd.print("Time = ");
  print2digitsLCD(hourDisplay);
  lcd.print(":");
  print2digitsLCD(minuteDisplay);
  lcd.print(".");
  print2digitsLCD(secondDisplay);
}
