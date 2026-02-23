/*
 * @Author: wushengran
 * @Date: 2024-05-23 15:14:48
 * @Description:
 *
 * Copyright (c) 2024 by atguigu, All Rights Reserved.
 */
#include "usart.h"
#include "m24c02.h"
#include <string.h>

int main(void)
{
	// 1. 初始化
	USART_Init();
	M24C02_Init();

	printf("尚硅谷I2C软件模拟实验开始...\n");

	// 2. 向EEPROM依次写入单个字符
	M24C02_WriteByte(0x00, 'a');
	M24C02_WriteByte(0x01, 'b');
	M24C02_WriteByte(0x02, 'c');

	// 3. 读取字符
	uint8_t byte1 = M24C02_ReadByte(0x00);
	uint8_t byte2 = M24C02_ReadByte(0x01);
	uint8_t byte3 = M24C02_ReadByte(0x02);

	// 4. 串口输出打印
	printf("byte1 = %c\t byte2 = %c\t byte3 = %c\n", byte1, byte2, byte3);

	// 5. 写入多个字符
	M24C02_WriteBytes(0x00, "123456", 6);

	// 6. 读取多个字符
	uint8_t buffer[100] = {0};
	M24C02_ReadBytes(0x00, buffer, 6);

	// 7. 串口打印
	printf("buffer = %s\n", buffer);

	// 8. 测试超出16个字节的写入
	// 清零缓冲区
	memset(buffer, 0, sizeof(buffer));

	M24C02_WriteBytes(0x00, "1234567890abcdefghijk", 21);
	M24C02_ReadBytes(0x00, buffer, 21);
	printf("buffer = %s\n", buffer);

	while (1)
	{
	}
}
