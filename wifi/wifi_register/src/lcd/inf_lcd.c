#include "inf_lcd.h"
#include "lcdfont.h"

#define LCD_SCAN_L2R_U2D 0

inf_lcd_dev_t g_lcd;
/* 字符绘制时使用的背景色；当 mode=0 时，空白点会被刷成这个颜色。 */
uint16_t g_lcd_back_color = WHITE;

/**
 * @brief  FSMC 读操作前的短忙等
 * @note   这里只做极短延时，目的是给总线一个稳定窗口，不依赖 SysTick。
 */
static void inf_lcd_opt_delay(volatile uint32_t i)
{
    while (i--) {
    }
}

/**
 * @brief  设置 GRAM 访问窗口
 * @note   调用后后续 RAMWR 数据只会写入该矩形窗口，使用完小窗口后要恢复全屏窗口。
 */
static void inf_lcd_set_window(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    inf_lcd_write_cmd(g_lcd.setxcmd);
    inf_lcd_write_data(sx >> 8);
    inf_lcd_write_data(sx & 0xff);
    inf_lcd_write_data(ex >> 8);
    inf_lcd_write_data(ex & 0xff);

    inf_lcd_write_cmd(g_lcd.setycmd);
    inf_lcd_write_data(sy >> 8);
    inf_lcd_write_data(sy & 0xff);
    inf_lcd_write_data(ey >> 8);
    inf_lcd_write_data(ey & 0xff);
}

/**
 * @brief  填充一个矩形区域
 * @note   目前作为滚动字符串擦除背景的内部工具，结束后恢复全屏 GRAM 窗口。
 */
static void inf_lcd_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    uint32_t ex;
    uint32_t ey;
    uint32_t totalpoint;

    if (g_lcd.width == 0 || g_lcd.height == 0 || width == 0 || height == 0) {
        return;
    }

    if (x >= g_lcd.width || y >= g_lcd.height) {
        return;
    }

    ex = (uint32_t)x + width - 1;
    ey = (uint32_t)y + height - 1;

    if (ex >= g_lcd.width) {
        ex = g_lcd.width - 1;
    }

    if (ey >= g_lcd.height) {
        ey = g_lcd.height - 1;
    }

    totalpoint = (ex - x + 1) * (ey - y + 1);
    inf_lcd_set_window(x, y, (uint16_t)ex, (uint16_t)ey);
    inf_lcd_write_ram_prepare();

    for (uint32_t i = 0; i < totalpoint; i++) {
        inf_lcd_write_data(color);
    }

    inf_lcd_set_window(0, 0, g_lcd.width - 1, g_lcd.height - 1);
}

/**
 * @brief  取 ASCII 字符点阵
 */
static uint8_t inf_lcd_get_ascii_font(char chr, uint8_t size, const uint8_t **pfont, uint8_t *csize)
{
    uint8_t index;

    if (chr < ' ' || chr > '~') {
        return 0;
    }

    *csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    index = (uint8_t)(chr - ' ');

    switch (size) {
        case 12: *pfont = asc2_1206[index]; break;
        case 16: *pfont = asc2_1608[index]; break;
        case 24: *pfont = asc2_2412[index]; break;
        case 32: *pfont = asc2_3216[index]; break;
        default: return 0;
    }

    return 1;
}

/**
 * @brief  在裁剪窗口内显示一个 ASCII 字符
 */
static void inf_lcd_show_char_clip(int32_t x, int32_t y, char chr, uint8_t size, uint16_t color,
                                   uint16_t clip_x, uint16_t clip_y, uint16_t clip_w, uint16_t clip_h)
{
    uint8_t temp;
    uint8_t t1;
    uint8_t t;
    int32_t x0 = x;
    int32_t y0 = y;
    uint8_t csize;
    const uint8_t *pfont;
    int32_t clip_ex;
    int32_t clip_ey;

    if (!inf_lcd_get_ascii_font(chr, size, &pfont, &csize) || clip_w == 0 || clip_h == 0) {
        return;
    }

    clip_ex = (int32_t)clip_x + clip_w - 1;
    clip_ey = (int32_t)clip_y + clip_h - 1;

    for (t = 0; t < csize; t++) {
        temp = pfont[t];

        for (t1 = 0; t1 < 8; t1++) {
            if ((temp & 0x80) &&
                x >= clip_x && x <= clip_ex &&
                y >= clip_y && y <= clip_ey &&
                x >= 0 && y >= 0 &&
                x < g_lcd.width && y < g_lcd.height) {
                inf_lcd_draw_point((uint16_t)x, (uint16_t)y, color);
            }

            temp <<= 1;
            y++;

            if ((y - y0) == size) {
                y = y0;
                x++;

                if ((x - x0) == (size / 2)) {
                    return;
                }

                break;
            }
        }
    }
}

