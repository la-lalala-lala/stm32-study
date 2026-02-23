#include "key.h"

void Key_Init()
{
    // 1. 配置时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPFEN;
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    // 2. GPIO工作模式配置，PF10：CNF - 10，MODE - 00
    GPIOF->CRH &= ~GPIO_CRH_MODE10;
    GPIOF->CRH |= GPIO_CRH_CNF10_1;
    GPIOF->CRH &= ~GPIO_CRH_CNF10_0;

    GPIOF->ODR &= ~GPIO_ODR_ODR10;

    // 3. AFIO配置引脚复用选择
    AFIO->EXTICR[2] |= AFIO_EXTICR3_EXTI10_PF;

    // 4. 配置EXTI
    EXTI->RTSR |= EXTI_RTSR_TR10;
    EXTI->IMR |= EXTI_IMR_MR10;

    // 5. NVIC配置
    NVIC_SetPriorityGrouping(3);    // 全部都是抢占优先级
    NVIC_SetPriority(EXTI15_10_IRQn, 3);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// 中断服务程序
void EXTI15_10_IRQHandler(void)
{
    // 先清除中断挂起标志位
    EXTI->PR |= EXTI_PR_PR10;

    // 防抖延迟
    Delay_ms(10);

    // 判断如果依然保持高电平，就翻转LED1
    if ((GPIOF->IDR & GPIO_IDR_IDR10) != 0)
    {
        LED_Toggle(LED1);
    }
}
