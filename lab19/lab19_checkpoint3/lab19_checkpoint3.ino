#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

// Hardware & I2C Address Configuration
#define KEYPAD_I2C_ADDR 0x20   // I2C Address ของ PCF8574 สำหรับ Keypad
#define LCD_I2C_ADDR    0x27   // I2C Address ของ LCD 1602
#define EEPROM_ADDR     0x50   // I2C Address ของ 24C32 บนโมดูล DS1307 RTC
#define EEPROM_WL_SIZE  100    // ขนาด Block สำหรับทำ Wear Leveling (100 ไบต์)

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


volatile int mode = 0;             // 0 = running mode, 1 = set time, 2 = set date
volatile int select_time = 0;      // 0 = hour, 1 = minute
volatile int select_date = 0;      // 0 = day, 1 = month, 2 = year
volatile bool isBlink = 0;         // สถานะการกะพริบตัวเลขบนหน้าจอ

// สถานะการสลับหน้าจอ (1 = Date บน / Time ล่าง, 2 = Time บน / Date ล่าง)
volatile uint8_t display_mode = 1;
unsigned int current_wl_addr = 0;  // ตำแหน่ง Address ปัจจุบันใน 24C32 EEPROM

// อุปกรณ์ I2C
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 16, 2);
Keypad_I2C keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, KEYPAD_I2C_ADDR);
tmElements_t tm;                   // โครงสร้างข้อมูลเวลาจาก TimeLib

// Mutex สำหรับป้องกัน Bus Collision บน I2C (LCD, Keypad, RTC, 24C32 EEPROM แชร์บัสร่วมกัน)
SemaphoreHandle_t xI2CMutex = NULL;


// Function Prototypes

void Task_Clock_Display(void *pvParameters);
void Task_Keypad(void *pvParameters);
void refresh_display(void);
void print_serial_time(void);
void print2digits(int number);
void print2digitsLCD(int number);

// EEPROM 24C32 Functions
uint8_t readEEPROM(unsigned int eeaddress);
void writeEEPROM(unsigned int eeaddress, uint8_t data);
uint8_t readModeFromEEPROM_WearLeveling(void);
void writeModeToEEPROM_WearLeveling(uint8_t new_mode);
int scan_EEPROM(void);

// FreeRTOS Stack Overflow Hook
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void) xTask;
  Serial.print(F("[FATAL] Stack Overflow in: "));
  Serial.println(pcTaskName);
  while (1);
}


// EEPROM 24C32 Low-Level Functions (I2C Address 0x50)
// ฟังก์ชันอ่านข้อมูล 1 ไบต์จาก Serial EEPROM 24C32
uint8_t readEEPROM(unsigned int eeaddress) {
  uint8_t rdata = 0xFF;
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((int)(eeaddress >> 8));   // Address MSB
  Wire.write((int)(eeaddress & 0xFF)); // Address LSB
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)EEPROM_ADDR, (uint8_t)1);
  if (Wire.available()) {
    rdata = Wire.read();
  }
  return rdata;
}

// ฟังก์ชันเขียนข้อมูล 1 ไบต์ลง Serial EEPROM 24C32
void writeEEPROM(unsigned int eeaddress, uint8_t data) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((int)(eeaddress >> 8));   // Address MSB
  Wire.write((int)(eeaddress & 0xFF)); // Address LSB
  Wire.write(data);
  Wire.endTransmission();
  delay(10); // รอ 10ms สำหรับ Write Cycle Time ของ 24C32
}

// ฟังก์ชันสแกนข้อมูลใน 24C32 EEPROM เพื่อตรวจสอบตำแหน่งที่มีข้อมูล
int scan_EEPROM(void) {
  int number_data_found = 0;
  Serial.println(F("Performing EEPROM 24C32 scanning..."));
  for (unsigned int i = 0; i < EEPROM_WL_SIZE; i++) {
    uint8_t d = readEEPROM(i);
    if (d != 0xFF) {
      number_data_found++;
      Serial.print(F("Data 0x0"));
      Serial.print(d, HEX);
      Serial.print(F(" is active at 24C32 location ["));
      Serial.print(i);
      Serial.println(F("] (All other slots are 0xFF)"));
    }
  }
  if (number_data_found == 0) {
    Serial.println(F("No data found in EEPROM 24C32"));
  }
  return number_data_found;
}

