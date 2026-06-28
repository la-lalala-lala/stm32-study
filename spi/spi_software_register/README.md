# NM25Q128EVB 软件 SPI 调试记录

## 文档依据

当前工程以根目录下的 `NM25Q128EVB.pdf` 为准，不再参考之前的 Micron N25Q128 文档。

根据 `NM25Q128EVB.pdf`：

- `RDID` 指令为 `0x9F`
- `RDID` 返回 3 字节：`Manufacturer ID` + `Memory Type` + `Memory Density`
- `NM25Q128EVB` 的 `RDID` 期望值为 `52 21 18`
- 因此代码中判断为：

```c
#define NM25Q128_MANUFACTURER_ID 0x52U
#define NM25Q128_MEMORY_TYPE     0x21U
#define NM25Q128_CAPACITY_ID     0x18U
#define NM25Q128_DEVICE_ID       0x2118U
```

实际串口验证结果：

```text
mid=0x52,did=0x2118
id ok
```

说明 `0x9F` 读 ID 已经正常。

## SPI 引脚修正

当前软件 SPI 使用 GPIOB：

```text
CS   PB12
SCK  PB13
MISO PB14
MOSI PB15
```

排查时发现原代码存在两个关键问题：

- 注释写的是 `CS PB12`，但代码曾使用 `GPIOC->ODR12`
- 注释写的是 `MISO PB14`，但代码曾读取 `GPIOB->IDR15`

这会导致 Flash 没有被正确片选，或者主机读到的是 MOSI 引脚状态，表现为：

```text
mid=0xff,did=0xffff
```

修正后需要保证 `CS` 和 `MISO` 都使用 GPIOB 对应引脚，例如：

```c
#define CS_HIGH (GPIOB->ODR |= GPIO_ODR_ODR12)
#define CS_LOW  (GPIOB->ODR &= ~GPIO_ODR_ODR12)

#define MISO_READ (GPIOB->IDR & GPIO_IDR_IDR14)
```

## GPIO 输出方式

软件 SPI 的 `CS/SCK/MOSI` 可以使用 `ODR` 输出：

```c
#define SCK_HIGH  (GPIOB->ODR |= GPIO_ODR_ODR13)
#define SCK_LOW   (GPIOB->ODR &= ~GPIO_ODR_ODR13)
#define MOSI_HIGH (GPIOB->ODR |= GPIO_ODR_ODR15)
#define MOSI_LOW  (GPIOB->ODR &= ~GPIO_ODR_ODR15)
#define CS_HIGH   (GPIOB->ODR |= GPIO_ODR_ODR12)
#define CS_LOW    (GPIOB->ODR &= ~GPIO_ODR_ODR12)
```

也可以使用 `BSRR/BRR` 输出：

```c
#define SCK_HIGH  (GPIOB->BSRR = GPIO_BSRR_BS13)
#define SCK_LOW   (GPIOB->BRR  = GPIO_BRR_BR13)
#define MOSI_HIGH (GPIOB->BSRR = GPIO_BSRR_BS15)
#define MOSI_LOW  (GPIOB->BRR  = GPIO_BRR_BR15)
```

`ODR |= bit` / `ODR &= ~bit` 是读-改-写整个端口寄存器；`BSRR/BRR` 是对单个引脚的置位/复位操作，理论上更适合 bit-bang SPI。

本工程最新实测结论：

- `ODR` 方式可以正常读 ID
- `BSRR/BRR` 方式也可以正常读 ID
- 之前读不到 ID 的核心原因不是 `ODR` 本身，而是 `CS/MISO` 引脚配置错误，以及 Flash 初始化状态没有处理好

## 初始化唤醒和软件复位

`nm25q128_init()` 中除了 `spi_init()`，还发送了 3 个控制指令：

```c
spi_swap_byte(0xab);
spi_swap_byte(0x66);
spi_swap_byte(0x99);
```

对应 `NM25Q128EVB.pdf`：

```text
0xAB  Release From Deep Power Down (RDP)
0x66  Enable Reset (RSTEN)
0x99  Reset Memory (RST)
```

作用：

- `0xAB`：让 Flash 从 Deep Power-Down 状态退出
- `0x66`：使能软件复位
- `0x99`：执行软件复位，让 Flash 回到默认 SPI 状态

本工程实测：只保留 `spi_init()`、删除 `0xAB/0x66/0x99` 后不能稳定读通 ID，因此当前保留这段初始化。

## 当前读 ID 流程

`nm25q128_read_id()` 流程：

```text
CS 拉低
发送 0x9F
读取 Manufacturer ID
读取 Memory Type
读取 Memory Density
CS 拉高
```

代码中组合为：

```c
*mid = spi_swap_byte(0xff);
*did = 0;
*did |= spi_swap_byte(0xff) << 8;
*did |= spi_swap_byte(0xff) << 0;
```

期望输出：

```text
mid=0x52,did=0x2118
id ok
```
