/*
 * @Author: wushengran
 * @Date: 2024-05-24 15:39:25
 * @Description: 
 * 
 * Copyright (c) 2024 by atguigu, All Rights Reserved. 
 */
#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

// 宏定义
#define LED1 GPIO_ODR_ODR0
#define LED2 GPIO_ODR_ODR1
#define LED3 GPIO_ODR_ODR8

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
