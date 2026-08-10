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
    v = ((float)x / 113.0) * 0.55;
                                  // Calculate for digital
                                  // Max 5V, need 9 interval
                                  // 5/9 ~= 0.55
                                  // 1023/9 ~= 113
    if (x<113)      PORTD = 0x00; //         Vin <= 0.55, Digital 0-122
    else if (x<227) PORTD = 0x01; // 0.55 <= Vin <= 1.11, Digital 113-227
    else if (x<342) PORTD = 0x03; // 1.11 <= Vin <= 1.67, Digital 227-342
    else if (x<455) PORTD = 0x07; // 1.67 <= Vin <= 2.22, Digital 342-455
    else if (x<596) PORTD = 0x0F; // 2.22 <= Vin <= 2.78, Digital 455-569
    else if (x<684) PORTD = 0x1F; // 2.78 <= Vin <= 3.34, Digital 569-684
    else if (x<799) PORTD = 0x3F; // 3.34 <= Vin <= 3.90, Digital 684-799
    else if (x<911) PORTD = 0x7F; // 3.90 <= Vin <= 4.45, Digital 799-911
    else            PORTD = 0xFF; // 4.45 <= Vin <= 5.00, Digital 911-1023

    Serial.print("ADC: ");
    Serial.print(x);
    Serial.print(", BIN: ");
    Serial.println(x, BIN);
    
    Serial.print("Voltage: ");
    Serial.print(v);
    Serial.println("V");
    
    Serial.println();
    ADCSRA |= (1<<ADIF);    // reset interrupt flag    
  }
}
