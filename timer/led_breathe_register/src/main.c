#include "usart/usart.h"
#include "delay/delay.h"
#include <string.h>
#include "tim/tim3.h"

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
	TIM3_Start();
	uint8_t dutyCycle = 0;
	uint8_t dir = 0;
	TIM3_SetDutyCycle(dutyCycle);
	while(1)
	{
        if (dir == 0)
        {
            dutyCycle += 1;
            if (dutyCycle >= 99)
            {
                dir = 1;
            }
        }
        else
        {
            dutyCycle -= 1;
            if (dutyCycle <= 1)
            {
                dir = 0;
            }
        }

        TIM3_SetDutyCycle(dutyCycle);
        Delay_ms(10);
	}
}