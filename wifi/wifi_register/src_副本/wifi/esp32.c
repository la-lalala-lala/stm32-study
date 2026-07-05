#include "esp32.h"

uint8_t esp32_read_buffer[1024] = {0};

void esp32_init(void){
    //  1、初始化串口3(通过串口2与ESP32进行通讯)
    usart3_init();

    // 2、重启 ESP32
    uint8_t *at_cmd = "AT+RST=0\r\n";
    esp32_send_cmd(at_cmd,strlen((char*)at_cmd));
    Delay_ms(3000);
}


void esp32_send_cmd(uint8_t *cmd, uint16_t cmd_length){
    printf("发送指令：%s",cmd);
    usart3_send_string(cmd,cmd_length);
    printf("读");

    do{
        esp32_read_response(esp32_read_buffer,1024);
        printf("%s",esp32_read_buffer);
    }while (strstr((char *)esp32_read_buffer,"OK") == NULL);// 一直等到读到OK
    printf("\r\n=====================\r\n");
}

void esp32_read_response(uint8_t respone_buff[], uint16_t size){
    // 清空缓冲区
    memset(respone_buff,0,size);
    //uint16_t rx_len = 0;
    usart3_receive_string(respone_buff,size);
}