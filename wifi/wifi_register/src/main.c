#include "usart/usart1.h"
#include <stdio.h>
#include <stm32f1xx.h>
#include "system/system.h"
#include "wifi/esp32.h"

int main(void){
    SystemClock_Config();
    Driver_USART1_Init();
    printf("串口打印测试\r\n");
    
    ESP32_Init();
    // 1、测试 AT 启动
    printf("测试AT启动\r\n");
    ESP32_Send_CMD("AT\r\n", 4,RESPONSE_OK);

    // 2、查看版本信息
    printf("查看版本信息\r\n");
    ESP32_Send_CMD("AT+GMR\r\n", 8,RESPONSE_OK);

    while (1)
    {

    }
}
