#ifndef __INF_LCD_H
#define __INF_LCD_H

#include "stm32f1xx.h"
#include "lcd_fsmc.h"
#include "../delay/delay.h"

/*
 * 战舰 V4: LCD_CS 接 FSMC_NE4，LCD_RS 接 FSMC_A10，16 位总线访问。
 * NE4 基地址为 0x6C000000；A10 在 16 位 FSMC 上的地址偏移为 2^10 * 2 = 0x800。
 * 因此 LCD_CMD 访问 RS=0，LCD_DATA 访问 RS=1。
 */
#define LCD_BASE 0x6C000000UL
#define LCD_CMD  ((volatile uint16_t *)(LCD_BASE))
#define LCD_DATA ((volatile uint16_t *)(LCD_BASE + (1UL << 11)))

/* 常用 RGB565 颜色。 */
#define WHITE   0xFFFF
#define BLACK   0x0000
#define BLUE    0x001F
#define BRED    0xF81F
#define GRED    0xFFE0
#define GBLUE   0x07FF
#define RED     0xF800
#define MAGENTA 0xF81F
#define GREEN   0x07E0
#define CYAN    0x7FFF
#define YELLOW  0xFFE0
#define BROWN   0xBC40
#define BRRED   0xFC07
#define GRAY    0x8430
#define LGRAY   0xC618

/**
 * @brief  LCD 设备运行时状态
 *
 * @note   width/height 会随显示方向变化；dir 只区分竖屏/横屏，
 *         不是 8 种扫描方向的完整编码。
 */
typedef struct {
    uint16_t width;    /* 当前显示方向下的宽度。 */
    uint16_t height;   /* 当前显示方向下的高度。 */
    uint16_t id;       /* LCD 控制器 ID，本板实测为 0x5310。 */
    uint8_t dir;       /* 0: 竖屏，1: 横屏。 */
    uint16_t wramcmd;  /* 写 GRAM 命令，NT35310 为 0x2C。 */
    uint16_t setxcmd;  /* 设置 X 地址命令，NT35310 为 0x2A。 */
    uint16_t setycmd;  /* 设置 Y 地址命令，NT35310 为 0x2B。 */
} inf_lcd_dev_t;

extern inf_lcd_dev_t g_lcd;
extern uint16_t g_lcd_back_color;

/* 底层 8080 风格命令/数据访问。 */
void inf_lcd_write_cmd(uint16_t cmd);
void inf_lcd_write_data(uint16_t data);
uint16_t inf_lcd_read_data(void);

/* LCD 上电、背光、ID 读取和基础显示入口。 */
void inf_lcd_init(void);
void inf_lcd_bk_open(void);
void inf_lcd_bk_close(void);
uint32_t inf_lcd_read_id(void);

/* 当前只移植了官方例程里 NT35310 需要的基础显示接口。 */
void inf_lcd_display_dir(uint8_t dir);
void inf_lcd_set_cursor(uint16_t x, uint16_t y);
void inf_lcd_write_ram_prepare(void);
void inf_lcd_clear(uint16_t color);
void inf_lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
void inf_lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode, uint16_t color);
void inf_lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color);
uint32_t inf_lcd_show_scroll_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                                    uint8_t size, char *p, uint16_t color, uint16_t back_color,
                                    uint32_t offset, uint16_t step);

void inf_lcd_nt35310_reginit(void);

#endif
