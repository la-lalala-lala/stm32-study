#include "usart/usart1.h"
#include <stdio.h>
#include <stm32f1xx.h>
#include "system/system.h"
#include "wifi/esp32.h"

int main(void){
    SystemClock_Config();
    Driver_USART1_Init();
    printf("串口打印测试\r\n");
    
    // 1、测试 AT 启动
    if (ESP32_Init() != ESP32_SUCCESS){
        printf("ESP32初始化失败\r\n");
        while (1){
        }
    }

    // 2、查看版本信息
    printf("查看版本信息\r\n");
    ESP32_Send_CMD("AT+GMR\r\n", RESPONSE_OK, 1000);

    // 3、启动网络
    if (ESP32_Start_Station() == ESP32_ERROR){
        printf("连接wifi失败\r\n");
        while (1){
        }
    }

    // 4、开启UDP客户端
    if (Esp32_Start_UDP_Client() == ESP32_ERROR){
        printf("连接UDP服务失败\r\n");
        while (1){
        }
    }

    // 5、UDP客户端数据处理
    uint8_t rx_Buff[1024] = {0};
    uint16_t real_receive_len = 0;
    uint8_t id = 0;
    uint8_t ip[16] = {0};
    // 服务端的
    uint16_t port = 9000;
    uint8_t udp_reply[] = "UDP client received.\n";
    uint8_t ping[] = "ping\n";
    Esp32_Send_Data(ping,(uint16_t)(sizeof(ping) - 1U), NULL);
    while (1){
        // 监听esp32 c3（串口3）读到的数据
        Esp32_Read_Data(rx_Buff, 1024, &real_receive_len, NULL, ip, &port);
        if (real_receive_len > 0){
           printf("ip:%s,port:%d,data:%s\n", ip, port, rx_Buff);
            //Esp32_Send_Data(rx_Buff, real_receive_len, id);
            Esp32_Send_Data(udp_reply,(uint16_t)(sizeof(udp_reply) - 1U), NULL);
            real_receive_len = 0;
        }
    }

    // // 4、开启TCP客户端
    // if (Esp32_Start_TCP_Client() == ESP32_ERROR){
    //     printf("连接TCP服务失败\r\n");
    //     while (1){
    //     }
    // }

    // // 5、TCP客户端数据处理
    // uint8_t rx_Buff[1024] = {0};
    // uint16_t real_receive_len = 0;
    // uint8_t id = 0;
    // uint8_t ip[16] = {0};
    // // 服务端的
    // uint16_t port = 9000;
    // uint8_t tcp_reply[] = "TCP client received.\n";
    // while (1){
    //     // 监听esp32 c3（串口3）读到的数据
    //     Esp32_Read_Data(rx_Buff, 1024, &real_receive_len, NULL, ip, &port);
    //     if (real_receive_len > 0){
    //        printf("ip:%s,port:%d,data:%s\n", ip, port, rx_Buff);
    //         //Esp32_Send_Data(rx_Buff, real_receive_len, id);
    //         Esp32_Send_Data(tcp_reply,(uint16_t)(sizeof(tcp_reply) - 1U), NULL);
    //         real_receive_len = 0;
    //     }
    // }

    // // 4、开启TCP服务
    // Esp32_Start_TCP_Server();

    // // 5、TCP服务端数据处理
    // uint8_t rx_Buff[1024] = {0};
    // uint16_t real_receive_len = 0;
    // uint8_t id = 0;
    // uint8_t ip[16] = {0};
    // uint16_t port = 0;
    // uint8_t tcp_reply[] = "TCP service received.\n";

    // while (1){
    //     // 监听esp32 c3（串口3）读到的数据
    //     Esp32_Read_Data(rx_Buff, 1024, &real_receive_len, &id, ip, &port);
    //     if (real_receive_len > 0){
    //         printf("id:%d,ip:%s,port:%d,data:%s\n", id, ip, port, rx_Buff);
    //         // Esp32_Send_Data(rx_Buff, real_receive_len, id);
    //         Esp32_Send_Data(tcp_reply,(uint16_t)(sizeof(tcp_reply) - 1U), &id);
    //         real_receive_len = 0;
    //     }
    // }
}
