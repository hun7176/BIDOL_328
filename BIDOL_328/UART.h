#ifndef UART_H_
#define UART_H_

#include <avr/io.h>

void UART_INIT(void); //UART 초기화
void UART_transmit(unsigned char data); //1바이트 송신
void UART_printString(char *str); //문자열 송신

void UART_INIT(void){
	UCSR0A |= (1<<U2X0);
	UBRR0H = 0x00;
	UBRR0L = 207;
	UCSR0C |= 0x06;
	UCSR0B |= (1<<TXEN0);
}

void UART_transmit(unsigned char data){
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

void UART_printString(char *str){
	for(int i=0; str[i];i++){
		UART_transmit(str[i]);
	}
}

#endif /* UART_H_ */