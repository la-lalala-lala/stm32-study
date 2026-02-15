#include "dma.h"

// DMA初始化
void DMA_Init(void)
{
    // 1 使能DMA1时钟
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    // 2 DMA相关配置
    // 2.1 数据传输方向: 从存储器读，发往串口外设
    DMA1_Channel4->CCR |= DMA_CCR_DIR;

    // 2.2 数据宽度: 8位 00
    DMA1_Channel4->CCR &= ~DMA_CCR_PSIZE;
    DMA1_Channel4->CCR &= ~DMA_CCR_MSIZE;

    // 2.3 地址自增：开启自增，串口地址不能自增
    DMA1_Channel4->CCR &= ~DMA_CCR_PINC;
    DMA1_Channel4->CCR |= DMA_CCR_MINC;

    // 2.4 中断使能：传输完成中断使能
    DMA1_Channel4->CCR |= DMA_CCR_TCIE;

    // 2.5 使能串口的DMA传输功能
    USART1->CR3 |= USART_CR3_DMAT;

    // 3 NVIC配置
    // 3.1 使能DMA1通道1中断
    NVIC_SetPriorityGrouping(3);
    NVIC_SetPriority(DMA1_Channel4_IRQn, 2);
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);
}

void DMA_Transmit(uint32_t src, uint32_t dst, uint32_t len)
{
    // 1 先关闭DMA通道
    DMA1_Channel4->CCR &= ~DMA_CCR_EN;
    
    // 2 设置外设地址
    DMA1_Channel4->CPAR = dst;

    // 3 设置存储器地址
    DMA1_Channel4->CMAR = src;

    // 4 设置传输长度
    DMA1_Channel4->CNDTR = len;

    // 5 开启DMA通道，开始传输数据
    DMA1_Channel4->CCR |= DMA_CCR_EN;
}

// 中断服务函数
void DMA1_Channel4_IRQHandler(void)
{
    // 1 检查是否是DMA1通道1中断
    if (DMA1->ISR & DMA_ISR_TCIF4)
    {
        // 2 清除中断标志位
        DMA1->IFCR |= DMA_IFCR_CTCIF4;

        // 3 执行其他操作，如数据处理或其他任务
 
        // 4 可选：关闭DMA通道，停止传输数据
        DMA1_Channel4->CCR &= ~DMA_CCR_EN;
    }
}