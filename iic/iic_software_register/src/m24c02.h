#ifndef __M24C02_H
#define __M24C02_H

#include "i2c.h"

// 宏定义
#define W_ADDR 0xA0
#define R_ADDR 0xA1

// 初始化
void M24C02_Init(void);

// 向EEPROM写入一个字节
void M24C02_WriteByte(uint8_t innerAddr, uint8_t byte);

// 读取EEPROM的一个字节
uint8_t M24C02_ReadByte(uint8_t innerAddr);

// 连续写入多个字节（页写）
void M24C02_WriteBytes(uint8_t innerAddr, uint8_t * bytes, uint8_t size);

// 连续读取多个字节
void M24C02_ReadBytes(uint8_t innerAddr, uint8_t * buffer, uint8_t size);

#endif