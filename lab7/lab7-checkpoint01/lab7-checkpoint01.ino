#include <avr/io.h>
#include <avr/interrupt.h>

void USART_Transmit(unsigned char data) {
  while ( !(UCSR0A & (1<<UDRE0) ) );
  UDR0 = data;
}

unsigned char TEXT[100] = {"My name is Soravit. My ID is 6710110428."};
unsigned char i;

int main(void) {
  UCSR0A = 0x00;
  UCSR0B = 0x08;  // enable transmit mode
  UCSR0C = 0b00101110;    // UMSEL0[1:0] 00; Asynchoronus
                          // UPM0[1:0] 10; odd parity 
                          // USBSN 1; 2 stop bit
  

  UBRR0H = 0;
  UBRR0L = 103;   // from calculation

  TEXT[40] = 13; // carriage return
  TEXT[41] = 10; // line feed

  for (i=0;i<100;i++) {
    USART_Transmit(TEXT[i]);
  }

  while(1) {
    // do nothing
 
  }
}
