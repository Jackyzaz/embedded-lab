#include <avr/io.h>
#include <util/delay.h>

const uint8_t anim_s1[5] = {0x81, 0x42, 0x24, 0x18, 0x00};
const uint8_t anim_s2[5] = {0x18, 0x24, 0x42, 0x81, 0x00};

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

void setup() {
  DDRD = 0b11110000;
  PORTD = 0b00001100;
  DDRB = 0b00001111;
  PORTB = 0b00000000;

  display_led(0);
}

void loop() {
  if (!(PIND & (1 << PIND2))) {
    _delay_ms(15);
    if (!(PIND & (1 << PIND2))) {
      play_s1();
      while (!(PIND & (1 << PIND2)));
      _delay_ms(15);
    }
  }

  if (!(PIND & (1 << PIND3))) {
    _delay_ms(15);
    if (!(PIND & (1 << PIND3))) {
      play_s2();
      while (!(PIND & (1 << PIND3)));
      _delay_ms(15);
    }
  }
}
