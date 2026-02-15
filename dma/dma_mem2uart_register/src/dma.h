#ifndef __DMA_H
#define __DMA_H

#include <stm32f1xx.h>


// DMA初始化
void DMA_Init(void);

// 数据传输
// @param src 源地址
// @param dst 目标地址
// @param len 传输长度
void DMA_Transmit(uint32_t src, uint32_t dst, uint32_t len);

#endif
