#ifndef __DRIVER_SPI_H
#define __DRIVER_SPI_H

#include "stm32f1xx.h"
#include "delay/delay.h"

// NM25Q128 芯片
// CS PB12
// SCK PB13
// MISO PB14
// MOSI PB15

#define CS_HIGH (GPIOB->ODR |= GPIO_ODR_ODR12)
#define CS_LOW (GPIOB->ODR &= ~GPIO_ODR_ODR12)


void spi_init(void);

void spi_start(void);

void spi_stop(void);

uint8_t spi_swap_byte(uint8_t ch);

#endif
