#ifndef __USART1_H
#define __USART1_H
#include <stm32f1xx.h>
#include <stdio.h>

/**
 * 正点原子战舰 STM32F103 串口1 操作单元
 * 通过板载的 USB 串口或者 STM32F103ZET6 的串口1 RX/TX -> PA10/PA9
 */

// 初始化
void usart1_init(void);

// 发送一个字符
void usart1_send(uint8_t ch);

// 接收一个字符
uint8_t usart1_receive(void);

// 发送字符串
void usart1_send_string(uint8_t *str,uint8_t size);

// 接收字符串
void usart1_receive_string(uint8_t buffer[],uint8_t *size);
#endif