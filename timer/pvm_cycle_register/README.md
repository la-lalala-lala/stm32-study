## 重要坑点记录

### TIM3_CH2 引脚重映射问题

**问题描述**：在使用 TIM3_CH2 输出 PWM 到 PB5 时，发现 LED 不亮或没有呼吸效果。

**根本原因**：
- **TIM3_CH2 的默认引脚是 PA7**，而不是 PB5
- **PB5 是 TIM3_CH2 的重映射引脚**（Remap），需要通过 AFIO_MAPR 寄存器配置重映射

**STM32F103 TIM3 引脚映射表**：

| 通道 | 默认引脚<br>（无重映射） | 部分重映射<br>（AFIO_MAPR[11:10]=10） | 完全重映射<br>（AFIO_MAPR[11:10]=11） |
|------|------------------------|--------------------------------------|--------------------------------------|
| CH1  | PA6                    | **PB4**                              | PC6                                  |
| CH2  | PA7                    | **PB5**                              | PC7                                  |
| CH3  | PB0                    | PB0                                  | PC8                                  |
| CH4  | PB1                    | PB1                                  | PC9                                  |

**解决方案**：

在使用 PB5 作为 TIM3_CH2 输出时，必须配置 AFIO 重映射：

```c
// 1. 开启 AFIO 时钟
RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

// 2. 配置 TIM3 部分重映射（AFIO_MAPR[11:10] = 10）
AFIO->MAPR &= ~(3U << 10);  // 清除 TIM3_REMAP 位
AFIO->MAPR |= (2U << 10);   // 设置部分重映射

// 3. 配置 GPIO 为复用推挽输出
GPIOB->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5);
GPIOB->CRL |= (GPIO_CRL_CNF5_1 | GPIO_CRL_MODE5);
```

**关键提示**：
- 不配置重映射时，TIM3_CH2 输出到 PA7
- 配置部分重映射后，TIM3_CH2 输出到 PB5
- 必须同时开启 AFIO 时钟，否则重映射配置无效
