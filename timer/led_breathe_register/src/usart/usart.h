#ifndef __USART_H
#define __USART_H
#include <stm32f1xx.h>
#include <stdio.h>

// 初始化
//void uart_init(uint32_t pclk2,uint32_t bound);

// 初始化
void init_usart(void);

// 发送一个字符
void usart_send(uint8_t ch);

// 接收一个字符
uint8_t usart_receive(void);

// 发送字符串
void usart_send_string(uint8_t *str,uint8_t size);

// 接收字符串
void usart_receive_string(uint8_t buffer[],uint8_t *size);
#endif