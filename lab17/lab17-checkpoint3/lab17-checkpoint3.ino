#include <Arduino.h>
#include "my_EEPROM.h"
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>

#define I2CADDR 0x20

const byte ROWS = 4;
const byte COLS = 4;

// Keypad mapping
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'E', '0', 'F', 'D'},
};

byte rowPins[ROWS] = {7, 6, 5, 4};
byte colPins[COLS] = {3, 2, 1, 0};

Keypad_I2C keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, I2CADDR);

// TM1638 Pins
const int strobe = 4;
const int clock = 3;
const int data = 2;

// Button Masks
const uint8_t SW1_MASK = 0x01; // S1
const uint8_t SW2_MASK = 0x02; // S2
const uint8_t SW3_MASK = 0x04; // S3
const uint8_t SW4_MASK = 0x08; // S4
const uint8_t SW5_MASK = 0x10; // S5
const uint8_t SW6_MASK = 0x20; // S6
const uint8_t SW7_MASK = 0x40; // S7
const uint8_t SW8_MASK = 0x80; // S8

uint8_t lastButtons = 0;

// 7-Segment Font Table (0-9, A-F)
const uint8_t digitFont[16] = {
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F, // 9
  0x77, // A
  0x7C, // b
  0x39, // C
  0x5E, // d
  0x79, // E
  0x71  // F
};

// State Machine Definition (ST1 to ST6)
enum stateType { ST1, ST2, ST3, ST4, ST5, ST6 };
stateType currentState = ST1;

// Sub-states for interactive input
enum subStateType { SUB_ADDR, SUB_DATA, SUB_COUNT };
subStateType subState = SUB_ADDR;

// Input Variables
uint16_t input_address = 0;
uint8_t data_input = 0;
uint8_t erase_count = 0;

// ST4 specific variables
uint8_t st4_nibble_count = 0;
uint8_t st4_nibble1 = 0;
bool st4_first_byte = true;

// Blinking Engine (Non-blocking)
unsigned long lastBlinkTime = 0;
bool blinkState = true;
const unsigned long BLINK_INTERVAL = 500; // ms

// Function Declarations
int scan_EEPROM(void);
void sendCommand(uint8_t value);
void reset(void);
uint8_t readTM1638_Buttons(void);
void updateDisplay(uint8_t digits[8]);
void handleBlink(void);
void handleButtons(void);
void handleKeypad(void);
void renderDisplay(void);
void I2C_bus_scan(void);
int8_t getHexVal(char key);

void setup() {
  // ตั้งค่าขาควบคุมของ TM1638 ให้เป็น OUTPUT
  pinMode(strobe, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);
  digitalWrite(strobe, HIGH);
  digitalWrite(clock, HIGH);

  Wire.begin();
  keypad.begin();
  Serial.begin(38400);

  I2C_bus_scan();

  currentState = ST1;
  subState = SUB_ADDR;
  sendCommand(0x8F);  // Activate 7-segment display and set brightness
  reset();
}

void loop() {
  handleBlink();
  handleButtons();
  handleKeypad();

  // จัดการ State ที่ประมวลผลคำสั่ง Serial ทันที
  if (currentState == ST6) {
    Serial.println();
    Serial.println("========================================");
    Serial.println("State ST6: Display and Scan EEPROM Data");
    Serial.println("========================================");
    display_all_data_in_EEPROM();
    scan_EEPROM();
    Serial.println("========================================");
    Serial.println("ST6 Done. Returning to ST1.");
    Serial.println("========================================");
    currentState = ST1;
    reset();
  }

  renderDisplay();
}

// 
// Blinking Engine
// 
void handleBlink() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
    lastBlinkTime = currentMillis;
    blinkState = !blinkState;
  }
}

// 
// Helper: แปลง Keypad char เป็น Hex Value (0-15)
// 
int8_t getHexVal(char key) {
  if (key >= '0' && key <= '9') return (key - '0');
  if (key >= 'A' && key <= 'F') return (key - 'A' + 10);
  return -1;
}

