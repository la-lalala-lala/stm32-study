#include "systick.h"

void SysTick_Init(void){
    // 1. 设置重装载值，每1ms产生一次中断
    SysTick->LOAD = 71999;
    
    // 2. 配置时钟源，AHB = 72MHZ
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

    // 3. 使能中断
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

    // 4. 开启定时器
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

// 定义一个变量，用于计数（单位ms）
uint32_t ms_count = 0;

// 中断服务程序
void SysTick_Handler(void){
    ms_count++;
    // 如果达到了1000ms，就翻转LED0（PB5）
    if (ms_count == 1000)
    {
        LED_Toggle(LED0);
        // 清0
        ms_count = 0;
    }
}
