#ifndef __DRIVER_ESP32_H
#define __DRIVER_ESP32_H
#include <stm32f1xx.h>
#include "stdio.h"
#include "string.h"
#include "../usart/usart3.h"
#include "../delay/delay.h"

#define RESPONSE_OK "OK"

#define ESP32_SUCCESS 1U
#define ESP32_ERROR   0U

/**
 * esp32 c3 模组连接 stm32说明
 * ATK 模块接口通过 P8 选择接入串口 3：
 * 模块 TXD -> PB11(USART3_RX)
 * 模块 RXD <- PB10(USART3_TX)
 * esp32 c3作为从机，stm32作为主机，配合官方的AT指令+串口3操作
 */


/**
 * @description: 初始化ESP32（wifi）
 */
uint8_t ESP32_Wifi_Init(void);

/**
 * @description: 初始化ESP32（ble）
 */
uint8_t ESP32_Ble_Init(void);

/**
 * @description: 发送AT指令
 * @param cmd 要发送的AT指令(指令必须以 \r\n结束)
 * @param expect_result 期待的响应字符串
 * @param timeout_ms 等待响应的总超时时间
 */
uint8_t ESP32_Send_CMD(const char *cmd, const char *expect_result,
                       uint32_t timeout_ms);

/**
 * @description: 发送AT指令后，用来接收响应
 *          要考虑到接收的响应为非定长数据。
 *
 * @param response_buffer 存储接收响应的缓冲区
 * @param size 缓冲区的长度
 * @param timeout_ms 等待首字节的超时时间
 */

uint16_t ESP32_ReadResponse(uint8_t response_buffer[], uint16_t size,
                            uint32_t timeout_ms);


/**
 * @description: 接收TCP服务端的数据，串口3将数据从esp32 c3发送给stm32
 * @param rxbuff 缓冲区
 * @param max_size 缓冲区最大长度
 * @param real_receive_len 实际接收的长度
 * @param id 连接id
 * @param ip 客户端IP
 * @param port 客户端端口
 */
void Esp32_Read_Data(uint8_t rxbuff[],
                                uint16_t max_size,
                                uint16_t *real_receive_len,
                                uint8_t *id,
                                uint8_t ip[],
                                uint16_t *port);

/**
 * @description: 发送TCP服务端的数据，stm32将数据从串口3发送给esp32 c3
 * @param txbuff 缓冲区
 * @param tx_len 缓冲区长度
 * @param id TCP连接id
 */
void Esp32_Send_Data(uint8_t txbuff[],
                                uint16_t tx_len,
                                uint8_t *id);

#endif
