#include <avr/interrupt.h>
#include <avr/io.h>

int main(void) {
  // put your setup code here, to run once:
  DDRD = 0b01000000; // enable PD6 output

  // COM0A -> 01 flip mode
  // WGM0 010 (CTC)
  // CS0 101 (divide 1024)
  TCCR0A = 0b01000010; // set wgm0[1:0] to 0
  TCCR0B = 0b00000101; // set wgm0[2] to 0 and set 101 scaler /1024

  TCNT0 = 0;
  OCR0A = 77;

  sei();

  while(1) {
    // empty loop
  }
}
