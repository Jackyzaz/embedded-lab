#include <avr/io.h>
#include <util/delay.h> // แนะนำให้ใช้ <util/delay.h> แทน <avr/delay.h>
#include <Arduino.h>

#define TURN_ALL_LED_OFF 0 // Index 16 คือ 0x00 ในตาราง (Close)
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
  0b00000000, // 16: Close
};

int counter = 0;
int adcVal, prevADC, DECODED;

void DISPLAY7segment(unsigned char a){
  // เขียนเฉพาะบิต 2-7 โดยรักษาค่าบิต 0,1 (RX/TX) ไว้เพื่อไม่ให้ Serial พัง
  PORTD = (PORTD & 0x03) | ((TABLE7SEG[a] << 2) & 0xFC);
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

int main(void) {
  // ---------------------------------------------------------
  // 1. จำเป็นต้องเรียก init() เพื่อให้ Serial และ Timer ของ Arduino ทำงานได้
  // ---------------------------------------------------------
  init(); 

  // ---------------------------------------------------------
  // 2. ส่วนของการตั้งค่า (เทียบเท่า setup())
  // ---------------------------------------------------------
  DDRC = 0x00;
  DDRD = 0xFC; // เว้นขา 0 (RX) และ 1 (TX) ให้เป็นไปตามการตั้งค่าของ Serial
  DDRB = 0xFF;
  
  DISPLAY7segment(0);
  
  ADMUX = 0b01000101;
  ADCSRA = 0x87;
  DECODED = 0;

  Serial.begin(9600);
  Serial.println("Hello World - C++ Main Mode");

  // ---------------------------------------------------------
  // 3. ส่วนของการทำงานหลัก (เทียบเท่า loop())
  // ---------------------------------------------------------
  while (1) {
    counter = 0;
    prevADC = 0;
    
    // ลูปเช็ค Debounce (จะติดลูปที่นี่จนกว่าจะมีการกดปุ่ม)
    do {
      adcVal = ADC_read(ADC5);
      if ((abs(adcVal - prevADC) < 30) && (adcVal > 33)) {
        counter++;
      } else {
        counter = 0;
      }
      _delay_ms(3);
      prevADC = adcVal;
    } while(counter < 20);

    Serial.print("adcVal: ");
    Serial.println(adcVal);
    
    if      ((adcVal>38)  && (adcVal<63 ))  DECODED = 1;
    else if ((adcVal>78) && (adcVal<101))  DECODED = 2;
    else if ((adcVal>119) && (adcVal<144))  DECODED = 3;
    else if ((adcVal>200) && (adcVal<260))  DECODED = 10; // A
    else if ((adcVal>186) && (adcVal<211))  DECODED = 4;
    else if ((adcVal>260) && (adcVal<324))  DECODED = 5;
    else if ((adcVal>401) && (adcVal<426))  DECODED = 6;
    else if ((adcVal>500) && (adcVal<600))  DECODED = 11; // B
    else if ((adcVal>460) && (adcVal<530))  DECODED = 7;
    else if ((adcVal>620) && (adcVal<660))  DECODED = 8;
    else if ((adcVal>720) && (adcVal<780))  DECODED = 9;
    else if ((adcVal>840) && (adcVal<880))  DECODED = 12; // C
    else if ((adcVal>800) && (adcVal<840))  DECODED = 14; // E
    else if ((adcVal>890) && (adcVal<910))  DECODED = 0;
    else if ((adcVal>910) && (adcVal<940))  DECODED = 15; // F
    else if ((adcVal>950) && (adcVal<1023)) DECODED = 13; // D

    // ลูปรอจนกว่าจะปล่อยปุ่ม
//    do {
//      adcVal = ADC_read(ADC5);
//    } while(adcVal > 33);

    Serial.print("Decoded: ");
    Serial.println(DECODED);
    Serial.println();
    
    DISPLAY7segment(DECODED);
  }

  return 0;
}
