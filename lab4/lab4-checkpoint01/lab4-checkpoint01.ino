#include <avr/interrupt.h>
#include <avr/io.h>

ISR(TIMER0_OVF_vect) {
  TCNT0 = 178;
  PORTB ^= 0x01;
}

int main(void) {
  DDRB = 0x01;

  TIMSK0 = 0b00000001; // enable T0
  TCCR0A = 0b00000000; // set wgm0[1:0] to 0
  TCCR0B = 0b00000101; // set wgm0[2] to 0 and set 101 scaler /1024

  TCNT0 = 178;

  sei();

  while(1) {
    // empty loop
  }
}
