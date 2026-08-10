#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#define ADC0 0

// #define MAX_TCNT1 255 -> หาค่า N ~= 16 ไม่ได้
#define MAX_PWM 4000  // (F_CPU / (2 * Prescaler * F_Target)) -> (16MHz / (2 * 1 * 2kHz))

// global val
volatile uint16_t adc_result = 0;

// prototype function
uint32_t calculate_OCR1A(uint16_t adc_val);

ISR(ADC_vect) {
//    uint16_t adc_value = ADC; 
    adc_result = ADC;
    OCR1A = calculate_OCR1A(adc_result);
}

uint32_t calculate_OCR1A(uint16_t adc_val) {
  return  min(4000,((uint32_t)adc_val * MAX_PWM) / 330);
}

int main(void) {

  DDRB |= (1 << DDB1); // Output PB1

  // WGM1[3:0] = 1010 -> Phase Correct PWM, TOP = ICR1
  TCCR1A = (1<<WGM11 | 0<<WGM10); 
  TCCR1B = (1<<WGM13 | 0<<WGM12);

  TCCR1A |= (1<<COM1A1 | 0<<COM1A0); // COM1A[1:0] = 10 -> count up high, count down low (ขึ้นลงตรงกัน)

  TCCR1B |= (0<<CS12 | 0<<CS11 | 1<<CS10); // CS0[2:1] = 001 -> N = 1


  // Setup ADC
  DIDR0 = 0b00111111; // disable buffer  adc0-adc5
  DDRC =  0x00; // PC0 input

  // ADMUX[7:6] = 01; (REFS1)(REFS0) ref AVCC 
  // ADMUX[5] = 0; (ADLAR) right align 2 bit high - 8 bit low ADCH:ADCL
  // ADMUX[3:0] = 0000; (MUX3210) use ADC0
  ADMUX = 0b01000000;


  // ADCSRA[7] = 1; (ADEN) Enable ADC
  // ADCSRA[6] = 1; (ADSC) enable start
  // ADCSRA[5] = 1; (ADATE) enable auto trigger
  // ADCSRA[4] = 0; (ADIF) interrupt flag
  // ADCSRA[3] = 1; (ADIE) set interrupt enable
  // ADCSRA[2:0] = 111; (ADSP) [2:0] division factor /128
  ADCSRA = 0b11101111;

  // Free Running mode
  ADCSRB = (0<<ADTS2 | 0<<ADTS1 | 0<<ADTS0); 

  TCNT1=0;
  ICR1 = MAX_PWM;
  OCR1A = MAX_PWM / 2; // default 50% Duty Cycle

  sei();
  Serial.begin(9600);
  while (1) {
    // Do nothing
    Serial.print("adc_value: ");
    Serial.println(adc_result);

    Serial.print("OCRA1A: ");
    Serial.println(OCR1A);
  }
}
