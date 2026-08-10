#include <avr/io.h>
int main(void) {
  DDRD |= (1<<DDD6 | 1<<DDD5); // output PD5-6
  TCCR0A = (1<<WGM01 | 1<<WGM00);
  TCCR0A |= (1<<COM0B1 | 0<<COM0B0); // when reach timer, flip bit at OC0B
  TCCR0A |= (1<<COM0A1 | 0<<COM0A0); // when reach timer, flip bit at OC0A
  TCCR0B = (0<<WGM02 | 1<<CS00);

  // WGM0[2:0] = 011 -> Fasst PWM
  // CS0[2:1] = 001 -> N = 1

  TCNT0 = 0;
  OCR0A = 51;
  OCR0B = 179;

  while (1) {
    // Do nothing
  }
}
