#include "led.h"

// 初始化
void LED_Init(void)
{
    // 1. 时钟配置
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	
	// 2. GPIO工作模式配置
	GPIOA->CRL &= ~GPIO_CRL_CNF0;
	GPIOA->CRL |= GPIO_CRL_MODE0;
	
	GPIOA->CRL &= ~GPIO_CRL_CNF1;
	GPIOA->CRL |= GPIO_CRL_MODE1;
	
	GPIOA->CRH &= ~GPIO_CRH_CNF8;
	GPIOA->CRH |= GPIO_CRH_MODE8;
	
	
	// 3. PA0、PA1、PA8输出高电平，默认关灯
    LED_Off(LED1);
    LED_Off(LED2);
    LED_Off(LED3);
}

// 控制某个LED的开关
void LED_On(uint16_t led)
{
    GPIOA->ODR &= ~led;
}
void LED_Off(uint16_t led)
{
    GPIOA->ODR |= led;
}

// 翻转LED状态
void LED_Toggle(uint16_t led)
{
    // 需要先判断当前LED状态，读取IDR对应位
    if ((GPIOA->IDR & led) == 0)
    {
        LED_Off(led);
    }
    else
    {
        LED_On(led);
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
