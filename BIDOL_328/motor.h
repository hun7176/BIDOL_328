#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "common.h"

#define PULSE_BASE 2000
#define PULSE_STEP 330

/* **********서보 모터********** */
// 서보모터 구동을 위한 PWM 타이머 초기화
void TIMER1_Init(void) {

  // 고속 PWM 모드로 설정
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM12) | (1 << WGM13);

  // 분주율 8, 2MHz로 설정
  TCCR1B |= (1 << CS11);

  // TOP을 40000으로 설정 (T=20ms <-> f=50Hz)
  ICR1 = 40000;

  // 비교 일치 시 OC1A 클리어
  TCCR1A |= (1 << COM1A1);

  // PB1(OC1A) 핀을 출력으로 설정
  DDRB |= (1 << 1);
}

// 노즐 수압 조절 (0~5)
void rotate_servo(int input) { OCR1A = PULSE_BASE + PULSE_STEP * input; }

/* **********스텝 모터********** */
// 노즐 수납/사출
void nozzle_move(int nozdest) {
  // 노즐을 수납 위치 / 세정 위치로 이동
  // 이미 해당 위치일 경우 아무 것도 하지 않음
  static int nozpos = NOZZLE_IDLE;

  // 방향 설정
  if (nozpos == NOZZLE_IDLE && nozdest == NOZZLE_WASH) {
    nozzle_setdirection(FRONT);
    nozpos = NOZZLE_WASH;
  } else if (nozpos == NOZZLE_WASH && nozdest == NOZZLE_IDLE) {
    nozzle_setdirection(BACK);
    nozpos = NOZZLE_IDLE;
  } else
    return;

  // 180도 이동
  for (int i = 0; i < 1024; i++) {
    STEPPER_STEP_PORT |= (1 << STEPPER_STEP_PIN);
    _delay_us(500);
    STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_PIN);
    _delay_us(500);
  }
}

// 노즐 이동 방향 설정
void nozzle_setdirection(int dir) {
  if (dir == FRONT) {
    STEPPER_DIR_PORT &= ~(1 << STEPPER_DIR_PIN);
  } else { // dir == BACK
    STEPPER_DIR_PORT |= (1 << STEPPER_DIR_PIN);
  }
}

#endif