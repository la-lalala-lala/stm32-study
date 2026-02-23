/*
 * @Author: wushengran
 * @Date: 2024-05-28 15:30:08
 * @Description: 
 * 
 * Copyright (c) 2024 by atguigu, All Rights Reserved. 
 */
#include "usart.h"

// 初始化
void USART_Init(void)
{
    // 1. 配置时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // 2. GPIO工作模式
    // PA9: 复用推挽输出，CNF-10，MODE-11
    // PA10: 浮空输入，CNF-01，MODE-00
    GPIOA->CRH |= GPIO_CRH_MODE9;
    GPIOA->CRH |= GPIO_CRH_CNF9_1;
    GPIOA->CRH &= ~GPIO_CRH_CNF9_0;

    GPIOA->CRH &= ~GPIO_CRH_MODE10;
    GPIOA->CRH &= ~GPIO_CRH_CNF10_1;
    GPIOA->CRH |= GPIO_CRH_CNF10_0;

    // 3. 串口配置
    // 3.1 波特率设置
    USART1->BRR = 0x271;

    // 3.2 收发使能及模块使能
    USART1->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);

    // 3.3 其它配置，设置数据帧格式
    USART1->CR1 &= ~USART_CR1_M;
    USART1->CR1 &= ~USART_CR1_PCE;
    USART1->CR2 &= ~USART_CR2_STOP;
}

// 发送一个字符
void USART_SendChar(uint8_t ch)
{
    // 判断SR里TXE是否为1
    while ((USART1->SR & USART_SR_TXE) == 0)
    {}

    // 向DR写入新的要发送的数据
    USART1->DR = ch;
}

// 接收一个字符
uint8_t USART_ReceiveChar(void)
{
    while ((USART1->SR & USART_SR_RXNE) == 0)
    {
        // 增加判断空闲帧的条件
        if (USART1->SR & USART_SR_IDLE)
        {
            return 0;
        }    
    }

    // 读取已经接收到的数据，等待接收下一个数据
    return USART1->DR;
}

// 发送字符串
void USART_SendString(uint8_t *str, uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        USART_SendChar(str[i]);
    } 
}

// 接收字符串
void USART_ReceiveString(uint8_t buffer[], uint8_t *size)
{
    // 定义一个变量，用来保存已经接收到的字符个数
    uint8_t i = 0;

    while ((USART1->SR & USART_SR_IDLE) == 0)
    {
        buffer[i] = USART_ReceiveChar();
        i++;
    }

    // 清除IDLE位
    // USART1->SR;
    USART1->DR;
    
    *size = --i;
}

// 重写fputc
int fputc(int ch, FILE * file)
{
    // 直接将字符发送到串口
    USART_SendChar(ch);
    return ch;
}
