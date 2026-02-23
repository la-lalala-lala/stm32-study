#include "m24c02.h"

// 初始化
void M24C02_Init(void)
{
    I2C_Init();
}

// 向EEPROM写入一个字节
void M24C02_WriteByte(uint8_t innerAddr, uint8_t byte)
{
    // 1. 发出开始信号
    I2C_Start();

    // 2. 发送写地址
    I2C_SendByte(W_ADDR);

    // 3. 等待EEPROM应答
    uint8_t ack = I2C_Wait4Ack();

    if (ack == ACK)
    {
        // 4. 发送内部地址
        I2C_SendByte(innerAddr);

        // 5. 等待应答
        I2C_Wait4Ack();

        // 6. 发送具体数据
        I2C_SendByte(byte);

        // 7. 等待应答
        I2C_Wait4Ack();

        // 8. 发出一个停止信号
        I2C_Stop();
    }
    else
    {
        printf("EEPROM 无应答！\n");
        I2C_Stop();
    }

    // 延迟等待写入周期结束
    Delay_ms(10);
}

// 读取EEPROM的一个字节
uint8_t M24C02_ReadByte(uint8_t innerAddr)
{
    // 1. 发出开始信号
    I2C_Start();

    // 2. 发送写地址（假写）
    I2C_SendByte(W_ADDR);

    // 3. 等待EEPROM应答
    I2C_Wait4Ack();

    // 4. 发送内部地址
    I2C_SendByte(innerAddr);

    // 5. 等待应答
    I2C_Wait4Ack();

    // 6. 发出开始信号
    I2C_Start();

    // 7. 发送读地址（真读）
    I2C_SendByte(R_ADDR);

    // 8. 等待EEPROM应答
    I2C_Wait4Ack();

    // 9. 读取一个字节
    uint8_t byte = I2C_ReadByte();

    // 10. 发送一个非应答
    I2C_Nack();

    // 11. 发出一个停止信号
    I2C_Stop();

    return byte;
}

// 连续写入多个字节（页写）
void M24C02_WriteBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t size)
{
    // 1. 发出开始信号
    I2C_Start();

    // 2. 发送写地址
    I2C_SendByte(W_ADDR);

    // 3. 等待EEPROM应答
    uint8_t ack = I2C_Wait4Ack();

    if (ack == ACK)
    {
        // 4. 发送内部地址
        I2C_SendByte(innerAddr);

        // 5. 等待应答
        I2C_Wait4Ack();

        // 利用循环不停发送数据
        for (uint8_t i = 0; i < size; i++)
        {
            // 6. 发送具体数据
            I2C_SendByte(bytes[i]);

            // 7. 等待应答
            I2C_Wait4Ack();
        }

        // 8. 发出一个停止信号
        I2C_Stop();
    }

    // 延迟等待写入周期结束
    Delay_ms(5);
}

// 连续读取多个字节
void M24C02_ReadBytes(uint8_t innerAddr, uint8_t *buffer, uint8_t size)
{
    // 1. 发出开始信号
    I2C_Start();

    // 2. 发送写地址（假写）
    I2C_SendByte(W_ADDR);

    // 3. 等待EEPROM应答
    I2C_Wait4Ack();

    // 4. 发送内部地址
    I2C_SendByte(innerAddr);

    // 5. 等待应答
    I2C_Wait4Ack();

    // 6. 发出开始信号
    I2C_Start();

    // 7. 发送读地址（真读）
    I2C_SendByte(R_ADDR);

    // 8. 等待EEPROM应答
    I2C_Wait4Ack();

    // 利用循环连续读取多个字节
    for (uint8_t i = 0; i < size; i++)
    {
        // 9. 读取一个字节
        buffer[i] = I2C_ReadByte();

        // 10. 发送一个应答或非应答
        if (i < size - 1)
        {
            I2C_Ack();
        }
        else
        {
            I2C_Nack();
        }
    }

    // 11. 发出一个停止信号
    I2C_Stop();
}