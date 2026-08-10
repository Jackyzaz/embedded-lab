#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#define TCNT1_V 62536   // 2ms สำหรับติดดับ
#define num_led 12

#define wdt_timeout_125ms 3
#define wdt_timeout_2sec  7

static unsigned char activeLED=0;
unsigned char LED_Status[num_led];

void WDT__enable(uint8_t timeout_v) {
  unsigned char bakSREG;
  uint8_t prescaler;
  prescaler = timeout_v & 0x07;
  prescaler |= (1<<WDE);

  if (timeout_v > 7) {
    prescaler |= (1<<WDP3);
  }

  bakSREG = SREG;
  cli();
  wdt_reset();
  WDTCSR |= ((1<<WDCE) | (1<<WDE));
  WDTCSR = prescaler;
  SREG = bakSREG; 
  sei();
}

ISR(TIMER1_OVF_vect) {
  TCNT1 = TCNT1_V;
  display_each_led(activeLED, LED_Status[activeLED]);
  activeLED++;
  if (activeLED >= num_led) {
    activeLED = 0;
  }
}

void display_each_led(int led_num, char status_on) {
  // ล้างขา PC0-PC3 ให้กลับไปเป็น High Impedance (INPUT) ทั้งหมดก่อนเสมอ
  DDRC &= 0b11110000;
  PORTC &= 0b11110000;

  // ถ้า status_on เป็น 0 (สั่งปิดไฟ) ไม่ต้องทำอะไรต่อ จบฟังก์ชันได้เลย
  if (!status_on) return;

  // ถ้า status_on เป็น 1 ค่อยมาเลือกเปิดเฉพาะขากับทิศทางที่ต้องการ
  switch(led_num) {
    case  0: DDRC |= 0b0011; PORTC |= 0b0001; break;
    case  1: DDRC |= 0b0011; PORTC |= 0b0010; break;
    case  2: DDRC |= 0b0110; PORTC |= 0b0010; break;
    case  3: DDRC |= 0b0110; PORTC |= 0b0100; break;
    case  4: DDRC |= 0b1100; PORTC |= 0b0100; break;
    case  5: DDRC |= 0b1100; PORTC |= 0b1000; break;
    case  6: DDRC |= 0b0101; PORTC |= 0b0001; break;
    case  7: DDRC |= 0b0101; PORTC |= 0b0100; break;
    case  8: DDRC |= 0b1010; PORTC |= 0b0010; break;
    case  9: DDRC |= 0b1010; PORTC |= 0b1000; break;
    case 10: DDRC |= 0b1001; PORTC |= 0b0001; break;
    case 11: DDRC |= 0b1001; PORTC |= 0b1000; break;
  }
}

int main(void) {
  cli();
  TIMSK1 = (1<<TOIE1);
  TCCR1B = 0x02;
  TCCR1A = 0x00;
  TCNT1 = TCNT1_V;
  WDT__enable(wdt_timeout_125ms);
  sei();
  int i;
  for (i=0; i<num_led; i++) {
    LED_Status[i] = 0;
  }

  while(1) {
    for (i=0; i<num_led; i++) { // allow led on
      LED_Status[i] = 1; 
    }
    for (i=0; i<10; i++) {      // loop timer
      _delay_ms(100);
      wdt_reset();
    }
    for (i=0; i<num_led; i++) {
      LED_Status[i] = 0;
    }
    
    for (i=0; i<10; i++) {
      _delay_ms(100);
      wdt_reset();
    }
  }
  
}
