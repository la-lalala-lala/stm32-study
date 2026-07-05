#include "lcd_fsmc.h"

#define LCD_FSMC_BCR4  (FSMC_Bank1->BTCR[6])
#define LCD_FSMC_BTR4  (FSMC_Bank1->BTCR[7])
#define LCD_FSMC_BWTR4 (FSMC_Bank1E->BWTR[6])

/* Short busy wait after enabling FSMC. Do not depend on SysTick here. */
static void lcd_fsmc_opt_delay(volatile uint32_t i)
{
    while (i--) {
    }
}

/**
 * 战舰 V4 TFTLCD 插座使用 FSMC Bank1 NE4 连接 MCU 屏：
 * - LCD_BL -> PB0
 * - LCD_CS -> PG12 / FSMC_NE4
 * - LCD_RS -> PG0  / FSMC_A10
 * - LCD_WR -> PD5  / FSMC_NWE
 * - LCD_RD -> PD4  / FSMC_NOE
 * - LCD_D[15:0] -> FSMC_D[15:0]
 *
 * 数据线不是按 GPIO 端口连续排列，而是按 STM32F103 的 FSMC 复用功能分布：
 * - PD0/PD1/PD8/PD9/PD10/PD14/PD15 -> D2/D3/D13/D14/D15/D0/D1
 * - PE7/PE8/PE9/PE10/PE11/PE12/PE13/PE14/PE15 -> D4..D12
 */
static void lcd_fsmc_init_gpio(void)
{
    /* FSMC 数据线配置为复用推挽输出，速度 50 MHz。 */
    GPIOD->CRL |= (GPIO_CRL_MODE0 | GPIO_CRL_MODE1);
    GPIOD->CRH |= (GPIO_CRH_MODE8 | GPIO_CRH_MODE9 | GPIO_CRH_MODE10 | GPIO_CRH_MODE14 | GPIO_CRH_MODE15);
    GPIOE->CRL |= (GPIO_CRL_MODE7);
    GPIOE->CRH |= (GPIO_CRH_MODE8 | GPIO_CRH_MODE9 | GPIO_CRH_MODE10 | GPIO_CRH_MODE11 | GPIO_CRH_MODE12 | GPIO_CRH_MODE13 | GPIO_CRH_MODE14 | GPIO_CRH_MODE15);

    GPIOD->CRL |= (GPIO_CRL_CNF0_1 | GPIO_CRL_CNF1_1);
    GPIOD->CRL &= ~(GPIO_CRL_CNF0_0 | GPIO_CRL_CNF1_0);
    GPIOD->CRH |= (GPIO_CRH_CNF8_1 |GPIO_CRH_CNF9_1 | GPIO_CRH_CNF10_1 | GPIO_CRH_CNF14_1 | GPIO_CRH_CNF15_1);
    GPIOD->CRH &= ~(GPIO_CRH_CNF8_0 | GPIO_CRH_CNF9_0 | GPIO_CRH_CNF10_0 | GPIO_CRH_CNF14_0 | GPIO_CRH_CNF15_0);
    GPIOE->CRL |= (GPIO_CRL_CNF7_1);
    GPIOE->CRL &= ~(GPIO_CRL_CNF7_0);
    GPIOE->CRH |= (GPIO_CRH_CNF8_1 | GPIO_CRH_CNF9_1 | GPIO_CRH_CNF10_1 | GPIO_CRH_CNF11_1 | GPIO_CRH_CNF12_1 | GPIO_CRH_CNF13_1 | GPIO_CRH_CNF14_1 | GPIO_CRH_CNF15_1);
    GPIOE->CRH &= ~(GPIO_CRH_CNF8_0 | GPIO_CRH_CNF9_0 | GPIO_CRH_CNF10_0 | GPIO_CRH_CNF11_0 | GPIO_CRH_CNF12_0 | GPIO_CRH_CNF13_0 | GPIO_CRH_CNF14_0 | GPIO_CRH_CNF15_0);

    /* PG0 = FSMC_A10。LCD 把这根地址线当作 RS/DC：0 为命令，1 为数据。 */
    GPIOG->CRL |= (GPIO_CRL_CNF0_1 | GPIO_CRL_MODE0);
    GPIOG->CRL &= ~(GPIO_CRL_CNF0_0);
   
    /* PD4/PD5 分别是读写控制线 FSMC_NOE/FSMC_NWE。 */
    GPIOD->CRL |= (GPIO_CRL_MODE4 | GPIO_CRL_MODE5);
    GPIOD->CRL |= (GPIO_CRL_CNF4_1 | GPIO_CRL_CNF5_1);
    GPIOD->CRL &= ~(GPIO_CRL_CNF4_0 | GPIO_CRL_CNF5_0);

    /*
     * 官方例程没有启用 NBL0/NBL1，LCD 以 16 位整字访问。
     * 如果后续接入需要字节写的外部 SRAM，再考虑 PE0/PE1。
     */

    /* PG12 = FSMC_NE4，是本板 LCD 片选。PG10/NE3 没有用于 LCD。 */
    GPIOG->CRH |= (GPIO_CRH_MODE12);
    GPIOG->CRH |= (GPIO_CRH_CNF12_1);
    GPIOG->CRH &= ~(GPIO_CRH_CNF12_0);

    /* PB0 是 LCD 背光控制脚，不属于 FSMC，总线初始化后直接拉高点亮背光。 */
    GPIOB->CRL &= ~(GPIO_CRL_CNF0);
    GPIOB->CRL |= (GPIO_CRL_MODE0);
    GPIOB->BSRR = GPIO_BSRR_BS0;

    /*
     * 战舰 V4 的 LCD_RST 接到系统复位，不需要软件 GPIO 控制。
     * 注意 PG15 在该开发板上是 OV_OE，不能当作 LCD 复位脚使用。
     */
}

void lcd_fsmc_init(void)
{
    /* 打开 FSMC 和相关 GPIO 端口时钟。 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPFEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPGEN;

    RCC->AHBENR |= RCC_AHBENR_FSMCEN;

    lcd_fsmc_init_gpio();

    /* NE4 对应 BTCR[6]/BTCR[7]，扩展写时序在 BWTR[6]。 */
    LCD_FSMC_BCR4 = 0x00000000;
    LCD_FSMC_BTR4 = 0x00000000;
    LCD_FSMC_BWTR4 = 0x00000000;

    /* 异步 SRAM/8080 总线、16 位数据宽度、写使能、读写使用独立时序。 */
    LCD_FSMC_BCR4 = FSMC_BCRx_WREN | FSMC_BCRx_EXTMOD | FSMC_BCRx_MWID_0;

    /* 读时序沿用官方例程：Mode A，ADDSET=0，DATAST=15。 */
    LCD_FSMC_BTR4 = (15U << FSMC_BTRx_DATAST_Pos);

    /* 写时序沿用官方例程：Mode A，ADDSET=0，DATAST=1。 */
    LCD_FSMC_BWTR4 = (1U << FSMC_BWTRx_DATAST_Pos);

    LCD_FSMC_BCR4 |= FSMC_BCRx_MBKEN;
    lcd_fsmc_opt_delay(0x1FFFF);
}
