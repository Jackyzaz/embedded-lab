#include <avr/interrupt.h>
#include <avr/io.h>

const int strobe = 7;
const int clock = 9;
const int data = 8;

const uint16_t HUNDRED_MILLI_SEC = 63973;

// interrupt counter
uint8_t timer = 0;
// each counter 
uint8_t count1 = 0;
uint8_t count2 = 0;
uint8_t count3 = 99;
uint8_t count4 = 99;
// stop tracking
bool run1 = 0;
bool run2 = 0;
bool run3 = 0;
bool run4 = 0;
// check rising edge var
uint8_t lastButtons = 0;
// hold 2 sec check
uint8_t hold_S2 = 0; 
uint8_t hold_S4 = 0; 
uint8_t hold_S6 = 0; 
uint8_t hold_S8 = 0;

void setup() {
  timer = 0;
  // ตั้งค่า Timer 1
  TIMSK1 |= 0x01; // Enable ISR Timer1
  TCCR1A = 0x00; // Normal Mode
  // CS1[2:0] = 101 -> Prescaler /1024 
  TCCR1B = 0x05;
  TCNT1 = HUNDRED_MILLI_SEC;
  // setup TM1638 pin
  pinMode(strobe, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);
  // setup command
  sendCommand(0x8f);
  reset();
  sei();
}

ISR(TIMER1_OVF_vect) {
  TCNT1 = HUNDRED_MILLI_SEC;
  timer++;
  if (timer >= 10) {
    // นับครบ 1 วินาที, reset timer
    timer = 0;
    // counter 1,2 นับขึ้น
    if (run1) {
      count1++;
      if (count1 > 99) count1 = 0;
    }
    if (run2) {
      count2++;
      if (count2 > 99) count2 = 0;
    }
    // counter3,4 นับลง
    if (run3) {
      if (count3 == 0) count3 = 99;
      else count3--;
    }
    if (run4) {
      if (count4 == 0) count4 = 99;
      else count4--;
    }
  }
}

void loop() {
  // อ่านสัญญาณ
  uint8_t buttons = readTM1638_Buttons();

  // rising edge detection
  uint8_t pressed = buttons & ~lastButtons;
  lastButtons = buttons;

  if (pressed) {
    cli();
    // counter1
    if (pressed & 0x01) run1 = 1;
    if (pressed & 0x02) run1 = 0;
    // counter2
    if (pressed & 0x04) run2 = 1;
    if (pressed & 0x08) run2 = 0;
    // counter 3
    if (pressed & 0x10) run3 = 1;
    if (pressed & 0x20) run3 = 0;
    // counter4
    if (pressed & 0x40) run4 = 1;
    if (pressed & 0x80) run4 = 0;
    sei();
  }
  // process ว่า stop หรือ reset
  process_buttons();
  display_counters();
}

void process_buttons() {
  delay(100); // 100ms เพื่อการตอบสนองที่ดี
  // reset (20 x 100ms) 
  // counter1
  if (buttons & 0x02) {
    hold_S2++; 
    if (hold_S2 == 20) {
      cli();
      count1 = 0; // ครบ 2 วินาที reset count1
      sei();
    }
  } else hold_S2 = 0;
  // counter2
  if (buttons & 0x08) {
    hold_S4++;
    if (hold_S4 == 20) {
      cli();
      count2 = 0; // ครบ 2 วินาที reset count2
      sei();
    }
  } else hold_S4 = 0;
  // counter3
  if (buttons & 0x20) {
    hold_S6++;
    if (hold_S6 == 20) {
      cli();
      count3 = 99; // ครบ 2 วินาที reset count3
      sei();
    }
  } else hold_S6 = 0;
  // counter4
  if (buttons & 0x80) {
    hold_S8++;
    if (hold_S8 == 20) {
      cli();
      count4 = 99; // ครบ 2 วินาที reset count4
      sei();
    }
  } else hold_S8 = 0;
}

void sendCommand(uint8_t value)
{
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}

void reset()
{
  sendCommand(0x40);
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc0);
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

void display_counters()
{
  static uint8_t digits[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 
                              0x6d, 0x7d, 0x07, 0x7f, 0x6f };
  sendCommand(0x44);
  //counter1
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc0);
  shiftOut(data, clock, LSBFIRST, digits[count1 / 10]);
  digitalWrite(strobe, HIGH);
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc2);
  shiftOut(data, clock, LSBFIRST, digits[count1 % 10]);
  digitalWrite(strobe, HIGH);
  //counter2
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc4);
  shiftOut(data, clock, LSBFIRST, digits[count2 / 10]);
  digitalWrite(strobe, HIGH);
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc6);
  shiftOut(data, clock, LSBFIRST, digits[count2 % 10]);
  digitalWrite(strobe, HIGH);
  //counter3
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc8);
  shiftOut(data, clock, LSBFIRST, digits[count3 / 10]);
  digitalWrite(strobe, HIGH);
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xca);
  shiftOut(data, clock, LSBFIRST, digits[count3 % 10]);
  digitalWrite(strobe, HIGH);
  //counter4
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xcc);
  shiftOut(data, clock, LSBFIRST, digits[count4 / 10]);
  digitalWrite(strobe, HIGH);
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xce);
  shiftOut(data, clock, LSBFIRST, digits[count4 % 10]);
  digitalWrite(strobe, HIGH);
}
