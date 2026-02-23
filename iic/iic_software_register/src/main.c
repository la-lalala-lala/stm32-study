#include "usart.h"
#include "m24c02.h"

int main(void)
{
	
	// 1. 初始化
	init_usart();
	M24C02_Init();

	printf("I2C软件模拟实验开始...\n");

	// 2. 向EEPROM依次写入单个字符
	printf("开始写入数据...\n");
	M24C02_WriteByte(0x00, 'a');
	Delay_ms(10);
	M24C02_WriteByte(0x01, 'b');
	Delay_ms(10);
	M24C02_WriteByte(0x02, 'c');
	Delay_ms(10);
	printf("数据写入完成\n");

	// 3. 读取字符
	printf("开始读取数据...\n");
	uint8_t byte1 = M24C02_ReadByte(0x00);
	Delay_ms(10);
	uint8_t byte2 = M24C02_ReadByte(0x01);
	Delay_ms(10);
	uint8_t byte3 = M24C02_ReadByte(0x02);
	Delay_ms(10);
	printf("数据读取完成\n");

	// 4. 串口输出打印
	printf("byte1 = %c (0x%02X)\t byte2 = %c (0x%02X)\t byte3 = %c (0x%02X)\n", 
		byte1, byte1, byte2, byte2, byte3, byte3);

	// 5. 写入多个字符
	M24C02_WriteBytes(0x00, "123456", 6);

	// 6. 读取多个字符
	uint8_t buffer[100] = {0};
	M24C02_ReadBytes(0x00, buffer, 6);

	// 7. 串口打印
	printf("buffer = %s\n", buffer);

	// // 8. 测试超出16个字节的写入
	// // 清零缓冲区
	// memset(buffer, 0, sizeof(buffer));

	// M24C02_WriteBytes(0x00, "1234567890abcdefghijk", 21);
	// M24C02_ReadBytes(0x00, buffer, 21);
	// printf("buffer = %s\n", buffer);

	while (1)
	{
	}
}