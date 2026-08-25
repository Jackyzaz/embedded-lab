#include "my_EEPROM.h"
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>

#define I2CADDR 0x20

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'E', '0', 'F', 'D'},
};

byte rowPins[ROWS] = {7, 6, 5, 4};
byte colPins[COLS] = {3, 2, 1, 0};

// TM1638
const int strobe = 4;
const int clock = 3;
const int data = 2;
void sendCommand(uint8_t value);
void reset();
uint8_t readTM1638_Buttons(void);
void setTM1638LED(uint8_t value, uint8_t position);
void display_Hello();

Keypad_I2C keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, I2CADDR);
void I2C_bus_scan(void);

int scan_EEPROM();

// moore state
const uint8_t sw0pressed = 0x80;
const uint8_t sw8pressed = 0x01;

enum stateType {ST1, ST2, ST3, ST4, ST5};
enum stateType currentState;

uint16_t input_address;
uint8_t data_input;

void setup()
{
  Wire.begin();
  keypad.begin();
  Serial.begin(38400);

  I2C_bus_scan();

  //  display_all_data_in_EEPROM();
  //  Serial.println("Begin Scan...");

  //  int found = scan_EEPROM();
  //  Serial.println("Finish Scan");
  
  currentState = ST1;
  sendCommand(0x8f);  // activate 7segments and set maximum brightness
  reset();
//  display_Hello();

}

void loop() {
  //  char key = keypad.getKey();
  //  if (key != NO_KEY) {
  //    Serial.print(key);
  //    Serial.println(" Is pressed");
  //  }
  uint8_t v = readTM1638_Buttons();
  switch (v) {
    case sw0pressed:
      switch (currentState) {
        case ST1: currentState = ST2; break;
        case ST4: currentState = ST5; break;
      }
      break;
    case sw8pressed:
      switch (currentState) {
        case ST1: currentState = ST3; input_address = 0; break;
        case ST3: currentState = ST4; data_input = 0 break;
        case ST4: currentState = ST3; input_address = 0; break;
      }
      break;
  }

  // output / action
  switch(currentState) {
    case ST1: reset(); break;
    case ST2: display_all_data_in_EEPROM(); 
              currentState = ST1; 
              break;
    case ST3: recieve_address(); break;
    case ST4: recieve_data(); break;
    case ST5: EEPROM_Erase_and_Write1B(input_address, data_input);
              currentState = ST1;
              break;
  }

}

void I2C_bus_scan(void) {
  Serial.println();
  Serial.println("www.9arduino.com ...");
  Serial.println("I2C scanner. Scanning ...");

  byte count = 0;

  Wire.begin();

  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found address:");
      Serial.print(i, DEC);
      Serial.print("(0x");
      Serial.print(i, HEX);
      Serial.println(")");
      count++;
      delay(1);
    }
  }
  Serial.println("Done");
  Serial.print("Found ");
  Serial.print(count, DEC);
  Serial.println(" device(s).");
}

int scan_EEPROM(void)
{
  int i, number_data_found = 0;
  uint8_t d;

  Serial.println("Performing EEPROM scanning...");
  for (i = 0; i < 1024; i++)
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
  else if (number_data_found == 0)
    Serial.println("no data found in EEPROM");

  return number_data_found;
}

// state funciton
void send_data_from_address_to_serial_monitor( ) {
  
}

void recieve_address() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    Serial.print(key);
    Serial.println(" Is pressed");
    
    if (input_address < 1023) {
      input_address = (input_address * 10) + (key - '0');
    }

    // display input_address with blink on tm1638 left 7th segment
  }
}

void recieve_data() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    Serial.print(key);
    Serial.println(" Is pressed");

    
    data_input = ...
    // display input_address with blink on tm1638 right 2 7-segment as hex
  }
}

// TM1638 Function
void sendCommand(uint8_t value)
{
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}


void reset()
{
  sendCommand(0x40); // set auto increment mode
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc0);   // set starting address to 0
  for (uint8_t i = 0; i < 16; i++)
  {
    shiftOut(data, clock, LSBFIRST, 0x00);
  }
  digitalWrite(strobe, HIGH);
}

uint8_t readTM1638_Buttons(void)
{
  uint8_t buttons = 0;
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0x42);

  pinMode(data, INPUT);

  for (uint8_t i = 0; i < 4; i++)
  {
    uint8_t v = shiftIn(data, clock, LSBFIRST) << i;
    buttons |= v;
  }

  pinMode(data, OUTPUT);
  digitalWrite(strobe, HIGH);
  return buttons;
}

void setTM1638LED(uint8_t value, uint8_t position)
{
  pinMode(data, OUTPUT);

  sendCommand(0x44);
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xC1 + (position << 1));
  shiftOut(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}

void display_Hello()
{
  sendCommand(0x44);  // set single address

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc0); // leftmost 7-segment digit
  shiftOut(data, clock, LSBFIRST, 0x76); // character to be displayed ='H'
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc2); // 2rd 7-segment
  shiftOut(data, clock, LSBFIRST, 0x79); // character to be displayed ='E'
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc4); // 3rd 7-segment
  shiftOut(data, clock, LSBFIRST, 0x38); // character to be displayed ='L'
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc6); // 4th 7-segment
  shiftOut(data, clock, LSBFIRST, 0x38); // character to be displayed ='L'
  digitalWrite(strobe, HIGH);

  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc8); // 5th 7-segment
  shiftOut(data, clock, LSBFIRST, 0x3f); // character to be displayed ='O'
  digitalWrite(strobe, HIGH);
}
