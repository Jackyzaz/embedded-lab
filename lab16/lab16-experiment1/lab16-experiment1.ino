#include <Arduino_FreeRTOS.h>
#define R_LED 3
#define G_LED 4
void setup() {
  xTaskCreate(Display_R_LED, "Red LED Task", 128, NULL, 1, NULL);
  xTaskCreate(Display_G_LED, "Green LED Task", 128, NULL, 1, NULL);
  vTaskStartScheduler();
}
void Display_R_LED(void *pvParameters)
{
  pinMode(R_LED, OUTPUT);
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(500)); //หน่วงเวลา 500 mS
    digitalWrite(R_LED, digitalRead(R_LED) ^ 1);
  }
}
void Display_G_LED(void *pvParameters)
{
  pinMode(G_LED, OUTPUT);
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000)); //หน่วงเวลา 1000 mS
    digitalWrite(G_LED, digitalRead(G_LED) ^ 1);
  }
}
void loop()
{
}
