#ifndef __TEMP_CONTROL_H__
#define __TEMP_CONTROL_H__

#define WTEMP1 65
#define WTEMP2 70
#define WTEMP3 75

#define STEMP1 65
#define STEMP2 70
#define STEMP3 75

void water_temp_control(void) {
  if (watertemp == 1) { // 65로 만들기
    if (wtemp_val < WTEMP1) {
      WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
    } else {
      WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
    }
  } else if (watertemp == 2) {
    if (wtemp_val < WTEMP2) {
      WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
    } else {
      WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
    }

  } else if (watertemp == 3) {
    if (wtemp_val < WTEMP3) {
      WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
    } else {
      WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
    }
  }
}
void seat_temp_control(void) {
  if (seattemp == 1) { // 65로 만들기
    if (stemp_val < STEMP1) {
      WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
      UART_printString("on temp1");
      UART_printString("\n");
    } else {
      WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
      UART_printString("off temp1");
      UART_printString("\n");
    }
  } else if (seattemp == 2) {
    if (stemp_val < STEMP2) {
      WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
      UART_printString("on temp2");
      UART_printString("\n");
    } else {
      WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
      UART_printString("off temp2");
      UART_printString("\n");
    }

  } else if (seattemp == 3) {
    if (stemp_val < STEMP3) {
      WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
      UART_printString("on temp3");
      UART_printString("\n");
    } else {
      WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
      UART_printString("off temp3");
      UART_printString("\n");
    }
  }
}

#endif /* WATERTEMP_CONTROL_H_ */