/**
 * @brief  向 LCD 写命令字
 * @param  cmd: 8/16 位命令值
 * @note   访问 LCD_CMD 地址，FSMC_A10=0，因此 RS=0。
 */
void inf_lcd_write_cmd(uint16_t cmd)
{
    /* LCD_CMD 地址让 FSMC_A10=0，即 LCD_RS=0。 */
    cmd = cmd;
    *LCD_CMD = cmd;
}

/**
 * @brief  向 LCD 写数据
 * @param  data: 要写入的数据字
 * @note   访问 LCD_DATA 地址，FSMC_A10=1，因此 RS=1。
 */
void inf_lcd_write_data(uint16_t data)
{
    /* LCD_DATA 地址让 FSMC_A10=1，即 LCD_RS=1。 */
    data = data;
    *LCD_DATA = data;
}

/**
 * @brief  从 LCD 读一个 16 位数据字
 * @return  读回的数据
 * @note   读 ID 时需要配合 dummy read 使用，这里只负责一次总线读取。
 */
uint16_t inf_lcd_read_data(void)
{
    volatile uint16_t data;
    /* 官方读 ID 流程依赖 dummy read，这里只补一个很短的总线等待。 */
    inf_lcd_opt_delay(2);
    data = *LCD_DATA;
    return data;
}

/**
 * @brief  打开 LCD 背光
 * @note   PB0 拉高后，开发板背光电源打开。
 */
void inf_lcd_bk_open(void)
{
    GPIOB->ODR |= GPIO_ODR_ODR0;
}

/**
 * @brief  关闭 LCD 背光
 */
void inf_lcd_bk_close(void)
{
    GPIOB->ODR &= ~GPIO_ODR_ODR0;
}

/**
 * @brief  识别 LCD 控制器 ID
 * @return  LCD ID，当前开发板实测为 0x5310
 *
 * @note   读取顺序与官方实验一致：
 *         1) 0xD3 探测 ILI9341/ST7796/ILI9806 类控制器
 *         2) 0x04 探测 ST7789，原始 0x8552 映射为 0x7789
 *         3) 0xD4 探测 NT35310，本项目最终就是这个分支
 */
uint32_t inf_lcd_read_id(void)
{
    uint32_t id = 0;

    inf_lcd_write_cmd(0xD3);
    inf_lcd_read_data();
    inf_lcd_read_data();
    id |= (inf_lcd_read_data() & 0xff) << 8;
    id |= (inf_lcd_read_data() & 0xff);

    if (id == 0x9341 || id == 0x7796 || id == 0x9806) {
        return id;
    }

    id = 0;
    inf_lcd_write_cmd(0x04);
    inf_lcd_read_data();
    inf_lcd_read_data();
    id |= (inf_lcd_read_data() & 0xff) << 8;
    id |= (inf_lcd_read_data() & 0xff);

    if (id == 0x8552) {
        return 0x7789;
    }

    id = 0;
    inf_lcd_write_cmd(0xD4);
    inf_lcd_read_data();
    inf_lcd_read_data();
    id |= (inf_lcd_read_data() & 0xff) << 8;
    id |= (inf_lcd_read_data() & 0xff);

    return id;
}

/**
 * @brief  按控制器 MADCTL(0x36) 设置扫描方向
 * @param  dir: 官方定义的 8 种扫描方向之一
 *
 * @note   g_lcd.dir 表示当前屏幕是竖屏还是横屏；当横屏时，需要把
 *         逻辑方向重新映射到控制器的 bit[7:5]，并同步交换宽高。
 */
