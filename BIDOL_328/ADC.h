#ifndef __ADC_H__
#define __ADC_H__

#include "common.h"

// ADC 초기화
void ADC_Init() {
  // AVCC를 기준 전압으로 설정
  ADMUX |= 0x40;

  // 프리스케일러 128로 설정
  ADCSRA |= 0x07;
  // ADC 활성화
  ADCSRA |= (1 << ADEN);
  // 자동 트리거/프리러닝 모드
  ADCSRA |= (1 << ADATE);
}

// 지정한 채널의 ADC 값 읽어서 반환
int read_ADC(unsigned char channel) {
  // 채널 선택
  ADMUX = ((ADMUX & 0xE0) | channel);

  // 변환 시작
  _delay_us(200);
  ADCSRA |= (1 << ADSC);

  while (!(ADCSRA & (1 << ADIF)))
    ;         // 변환 완료 대기
  return ADC; // 변환 완료시 10비트 값 반환
}

#endif