#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

volatile uint8_t counter = 0;

void display_led(uint8_t val) {
  val = ~val; // ca
  PORTD = (PORTD & 0x0F) | ((val & 0x0F) << 4);
  PORTB = ((val >> 4) & 0x0F);
}

ISR(INT0_vect) {
  _delay_ms(15);
  if (!(PIND & (1 << PIND2))) {
    counter++;
    display_led(counter);
    while (!(PIND & (1 << PIND2)));
    _delay_ms(15);
  }
}

ISR(INT1_vect) {
  _delay_ms(15);
  if (!(PIND & (1 << PIND3))) {
    counter--;
    display_led(counter);
    while (!(PIND & (1 << PIND3)));
    _delay_ms(15);
  }
}

void setup() {
  DDRD = 0b11110011;  // PD2-3 Input PD4-7 Output
  PORTD = 0b00001100; // Input Pullup Mode
  DDRB = 0b00001111;  // Output PB0-3 

  display_led(counter);

  EICRA = 0b00000000; // low level interrupt
  EIMSK = 0b00000011;

  sei();
}

void loop() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_bod_disable();
  sleep_cpu();
  sleep_disable();
}
