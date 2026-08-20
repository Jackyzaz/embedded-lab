#include <Arduino_FreeRTOS.h>
#include <semphr.h>

SemaphoreHandle_t xSemaphore;

// ประกาศ Function Prototype เพื่อให้ compiler รู้จักฟังก์ชันล่วงหน้า
void PrintHello(void *pvParameters);
void PrintNumber(void *pvParameters);

void setup() {
  Serial.begin(9600);

  // สร้าง Mutex ใน setup() ก่อนเริ่ม Task เสมอ
  xSemaphore = xSemaphoreCreateMutex();

  if (xSemaphore != NULL) {
    Serial.println("Mutex created successfully");
  }

  // สร้าง Tasks

  xTaskCreate(PrintHello, "Print Hello", 100, NULL, 1, NULL);
  xTaskCreate(PrintNumber, "Print numbers", 100, NULL, 1, NULL);  
  xTaskCreate(PrintNumber, "Print numbers", 100, NULL, 1, NULL);


  vTaskStartScheduler();
}

void PrintHello(void *pvParameters) {
  while (1) {
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
      Serial.println("Hello,PANYAYOTaABCDEFGHIJKLMNOPQRSTUVWXYZ");
      xSemaphoreGive(xSemaphore);
      vTaskDelay(pdMS_TO_TICKS(100)); // หน่วงเวลา 100 ms
    }
  }
}

void PrintNumber(void *pvParameters) {
  while (1) {
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
      Serial.println("1234567890+-* /= == == == == == == == == == == == == == ");
      xSemaphoreGive(xSemaphore);
      vTaskDelay(pdMS_TO_TICKS(100)); // หน่วงเวลา 100 ms
    }
  }
}

void loop() {
  // ว่างไว้สำหรับ FreeRTOS
}
