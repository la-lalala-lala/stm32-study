# STM32F103 SPI2 硬件 SPI 修改记录

本工程使用 STM32F103ZE，通过寄存器方式配置 SPI2 访问 NM25Q128 Flash。

## 当前硬件连接

| 信号 | 引脚 | 配置 |
| --- | --- | --- |
| CS | PB12 | 通用推挽输出 |
| SCK | PB13 | SPI2_SCK，复用推挽输出 |
| MISO | PB14 | SPI2_MISO，浮空输入 |
| MOSI | PB15 | SPI2_MOSI，复用推挽输出 |

## 修改过程

1. 确认当前使用的是 SPI2，不是 SPI1。

   依据是 SPI2 时钟通过 `RCC->APB1ENR` 打开，引脚使用 `PB13/PB14/PB15`：

   ```c
   RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
   RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
   ```

2. 将原来的软件模拟 SPI 改为硬件 SPI2。

   发送和接收通过 SPI2 数据寄存器完成，发送前等待 `TXE`，接收前等待 `RXNE`：

   ```c
   while ((SPI2->SR & SPI_SR_TXE) == 0) {
   }
   SPI2->DR = ch;

   while ((SPI2->SR & SPI_SR_RXNE) == 0) {
   }
   return (uint8_t)SPI2->DR;
   ```

3. 修正 GPIOB CRH 的引脚模式。

   STM32F1 每个 GPIO 引脚由 4 位控制：`CNF[1:0] + MODE[1:0]`。

   | 引脚 | 目标 | 四位配置 |
   | --- | --- | --- |
   | PB13 SCK | 复用推挽输出 | `1011` |
   | PB15 MOSI | 复用推挽输出 | `1011` |
   | PB12 CS | 通用推挽输出 | `0011` |
   | PB14 MISO | 浮空输入 | `0100` |

4. 修正 8 位数据帧配置。

   错误写法：

   ```c
   SPI2->CR1 |= ~SPI_CR1_DFF;
   ```

   这会把 `CR1` 里除 `DFF` 外的大量位都置 1，污染 SPI 配置。

   正确写法：

   ```c
   SPI2->CR1 &= ~SPI_CR1_DFF;
   ```

   `DFF = 0` 表示 8 位数据帧。

## SPI2 波特率计算

本工程在 `src/system/system.c` 中配置：

```c
HCLK  = SYSCLK = 72MHz
PCLK1 = HCLK / 2 = 36MHz
PCLK2 = HCLK     = 72MHz
```

SPI2 挂在 APB1，所以 SPI2 的输入时钟是：

```text
fPCLK = PCLK1 = 36MHz
```

SPI 的实际 SCK 频率由 `SPI_CR1_BR[2:0]` 决定：

| BR[2:0] | 分频 | SPI2 SCK |
| --- | --- | --- |
| `000` | `/2` | 18MHz |
| `001` | `/4` | 9MHz |
| `010` | `/8` | 4.5MHz |
| `011` | `/16` | 2.25MHz |
| `100` | `/32` | 1.125MHz |
| `101` | `/64` | 562.5kHz |
| `110` | `/128` | 281.25kHz |
| `111` | `/256` | 140.625kHz |

因此，如果 SPI2 要配置为 9MHz，应选择 4 分频，也就是 `BR[2:0] = 001`：

```c
SPI2->CR1 &= ~SPI_CR1_BR;
SPI2->CR1 |= SPI_CR1_BR_0; // 36MHz / 4 = 9MHz
```

之前容易误写成 8 分频：

```c
SPI2->CR1 |= SPI_CR1_BR_1; // BR = 010
```

但 `BR = 010` 对 SPI2 来说是：

```text
36MHz / 8 = 4.5MHz
```

注意：如果换成 SPI1，SPI1 挂在 APB2，当前工程中 `PCLK2 = 72MHz`，此时要得到 9MHz 才是选择 8 分频，即 `BR[2:0] = 010`。

## 当前 SPI2 关键配置

```c
SPI2->CR1 &= ~SPI_CR1_DFF;      // 8 位数据帧
SPI2->CR1 |= SPI_CR1_SSM;       // 软件 NSS 管理
SPI2->CR1 |= SPI_CR1_SSI;
SPI2->CR2 &= ~SPI_CR2_SSOE;     // 不使用硬件 NSS 输出
SPI2->CR1 &= ~SPI_CR1_LSBFIRST; // 高位先行
SPI2->CR1 &= ~SPI_CR1_BR;
SPI2->CR1 |= SPI_CR1_BR_0;      // 4 分频，SPI2 SCK = 9MHz
SPI2->CR1 |= SPI_CR1_MSTR;      // 主机模式
SPI2->CR1 &= ~SPI_CR1_CPHA;
SPI2->CR1 &= ~SPI_CR1_CPOL;     // SPI Mode 0
SPI2->CR1 |= SPI_CR1_SPE;       // 使能 SPI2
```
