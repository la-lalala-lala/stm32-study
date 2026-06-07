#include "usart/usart.h"
#include "delay/delay.h"
#include <string.h>
#include <stm32f1xx.h>
#include "tim/tim1.h"

/**
 * TIM3_CH2的默认引脚是PA7，PB5是重映射引脚 。
 * 没有配置重映射，TIM3_CH2输出到默认引脚PA7
 * 需要配置部分重映射，TIM3_CH2输出到PB5
 * 使用高级定时器的重复计数器，当计数器溢出时，在溢出中断中停止定时器工作。
 * 重复计数器寄存器的值设置为4，即可输出5个周期的PWM波，发光二极管会闪烁5次
 * 本例使用 PA8 引脚作为输出引脚 复用tim1
 */

int main(void)
{
	init_usart();
	TIM1_Init();
	printf("串口通信测试\n");
	TIM1_Start();
	while(1)
	{

	}
}