#include "esp32.h"

static uint8_t esp32_read_buffer[1024] = {0};

void esp32_init(void){
    usart3_init();

    const char *at_cmd = "AT+RST=0\r\n";
    esp32_send_cmd(at_cmd,strlen(at_cmd));
    Delay_ms(3000);
}

uint8_t esp32_send_cmd(const char *cmd, uint16_t cmd_length){
    uint8_t received_ok = 0;
    uint16_t total_rx_len = 0;

    printf("发送指令：%s",cmd);

    if (usart3_send_string((const uint8_t *)cmd,cmd_length) == 0)
    {
        printf("\r\nUSART3发送超时(SR=0x%04lX, RX=%u, TX=%u)\r\n",
               usart3_debug_status(),
               usart3_rx_pin_level(),
               usart3_tx_pin_level());
        return 0;
    }


    for (uint8_t retry = 0; retry < 10; retry++)
    {
        uint16_t rx_len = esp32_read_response(esp32_read_buffer,sizeof(esp32_read_buffer));
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
        printf("\r\nESP32未返回OK(rx=%u, SR=0x%04lX, RX=%u, TX=%u)\r\n",
               total_rx_len,
               usart3_debug_status(),
               usart3_rx_pin_level(),
               usart3_tx_pin_level());
    }

    printf("\r\n=====================\r\n");
    return received_ok;
}

uint16_t esp32_read_response(uint8_t respone_buff[], uint16_t size){
    memset(respone_buff,0,size);
    return usart3_receive_string(respone_buff,size);
}
