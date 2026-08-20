#include <Arduino_FreeRTOS.h>

#define R_LED 3
#define G_LED 4
#define Y_LED 5
#define O_LED 6
#define PUSH_SW 2
#define VR_PIN A0

TaskHandle_t TaskHandle_Mode[4];
volatile int current_mode = 0;
volatile TickType_t led_delay = pdMS_TO_TICKS(500);

void setLEDs(bool r, bool g, bool y, bool o) {
  digitalWrite(R_LED, r ? HIGH : LOW);
  digitalWrite(G_LED, g ? HIGH : LOW);
  digitalWrite(Y_LED, y ? HIGH : LOW);
  digitalWrite(O_LED, o ? HIGH : LOW);
}

// Mode 1: R -> G -> Y -> O -> E
void Task_Mode1(void *pvParameters) {
  while (1) {
    setLEDs(1, 0, 0, 0); vTaskDelay(led_delay);
    setLEDs(0, 1, 0, 0); vTaskDelay(led_delay);
    setLEDs(0, 0, 1, 0); vTaskDelay(led_delay);
    setLEDs(0, 0, 0, 1); vTaskDelay(led_delay);
    setLEDs(0, 0, 0, 0); vTaskDelay(led_delay);
  }
}

// Mode 2: O -> Y -> G -> R -> E
void Task_Mode2(void *pvParameters) {
  while (1) {
    setLEDs(0, 0, 0, 1); vTaskDelay(led_delay);
    setLEDs(0, 0, 1, 0); vTaskDelay(led_delay);
    setLEDs(0, 1, 0, 0); vTaskDelay(led_delay);
    setLEDs(1, 0, 0, 0); vTaskDelay(led_delay);
    setLEDs(0, 0, 0, 0); vTaskDelay(led_delay);
  }
}

// Mode 3: E -> R -> RG -> RGY -> RGYO -> RGY -> RG -> R
void Task_Mode3(void *pvParameters) {
  while (1) {
    setLEDs(0, 0, 0, 0); vTaskDelay(led_delay);
    setLEDs(1, 0, 0, 0); vTaskDelay(led_delay);
    setLEDs(1, 1, 0, 0); vTaskDelay(led_delay);
    setLEDs(1, 1, 1, 0); vTaskDelay(led_delay);
    setLEDs(1, 1, 1, 1); vTaskDelay(led_delay);
    setLEDs(1, 1, 1, 0); vTaskDelay(led_delay);
    setLEDs(1, 1, 0, 0); vTaskDelay(led_delay);
    setLEDs(1, 0, 0, 0); vTaskDelay(led_delay);
  }
}

// Mode 4: E -> O -> OY -> OYG -> OYGR -> OYG -> OY -> O
void Task_Mode4(void *pvParameters) {
  while (1) {
    setLEDs(0, 0, 0, 0); vTaskDelay(led_delay);
    setLEDs(0, 0, 0, 1); vTaskDelay(led_delay);
    setLEDs(0, 0, 1, 1); vTaskDelay(led_delay);
    setLEDs(0, 1, 1, 1); vTaskDelay(led_delay);
    setLEDs(1, 1, 1, 1); vTaskDelay(led_delay);
    setLEDs(0, 1, 1, 1); vTaskDelay(led_delay);
    setLEDs(0, 0, 1, 1); vTaskDelay(led_delay);
    setLEDs(0, 0, 0, 1); vTaskDelay(led_delay);
  }
}

void Task_Read_Switch(void *pvParameters) {
  pinMode(PUSH_SW, INPUT_PULLUP);
  while (1) {
    if (digitalRead(PUSH_SW) == LOW) {
      vTaskDelay(pdMS_TO_TICKS(20)); // Debounce
      if (digitalRead(PUSH_SW) == LOW) {
        int prev_mode = current_mode;
        current_mode = (current_mode + 1) % 4;

        vTaskSuspend(TaskHandle_Mode[prev_mode]);
        vTaskResume(TaskHandle_Mode[current_mode]);

        while (digitalRead(PUSH_SW) == LOW) {
          vTaskDelay(pdMS_TO_TICKS(10));
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void Task_Read_VR(void *pvParameters) {
  while (1) {
    int adc = analogRead(VR_PIN);
    int delay_ms = 1500 - ((long)adc * 1450 / 1023);
    led_delay = pdMS_TO_TICKS(delay_ms);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(Y_LED, OUTPUT);
  pinMode(O_LED, OUTPUT);

  xTaskCreate(Task_Mode1, "M1", 64, NULL, 1, &TaskHandle_Mode[0]);
  xTaskCreate(Task_Mode2, "M2", 64, NULL, 1, &TaskHandle_Mode[1]);
  xTaskCreate(Task_Mode3, "M3", 64, NULL, 1, &TaskHandle_Mode[2]);
  xTaskCreate(Task_Mode4, "M4", 64, NULL, 1, &TaskHandle_Mode[3]);

  xTaskCreate(Task_Read_Switch, "SW", 70, NULL, 2, NULL);
  xTaskCreate(Task_Read_VR, "VR", 70, NULL, 1, NULL);

  // only mode 1 start
  vTaskSuspend(TaskHandle_Mode[1]);
  vTaskSuspend(TaskHandle_Mode[2]);
  vTaskSuspend(TaskHandle_Mode[3]);

  vTaskStartScheduler();
}

void loop() {}
