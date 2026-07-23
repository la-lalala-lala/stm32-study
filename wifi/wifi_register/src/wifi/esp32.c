#include "esp32.h"


static uint8_t esp32_read_buffer[1024] = {0};

/**
 * @description: 初始化ESP32
 */
void ESP32_Init(void){
    // 1. 初始化串口3(通过串口3与ESP32进行通讯)
    Driver_USART3_Init();
    // 2. 重启 ESP32
    const char *at_cmd = "AT+RST=0\r\n";
    ESP32_Send_CMD(at_cmd,strlen(at_cmd),RESPONSE_READY);
    Delay_ms(3000);
}

/**
 * @description: 发送AT指令
 * @param {uint8_t} *cmd 要发送的AT指令(指令必须以 \r\n结束)
 * @param {uint16_t} cmd_length 指令长度
 * @param {unin16_t} expect_result 期待的返回
 */
uint8_t ESP32_Send_CMD(const char *cmd, uint16_t cmd_length,char * expect_result){
    printf("发送指令：%s",cmd);
    Driver_USART3_SendString(cmd,cmd_length);
    do
    {
        ESP32_ReadResponse(esp32_read_buffer, 1024);
        printf("%s", esp32_read_buffer);
    }while(strstr((char *)esp32_read_buffer, expect_result) == NULL); // 一直等到读到OK
    printf("\r\n=====================\r\n");
}

uint16_t ESP32_ReadResponse(uint8_t respone_buff[], uint16_t size){
    // 清空缓冲区
    memset(respone_buff,0,size);
    uint16_t rxLen = 0;
    Driver_USART3_ReceiveString(respone_buff,&size);
}