#ifndef __TIM4_H__
#define __TIM4_H__
#include <stm32f1xx.h>

void TIM4_Init(void);

void TIM4_Start(void);

void TIM4_Stop(void);

// 返回PWM的周期 ms
double TIM4_GetPWMCycle(void);

// 返回PWM的频率
double TIM4_GetPWMFreq(void);

#endif
