# STM32F103C8T6 与 0.96 寸 SPI OLED 使用记录

本文记录当前项目中 STM32F103C8T6 连接并驱动 0.96 寸、7 针、4 线 SPI OLED 的完整过程，便于以后重新接线、配置 CubeMX、移植驱动和排查故障。

当前驱动按以下硬件假设编写：

- OLED 控制器：SSD1306
- 分辨率：128x64
- 通信方式：4 线 SPI
- 字库：6x8 等宽 ASCII 字库
- MCU 主频：72 MHz
- SPI1 时钟：9 MHz

如果实物使用 SH1106、CH1116 或其他控制器，初始化命令和列地址可能需要调整。例如 SH1106 常见情况需要增加 2 列偏移。

## 1. 硬件接线

| OLED 引脚 | STM32F103C8T6 | 作用 | 有效电平或说明 |
|---|---|---|---|
| GND | GND | 电源地 | 必须共地 |
| VCC | 3.3V | OLED 电源 | 当前项目使用 3.3V |
| SCL/D0 | PA5 | SPI1_SCK | SPI 时钟 |
| SDA/D1 | PA7 | SPI1_MOSI | STM32 向 OLED 发送数据 |
| RST/RES | PB9 | OLED 复位 | 低电平有效 |
| DC | PA1 | 命令/数据选择 | 低电平为命令，高电平为数据 |
| CS | PA4 | OLED 片选 | 低电平有效 |

OLED 是只写设备，因此 PA6/SPI1_MISO 不需要连接到 OLED。当前 CubeMX 采用全双工主机模式，所以 PA6 仍被配置为输入，这不会影响显示。

## 2. SPI1 是否需要重映射

不需要重映射。当前接线正好使用 STM32F103C8T6 的 SPI1 默认引脚：

| SPI1 信号 | 默认引脚 | 当前用途 |
|---|---|---|
| NSS | PA4 | 作为普通 GPIO 手动控制 OLED_CS |
| SCK | PA5 | OLED_SCL/D0 |
| MISO | PA6 | OLED 不使用 |
| MOSI | PA7 | OLED_SDA/D1 |

SPI1 重映射后的引脚是 PA15/NSS、PB3/SCK、PB4/MISO、PB5/MOSI。当前工程不要启用 SPI1 重映射。

PA4 使用软件片选，因此必须配置为普通 GPIO 推挽输出，而不是 SPI1 硬件 NSS 复用输出。

## 3. STM32CubeMX 配置

### 3.1 系统时钟

当前项目时钟配置为：

- 外部高速晶振 HSE：8 MHz
- PLL 倍频：9
- SYSCLK/HCLK：72 MHz
- APB2：72 MHz
- SPI1 时钟源：APB2

### 3.2 SPI1 配置

在 CubeMX 中启用 `SPI1`，选择 `Full-Duplex Master`，参数如下：

| 参数 | 当前值 |
|---|---|
| Mode | Master |
| Direction | 2 Lines Full Duplex |
| Data Size | 8 Bits |
| Clock Polarity | Low |
| Clock Phase | 1 Edge |
| SPI Mode | Mode 0 |
| NSS | Software |
| First Bit | MSB First |
| Baud Rate Prescaler | 8 |
| 实际 SPI 时钟 | 72 MHz / 8 = 9 MHz |
| TI Mode | Disable |
| CRC Calculation | Disable |

9 MHz 低于 SSD1306 常见的约 10 MHz SPI 上限。如果使用较长杜邦线、接触不良或模块质量一般，出现乱码时可先将分频改为 16，使 SPI 时钟降到 4.5 MHz 验证。

### 3.3 GPIO 配置

| 引脚标签 | 引脚 | GPIO 模式 | 上下拉 | 速度 | 初始电平 |
|---|---|---|---|---|---|
| OLED_CS | PA4 | Output Push-Pull | No Pull | Low | High |
| OLED_DC | PA1 | Output Push-Pull | No Pull | Low | Low |
| OLED_RST | PB9 | Output Push-Pull | No Pull | Low | High |

