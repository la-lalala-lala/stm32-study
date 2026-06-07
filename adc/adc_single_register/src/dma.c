#include "dma.h"

// DMA初始化
void DMA_Init(void)
{
    // 1 使能DMA1时钟
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    // 2 DMA相关配置
    // 2.1 数据传输方向: 存储器到存储器 ，从外设读
    DMA1_Channel1->CCR |= DMA_CCR_MEM2MEM;
    DMA1_Channel1->CCR &= ~DMA_CCR_DIR;

    // 2.2 数据宽度: 8位 00
    DMA1_Channel1->CCR &= ~DMA_CCR_PSIZE;
    DMA1_Channel1->CCR &= ~DMA_CCR_MSIZE;

    // 2.3 地址自增：开启自增
    DMA1_Channel1->CCR |= DMA_CCR_PINC;
    DMA1_Channel1->CCR |= DMA_CCR_MINC;

    // 2.4 中断使能：传输完成中断使能
    DMA1_Channel1->CCR |= DMA_CCR_TCIE;

    // 3 NVIC配置
    // 3.1 使能DMA1通道1中断
    NVIC_SetPriorityGrouping(3);
    NVIC_SetPriority(DMA1_Channel1_IRQn, 2);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void DMA_Transmit(uint32_t src, uint32_t dst, uint32_t len)
{
    // 1 先关闭DMA通道
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;
    
    // 2 设置外设地址
    DMA1_Channel1->CPAR = src;

    // 3 设置存储器地址
    DMA1_Channel1->CMAR = dst;

    // 4 设置传输长度
    DMA1_Channel1->CNDTR = len;

    // 5 开启DMA通道，开始传输数据
    DMA1_Channel1->CCR |= DMA_CCR_EN;
}

// 中断服务函数
void DMA1_Channel1_IRQHandler(void)
{
    // 1 检查是否是DMA1通道1中断
    if (DMA1->ISR & DMA_ISR_TCIF1)
    {
        // 2 清除中断标志位
        DMA1->IFCR |= DMA_IFCR_CTCIF1;

        // 3 执行其他操作，如数据处理或其他任务
        // 传输完成
        isFinished = 1;
        // 4 可选：关闭DMA通道，停止传输数据
        DMA1_Channel1->CCR &= ~DMA_CCR_EN;
    }
}