// 
// Display Renderer (7-Segment)
// 
void renderDisplay() {
  uint8_t digits[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  switch (currentState) {
    case ST1:
    case ST6:
      // ดับ 7-Segment ทุกหลัก
      break;

    case ST2:
    case ST3:
      if (subState == SUB_ADDR) {
        // 4 หลักซ้ายกระพริบ แสดง Address ฐาน 10
        if (blinkState) {
          digits[0] = digitFont[(input_address / 1000) % 10];
          digits[1] = digitFont[(input_address / 100) % 10];
          digits[2] = digitFont[(input_address / 10) % 10];
          digits[3] = digitFont[input_address % 10];
        }
        // 2 หลักขวาสุดแสดง Data ฐาน 16
        digits[6] = digitFont[(data_input >> 4) & 0x0F];
        digits[7] = digitFont[data_input & 0x0F];
      } else { // SUB_DATA
        // 4 หลักซ้ายติดค้าง
        digits[0] = digitFont[(input_address / 1000) % 10];
        digits[1] = digitFont[(input_address / 100) % 10];
        digits[2] = digitFont[(input_address / 10) % 10];
        digits[3] = digitFont[input_address % 10];
        // 2 หลักขวาสุดกระพริบ แสดง Data ฐาน 16
        if (blinkState) {
          digits[6] = digitFont[(data_input >> 4) & 0x0F];
          digits[7] = digitFont[data_input & 0x0F];
        }
      }
      break;

    case ST4:
      if (subState == SUB_ADDR) {
        // ขั้นที่ 1: 4 หลักซ้ายกระพริบ แสดงแอดเดรสเริ่มต้น ฐาน 10, 4 หลักขวาดับ
        if (blinkState) {
          digits[0] = digitFont[(input_address / 1000) % 10];
          digits[1] = digitFont[(input_address / 100) % 10];
          digits[2] = digitFont[(input_address / 10) % 10];
          digits[3] = digitFont[input_address % 10];
        }
      } else { // SUB_DATA
        // 4 หลักซ้ายติดค้าง แสดงตำแหน่งปัจจุบัน
        digits[0] = digitFont[(input_address / 1000) % 10];
        digits[1] = digitFont[(input_address / 100) % 10];
        digits[2] = digitFont[(input_address / 10) % 10];
        digits[3] = digitFont[input_address % 10];

        // 2 หลักขวา
        if (st4_first_byte && st4_nibble_count == 0) {
          // ยังไม่ได้เริ่มพิมพ์ไบต์แรก 2 หลักขวาดับ
          digits[6] = 0;
          digits[7] = 0;
        } else if (st4_nibble_count == 1) {
          // พิมพ์หลักแรก (หลัก 7 แสดงค่า, หลัก 6 ดับ)
          digits[6] = 0;
          digits[7] = digitFont[st4_nibble1 & 0x0F];
        } else {
          // พิมพ์ครบ 2 หลัก บันทึกแล้ว
          digits[6] = digitFont[(data_input >> 4) & 0x0F];
          digits[7] = digitFont[data_input & 0x0F];
        }
      }
      break;

    case ST5:
      if (subState == SUB_ADDR) {
        // 4 หลักซ้ายกระพริบ แสดง Start Address ฐาน 10
        if (blinkState) {
          digits[0] = digitFont[(input_address / 1000) % 10];
          digits[1] = digitFont[(input_address / 100) % 10];
          digits[2] = digitFont[(input_address / 10) % 10];
          digits[3] = digitFont[input_address % 10];
        }
        // 2 หลักขวาสุดแสดง Count ฐาน 10
        digits[6] = digitFont[(erase_count / 10) % 10];
        digits[7] = digitFont[erase_count % 10];
      } else { // SUB_COUNT
        // 4 หลักซ้ายติดค้าง
        digits[0] = digitFont[(input_address / 1000) % 10];
        digits[1] = digitFont[(input_address / 100) % 10];
        digits[2] = digitFont[(input_address / 10) % 10];
        digits[3] = digitFont[input_address % 10];
        // 2 หลักขวาสุดกระพริบ แสดง Count ฐาน 10
        if (blinkState) {
          digits[6] = digitFont[(erase_count / 10) % 10];
          digits[7] = digitFont[erase_count % 10];
        }
      }
      break;
  }

  updateDisplay(digits);
}

void updateDisplay(uint8_t digits[8]) {
  sendCommand(0x40); // Set auto-increment mode
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xC0);
  for (uint8_t i = 0; i < 8; i++) {
    shiftOut(data, clock, LSBFIRST, digits[i]); // 7-Segment digit
    shiftOut(data, clock, LSBFIRST, 0x00);      // Discrete LED (OFF)
  }
  digitalWrite(strobe, HIGH);
}

