/********************************** (C) COPYRIGHT  *******************************
 * File Name          : debug.c
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : This file contains all the functions prototypes for UART
 *                      Printf , Delay functions.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "debug.h"

/*******************************************************************************
 * Function Name  : Delay_Init
 * Description    : Initializes Delay Funcation.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void Delay_Init(void)
{
    __NOP();
}

extern void vPortSetupTimerInterrupt(void);
/*******************************************************************************
 * Function Name  : Delay_Us
 * Description    : Microsecond Delay Time.
 * Input          : n - Microsecond number.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void Delay_Us(uint32_t n)
{
    vu32 ticks;
    vu32 told, tnow, reload, tcnt = 0;
    if ((0x0001 & (SysTick->CTRL)) == 0)
    {
        vPortSetupTimerInterrupt();
    }

    ticks = n * (SystemCoreClock / 1000000);
    reload = SysTick->LOAD;
    told = SysTick->VAL;

    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += reload - tnow + told;

            told = tnow;
            if (tcnt >= ticks)
                break;
        }
    }
}

/*******************************************************************************
 * Function Name  : Delay_Ms
 * Description    : Millisecond Delay Time.
 * Input          : n - Millisecond number.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void Delay_Ms(uint32_t n)
{
    vu32 ticks;
    vu32 told, tnow, reload, tcnt = 0;
    if ((0x0001 & (SysTick->CTRL)) == 0)
    {
        vPortSetupTimerInterrupt();
    }

    ticks = n * 1000 * (SystemCoreClock / 1000000);
    reload = SysTick->LOAD;
    told = SysTick->VAL;

    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += reload - tnow + told;

            told = tnow;
            if (tcnt >= ticks)
                break;
        }
    }
}

/*******************************************************************************
 * Function Name  : USART_Printf_Init
 * Description    : Initializes the USARTx peripheral.
 * Input          : baudrate - USART communication baud rate.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void USART_Printf_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

#if (DEBUG == DEBUG_UART1)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#elif (DEBUG == DEBUG_UART2)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#elif (DEBUG == DEBUG_UART3)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

#endif

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;

#if (DEBUG == DEBUG_UART1)
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);

#elif (DEBUG == DEBUG_UART2)
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);

#elif (DEBUG == DEBUG_UART3)
    USART_Init(USART3, &USART_InitStructure);
    USART_Cmd(USART3, ENABLE);

#endif
}

int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        ;

    return (ch);
}

int fgetc(FILE *f)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
        ;

    return (int)USART_ReceiveData(USART1);
}
