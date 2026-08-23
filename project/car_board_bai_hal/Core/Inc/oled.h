#ifndef __OLED_H
#define __OLED_H

#include "stm32f1xx_hal.h"
#include <stdarg.h>
#include <stdio.h>

/* 当前驱动按常见的 0.96 寸 SSD1306、128x64 分辨率配置。 */
#define OLED_WIDTH       128U
#define OLED_HEIGHT       64U
#define OLED_PAGE_COUNT  (OLED_HEIGHT / 8U)
#define OLED_FONT_WIDTH    6U

/**
 * @brief 初始化并点亮 OLED。
 * @note  调用前必须先完成 GPIO 和 SPI1 初始化。
 */
void OLED_Init(void);

/**
 * @brief 清空整块 OLED 显存。
 */
void OLED_Clear(void);

/**
 * @brief 设置后续显存写入位置。
 * @param x    水平像素坐标，范围为 0~127。
 * @param page 页地址，范围为 0~7；每页对应 8 行像素。
 * @note  page 是页编号，不是像素坐标。
 */
void OLED_SetCursor(uint8_t x, uint8_t page);

/**
 * @brief 显示一个 6x8 ASCII 字符。
 * @param x         字符左上角的水平像素坐标。
 * @param page      字符所在页，范围为 0~7。
 * @param character 要显示的字符；不支持的字符按空格处理。
 */
void OLED_ShowChar(uint8_t x, uint8_t page, char character);

/**
 * @brief 显示以空字符结尾的 ASCII 字符串。
 * @param x      字符串起始水平像素坐标。
 * @param page   字符串起始页，范围为 0~7。
 * @param string 要显示的字符串，支持换行符并在行末自动换页。
 */
void OLED_ShowString(uint8_t x, uint8_t page, const char *string);


/**
 * @brief 清屏并从第一行重新开始
 */
void OLED_PrintReset(void);


/**
 * @brief 从上次位置的下一行显示字符串
 * @param text   要显示的字符串
 */
void OLED_PrintLine(const char *format, ...);
#endif /* __OLED_H */
