#include "esp32.h"


/**
 * @description: 硬件复位
 */
void ESP8266_HardwareReset(void){
    // esp32-01s 的 RST 脚必须拉低才能复位模块，拉高后模块恢复运行。
    /* RST 低电平有效 */
    HAL_GPIO_WritePin(ESP_RST_GPIO_Port, ESP_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    /* 释放 RST，由外部电阻拉高 */
    HAL_GPIO_WritePin(ESP_RST_GPIO_Port, ESP_RST_Pin, GPIO_PIN_SET);
}

/**
 * @description: 初始化ESP32
 */
void ESP32_Init(void){
    // 硬件复位
    ESP8266_HardwareReset();
    /* 1. 初始化串口3(通过串口3与ESP32进行通讯) */
    MX_USART3_UART_Init();
    HAL_Delay(3000);

    /* 2. 测试AT启动 */
    printf("测试AT启动\r\n");
    ESP32_Send_CMD("AT\r\n", 4,3000);
    HAL_Delay(1000);
}

/**
 * @description: 发送AT指令
 * @param {uint8_t} *cmd 要发送的AT指令(指令必须以 \r\n结束)
 * @param {uint16_t} cmdLength 指令长度
 * @param {uint32_t} timeout_ms 超时时间
 * @return 执行结果
 */
static uint8_t rBuff[ESP32_RX_BUFFER_SIZE];

bool ESP32_Send_CMD(uint8_t *cmd, uint16_t cmdLength,uint32_t timeout_ms){
    printf("send command:%s\r\n",cmd);
    uint32_t startTick = HAL_GetTick();
    HAL_UART_Transmit(&huart3, cmd, cmdLength, 1000);
    while ((HAL_GetTick() - startTick) < timeout_ms) {
        uint16_t receivedLength= ESP32_ReadResponse(rBuff,sizeof(rBuff),ESP32_RX_WAIT_MS);
        if (receivedLength == 0) {
            continue;
        }
        printf("%s", rBuff);
        if (strstr((char *)rBuff, "ERROR") != NULL) {
            printf("\r\n=====================\r\n");
            return false;
        }

        if (strstr((char *)rBuff, "OK") != NULL) {
            printf("\r\n=====================\r\n");
            return true;
        }
    }
    printf("ESP8266 response timeout\r\n");
    return false;
}

/**
 * @description: 发送AT指令后，用来接收响应
 *          要考虑到接收的响应为非定长数据。
 *
 * @param {uint8_t} responeBuff[] 存储接收的的响应的缓冲区。
 * @param {unin16_t} Size 缓冲区的长度
 * @param {uint32_t} timeout_ms 超时时间
 * @return 实际接收字节数
 */

uint16_t ESP32_ReadResponse(uint8_t responeBuff[], uint16_t size,uint32_t timeout_ms){
    // 实际接收的长度
    uint16_t receivedLength = 0;
    // 清空缓冲区
    memset(responeBuff, 0, size);
    HAL_UARTEx_ReceiveToIdle(&huart3, responeBuff, size-1, &receivedLength, timeout_ms);
    if (receivedLength >= size) {
        receivedLength = size - 1U;
    }
    responeBuff[receivedLength] = '\0';
    return receivedLength;
}