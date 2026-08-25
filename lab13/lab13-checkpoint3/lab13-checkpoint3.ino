#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>

#define I2CADDR 0x20

const byte ROWS = 4;
const byte COLS = 4;

// กำหนดคีย์แมปตามโจทย์
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', '+'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {7, 6, 5, 4};
byte colPins[COLS] = {3, 2, 1, 0};

Keypad_I2C keypad( makeKeymap(hexaKeys), rowPins, 
                  colPins, ROWS, COLS, I2CADDR);

// ตัวแปรสำหรับเครื่องคิดเลข
long num1 = 0;
long num2 = 0;
char op = 0;
bool entering_num2 = false;

void setup() {
  Wire.begin();
  keypad.begin();
  Serial.begin(38400);
  Serial.println("--- Simple Calculator Ready ---");
}

void loop() {
  char key = keypad.getKey();
  
  if (key != NO_KEY) {
    // 1. รับค่าตัวเลข ('0' - '9') สูงสุดไม่เกิน 2 หลัก (0 - 99)
    if (key >= '0' && key <= '9') {
      if (!entering_num2) {
        if (num1 <= 9) { // ตรวจว่าเกิน 2 หลักแล้วยัง
          num1 = num1 * 10 + (key - '0');
          Serial.print(key);
        }
      } else {
        if (num2 <= 9) { // ตรวจว่าเกิน 2 หลักแล้วยัง
          num2 = num2 * 10 + (key - '0');
          Serial.print(key);
        }
      }
    }
    // 2. รับเครื่องหมายคำนวณ ('+' หรือ '*')
    else if (key == '+' || key == '*') {
      op = key;
      entering_num2 = true;
      Serial.print(op);
    }
    // 3. ปุ่ม '#' ทำหน้าที่แทน '=' เพื่อคำนวณและแสดงผลลัพธ์
    else if (key == '#') {
      if (entering_num2 && op != 0) {
        Serial.print("=");
        long result = 0;
        if (op == '+') {
          result = num1 + num2;
        } else if (op == '*') {
          result = num1 * num2;
        }
        Serial.println(result);

        // รีเซ็ตค่า เตรียมคำนวณในรอบถัดไป
        num1 = 0;
        num2 = 0;
        op = 0;
        entering_num2 = false;
      }
    }
    // 4. ปุ่ม 'C' สำหรับล้างค่า (Clear)
    else if (key == 'C') {
      num1 = 0;
      num2 = 0;
      op = 0;
      entering_num2 = false;
      Serial.println();
      Serial.println("--- Clear ---");
    }
  }
}
