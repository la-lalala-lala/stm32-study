# STM32F103 战舰 V4 TFTLCD FSMC 寄存器驱动记录

## 项目目标

本项目在正点原子 STM32F103 战舰 V4 开发板上，使用寄存器方式驱动 TFTLCD(MCU 屏)：

- 打开 LCD 背光。
- 通过 FSMC 8080 并口读取 LCD 控制器 ID。
- 根据实测 ID `0x5310` 初始化 NT35310。
- 移植官方 LCD 实验中的基础显示案例，完成清屏和 ASCII 字符显示。

当前工程已经能正常点亮背光、读取 `LCD ID:5310`，并循环显示官方实验中的彩色背景和字符串。

## 参考资料

- `战舰V4 硬件参考手册_V1.0.pdf`
- `STM32F103 战舰开发指南LCD实验参考.pdf`
- `实验13 TFTLCD（MCU屏）实验`
- `ILI9341.pdf`

注意：`ILI9341.pdf` 可用于理解 ILI9341 的指令体系，但本开发板当前这块屏实测不是 ILI9341，而是 NT35310 兼容路径，ID 为 `0x5310`。

## 硬件连接

战舰 V4 的 TFTLCD 插座和 STM32F103ZET6 通过 FSMC Bank1 连接：

| LCD 信号 | STM32 引脚 | FSMC 功能 | 说明 |
| --- | --- | --- | --- |
| LCD_BL | PB0 | 普通 GPIO | 高电平打开背光 |
| LCD_CS | PG12 | FSMC_NE4 | LCD 片选 |
| LCD_RS | PG0 | FSMC_A10 | 命令/数据选择 |
| LCD_WR | PD5 | FSMC_NWE | 写使能 |
| LCD_RD | PD4 | FSMC_NOE | 读使能 |
| LCD_D0 | PD14 | FSMC_D0 | 16 位数据总线 |
| LCD_D1 | PD15 | FSMC_D1 | 16 位数据总线 |
| LCD_D2 | PD0 | FSMC_D2 | 16 位数据总线 |
| LCD_D3 | PD1 | FSMC_D3 | 16 位数据总线 |
| LCD_D4..D12 | PE7..PE15 | FSMC_D4..D12 | 16 位数据总线 |
| LCD_D13 | PD8 | FSMC_D13 | 16 位数据总线 |
| LCD_D14 | PD9 | FSMC_D14 | 16 位数据总线 |
| LCD_D15 | PD10 | FSMC_D15 | 16 位数据总线 |

LCD 复位脚接开发板系统复位，不需要软件控制。不要把 PG15 当作 LCD 复位脚使用，战舰 V4 上 PG15 是 `OV_OE`。

## FSMC 地址映射

本项目使用 FSMC Bank1 的第 4 片选 NE4：

- NE4 基地址：`0x6C000000`
- LCD_RS 接到 FSMC_A10
- 16 位 FSMC 访问时，地址线偏移需要乘以 2
- A10 对应数据地址偏移：`2^10 * 2 = 0x800`

因此当前项目直接使用两个 16 位访问地址：

```c
#define LCD_BASE 0x6C000000UL
#define LCD_CMD  ((volatile uint16_t *)(LCD_BASE))
#define LCD_DATA ((volatile uint16_t *)(LCD_BASE + (1UL << 11)))
```

访问 `LCD_CMD` 时 `A10=0`，即 `RS=0`，写入 LCD 命令；访问 `LCD_DATA` 时 `A10=1`，即 `RS=1`，读写 LCD 数据。

## FSMC 初始化要点

FSMC 初始化位于 `src/lcd/lcd_fsmc.c`：

- 打开 GPIOB/GPIOD/GPIOE/GPIOF/GPIOG 和 FSMC 时钟。
- 把 FSMC 数据线、地址线、读写控制线、片选线配置为复用推挽 50 MHz。
- PB0 配置为普通推挽输出并拉高，打开 LCD 背光。
- 使用 NE4，所以配置 `BTCR[6]`、`BTCR[7]` 和 `BWTR[6]`。
- 使用异步 SRAM/8080 总线模式、16 位数据宽度、写使能、读写独立时序。

时序沿用官方 LCD 实验：

- 读时序：Mode A，`ADDSET=0`，`DATAST=15`
- 写时序：Mode A，`ADDSET=0`，`DATAST=1`

CMSIS 头文件里没有 `FSMC_BCR4_MBKEN`、`FSMC_BCR4_MUXEN` 这种带编号的宏，所以项目使用：

- `FSMC_Bank1->BTCR[6]` 表示 BCR4
- `FSMC_Bank1->BTCR[7]` 表示 BTR4
- `FSMC_Bank1E->BWTR[6]` 表示 BWTR4
- 位定义使用通用宏，例如 `FSMC_BCRx_MBKEN`、`FSMC_BCRx_WREN`

