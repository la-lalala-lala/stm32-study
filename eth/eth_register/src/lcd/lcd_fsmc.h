#ifndef __LCD_FSMC_H
#define __LCD_FSMC_H
#include "stm32f1xx.h"

/**
 * 引脚定义
 * LCD_BL(背光控制)对应 PB0；
 * LCD_CS 对应 PG12 即 FSMC_NE4；
 * LCD_RS 对应 PG0 即 FSMC_A10；
 * LCD_WR 对应 PD5 即 FSMC_NWE；
 * LCD_RD 对应 PD4 即 FSMC_NOE；
 * LCD _D[15:0]则直接连接在 FSMC_D15~FSMC_D0；
 * D2 PD0
 * D3 PD1
 * D13 PD8
 * D14 PD9
 * D15 PD10
 * D0 PD14
 * D1 PD15

 * D4 PE7
 * D5 PE8 
 * D6 PE9
 * D7 PE10
 * D8 PE11
 * D9 PE12
 * D10 PE13
 * D11 PE14
 * D12 PE15
 * 
 */

/* Configure FSMC pins and timings for the LCD module. */
void lcd_fsmc_init(void);


#endif
