#include "oled.h"

#include "main.h"
#define OLED_FONT_ASCII_ONLY
#include "oledfont.h"
#include "spi.h"

/* 单次阻塞式 SPI 发送的超时时间，单位为毫秒。 */
#define OLED_SPI_TIMEOUT 100U
// oled 打印缓冲区大小
#define OLED_PRINT_BUFFER_SIZE 128U
// 记录当前页号
static uint8_t oled_next_page = 0U;
// oled 打印缓冲区
static char oled_print_buffer[OLED_PRINT_BUFFER_SIZE];

/**
 * @brief 向 SSD1306 连续发送命令字节。
 * @note  DC 拉低表示命令，CS 在整组命令发送期间保持低电平。
 */
static void OLED_WriteCommands(const uint8_t *commands, uint16_t size){
  if ((commands == NULL) || (size == 0U)){
    return;
  }

  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET);
  (void)HAL_SPI_Transmit(&hspi1, (uint8_t *)commands, size, OLED_SPI_TIMEOUT);
  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief 向 SSD1306 发送单个命令字节。
 */
static void OLED_WriteCommand(uint8_t command){
  OLED_WriteCommands(&command, 1U);
}

/**
 * @brief 向 SSD1306 显存连续写入数据。
 * @note  DC 拉高表示显存数据，阻塞发送结束后再释放 CS。
 */
static void OLED_WriteData(const uint8_t *data, uint16_t size){
  if ((data == NULL) || (size == 0U)){
    return;
  }

  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET);
  (void)HAL_SPI_Transmit(&hspi1, (uint8_t *)data, size, OLED_SPI_TIMEOUT);
  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief 产生一次低有效的硬件复位脉冲。
 */
static void OLED_Reset(void){
  /* 复位前先取消片选，避免控制线切换被误识别为 SPI 数据。 */
  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(1U);

  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10U);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(10U);
}

void OLED_SetCursor(uint8_t x, uint8_t page){
  uint8_t commands[3];

  if ((x >= OLED_WIDTH) || (page >= OLED_PAGE_COUNT)){
    return;
  }

  /* 页地址、列地址低 4 位、列地址高 4 位。 */
  commands[0] = (uint8_t)(0xB0U | page);
  commands[1] = (uint8_t)(0x00U | (x & 0x0FU));
  commands[2] = (uint8_t)(0x10U | ((x >> 4U) & 0x0FU));
  OLED_WriteCommands(commands, sizeof(commands));
}

void OLED_Clear(void){
  /* SSD1306 每页包含 128 列，每列的一个字节控制 8 个垂直像素。 */
  static const uint8_t blank_page[OLED_WIDTH] = {0U};

  for (uint8_t page = 0U; page < OLED_PAGE_COUNT; ++page){
    OLED_SetCursor(0U, page);
    OLED_WriteData(blank_page, OLED_WIDTH);
  }
}

void OLED_ShowChar(uint8_t x, uint8_t page, char character){
  uint8_t columns[OLED_FONT_WIDTH] = {0U};
  uint8_t index;

  if ((x > (OLED_WIDTH - OLED_FONT_WIDTH)) || (page >= OLED_PAGE_COUNT)){
    return;
  }

  index = (uint8_t)character;
  if ((index < (uint8_t)' ') || (index > (uint8_t)'~')){
    index = (uint8_t)' ';
  }

  /*
   * F6x8 按行存储且高位在左，而 SSD1306 显存按列存储且低位在上。
   * 写入前必须将 8 行字模转置为 6 个列字节，否则字符会转置或变形。
   */
  for (uint8_t row = 0U; row < 8U; ++row){
    for (uint8_t column = 0U; column < OLED_FONT_WIDTH; ++column){
      if ((F6x8[index - (uint8_t)' '][row] & (0x80U >> column)) != 0U){
        columns[column] |= (uint8_t)(1U << row);
      }
    }
  }

  OLED_SetCursor(x, page);
  OLED_WriteData(columns, sizeof(columns));
}

