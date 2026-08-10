#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>


#define wdt_timeout_2sec 7

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

void wait_until_sw_pressed(void) {
  uint8_t sw;
  do {
    do {
       wdt_reset();

       sw = PIND >> 2;
       sw &= 0x01;
    } while (sw);
    _delay_ms(10);
    wdt_reset();
    sw = PIND >> 2;
    sw &= 0x01;
  } while (sw);
  do {
    wdt_reset();
    sw = PIND >> 2;
    sw &= 0x01;
  } while (!sw);
}

void display_led(int count) {
  PORTD = 0b00000000;
  if (count == 0) PORTB = 0b00000000;
  else if (count == 1) PORTB = 0b00000001;
  else if (count == 2) PORTB = 0b00000011;
  else if (count == 3) PORTB = 0b00000111;
  else if (count == 4) PORTB = 0b00001111;
  else if (count == 5) PORTB = 0b00011111;
  else if (count == 6) PORTB = 0b00111111;
  else if (count == 7) {PORTB = 0b00111111; PORTD |= 0b01000000;}
  else if (count == 8) {PORTB = 0b00111111; PORTD |= 0b11000000;}
}

int main(void) {
  
  WDT__enable(wdt_timeout_2sec);
  
  DDRD = 0b11111011; // PD2 Input
  DDRB = 0xFF; // Port B Output
  PORTB = 0xFF;
   
  unsigned char count=0;
  Serial.begin(9600);

  while(1) {
    wait_until_sw_pressed();
    Serial.print("count");
    Serial.println(count);

    count++;
    if (count>8) count = 0;
    display_led(count);
  }
  
}
