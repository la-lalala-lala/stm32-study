#include "usart/usart.h"
#include "delay/delay.h"
#include <string.h>
#include "tim/tim3.h"
#include "tim/tim4.h"
/**
 * TIM3_CH2的默认引脚是PA7，PB5是重映射引脚 。
 * 没有配置重映射，TIM3_CH2输出到默认引脚PA7
 * 需要配置部分重映射，TIM3_CH2输出到PB5
 */

int main(void)
{
	init_usart();
	// 初始化
	init_usart();
	printf("串口通信测试\n");
	TIM3_Init();
    TIM4_Init();
    TIM4_Start();
	TIM3_Start();
	while(1)
	{
        printf("T = %.2f ms，f = %.2f Hz\n", TIM4_GetPWMCycle(), TIM4_GetPWMFreq());
        Delay_ms(1000);
	}
}