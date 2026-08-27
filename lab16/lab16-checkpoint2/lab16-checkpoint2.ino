#include <Arduino_FreeRTOS.h>             // เรียกใช้ FreeRTOS Library

#define R_LED 3                           // กำหนดขา LED สีแดง (Red)
#define G_LED 4                           // กำหนดขา LED สีเขียว (Green)
#define Y_LED 5                           // กำหนดขา LED สีเหลือง (Yellow)
#define O_LED 6                           // กำหนดขา LED สีส้ม (Orange)
#define PUSH_SW 2                         // กำหนดขาปุ่มกด (Switch)
#define VR_PIN A0                         // กำหนดขา Analog อ่านค่า Potentiometer (VR)

int current_mode = 0;                     // โหมดการทำงานปัจจุบัน (วน 0->1->2->3->0)
TickType_t led_delay = pdMS_TO_TICKS(500);// ระยะเวลา Delay ของ LED (ค่าเริ่มต้น 500ms)

void setLEDs(int r, int g, int y, int o) {// ฟังก์ชันช่วยสำหรับเขียนสถานะ LED 4 ดวงพร้อมกัน
  digitalWrite(R_LED, r);                 // สั่งสถานะ LED สีแดง
  digitalWrite(G_LED, g);                 // สั่งสถานะ LED สีเขียว
  digitalWrite(Y_LED, y);                 // สั่งสถานะ LED สีเหลือง
  digitalWrite(O_LED, o);                 // สั่งสถานะ LED สีส้ม
}

void Task_LED(void *pvParameters) {       // Task สำหรับควบคุมลำดับการติด/ดับของ LED
  int step = 0;                           // ลำดับขั้น (step) ของแอนิเมชันในโหมดปัจจุบัน
  int last_mode = -1;                     // เก็บโหมดล่าสุดเพื่อตรวจจับการเปลี่ยนโหมด
  while (1) {                             // วนลูปการทำงานของ Task
    int mode = current_mode;              // อ่านโหมดการทำงานปัจจุบัน
    if (mode != last_mode) {              // ตรวจสอบว่ามีการเปลี่ยนโหมดหรือไม่
      last_mode = mode;                   // อัปเดตโหมดล่าสุด
      step = 0;                           // รีเซ็ตเริ่มลำดับแรกใหม่ทุกครั้งที่เปลี่ยนโหมด
    }

    switch (mode) { // ตรวจสอบรูปแบบการแสดงผลตามโหมด
      case 0:    // โหมด 1: R -> G -> Y -> O -> ดับทั้งหมด (5 Step)
        if      (step == 0) setLEDs(1, 0, 0, 0);  // สเต็ป 0: ติดสีแดง
        else if (step == 1) setLEDs(0, 1, 0, 0);  // สเต็ป 1: ติดสีเขียว
        else if (step == 2) setLEDs(0, 0, 1, 0);  // สเต็ป 2: ติดสีเหลือง
        else if (step == 3) setLEDs(0, 0, 0, 1);  // สเต็ป 3: ติดสีส้ม
        else if (step == 4) setLEDs(0, 0, 0, 0);  // สเต็ป 4: ดับทั้งหมด
        step = (step + 1) % 5;                    // เลื่อนสเต็ปถัดไป (วน 0-4)
        break;

      case 1: // โหมด 2: O -> Y -> G -> R -> ดับทั้งหมด (5 Step)
        if      (step == 0) setLEDs(0, 0, 0, 1);  // สเต็ป 0: ติดสีส้ม
        else if (step == 1) setLEDs(0, 0, 1, 0);  // สเต็ป 1: ติดสีเหลือง
        else if (step == 2) setLEDs(0, 1, 0, 0);  // สเต็ป 2: ติดสีเขียว
        else if (step == 3) setLEDs(1, 0, 0, 0);  // สเต็ป 3: ติดสีแดง
        else if (step == 4) setLEDs(0, 0, 0, 0);  // สเต็ป 4: ดับทั้งหมด
        step = (step + 1) % 5;                    // เลื่อนสเต็ปถัดไป (วน 0-4)
        break;

      case 2: // โหมด 3: ดับ -> R -> RG -> RGY -> RGYO -> RGY -> RG -> R (8 Step)
        if      (step == 0) setLEDs(0, 0, 0, 0);  // สเต็ป 0: ดับทั้งหมด
        else if (step == 1) setLEDs(1, 0, 0, 0);  // สเต็ป 1: ติด R
        else if (step == 2) setLEDs(1, 1, 0, 0);  // สเต็ป 2: ติด R + G
        else if (step == 3) setLEDs(1, 1, 1, 0);  // สเต็ป 3: ติด R + G + Y
        else if (step == 4) setLEDs(1, 1, 1, 1);  // สเต็ป 4: ติดครบทุกดวง (R + G + Y + O)
        else if (step == 5) setLEDs(1, 1, 1, 0);  // สเต็ป 5: ติด R + G + Y
        else if (step == 6) setLEDs(1, 1, 0, 0);  // สเต็ป 6: ติด R + G
        else if (step == 7) setLEDs(1, 0, 0, 0);  // สเต็ป 7: ติด R
        step = (step + 1) % 8;                    // เลื่อนสเต็ปถัดไป (วน 0-7)
        break;

      case 3: // โหมด 4: ดับ -> O -> OY -> OYG -> OYGR -> OYG -> OY -> O (8 Step)
        if      (step == 0) setLEDs(0, 0, 0, 0);  // สเต็ป 0: ดับทั้งหมด
        else if (step == 1) setLEDs(0, 0, 0, 1);  // สเต็ป 1: ติด O
        else if (step == 2) setLEDs(0, 0, 1, 1);  // สเต็ป 2: ติด O + Y
        else if (step == 3) setLEDs(0, 1, 1, 1);  // สเต็ป 3: ติด O + Y + G
        else if (step == 4) setLEDs(1, 1, 1, 1);  // สเต็ป 4: ติดครบทุกดวง (O + Y + G + R)
        else if (step == 5) setLEDs(0, 1, 1, 1);  // สเต็ป 5: ติด O + Y + G
        else if (step == 6) setLEDs(0, 0, 1, 1);  // สเต็ป 6: ติด O + Y
        else if (step == 7) setLEDs(0, 0, 0, 1);  // สเต็ป 7: ติด O
        step = (step + 1) % 8;                    // เลื่อนสเต็ปถัดไป (วน 0-7)
        break;
    }

    vTaskDelay(led_delay);                        // หน่วงเวลาตามความเร็วที่ตั้งไว้
  }
}

