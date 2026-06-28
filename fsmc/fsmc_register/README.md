# STM32F103ZE FSMC 外扩 SRAM 实验记录

本实验基于正点原子战舰 V4 STM32F103ZET6 开发板，目标是使用 FSMC 访问板载 1MB 外部 SRAM。硬件参考手册中给出的关键信息如下：

- 外部 SRAM 容量为 1MB，16 位并口访问。
- 外部 SRAM 片选接 `FSMC_NE3`，对应 MCU 引脚 `PG10`。
- LCD 片选接 `FSMC_NE4`，对应 `PG12`，不要和 SRAM 混用。
- SRAM 挂在 FSMC Bank1 的第 3 个 NOR/SRAM 区，所以 CPU 访问基地址为 `0x68000000`。

## FSMC 地址空间

STM32F1 FSMC Bank1 的 NOR/SRAM 区地址划分如下：

```text
Bank1 NE1: 0x60000000
Bank1 NE2: 0x64000000
Bank1 NE3: 0x68000000  <-- 战舰 V4 板载 SRAM
Bank1 NE4: 0x6C000000  <-- 战舰 V4 LCD
```

因此，板载 SRAM 应通过 `0x68000000` 开始的地址访问。

## GPIO 初始化整理

FSMC 的 GPIO 需要配置为复用推挽输出。STM32F1 每个 GPIO 引脚由 `MODE[1:0]` 和 `CNF[1:0]` 配置：

```text
MODE = 11: 输出模式，最大速度 50MHz
CNF  = 10: 复用功能推挽输出
```

虽然 SRAM 数据线既要写也要读，但 GPIO 仍配置成复用推挽输出。读写方向由 FSMC 外设硬件控制，不需要软件手动切换输入/输出。

### 地址线

当前 `fsmc.c` 配置了外部 SRAM 地址线：

```text
PF0  -> FSMC_A0
PF1  -> FSMC_A1
PF2  -> FSMC_A2
PF3  -> FSMC_A3
PF4  -> FSMC_A4
PF5  -> FSMC_A5
PF12 -> FSMC_A6
PF13 -> FSMC_A7
PF14 -> FSMC_A8
PF15 -> FSMC_A9
PG0  -> FSMC_A10
PG1  -> FSMC_A11
PG2  -> FSMC_A12
PG3  -> FSMC_A13
PG4  -> FSMC_A14
PG5  -> FSMC_A15
PD11 -> FSMC_A16
PD12 -> FSMC_A17
PD13 -> FSMC_A18
```

### 数据线

外部 SRAM 是 16 位数据总线，因此需要配置 `FSMC_D0-D15`：

```text
PD14 -> FSMC_D0
PD15 -> FSMC_D1
PD0  -> FSMC_D2
PD1  -> FSMC_D3
PE7  -> FSMC_D4
PE8  -> FSMC_D5
PE9  -> FSMC_D6
PE10 -> FSMC_D7
PE11 -> FSMC_D8
PE12 -> FSMC_D9
PE13 -> FSMC_D10
PE14 -> FSMC_D11
PE15 -> FSMC_D12
PD8  -> FSMC_D13
PD9  -> FSMC_D14
PD10 -> FSMC_D15
```

### 控制线

SRAM 还需要读写、字节选择和片选信号：

```text
PD4  -> FSMC_NOE   读使能，接 SRAM 的 OE
PD5  -> FSMC_NWE   写使能，接 SRAM 的 WE
PE0  -> FSMC_NBL0  低字节使能
PE1  -> FSMC_NBL1  高字节使能
PG10 -> FSMC_NE3   SRAM 片选
```

这里 `PG10` 配置为 FSMC 复用功能是正确的，因为硬件手册中 `PG10/FSMC_NE3` 就是外部 SRAM 片选。`PG12/FSMC_NE4` 是 LCD 片选。

## FSMC 寄存器初始化

当前实验使用寄存器方式配置 `FSMC_Bank1->BTCR[4]` 和 `FSMC_Bank1->BTCR[5]`：

```c
FSMC_Bank1->BTCR[4] |= FSMC_BCRx_MBKEN;
FSMC_Bank1->BTCR[4] &= ~FSMC_BCRx_MUXEN;
FSMC_Bank1->BTCR[4] &= ~FSMC_BCRx_MTYP;
FSMC_Bank1->BTCR[4] &= ~FSMC_BCRx_MWID;
FSMC_Bank1->BTCR[4] |= FSMC_BCRx_MWID_0;
FSMC_Bank1->BTCR[4] |= FSMC_BCRx_WREN;

FSMC_Bank1->BTCR[5] &= ~FSMC_BTRx_ADDSET;
FSMC_Bank1->BTCR[5] &= ~FSMC_BTRx_ADDHLD;
FSMC_Bank1->BTCR[5] &= ~FSMC_BTRx_DATAST;
FSMC_Bank1->BTCR[5] |= (FSMC_BTRx_DATAST_6 |
                         FSMC_BTRx_DATAST_2 |
                         FSMC_BTRx_DATAST_1 |
                         FSMC_BTRx_DATAST_0);
```

