#ifndef __USART_H
#define __USART_H

#include "stm32f1xx.h"

// 初始化
void USART_Init(void);

// 发送一个字符
void USART_SendChar(uint8_t ch);

// 接收一个字符
uint8_t USART_ReceiveChar(void);

// 发送字符串
void USART_SendString(uint8_t *str, uint8_t size);

// 接收字符串
void USART_ReceiveString(uint8_t buffer[], uint8_t *size);

#endif