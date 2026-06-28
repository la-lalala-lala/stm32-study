#include "spi.h"

// NM25Q128 芯片
// CS PB12
// SCK PB13
// MISO PB14
// MOSI PB15

void spi_init(void){
    /* 1. 打开GPIO引脚时钟 PB*/
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    /* 2. 配置引脚模式 */
    // 0011  PB13 SCK 输出  PB15 MOSI 输出   PB12 CS 输出
    GPIOB->CRH &= ~(GPIO_CRH_CNF13);
    GPIOB->CRH |= (GPIO_CRH_MODE13);

    GPIOB->CRH &= ~(GPIO_CRH_CNF15);
    GPIOB->CRH |= (GPIO_CRH_MODE15);

    GPIOB->CRH &= ~(GPIO_CRH_CNF12);
    GPIOB->CRH |= (GPIO_CRH_MODE12);
    // 0100 PB14 MISO 浮空输入
    GPIOB->CRH &= ~(GPIO_CRH_CNF14_1 | GPIO_CRH_MODE14);
    GPIOB->CRH |= (GPIO_CRH_CNF14_0);

    /* 3. 初始化  默认不片选的   默认时钟空闲 */
    CS_HIGH;
    SCK_LOW;
}

void spi_start(void){
    CS_LOW;
}

void spi_stop(void){
    CS_HIGH;
}

uint8_t spi_swap_byte(uint8_t ch){
    // 1. 根据 ch 当前最高位，设置 PB15(MOSI)
    // 2. 拉高 PB13(SCK)，让 Flash 采样 MOSI
    // 3. 读取 PB14(MISO)，得到 Flash 返回的一位
    // 4. 拉低 PB13(SCK)
    // 5. ch 左移，准备发送下一位
    // 通过 PB15 这个 MOSI 引脚向 Flash 写数据
    uint8_t rbyte = 0;
    for (uint8_t i = 0; i < 8; i++){
        // 上升沿之前  => 准备需要发送的数据
        if (ch & 0x80){
            // 最到位为1
            MOSI_HIGH;// 当前最高位是 1，就让 PB15 输出高
        }else{
            MOSI_LOW;// 当前最高位是 0，就让 PB15 输出低
        }
        spi_delay;
        // 写数据的移位在写之后
        ch <<= 1;
        // 上升沿触发发送  Flash 会在 SCK 的上升沿采样 MOSI 上的数据。
        SCK_HIGH;
        spi_delay;
        rbyte <<= 1;
        // 高电平时机  =>  读取别人给你发送的数据
        if (MISO_READ){
            // 读取到高电平  => 存放到rbyte
            // 从 PB14 这个 MISO 引脚读取 Flash 返回的数据
            rbyte |= 1;
        }
        SCK_LOW;
        spi_delay;
    }
    return rbyte;
}
