#include "usart/usart.h"
#include <stdio.h>
#include <stm32f1xx.h>
#include "delay/delay.h"
#include "lcd/inf_lcd.h"
#include "system/system.h"

int main(void)
{
    char lcd_id[16];
    uint32_t scroll_offset = 0;
    char scroll_text[] = "INF LCD SCROLL STRING TEST - STM32 FSMC TFTLCD";

    SystemClock_Config();
    init_usart();
    printf("boot\r\n");

    inf_lcd_init();
    snprintf(lcd_id, sizeof(lcd_id), "LCD ID:%04X", g_lcd.id);
    printf("%s\r\n", lcd_id);

    inf_lcd_show_string(10, 40, 300, 32, 32, "STM32", RED);
    inf_lcd_show_string(10, 85, 300, 24, 24, "TFTLCD TEST", BLUE);
    inf_lcd_show_string(10, 120, 300, 16, 16, lcd_id, RED);
    while (1) {
        scroll_offset = inf_lcd_show_scroll_string(10, 160, 300, 24,
                                                   24, scroll_text, WHITE, BLUE,
                                                   scroll_offset, 2);
        Delay_ms(30);
    }
}