初始电平的含义：

- `OLED_CS = High`：上电后先不选中 OLED。
- `OLED_DC = Low`：默认进入命令状态。
- `OLED_RST = High`：复位信号正常释放，驱动初始化时再主动产生低脉冲。

SPI 引脚由 CubeMX 自动配置：

- PA5/SPI1_SCK：复用推挽输出，高速。
- PA7/SPI1_MOSI：复用推挽输出，高速。
- PA6/SPI1_MISO：输入，无上下拉；OLED 不连接该引脚。

## 4. 项目文件职责

| 文件 | 职责 |
|---|---|
| [`Core/Inc/main.h`](Core/Inc/main.h) | 保存 OLED_DC、OLED_CS、OLED_RST 引脚宏 |
| [`Core/Src/gpio.c`](Core/Src/gpio.c) | 初始化三个 OLED 控制 GPIO 及其初始电平 |
| [`Core/Inc/spi.h`](Core/Inc/spi.h) | 声明 SPI1 句柄和初始化函数 |
| [`Core/Src/spi.c`](Core/Src/spi.c) | 配置 SPI1 Mode 0、软件 NSS 和 9 MHz 时钟 |
| [`Core/Inc/oled.h`](Core/Inc/oled.h) | 声明 OLED 尺寸和公开显示接口 |
| [`Core/Src/oled.c`](Core/Src/oled.c) | 实现复位、命令/数据发送、初始化、清屏和文字显示 |
| [`Core/Inc/oledfont.h`](Core/Inc/oledfont.h) | 保存 ASCII 和其他点阵字库 |
| [`Core/Src/main.c`](Core/Src/main.c) | 按顺序初始化外设并调用 OLED 显示接口 |
| [`CMakeLists.txt`](CMakeLists.txt) | 将 `Core/Src/oled.c` 加入 CMake 构建 |

`oled.c` 在包含字库前定义了 `OLED_FONT_ASCII_ONLY`，因此当前固件只编译 `F6x8` ASCII 字库。`oledfont.h` 后面的 8x16 和中文字库仍保留在源文件中，但不会占用当前固件的 Flash。

PlatformIO 的 `build_src_filter` 已包含整个 `Core/Src`，所以会自动编译 `oled.c`。HAL 配置中也已启用 `HAL_SPI_MODULE_ENABLED`。

## 5. 程序启动顺序

当前 `main()` 中与 OLED 相关的执行顺序是：

```c
HAL_Init();
SystemClock_Config();

MX_GPIO_Init();
MX_USART3_UART_Init();
MX_SPI1_Init();

OLED_Init();
OLED_PrintReset();
OLED_PrintLine("unzip: start");
OLED_PrintLine("open archive.zip");
OLED_PrintLine("this is a very long message");
OLED_PrintLine("verify checksum OK");
```

必须先初始化 GPIO 和 SPI1，再调用 `OLED_Init()`。否则控制引脚或 SPI 外设尚未准备好，OLED 无法正确接收命令。

## 6. OLED 初始化过程

`OLED_Init()` 的执行过程如下：

1. 令 CS 为高、DC 为低，避免复位期间误接收数据。
2. RST 先保持高电平 1 ms。
3. RST 拉低 10 ms，产生硬件复位。
4. RST 拉高并等待 10 ms。
5. 在 DC 为低、CS 为低时发送 SSD1306 初始化命令。
6. 以页为单位清空 8 页显存，每页写入 128 个 `0x00`。
7. 发送 `0xAF` 开启显示。

主要初始化命令如下：

