#ifndef TIMER_CONTROL_H
#define TIMER_CONTROL_H

#include "common.h"

#define PULSE_BASE 2000
#define PULSE_STEP 330

// 서보모터 구동을 위한 PWM 타이머
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

void rotate_servo(int input) { OCR1A = PULSE_BASE + PULSE_STEP * input; }

#endif
