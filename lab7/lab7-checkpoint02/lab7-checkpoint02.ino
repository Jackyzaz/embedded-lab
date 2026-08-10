#include <avr/io.h>
#include <avr/interrupt.h>

void USART_Transmit(unsigned char data) {
  while ( !(UCSR0A & (1<<UDRE0) ) ); // wait for buffer ready
  UDR0 = data;
}

unsigned char USART_Receive(void) {
  while ( !(UCSR0A & (1<<RXC0)) ); // wait for buffer to recieve
  return UDR0;
}

unsigned char TEXT[20] = {"Hello World"};
unsigned char i;
unsigned char temp;


int main(void) {
  UCSR0A = 0x00;
  UCSR0B = 0x18;          // enable transmit and recieve mode
  UCSR0C = 0b00100110;    // UMSEL0[1:0] 00; Asynchoronus
                          // UPM0[1:0] 10; even parity 
                          // USBSN 0; 1 stop bit
  UBRR0H = 0;
  UBRR0L = 103;   // from calculation

  TEXT[11] = 13; // carriage return
  TEXT[12] = 10; // line feed

  for (i=0;i<20;i++) {
    USART_Transmit(TEXT[i]);
  }

 
  while(1) {
    temp = USART_Receive();
    temp +=2;
    USART_Transmit(temp);
  }
}