void Task_Read_Switch(void *pvParameters) {       // Task สำหรับตรวจจับการกดสวิตช์เพื่อเปลี่ยนโหมด
  pinMode(PUSH_SW, INPUT_PULLUP);                 // กำหนดโหมดขาปุ่มกดเป็น Input แบบ Pull-up
  while (1) {                                     // วนลูปอ่านค่าปุ่มกด
    if (digitalRead(PUSH_SW) == LOW) {            // ตรวจสอบว่ามีการกดปุ่ม (Active LOW)
      vTaskDelay(pdMS_TO_TICKS(10));              // หน่วงเวลา 10ms เพื่อป้องกันสัญญาณกระเพื่อม (Debounce)
      if (digitalRead(PUSH_SW) == LOW) {          // ตรวจสอบอีกครั้งเพื่อยืนยันการกดจริง
        current_mode = (current_mode + 1) % 4;    // เปลี่ยนเป็นโหมดถัดไป (วน 0, 1, 2, 3)
        while (digitalRead(PUSH_SW) == LOW) {     // รอจนกว่าจะปล่อยมือจากปุ่มกด
          vTaskDelay(pdMS_TO_TICKS(10));          // หน่วงเวลา 10ms ขณะรอปล่อยปุ่ม
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));                // หน่วงเวลา 10ms ก่อนรอบการตรวจจับถัดไป
  }
}

void Task_Read_VR(void *pvParameters) {           // Task สำหรับอ่านค่า Potentiometer (VR)
  while (1) {                                     // วนลูปอ่านค่า Analog
    int adc = analogRead(VR_PIN);                 // อ่านค่าแรงดันจากขา A0 (ช่วง 0 - 1023)
    int delay_ms = 1500 - ((adc / 1023.0) * 1450);// แปลงค่า ADC เป็นช่วงเวลา 50ms (หมุนสุด) ถึง 1500ms (หมุนต่ำสุด)
    Serial.println(delay_ms);                     // พิมพ์ค่า delay (ms) ออก Serial Monitor
    led_delay = pdMS_TO_TICKS(delay_ms);          // แปลงหน่วยเป็น Tick ของ FreeRTOS และอัปเดตลงตัวแปรโกลบอล
    vTaskDelay(pdMS_TO_TICKS(100));               // หน่วงเวลา 100ms ก่อนอ่านค่ารอบถัดไป
  }
}

void setup() {
  Serial.begin(9600);                       // เริ่มต้นใช้งาน Serial ที่ baud rate 9600
  pinMode(R_LED, OUTPUT);                   // กำหนดขา LED สีแดงเป็น Output
  pinMode(G_LED, OUTPUT);                   // กำหนดขา LED สีเขียวเป็น Output
  pinMode(Y_LED, OUTPUT);                   // กำหนดขา LED สีเหลืองเป็น Output
  pinMode(O_LED, OUTPUT);                   // กำหนดขา LED สีส้มเป็น Output

  xTaskCreate(Task_LED, "LED_Task", 100, NULL, 1, NULL);        // สร้าง Task ควบคุม LED (Priority 1)
  xTaskCreate(Task_Read_Switch, "SW_Task", 80, NULL, 2, NULL);  // สร้าง Task อ่าน Switch (Priority 2)
  xTaskCreate(Task_Read_VR, "VR_Task", 80, NULL, 1, NULL);      // สร้าง Task อ่าน Potentiometer (Priority 1)

  vTaskStartScheduler();                    // เริ่มต้นการทำงานของ FreeRTOS Scheduler
}

void loop() {
  // Empty Loop
}
