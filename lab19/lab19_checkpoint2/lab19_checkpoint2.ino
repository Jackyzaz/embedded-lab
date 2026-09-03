#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

// I2C Address ของ PCF8574 สำหรับ Keypad
#define I2CADDR 0x20

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

// สถานะการทำงานร่วมกันระหว่าง Tasks
volatile int mode = 0;             // 0 = running mode, 1 = set time, 2 = set date
volatile int select_time = 0;      // 0 = hour, 1 = minute
volatile int select_date = 0;      // 0 = day, 1 = month, 2 = year
volatile bool isBlink = 0;         // สถานะการกะพริบตัวเลขบนหน้าจอ

// อุปกรณ์ I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Keypad_I2C keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, I2CADDR);
tmElements_t tm;                   // โครงสร้างข้อมูลเวลาจาก TimeLib

// Mutex สำหรับป้องกัน Bus Collision บน I2C (LCD, Keypad, RTC แชร์บัสร่วมกัน)
SemaphoreHandle_t xI2CMutex = NULL;

// Function Prototypes
void Task_Clock_Display(void *pvParameters);
void Task_Keypad(void *pvParameters);
void refresh_display(void);
void print_serial_time(void);
void print2digits(int number);
void print2digitsLCD(int number);

// FreeRTOS Stack Overflow Hook เพื่อตรวจจับและพิมพ์ชื่อ Task ที่ Stack ล้น
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void) xTask;
  Serial.print(F("[FATAL] Stack Overflow in: "));
  Serial.println(pcTaskName);
  while (1);
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.print('0');
  }
  Serial.print(number);
}

void print2digitsLCD(int number) {
  if (number >= 0 && number < 10) {
    lcd.print('0');
  }
  lcd.print(number);
}

// พิมพ์เวลาและวันที่ออก Serial Monitor
void print_serial_time() {
  Serial.print(F("Time: "));
  print2digits(tm.Hour);
  Serial.print(F(":"));
  print2digits(tm.Minute);
  Serial.print(F(":"));
  print2digits(tm.Second);
  Serial.print(F(" | Date: "));
  print2digits(tm.Day);
  Serial.print(F("/"));
  print2digits(tm.Month);
  Serial.print(F("/"));
  Serial.println(tmYearToCalendar(tm.Year));
}

// ฟังก์ชันรีเฟรชหน้าจอ LCD (ต้องเรียกภายใต้การ Lock Mutex)
void refresh_display() {
  // แถว 0: Date: DD/MM/YYYY (ความยาว 16 ช่องพอดี)
  lcd.setCursor(0, 0);
  lcd.print(F("Date: "));
  print2digitsLCD(tm.Day);
  lcd.print(F("/"));
  print2digitsLCD(tm.Month);
  lcd.print(F("/"));
  lcd.print(tmYearToCalendar(tm.Year));

  // แถว 1: Time = HH:MM.SS (ความยาว 16 ช่องพอดี)
  lcd.setCursor(0, 1);
  lcd.print(F("Time = "));
  print2digitsLCD(tm.Hour);
  lcd.print(F(":"));
  print2digitsLCD(tm.Minute);
  lcd.print(F("."));
  print2digitsLCD(tm.Second);
  lcd.print(F(" "));
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("Lab19.2 RTOS Checkpoint 2"));

  // ดับไฟ LED บนบอร์ด (Pin 13) เริ่มต้น
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // 1. สร้าง Mutex สำหรับควบคุม I2C Bus ก่อนเริ่ม Task
  xI2CMutex = xSemaphoreCreateMutex();

  // 2. กำหนดค่าเริ่มต้นให้กับอุปกรณ์ I2C
  Wire.begin();

  keypad.begin();
  lcd.init();
  lcd.backlight();
  lcd.home();

  // 3. อ่านค่าเวลาเริ่มต้นจาก DS1307
  if (!RTC.read(tm)) {
    tm.Hour = 12;
    tm.Minute = 0;
    tm.Second = 0;
    tm.Day = 1;
    tm.Month = 1;
    tm.Year = CalendarYrToTm(2026);
    tm.Wday = 1;
  }
  refresh_display();
  print_serial_time();

  // 4. จัดลำดับ Priority ใหม่:
  // ClockTask: Priority 2 (สูงสุด) เพื่อให้อ่าน RTC และเขียนจอ LCD ได้อย่างราบรื่นไม่โดนขัด
  // KeypadTask: Priority 1 สแกนปุ่มกดทุก 60ms ในช่วงที่ ClockTask หลับพัก
  xTaskCreate(Task_Clock_Display, "ClockTask", 200, NULL, 2, NULL);
  xTaskCreate(Task_Keypad, "KeypadTask", 200, NULL, 1, NULL);

  Serial.println(F("[OK] Scheduler Starting..."));
  Serial.flush();

  // 5. เริ่มต้นการทำงานของ RTOS Scheduler
  vTaskStartScheduler();
}