// 
// Buttons Handlers (TM1638 S1 - S8)
// 
void handleButtons() {
  uint8_t currentButtons = readTM1638_Buttons();
  uint8_t pressed = currentButtons & (~lastButtons); // Edge Detection (กดครั้งแรก)
  lastButtons = currentButtons;
  if (pressed == 0) return;

  bool s1_pressed = (pressed & SW1_MASK);
  bool s2_pressed = (pressed & SW2_MASK);
  bool s3_pressed = (pressed & SW3_MASK);
  bool s4_pressed = (pressed & SW4_MASK);
  bool s6_pressed = (pressed & SW6_MASK);
  bool s8_pressed = (pressed & SW8_MASK);

  // ปุ่ม S1: เข้า ST2 จาก ST1 หรือสั่งบันทึก EEPROM เมื่ออยู่ใน ST2 / ST3
  if (s1_pressed) {
    if (currentState == ST1) {
      currentState = ST2;
      subState = SUB_ADDR;
      input_address = 0;
      data_input = 0;
      Serial.println("\n[State] -> ST2: Write 1 Byte to EEPROM");
    } else if (currentState == ST2) {
      // เมื่ออยู่ใน ST2 กด S1 เพื่อสั่งเขียนลง EEPROM ทันที
      Serial.print("ST2: Writing 0x");
      if (data_input < 0x10) Serial.print("0");
      Serial.print(data_input, HEX);
      Serial.print(" to EEPROM Address: ");
      Serial.println(input_address);

      EEPROM_Erase_and_Write1B(input_address % 1024, data_input);
      Serial.println("ST2: Write completed! Returning to ST1.");
      input_address = 0;
      data_input = 0;
      currentState = ST1;
      reset();
    } else if (currentState == ST3) {
      // เมื่ออยู่ใน ST3 กด S1 เพื่อสั่งเขียน 1 Page (16 ไบต์) ลง EEPROM ทันที
      Serial.print("ST3: Writing 1 Page (16 bytes) of 0x");
      if (data_input < 0x10) Serial.print("0");
      Serial.print(data_input, HEX);
      Serial.print(" starting at Address: ");
      Serial.println(input_address);

      for (uint16_t i = 0; i < 16; i++) {
        uint16_t target_addr = (input_address + i) % 1024;
        EEPROM_Erase_and_Write1B(target_addr, data_input);
      }
      Serial.println("ST3: Page write completed! Returning to ST1.");
      input_address = 0;
      data_input = 0;
      currentState = ST1;
      reset();
    }
  }

  // ปุ่ม S2: เข้า ST3 (เขียนค่าเดียวลง 1 Page / 16 ไบต์)
  if (s2_pressed) {
    if (currentState == ST1) {
      currentState = ST3;
      subState = SUB_ADDR;
      input_address = 0;
      data_input = 0;
      Serial.println("\n[State] -> ST3: Write 1 Page (16 Bytes) to EEPROM");
    }
  }

  // ปุ่ม S3: เข้า ST4 (เขียนหลายค่าต่อเนื่อง)
  if (s3_pressed) {
    if (currentState == ST1) {
      currentState = ST4;
      subState = SUB_ADDR;
      input_address = 0;
      data_input = 0;
      st4_nibble_count = 0;
      st4_first_byte = true;
      Serial.println("\n[State] -> ST4: Sequential Write to EEPROM");
    }
  }

  // ปุ่ม S4: เข้า ST5 (ลบค่า xx ตำแหน่ง)
  if (s4_pressed) {
    if (currentState == ST1) {
      currentState = ST5;
      subState = SUB_ADDR;
      input_address = 0;
      erase_count = 0;
      Serial.println("\n[State] -> ST5: Erase xx Locations in EEPROM");
    }
  }

  // ปุ่ม S6: ออกจากสถานะและกลับสู่ ST1 (ดับแอลอีดี 7 ส่วนทุกดวง)
  if (s6_pressed) {
    if (currentState != ST1) {
      Serial.println("\n[State] -> Return to ST1 (All Displays OFF)");
      currentState = ST1;
      input_address = 0;
      data_input = 0;
      erase_count = 0;
      st4_nibble_count = 0;
      st4_first_byte = true;
      reset();
    }
  }

  // ปุ่ม S8: ปุ่มยืนยัน / เปลี่ยนโหมดย่อย / สั่งบันทึก
  if (s8_pressed) {
    switch (currentState) {
      case ST1:
        // จาก ST1 กด S8 ไป ST6 แสดงข้อมูลออก Serial
        currentState = ST6;
        break;

      case ST2:
        if (subState == SUB_ADDR) {
          subState = SUB_DATA;
          data_input = 0;
          Serial.println("ST2: Switch to Data entry (Hex):");
        } else if (subState == SUB_DATA) {
          subState = SUB_ADDR;
          Serial.println("ST2: Switch back to Address entry (Dec):");
        }
        break;

      case ST3:
        if (subState == SUB_ADDR) {
          subState = SUB_DATA;
          data_input = 0;
          Serial.println("ST3: Switch to Data entry (Hex):");
        } else if (subState == SUB_DATA) {
          subState = SUB_ADDR;
          Serial.println("ST3: Switch back to Start Address entry (Dec):");
        }
        break;

      case ST4:
        if (subState == SUB_ADDR) {
          subState = SUB_DATA;
          data_input = 0;
          st4_nibble_count = 0;
          st4_first_byte = true;
          Serial.print("ST4: Start Address set to ");
          Serial.print(input_address);
          Serial.println(". Ready to input data via keypad (2 digits each):");
        }
        break;

      case ST5:
        if (subState == SUB_ADDR) {
          subState = SUB_COUNT;
          erase_count = 0;
          Serial.println("ST5: Start Address confirmed. Enter Erase Count (00-99):");
        } else if (subState == SUB_COUNT) {
          Serial.print("ST5: Erasing ");
          Serial.print(erase_count);
          Serial.print(" bytes starting from Address: ");
          Serial.println(input_address);

          for (uint16_t i = 0; i < erase_count; i++) {
            uint16_t target_addr = (input_address + i) % 1024;
            EEPROM_Erase_only(target_addr);
          }
          Serial.println("ST5: Erase completed! Returning to ST1.");
          input_address = 0;
          erase_count = 0;
          currentState = ST1;
          reset();
        }
        break;

      default:
        break;
    }
  }
}