static void inf_lcd_scan_dir(uint8_t dir)
{
    uint16_t regval = 0;
    uint16_t temp;

    /* 横屏时需要把逻辑扫描方向换算到控制器 MADCTL(0x36) 的 bit[7:5]。 */
    if (g_lcd.dir == 1) {
        switch (dir) {
            case 0: dir = 6; break;
            case 1: dir = 7; break;
            case 2: dir = 4; break;
            case 3: dir = 5; break;
            case 4: dir = 1; break;
            case 5: dir = 0; break;
            case 6: dir = 3; break;
            case 7: dir = 2; break;
            default: break;
        }
    }

    switch (dir) {
        case 0: regval |= (0 << 7) | (0 << 6) | (0 << 5); break;
        case 1: regval |= (1 << 7) | (0 << 6) | (0 << 5); break;
        case 2: regval |= (0 << 7) | (1 << 6) | (0 << 5); break;
        case 3: regval |= (1 << 7) | (1 << 6) | (0 << 5); break;
        case 4: regval |= (0 << 7) | (0 << 6) | (1 << 5); break;
        case 5: regval |= (0 << 7) | (1 << 6) | (1 << 5); break;
        case 6: regval |= (1 << 7) | (0 << 6) | (1 << 5); break;
        case 7: regval |= (1 << 7) | (1 << 6) | (1 << 5); break;
        default: break;
    }

    inf_lcd_write_cmd(0x36);
    inf_lcd_write_data(regval);

    /* bit5(MV) 会交换 X/Y 方向，宽高也要跟着同步。 */
    if (regval & 0x20) {
        if (g_lcd.width < g_lcd.height) {
            temp = g_lcd.width;
            g_lcd.width = g_lcd.height;
            g_lcd.height = temp;
        }
    } else {
        if (g_lcd.width > g_lcd.height) {
            temp = g_lcd.width;
            g_lcd.width = g_lcd.height;
            g_lcd.height = temp;
        }
    }

    /* 设置整屏显示窗口，后续写 GRAM 会从这个窗口内递增。 */
    inf_lcd_write_cmd(g_lcd.setxcmd);
    inf_lcd_write_data(0);
    inf_lcd_write_data(0);
    inf_lcd_write_data((g_lcd.width - 1) >> 8);
    inf_lcd_write_data((g_lcd.width - 1) & 0xff);

    inf_lcd_write_cmd(g_lcd.setycmd);
    inf_lcd_write_data(0);
    inf_lcd_write_data(0);
    inf_lcd_write_data((g_lcd.height - 1) >> 8);
    inf_lcd_write_data((g_lcd.height - 1) & 0xff);
}

/**
 * @brief  设置 LCD 显示方向与当前 GRAM 窗口参数
 * @param  dir: 0 竖屏，1 横屏
 *
 * @note   本项目只保留 NT35310/ILI9341/ST7789 这类常见 8080 控制器的
 *         基础窗口定义：0x2A 设置 X，0x2B 设置 Y，0x2C 写 GRAM。
 */
void inf_lcd_display_dir(uint8_t dir)
{
    g_lcd.dir = dir;
    /* NT35310/ILI9341/ST7789 这一类控制器使用 0x2A/0x2B/0x2C。 */
    g_lcd.wramcmd = 0x2C;
    g_lcd.setxcmd = 0x2A;
    g_lcd.setycmd = 0x2B;

    if (dir == 0) {
        g_lcd.width = 320;
        g_lcd.height = 480;
    } else {
        g_lcd.width = 480;
        g_lcd.height = 320;
    }

    inf_lcd_scan_dir(LCD_SCAN_L2R_U2D);
}

/**
 * @brief  设置单个像素/单次写入的起始坐标
 * @param  x: X 坐标
 * @param  y: Y 坐标
 */
void inf_lcd_set_cursor(uint16_t x, uint16_t y)
{
    /* 对 NT35310，设置单点位置只需要写入 X/Y 起始地址。 */
    inf_lcd_write_cmd(g_lcd.setxcmd);
    inf_lcd_write_data(x >> 8);
    inf_lcd_write_data(x & 0xff);

    inf_lcd_write_cmd(g_lcd.setycmd);
    inf_lcd_write_data(y >> 8);
    inf_lcd_write_data(y & 0xff);
}

void inf_lcd_write_ram_prepare(void)
{
    inf_lcd_write_cmd(g_lcd.wramcmd);
}

/**
 * @brief  在指定坐标绘制一个像素点
 * @param  x: X 坐标
 * @param  y: Y 坐标
 * @param  color: 16 位 RGB565 颜色
 *
 * @note   画点流程固定为：设坐标 -> 写 GRAM 命令 -> 写一个像素。
 */
void inf_lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= g_lcd.width || y >= g_lcd.height) {
        return;
    }

    inf_lcd_set_cursor(x, y);
    inf_lcd_write_ram_prepare();
    inf_lcd_write_data(color);
}

/**
 * @brief  使用单色填充整个屏幕
 * @param  color: 16 位 RGB565 颜色
 *
 * @note   这里直接连续写 width*height 个像素，不再逐点设坐标。
 */
void inf_lcd_clear(uint16_t color)
{
    uint32_t totalpoint = (uint32_t)g_lcd.width * g_lcd.height;

    /* 全屏窗口已在扫描方向配置中设置，这里连续写 width*height 个像素。 */
    inf_lcd_set_cursor(0, 0);
    inf_lcd_write_ram_prepare();

    for (uint32_t i = 0; i < totalpoint; i++) {
        inf_lcd_write_data(color);
    }
}

/**
 * @brief  显示一个 ASCII 字符
 * @param  x: 起始 X 坐标
 * @param  y: 起始 Y 坐标
 * @param  chr: ASCII 字符
 * @param  size: 字号，支持 12/16/24/32
 * @param  mode: 显示模式，0 为背景填充，1 为透明显示
 * @param  color: 字体颜色
 *
 * @note   官方字库按“竖向按字节取模”的方式组织，每个字节对应 8 个像素点。
 */
