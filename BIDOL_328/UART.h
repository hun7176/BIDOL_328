﻿#ifndef __UART_H__
#define __UART_H__

#include "common.h"

// UART 초기화
void UART_Init(void) {
  UCSR0A |= (1 << U2X0);
  UBRR0H = 0x00;
  UBRR0L = 207;
  UCSR0C |= 0x06;
  UCSR0B |= (1 << TXEN0);
}

// 1바이트 데이터 송신
void UART_transmit(unsigned char data) {
  while (!(UCSR0A & (1 << UDRE0)))
    ;          // 송신 준비 완료까지 대기
  UDR0 = data; // 한 글자 송신
}

// 문자열 송신
void UART_printString(char *str) {
  while (*str)               // 문자열의 끝('\0') 이전까지
    UART_transmit(*(str++)); // 한 글자씩 송신
}

// 정수 송신
void UART_printInteger(int n) {
  while (n > 0) {
    UART_transmit((n % 10) + '0'); // 일의 자리부터 역순으로 송신
    n /= 10;
  }
}

void int_to_string(int n, char *buffer) {
  sprintf(buffer, "%04d", n);
  buffer[4] = '\0';
}

#endif