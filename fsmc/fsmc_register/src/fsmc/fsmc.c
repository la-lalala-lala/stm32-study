#include "fsmc.h"

void fsmc_init_gpio(){
    // 复用推挽输出 1011
    // 地址线  A0-A18

    // PD11-PD13 A16-A18
    GPIOD->CRH |= (GPIO_CRH_CNF11_1 | GPIO_CRH_CNF12_1 | GPIO_CRH_CNF13_1 | GPIO_CRH_MODE11 | GPIO_CRH_MODE12 | GPIO_CRH_MODE13);
    GPIOD->CRH &= ~(GPIO_CRH_CNF11_0 | GPIO_CRH_CNF12_0 | GPIO_CRH_CNF13_0);

    // PG0-PG5 A10-A15
    GPIOG->CRL |= (GPIO_CRL_CNF0_1 | GPIO_CRL_CNF1_1 | GPIO_CRL_CNF2_1 | GPIO_CRL_CNF3_1 | GPIO_CRL_CNF4_1 | GPIO_CRL_CNF5_1 | GPIO_CRL_MODE0 | GPIO_CRL_MODE1 | GPIO_CRL_MODE2 | GPIO_CRL_MODE3 | GPIO_CRL_MODE4 | GPIO_CRL_MODE5);
    GPIOG->CRL &= ~(GPIO_CRL_CNF0_0 | GPIO_CRL_CNF1_0 | GPIO_CRL_CNF2_0 | GPIO_CRL_CNF3_0 | GPIO_CRL_CNF4_0 | GPIO_CRL_CNF5_0);

    // PF12-PF15 A6-A9
    GPIOF->CRH |= (GPIO_CRH_CNF12_1 | GPIO_CRH_CNF13_1 | GPIO_CRH_CNF14_1 | GPIO_CRH_CNF15_1 | GPIO_CRH_MODE12 | GPIO_CRH_MODE13 | GPIO_CRH_MODE14 | GPIO_CRH_MODE15);
    GPIOF->CRH &= ~(GPIO_CRH_CNF12_0 | GPIO_CRH_CNF13_0 | GPIO_CRH_CNF14_0 | GPIO_CRH_CNF15_0);

    // PF0-PF5  A0-A5
    GPIOF->CRL |= (GPIO_CRL_CNF0_1 | GPIO_CRL_CNF1_1 | GPIO_CRL_CNF2_1 | GPIO_CRL_CNF3_1 | GPIO_CRL_CNF4_1 | GPIO_CRL_CNF5_1 | GPIO_CRL_MODE0 | GPIO_CRL_MODE1 | GPIO_CRL_MODE2 | GPIO_CRL_MODE3 | GPIO_CRL_MODE4 | GPIO_CRL_MODE5);
    GPIOF->CRL &= ~(GPIO_CRL_CNF0_0 | GPIO_CRL_CNF1_0 | GPIO_CRL_CNF2_0 | GPIO_CRL_CNF3_0 | GPIO_CRL_CNF4_0 | GPIO_CRL_CNF5_0);

    /*
     * 2. 数据端口配置为 FSMC 复用推挽输出。
     *
     * STM32F1 每个 GPIO 引脚由 MODE[1:0] 和 CNF[1:0] 配置：
     * MODE = 11：输出模式，最大速度 50MHz。
     * CNF  = 10：复用功能推挽输出，由 FSMC 外设接管引脚。
     *
     * 外部 SRAM 是 16 位数据总线，需要配置 FSMC_D0-D15。
     * 读周期时 FSMC 会自动控制数据线方向，所以数据线仍配置为复用推挽。
     */

    /* =============MODE: 配置输出速度为 50MHz=============== */
    // PD0 -> FSMC_D2, PD1 -> FSMC_D3
    GPIOD->CRL |= (GPIO_CRL_MODE0 |
                   GPIO_CRL_MODE1);
    // PD8 -> FSMC_D13, PD9 -> FSMC_D14, PD10 -> FSMC_D15,
    // PD14 -> FSMC_D0, PD15 -> FSMC_D1
    GPIOD->CRH |= (GPIO_CRH_MODE8 |
                   GPIO_CRH_MODE9 |
                   GPIO_CRH_MODE10 |
                   GPIO_CRH_MODE14 |
                   GPIO_CRH_MODE15);

    // PE7 -> FSMC_D4
    GPIOE->CRL |= (GPIO_CRL_MODE7);
    // PE8 -> FSMC_D5, PE9 -> FSMC_D6, PE10 -> FSMC_D7, PE11 -> FSMC_D8,
    // PE12 -> FSMC_D9, PE13 -> FSMC_D10, PE14 -> FSMC_D11, PE15 -> FSMC_D12
    GPIOE->CRH |= (GPIO_CRH_MODE8 |
                   GPIO_CRH_MODE9 |
                   GPIO_CRH_MODE10 |
                   GPIO_CRH_MODE11 |
                   GPIO_CRH_MODE12 |
                   GPIO_CRH_MODE13 |
                   GPIO_CRH_MODE14 |
                   GPIO_CRH_MODE15);

    /* =============CNF: 配置为复用推挽输出=============== */
    // PD0 -> FSMC_D2, PD1 -> FSMC_D3
    GPIOD->CRL |= (GPIO_CRL_CNF0_1 |
                   GPIO_CRL_CNF1_1);
    GPIOD->CRL &= ~(GPIO_CRL_CNF0_0 |
                    GPIO_CRL_CNF1_0);

    // PD8 -> FSMC_D13, PD9 -> FSMC_D14, PD10 -> FSMC_D15,
    // PD14 -> FSMC_D0, PD15 -> FSMC_D1
    GPIOD->CRH |= (GPIO_CRH_CNF8_1 |
                   GPIO_CRH_CNF9_1 |
                   GPIO_CRH_CNF10_1 |
                   GPIO_CRH_CNF14_1 |
                   GPIO_CRH_CNF15_1);
    GPIOD->CRH &= ~(GPIO_CRH_CNF8_0 |
                    GPIO_CRH_CNF9_0 |
                    GPIO_CRH_CNF10_0 |
                    GPIO_CRH_CNF14_0 |
                    GPIO_CRH_CNF15_0);

    // PE7 -> FSMC_D4
    GPIOE->CRL |= (GPIO_CRL_CNF7_1);
    GPIOE->CRL &= ~(GPIO_CRL_CNF7_0);

    // PE8 -> FSMC_D5, PE9 -> FSMC_D6, PE10 -> FSMC_D7, PE11 -> FSMC_D8,
    // PE12 -> FSMC_D9, PE13 -> FSMC_D10, PE14 -> FSMC_D11, PE15 -> FSMC_D12
    GPIOE->CRH |= (GPIO_CRH_CNF8_1 |
                   GPIO_CRH_CNF9_1 |
                   GPIO_CRH_CNF10_1 |
                   GPIO_CRH_CNF11_1 |
                   GPIO_CRH_CNF12_1 |
                   GPIO_CRH_CNF13_1 |
                   GPIO_CRH_CNF14_1 |
                   GPIO_CRH_CNF15_1);
    GPIOE->CRH &= ~(GPIO_CRH_CNF8_0 |
                    GPIO_CRH_CNF9_0 |
                    GPIO_CRH_CNF10_0 |
                    GPIO_CRH_CNF11_0 |
                    GPIO_CRH_CNF12_0 |
                    GPIO_CRH_CNF13_0 |
                    GPIO_CRH_CNF14_0 |
                    GPIO_CRH_CNF15_0);

    /*
     * 3. 控制端口配置为 FSMC 复用推挽输出。
     * PD4  -> FSMC_NOE  读使能，接 SRAM 的 OE
     * PD5  -> FSMC_NWE  写使能，接 SRAM 的 WE
     * PE0  -> FSMC_NBL0 低字节使能
     * PE1  -> FSMC_NBL1 高字节使能
     * PG10 -> FSMC_NE3  片选信号，板载 SRAM 使用
     */
    // PD4 -> FSMC_NOE, PD5 -> FSMC_NWE
    GPIOD->CRL |= (GPIO_CRL_MODE4 |
                   GPIO_CRL_MODE5);
    GPIOD->CRL |= (GPIO_CRL_CNF4_1 |
                   GPIO_CRL_CNF5_1);
    GPIOD->CRL &= ~(GPIO_CRL_CNF4_0 |
                    GPIO_CRL_CNF5_0);

    // PE0 -> FSMC_NBL0, PE1 -> FSMC_NBL1
    GPIOE->CRL |= (GPIO_CRL_MODE0 |
                   GPIO_CRL_MODE1);
    GPIOE->CRL |= (GPIO_CRL_CNF0_1 |
                   GPIO_CRL_CNF1_1);
    GPIOE->CRL &= ~(GPIO_CRL_CNF0_0 |
                    GPIO_CRL_CNF1_0);

    // PG10 -> FSMC_NE3
    GPIOG->CRH |= (GPIO_CRH_MODE10);
    GPIOG->CRH |= (GPIO_CRH_CNF10_1);
    GPIOG->CRH &= ~(GPIO_CRH_CNF10_0);
}

