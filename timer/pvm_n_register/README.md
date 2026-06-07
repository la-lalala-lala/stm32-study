

---

## TIM1 有限次数 PWM 输出

### 功能描述

使用 STM32F103 高级定时器 TIM1 的**重复计数器(RCR)** 功能，实现 LED 闪烁固定次数后自动停止的效果。

### 核心原理

**重复计数器(RCR)的工作机制**：
- 普通定时器：每次计数器溢出都产生更新事件
- 高级定时器+RCR：RCR 递减到 0 时才产生更新事件

通过设置 RCR=4，定时器会在第 5 次溢出时才触发更新中断，从而实现输出 5 个 PWM 周期后自动停止。

### 硬件连接

| 引脚 | 功能 | 连接 |
|------|------|------|
| PA8 | TIM1_CH1 | LED（低电平有效） |

### 时序配置

| 参数 | 值 | 计算过程 | 结果 |
|------|-----|----------|------|
| 系统时钟 | 72MHz | - | - |
| 预分频器 PSC | 7199 | 72MHz/(7199+1) | 10kHz 计数频率 |
| 自动重装载 ARR | 4999 | (4999+1)/10kHz | 0.5s PWM 周期 |
| 重复计数器 RCR | 4 | 4+1=5 次溢出 | 5 个 PWM 周期 |
| 占空比 | 50% | CCR1=2500 | 亮 0.25s，灭 0.25s |
| 总闪烁时间 | 2.5s | 0.5s × 5 | 闪烁 5 次后停止 |

### 关键代码解析

#### 1. GPIO 配置（复用推挽输出）
```c
// PA8 配置为复用推挽输出，50MHz
GPIOA->CRH |= GPIO_CRH_MODE8;      // MODE8 = 11 (50MHz)
GPIOA->CRH |= GPIO_CRH_CNF8_1;     // CNF8 = 10 (复用推挽)
GPIOA->CRH &= ~GPIO_CRH_CNF8_0;
```

#### 2. 时基单元配置
```c
// 预分频器：10kHz 计数频率
TIM1->PSC = 7199;

// 自动重装载：0.5s 周期
TIM1->ARR = 4999;

// 向上计数
TIM1->CR1 &= ~TIM_CR1_DIR;

// 重复计数器：5 次溢出后产生更新事件
TIM1->RCR = 4;
```

#### 3. PWM 模式配置
```c
// 通道 1 配置为 PWM 模式 1
TIM1->CCMR1 &= ~TIM_CCMR1_CC1S;    // 输出模式
TIM1->CCMR1 |= TIM_CCMR1_OC1M_2;   // OC1M = 110 (PWM 模式 1)
TIM1->CCMR1 |= TIM_CCMR1_OC1M_1;
TIM1->CCMR1 &= ~TIM_CCMR1_OC1M_0;

// 50% 占空比
TIM1->CCR1 = 2500;

// 低电平有效（闪烁后熄灭）
TIM1->CCER |= TIM_CCER_CC1P;
```

#### 4. 更新事件与中断配置
```c
// 仅允许计数器溢出产生更新事件
TIM1->CR1 |= TIM_CR1_URS;

// 手动产生更新事件，刷新寄存器
TIM1->EGR |= TIM_EGR_UG;

// 使能更新中断
TIM1->DIER |= TIM_DIER_UIE;

// NVIC 配置
NVIC_SetPriorityGrouping(3);
NVIC_SetPriority(TIM1_UP_IRQn, 3);
NVIC_EnableIRQ(TIM1_UP_IRQn);
```

#### 5. 中断服务程序
```c
void TIM1_UP_IRQHandler(void) {
    // 清除中断标志位
    TIM1->SR &= ~TIM_SR_UIF;
    // 停止计数器
    TIM1->CR1 &= ~TIM_CR1_CEN;
}
```

### 工作流程

```
启动 TIM1_Start()
    ↓
计数器开始计数 (CEN=1)
    ↓
每 0.5s 产生一次溢出
    ↓
重复计数器 RCR 从 4 递减到 0
    ↓
第 5 次溢出时产生更新事件
    ↓
触发更新中断
    ↓
中断服务程序中停止定时器
    ↓
PWM 输出停止，LED 闪烁 5 次后熄灭
```

### 关键要点

1. **URS 位的作用**：`TIM_CR1_URS=1` 时，只有计数器溢出才产生更新事件，避免其他事件触发中断
2. **手动更新事件**：`TIM_EGR_UG=1` 用于初始化时刷新所有寄存器
3. **高级定时器特性**：必须设置 `TIM_BDTR_MOE` 主输出使能位，否则 PWM 无输出
4. **中断处理**：在中断中清除标志位并停止定时器，实现自动停止功能