// 
// Keypad Input Handler (4x4 Matrix)
// 
void handleKeypad() {
  char key = keypad.getKey();
  if (key == NO_KEY) return;

  switch (currentState) {
    case ST2:
      if (subState == SUB_ADDR) {
        if (key >= '0' && key <= '9') {
          input_address = (input_address * 10 + (key - '0')) % 10000;
          Serial.print("ST2 Address: ");
          Serial.println(input_address);
        }
      } else if (subState == SUB_DATA) {
        int8_t hexVal = getHexVal(key);
        if (hexVal >= 0) {
          data_input = ((data_input << 4) | (uint8_t)hexVal) & 0xFF;
          Serial.print("ST2 Data: 0x");
          if (data_input < 0x10) Serial.print("0");
          Serial.println(data_input, HEX);
        }
      }
      break;

    case ST3:
      if (subState == SUB_ADDR) {
        if (key >= '0' && key <= '9') {
          input_address = (input_address * 10 + (key - '0')) % 10000;
          Serial.print("ST3 Start Address: ");
          Serial.println(input_address);
        }
      } else if (subState == SUB_DATA) {
        int8_t hexVal = getHexVal(key);
        if (hexVal >= 0) {
          data_input = ((data_input << 4) | (uint8_t)hexVal) & 0xFF;
          Serial.print("ST3 Page Data: 0x");
          if (data_input < 0x10) Serial.print("0");
          Serial.println(data_input, HEX);
        }
      }
      break;

    case ST4:
      if (subState == SUB_ADDR) {
        if (key >= '0' && key <= '9') {
          input_address = (input_address * 10 + (key - '0')) % 10000;
          Serial.print("ST4 Start Address: ");
          Serial.println(input_address);
        }
      } else if (subState == SUB_DATA) {
        int8_t hexVal = getHexVal(key);
        if (hexVal >= 0) {
          if (st4_nibble_count == 0) {
            // หากไม่ใช่ไบต์แรก ให้เลื่อนตำแหน่งแอดเดรสขึ้น 1 โดยอัตโนมัติ (ตามรูปที่ 17.6)
            if (!st4_first_byte) {
              input_address = (input_address + 1) % 1024;
            }
            st4_nibble1 = (uint8_t)hexVal;
            data_input = (uint8_t)hexVal;
            st4_nibble_count = 1;

            Serial.print("ST4 Address [");
            Serial.print(input_address);
            Serial.print("] 1st Digit: ");
            Serial.println(key);
          } else if (st4_nibble_count == 1) {
            uint8_t st4_nibble2 = (uint8_t)hexVal;
            data_input = ((st4_nibble1 << 4) | st4_nibble2) & 0xFF;
            st4_nibble_count = 0;
            st4_first_byte = false;

            // บันทึกค่าลงหน่วยความจำทันทีเมื่อกดครบ 2 หลัก
            EEPROM_Erase_and_Write1B(input_address % 1024, data_input);

            Serial.print("ST4 Address [");
            Serial.print(input_address);
            Serial.print("] 2nd Digit: ");
            Serial.print(key);
            Serial.print(" -> Saved 0x");
            if (data_input < 0x10) Serial.print("0");
            Serial.println(data_input, HEX);
          }
        }
      }
      break;

    case ST5:
      if (subState == SUB_ADDR) {
        if (key >= '0' && key <= '9') {
          input_address = (input_address * 10 + (key - '0')) % 10000;
          Serial.print("ST5 Start Address: ");
          Serial.println(input_address);
        }
      } else if (subState == SUB_COUNT) {
        if (key >= '0' && key <= '9') {
          erase_count = (erase_count * 10 + (key - '0')) % 100;
          Serial.print("ST5 Erase Count: ");
          Serial.println(erase_count);
        }
      }
      break;

    default:
      break;
  }
}