void fsmc_init(void){
    /* 1. 打开时钟 DEFG */
    RCC->AHBENR |= RCC_AHBENR_FSMCEN;
    // AFIO主要功能 => 引脚重定向
    // 开启外部中断EXTI设置
    // RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPFEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPGEN;

    /* 2. 配置引脚模式 */
    fsmc_init_gpio();

        /* 3. 设置FSMC_BCR3 */
    // 将FSMC需要使用的BCR和BTR寄存器存放到一个数组中
    // NE1:BCR1[0] BTR1[1] 
    // NE2:BCR2[2] BTR2[3] 
    // NE3:BCR3[4] BTR3[5] 
    // NE4:BCR4[6] BTR4[7] 
    // 3.1 存储块使能
    FSMC_Bank1->BTCR[4] |= FSMC_BCRx_MBKEN;

    // 3.2 不使用数据线和地址线复用
    FSMC_Bank1->BTCR[4] &= ~FSMC_BCRx_MUXEN;

    // 3.3 存储器类型 SRAM: MTYP = 00
    FSMC_Bank1->BTCR[4] &= ~FSMC_BCRx_MTYP;

    // 3.4 数据宽度 16 位: MWID = 01
    FSMC_Bank1->BTCR[4] &= ~FSMC_BCRx_MWID;
    FSMC_Bank1->BTCR[4] |= FSMC_BCRx_MWID_0;

    // 3.5 打开写使能
    FSMC_Bank1->BTCR[4] |= FSMC_BCRx_WREN;

    /* 4. 配置BTR3  */
    // 4.1 地址设置时间  0+1个周期
    FSMC_Bank1->BTCR[5] &= ~FSMC_BTRx_ADDSET;

    // 4.2 地址保持时间  0+1个周期
    FSMC_Bank1->BTCR[5] &= ~FSMC_BTRx_ADDHLD;

    // 4.3 数据保持时间  合计高于55ns   71+1 = 1us
    FSMC_Bank1->BTCR[5] &= ~FSMC_BTRx_DATAST;
    FSMC_Bank1->BTCR[5] |= (FSMC_BTRx_DATAST_6 |
                             FSMC_BTRx_DATAST_2 |
                             FSMC_BTRx_DATAST_1 |
                             FSMC_BTRx_DATAST_0);
}