| 命令 | 作用 |
|---|---|
| `0xAE` | 关闭显示，避免初始化时出现随机画面 |
| `0xD5, 0x80` | 设置显示时钟分频和振荡器频率 |
| `0xA8, 0x3F` | 设置 1/64 复用率，对应 64 行 |
| `0xD3, 0x00` | 显示偏移为 0 |
| `0x40` | 显示起始行为第 0 行 |
| `0x8D, 0x14` | 开启内部电荷泵 |
| `0x20, 0x02` | 使用页寻址模式 |
| `0xA1` | 段地址重映射 |
| `0xC8` | 反向扫描 COM 输出 |
| `0xDA, 0x12` | 设置 128x64 面板 COM 引脚配置 |
| `0x81, 0xCF` | 设置对比度 |
| `0xD9, 0xF1` | 设置预充电周期 |
| `0xDB, 0x40` | 设置 VCOMH 取消选择电平 |
| `0xA4` | 使用显存内容驱动显示 |
| `0xA6` | 正常显示，非反色 |
| `0xAF` | 开启显示 |

## 7. 命令与数据发送规律

发送命令时：

```text
DC = Low
CS = Low
SPI 发送命令字节
CS = High
```

发送显存数据时：

```text
DC = High
CS = Low
SPI 发送显存数据
CS = High
```

当前使用阻塞式 `HAL_SPI_Transmit()`，函数返回后才释放 CS，因此局部数组在发送期间是有效的。

## 8. 坐标、页和字符显示规律

### 8.1 OLED 坐标

SSD1306 屏幕尺寸为 128x64：

- 水平坐标 `x`：0-127。
- 垂直方向分为 8 页，`page`：0-7。
- 每页高度为 8 像素。
- 页的起始纵坐标：`y = page * 8`。

例如 `page = 3` 时，字符占用纵向像素 `y = 24-31`。

### 8.2 6x8 等宽字体

每个 ASCII 字符固定占用：

- 宽度：6 像素。
- 高度：8 像素。
- 空格同样占用 6 像素。

字库 `F6x8` 按行存储，且高位在左；SSD1306 显存按列存储，且低位在上。因此 `OLED_ShowChar()` 会先把 8 行字模转置为 6 个列字节，再发送到 OLED。

支持的字符范围是 ASCII `0x20-0x7E`。范围之外的字符当前按空格显示。

### 8.3 字符串宽度和水平居中

字符串宽度计算公式：

```text
字符串宽度 = 字符数量 * 6
居中起点 x = (128 - 字符串宽度) / 2
```

示例：

| 字符串 | 字符数 | 宽度 | 居中 x |
|---|---:|---:|---:|
| `Hello World` | 11 | 66 | 31 |
| `Hello World2` | 12 | 72 | 28 |

严格居中显示 `Hello World2` 应写成：

```c
OLED_ShowString(28U, 3U, "Hello World2");
```

从 `x = 28` 开始时，各字符占用范围如下：

| 字符 | 水平像素范围 |
|---|---|
| H | 28-33 |
| e | 34-39 |
| l | 40-45 |
| l | 46-51 |
| o | 52-57 |
| 空格 | 58-63 |
| W | 64-69 |
| o | 70-75 |
| r | 76-81 |
| l | 82-87 |
| d | 88-93 |
| 2 | 94-99 |

左右空白均为 28 像素：

```text
左侧空白 = 28
文字宽度 = 72
右侧空白 = 128 - 28 - 72 = 28
```

下面是固定坐标显示的历史示例；当前 `main.c` 已改用第 12 节介绍的 `OLED_PrintLine()`，因此这段代码不代表当前启动流程：

```c
OLED_ShowString(31U, 3U, "Hello World2");
```

此时文字范围为 `x = 31-102`，左侧空白为 31 像素，右侧空白为 25 像素，因此整体向右偏 3 像素。该调用仍然有效且不会越界，只是不属于严格数学居中。

这里的居中以字符占用框计算。不同字形内部可能带有空白列，所以肉眼观察可能存在约 1 像素的视觉差异。

### 8.4 自动换页

一个 6 像素宽字符最后允许从 `x = 122` 开始，占用 `122-127`。当下一个字符的起点大于 122 时，`OLED_ShowString()` 会：

1. 将 `x` 设为 0。
2. 将 `page` 加 1。
3. 从下一页继续显示。

字符串中的换行符 `\n` 也会将 `x` 设为 0，并进入下一页。当 `page >= 8` 时停止显示，避免写出屏幕范围。

## 9. 当前注意项

