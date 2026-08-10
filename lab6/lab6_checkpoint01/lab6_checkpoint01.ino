#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#define TURN_ALL_LED_OFF 0x00
#define ADC5 5

unsigned char TABLE7SEG[] = 
{
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9
  0b01110111, // A
  0b01111100, // B
  0b00111001, // C
  0b01011110, // D
  0b01111001, // E
  0b01110001, // F
  0b00000000, // Close
};

int counter = 0;

int adcVal, prevADC, DECODED;

void DISPLAY7segment(signed char a){
  PORTD = (TABLE7SEG[a] << 2);
  PORTB = (TABLE7SEG[a] >> 6);
  PORTB |= 0x02;
}
unsigned int ADC_read(unsigned char a){
  ADMUX &= 0xF0;
  ADMUX |= a;
  ADCSRA |= (1<<ADSC); // Start ADC
  while(!(ADCSRA & (1<<ADIF)));
  ADCSRA |= 1<<ADIF; // RESET FLAG
  return ADC;
}
int main(void){
  DDRC = 0x00;
  DDRD = 0xFF;
  DDRB = 0xFF;
  DISPLAY7segment(TURN_ALL_LED_OFF);
  
  // ADMUX[7:6] = 01; (REFS1)(REFS0) ref AVCC 
  // ADMUX[5] = 0; (ADLAR) right align 2 bit high - 8 bit low ADCH:ADCL
  // ADMUX[3:0] = 0101; (MUX3210) use ADC5
  ADMUX = 0b01000101;

  // ADCSRA[7] = 1; (ADEN) Enable ADC
  // ADCSRA[6] = 0; (ADSC) disable start
  // ADCSRA[5] = 0; (ADSC) disable auto trigger
  // ADCSRA[4] = 0; (ADIF) interrupt flag
  // ADCSRA[3] = 0; (ADIE) set interrupt disable
  // ADCSRA[2:0] = 111; (ADSP) [210] division factor /128
  ADCSRA = 0x87;
  DECODED = 0;

  Serial.begin(9600);
  while(1){
    counter = 0;
    prevADC = 0;
      // Prevent Debounce
      do{
        adcVal = ADC_read(ADC5);
//        Serial.print("adcValRaw: ");
//        Serial.println(adcVal);
//        if ((abs(adcVal-prevADC)<6) && (adcVal>33)) {
        if ((abs(adcVal-prevADC)<20) && (adcVal>33)) {

          counter++;
        } else {
          counter = 0;
        }
        _delay_ms(3);
        prevADC = adcVal;
//        Serial.print("counter: ");
//        Serial.println(counter);
      }while(counter<20);

      Serial.print("adcVal: ");
      Serial.println(adcVal);
      
      if      ((adcVal>33) &&  (adcVal<65 ))   DECODED = 1;
      else if ((adcVal>71)  && (adcVal<103 )) DECODED = 2;
      else if ((adcVal>114) && (adcVal<146 )) DECODED = 3;
      else if ((adcVal>181) && (adcVal<213 )) DECODED = 4;
      else if ((adcVal>294) && (adcVal<326 )) DECODED = 5;
      else if ((adcVal>396) && (adcVal<428 )) DECODED = 6;
      else if ((adcVal>440) && (adcVal<560 )) DECODED = 7;
      else if ((adcVal>580) && (adcVal<660 )) DECODED = 8;
      else if ((adcVal>680) && (adcVal<780 )) DECODED = 9;
      else if ((adcVal>790) && (adcVal<840 )) DECODED = 10;
      else if ((adcVal>860) && (adcVal<880 )) DECODED = 0;
      else if ((adcVal>890) && (adcVal<1023)) DECODED = 11;

      // Loop until release
      do{
        adcVal = ADC_read(ADC5);
      }while(adcVal > 33);

       Serial.print("Decoded: ");
      Serial.println(DECODED);
      Serial.println();
      DISPLAY7segment(DECODED);
  }
}
