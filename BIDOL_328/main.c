﻿#define F_CPU 16000000L // MCU 동작 클럭 설정

#include "common.h"

volatile uint16_t led = 0;    // 74595로 전송할 LED 데이터
volatile int state = ST_IDLE; // 현재 상태
volatile int watertemp = 0;   // 현재 온수 단계 (0~4)
volatile int seattemp = 0;    // 현재 변좌 온도 단계 (0~4)
volatile int waterpres = 0;   // 현재 수압 단계 (1~5)
volatile int nozzpos = 0;     // 현재 노즐 위치 (-2(rear)~2(front))
volatile int wpflag = 0;      // 수압 변화 플래그

volatile int mvoffset = 0;  // 무브세정 위치 오프셋
volatile int mvdir = FRONT; // 무브세정 이동중인 방향

volatile int button = 0;     // 스위치 ADC 입력값
volatile int prevbt = 0;     // 스위치 ADC 직전입력값
volatile int wtemp_val = 0;  // 수온센서 ADC 입력값
volatile int stemp_val = 0;  // 변좌온도센서 ADC 입력값
volatile int wlevel_val = 0; // 수위센서 ADC 입력값

#include "ADC.h"
#include "GPIO.h"
#include "UART.h"
#include "exinterrupt.h"
#include "motor.h"
#include "temp_control.h"

