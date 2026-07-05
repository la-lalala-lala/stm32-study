#include "usart/usart1.h"
#include <stdio.h>
#include <stm32f1xx.h>
#include "system/system.h"
#include "wifi/esp32.h"

int main(void){
    SystemClock_Config();
    usart1_init();
    printf("串口打印测试\r\n");

    esp32_init();
    // 1、测试 AT 启动
    printf("测试AT启动\r\n");
    esp32_send_cmd("AT\r\n", 4);

    // 2、查看版本信息
    printf("查看版本信息\r\n");
    esp32_send_cmd("AT+GMR\r\n", 8);

    while (1)
    {
    }
}