// Wear Leveling (ช่องอื่นทั้งหมดจะเป็น 0xFF เสมอ)
// สแกนหาช่องเดียวที่ไม่ใช่ 0xFF จากทั้งหมด 100 ช่อง
uint8_t readModeFromEEPROM_WearLeveling() {
  uint8_t found_mode = 1;
  bool found = false;
  for (unsigned int i = 0; i < EEPROM_WL_SIZE; i++) {
    uint8_t val = readEEPROM(i);
    if (val == 1 || val == 2) {
      found_mode = val;
      current_wl_addr = i;
      found = true;
      break; // พบตำแหน่งปัจจุบันแล้ว
    }
  }
  // หากเป็น EEPROM ใหม่หรือยังไม่มีข้อมูล ให้เขียนค่าเริ่มต้นลงช่อง 0
  if (!found) {
    found_mode = 1;
    current_wl_addr = 0;
    writeEEPROM(0, 1);
  }
  Serial.print(F("[EEPROM WL] Active Display Mode: "));
  Serial.print(found_mode);
  Serial.print(F(" at Address: ["));
  Serial.print(current_wl_addr);
  Serial.println(F("]"));

  return found_mode;
}

// บันทึก Mode ใหม่: เขียนช่องถัดไป และลบช่องเดิมให้เป็น 0xFF
void writeModeToEEPROM_WearLeveling(uint8_t new_mode) {
  unsigned int next_addr = (current_wl_addr + 1) % EEPROM_WL_SIZE;
  // 1. เขียนค่าโหมดใหม่ลงช่องถัดไป
  writeEEPROM(next_addr, new_mode);
  // 2. เคลียร์ช่องเดิมให้กลับเป็น 0xFF (ทำให้ช่องอื่นๆ ในบล็อกเป็น 0xFF ทั้งหมดเสมอ)
  writeEEPROM(current_wl_addr, 0xFF);
  current_wl_addr = next_addr;
  display_mode = new_mode;
  Serial.print(F("[EEPROM WL] Moved Mode "));
  Serial.print(new_mode);
  Serial.print(F(" -> Address: ["));
  Serial.print(current_wl_addr);
  Serial.println(F("] (Cleared previous slot to 0xFF)"));
}


// Display & Formatting Helpers
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
  Serial.print(tmYearToCalendar(tm.Year));
  Serial.print(F(" | [Disp Mode: "));
  Serial.print(display_mode);
  Serial.print(F(" @ Addr: "));
  Serial.print(current_wl_addr);
  Serial.println(F("]"));
}

// ฟังก์ชันรีเฟรชหน้าจอ LCD แบบมีวินาที (ต้องเรียกภายใต้การ Lock Mutex)
void refresh_display() {
  uint8_t date_row = (display_mode == 1) ? 0 : 1;
  uint8_t time_row = (display_mode == 1) ? 1 : 0;

  // แถววันที่: D/M/Y=DD/MM/YYYY (ความยาว 16 ช่องพอดี)
  lcd.setCursor(0, date_row);
  lcd.print(F("D/M/Y="));
  print2digitsLCD(tm.Day);
  lcd.print(F("/"));
  print2digitsLCD(tm.Month);
  lcd.print(F("/"));
  lcd.print(tmYearToCalendar(tm.Year));

  // แถวเวลา: Time = HH:MM.SS (ความยาว 16 ช่องพอดี)
  lcd.setCursor(0, time_row);
  lcd.print(F("Time = "));
  print2digitsLCD(tm.Hour);
  lcd.print(F(":"));
  print2digitsLCD(tm.Minute);
  lcd.print(F("."));
  print2digitsLCD(tm.Second);
  lcd.print(F(" "));
}


// Setup & Loop

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("Lab19.3 RTOS Checkpoint 3 (24C32 Wear Leveling)"));

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

  // 3. อ่านค่า Mode ล่าสุดจาก 24C32 EEPROM ด้วย Wear Leveling
  display_mode = readModeFromEEPROM_WearLeveling();
  scan_EEPROM(); // แสดงตำแหน่งที่มีข้อมูลใน 24C32 ออกทาง Serial Monitor

  // 4. อ่านค่าเวลาเริ่มต้นจาก DS1307
   if (RTC.read(tm)) {
    Serial.println("[3] RTC read initial success");
  } else {
    Serial.println("[3] RTC read initial failed, using fallback");
    while(1); // Blocking
  }
  refresh_display();

  // 5. สร้าง FreeRTOS Tasks ด้วย Priority และ Stack ที่เหมาะสม
  xTaskCreate(Task_Clock_Display, "ClockTask", 140, NULL, 2, NULL);
  xTaskCreate(Task_Keypad, "KeypadTask", 160, NULL, 1, NULL);

  Serial.println(F("[OK] Scheduler Starting..."));
  Serial.flush();

  // 6. เริ่มต้นการทำงานของ RTOS Scheduler
  vTaskStartScheduler();
}

