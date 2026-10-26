#include <avr/io.h>
#include <avr/interrupt.h>
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
  Serial.println(counter);

}

ISR(INT1_vect) {
  _delay_ms(15);
  if (!(PIND & (1 << PIND3))) {
    counter--;
    display_led(counter);
    while (!(PIND & (1 << PIND3)));
    _delay_ms(15);
  }
  Serial.println(counter);
}

void setup() {
  DDRD = 0b11110011;  // PD2-3 Input PD4-7 Output
  PORTD = 0b00001100; // Input Pullup Mode
  DDRB = 0b00001111;  // Output PB0-3 Output

  display_led(counter);

  EICRA = 0b00000000; // low level interrupt
  EIMSK = 0b00000011; // enable int0 int1

  sei();
  Serial.begin(9600);
}

void loop() {
  // empty loop
}
