#include "tim3.h"


void TIM3_Init(void)
{
    /* 1.  开启时钟*/
    /* 1.1 定时器3的时钟 */
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    /* 1.2 GPIO的时钟 PB 和 AFIO */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    
    /* 1.3 使能TIM3部分重映射，将CH2映射到PB5 */
    /* AFIO_MAPR寄存器TIM3_REMAP[11:10] = 10，表示部分重映射 */
    AFIO->MAPR &= ~AFIO_MAPR_TIM3_REMAP_Msk;
    AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP_1;

    /* 2. 设置GPIO的复用推挽输出 PB5 CNF=10 MODE=11*/
    GPIOB->CRL |= (GPIO_CRL_CNF5_1 | GPIO_CRL_MODE5);
    GPIOB->CRL &= ~GPIO_CRL_CNF5_0;

    /* 3. 定时器配置 */
    /* 3.1 预分频器的配置 */
    TIM3->PSC = 7200 - 1;
    /* 3.2 自动重装载寄存器的配置 */
    TIM3->ARR = 100 - 1;
    /* 3.3 计数器的计数方向 0=向上 1=向下*/
    TIM3->CR1 &= ~TIM_CR1_DIR;
    /* 3.4 配置通道2的捕获比较寄存器 */
    TIM3->CCR2 = 97;
    /* 3.5 把通道2配置为输出  CCMR1_CC2S=00 */
    TIM3->CCMR1 &= ~TIM_CCMR1_CC2S;
    /* 3.6 配置通道的输出比较模式 CCMR1_OC2M=110*/
    TIM3->CCMR1 |= TIM_CCMR1_OC2M_2;
    TIM3->CCMR1 |= TIM_CCMR1_OC2M_1;
    TIM3->CCMR1 &= ~TIM_CCMR1_OC2M_0;
    /* 3.7 使能通道2  CCER_CC2E=1 */
    TIM3->CCER |= TIM_CCER_CC2E;

    /* 3.8 设置通道的极性 0=高电平有效  1=低电平有效 */
    TIM3->CCER |= TIM_CCER_CC2P;
}

void TIM3_Start(void){
    // 使能计数器
    TIM3->CR1 |= TIM_CR1_CEN;
}

void TIM3_Stop(void){
    // 使能计数器
    TIM3->CR1 &= ~TIM_CR1_CEN;
}

void TIM3_SetDutyCycle(uint8_t dutyCycle){
    // 设置占空比
    TIM3->CCR2 = dutyCycle;
}
