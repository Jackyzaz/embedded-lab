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

const uint8_t SW1_MASK = 0x01;
const uint8_t SW2_MASK = 0x02;
const uint8_t SW3_MASK = 0x04;
const uint8_t SW4_MASK = 0x08;
const uint8_t SW6_MASK = 0x20;
const uint8_t SW7_MASK = 0x40;
const uint8_t SW8_MASK = 0x80;

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

// State Machine
enum stateType {ST1, ST2, ST3, ST4, ST5, ST6};
stateType currentState = ST1;

uint16_t input_address = 0;
uint8_t data_input = 0;

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
void renderDisplay(void);
void receive_address(void);
void receive_data(void);
void I2C_bus_scan(void);

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
  sendCommand(0x8F);  // Activate 7-segment display and set brightness
  reset();
}

void loop() {
  handleBlink();
  handleButtons();

  // State Actions
  switch (currentState) {
    case ST1:
      // รอกดปุ่ม (หน้าจอดับ)
      break;

    case ST2:
      display_all_data_in_EEPROM();
      scan_EEPROM();
      currentState = ST1;
      break;

    case ST3:
      receive_address();
      break;

    case ST4:
      receive_data();
      break;

    case ST5:
      Serial.print("Writing 0x");
      if (data_input < 0x10) Serial.print("0");
      Serial.print(data_input, HEX);
      Serial.print(" to EEPROM Address: ");
      Serial.println(input_address);

      EEPROM_Erase_and_Write1B(input_address, data_input);
      data_input = 0;
      input_address = 0;
      currentState = ST1;
      break;
  }

  renderDisplay();
}

// 
// Display & Blinking
// 
void handleBlink() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
    lastBlinkTime = currentMillis;
    blinkState = !blinkState;
  }
}

void renderDisplay() {
  uint8_t digits[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  switch (currentState) {
    case ST1:
    case ST2:
    case ST5:
      // ดับ 7-segment ทั้ง 8 หลัก
      break;
    case ST3:
      // 4 หลักซ้ายกระพริบ แสดงเลข Address ฐาน 10 (4 หลัก)
      if (blinkState) {
        digits[0] = digitFont[(input_address / 1000) % 10];
        digits[1] = digitFont[(input_address / 100) % 10];
        digits[2] = digitFont[(input_address / 10) % 10];
        digits[3] = digitFont[input_address % 10];
      }
      digits[6] = digitFont[(data_input >> 4) & 0x0F];
      digits[7] = digitFont[data_input & 0x0F];
      break;
    case ST4:
      // 4 หลักซ้ายติดค้าง แสดง Address
      digits[0] = digitFont[(input_address / 1000) % 10];
      digits[1] = digitFont[(input_address / 100) % 10];
      digits[2] = digitFont[(input_address / 10) % 10];
      digits[3] = digitFont[input_address % 10];

      // 2 หลักขวาสุด (ตำแหน่ง 6 และ 7) กระพริบ แสดง Data ฐาน 16 (2 หลัก)
      if (blinkState) {
        digits[6] = digitFont[(data_input >> 4) & 0x0F];
        digits[7] = digitFont[data_input & 0x0F];
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
// Keypad & Buttons Handlers
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
  bool s7_pressed = (pressed & SW7_MASK);
  bool s8_pressed = (pressed & SW8_MASK);
  if (s1_pressed) {
    switch (currentState) {
      case ST2:
        currentState = ST1;
        break;
      default:
        break;
    }
  }
  if (s2_pressed) {
    switch (currentState) {
      case ST1:
        currentState = ST3;
        break;
      default:
        break;
    }
  }
  if (s3_pressed) {
    switch (currentState) {
      case ST1:
        currentState = ST4;
        break;
      default:
        break;
    }
  }
  if (s4_pressed) {
    switch (currentState) {
      case ST1:
        currentState = ST5;
        break;
      default:
        break;
    }
  }
  if (s6_pressed) {
    switch (currentState) {
      case ST1:
        currentState = ST2;
        break;
      case ST3:
        currentState = ST1;
        break;
      case ST4:
        currentState = ST1;
        break;
      case ST5:
        currentState = ST1;
        break;
      default:
        break;
    }
  }
  if (s8_pressed) {
    switch (currentState) {
      case ST1:
        currentState = ST6;
        input_address = 0;
        break;
      default:
        break;
    }
  }
  Serial.print("State Changed to ");
  Serial.println(currentState);
}

void receive_address() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    // รับเฉพาะเลข 0-9
    if (key >= '0' && key <= '9') {
      input_address = (input_address * 10 + (key - '0')) % 10000;
      Serial.print("Address: ");
      Serial.println(input_address);
    }
  }
}

void receive_data() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    int8_t hexVal = -1;

    if (key >= '0' && key <= '9') {
      hexVal = key - '0';
    } else if (key >= 'A' && key <= 'F') {
      hexVal = key - 'A' + 10;
    }

    if (hexVal >= 0) {
      // เลื่อนฐาน 16 (Hex Shift 4 บิต) เพื่อรับ 2 หลัก
      data_input = ((data_input << 4) | (uint8_t)hexVal) & 0xFF;
      Serial.print("Data: 0x");
      if (data_input < 0x10)  // helper ช่วยพิม 0
        Serial.print("0");
      Serial.println(data_input, HEX);
    }
  }
}

int scan_EEPROM(void)
{
  int i, number_data_found = 0;
  uint8_t d;
  
  Serial.println("Performing EEPROM scanning...");
  for(i=0; i<1024; i++)
  {
    d = EEPROM_read1byte(i);
    if (d != 0xFF)
    {
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
  else if(number_data_found == 0)
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