void OLED_ShowString(uint8_t x, uint8_t page, const char *string){
  if (string == NULL){
    return;
  }

  while (*string != '\0'){
    /* 换行符从下一页的第 0 列继续显示。 */
    if (*string == '\n'){
      x = 0U;
      ++page;
      ++string;
      continue;
    }

    if (page >= OLED_PAGE_COUNT){
      break;
    }

    /* 当前页剩余宽度不足一个字符时自动换到下一页。 */
    if (x > (OLED_WIDTH - OLED_FONT_WIDTH)){
      x = 0U;
      ++page;
      if (page >= OLED_PAGE_COUNT){
        break;
      }
    }

    OLED_ShowChar(x, page, *string++);
    x = (uint8_t)(x + OLED_FONT_WIDTH);
  }
}

void OLED_Init(void)
{
  static const uint8_t init_commands[] ={
    0xAEU,             /* 关闭显示，防止初始化期间出现随机画面 */
    0xD5U, 0x80U,      /* 设置显示时钟分频与振荡器频率 */
    0xA8U, 0x3FU,      /* 设置 1/64 复用率，对应 64 行 */
    0xD3U, 0x00U,      /* 显示垂直偏移为 0 */
    0x40U,             /* 显示起始行为第 0 行 */
    0x8DU, 0x14U,      /* 开启内部电荷泵 */
    0x20U, 0x02U,      /* 使用页寻址模式 */
    0xA1U,             /* 段地址重映射，使水平方向符合模块安装方向 */
    0xC8U,             /* 反向扫描 COM，使垂直方向符合模块安装方向 */
    0xDAU, 0x12U,      /* 设置 128x64 面板的 COM 引脚配置 */
    0x81U, 0xCFU,      /* 设置对比度 */
    0xD9U, 0xF1U,      /* 设置预充电周期 */
    0xDBU, 0x40U,      /* 设置 VCOMH 取消选择电平 */
    0xA4U,             /* 显示内容来自显存，而不是强制全亮 */
    0xA6U              /* 使用正常显示模式，非反色 */
  };

  OLED_Reset();

  /* 先完成配置和清屏，最后再开启显示，避免上电时闪出随机像素。 */
  OLED_WriteCommands(init_commands, sizeof(init_commands));
  OLED_Clear();
  OLED_WriteCommand(0xAFU); /* 开启显示 */
}

void OLED_PrintReset(void){
  // 清屏，并从第一行重新开始
  OLED_Clear();
  oled_next_page = 0U;
}

/*
 * 显示字符串，并返回下一次应该使用的页号。
 * 支持长字符串和 '\n' 换行。
 */
static uint8_t OLED_PrintText(uint8_t page, const char *text){
  uint8_t x = 0U;
  if (text == NULL){
    return page;
  }

  /* 空字符串也作为一行处理 */
  if (*text == '\0'){
    return (page < OLED_PAGE_COUNT) ? page + 1U : page;
  }

  while (*text != '\0'){
    /* 忽略回车符 */
    if (*text == '\r'){
        text++;
        continue;
    }

    /* 换行 */
    if (*text == '\n'){
        x = 0U;
        page++;
        text++;
        continue;
    }

    if (page >= OLED_PAGE_COUNT){
      break;
    }

    /* 当前行空间不足时自动换到下一页 */
    if (x > (OLED_WIDTH - OLED_FONT_WIDTH)){
      x = 0U;
      page++;

      if (page >= OLED_PAGE_COUNT){
          break;
      }
    }
    OLED_ShowChar(x, page, *text++);
    x = (uint8_t)(x + OLED_FONT_WIDTH);
  }

  /* 当前页显示过内容，下一次从下一页开始 */
  if ((x != 0U) && (page < OLED_PAGE_COUNT)){
    page++;
  }

  return page;
}

void OLED_PrintLine(const char *format, ...){
  va_list args;
  int length;

  if (format == NULL){
      return;
  }

  if (oled_next_page >= OLED_PAGE_COUNT){
      OLED_PrintReset();
  }

  va_start(args, format);
  length = vsnprintf(oled_print_buffer,sizeof(oled_print_buffer),format,args);
  va_end(args);

  if (length < 0){
      return;
  }

  /*
  * 如果格式化结果超过缓冲区，vsnprintf 会自动截断。
  * 最多显示 OLED_PRINT_BUFFER_SIZE - 1 个字符。
  */
  oled_print_buffer[OLED_PRINT_BUFFER_SIZE - 1U] = '\0';
  oled_next_page = OLED_PrintText(oled_next_page,oled_print_buffer);
}