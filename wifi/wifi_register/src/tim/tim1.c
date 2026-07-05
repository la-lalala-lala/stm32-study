#include "tim1.h"


void TIM1_Init(void)
{
    // 1.  开启时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    // 2.GPIO工作模式，PA8：复用推挽输出 CNF-10 MODE-11
    GPIOA->CRH |= GPIO_CRH_MODE8;
    GPIOA->CRH |= GPIO_CRH_CNF8_1;
    GPIOA->CRH &= ~GPIO_CRH_CNF8_0;

    // 3. 时基部分
    // 3.1 预分频器的配置 7199 得到10000 Hz
    TIM1->PSC = 7199;

    // 3.2 重装载值，4999，每0.5s产生一次溢出
    TIM1->ARR = 4999;
    // 3.3 计数方向 0=向上 1=向下*/
    TIM1->CR1 &= ~TIM_CR1_DIR;

    // 3.4 重复计数
    TIM1->RCR = 4;

    // 4. 输出通道部分
    // 4.1 配置通道1为输出模式
    TIM1->CCMR1 &= ~TIM_CCMR1_CC1S;
    // 4.2 配置通道1为PWM1模式，OC1M - 110
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_2;
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_1;
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M_0;
    // 4.3 配置CCR，占比50%
    TIM1->CCR1 = 2500;
    // 4.4 配置极性
    //TIM1->CCER &= ~TIM_CCER_CC1P;
    TIM1->CCER |= TIM_CCER_CC1P;// 低电平有效，闪烁后熄灭
    // 4.5 产生一个更新事件，刷新寄存器
    TIM1->CR1 |= TIM_CR1_URS;
    TIM1->EGR |= TIM_EGR_UG;
    //TIM1->SR &= ~TIM_SR_UIF;

    // 4.6 通道1使能
    TIM1->CCER |= TIM_CCER_CC1E;
    // 4.7 主输出使能
    TIM1->BDTR |= TIM_BDTR_MOE;

    // 5. 中断功能
    // 5.1 更新中断使能
    TIM1->DIER |= TIM_DIER_UIE;
    // 5.2 NVIC配置
    NVIC_SetPriorityGrouping(3);
    NVIC_SetPriority(TIM1_UP_IRQn, 3);
    NVIC_EnableIRQ(TIM1_UP_IRQn);
}

void TIM1_Start(void){
    // 使能计数器
    TIM1->CR1 |= TIM_CR1_CEN;
}

// 中断服务程序
void TIM1_UP_IRQHandler(void){
    // 清除中断标志位
    TIM1->SR &= ~TIM_SR_UIF;
    // 停掉定时器
    TIM1->CR1 &= ~TIM_CR1_CEN;
}