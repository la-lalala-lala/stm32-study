#ifndef __LED_H
#define __LED_H

#include <stm32f1xx.h>

// 宏定义 正点原子 LED0 – PB5
#define LED0 GPIO_ODR_ODR5

// 初始化
void LED_Init(void);

// 控制某个LED的开关
void LED_On(uint16_t led);
void LED_Off(uint16_t led);

// 翻转LED状态
void LED_Toggle(uint16_t led);

// 对一组LED灯，全开全关
void LED_OnAll(uint16_t leds[], uint8_t size);
void LED_OffAll(uint16_t leds[], uint8_t size);

#endif