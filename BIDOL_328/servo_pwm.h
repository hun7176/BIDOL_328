#ifndef TIMER_CONTROL_H
#define TIMER_CONTROL_H

#include "common.h"

#define PULSE_MIN 2000
#define PULSE_MID 3000
#define PULSE_MAX 4000

void INIT_TIMER1(void) { // 서보모터용 pwm 생성
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM12) |
            (1 << WGM13); // WGM 설정으로 고속 PWM 모드, TOP을 ICR1로 설정
  TCCR1B |= (1 << CS11); // 분주율 8, 2MHz로 설정
  ICR1 = 40000; // TOP을 40000으로 설정 (50Hz PWM 신호를 위한 20ms 주기)
  TCCR1A |= (1 << COM1A1); // 비교 일치 시 OC1A 클리어
  DDRB |= (1 << 1);        // PB1/OC1A 핀을 출력으로 설정
}

void rotate_servo(int input) {
  if (input == 1)
    OCR1A = PULSE_MIN;
  else if (input == 2)
    OCR1A = PULSE_MID;
  else if (input == 3)
    OCR1A = PULSE_MAX;
}

#endif
