#ifndef __COMMON_H__
#define __COMMON_H__

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

/* 상태 상수 */
// 상수의 저장에 enum을 사용할 수 있지만, 메모리를 사용하는 enum 대신
// 전처리 단계에서 처리되는 매크로 상수를 사용함

// 무브세정
#define FRONT 0
#define BACK 1

// 노즐 위치
#define NOZZLE_IDLE 0
#define NOZZLE_WASH 1

/* state 정의*/
#define ST_IDLE 0       // 정지 상태
#define ST_POWER_SAVE 1 // 절전 상태
#define ST_WASH 2       // 세정 상태
#define ST_WASH_MOVE 3  // 무브세정 상태
#define ST_DRY 4        // 건조 상태

/* Port/Pin 정의 */
// ADC
#define ADC_SW_PIN 0          // ADC0: ADC 스위치 리더
#define ADC_SEAT_THM_PIN 3    // ADC3: 변좌 온도 센서
#define ADC_WATER_THM_PIN 4   // ADC4: 온수 온도 센서
#define ADC_WATER_LEVEL_PIN 5 // ADC5: 수위 센서

// 74595 IC
#define SRCK_DDR DDRC   // PC1: 74595 레지스터 시프트 클록 신호 핀
#define SRCK_PORT PORTC //
#define SRCK_PIN 1      //
#define RCK_DDR DDRC    // PC2: 74595 래치 클록 신호 핀
#define RCK_PORT PORTC  //
#define RCK_PIN 2       //
#define SER_DDR DDRD    // PD0: 74595 데이터 전송 핀
#define SER_PORT PORTD  //
#define SER_PIN 0       //

// 히터
#define SEAT_HEAT_PORT PORTD  // PD7: 변좌 온열 히터
#define SEAT_HEAT_DDR DDRD    //
#define SEAT_HEAT_PIN 7       //
#define WATER_HEAT_PORT PORTB // PB0: 온수 온열 히터
#define WATER_HEAT_DDR DDRB   //
#define WATER_HEAT_PIN 0      //

// 모터(스테퍼, 서보, DC)
#define SERVO_PORT PORTB        // PB1: 서보모터 PWM
#define SERVO_DDR DDRB          //
#define SERVO_PIN 1             //
#define STEPPER_STEP_PORT PORTD // PD4: 스테퍼모터 스텝
#define STEPPER_STEP_DDR DDRD   //
#define STEPPER_STEP_PIN 4      //
#define STEPPER_DIR_PORT PORTD  // PD5: 스테퍼모터 방향
#define STEPPER_DIR_DDR DDRD    //
#define STEPPER_DIR_PIN 5       //
#define DC_PORT PORTD           // PD6: DC모터 가동
#define DC_DDR DDRD             //
#define DC_PIN 6                //

// 외부 인터럽트 버튼
#define SEAT_SENSOR_PORT PORTD // PD2(INT0): 착석 센서
#define SEAT_SENSOR_DDR DDRD   //
#define SEAT_SENSOR_PIN 2      //
#define STOP_BT_PORT PORTD     // PD3(INT1): 정지 버튼
#define STOP_BT_DDR DDRD       //
#define STOP_BT_PIN 3          //

/* LED 시리얼 데이터 비트 번호 */
#define LDaBASE 0
#define LDa1 1
#define LDa2 2
#define LDa3 3
#define LDa4 4
#define LDa5 5

#define LDbBASE 8
#define LDb1 9
#define LDb2 10
#define LDb3 11

#define LDcBASE 11
#define LDc1 12
#define LDc2 13
#define LDc3 14

#endif