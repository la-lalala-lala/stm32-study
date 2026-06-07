#ifndef __TIM3_H__
#define __TIM3_H__
#include <stm32f1xx.h>

void TIM3_Init(void);

void TIM3_Start(void);

void TIM3_Stop(void);

void TIM3_SetDutyCycle(uint8_t dutyCycle);

#endif