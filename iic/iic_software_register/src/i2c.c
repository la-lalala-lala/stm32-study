#include "i2c.h"

// 初始化
void I2C_Init(void)
{
    // 1. 配置时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // 2. GPIO工作模式配置：通用开漏输出 CNF-01，MODE-11
    // SCL: PB6
    // SDA: PB7
    GPIOB->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_CNF6 | GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
    GPIOB->CRL |= (GPIO_CRL_MODE6_1 | GPIO_CRL_MODE6_0); // MODE6: 11 (50MHz)
    GPIOB->CRL |= (GPIO_CRL_CNF6_0); // CNF6: 01 (通用开漏输出)
    GPIOB->CRL |= (GPIO_CRL_MODE7_1 | GPIO_CRL_MODE7_0); // MODE7: 11 (50MHz)
    GPIOB->CRL |= (GPIO_CRL_CNF7_0); // CNF7: 01 (通用开漏输出)
    
    // 初始状态：SCL和SDA都拉高
    SCL_HIGH;
    SDA_HIGH;
    I2C_DELAY;
}

// 发出起始信号
void I2C_Start(void)
{
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

// 发出停止信号
void I2C_Stop(void)
{
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

// 主机发出应答信号
void I2C_Ack(void)
{
    // 1. SCL拉低，SDA拉高，准备发出信号
    SCL_LOW;
    SDA_HIGH;
    I2C_DELAY;

    // 2. SCL保持不变，SDA拉低，输出应答
    SDA_LOW;
    I2C_DELAY;

    // 3. SDA保持不变，SCL拉高，开始数据线上信号采样
    SCL_HIGH;
    I2C_DELAY;

    // 4. SDA保持不变，SCL拉低，结束数据线上信号采样
    SCL_LOW;
    I2C_DELAY;

    // 5. SDA拉高，释放数据总线
    SDA_HIGH;
    I2C_DELAY;
}

// 主机发出非应答信号
void I2C_Nack(void)
{
    // 1. SCL拉低，SDA拉高，准备发出信号
    SCL_LOW;
    SDA_HIGH;
    I2C_DELAY;

    // 2. SDA保持不变，SCL拉高，开始数据线上信号采样
    SCL_HIGH;
    I2C_DELAY;

    // 3. SDA保持不变，SCL拉低，结束数据线上信号采样
    SCL_LOW;
    I2C_DELAY;
}

// 主机等待从设备发来应答信号
uint8_t I2C_Wait4Ack(void)
{
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
    if ((GPIOB->IDR & GPIO_IDR_IDR7) == 0)
    {
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

// 主机发送一个字节的数据（写入）
void I2C_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        // 1. SCL拉低，等待数据翻转
        SCL_LOW;
        I2C_DELAY;

        // 2. 取字节的最高位，向SDA写入数据
        if (byte & 0x80)
        {
            SDA_HIGH;
        }
        else
        {
            SDA_LOW;
        }
        I2C_DELAY;

        // 3. SCL拉高，数据采样
        SCL_HIGH;
        I2C_DELAY;

        // 4. SCL拉低，采样结束
        SCL_LOW;
        I2C_DELAY;

        // 5. 左移1位
        byte <<= 1;
    }
    
    // 释放SDA总线，准备接收应答
    SDA_HIGH;
    I2C_DELAY;
}

// 主机从EEPROM接收一个字节的数据（读取）
uint8_t I2C_ReadByte(void)
{
    // 定义一个变量，用来保存接收的数据
    uint8_t data = 0;
    
    // 保存当前的GPIO配置
    uint32_t original_crl = GPIOB->CRL;
    
    // 将SDA引脚设置为输入模式
    GPIOB->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
    GPIOB->CRL |= GPIO_CRL_CNF7_0; // 浮空输入

    // 循环处理每一位
    for (uint8_t i = 0; i < 8; i++)
    {
        // 1. SCL拉低，等待数据翻转
        SCL_LOW;
        I2C_DELAY;

        // 2. SCL拉高，开始采样
        SCL_HIGH;
        I2C_DELAY;

        // 3. 数据采样，读取SDA
        data <<= 1;     // 先做左移，新存入的位永远在最低位
        if (READ_SDA)
        {
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