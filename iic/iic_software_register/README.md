# I2C 软件模拟实验

## 项目简介
本项目演示了在 STM32F103C8T6 微控制器上使用软件模拟 I2C 协议与 M24C02 EEPROM 进行通信的实现。通过串口打印操作结果，验证 I2C 通信的正确性。

## 问题分析与解决

### 问题描述
1. 串口没有打印信息
2. 串口打印的数据全是 0
3. 向 EEPROM 写入 'a', 'b', 'c'，但读取到的是 'g', 'h', 'i'
4. EEPROM 无应答，读取到的数据都是 0xFF

### 问题原因与解决方案

#### 问题 1：串口没有打印信息

**原因**：系统时钟未正确初始化，导致串口波特率计算错误。

**解决方案**：
- 添加 `SystemClock_Config` 函数，配置系统时钟为 72MHz
- 在 `main` 函数开始处调用该函数，确保系统时钟在串口初始化前正确配置

#### 问题 2：串口打印的数据全是 0

**原因**：串口波特率与系统时钟不匹配，导致接收端无法正确解析数据。

**解决方案**：
- 根据系统时钟频率自动计算波特率
- 确保 `USART_Init` 函数能够根据当前系统时钟频率设置正确的波特率

#### 问题 3：读取到错误的数据

**原因**：I2C 通信中，读取数据时没有正确释放 SDA 总线，导致无法正确读取 EEPROM 返回的数据。

**解决方案**：
- 修改 `I2C_ReadByte` 函数，在读取数据时将 SDA 引脚设置为输入模式
- 确保在读取完成后恢复 SDA 引脚为输出模式

#### 问题 4：EEPROM 无应答

**原因**：I2C 通信时序不符合协议规范，导致无法与 EEPROM 建立有效的通信。

**解决方案**：
- 优化 `I2C_Init` 函数，确保 GPIO 引脚配置正确
- 修正 `I2C_Start` 函数，按照 I2C 协议规范实现起始信号时序
- 修正 `I2C_Stop` 函数，按照 I2C 协议规范实现停止信号时序
- 优化 `I2C_Wait4Ack` 函数，确保能正确检测从机的应答信号
- 优化 `I2C_SendByte` 函数，使其更加符合 I2C 协议规范

## 关键修改

### 1. 系统时钟配置
```c
void SystemClock_Config(uint32_t system_clock_freq) {
    uint32_t hse_startup_timeout = 10000; // Startup timeout for HSE (in ms)
    RCC->CR |= RCC_CR_HSEON; // Enable HSE
    while (!(RCC->CR & RCC_CR_HSERDY) && hse_startup_timeout--); // Wait until HSE is ready or timeout
    if (!(RCC->CR & RCC_CR_HSERDY)) {
        // HSE failed to start, default to HSI
        RCC->CR |= RCC_CR_HSION; // Enable HSI
        while (!(RCC->CR & RCC_CR_HSIRDY)); // Wait until HSI is ready
        RCC->CFGR &= ~RCC_CFGR_SW; // Select HSI as system clock source
        while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI); // Wait until HSI is used as system clock
    } else {
        // HSE started successfully
        RCC->CFGR &= ~RCC_CFGR_PLLSRC; // Clear PLL source
        RCC->CFGR |= RCC_CFGR_PLLSRC; // PLL source is HSE

        // Configure PLLMULL
        RCC->CFGR &= ~RCC_CFGR_PLLMULL; // Clear PLLMULL bits
        RCC->CFGR |= RCC_CFGR_PLLMULL9; // PLL multiplication factor = 9 (for 72 MHz with 8 MHz HSE)
        
        RCC->CR |= RCC_CR_PLLON; // Enable PLL
        while (!(RCC->CR & RCC_CR_PLLRDY)); // Wait until PLL is ready

        // Set Flash latency
        if (system_clock_freq > 24000000) {
            FLASH->ACR |= FLASH_ACR_LATENCY_2; // Two wait states
        } else {
            FLASH->ACR |= FLASH_ACR_LATENCY_1; // One wait state
        }

        // Configure AHB, APB1, APB2 prescalers
        RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // AHB = SYSCLK / 1
        RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 = HCLK / 2
        RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB2 = HCLK / 1

        // Select PLL as system clock source
        RCC->CFGR &= ~RCC_CFGR_SW; // Clear SW bits
        RCC->CFGR |= RCC_CFGR_SW_PLL; // Select PLL as system clock
        while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); // Wait until PLL is used as system clock
    }
}
```

### 2. I2C 起始信号修正
```c
void I2C_Start(void) {
    // 1. SCL和SDA都拉高
    SDA_HIGH;
    I2C_DELAY;
    SCL_HIGH;
    I2C_DELAY;

    // 2. SDA拉低（在SCL高电平时）
    SDA_LOW;
    I2C_DELAY;

    // 3. SCL拉低，准备发送数据
    SCL_LOW;
    I2C_DELAY;
}
```

