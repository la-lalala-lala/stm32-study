#include "spi1.h"

// CH395 SPI1 连接
// CS   PG9
// SCK  PA5
// MISO PA6
// MOSI PA7

void spi1_init(void)
{
    /* 开启 GPIOA、GPIOG 和 SPI1 时钟（SPI1 挂载在 APB2 总线上）。 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPGEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* PA5 SCK：复用推挽输出，速度 50 MHz（配置值 1011）。 */
    GPIOA->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5);
    GPIOA->CRL |= (GPIO_CRL_MODE5 | GPIO_CRL_CNF5_1);

    /* PA7 MOSI：复用推挽输出，速度 50 MHz（配置值 1011）。 */
    GPIOA->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
    GPIOA->CRL |= (GPIO_CRL_MODE7 | GPIO_CRL_CNF7_1);

    /* PA6 MISO：浮空输入（配置值 0100）。 */
    GPIOA->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_CNF6);
    GPIOA->CRL |= GPIO_CRL_CNF6_0;

    /* PG9 CS：通用推挽输出，速度 50 MHz（配置值 0011）。 */
    GPIOG->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
    GPIOG->CRH |= GPIO_CRH_MODE9;
    GPIOG->ODR |= GPIO_ODR_ODR9;

    /* 配置 SPI1：8 位数据、软件 NSS、先发送高位、模式 0、主机模式。 */
    SPI1->CR1 &= ~SPI_CR1_DFF;
    SPI1->CR1 |= (SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR);
    SPI1->CR2 &= ~SPI_CR2_SSOE;
    // 3.3 高位优先
    SPI1->CR1 &= ~SPI_CR1_LSBFIRST;

    // 设置波特率 BR=010，SPI 时钟为 PCLK2 的 1/8（PCLK2=72 MHz 时为 9 MHz）
    SPI1->CR1 &= ~SPI_CR1_BR;
    SPI1->CR1 |= SPI_CR1_BR_1;

    // 相位和极性都配置为0  => 选择SPI模式0
    SPI1->CR1 &= ~(SPI_CR1_CPHA | SPI_CR1_CPOL);
     // 使能
    SPI1->CR1 |= SPI_CR1_SPE;
}

void spi1_start(void)
{
    CS_LOW;
}

void spi1_stop(void)
{
    CS_HIGH;
}

uint8_t spi1_swap_byte(uint8_t ch){
    // 对应SPI的时序  => 写缓存是交换数据之前写
    //          读缓存 => 交换数据完成之后再读的
    /* 1. 先写缓存  */
    while ((SPI1->SR & SPI_SR_TXE) == 0)
    {
        /* code */
    }
    SPI1->DR = ch;
     /* 2. 读缓存 */
     while ((SPI1->SR & SPI_SR_RXNE)==0)
     {
        /* code */
     }
     return (uint8_t)SPI1->DR;
}
