#ifndef __GPIO_H__
#define __GPIO_H__

#include "common.h"

// GPIO 핀 초기화
void GPIO_Init(void) {
  // 74595 제어용 핀
  SRCK_DDR |= (1 << SRCK_PIN);
  SRCK_PORT &= ~(1 << SRCK_PIN);
  RCK_DDR |= (1 << RCK_PIN);
  RCK_PORT &= ~(1 << RCK_PIN);
  SER_DDR |= (1 << SER_PIN);
  SER_PORT &= ~(1 << SER_PIN);

  // 히터 제어용 핀
  SEAT_HEAT_DDR |= (1 << SEAT_HEAT_PIN);
  SEAT_HEAT_PORT &= ~(1 << SEAT_HEAT_PIN);
  WATER_HEAT_DDR |= (1 << WATER_HEAT_PIN);
  WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);

  // 모터 제어용 핀
  SERVO_DDR |= (1 << SERVO_PIN);
  SERVO_PORT &= ~(1 << SERVO_PIN);
  STEPPER_STEP_DDR |= (1 << STEPPER_STEP_PIN);
  STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_PIN);
  STEPPER_DIR_DDR |= (1 << STEPPER_DIR_PIN);
  STEPPER_DIR_PORT |= (1 << STEPPER_DIR_PIN);
  DC_DDR |= (1 << DC_PIN);
  DC_PORT &= ~(1 << DC_PIN);
}

// LED 상태 데이터 시리얼 전송
void write_LED(void) {
  for (int i = 15; i >= 0; i--) {
    // 시리얼 데이터 비트 단위 추출
    if (led & (1 << i))
      SER_PORT |= (1 << SER_PIN);
    else
      SER_PORT &= ~(1 << SER_PIN);

    // 시리얼 클럭 신호 생성해 데이터 전송
    SRCK_PORT |= (1 << SRCK_PIN);
    SRCK_PORT &= ~(1 << SRCK_PIN);
  }

  // 래치 클록 신호 생성해 LED 업데이트
  RCK_PORT |= (1 << RCK_PIN);
  RCK_PORT &= ~(1 << RCK_PIN);
}

#endif