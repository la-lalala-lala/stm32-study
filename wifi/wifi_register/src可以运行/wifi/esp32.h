#ifndef __ESP32_H
#define __ESP32_H
#include <stm32f1xx.h>
#include "stdio.h"
#include "string.h"
#include "../usart/usart3.h"
#include "../delay/delay.h"
/**
 * esp32 c3 模组连接 stm32说明
 * ATK 模块接口：GBC_TX/GBC_RX 可通过 P8 排针接入 PB11/PB10（即串口 3）
 * 模块 TXD -> GBC_TX -> PB11(USART3_RX)
 * 模块 RXD <- GBC_RX <- PB10(USART3_TX)
 * esp32 c3作为从机，stm32作为主机，配合官方的AT指令+串口3操作
 */


// 初始化ESP32
void esp32_init(void);

/**
 * @description: 发送AT指令
 * @param {char} *cmd 要发送的AT指令(指令必须以 \r\n结束)
 * @param {uint16_t} cmd_length 指令长度
 */
void esp32_send_cmd(const char *cmd, uint16_t cmd_length);

/**
 * @description: 发送AT指令后，用来接收响应
 *          要考虑到接收的响应为非定长数据。
 *
 * @param {uint8_t} respone_buff[] 存储接收的的响应的缓冲区。
 * @param {uint16_t} size 缓冲区的长度
 */

uint16_t esp32_read_response(uint8_t respone_buff[], uint16_t size);

#endif
