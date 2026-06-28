#include "tim6.h"


void TIM6_Init(void)
{
    // 1. 开启时钟
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

    // 2. 设置预分频7199，做7200分频，得到 10000Hz
    TIM6->PSC = 7199; 

    // 3. 设置自动重装载值9999，表示计数1000次 产生一个UEV
    TIM6->ARR = 9999;

    // 4. 更新中断使能
    TIM6->DIER |= TIM_DIER_UIE;

    // 5. NVIC配置
    NVIC_SetPriorityGrouping(3);// 设置分组3，表示4位抢占优先级，0位响应优先级
    NVIC_SetPriority(TIM6_IRQn, 2);// 设置TIM6中断优先级为2
    NVIC_EnableIRQ(TIM6_IRQn);// 使能TIM6中断

    // 6.开启定时器
    TIM6->CR1 |= TIM_CR1_CEN;
}

void TIM6_IRQHandler(void){
    // 清除中断标志位
    TIM6->SR &= ~TIM_SR_UIF;

    // 翻转LED2
    LED_Toggle(LED0);
}
