#include "usart/usart.h"
#include "delay/delay.h"
#include <string.h>
#include "led/led.h"
#include "tim/tim6.h"


int main(void)
{
	init_usart();
	// 初始化
	init_usart();
	printf("串口通信测试\n");
	LED_Init();
	TIM6_Init();
	while(1)
	{
	}
}