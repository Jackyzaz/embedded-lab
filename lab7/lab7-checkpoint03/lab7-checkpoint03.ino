#include <avr/io.h>
#define F_CPU 16000000UL

int main(void) {
  uint16_t x;
  float v;

  DIDR0 = 0b00111111; // disable buffer  adc0-adc5
  DDRD =  0xFF;
  DDRC =  0x00;

  // ADMUX[7:6] = 01; (REFS1)(REFS0) ref AVCC 
  // ADMUX[5] = 0; (ADLAR) right align 2 bit high - 8 bit low ADCH:ADCL
  // ADMUX[3:0] = 0000; (MUX3210) use ADC0
  ADMUX = 0b01000000;

  
  // ADCSRA[7] = 1; (ADEN) Enable ADC
  // ADCSRA[6] = 0; (ADSC) disable start
  // ADCSRA[5] = 0; (ADSC) disable auto trigger
  // ADCSRA[4] = 0; (ADIF) interrupt flag
  // ADCSRA[3] = 0; (ADIE) set interrupt disable
  // ADCSRA[2:0] = 111; (ADSP) [210] division factor /128
  ADCSRA = 0b10000111;

  Serial.begin(9600);

  while(1) {
    ADCSRA |= (1<<ADSC);          // ADSC=1 start convert
    while( !(ADCSRA & (1<<ADIF)) ) {
      // do nothing  
    }
    x = ADC;
    ADCSRA |= (1<<ADIF);    // reset interrupt flag    

    v = ((float)x / 1023) * 5;
                                
    
    Serial.print("Voltage: ");
    Serial.print(v);
    Serial.println("V");
 
//    Serial.println();
  }
}
