#include <avr/interrupt.h>
#include <avr/io.h>

const int ONE_SEC_TCNT = 3036;

int timer = 0;

ISR(TIMER1_OVF_vect) {
  TCNT1 = ONE_SEC_TCNT;
  timer++;
  if (timer==10) {
    timer=0;
    PORTB ^= 0x20;
  }
  
}

int main(void) {
  // WGM1 = 0000 Normal Mode
  // CS1 = 100 divide 1024
  timer = 0;
  DDRB = 0x20; // PB5 output
  PORTB = 0x20;
  
  TIMSK1 |= 0x01;
  TCCR1A = 0x00; // WGM1[3:0] = 0000, COMnX01 -> 0
  TCCR1B = 0x04; // CS1[2:0] = 100 -> /256
  TCCR1C = 0x00;

  TCNT1 = ONE_SEC_TCNT;
  
  sei();

  while(1) {
    // empty loop
  }
}