void loop() {
  // empty loop
}


// Task 1: จัดการเวลา และควบคุมการแสดงผลบนหน้าจอ LCD (Priority 2 - สูงสุด)

void Task_Clock_Display(void *pvParameters) {
  (void) pvParameters;
  uint8_t last_second = 0xFF; // ตัวแปรจำค่าวินาทีล่าสุด เพื่ออัปเดตเฉพาะเมื่อวินาทีเปลี่ยนจริง
  TickType_t xLastWakeTime = xTaskGetTickCount(); // เก็บ Tick เริ่มต้นสำหรับ Periodic Delay

  while (1) {
    uint8_t date_row = (display_mode == 1) ? 0 : 1;
    uint8_t time_row = (display_mode == 1) ? 1 : 0;

    if (mode == 0) {
      // --- Running Mode: ตรวจเช็ค RTC ทุก 100ms แบบ Absolute Periodic ---
      if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        if (RTC.read(tm)) {
          if (tm.Second != last_second) {
            last_second = tm.Second;
            refresh_display();
            print_serial_time();
          }
        }
        xSemaphoreGive(xI2CMutex);
      }
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    } else if (mode == 1) {
      last_second = 0xFF; // รีเซ็ตเพื่อให้รีเฟรชทันทีเมื่อออกจากโหมดตั้งค่า
      // --- Setting Time Mode: กะพริบหลักเวลาทุก 500ms แบบ Absolute Periodic ---
      if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        if (select_time == 1) {
          // กะพริบหลักนาที (ตำแหน่งช่องที่ 10-11 ของบรรทัดเวลา)
          lcd.setCursor(10, time_row);
          if (isBlink) {
            lcd.print(F("  "));
          } else {
            print2digitsLCD(tm.Minute);
          }
        } else {
          // กะพริบหลักชั่วโมง (ตำแหน่งช่องที่ 7-8 ของบรรทัดเวลา)
          lcd.setCursor(7, time_row);
          if (isBlink) {
            lcd.print(F("  "));
          } else {
            print2digitsLCD(tm.Hour);
          }
        }
        isBlink = !isBlink;
        xSemaphoreGive(xI2CMutex);
      }
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    } else if (mode == 2) {
      last_second = 0xFF; // รีเซ็ตเพื่อให้รีเฟรชทันทีเมื่อออกจากโหมดตั้งค่า
      // --- Setting Date Mode: กะพริบหลักวันที่ทุก 500ms แบบ Absolute Periodic ---
      if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        if (select_date == 0) {
          // กะพริบหลักวัน (ตำแหน่งช่องที่ 6-7 ของบรรทัดวันที่)
          lcd.setCursor(6, date_row);
          if (isBlink) {
            lcd.print(F("  "));
          } else {
            print2digitsLCD(tm.Day);
          }
        } else if (select_date == 1) {
          // กะพริบหลักเดือน (ตำแหน่งช่องที่ 9-10 ของบรรทัดวันที่)
          lcd.setCursor(9, date_row);
          if (isBlink) {
            lcd.print(F("  "));
          } else {
            print2digitsLCD(tm.Month);
          }
        } else if (select_date == 2) {
          // กะพริบหลักปี (ตำแหน่งช่องที่ 12-15 ของบรรทัดวันที่)
          lcd.setCursor(12, date_row);
          if (isBlink) {
            lcd.print(F("    "));
          } else {
            lcd.print(tmYearToCalendar(tm.Year));
          }
        }
        isBlink = !isBlink;
        xSemaphoreGive(xI2CMutex);
      }
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
  }
}


// Task 2: สแกน Keypad และประมวลผลคำสั่งตั้งค่า (Priority 1)

void Task_Keypad(void *pvParameters) {
  (void) pvParameters;
  TickType_t xLastWakeTime = xTaskGetTickCount(); // เก็บ Tick เริ่มต้นสำหรับ Periodic Delay

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
        if (key == '0') {
          // กดปุ่ม '0': ทอกเกิลสลับระหว่าง Mode1 และ Mode2 พร้อมบันทึก 24C32 Wear Leveling
          uint8_t new_mode = (display_mode == 1) ? 2 : 1;
          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(300)) == pdTRUE) {
            writeModeToEEPROM_WearLeveling(new_mode);
            refresh_display();
            xSemaphoreGive(xI2CMutex);
          }
        } else if (key == 'B') {
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

    // หน่วงเวลาสแกนปุ่ม 60ms แบบ Absolute Periodic (แม่นยำ 16.67 Hz)
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(60));
  }
}
