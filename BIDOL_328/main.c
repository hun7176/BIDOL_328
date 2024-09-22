#define F_CPU 16000000L
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#include "servo_pwm.h"
#include "water_sensor.h"
#include "UART.h"

/* state 이름 정의*/
#define ST_IDLE 0       // 정지 상태
#define ST_POWER_SAVE 1 // 절전 상태
#define ST_WASH 2       // 세정 상태
#define ST_WASH_MOVE 3  // 무브세정 상태
#define ST_DRY 4        // 건조 상태

/* 핀 번호 매크로 */
#define ADC_PIN 0 // PA0: ADC 스위치 리더

#define SRCK_PIN 1 // PA1: 74595 레지스터 시프트 클록 신호 핀
#define RCK_PIN 2  // PA2: 74595 래치 클록 신호 핀
#define SER_PIN 3  // PA3: 74595 데이터 전송 핀

#define HEAT_PIN 4    // PA4: 히터 제어 핀
#define SERVO_PIN 5   // PA5: 서보모터 PWM 제어 핀
#define STEPCK_PIN 6  // PA6: 스텝모터 클록
#define STEPDIR_PIN 7 // PA7: 스텝모터 방향, HIGH면 CCW

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

void ADC_Init(unsigned char channel); // ADC 초기화
void GPIO_Init(void);                 // GPIO 초기화
int read_ADC(void);                   // ADC 값 읽기
void write_LED(void);                 // LED 상태 시리얼 전송

volatile uint16_t led = 0;    // 74595로 전송할 LED 데이터
volatile int state = ST_IDLE; // 현재 상태
volatile int watertemp = 0;   // 현재 온수 단계 (0~4)
volatile int seattemp = 0;    // 현재 변좌 온도 단계 (0~4)
volatile int waterpres = 0;   // 현재 수압 단계 (1~5)
volatile int nozzpos = 0; // 현재 노즐 위치 (-2~2), 높은 숫자가 front

int main(void) {
	volatile int button = 0;
	volatile int prevbt = 0;
	GPIO_Init();       // GPIO 초기화
	ADC_Init(ADC_PIN); // ADC0 초기화
	write_LED();
	INIT_TIMER1();

	waterpres = 3, nozzpos = 0;

	while (1) {
		prevbt = button;
		button = read_ADC(); // 버튼이 연결된 ADC 읽기

		// ADC 전압 값에 따라 버튼을 구분해 상태 변경
		if (prevbt > 64 || button < 64) { // 이미 처리한 버튼 또는 버튼 안 누름

			} else if (button < 192) { // 1번 버튼: 변좌 온도
			// 변좌 온도를 4단계 안에서 순환
			if (seattemp == 3) { // 3단계일 경우 0단계로 초기화
				seattemp = 0;
				led &= ~(1 << LDc1 | 1 << LDc2 | 1 << LDc3);
				write_LED();
				} else { // 한 단계 높이고 LED 추가 점등
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
				} else { // 한 단계 높이고 LED 추가 점등
				watertemp++;
				led |= ((led & (1 << LDb2)) ? (1 << LDb1) : 0);
				led |= ((led & (1 << LDb3)) ? (1 << LDb2) : 0);
				led |= (1 << LDb3);
				write_LED();
			}

			} else if (button < 448) { // 3번 버튼: 노즐 뒤로
			// 위치가 -1단계 이상일 경우 뒤로 한 칸 이동
			nozzpos > -2 ? nozzpos-- : nozzpos;
			led &= ~(1 << LDa1 | 1 << LDa2 | 1 << LDa3 | 1 << LDa4 | 1 << LDa5);
			led |= (1 << (LDa3 - nozzpos));
			write_LED();

			} else if (button < 576) { // 4번 버튼: 수압 감소
			// 수압이 2단계 이상일 경우 한 단계 감소
			waterpres > 1 ? waterpres-- : waterpres;
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
			nozzpos < 2 ? nozzpos++ : nozzpos;
			led &= ~(1 << LDa1 | 1 << LDa2 | 1 << LDa3 | 1 << LDa4 | 1 << LDa5);
			led |= (1 << (LDa3 - nozzpos));
			write_LED();

			} else if (button < 832) { // 6번 버튼: 수압 증가
			// 수압이 4단계 이하일 경우 한 단계 증가
			waterpres < 5 ? waterpres++ : waterpres;
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
			// TODO: dry timer start, dry motor start

			} else { // button < 1023 // 8번 버튼: 세정
			// 정지 중이거나 무브 세정 중일 경우 세정 모드로 변경
			if (state == ST_IDLE || state == ST_WASH_MOVE)
			state = ST_WASH;

			// 세정 중일 경우 무브 세정 모드로 변경
			else if (state == ST_WASH)
			state = ST_WASH_MOVE;

			// TODO: if idle-> wash timer start
			// stepper&servo motor will controlled in swtich statement
			// with their own value
		} // end button if-statement

		// 버튼을 누르지 않아도 계속 진행해야 할 일들
		if (state == ST_WASH_MOVE) {
			// 스텝모터 이동(nozzpos 기준으로 앞뒤로)
		}
		if (state == ST_WASH || state == ST_WASH_MOVE) {
			// 서보모터 조절(waterpres)
			// 스텝모터 조절(nozzpos)
		}
		// TODO: 변좌온도(seattemp), 온수온도(watertemp) 모니터링해 히터 켜기/끄기

	} // end while
}

void ADC_Init(unsigned char channel) {
	// AVCC를 기준 전압으로 설정
	ADMUX &= ~(1 << REFS1);
	ADMUX &= ~(1 << REFS0);

	// 프리스케일러 128로 설정
	ADCSR |= 0x07;
	// ADC 활성화
	ADCSR |= (1 << ADEN);
	// 프리러닝 모드
	ADCSR |= (1 << ADFR);

	// 채널 선택
	ADMUX = ((ADMUX & 0xE0) | channel);

	// 변환 시작
	ADCSR |= (1 << ADSC);
}

void GPIO_Init(void) {
	DDRA = 0xFE;      // PA0은 입력, PA1~PA7은 출력
	PORTA &= ~(0xFE); // 출력 핀 초기화

	// TODO: PWM 초기화 코드 추가

	PORTA |= (1 << 7); // 스텝모터 방향을 CCW로 설정
}

int read_ADC(void) {
	while (!(ADCSR & (1 << ADIF)))
	;         // 변환 완료 대기
	return ADC; // 변환 완료시 10비트 값 반환
}

void write_LED(void) {
	for (int i = 15; i >= 0; i--) {
		// 시리얼 데이터 설정
		if (led & (1 << i))
		PORTA |= (1 << SER_PIN);
		else
		PORTA &= ~(1 << SER_PIN);

		// 시리얼 클록 신호 생성
		PORTA |= (1 << SRCK_PIN);
		PORTA &= ~(1 << SRCK_PIN);
	}

	// 래치 클록 신호 생성
	PORTA |= (1 << RCK_PIN);
	PORTA &= ~(1 << RCK_PIN);
}



rotate_servo(3);