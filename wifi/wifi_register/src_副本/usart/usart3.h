#ifndef __USART3_H
#define __USART3_H
#include <stm32f1xx.h>
#include <stdio.h>

/**
 * 正点原子战舰 STM32F103 串口3 操作单元
 * 通过STM32F103ZET6 的串口3 RX/TX -> PB10/PB11
 */

// 初始化
void usart3_init(void);

// 发送一个字符
void usart3_send(uint8_t ch);

// 接收一个字符
uint8_t usart3_receive(void);

// 发送字符串
void usart3_send_string(uint8_t *str,uint8_t size);

// 接收字符串
void usart3_receive_string(uint8_t buffer[],uint8_t *size);
#endif