#include "spi.h"

// NM25Q128 芯片
// CS PB12
// SCK PB13
// MISO PB14
// MOSI PB15

void spi_init(void){
    // 本实验用到 SPI2,使用 PB13、PB14 和 PB15 作为 SPI_SCK、SPI_MISO 和 SPI_MOSI
    /* 1. 打开时钟  SPI2时钟，打开GPIO引脚时钟 PB*/
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    /* 2. 配置引脚模式 */
    // PB13 SCK 复用推挽输出1011
    // PB15 MOSI 复用推挽输出1011
    // PB12 CS 通用推挽 0011
    // PB14 MISO 浮空输入  0100
    GPIOB->CRH |= (GPIO_CRH_CNF13_1 | GPIO_CRH_MODE13);
    GPIOB->CRH &= ~(GPIO_CRH_CNF13_0);

    GPIOB->CRH |= (GPIO_CRH_CNF15_1 | GPIO_CRH_MODE15);
    GPIOB->CRH &= ~(GPIO_CRH_CNF15_0);
    
    GPIOB->CRH |= GPIO_CRH_MODE12;
    GPIOB->CRH &= ~GPIO_CRH_CNF12;

    GPIOB->CRH &= ~(GPIO_CRH_CNF14_1 | GPIO_CRH_MODE14);
    GPIOB->CRH |= GPIO_CRH_CNF14_0;

    /* 3. 配置SPI2 */
    // 3.1 配置8位数据帧
    SPI2->CR1 &= ~SPI_CR1_DFF;
    // 3.2 配置软件从设备选择 => 手动拉高拉低片选线
    SPI2->CR1 |= SPI_CR1_SSM;
    SPI2->CR1 |= SPI_CR1_SSI;
    SPI2->CR2 &= ~SPI_CR2_SSOE;
    // 3.3 高位优先
    SPI2->CR1 &= ~SPI_CR1_LSBFIRST;
    // 3.4 设置波特率 选择8分频 010  9MHz
    // SPI2->CR1 &= ~SPI_CR1_BR;
    // SPI2->CR1 |= SPI_CR1_BR_1;

    // 3.4 设置波特率：因 SPI2 在 APB1(36MHz)，若要 9MHz 速率，需选择 4分频(001)
    SPI2->CR1 &= ~SPI_CR1_BR;
    SPI2->CR1 |= SPI_CR1_BR_0; // 4分频 -> 9MHz

    // 3.5 设置为主设备
    SPI2->CR1 |= SPI_CR1_MSTR;
    // 3.6 相位和极性都配置为0  => 选择SPI模式0
    SPI2->CR1 &= ~SPI_CR1_CPHA;
    SPI2->CR1 &= ~SPI_CR1_CPOL;
    // 4. 使能
    SPI2->CR1 |= SPI_CR1_SPE;
}

void spi_start(void){
    CS_LOW;
}

void spi_stop(void){
    CS_HIGH;
}

uint8_t spi_swap_byte(uint8_t ch){
    // 对应SPI的时序  => 写缓存是交换数据之前写
    //          读缓存 => 交换数据完成之后再读的
    /* 1. 先写缓存  */
    while ((SPI2->SR & SPI_SR_TXE) == 0)
    {
        /* code */
    }
    SPI2->DR = ch;
     /* 2. 读缓存 */
     while ((SPI2->SR & SPI_SR_RXNE)==0)
     {
        /* code */
     }
     return (uint8_t)SPI2->DR;
}
