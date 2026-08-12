#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f1xx.h"

void Delay_us(uint16_t us);
void Delay_ms(uint16_t ms);
void Delay_s(uint16_t s);
uint32_t Delay_GetCycleCount(void);
uint8_t Delay_TimeoutElapsed(uint32_t start_cycles, uint32_t timeout_ms);

#endif
