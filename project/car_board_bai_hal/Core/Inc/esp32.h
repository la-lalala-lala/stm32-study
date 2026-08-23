#ifndef __ESP32_H
#define __ESP32_H

#include "usart.h"
#include "string.h"
#include "stdio.h"
#include <stdbool.h>

#define ESP32_RX_BUFFER_SIZE  2048U
#define ESP32_RX_WAIT_MS      100U

void ESP32_Init(void);
bool ESP32_Send_CMD(uint8_t *cmd, uint16_t cmdLength,uint32_t timeout_ms);
uint16_t ESP32_ReadResponse(uint8_t responeBuff[], uint16_t size,uint32_t timeout_ms);
#endif