#include <avr/io.h>
#include <avr/interrupt.h>
//#define F_CPU 16000000UL


int main(void) {
  DDRD = 0xFF;
  DDRB = 0xFF;
  DDRC = 0xFE;
  
  // ADMUX[7:6] = 00 -> (REFS1)(REFS0) ref AREF
  // ADMUX[5]   = 1  -> (ADLAR) Left aligned - 8bit ADCH: 2bit ADCL
  // ADMUX[3:0] = 0000 -> (MUX) ADC0 pin / PC0
  ADMUX = 0b00100000;

  Serial.begin(9600);
  while(1) {
    // ADCSRA[7]=1 -> (ADEN) ADC enable
    // ADCSRA[6]=1 -> (ADSC) ADC conversion
    // ADCSRA[5]=0 -> (ADSC) Disable auto trigger
    // ADCSRA[4]=0 -> (ADIF) ADC Interrupt Flag
    // ADCSRA[3]=0 -> (ADIE) ADC Interrupt Disable
    // ADCSRA[2:0]=111 -> (ADSP) Division factor / 128
    ADCSRA = 0b11000111;

    while (!(ADCSRA & (1 << ADIF)) ) {
      // do nothing
    }

    uint8_t low_bytes = ADCL;
    uint8_t high_bytes = ADCH;
    
    PORTD = high_bytes;
    PORTB = (low_bytes >> 6);
    
    uint16_t x = ADC;
    Serial.println(x);
    ADCSRA |= (1 << ADIF);
  }
}
