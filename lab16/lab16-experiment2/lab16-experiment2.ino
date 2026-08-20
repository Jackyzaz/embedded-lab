#include <Arduino_FreeRTOS.h>
#define R_LED 3
#define G_LED 4
#define PUSH_SW 2
int v_delay;
void setup() {
  Serial.begin(9600); // setup serial
  xTaskCreate(Read_Poten, "Read Potentiometer", 128, NULL, 1, NULL);
  xTaskCreate(Read_Switch, "Read push button switch", 128, NULL, 1, NULL);
  xTaskCreate(Display_R_LED, "Red LED Task", 128, NULL, 1, NULL);
  xTaskCreate(Display_G_LED, "Green LED Task", 128, NULL, 1, NULL);
  vTaskStartScheduler();
}
void Display_R_LED(void *pvParameters)
{
  pinMode(R_LED, OUTPUT);
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(500));
    digitalWrite(R_LED, digitalRead(R_LED) ^ 1);
  }
}
void Display_G_LED(void *pvParameters)
{
  pinMode(G_LED, OUTPUT);
  while (1)
  {
    vTaskDelay(v_delay);
    digitalWrite(G_LED, digitalRead(G_LED) ^ 1);
  }
}
void Read_Poten(void *pvParameters)
{
  while (1)
  {
    int val = analogRead(A0); // read the input pin
    Serial.println(val);
  }
}
void Read_Switch(void *pvParameters)
{
  pinMode(PUSH_SW, INPUT);
  const TickType_t delay_0300ms = pdMS_TO_TICKS(300);
  const TickType_t delay_1000ms = pdMS_TO_TICKS(1000);
  while (1)
  {
    int sw_status = digitalRead(PUSH_SW);
//    Serial.println(sw_status);
    if (sw_status == LOW)
      v_delay = delay_0300ms;
    else
      v_delay = delay_1000ms;
  }
}
void loop() {}
