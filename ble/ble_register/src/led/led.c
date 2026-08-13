#include "led.h"

// 初始化
void LED_Init(void)
{
    // 以下代码初始化正点原子发板LED0 - PB5
    // 1. 时钟配置
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	
	// 2. GPIO工作模式配置 - 配置PB5为推挽输出
	GPIOB->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5);
	GPIOB->CRL |= GPIO_CRL_MODE5;
	
	// 3. PB5输出高电平，默认关灯（低电平触发）
    LED_Off(LED0);
}

// 控制某个LED的开关
void LED_On(uint16_t led)
{
    GPIOB->ODR &= ~led;  // 低电平触发
}
void LED_Off(uint16_t led)
{
    GPIOB->ODR |= led;  // 高电平关闭
}

// 翻转LED状态
void LED_Toggle(uint16_t led)
{
    // 需要先判断当前LED状态，读取IDR对应位
    if ((GPIOB->IDR & led) == 0)
    {
        LED_Off(led);  // 当前是低电平（亮），需要关
    }
    else
    {
        LED_On(led);  // 当前是高电平（灭），需要开
    }  
}

// 对一组LED灯，全开全关
void LED_OnAll(uint16_t leds[], uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        LED_On(leds[i]);
    }
}

void LED_OffAll(uint16_t leds[], uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        LED_Off(leds[i]);
    }
}