含义：

- `BTCR[4]` 对应 Bank1 SRAM3 的 BCR3，也就是 NE3。
- `BTCR[5]` 对应 Bank1 SRAM3 的 BTR3。
- `FSMC_BCRx_MBKEN` 使能存储块。
- `FSMC_BCRx_MUXEN = 0` 表示不使用地址/数据复用。
- `FSMC_BCRx_MTYP = 0` 表示存储器类型为 SRAM。
- `FSMC_BCRx_MWID_0` 设置数据总线宽度为 16 位。
- `FSMC_BCRx_WREN` 打开写使能。
- `ADDSET = 0` 表示地址建立时间为 `0 + 1 = 1` 个 HCLK 周期。
- `ADDHLD = 0` 表示地址保持时间为 `0 + 1 = 1` 个 HCLK 周期。
- `DATAST_6 | DATAST_2 | DATAST_1 | DATAST_0` 等价于 `64 + 4 + 2 + 1 = 71`。
- `DATAST = 71` 表示数据阶段为 `71 + 1 = 72` 个 HCLK 周期。在 72MHz HCLK 下约为 1us，远大于 SRAM 约 55ns 的要求，功能上保守可用，但速度偏慢。

注意：当前 STM32CubeF1 CMSIS 头文件中使用的是 `FSMC_BCRx_*`、`FSMC_BTRx_*` 通用位名，不是 `FSMC_BCR3_*`、`FSMC_BTR3_*`。使用后者会编译失败。

## main.c 地址问题整理

之前尝试过如下写法：

```c
uint8_t v1 __attribute__((at(0x68000000)));
uint8_t v2 __attribute__((at(0x68000004)));
```

这个写法在当前 PlatformIO/GCC 工具链下不会把变量放到绝对地址。实际编译后，`v1` 和 `v2` 被链接进了普通 `.bss`，也就是片内 SRAM。例如符号表中能看到类似结果：

```text
20000094 B v1
20000095 B v2
```

所以串口打印会出现：

```text
v1= 0x20000094
v2= 0x20000095
```

这不是 FSMC 初始化失败，而是变量没有被放到 `0x68000000`。在 GCC 工程里，若要把变量真正链接到外部 SRAM，需要修改链接脚本；本实验只是验证 FSMC 访问，因此更直接可靠的方式是使用指针访问映射地址。

## 推荐访问方式

当前 `main.c` 使用如下方式访问外部 SRAM：

```c
#define FSMC_SRAM_BASE ((uint32_t)0x68000000)
#define V1             ((volatile uint8_t *)(FSMC_SRAM_BASE + 0x00))
#define V2             ((volatile uint8_t *)(FSMC_SRAM_BASE + 0x04))
#define V3             ((volatile uint8_t *)(FSMC_SRAM_BASE + 0x08))
#define V4             ((volatile uint8_t *)(FSMC_SRAM_BASE + 0x0C))
```

读写示例：

```c
*V1 = 11;
printf("v1= %p , %d\n", (void *)V1, *V1);

*V2 = 12;
printf("v2= %p , %d\n", (void *)V2, *V2);
```

预期输出中的外部 SRAM 地址应类似：

```text
v1= 0x68000000 , 11
v2= 0x68000004 , 12
v3= 0x68000008 , 13
v4= 0x6800000c , 24
```

这里使用 `volatile` 是为了告诉编译器：这些地址对应外部硬件/外部总线访问，不能随意优化读写操作。

## 地址输出如何判断

实验中不同变量会出现在不同地址空间：

```text
0x08000000 起: Flash，代码、const 全局数据、字符串常量
0x20000000 起: 片内 SRAM，全局非 const 变量、.data、.bss、栈
0x68000000 起: FSMC Bank1 NE3，板载外部 SRAM
```

例子：

- 局部变量 `uint8_t v`、局部数组 `buff[1024]` 在栈上，地址通常接近 `0x20010000`，例如 `0x2000fbf8`。
- `const uint8_t buff1[1024]` 是只读全局数组，会放在 Flash，例如 `0x08001620`。
- `const char *name = "hello"` 中，`name` 变量本身在 SRAM，但它指向的字符串 `"hello"` 在 Flash，所以打印 `name` 会得到 `0x0800xxxx`。
- `V1/V2/V3/V4` 是显式指向 `0x68000000` 的指针，才是真正访问外部 SRAM。

## 构建验证

修改后已执行：

```sh
~/.platformio/penv/bin/platformio run
```

构建通过。由于 PlatformIO 需要写入 `~/.platformio/platforms.lock` 和缓存，在受限环境中运行可能需要外部权限。
