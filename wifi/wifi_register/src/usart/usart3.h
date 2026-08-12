#ifndef __USART3_H
#define __USART3_H
#include <stm32f1xx.h>
#include <stdio.h>
#include "../delay/delay.h"

/**
 * 正点原子战舰 STM32F103 串口3 操作单元
 * P8 选择 ATK 模块接口时：
 * PB10(USART3_TX) -> ATK-MODULE RXD
 * PB11(USART3_RX) <- ATK-MODULE TXD
 * 驱动本身只处理通用 UART 数据，不依赖 MB026 或 AT 指令协议。
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
 * @brief 发送指定长度的数据
 * @param str 数据地址；由 len 指定长度，因此不要求以 '\0' 结尾
 * @param len 发送字节数
 */
void Driver_USART3_SendString(const char *str, uint16_t len);

/**
 * @brief 丢弃 USART3 中尚未处理的字节，并清除当前接收状态
 * @note 只应在明确开始一笔新事务且允许丢弃历史数据时调用；连续或异步接收时不要调用。
 */
void Driver_USART3_FlushReceive(void);

/**
 * @brief 轮询接收一批长度不确定的数据
 * @param buff 接收缓冲区
 * @param capacity 最多写入 buff 的字节数；函数不会在末尾自动添加 '\0'
 * @param first_byte_timeout_ms 从函数进入到收到首个有效字节的最长等待时间
 * @param inter_byte_timeout_ms 收到数据后允许的最大连续静默时间；超时即返回当前数据
 * @return 实际写入 buff 的字节数；首字节超时返回 0
 * @note 两个超时都不是整个报文的总超时。只要字节持续到达且缓冲区未满，接收就会继续。
 *       本函数为阻塞轮询接口，不适合持续高速数据或大量异步消息。
 */
uint16_t Driver_USART3_ReceiveString(uint8_t buff[], uint16_t capacity,
                                    uint32_t first_byte_timeout_ms,
                                    uint32_t inter_byte_timeout_ms);
#endif