void inf_lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t temp;
    uint8_t t1;
    uint8_t t;
    uint16_t y0 = y;
    uint8_t csize;
    const uint8_t *pfont;

    /* 官方 ASCII 字库从空格到 '~'，不包含中文点阵。 */
    if (!inf_lcd_get_ascii_font(chr, size, &pfont, &csize)) {
        return;
    }

    for (t = 0; t < csize; t++) {
        temp = pfont[t];

        for (t1 = 0; t1 < 8; t1++) {
            if (temp & 0x80) {
                inf_lcd_draw_point(x, y, color);
            } else if (mode == 0) {
                inf_lcd_draw_point(x, y, g_lcd_back_color);
            }

            temp <<= 1;
            y++;

            if (y >= g_lcd.height) {
                return;
            }

            if ((y - y0) == size) {
                y = y0;
                x++;

                if (x >= g_lcd.width) {
                    return;
                }

                break;
            }
        }
    }
}

/**
 * @brief  在指定矩形区域内显示一帧横向滚动字符串
 * @param  x: 滚动区域起始 X
 * @param  y: 滚动区域起始 Y
 * @param  width: 滚动区域宽度
 * @param  height: 滚动区域高度
 * @param  size: 字号，支持 12/16/24/32
 * @param  p: ASCII 字符串指针
 * @param  color: 字体颜色
 * @param  back_color: 滚动区域背景色
 * @param  offset: 当前滚动偏移，首次调用传 0
 * @param  step: 每次调用后前进的像素数，传 0 时按 1 像素处理
 * @return 下一帧应传入的 offset
 *
 * @note   这是软件滚动，不依赖 LCD 控制器硬件滚动命令。
 *         外层周期性调用本函数，并配合 Delay_ms() 控制滚动速度。
 */
uint32_t inf_lcd_show_scroll_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                                    uint8_t size, char *p, uint16_t color, uint16_t back_color,
                                    uint32_t offset, uint16_t step)
{
    uint16_t char_width = size / 2;
    uint32_t len = 0;
    uint32_t text_width;
    uint32_t cycle_width;
    int32_t draw_x;
    char *str;

    if (p == 0 || char_width == 0 || width == 0 || height == 0) {
        return 0;
    }

    while (p[len] >= ' ' && p[len] <= '~') {
        len++;
    }

    if (len == 0) {
        inf_lcd_fill_rect(x, y, width, height, back_color);
        return 0;
    }

    text_width = len * char_width;
    cycle_width = text_width + width;
    offset %= cycle_width;
    draw_x = (int32_t)x + width - (int32_t)offset;

    inf_lcd_fill_rect(x, y, width, height, back_color);

    str = p;
    while (*str >= ' ' && *str <= '~') {
        if (draw_x >= (int32_t)x + width) {
            break;
        }

        if (draw_x + char_width > x) {
            inf_lcd_show_char_clip(draw_x, y, *str, size, color, x, y, width, height);
        }

        draw_x += char_width;
        str++;
    }

    if (step == 0) {
        step = 1;
    }

    offset += step;
    if (offset >= cycle_width) {
        offset %= cycle_width;
    }

    return offset;
}

/**
 * @brief  在矩形区域内显示字符串
 * @param  x: 起始 X
 * @param  y: 起始 Y
 * @param  width: 可显示宽度
 * @param  height: 可显示高度
 * @param  size: 字号
 * @param  p: 字符串指针
 * @param  color: 字体颜色
 *
 * @note   width/height 传入的是“区域尺寸”，不是结束坐标。
 *         超出区域后自动换行，适合调试信息和固定布局文本。
 */
void inf_lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color)
{
    uint16_t x0 = x;
    width += x;
    height += y;

    while (*p >= ' ' && *p <= '~') {
        if (x >= width) {
            x = x0;
            y += size;
        }

        if (y >= height) {
            break;
        }

        inf_lcd_show_char(x, y, *p, size, 0, color);
        x += size / 2;
        p++;
    }
}

/**
 * @brief  LCD 子系统初始化入口
 *
 * @note   初始化顺序固定为：
 *         1) FSMC 和 GPIO 初始化
 *         2) 读取 LCD ID
 *         3) 按 ID 选择控制器初始化序列
 *         4) 设置显示方向
 *         5) 打开背光并清屏
 */
void inf_lcd_init(void)
{
    lcd_fsmc_init();

    g_lcd.id = (uint16_t)inf_lcd_read_id();
    if (g_lcd.id == 0x5310) {
        /* 当前开发板实测为 NT35310，初始化序列从官方 lcd_ex.c 移植。 */
        inf_lcd_nt35310_reginit();
    }

    inf_lcd_display_dir(0);
    inf_lcd_bk_open();
    inf_lcd_clear(WHITE);
}
