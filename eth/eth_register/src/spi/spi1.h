#ifndef __DRIVER_SPI1_H
#define __DRIVER_SPI1_H

#include "stm32f1xx.h"
#include "delay/delay.h"

// CH395 SPI1 connection
// CS   PG9
// SCK  PA5
// MISO PA6
// MOSI PA7

#define CS_HIGH (GPIOG->ODR |= GPIO_ODR_ODR9)
#define CS_LOW  (GPIOG->ODR &= ~GPIO_ODR_ODR9)

void spi1_init(void);

void spi1_start(void);

void spi1_stop(void);

uint8_t spi1_swap_byte(uint8_t ch);

#endif
