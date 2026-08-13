#include "usart/usart1.h"
#include <stdio.h>
#include <stm32f1xx.h>
#include "system/system.h"
#include "wifi/esp32.h"
#include "ble/ble.h"
int main(void){
    SystemClock_Config();
    Driver_USART1_Init();
    printf("串口打印测试\r\n");
    
    ESP32_Ble_Init();

    uint8_t response[128];
    while (1){
        uint16_t len = ESP32_ReadResponse(response, sizeof(response), 2000);
        if (len > 0) {
            printf("收到 ESP32 响应：%s\r\n", response);
        }
    }

}
