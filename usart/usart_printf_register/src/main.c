#include "usart.h"
#include "delay.h"
#include <string.h>

/**
 * PA9作为TX，PA10作为RX使用
 */

// 定义全局变量，接收缓冲区和size
uint8_t buffer[100] = {0};
uint8_t size = 0;

// 1. 配置 72MHz 时钟 (针对 8MHz 外部晶振)
void SystemClock_Config(void) {
    // 启动 HSE (外部高速晶振)
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    // Flash 等待周期设置
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    // PLL 设置: HSE 源, 9 倍频 (8 * 9 = 72MHz)
    RCC->CFGR |= RCC_CFGR_PLLSRC;       // HSE 选择为 PLL 源 (F103 中通常是 bit 16)
    RCC->CFGR |= RCC_CFGR_PLLMULL9;    // 9 倍频
    
    // APB1 分频设置: HCLK/2 = 36MHz (STM32F103 APB1 最大 36MHz)
    RCC->CFGR |= RCC_CFGR_PPRE1_2;
    
    // 启动 PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // 切换系统时钟到 PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while (((RCC->CFGR >> 2) & 0x03) != 0x02);
}

int main(void)
{
	SystemClock_Config();
	// 初始化
	USART_Init();

	// 发送单个字符
	USART_SendChar('a');
	USART_SendChar('t');
	USART_SendChar('\n');

	// 发送字符串
	uint8_t * str = "Hello, World!\n";
	USART_SendString(str, strlen((char *)str));

	while(1)
	{
		// // 不停发送字符
		// USART_SendChar('x');
		// USART_SendChar('\n');

		// Delay_ms(1000);

		// 接收字符，再发回来
		// uint8_t ch = USART_ReceiveChar();
		// USART_SendChar(ch - 32);

		// 接收字符串，再发回来
		// USART_ReceiveString(buffer, &size);
		// USART_SendString(buffer, size);
	}
}
