#include "esp32.h"

uint8_t esp32_read_buffer[1024] = {0};

void esp32_init(void){
    //  1、初始化串口3(通过串口3与ESP32进行通讯)
    usart3_init();

    // 2、重启 ESP32
    const char *at_cmd = "AT+RST=0\r\n";
    esp32_send_cmd(at_cmd,strlen(at_cmd));
    Delay_ms(3000);
    usart3_flush_rx();
}


void esp32_send_cmd(const char *cmd, uint16_t cmd_length){
    uint8_t received_ok = 0;
    uint16_t total_rx_len = 0;

    printf("发送指令：%s",cmd);
    if (usart3_send_string((const uint8_t *)cmd,cmd_length) == 0)
    {
        printf("\r\nUSART3发送超时\r\n");
        return;
    }

    printf("读\r\n");

    for (uint8_t retry = 0; retry < 10; retry++)
    {
        uint16_t rx_len = esp32_read_response(esp32_read_buffer,1024);
        total_rx_len += rx_len;

        if (rx_len > 0)
        {
            printf("%s",(char *)esp32_read_buffer);
            if (strstr((char *)esp32_read_buffer,"OK") != NULL)
            {
                received_ok = 1;
                break;
            }
        }

        Delay_ms(100);
    }

    if (received_ok == 0)
    {
        printf("\r\nESP32未返回OK(rx=%u, mode=%s, stat=0x%04lX, RX=%u, TX=%u)\r\n",
               total_rx_len,
               usart3_debug_mode(),
               usart3_debug_status(),
               usart3_rx_pin_level(),
               usart3_tx_pin_level());
    }

    printf("\r\n=====================\r\n");
}

uint16_t esp32_read_response(uint8_t respone_buff[], uint16_t size){
    // 清空缓冲区
    memset(respone_buff,0,size);
    return usart3_receive_string(respone_buff,size);
}
