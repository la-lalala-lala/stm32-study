#ifndef __DRIVER_SPI_H
#define __DRIVER_SPI_H

#include "stm32f1xx.h"
#include "delay/delay.h"

// NM25Q128 芯片
// CS PB12
// SCK PB13
// MISO PB14
// MOSI PB15

// 宏定义：ODR 和 BSRR/BRR 均可用于当前软件 SPI
// #define SCK_HIGH (GPIOB->BSRR = GPIO_BSRR_BS13)
// #define SCK_LOW (GPIOB->BRR = GPIO_BRR_BR13)

// #define MOSI_HIGH (GPIOB->BSRR = GPIO_BSRR_BS15)
// #define MOSI_LOW (GPIOB->BRR = GPIO_BRR_BR15)

// #define CS_HIGH (GPIOB->BSRR = GPIO_BSRR_BS12)
// #define CS_LOW (GPIOB->BRR = GPIO_BRR_BR12)
#define SCK_HIGH (GPIOB->ODR |= GPIO_ODR_ODR13)
#define SCK_LOW (GPIOB->ODR &= ~GPIO_ODR_ODR13)

#define MOSI_HIGH (GPIOB->ODR |= GPIO_ODR_ODR15)
#define MOSI_LOW (GPIOB->ODR &= ~GPIO_ODR_ODR15)

#define CS_HIGH (GPIOB->ODR |= GPIO_ODR_ODR12)
#define CS_LOW (GPIOB->ODR &= ~GPIO_ODR_ODR12)


// 读MISO线
#define MISO_READ (GPIOB->IDR & GPIO_IDR_IDR14)

#define spi_delay Delay_us(1)

void spi_init(void);

void spi_start(void);

void spi_stop(void);

uint8_t spi_swap_byte(uint8_t ch);

#endif
