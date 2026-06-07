#include "usart/usart.h"
#include "delay/delay.h"
#include <string.h>
#include "led/led.h"
#include "systick/systick.h"

/**
 * PA9作为TX，PA10作为RX使用
 */

// 定义全局变量，接收缓冲区和size
uint8_t buffer[100] = {0};
uint8_t size = 0;

int main(void)
{
	init_usart();
	// 初始化
	init_usart();
	printf("串口通信测试\n");
	LED_Init();
	SysTick_Init();
	// 测试LED闪烁
	while(1)
	{
	}
}
