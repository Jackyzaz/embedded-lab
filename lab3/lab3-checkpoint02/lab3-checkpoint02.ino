#include <avr/io.h>
#include <avr/delay.h>
#define F_CPU 16000000UL

#define btn = 2; // PD2

int count = 0;

ISR(INT0_vect) {
  wait_until_sw_pressed();
  count++;
  if (count>9) count = 0;
  display7Segment(count);
  
}

int main() {
  pinMode(btn, INPUT_PULLUP);
  
  EIMSK = 0b00000001; // INT0 (PD2)
  EICRA = 0b00000010; // Falling Edge
  
  for (int i = segmentA; i<=segmentF; i++){
    pinMode(i, OUTPUT);
  }
  
  pinMode(segmentG, OUTPUT);
  
  pinMode(digit1, OUTPUT);
  digitalWrite(digit1, HIGH);
  
  display7Segment(count);

  sei();
}

void wait_until_sw_pressed() {
  int sw;
  do {
    _delay_ms(10);
    sw = PIND >> 2;
    sw &= 0x01; 
  } while(sw); // break loop when get 0
  do {
    sw = PIND >> 2;
    sw &= 0x01;
  } while(!sw); // break loop when get 1 (key released)
}

void setup() {
  // put your setup code here, to run once:
 
}

void loop() {
  // empty loop
}
