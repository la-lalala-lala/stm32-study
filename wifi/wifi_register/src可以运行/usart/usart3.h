#ifndef __USART3_H
#define __USART3_H
#include <stm32f1xx.h>
#include <stdio.h>

/**
 * 正点原子战舰 STM32F103 串口3 操作单元
 * P8 选择 ATK 模块接口时：
 * PB10(USART3_TX) -> ATK-MODULE RXD
 * PB11(USART3_RX) <- ATK-MODULE TXD
 */

// 初始化
void usart3_init(void);

// 发送一个字符
uint8_t usart3_send(uint8_t ch);

// 接收一个字符
uint8_t usart3_receive(uint8_t *ch, uint32_t timeout);

// 发送字符串
uint8_t usart3_send_string(const uint8_t *str,uint16_t size);

// 接收字符串
uint16_t usart3_receive_string(uint8_t buffer[],uint16_t size);

// 清空接收寄存器和错误状态
void usart3_flush_rx(void);

// 调试信息
uint32_t usart3_debug_status(void);
uint8_t usart3_rx_pin_level(void);
uint8_t usart3_tx_pin_level(void);
const char *usart3_debug_mode(void);
#endif
