#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#define I2CADDR 0x20    // Address ของ PCF8574 สำหรับ Keypad

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
volatile int mode = 0;             // 0 = running mode, 1 = time setting mode
volatile int select_time = 0;      // 0 = hour, 1 = minute
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
void print2digits(int number);
void print2digitsLCD(int number);

// LCD Helper function
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

// ฟังก์ชันรีเฟรชหน้าจอ LCD และส่งค่าออก Serial Monitor (ต้องเรียกภายใต้การ Lock Mutex)
void refresh_display() {
  Serial.print("Ok, Time = ");
  print2digits(tm.Hour);
  Serial.print(":");
  print2digits(tm.Minute);
  Serial.print(":");
  print2digits(tm.Second);
  Serial.print(", Date (D/M/Y) = ");
  Serial.print(tm.Day);
  Serial.print("/");
  Serial.print(tm.Month);
  Serial.print("/");
  Serial.println(tmYearToCalendar(tm.Year));

  lcd.setCursor(0, 0);
  lcd.print("D/M/Y= ");
  lcd.print(tm.Day);
  lcd.print("/");
  lcd.print(tm.Month);
  lcd.print("/");
  lcd.print(tmYearToCalendar(tm.Year));
  lcd.print("   ");

  lcd.setCursor(0, 1);
  lcd.print("Time = ");
  print2digitsLCD(tm.Hour);
  lcd.print(":");
  print2digitsLCD(tm.Minute);
  lcd.print(".");
  print2digitsLCD(tm.Second);
  lcd.print("   ");
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Lab19.1 RTOS 2");
  
  // 1. สร้าง Mutex สำหรับควบคุม I2C Bus ก่อนเริ่ม Task
  xI2CMutex = xSemaphoreCreateMutex();
  if (xI2CMutex != NULL) {
    Serial.println("[1] I2C Mutex created successfully");
  }
  xjl
  // 2. กำหนดค่าเริ่มต้นให้กับอุปกรณ์ I2C
  Wire.begin();
  keypad.begin();
  lcd.init();
  lcd.backlight();
  lcd.home();
  Serial.println("[2] I2C Devices Initialized");
  
  // 3. อ่านค่าเวลาเริ่มต้นจาก DS1307
  if (RTC.read(tm)) {
    Serial.println("[3] RTC read initial success");
  } else {
    Serial.println("[3] RTC read initial failed, using fallback");
    while(1); // Blocking
  }
  refresh_display();

  // 4. สร้าง FreeRTOS Tasks ด้วย Stack ขนาด 160 words
  BaseType_t t1 = xTaskCreate(Task_Keypad, "KeypadTask", 160, NULL, 2, NULL);
  BaseType_t t2 = xTaskCreate(Task_Clock_Display, "ClockTask", 160, NULL, 1, NULL);
  if (t1 == pdPASS && t2 == pdPASS) {
    Serial.println("[4] Tasks created successfully. Starting Scheduler...");
  } else {
    Serial.println("[FAIL] Out of Memory! Task creation failed.");
  }

  // 5. เริ่มต้นการทำงานของ RTOS Scheduler
  vTaskStartScheduler();
}

void loop() {
  // empty loop
}

// Task 1: จัดการเวลา และควบคุมการแสดงผลบนหน้าจอ LCD
void Task_Clock_Display(void *pvParameters) {
  (void) pvParameters;

  while (1) {
    if (mode == 0) {
      // --- Running Mode: อ่านเวลาจาก RTC และแสดงผลทุก 1 วินาที ---
      if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        if (RTC.read(tm)) {
          refresh_display();
        }
        xSemaphoreGive(xI2CMutex);
      }
      vTaskDelay(pdMS_TO_TICKS(1000));
      
    } else {
      // --- Setting Mode: ทำการกะพริบหลักที่กำลังเลือกแก้ไขทุก 500ms ---
      if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        if (select_time == 1) {
          // กะพริบหลักนาที (ตำแหน่งช่องที่ 10-11 บรรทัดที่ 1)
          lcd.setCursor(10, 1);
          if (isBlink) {
            lcd.print("  ");
          } else {
            print2digitsLCD(tm.Minute);
          }
          isBlink = !isBlink;
        } else {
          // กะพริบหลักชั่วโมง (ตำแหน่งช่องที่ 7-8 บรรทัดที่ 1)
          lcd.setCursor(7, 1);
          if (isBlink) {
            lcd.print("  ");
          } else {
            print2digitsLCD(tm.Hour);
          }
          isBlink = !isBlink;
        }
        xSemaphoreGive(xI2CMutex);
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
}

// Task 2: สแกน Keypad และประมวลผลคำสั่งตั้งเวลา (State Machine)
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
        // [โหมดปกติ] กด 'B' เพื่อเข้าสู่โหมดตั้งเวลา (เริ่มที่ชั่วโมง)
        if (key == 'B') {
          mode = 1;
          select_time = 0; // 0 = ชั่วโมง
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
          // กด '#' เพื่อบันทึกค่าเวลาใหม่ลง DS1307 RTC และกลับสู่โหมด Run
          tm.Second = 0; // รีเซ็ตวินาทีเป็น 00 หลังตั้งเวลาใหม่

          if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            if (RTC.write(tm)) {
              Serial.println("[/] Write New Time Success");
              mode = 0;
              refresh_display();
            } else {
              Serial.println("[!] Write New Time Failed");
            }
            xSemaphoreGive(xI2CMutex);
          }
        }
      }
    }
    // หน่วงเวลาสแกนปุ่ม 40ms (Non-blocking delay สลับให้ Task อื่นทำงาน)
    vTaskDelay(pdMS_TO_TICKS(40));
  }
}
