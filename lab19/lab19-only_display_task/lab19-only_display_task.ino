#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
tmElements_t tm;

SemaphoreHandle_t xI2CSemaphore;

// Function Prototypes
void Task_Display_Clock(void *pvParameters);
void print2digits(int number);
void print2digitsLCD(int number);

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

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("--- [Lab 19: Only Display Task Test] ---"));
  Serial.println(F("========================================"));

  // 1. สร้าง Mutex
  xI2CSemaphore = xSemaphoreCreateMutex();
  if (xI2CSemaphore != NULL) {
    Serial.println(F("[1] I2C Semaphore created successfully"));
  }

  // 2. เริ่มต้น I2C และ LCD
  Wire.begin();
  #if defined(WIRE_HAS_TIMEOUT)
    Wire.setWireTimeout(25000, true); // ป้องกัน I2C Bus Hang
  #endif

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("RTOS Clock Test");
  Serial.println(F("[2] LCD Initialized"));

  // 3. ทดสอบอ่าน RTC ครั้งแรก
  if (RTC.read(tm)) {
    Serial.println(F("[3] RTC Read Initial Success"));
  } else {
    Serial.println(F("[3] RTC Read Failed"));
  }

  // 4. เพิ่ม Stack Size เป็น 160 words (320 bytes) เพื่อรองรับ Deep Call Stack ของ LiquidCrystal_I2C + Wire
  BaseType_t result = xTaskCreate(
    Task_Display_Clock,
    "DisplayTask",
    160,          // ขยาย Stack เป็น 160 words ป้องกัน Stack Overflow
    NULL,
    1,            // Priority 1
    NULL
  );

  if (result == pdPASS) {
    Serial.println(F("[4] Task created (Stack: 160 words). Starting Scheduler..."));
  } else {
    Serial.println(F("[FAIL] Task creation failed"));
  }

  // 5. เริ่ม Scheduler
  vTaskStartScheduler();
}

void loop() {
  // หากหลุดมาตรงนี้ แสดงว่า Scheduler ไม่เริ่มทำงาน
}

// -------------------------------------------------------------
// Task: อ่านค่าเวลาจาก DS1307 และแสดงผลออก LCD / Serial
// -------------------------------------------------------------
void Task_Display_Clock(void *pvParameters) {
  (void) pvParameters;

  while (1) {
    // ขอ Semaphore เพื่อจองบัส I2C
    if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(200)) == pdTRUE) {

      if (RTC.read(tm)) {
        // ส่งค่าออก Serial Monitor
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

        // ตรวจสอบ Stack ที่เหลืออยู่ (Stack Watermark)
        UBaseType_t stackLeft = uxTaskGetStackHighWaterMark(NULL);
        Serial.print(F(" | Stack Free: "));
        Serial.print(stackLeft);
        Serial.println(F(" words"));

        // แสดงผลบนจอ LCD
        lcd.setCursor(0, 0);
        lcd.print(F("D/M/Y= "));
        lcd.print(tm.Day);
        lcd.print(F("/"));
        lcd.print(tm.Month);
        lcd.print(F("/"));
        lcd.print(tmYearToCalendar(tm.Year));
        lcd.print(F("  "));

        lcd.setCursor(0, 1);
        lcd.print(F("Time = "));
        print2digitsLCD(tm.Hour);
        lcd.print(F(":"));
        print2digitsLCD(tm.Minute);
        lcd.print(F("."));
        print2digitsLCD(tm.Second);
        lcd.print(F("  "));
      } else {
        Serial.println(F("[WARN] RTC Read Failed in Task!"));
        lcd.setCursor(0, 0);
        lcd.print(F("RTC Read Error! "));
      }

      // คืน Semaphore
      xSemaphoreGive(xI2CSemaphore);
    } else {
      Serial.println(F("[WARN] Semaphore timeout!"));
    }

    // หน่วงเวลา 1000ms เพื่อให้ระบบสลับงาน
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
