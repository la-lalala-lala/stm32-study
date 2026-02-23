# STM32 Keil SPL 代码移植到 PlatformIO 指南

本项目记录了将 Keil 标准外设库 (SPL) 代码移植到 PlatformIO 的完整过程。

## 问题背景

在 Keil 中开发的 STM32 代码使用标准外设库，直接移植到 PlatformIO 时遇到以下问题：

1. 找不到 `stm32f10x.h` 头文件
2. SysTick 相关宏未定义
3. 系统时钟未正确配置，导致串口波特率错误

## 解决方案

### 1. 框架选择

PlatformIO 支持多种 STM32 框架：

| 框架 | 说明 |
|------|------|
| `cmsis` | 仅 CMSIS 核心，需要手动配置时钟 |
| `stm32cube` | STM32Cube HAL，包含完整时钟初始化 |

**推荐使用 `stm32cube` 框架**，它提供了完善的时钟初始化。

### 2. 头文件兼容

Keil SPL 与 STM32Cube 头文件命名不同：

| Keil SPL | STM32Cube |
|----------|-----------|
| `stm32f10x.h` | `stm32f1xx.h` |

创建兼容性头文件 `src/stm32f10x.h`：

```c
#ifndef __STM32F10X_H
#define __STM32F10X_H

#include "stm32f1xx.h"

#ifndef SysTick_CTRL_COUNTFLAG
#define SysTick_CTRL_COUNTFLAG     SysTick_CTRL_COUNTFLAG_Msk
#endif

#ifndef SysTick_CTRL_ENABLE
#define SysTick_CTRL_ENABLE        SysTick_CTRL_ENABLE_Msk
#endif

#endif
```

### 3. SysTick 宏差异

CMSIS Core 中 SysTick 宏命名带有 `_Msk` 后缀：

| Keil SPL | CMSIS/STM32Cube |
|----------|-----------------|
| `SysTick_CTRL_COUNTFLAG` | `SysTick_CTRL_COUNTFLAG_Msk` |
| `SysTick_CTRL_ENABLE` | `SysTick_CTRL_ENABLE_Msk` |

通过兼容性头文件定义别名解决。

### 4. 时钟配置

**这是最关键的问题！**

CMSIS 框架的 `SystemInit()` 仅重置 RCC，不配置 PLL，系统运行在 8MHz (HSI)。

而 Keil SPL 代码通常假设系统时钟为 72MHz：
- 波特率 `BRR = 0x271` 是按 72MHz 计算的
- SysTick 延时也是按 72MHz 计算的

**解决方案**：使用 `stm32cube` 框架，它会自动配置时钟。

### 5. platformio.ini 配置

```ini
[env:genericSTM32F103C8]
platform = ststm32
board = genericSTM32F103C8
framework = stm32cube
build_flags = 
    -D HSE_VALUE=8000000U
    -D SYSCLK_FREQ=72000000
upload_protocol = stlink
```

## 文件结构

```
src/
├── main.c
├── usart.c
├── usart.h
├── delay.c
├── delay.h
└── stm32f10x.h      # 兼容性头文件（新增）
```

## 移植要点总结

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| 找不到 `stm32f10x.h` | 框架头文件命名不同 | 创建兼容性头文件 |
| `SysTick_CTRL_*` 未定义 | CMSIS 宏命名不同 | 在兼容头文件中定义别名 |
| 串口乱码 | 时钟未配置 | 使用 `stm32cube` 框架 |

## 注意事项

1. **不要修改源代码**：通过创建兼容性头文件，保持原有代码不变
2. **时钟是关键**：确保系统时钟配置正确，否则波特率和延时都会出错
3. **晶振频率**：根据实际硬件设置 `HSE_VALUE`
