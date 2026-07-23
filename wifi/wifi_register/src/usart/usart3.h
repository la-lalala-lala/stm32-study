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

/**
 * 初始化USART3  完成相关配置 能够调用下面收发数据的代码
 */
void Driver_USART3_Init(void);

/**
 * 发送一个字节
 * uint8_t ch: 需要发送的一个字节
 */
void Driver_USART3_SendChar(uint8_t ch);

/**
 * 接收一个字节
 * return: uint8_t 接收到的字节
 */
uint8_t Driver_USART3_ReceiveChar(void);

/**
 * 发送多个字节
 * uint8_t *str: 一个字符串
 * uint8_t len: 字符串长度
 */
void Driver_USART3_SendString(uint8_t *str,uint8_t len);

/**
 * 接收多个字节  ->  由于不能确定接收数据的长度 ->  推荐buff够大
 * 收到空闲位结束
 * uint8_t buff[]: 存接收到的数据
 * uint8_t * len: 实际接收数据的长度 -> 需要函数中赋值
 */
void Driver_USART3_ReceiveString(uint8_t buff[],uint8_t * len);
#endif
