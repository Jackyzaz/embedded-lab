#include <Arduino_FreeRTOS.h>

#define R_LED 3
#define G_LED 4
#define Y_LED 5
#define O_LED 6
#define PUSH_SW 2
#define VR_PIN A0

volatile int current_mode = 0; //0->1->2->3->0
volatile TickType_t led_delay = pdMS_TO_TICKS(500); // default 500ms

// remember last state
int step_per_mode[4] = {0, 0, 0, 0};

void setLEDs(int r, int g, int y, int o) {
  digitalWrite(R_LED, r);
  digitalWrite(G_LED, g);
  digitalWrite(Y_LED, y);
  digitalWrite(O_LED, o);
}

void Task_LED(void *pvParameters) {
  while (1) {
    int mode = current_mode;
    int step = step_per_mode[mode];

    switch (mode) {
      case 0: // Mode 1: R -> G -> Y -> O -> E
        if      (step == 0) setLEDs(1, 0, 0, 0);
        else if (step == 1) setLEDs(0, 1, 0, 0);
        else if (step == 2) setLEDs(0, 0, 1, 0);
        else if (step == 3) setLEDs(0, 0, 0, 1);
        else if (step == 4) setLEDs(0, 0, 0, 0);
        step++;
        if (step>4) step = 0;
        step_per_mode[0] = step;
        break;

      case 1: // Mode 2: O -> Y -> G -> R -> E
        if      (step == 0) setLEDs(0, 0, 0, 1);
        else if (step == 1) setLEDs(0, 0, 1, 0);
        else if (step == 2) setLEDs(0, 1, 0, 0);
        else if (step == 3) setLEDs(1, 0, 0, 0);
        else if (step == 4) setLEDs(0, 0, 0, 0);
        step++;
        if (step>4) step = 0;
        step_per_mode[1] = step;
        break;

      case 2: // Mode 3: E -> R -> RG -> RGY -> RGYO -> RGY -> RG -> R
        if      (step == 0) setLEDs(0, 0, 0, 0);
        else if (step == 1) setLEDs(1, 0, 0, 0);
        else if (step == 2) setLEDs(1, 1, 0, 0);
        else if (step == 3) setLEDs(1, 1, 1, 0);
        else if (step == 4) setLEDs(1, 1, 1, 1);
        else if (step == 5) setLEDs(1, 1, 1, 0);
        else if (step == 6) setLEDs(1, 1, 0, 0);
        else if (step == 7) setLEDs(1, 0, 0, 0);
        step++;
        if (step>8) step = 0;
        step_per_mode[2] = step;
        break;

      case 3: // Mode 4: E -> O -> OY -> OYG -> OYGR -> OYG -> OY -> O
        if      (step == 0) setLEDs(0, 0, 0, 0);
        else if (step == 1) setLEDs(0, 0, 0, 1);
        else if (step == 2) setLEDs(0, 0, 1, 1);
        else if (step == 3) setLEDs(0, 1, 1, 1);
        else if (step == 4) setLEDs(1, 1, 1, 1);
        else if (step == 5) setLEDs(0, 1, 1, 1);
        else if (step == 6) setLEDs(0, 0, 1, 1);
        else if (step == 7) setLEDs(0, 0, 0, 1);
        step++;
        if (step>8) step = 0;
        step_per_mode[3] = step;
        break;
    }

    vTaskDelay(led_delay);
  }
}

void Task_Read_Switch(void *pvParameters) {
  pinMode(PUSH_SW, INPUT_PULLUP);
  while (1) {
    if (digitalRead(PUSH_SW) == LOW) {
      vTaskDelay(pdMS_TO_TICKS(20)); // Debounce
      if (digitalRead(PUSH_SW) == LOW) {
        current_mode = (current_mode + 1) % 4;
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
    int delay_ms = 1500 - ((adc / 1023.0) * 1450); // max -> 50ms, min = 1500ms
    Serial.println(delay_ms);
    led_delay = pdMS_TO_TICKS(delay_ms);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(Y_LED, OUTPUT);
  pinMode(O_LED, OUTPUT);

  xTaskCreate(Task_LED, "LED_Task", 100, NULL, 1, NULL);
  xTaskCreate(Task_Read_Switch, "SW_Task", 80, NULL, 2, NULL);
  xTaskCreate(Task_Read_VR, "VR_Task", 80, NULL, 1, NULL);

  vTaskStartScheduler();
}

void loop() {}
