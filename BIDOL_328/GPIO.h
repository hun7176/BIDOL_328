#ifndef __GPIO_H__
#define __GPIO_H__

#include "common.h"

void GPIO_Init(void) {
  // 74595 제어용 핀
  SRCK_DDR |= (1 << SRCK_PIN);
  RCK_DDR |= (1 << RCK_PIN);
  SER_DDR |= (1 << SER_PIN);
  SRCK_PORT &= ~(1 << SRCK_PIN);
  RCK_PORT &= ~(1 << RCK_PIN);
  SER_PORT &= ~(1 << SER_PIN);

  // 히터 제어용 핀
  SEAT_HEAT_DDR |= (1 << SEAT_HEAT_PIN);
  WATER_HEAT_DDR |= (1 << WATER_HEAT_PIN);
  SEAT_HEAT_PORT &= ~(1 << SEAT_HEAT_PIN);
  WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);

  // 모터 제어용 핀
  SERVO_DDR |= (1 << SERVO_PIN);
  STEPPER_STEP_DDR |= (1 << STEPPER_STEP_PIN);
  STEPPER_DIR_DDR |= (1 << STEPPER_DIR_PIN);
  DC_DDR |= (1 << DC_PIN);
  SERVO_PORT &= ~(1 << SERVO_PIN);
  STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_PIN);
  STEPPER_DIR_PORT |= (1 << STEPPER_DIR_PIN);
  DC_PORT &= ~(1 << DC_PIN);
}

void write_LED(void) {
  for (int i = 15; i >= 0; i--) {
    // 시리얼 데이터 비트 단위 추출
    if (led & (1 << i))
      SER_PORT |= (1 << SER_PIN);
    else
      SER_PORT &= ~(1 << SER_PIN);

    // 시리얼 클록 신호 생성
    SRCK_PORT |= (1 << SRCK_PIN);
    SRCK_PORT &= ~(1 << SRCK_PIN);
  }

  // 래치 클록 신호 생성
  RCK_PORT |= (1 << RCK_PIN);
  RCK_PORT &= ~(1 << RCK_PIN);
}

void EXTI_Init(void) { // 외부 인터럽트 초기화
  STOP_BT_DDR &= ~(1 << STOP_BT_PIN);
  SEAT_SENSOR_DDR &= ~(1 << SEAT_SENSOR_PIN);
  EIMSK |= (1 << INT0 | 1 << INT1);   // INT0, INT1 인터럽트 활성화
  EICRA |= (1 << ISC01 | 1 << ISC00); // 버튼 상태 변화 감지
  EICRA |= (1 << ISC11 | 1 << ISC10); // 버튼 상태 변화 감지
  sei();                              // 전역 인터럽트 허용
}

#endif