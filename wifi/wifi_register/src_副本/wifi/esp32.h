#ifndef __ESP32_H
#define __ESP32_H
#include <stm32f1xx.h>
#include "stdio.h"
#include "string.h"
#include "../usart/usart3.h"
#include "../delay/delay.h"
/**
 * esp32 c3 模组连接 stm32说明
 * TX/RX 可通过 P8 排针，接入 PB11/PB10（即串口 3）
 * esp32 c3作为从机，stm32作为主机，配合官方的AT指令+串口3操作
 */


// 初始化ESP32
void esp32_init(void);

/**
 * @description: 发送AT指令
 * @param {uint8_t} *cmd 要发送的AT指令(指令必须以 \r\n结束)
 * @param {uint16_t} cmd_length 指令长度
 */
void esp32_send_cmd(uint8_t *cmd, uint16_t cmd_length);

/**
 * @description: 发送AT指令后，用来接收响应
 *          要考虑到接收的响应为非定长数据。
 *
 * @param {uint8_t} respone_buff[] 存储接收的的响应的缓冲区。
 * @param {unin16_t} Size 缓冲区的长度
 */

void esp32_read_response(uint8_t respone_buff[], uint16_t size);

#endif