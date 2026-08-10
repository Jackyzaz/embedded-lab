#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

// PD6 -> OCR0A 
// PD5 -> OCR0B
// PD3 -> OCR2B

#define wdt_timeout_2sec 7

const char level = 5;
unsigned char led_level[level] = {0, 64, 128, 192, 255};

void WDT__enable(uint8_t timeout_v) {
  unsigned char bakSREG;
  uint8_t prescaler;
  prescaler = timeout_v & 0x07;
  prescaler |= (1<<WDE);

  if (timeout_v > 7) {
    prescaler |= (1<<WDP3);
  }

  bakSREG = SREG;
  cli();
  wdt_reset();
  WDTCSR |= ((1<<WDCE) | (1<<WDE));
  WDTCSR = prescaler;
  SREG = bakSREG; 
  sei();
}

int main(void) {
  
  DDRD |= (1<<DDD6 | 1<<DDD5 | 1<<DDD3); // output PD5-6
  TCCR0A = (1<<WGM01 | 1<<WGM00);
  TCCR0A |= (1<<COM0B1 | 0<<COM0B0); // when reach timer, flip bit at OC0B
  TCCR0A |= (1<<COM0A1 | 0<<COM0A0); // when reach timer, flip bit at OC0A
  TCCR0B = (0<<WGM02 | 1<<CS00);

  TCCR2A = (1<<WGM21 | 1<<WGM20);
  TCCR2A |= (1<<COM2B1 | 0<<COM2B0); // when reach timer, flip bit at OC0B
  TCCR2A |= (1<<COM2A1 | 0<<COM2A0); // when reach timer, flip bit at OC0A
  TCCR2B = (0<<WGM22 | 1<<CS20);

  // WGMx[2:0] = 011 -> Fast PWM
  // CSx[2:1] = 001 -> N = 1
  // f = 62.5 kHz

  WDT__enable(wdt_timeout_2sec);
  
  TCNT0 = 0;
  OCR0A = 0;
  OCR0B = 0;
  OCR2B = 0;
  
  int i = 0;
  int j = 0;
  while (1) {
    for (i=0; i<level; i++) {
      OCR0A=led_level[i];
      OCR0B=led_level[i];
      OCR2B=led_level[i];
      
      for (j=0; j<10; j++) { //safe delay 100ms
        _delay_ms(100);
        wdt_reset();
      }
    }    
  }
}
