#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

const uint8_t anim_s1[5] = {0x81, 0x42, 0x24, 0x18, 0x00};
const uint8_t anim_s2[5] = {0x18, 0x24, 0x42, 0x81, 0x00};

volatile uint8_t mode = 0;

void display_led(uint8_t val) {
  val = ~val; // ca
  PORTD = (PORTD & 0x0F) | ((val & 0x0F) << 4);
  PORTB = ((val >> 4) & 0x0F);
}

void play_s1() {
  for (int r = 0; r < 3; r++) {
    for (int i = 0; i < 5; i++) {
      display_led(anim_s1[i]);
      _delay_ms(150);
    }
  }
}

void play_s2() {
  for (int r = 0; r < 3; r++) {
    for (int i = 0; i < 5; i++) {
      display_led(anim_s2[i]);
      _delay_ms(150);
    }
  }
}

ISR(INT0_vect) {
  while (!(PIND & (1 << PIND2)));
  _delay_ms(15);
  mode = 1;
  play_s1();

  
}

ISR(INT1_vect) {
  while (!(PIND & (1 << PIND3)));
  _delay_ms(15);
  mode = 2;
  play_s2();
}

void setup() {
  DDRD = 0b11110000;
  PORTD = 0b00001100;
  DDRB = 0b00001111;
  PORTB = 0b00000000;

  display_led(0);

  EICRA = 0b00000000; // low level
  EIMSK = 0b00000011;

  sei();
}

void loop() {
  mode = 0; 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_bod_disable();
  sleep_cpu();
  sleep_disable();
}
