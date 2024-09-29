#ifndef __TEMP_CONTROL_H__
#define __TEMP_CONTROL_H__

#define WTEMP1 65
#define WTEMP2 70
#define WTEMP3 75

#define STEMP1 65
#define STEMP2 70
#define STEMP3 75

// 수온 조절
void water_temp_control(void) {
  if (watertemp == 1) { // 온도 1단계
    if (wtemp_val < WTEMP1) {
      WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
    } else {
      WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
    }

  } else if (watertemp == 2) { // 온도 2단계
    if (wtemp_val < WTEMP2) {
      WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
    } else {
      WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
    }

  } else if (watertemp == 3) { // 온도 3단계
    if (wtemp_val < WTEMP3) {
      WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
    } else {
      WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
    }

  } else { // 온도 0단계
    WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
  }
}

// 변좌 온도 조절
void seat_temp_control(void) {
  if (seattemp == 1) { // 온도 1단계
    if (stemp_val < STEMP1) {
      SEAT_HEAT_PORT |= (1 << SEAT_HEAT_PIN);
      UART_printString("on temp1");
      UART_printString("\n");
    } else {
      SEAT_HEAT_PORT &= ~(1 << SEAT_HEAT_PIN);
      UART_printString("off temp1");
      UART_printString("\n");
    }

  } else if (seattemp == 2) { // 온도 2단계
    if (stemp_val < STEMP2) {
      SEAT_HEAT_PORT |= (1 << SEAT_HEAT_PIN);
      UART_printString("on temp2");
      UART_printString("\n");
    } else {
      SEAT_HEAT_PORT &= ~(1 << SEAT_HEAT_PIN);
      UART_printString("off temp2");
      UART_printString("\n");
    }

  } else if (seattemp == 3) { // 온도 3단계
    if (stemp_val < STEMP3) {
      SEAT_HEAT_PORT |= (1 << SEAT_HEAT_PIN);
      UART_printString("on temp3");
      UART_printString("\n");
    } else {
      SEAT_HEAT_PORT &= ~(1 << SEAT_HEAT_PIN);
      UART_printString("off temp3");
      UART_printString("\n");
    }

  } else { // 온도 0단계
    SEAT_HEAT_PORT &= ~(1 << SEAT_HEAT_PIN);
  }
}

#endif