// 
// Scan EEPROM
// 
int scan_EEPROM(void) {
  int i, number_data_found = 0;
  uint8_t d;
  
  Serial.println("Performing EEPROM scanning...");
  for (i = 0; i < 1024; i++) {
    d = EEPROM_read1byte(i);
    if (d != 0xFF) {
      number_data_found++;
      if (d < 0x10) Serial.print("0");
      Serial.print(d, HEX);
      Serial.print(" is found in EEPROM location[");
      Serial.print(i);
      Serial.println("]");     
    }
  }
  
  if (number_data_found > 1)
    Serial.println("More than one data are found in EEPROM");
  else if (number_data_found == 0)
    Serial.println("no data found in EEPROM");

  return number_data_found;
}

// 
// TM1638 Hardware Low-Level Functions
// 
void sendCommand(uint8_t value) {
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}

void reset() {
  sendCommand(0x40); // Set auto increment mode
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xC0); // Starting address 0
  for (uint8_t i = 0; i < 16; i++) {
    shiftOut(data, clock, LSBFIRST, 0x00);
  }
  digitalWrite(strobe, HIGH);
}

uint8_t readTM1638_Buttons(void) {
  uint8_t buttons = 0;
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0x42);

  pinMode(data, INPUT);
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t v = shiftIn(data, clock, LSBFIRST) << i;
    buttons |= v;
  }
  pinMode(data, OUTPUT);
  digitalWrite(strobe, HIGH);
  return buttons;
}

// 
// I2C Bus Scan
// 
void I2C_bus_scan(void) {
  Serial.println();
  Serial.println("I2C scanner. Scanning ...");
  byte count = 0;

  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found address: ");
      Serial.print(i, DEC);
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.println(")");
      count++;
      delay(1);
    }
  }
  Serial.print("Found ");
  Serial.print(count, DEC);
  Serial.println(" device(s).");
}