### 3. I2C 停止信号修正
```c
void I2C_Stop(void) {
    // 1. SCL拉低，SDA拉低
    SCL_LOW;
    I2C_DELAY;
    SDA_LOW;
    I2C_DELAY;

    // 2. SCL拉高
    SCL_HIGH;
    I2C_DELAY;

    // 3. SDA拉高（在SCL高电平时）
    SDA_HIGH;
    I2C_DELAY;
}
```

### 4. I2C 读取函数优化
```c
uint8_t I2C_ReadByte(void) {
    // 定义一个变量，用来保存接收的数据
    uint8_t data = 0;
    
    // 保存当前的GPIO配置
    uint32_t original_crl = GPIOB->CRL;
    
    // 将SDA引脚设置为输入模式
    GPIOB->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
    GPIOB->CRL |= GPIO_CRL_CNF7_0; // 浮空输入

    // 循环处理每一位
    for (uint8_t i = 0; i < 8; i++) {
        // 1. SCL拉低，等待数据翻转
        SCL_LOW;
        I2C_DELAY;

        // 2. SCL拉高，开始采样
        SCL_HIGH;
        I2C_DELAY;

        // 3. 数据采样，读取SDA
        data <<= 1;     // 先做左移，新存入的位永远在最低位
        if (READ_SDA) {
            data |= 0x01;   // 先存入最低位，然后每次都左移1位
        }

        // 4. SCL拉低，结束采样
        SCL_LOW;
        I2C_DELAY;
    }
    
    // 恢复SDA引脚为输出模式
    GPIOB->CRL = original_crl;
    
    return data;
}
```

### 5. I2C 应答检测优化
```c
uint8_t I2C_Wait4Ack(void) {
    uint8_t ack = NACK;
    
    // 保存当前的GPIO配置
    uint32_t original_crl = GPIOB->CRL;
    
    // 1. SCL拉低
    SCL_LOW;
    I2C_DELAY;
    
    // 2. 将SDA引脚设置为输入模式
    GPIOB->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
    GPIOB->CRL |= GPIO_CRL_CNF7_0; // 浮空输入
    I2C_DELAY;

    // 3. SCL拉高，开始数据采样
    SCL_HIGH;
    I2C_DELAY;

    // 4. 读取SDA数据线上的电平
    if ((GPIOB->IDR & GPIO_IDR_IDR7) == 0) {
        ack = ACK; // 应答信号
    }

    // 5. SCL拉低，结束数据采样
    SCL_LOW;
    I2C_DELAY;

    // 6. 恢复SDA引脚为输出模式
    GPIOB->CRL = original_crl;
    SDA_HIGH; // 释放SDA总线
    I2C_DELAY;

    return ack;
}
```

## 运行结果
修复后，串口输出如下：

```
I2C软件模拟实验开始...
开始写入数据...
数据写入完成
开始读取数据...
数据读取完成
byte1 = a (0x61)    byte2 = b (0x62)    byte3 = c (0x63)
```

## 技术说明

### I2C 协议规范
- **起始信号**：在 SCL 高电平时，SDA 从高电平跳变到低电平
- **停止信号**：在 SCL 高电平时，SDA 从低电平跳变到高电平
- **数据传输**：在 SCL 低电平时，SDA 状态可以改变；在 SCL 高电平时，SDA 状态必须保持稳定
- **应答信号**：在发送完一个字节后，接收方会在第 9 个时钟周期拉低 SDA 表示应答

### GPIO 配置
- **SCL 引脚**：配置为通用开漏输出模式
- **SDA 引脚**：配置为通用开漏输出模式，在需要读取数据时切换为输入模式

### EEPROM 通信
- **设备地址**：M24C02 的设备地址为 0xA0（写）和 0xA1（读）
- **内部地址**：M24C02 的内部地址为 0x00-0xFF
- **写入周期**：EEPROM 有内部写入周期，通常需要 5ms 左右

### 系统配置
- **系统时钟**：72MHz
- **串口波特率**：115200
- **I2C 时钟频率**：约 100kHz

## 文件结构
- `src/i2c.c`：I2C 通信核心实现
- `src/i2c.h`：I2C 相关宏定义和函数声明
- `src/m24c02.c`：M24C02 EEPROM 操作函数
- `src/m24c02.h`：M24C02 相关宏定义和函数声明
- `src/usart.c`：串口初始化和重定向函数
- `src/usart.h`：串口相关宏定义和函数声明
- `src/delay.c`：延时函数
- `src/delay.h`：延时函数声明
- `src/main.c`：主函数，测试 I2C 通信

## 注意事项
1. 确保 I2C 总线上有合适的上拉电阻（通常为 4.7kΩ）
2. 注意 I2C 通信的时序，确保符合协议规范
3. 在 EEPROM 写入操作后，必须等待足够的时间让写入周期完成
4. 确保系统时钟配置正确，否则会影响串口波特率和 I2C 通信速度
5. 在读取 I2C 数据时，必须将 SDA 引脚设置为输入模式