int main(void) {

  /* ******************** */
  /*   Initialize Phase   */
  /* ******************** */

  GPIO_Init();   // GPIO 초기화
  ADC_Init();    // ADC 초기화
  EXTI_Init();   // 외부 인터럽트 초기화
  TIMER1_Init(); // 서보모터 PWM 타이머 초기화
  UART_Init();   // 디버그용 UART 초기화

  waterpres = 4, nozzpos = 0; // 기본 수압 4단계, 노즐 가운데

  // 수압 4단계로 LED 초기화
  led |= (1 << LDa2 | 1 << LDa3 | 1 << LDa4 | 1 << LDa5);
  write_LED();

  // end initialize phase

  while (1) {

    /* ******************** */
    /*      Read Phase      */
    /* ******************** */

    // 이전 버튼 입력 저장
    prevbt = button;

    // 버튼, 온도, 수위 값 읽어오기
    button = read_ADC(ADC_SW_PIN);
    wtemp_val = read_ADC(ADC_SEAT_THM_PIN);
    stemp_val = read_ADC(ADC_WATER_THM_PIN);
    wlevel_val = read_ADC(ADC_WATER_LEVEL_PIN);

    // ADC 전압 값에 따라 버튼을 구분해 처리
    if (prevbt > 64 || button < 64) {
      // 이미 처리한 버튼
      // 또는 버튼 안 누름

    } else if (button < 192) { // 1번 버튼: 변좌 온도
      // 변좌 온도를 4단계 안에서 순환
      if (seattemp == 3) { // 3단계일 경우 0단계로 초기화
        seattemp = 0;
        led &= ~(1 << LDc1 | 1 << LDc2 | 1 << LDc3);
        write_LED();

      } else { // 2단계 이하면 한 단계 높이고 LED 추가 점등
        seattemp++;
        led |= ((led & (1 << LDc2)) ? (1 << LDc1) : 0);
        led |= ((led & (1 << LDc3)) ? (1 << LDc2) : 0);
        led |= (1 << LDc3);
        write_LED();
      }

    } else if (button < 320) { // 2번 버튼: 온수 온도
      // 온수 온도를 4단계 안에서 순환
      if (watertemp == 3) { // 3단계일 경우 0단계로 초기화
        watertemp = 0;
        led &= ~(1 << LDb1 | 1 << LDb2 | 1 << LDb3);
        write_LED();

      } else { // 2단계 이하면 한 단계 높이고 LED 추가 점등
        watertemp++;
        led |= ((led & (1 << LDb2)) ? (1 << LDb1) : 0);
        led |= ((led & (1 << LDb3)) ? (1 << LDb2) : 0);
        led |= (1 << LDb3);
        write_LED();
      }

    } else if (button < 448) { // 3번 버튼: 노즐 뒤로
      // 위치가 -1단계 이상일 경우 뒤로 한 칸 이동
      if (nozzpos > -2) {
        nozzpos--;
        nozzle_setdirection(BACK);
        for (int i = 0; i < 128; i++) {
          STEPPER_STEP_PORT |= (1 << STEPPER_STEP_PIN);
          _delay_us(500);
          STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_PIN);
          _delay_us(500);
        }
      }

      // 상태 표시 LED 업데이트
      led &= ~(1 << LDa1 | 1 << LDa2 | 1 << LDa3 | 1 << LDa4 | 1 << LDa5);
      led |= (1 << (LDa3 - nozzpos));
      write_LED();

    } else if (button < 576 && !wpflag) { // 4번 버튼: 수압 감소
      // 수압이 2단계 이상일 경우 한 단계 감소
      waterpres > 1 ? waterpres-- : waterpres;
      wpflag = 1;

      // 상태 표시 LED 업데이트
      led &= ~(1 << LDa1 | 1 << LDa2 | 1 << LDa3 | 1 << LDa4 | 1 << LDa5);
      switch (waterpres) {
      case 5:
        led |= (1 << LDa1);
      case 4:
        led |= (1 << LDa2);
      case 3:
        led |= (1 << LDa3);
      case 2:
        led |= (1 << LDa4);
      case 1:
        led |= (1 << LDa5);
      }
      write_LED();

    } else if (button < 704) { // 5번 버튼: 노즐 앞으로
      // 위치가 1단계 이하일 경우 앞으로 한 칸 이동
      if (nozzpos < 2) {
        nozzpos++;
        nozzle_setdirection(FRONT);
        for (int i = 0; i < 128; i++) {
          STEPPER_STEP_PORT |= (1 << STEPPER_STEP_PIN);
          _delay_us(500);
          STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_PIN);
          _delay_us(500);
        }
      }

      // 상태 표시 LED 업데이트
      led &= ~(1 << LDa1 | 1 << LDa2 | 1 << LDa3 | 1 << LDa4 | 1 << LDa5);
      led |= (1 << (LDa3 - nozzpos));
      write_LED();

    } else if (button < 832 && !wpflag) { // 6번 버튼: 수압 증가
      // 수압이 4단계 이하일 경우 한 단계 증가
      waterpres < 5 ? waterpres++ : waterpres;
      wpflag = 1;

      // 상태 표시 LED 업데이트
      led &= ~(1 << LDa1 | 1 << LDa2 | 1 << LDa3 | 1 << LDa4 | 1 << LDa5);
      switch (waterpres) {
      case 5:
        led |= (1 << LDa1);
      case 4:
        led |= (1 << LDa2);
      case 3:
        led |= (1 << LDa3);
      case 2:
        led |= (1 << LDa4);
      case 1:
        led |= (1 << LDa5);
      }
      write_LED();

    } else if (button < 960) { // 7번 버튼: 건조
      // 세정 중이 아닐 경우 건조 모드로 변경
      if (state == ST_IDLE)
        state = ST_DRY;

      // 건조 모터 가동
      DC_PORT |= (1 << DC_PIN);

    } else { // (button < 1023) // 8번 버튼: 세정
      // 세정 중일 경우 무브 세정 모드와 토글
      if (state == ST_WASH_MOVE) {
        // 무브세정 중 일반세정으로 변경
        // 기본 위치로 돌아가는 방향으로 설정
        if (mvoffset > 0)
          nozzle_setdirection(BACK);
        else
          nozzle_setdirection(FRONT);

        // 기본 위치로 이동
        for (int i = (mvoffset > 0 ? mvoffset : (-mvoffset)); i > 0; i--) {
          STEPPER_STEP_PORT |= (1 << STEPPER_STEP_PIN);
          _delay_us(500);
          STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_PIN);
          _delay_us(500);
        }

        mvoffset = 0;
        state = ST_WASH;

      } else if (state == ST_WASH) {
        // 일반세정 중 무브세정으로 변경
        state = ST_WASH_MOVE;
        mvdir = FRONT;
        mvoffset = 0;
        nozzle_setdirection(FRONT);
      }

      // 정지 중일 경우 세정 모드 진입하고 노즐 사출, 세정 시작
      if (state == ST_IDLE) {
        state = ST_WASH;
        nozzle_move(NOZZLE_WASH);
        wpflag = 1;
      }

    } // end read phase

    /* ******************** */
    /*    Execute  Phase    */
    /* ******************** */

    if (wpflag) { // 수압 변동, 세정 모드에서만 적용
      if (state == ST_WASH || state == ST_WASH_MOVE)
        rotate_servo(waterpres);
      wpflag = 0;
    }

    if (state == ST_WASH_MOVE) { // 무브세정 움직임: +-22.5도

      // 끝에 도달하면 방향 전환
      if (mvoffset >= 128) {
        mvdir = BACK;
      } else if (mvoffset <= -128) {
        mvdir = FRONT;
      }

      // 노즐 이동
      nozzle_setdirection(mvdir);
      mvoffset += (mvdir == FRONT ? 16 : (-16));
      for (int i = 0; i < 16; i++) {
        STEPPER_STEP_PORT |= (1 << STEPPER_STEP_PIN);
        _delay_us(500);
        STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_PIN);
        _delay_us(500);
      }
    }

    if (wlevel_val > 250) // 수위가 일정 이상 일떄만 온수 작동
      water_temp_control();

    seat_temp_control(); // 변좌 온열 작동

  } // end execute phase
}
