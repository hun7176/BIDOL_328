#define F_CPU 16000000L

#include "common.h"

#include "UART.h"
#include "servo_pwm.h"
#include "water_sensor.h"

void ADC_Init(unsigned char channel); // ADC 초기화
void GPIO_Init(void);                 // GPIO 초기화
void EXTI_Init(void);                 // 외부 인터럽트 초기화
int read_ADC(void);                   // ADC 값 읽기
void write_LED(void);                 // LED 상태 시리얼 전송
ISR(INT1_vect);                       // 정지 버튼 작동

volatile uint16_t led = 0;    // 74595로 전송할 LED 데이터
volatile int state = ST_IDLE; // 현재 상태
volatile int watertemp = 0;   // 현재 온수 단계 (0~4)
volatile int seattemp = 0;    // 현재 변좌 온도 단계 (0~4)
volatile int waterpres = 0;   // 현재 수압 단계 (1~5)
volatile int nozzpos = 0; // 현재 노즐 위치 (-2~2), 높은 숫자가 front

int main(void) {
  volatile int button = 0;
  volatile int prevbt = 0;
  GPIO_Init();                // GPIO 초기화
  ADC_Init(ADC_SEAT_THM_PIN); // ADC0 초기화
  EXTI_Init();                // 외부 인터럽트 초기화
  TIMER1_Init();              // 서보모터 PWM 초기화
  UART_Init();                // 디버그용 UART 초기화
  write_LED();                // LED 초기화

  waterpres = 3, nozzpos = 0;
  UART_printString("Initialize complete\n");

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
      rotate_servo(waterpres);
      // 스텝모터 조절(nozzpos)
      UART_printString("WASHING\n");
    }
    // TODO: 변좌온도(seattemp), 온수온도(watertemp) 모니터링해 히터 켜기 / 끄기
    // UART_printString(statestr);
    UART_printInteger(button);
    // _delay_ms(1000);
  } // end while
}

void ADC_Init(unsigned char channel) {
  // AVCC를 기준 전압으로 설정
  ADMUX |= 0x40;

  // 프리스케일러 128로 설정
  ADCSRA |= 0x07;
  // ADC 활성화
  ADCSRA |= (1 << ADEN);
  // 자동 트리거/프리러닝 모드
  ADCSRA |= (1 << ADATE);

  // 채널 선택
  // TODO: 채널 변경 함수 만들기
  ADMUX = ((ADMUX & 0xE0) | channel);

  // 변환 시작
  ADCSRA |= (1 << ADSC);
}

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
  SERVO_PORT &= ~(1 << SERVO_PIN);
  STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_PIN);
  STEPPER_DIR_PORT &= ~(1 << STEPPER_DIR_PIN); // LOW = CW

  // 인터럽트 버튼
  // TODO: EXTI_Init()로 분리
}

int read_ADC(void) {
  while (!(ADCSRA & (1 << ADIF)))
    ;         // 변환 완료 대기
  return ADC; // 변환 완료시 10비트 값 반환
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

ISR(INT1_vect) { // 정지 버튼 작동
  // 노즐 닫기
  rotate_servo(0);

  // 노즐 수납하기
  // TODO

  // DC모터 팬 정지하기
  // TODO

  // 상태 바꾸기
  state = ST_IDLE;
}

void EXTI_Init(void) {              // 외부 인터럽트 초기화
  EIMSK |= (1 << INT0 | 1 << INT1); // INT0, INT1 인터럽트 활성화
  EICRA |= (1 << ISC00);            // 버튼 상태 변화 감지
  sei();                            // 전역 인터럽트 허용
}