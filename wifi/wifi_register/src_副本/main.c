#include "usart/usart1.h"
#include <stdio.h>
#include <stm32f1xx.h>
#include "system/system.h"
#include "wifi/esp32.h"

uint8_t buffer[10000];

int main(void){
    SystemClock_Config();
    usart1_init();
    printf("串口打印测试\r\n");

    esp32_init();
    // 1、测试 AT 启动
    esp32_send_cmd("AT\r\n", 4);
    esp32_read_response(buffer, 1000);
    printf("测试AT启动: %s", buffer);
    memset(buffer, 0, sizeof(buffer));

    // 2、查看版本信息
    esp32_send_cmd("AT+GMR\r\n", 8);
    esp32_read_response(buffer, 1000);
    printf("查看版本信息: %s", buffer);

}