void loop() {
  // empty loop
}

// 
// Task 1: จัดการเวลา และควบคุมการแสดงผลบนหน้าจอ LCD (Priority 2 - สูงสุด)
// 
void Task_Clock_Display(void *pvParameters) {
  (void) pvParameters;

  while (1) {
    if (mode == 0) {
      // --- Running Mode: ตรวจเช็คเวลาจาก RTC ทุก 500ms เพื่อความลื่นไหลของเข็มวินาที ---
      if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        if (RTC.read(tm)) {
          refresh_display();
          print_serial_time();
        }
        xSemaphoreGive(xI2CMutex);
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    } else if (mode == 1) {
      // --- Setting Time Mode: กระพริบหลักเวลาทุก 500ms ---
      if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        if (select_time == 1) {
          // กะพริบหลักนาที (ตำแหน่งช่องที่ 10-11 บรรทัดที่ 1)
          lcd.setCursor(10, 1);
          if (isBlink) {
            lcd.print(F("  "));
          } else {
            print2digitsLCD(tm.Minute);
          }
        } else {
          // กะพริบหลักชั่วโมง (ตำแหน่งช่องที่ 7-8 บรรทัดที่ 1)
          lcd.setCursor(7, 1);
          if (isBlink) {
            lcd.print(F("  "));
          } else {
            print2digitsLCD(tm.Hour);
          }
        }
        isBlink = !isBlink;
        xSemaphoreGive(xI2CMutex);
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    } else if (mode == 2) {
      // --- Setting Date Mode: กระพริบหลักวันที่ทุก 500ms ---
      if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        if (select_date == 0) {
          // กะพริบหลักวัน (ตำแหน่งช่องที่ 6-7 บรรทัดที่ 0)
          lcd.setCursor(6, 0);
          if (isBlink) {
            lcd.print(F("  "));
          } else {
            print2digitsLCD(tm.Day);
          }
        } else if (select_date == 1) {
          // กะพริบหลักเดือน (ตำแหน่งช่องที่ 9-10 บรรทัดที่ 0)
          lcd.setCursor(9, 0);
          if (isBlink) {
            lcd.print(F("  "));
          } else {
            print2digitsLCD(tm.Month);
          }
        } else if (select_date == 2) {
          // กะพริบหลักปี (ตำแหน่งช่องที่ 12-15 บรรทัดที่ 0)
          lcd.setCursor(12, 0);
          if (isBlink) {
            lcd.print(F("    "));
          } else {
            lcd.print(tmYearToCalendar(tm.Year));
          }
        }
        isBlink = !isBlink;
        xSemaphoreGive(xI2CMutex);
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
}

// 
// Task 2: สแกน Keypad และประมวลผลคำสั่งตั้งค่า (Priority 1)
// 
void Task_Keypad(void *pvParameters) {
  (void) pvParameters;

  while (1) {
    char key = NO_KEY;

    // จอง Mutex สั้นๆ เพื่ออ่านค่าจาก Keypad ผ่าน PCF8574 บน I2C
    if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      key = keypad.getKey();
      xSemaphoreGive(xI2CMutex);
    }

    if (key != NO_KEY) {
      if (mode == 0) {
        // [โหมดปกติ]
        if (key == 'B') {
          // กด B เข้าสู่โหมดตั้งเวลา (เริ่มที่ชั่วโมง)
          mode = 1;
          select_time = 0;
          isBlink = 0;
          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            refresh_display();
            xSemaphoreGive(xI2CMutex);
          }
        } else if (key == 'A') {
          // กด A เข้าสู่โหมดตั้งวันที่ (เริ่มที่วัน)
          mode = 2;
          select_date = 0;
          isBlink = 0;
          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            refresh_display();
            xSemaphoreGive(xI2CMutex);
          }
        }
      } else if (mode == 1) {
        // [โหมดตั้งเวลา]
        if (key == 'B') {
          // สลับหลักการตั้งค่า: ชั่วโมง (0) <-> นาที (1)
          select_time = !select_time;
          isBlink = 0;
          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            refresh_display();
            xSemaphoreGive(xI2CMutex);
          }
        } else if (key == 'C') {
          // เพิ่มค่าเวลา (+1)
          if (select_time == 1) {
            tm.Minute++;
            if (tm.Minute >= 60) tm.Minute = 0; // วน 0-59
          } else {
            tm.Hour++;
            if (tm.Hour >= 24) tm.Hour = 0;     // วน 0-23
          }
          isBlink = 0;
          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            refresh_display();
            xSemaphoreGive(xI2CMutex);
          }
        } else if (key == 'D') {
          // ลดค่าเวลา (-1) ป้องกัน Underflow
          if (select_time == 1) {
            if (tm.Minute == 0) tm.Minute = 59;
            else tm.Minute--;
          } else {
            if (tm.Hour == 0) tm.Hour = 23;
            else tm.Hour--;
          }
          isBlink = 0;
          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            refresh_display();
            xSemaphoreGive(xI2CMutex);
          }
        } else if (key == '#') {
          // กด '#' บันทึกเวลาลง DS1307
          tm.Second = 0;
          if (tm.Wday < 1 || tm.Wday > 7) tm.Wday = 1;

          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            if (RTC.write(tm)) {
              Serial.println(F("[/] Write New Time Success"));
              mode = 0;
              refresh_display();
            } else {
              Serial.println(F("[!] Write New Time Failed"));
            }
            xSemaphoreGive(xI2CMutex);
          }
        }
      } else if (mode == 2) {
        // [โหมดตั้งวันที่]
        if (key == 'A') {
          // สลับหลักการตั้งค่า: วัน (0) -> เดือน (1) -> ปี (2) -> วัน (0)
          select_date = (select_date + 1) % 3;
          isBlink = 0;
          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            refresh_display();
            xSemaphoreGive(xI2CMutex);
          }
        } else if (key == 'C') {
          // เพิ่มค่า (+1)
          if (select_date == 0) {
            tm.Day++;
            if (tm.Day > 31) tm.Day = 1; // วน 1-31
          } else if (select_date == 1) {
            tm.Month++;
            if (tm.Month > 12) tm.Month = 1; // วน 1-12
          } else if (select_date == 2) {
            int curYear = tmYearToCalendar(tm.Year);
            curYear++;
            if (curYear > 2099) curYear = 2000;
            tm.Year = CalendarYrToTm(curYear);
          }
          isBlink = 0;
          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            refresh_display();
            xSemaphoreGive(xI2CMutex);
          }
        } else if (key == 'D') {
          // ลดค่า (-1)
          if (select_date == 0) {
            if (tm.Day <= 1) tm.Day = 31;
            else tm.Day--;
          } else if (select_date == 1) {
            if (tm.Month <= 1) tm.Month = 12;
            else tm.Month--;
          } else if (select_date == 2) {
            int curYear = tmYearToCalendar(tm.Year);
            curYear--;
            if (curYear < 2000) curYear = 2099;
            tm.Year = CalendarYrToTm(curYear);
          }
          isBlink = 0;
          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            refresh_display();
            xSemaphoreGive(xI2CMutex);
          }
        } else if (key == '#') {
          // กด '#' บันทึกวันที่ลง DS1307
          tm.Second = 0; // เคลียร์วินาทีเป็น 0 และปลดล็อก Clock Halt (CH bit = 0)
          if (tm.Wday < 1 || tm.Wday > 7) tm.Wday = 1; // กำหนด Day of Week ให้ถูกต้อง (1-7)

          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            if (RTC.write(tm)) {
              Serial.println(F("[/] Write New Date Success"));
              mode = 0;
              refresh_display();
            } else {
              Serial.println(F("[!] Write New Date Failed"));
            }
            xSemaphoreGive(xI2CMutex);
          }
        }
      }
    }

    // หน่วงเวลาสแกนปุ่ม 60ms เพื่อลดการยิงคำสั่ง I2C Bus รัวเกินไป
    vTaskDelay(pdMS_TO_TICKS(60));
  }
}
