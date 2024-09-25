#define F_CPU 16000000L

#include "common.h"

// ADC.h
void ADC_Init(unsigned char channel); // ADC 초기화
int read_ADC(void);                   // ADC 값 읽기

// GPIO.h
void GPIO_Init(void); // GPIO 초기화
void EXTI_Init(void); // 외부 인터럽트 초기화
void write_LED(void); // LED 상태 시리얼 전송

// main.c
ISR(INT0_vect);                // 일어남
ISR(INT1_vect);                // 정지 버튼 작동
void nozzle_move(int nozdest); // 노즐 수납/사출

volatile uint16_t led = 0;    // 74595로 전송할 LED 데이터
volatile int state = ST_IDLE; // 현재 상태
volatile int watertemp = 0;   // 현재 온수 단계 (0~4)
volatile int seattemp = 0;    // 현재 변좌 온도 단계 (0~4)
volatile int waterpres = 0;   // 현재 수압 단계 (1~5)
volatile int nozzpos = 0; // 현재 노즐 위치 (-2~2), 높은 숫자가 front

volatile int processing = 0; // 버튼 입력 플래그
volatile int wtflag = 0;     // 수온 변화 플래그
volatile int stflag = 0;     // 변좌 온도 변화 플래그
volatile int wpflag = 0;     // 수압 변화 플래그
volatile int nzmvstop = 0;   // 무브세정 정지 플래그

volatile int mvoffset = 0;  // 무브세정 위치 오프셋
volatile int mvdir = FRONT; // 무브세정 이동중인 방향

#include "ADC.h"
#include "GPIO.h"
#include "UART.h"
#include "servo_pwm.h"
#include "water_sensor.h"
// volatile int dcflag = 0; // dc모터 속도 변화 플래그
// volatile int svflag = 0; // 서보모터 속도 변화 플래그

int main(void) {
  volatile int button = 0;
  volatile int prevbt = 0;
  GPIO_Init();          // GPIO 초기화
  ADC_Init(ADC_SW_PIN); // ADC0 초기화
  EXTI_Init();          // 외부 인터럽트 초기화
  TIMER1_Init();        // 서보모터 PWM 초기화
  UART_Init();          // 디버그용 UART 초기화
  write_LED();          // LED 초기화

  waterpres = 3, nozzpos = 0;
  UART_printString("Initialize complete\n");

  while (1) {
    prevbt = button;
    button = read_ADC(); // 버튼이 연결된 ADC 읽기

    // ADC 전압 값에 따라 버튼을 구분해 상태 변경
    if (processing || prevbt > 64 || button < 64) {
      // 이전 입력 처리중
      // 또는 이미 처리한 버튼
      // 또는 버튼 안 누름

    } else if (button < 192 && !stflag) { // 1번 버튼: 변좌 온도
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
      stflag = 1;

    } else if (button < 320 && !wtflag) { // 2번 버튼: 온수 온도
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
      wtflag = 1;

    } else if (button < 448) { // 3번 버튼: 노즐 뒤로
      // 위치가 -1단계 이상일 경우 뒤로 한 칸 이동
      if (nozzpos > -2) {
        nozzpos--;
        STEPPER_DIR_PORT |= (1 << STEPPER_DIR_PIN);
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
        STEPPER_DIR_PORT &= ~(1 << STEPPER_DIR_PIN);
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
      // TODO: dry timer start

    } else { // button < 1023 // 8번 버튼: 세정
      // 세정 중일 경우 무브 세정 모드와 토글
      if (state == ST_WASH_MOVE) {
        state = ST_WASH;
        mvoffset = 0;
        mvdir = FRONT;
        // npflag = 1; // 노즐 위치 원상 복구 필요
      } else if (state == ST_WASH)
        state = ST_WASH_MOVE;

      // 정지 중일 경우 세정 모드 진입하고 노즐 사출, 세정 시작
      if (state == ST_IDLE) {
        state = ST_WASH;
        nozzle_move(NOZZLE_WASH);
        wpflag = 1;
      }

      // TODO: if idle-> wash timer start
    } // end button if-statement

    /////////////////////////////////////////////
    /* 버튼을 누르지 않아도 계속 진행해야 할 일들 */
    if (wpflag) { // 수압 변화, 세정 모드에서만 적용
      if (state == ST_WASH || state == ST_WASH_MOVE)
        rotate_servo(waterpres);
      wpflag = 0;
    }
    if (nzmvstop) {
      // 기본 위치로 돌아가는 방향으로 설정
      if (mvoffset > 0)
        STEPPER_DIR_PORT |= (1 << STEPPER_DIR_PIN);
      else
        STEPPER_DIR_PORT &= ~(1 << STEPPER_DIR_PIN);

      // 기본 위치로 이동
      for (int i = (mvoffset > 0 ? mvoffset : -mvoffset); i > 0; i--) {
        STEPPER_STEP_PORT |= (1 << STEPPER_STEP_PIN);
        _delay_us(500);
        STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_PIN);
        _delay_us(500);
      }
      mvoffset = 0;
      nzmvstop = 0;
    }
    if (wtflag) { // 수온 변화
      wtflag = 0;
    }
    if (stflag) { // 변좌 온도 변화
      stflag = 0;
    }
    if (state == ST_WASH_MOVE) {
      // 무브세정 움직임: +-22.5도

      // 끝에 도달하면 방향 전환
      if (mvoffset >= 128) {
        mvdir = BACK;
        STEPPER_DIR_PORT &= ~(1 << STEPPER_DIR_PIN);
      } else if (mvoffset <= -128) {
        mvdir = FRONT;
        STEPPER_DIR_PORT |= (1 << STEPPER_DIR_PIN);
      }

      // 노즐 이동
      mvoffset += (mvdir == FRONT ? 16 : -16);
      for (int i = 0; i < 16; i++) {
        STEPPER_STEP_PORT |= (1 << STEPPER_STEP_PIN);
        _delay_us(500);
        STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_PIN);
        _delay_us(500);
      }
    }
    // TODO: 변좌온도(seattemp), 온수온도(watertemp) 모니터링해 히터 켜기 / 끄기
    // UART_printString(statestr);
    // UART_printInteger(button);
    // _delay_ms(1000);
  } // end while
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

void nozzle_move(int nozdest) {
  // 노즐을 수납 위치 / 세정 위치로 이동
  // 이미 해당 위치일 경우 아무 것도 하지 않음
  static int nozpos = NOZZLE_IDLE;

  if (nozpos == NOZZLE_IDLE && nozdest == NOZZLE_WASH) {
    STEPPER_DIR_PORT &= ~(1 << STEPPER_DIR_PIN);
    nozpos = NOZZLE_WASH;
  } else if (nozpos == NOZZLE_WASH && nozdest == NOZZLE_IDLE) {
    STEPPER_DIR_PORT |= (1 << STEPPER_DIR_PIN);
    nozpos = NOZZLE_IDLE;
  } else
    return;

  for (int i = 0; i < 1024; i++) {
    STEPPER_STEP_PORT |= (1 << STEPPER_STEP_PIN);
    _delay_us(500);
    STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_PIN);
    _delay_us(500);
  }
}