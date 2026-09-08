#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

// อุปกรณ์ I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);
tmElements_t tm;

// Mutex สำหรับปกป้อง Shared Resources ทั้ง 2 ชิ้น
SemaphoreHandle_t xMutex_LCD = NULL;
SemaphoreHandle_t xMutex_RTC = NULL;

// Function Prototypes สำหรับ Tasks
void Task_Display(void *pvParameters);
void Task_RTCLogger(void *pvParameters);

void setup() {
  Serial.begin(9600);
  while (!Serial); // รอ Serial พร้อมใช้งาน

  // กำหนดค่าเริ่มต้นให้กับบัส I2C และหน้าจอ LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Deadlock Demo..."));

  // 1. สร้าง Mutex ทั้งสองตัว
  xMutex_LCD = xSemaphoreCreateMutex();
  xMutex_RTC = xSemaphoreCreateMutex();

  if (xMutex_LCD != NULL && xMutex_RTC != NULL) {
    Serial.println(F("[INIT] Mutex_LCD and Mutex_RTC created successfully."));
  } else {
    Serial.println(F("[ERROR] Failed to create Mutexes. System halted."));
    while (1);
  }

  // 2. สร้าง Tasks เพื่อจำลอง Deadlock
  // ให้ Priority เท่ากัน (Priority 1) เพื่อให้ Scheduler สลับการทำงานได้
  xTaskCreate(Task_Display,   "Task_Display",   140, NULL, 1, NULL);
  xTaskCreate(Task_RTCLogger, "Task_RTCLogger", 140, NULL, 1, NULL);
  
  // 3. เริ่มต้นการทำงานของ RTOS Scheduler
  vTaskStartScheduler();
}

void loop() {
  // empty loop
}

void Task_Display(void *pvParameters) {
  (void) pvParameters;

  while (1) {    
    // 1. ขอสิทธิ์ใช้งาน LCD ด้วย portMAX_DELAY (รอจนกว่าจะได้)
    if (xSemaphoreTake(xMutex_LCD, portMAX_DELAY) == pdTRUE) {
      Serial.println(F("[Task A: Display] >> LOCKED LCD Mutex successfully."));
      
      // จำลองการประมวลผล / หน่วงเวลาเพื่อให้ Task B ได้โอกาส Run และ Lock RTC Mutex
      Serial.println(F("[Task A: Display] Processing LCD draw... Delaying 50ms"));
      vTaskDelay(pdMS_TO_TICKS(50));
      
      // 2. พยายามขอสิทธิ์เข้าถึง RTC เพื่ออ่านค่าเวลามาอัปเดตบน LCD
      Serial.println(F("[Task A: Display] Now requesting RTC Mutex (Waiting...)..."));
      if (xSemaphoreTake(xMutex_RTC, portMAX_DELAY) == pdTRUE) {
        // บล็อกโค้ดนี้จะไม่มีวันได้ทำงานเมื่อเกิด Deadlock
        Serial.println(F("[Task A: Display] >> LOCKED RTC Mutex. Updating display..."));
        xSemaphoreGive(xMutex_RTC);
      }
      xSemaphoreGive(xMutex_LCD);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


void Task_RTCLogger(void *pvParameters) {
  (void) pvParameters;
  while (1) {
    // 1. ขอสิทธิ์ใช้งาน RTC ด้วย portMAX_DELAY
    if (xSemaphoreTake(xMutex_RTC, portMAX_DELAY) == pdTRUE) {
      Serial.println(F("[Task B: Logger]  >> LOCKED RTC Mutex successfully."));
      // อ่านค่า RTC
      RTC.read(tm);
      // หน่วงเวลาสั้นๆ เพื่อให้ Task A สลับขึ้นมาทำงานและพยายามขอ RTC Mutex
      Serial.println(F("[Task B: Logger]  Working with RTC... Delaying 50ms"));
      vTaskDelay(pdMS_TO_TICKS(50));

      // 2. พยายามขอสิทธิ์ LCD เพื่อพิมพ์ข้อความสถานะ
      // ในขณะนี้ LCD ถูก Task A ถือครองอยู่ และ Task A ก็กำลังรอ RTC จาก Task B
      Serial.println(F("[Task B: Logger]  Now requesting LCD Mutex (Waiting...)..."));
      if (xSemaphoreTake(xMutex_LCD, portMAX_DELAY) == pdTRUE) {
        // บล็อกโค้ดนี้จะไม่มีวันได้ทำงานเมื่อเกิด Deadlock
        Serial.println(F("[Task B: Logger]  >> LOCKED LCD Mutex."));
        xSemaphoreGive(xMutex_LCD);
      }
      xSemaphoreGive(xMutex_RTC);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}