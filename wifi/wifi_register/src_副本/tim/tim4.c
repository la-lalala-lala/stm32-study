#include "tim4.h"


void TIM4_Init(void)
{
    // 1.  开启时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    //RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    // 2.GPIO工作模式，PB6：浮空输入 CNF-01 MODE-00
    GPIOB->CRL &= ~GPIO_CRL_MODE6;
    GPIOB->CRL &= ~GPIO_CRL_CNF6_1;
    GPIOB->CRL |= GPIO_CRL_CNF6_0;
    
    /* 1.3 TIM4重映射配置 */
    /* AFIO_MAPR寄存器TIM4_REMAP[12] = 0，表示不复用，CH1-CH4默认在PB6-PB9 */
    /* 如果TIM4_REMAP=1，则CH1-CH4会重映射到PD12-PD15 */
    //AFIO->MAPR &= ~AFIO_MAPR_TIM4_REMAP_Msk;

    /* 2. 设置GPIO的复用推挽输出 PB5 CNF=10 MODE=11*/
    GPIOB->CRL |= (GPIO_CRL_CNF5_1 | GPIO_CRL_MODE5);
    GPIOB->CRL &= ~GPIO_CRL_CNF5_0;

    // 3. 定时器配置
    // 时基部分
    // 3.1 预分频器的配置 71 得到1MHz
    TIM4->PSC = 71;
    // 3.2 重装载值，65535 尽量在信号一个周期内不要产生溢出
    TIM4->ARR = 65535;
    // 3.3 计数方向 0=向上 1=向下*/
    TIM4->CR1 &= ~TIM_CR1_DIR;

    // 4. 输入通道部分
    // 4.1 TI1的输入选择
    TIM4->CR2 &= ~TIM_CR2_TI1S;
    // 4.2 输入滤波器
    TIM4->CCMR1 &= ~TIM_CCMR1_IC1F;
    // 4.3 配置极性：上升沿触发
    TIM4->CCER &= ~TIM_CCER_CC1P;
    // 4.4 选择通道1的输入映射为：TI1：CC1S-01
    TIM4->CCMR1 &= ~TIM_CCMR1_CC1S_1;
    TIM4->CCMR1 |= TIM_CCMR1_CC1S_0;
    // 4.5 预分频器
    TIM4->CCMR1 &= ~TIM_CCMR1_IC1PSC;
    // 4.6 通道1输入捕获使能
    TIM4->CCER |= TIM_CCER_CC1E;
    // 4.7 开启捕获中断使能
    TIM4->DIER |= TIM_DIER_CC1IE;

    // 5. NVIC配置
    NVIC_SetPriorityGrouping(3);
    NVIC_SetPriority(TIM4_IRQn, 3);
    NVIC_EnableIRQ(TIM4_IRQn);
    
}

void TIM4_Start(void){
    // 使能计数器
    TIM4->CR1 |= TIM_CR1_CEN;
}

void TIM4_Stop(void){
    // 使能计数器
    TIM4->CR1 &= ~TIM_CR1_CEN;
}



// 返回PWM的周期 ms
double TIM4_GetPWMCycle(void){
    return TIM4->CCR1/1000.0;
}

// 返回PWM的频率
double TIM4_GetPWMFreq(void){
    return 1000000/TIM4->CCR1;
}

void TIM4_IRQHandler(void){
    if(TIM4->SR & TIM_SR_CC1IF){
        // 清除中断标志位
        TIM4->SR &= ~TIM_SR_CC1IF;
        TIM4->CNT = 0;
    }
}