1. 当前驱动假设控制器是 SSD1306 128x64。控制器或分辨率不同会导致黑屏、偏移或显示不完整。
2. `F6x8` 在 `oledfont.h` 中直接定义且具有外部链接。目前只有 `oled.c` 包含它，所以可以正常链接；不要在其他 `.c` 中再次包含该头文件，否则可能产生重复定义。
3. `F6x8[][8]` 的初始化缺少每个字符对应的分组花括号。数据顺序正确，当前工程可以编译，但 GCC 会产生 `-Wmissing-braces` 警告；启用 `-Werror` 后会导致构建失败。
4. `oledfont.h` 中未使用的大字库只是被条件编译排除，并没有从源文件物理删除。
5. `OLED_WriteCommands()` 和 `OLED_WriteData()` 当前忽略 `HAL_SPI_Transmit()` 返回值。正常情况下不影响显示，但 SPI 超时或状态异常时，上层无法获知失败原因。
6. 当前 `oled.h` 面向纯 C 工程。未来若从 C++ 文件调用这些 C 函数，需要恢复 `extern "C"` 保护。
7. 当前 `main.c` 使用 `OLED_PrintLine()` 从第 0 页开始连续输出，不再使用原先固定坐标的 `Hello World2` 示例。
8. 每次重新用 CubeMX 生成代码后，应确认 SPI1 仍为软件 NSS、分频 8，并检查 `main.c` 中 OLED 调用仍位于 `USER CODE` 区域。

## 10. 黑屏或乱码排查顺序

1. 检查 OLED VCC 是否为 3.3V，并确认 STM32 与 OLED 共地。
2. 逐根核对 PA5/SCK、PA7/MOSI、PA4/CS、PA1/DC、PB9/RST。
3. 确认没有启用 SPI1 重映射。
4. 确认 PA4 是普通 GPIO 输出，SPI1 使用软件 NSS。
5. 确认 CS 空闲时为高、RST 正常时为高，并能观察到一次低电平复位脉冲。
6. 确认 SPI 为 Mode 0、8 位、MSB First。
7. 将 SPI 分频从 8 改为 16，以 4.5 MHz 排除信号完整性问题。
8. 用逻辑分析仪检查 PA5 是否有时钟、PA7 是否有数据、发送时 CS 是否拉低，以及 DC 命令/数据电平是否正确。
9. 确认 OLED 控制器是否真的是 SSD1306；若为 SH1106，检查列偏移和初始化命令。
10. 临时检查并处理 `HAL_SPI_Transmit()` 返回值，以区分硬件无显示和 SPI 软件发送失败。

## 11. 快速检查清单

烧录前确认：

- [ ] 接线与第 1 节一致。
- [ ] SPI1 未重映射。
- [ ] SPI1 为 Mode 0、软件 NSS、9 MHz。
- [ ] PA4、PA1、PB9 均为推挽输出。
- [ ] CS 和 RST 初始为高，DC 初始为低。
- [ ] `MX_GPIO_Init()` 在 `MX_SPI1_Init()` 之前执行。
- [ ] `OLED_Init()` 在 GPIO 和 SPI1 初始化之后执行。
- [ ] 显示文本的 `x`、`page` 和字符串长度不会超出屏幕。
- [ ] `Core/Src/oled.c` 已加入所使用的构建系统。

## 12. printf 风格的连续输出

当前版本实际实现的是 `OLED_PrintReset()` 和 `OLED_PrintLine()`，不是日志缓冲区滚屏接口。

### 12.1 接口作用

```c
void OLED_PrintReset(void);
void OLED_PrintLine(const char *format, ...);
```

- `OLED_PrintReset()`：清屏，并把下一次输出位置重置到第 0 页。
- `OLED_PrintLine()`：使用 `vsnprintf()` 按 `printf` 的方式格式化字符串，再从上一次输出结束的位置继续显示。
- 每次调用结束后，内部页号会更新，因此下一次调用会从下一行继续。
- 当前驱动使用 6x8 字体，每页（可视为一行）最多显示 21 个 ASCII 字符，屏幕共有 8 页。