`lcd_fsmc_opt_delay()` 是 FSMC 使能后的短忙等，用来等待总线稳定，不依赖 SysTick；毫秒级 LCD 指令等待仍然使用 `Delay_ms()`。

## 调试过程与现象

早期现象：

- 背光已打开。
- 屏幕有线条闪烁。
- 串口连接后闪烁消失。
- `inf_lcd_read_id()` 没有读到有效 ID，曾经返回 `0x0000`。

排查后修正的关键点：

- 不使用 PG15 做 LCD 复位，避免误碰摄像头相关管脚。
- FSMC 时序改为官方例程的读写独立时序。
- 按官方 LCD 实验的 ID 探测顺序读取，而不是只按 ILI9341 的 `0xD3` 读取。
- 确认 LCD 命令/数据地址为 `0x6C000000` / `0x6C000800`。

曾经记录到的原始读数：

```text
D3:0000 0000 0000 0000
D4:0001 0001 0053 0010
04:0000 0000 0080 0000
ID:0x5310
```

结论：`0xD3` 不返回 ILI9341 的 `0x9341`，`0xD4` 返回 `0x53 0x10`，因此当前屏幕走 NT35310 初始化路径。

## LCD ID 识别逻辑

`src/lcd/inf_lcd.c` 中的 `inf_lcd_read_id()` 按官方例程保留了多路径探测：

1. 发送 `0xD3`，尝试读取 ILI9341/ST7796/ILI9806 类 ID。
2. 如果不是上述 ID，发送 `0x04`，尝试读取 ST7789；官方把原始 `0x8552` 映射为 `0x7789`。
3. 如果仍未匹配，发送 `0xD4`，尝试读取 NT35310；本板实测为 `0x5310`。

当前项目只为 `0x5310` 执行完整初始化。如果后续更换其他控制器屏幕，需要补充对应初始化序列。

## 当前驱动结构

LCD 相关代码位于 `src/lcd`：

| 文件 | 职责 |
| --- | --- |
| `lcd_fsmc.c/.h` | 配置 GPIO、FSMC Bank1 NE4 和背光脚 |
| `inf_lcd.c/.h` | LCD 命令/数据读写、ID 读取、显示方向、清屏、画点、ASCII 字符显示 |
| `inf_lcd_nt35310.c` | NT35310 官方初始化序列移植 |
| `lcdfont.h` | 官方 ASCII 点阵字库 |

`inf_lcd_nt35310.c` 来自官方 `lcd_ex_nt35310_reginit()` 的机械移植，主要替换如下：

- `lcd_wr_regno()` -> `inf_lcd_write_cmd()`
- `lcd_wr_data()` -> `inf_lcd_write_data()`
- `delay_ms()` -> `Delay_ms()`

该文件里的寄存器值属于屏幕厂家初始化参数，除非有新的模组资料或实测依据，否则应尽量保持和官方序列一致。

## 已移植的显示能力

当前已经移植并验证：

- `inf_lcd_init()`
- `inf_lcd_read_id()`
- `inf_lcd_display_dir()`
- `inf_lcd_set_cursor()`
- `inf_lcd_write_ram_prepare()`
- `inf_lcd_clear()`
- `inf_lcd_draw_point()`
- `inf_lcd_show_char()`
- `inf_lcd_show_string()`

尚未移植官方完整 LCD 库中的读点、画线、画矩形、区域填充、圆形、数字显示、图片显示等功能。

## 主程序显示案例

`src/main.c` 保留了官方 TFTLCD 实验的最小案例：

- 初始化系统时钟和串口。
- 调用 `inf_lcd_init()`。
- 串口打印 `LCD ID:%04X`。
- 循环切换背景色。
- 显示：
  - `STM32`
  - `TFTLCD TEST`
  - `ATOM@ALIENTEK`
  - `LCD ID:5310`

## 构建命令

使用 PlatformIO 构建：

```powershell
D:\Program\platformio\penv\Scripts\platformio.exe run
```

如果在受限环境中执行，该命令可能需要写入 PlatformIO 安装目录下的锁文件，例如 `D:\Program\platformio\platforms.lock`。

## 后续扩展建议

- 如果要支持 ILI9341/ST7789/ST7796/ILI9806，需要继续从官方 `lcd_ex.c` 移植对应初始化函数。
- 如果要提升刷屏速度，可以再移植官方的区域填充函数，避免逐点调用带来的额外开销。
- 如果要显示中文，需要增加中文字库或图片/字模资源。
- 如果 ID 再次读错，优先检查 FSMC 地址、RS/A10、CS/NE4、读时序和 dummy read 次数。
- 如果屏幕只亮背光但无显示，优先确认是否执行了匹配 ID 的初始化序列以及 `0x11`/`0x29` 是否发送。
