#ifndef __EXINTERRUPT_H__
#define __EXINTERRUPT_H__

#include "common.h"

void EXTI_Init(void) { // 외부 인터럽트 초기화
  STOP_BT_DDR &= ~(1 << STOP_BT_PIN);
  SEAT_SENSOR_DDR &= ~(1 << SEAT_SENSOR_PIN);
  EIMSK |= (1 << INT0 | 1 << INT1);   // INT0, INT1 인터럽트 활성화
  EICRA |= (1 << ISC01 | 1 << ISC00); // 버튼 상태 변화 감지
  EICRA |= (1 << ISC11 | 1 << ISC10); // 버튼 상태 변화 감지
  sei();                              // 전역 인터럽트 허용
}

ISR(INT0_vect) { // 일어남
  // 노즐 닫기
  rotate_servo(0);

  // 노즐 수납하기
  nozzle_move(NOZZLE_IDLE);

  // DC모터 팬 정지하기
  DC_PORT &= ~(1 << DC_PIN);

  // 상태 바꾸기
  state = ST_IDLE;
  UART_printString("idle\n");
}

ISR(INT1_vect) { // 정지 버튼 작동
  // 노즐 닫기
  rotate_servo(0);

  // 노즐 수납하기
  nozzle_move(NOZZLE_IDLE);

  // DC모터 팬 정지하기
  DC_PORT &= ~(1 << DC_PIN);

  // 상태 바꾸기
  state = ST_IDLE;
  UART_printString("idle\n");
}

#endif