### 12.2 基本用法

```c
OLED_Init();
OLED_PrintReset();

OLED_PrintLine("unzip: start");
OLED_PrintLine("open archive.zip");
OLED_PrintLine("verify checksum OK");
```

显示效果类似：

```text
unzip: start
open archive.zip
verify checksum OK
```

### 12.3 printf 格式化参数

```c
int file_index = 3;
unsigned long file_size_kb = 128UL;
int progress = 75;
unsigned long crc_value = 0x12AB34CDUL;
const char *file_name = "resource.dat";

OLED_PrintLine("file=%03d", file_index);
OLED_PrintLine("size=%lu KB", file_size_kb);
OLED_PrintLine("name=%s", file_name);
OLED_PrintLine("progress=%d%%", progress);
OLED_PrintLine("crc=0x%08lX", crc_value);
```

`OLED_PrintLine()` 的格式化缓冲区大小为 128 字节。格式化结果超过 127 个字符时，`vsnprintf()` 会截断，当前实现只显示截断后的内容。

### 12.4 长字符串和换页

长字符串会自动换页，并且内部页号会根据实际占用的页数更新。例如：

```c
OLED_PrintReset();
OLED_PrintLine("this is a very long message with more than twenty-one characters");
OLED_PrintLine("next message");
```

第一条文本超过一页时，前 21 个字符显示在当前页，后续字符从下一页开始；`"next message"` 会继续从长文本结束后的下一页显示，而不是覆盖长文本末尾。

字符串中也可以显式使用换行符：

```c
OLED_PrintLine("step 1\nstep 2\nstep 3");
```

处理规则：

- `\n`：进入下一页，并从第 0 列开始。
- `\r`：忽略，兼容串口常见的 `\r\n`。
- 当前页剩余空间不足一个 6 像素字符时，自动换到下一页。
- 空字符串也会占用一页。

### 12.5 页用完后的行为

当前实现不是“保留最后 8 行并向上平移”的真正终端滚屏。页号达到 8 后，下一次调用 `OLED_PrintLine()` 会自动执行 `OLED_PrintReset()`，清屏并从第 0 页重新开始：

```c
if (oled_next_page >= OLED_PAGE_COUNT)
{
    OLED_PrintReset();
}
```

因此它适合按行连续输出状态信息；如果需要始终保留最新 8 行，需要另行实现行缓冲区滚屏。

### 12.6 当前 main.c 的测试调用

当前 `main.c` 在初始化 OLED 后进行了以下测试：

```c
OLED_Init();
OLED_PrintReset();
OLED_PrintLine("unzip: start");
OLED_PrintLine("open archive.zip");
OLED_PrintLine("this is a very long message");
OLED_PrintLine("verify checksum OK");

for (int i = 0; i <= 60; i++)
{
    OLED_PrintLine("line=%d", i + 1);
    HAL_Delay(1000);
}
```

真实解压流程中，可以在每个文件处理完成或进度变化时调用：

```c
OLED_PrintLine("extract %s: %d%%", file_name, progress);
```

当前字库只支持 ASCII。中文字符串经过 `vsnprintf()` 后仍不能由现有 6x8 字库正确显示。

## 13. F6x8 字库初始化警告

编译时如果看到：

```text
warning: missing braces around initializer [-Wmissing-braces]
```

原因是 `F6x8` 声明为二维数组，但每个字符的 8 个字节没有单独用花括号包起来。数据通常仍能正确排列，但 GCC 会提示初始化层级不够明确。

推荐将字库写成：

```c
const unsigned char F6x8[95][8] =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* 空格 */
    {0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00}, /* ! */
    {0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00}, /* " */
    /* 每个字符使用一组 8 字节花括号 */
};
```

这样不需要修改 `oled.c` 中的访问方式：

```c
F6x8[index - (uint8_t)' '][row]
```

如果暂时不整理字库，也可以用 GCC diagnostic pragma 局部屏蔽该警告，但这只是隐藏诊断；长期建议补齐二维数组的嵌套花括号。
