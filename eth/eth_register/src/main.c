#include "usart/usart1.h"
#include <stdio.h>
#include <stm32f1xx.h>
#include "system/system.h"
#include "eth/ch395.h"
#include "eth/socket.h"
#include "delay/delay.h"


int main(void){
    /* 开发板静态网络参数。 */
    uint8_t mac[6] = {0x02U, 0x12U, 0x34U, 0x56U, 0x78U, 0x9AU};
    uint8_t ip[4] = {192U, 168U, 124U, 13U};
    uint8_t gateway[4] = {192U, 168U, 124U, 1U};
    uint8_t netmask[4] = {255U, 255U, 255U, 0U};

    SystemClock_Config();
    Driver_USART1_Init();
    printf("test usart print\r\n");

    if (ch395StaticInit(ip, gateway, netmask, mac) != 0){
        while (1)
        {
            /* CH395 初始化失败，停止在此处。 */
        }
    }
    // UDP Client 实验
    /* nc -u -l -v 9000 所在 PC 的地址和端口。 */
    // uint8_t remote_ip[4] = {192U, 168U, 124U, 12U};
    // uint16_t remote_port = 9000U;
    // uint16_t local_port = 9000U;
    // uint8_t socket_interrupt_status = 0U;
    // uint8_t receive_buffer[1024];
    // int receive_length;
    // if (socket_udp_start(0U, remote_ip, remote_port, local_port,&socket_interrupt_status) != 0){
    //     printf("UDP client initialization failed\r\n");
    //     while (1){
    //         /* UDP Client 初始化失败，停止在此处。 */
    //     }
    // }
    // printf("UDP client initialization success\r\n");

    // /* 启动后只发送一次，后续不再周期发送。 */
    // const uint8_t startup_message[] = "ping\r\n";

    // if (socket_udp_send(0U, startup_message,(uint16_t)(sizeof(startup_message) - 1U),&socket_interrupt_status) != 0){
    //     printf("UDP startup send failed\r\n");
    // }

    // while (1){
    //     /* 收到 nc 发来的原文后打印，并把同一段数据原文发回。 */
    //     receive_length = socket_udp_receive(0U, receive_buffer, (uint16_t)sizeof(receive_buffer),&socket_interrupt_status);
    //     if (receive_length > 0){
    //         printf("UDP receive: %.*s\r\n",receive_length, (char *)receive_buffer);
    //         if (socket_udp_send(0U, receive_buffer,(uint16_t)receive_length,&socket_interrupt_status) != 0){
    //             printf("UDP send failed\r\n");
    //         }
    //     }
    //     Delay_ms(1);
    // }

    // TCP Client 实验
    /* nc -4 -l -v 9000 所在 PC 的地址和端口。 */
    uint8_t tcp_remote_ip[4] = {192U, 168U, 124U, 12U};
    uint16_t tcp_remote_port = 9000U;
    uint16_t tcp_local_port = 9000U;
    uint8_t tcp_socket_interrupt_status = 0U;
    uint8_t tcp_receive_buffer[1024];
    int tcp_receive_length;

    if (socket_tcp_client_start(0U,tcp_remote_ip,tcp_remote_port,tcp_local_port,&tcp_socket_interrupt_status) != 0){
        printf("TCP client initialization failed\r\n");
        while (1){
            /* TCP Client 初始化失败，停止在此处。 */
        }
    }

    printf("TCP client initialization success\r\n");

    while (1){
        /* 循环监听 TCP Server；收到数据后原文打印并回发。 */
        tcp_receive_length = socket_tcp_client_receive(0U,tcp_receive_buffer,(uint16_t)sizeof(tcp_receive_buffer),&tcp_socket_interrupt_status);

        if (tcp_receive_length > 0){
            printf("TCP receive: %.*s\r\n",tcp_receive_length, (char *)tcp_receive_buffer);

            if (socket_tcp_client_send(0U,tcp_receive_buffer,(uint16_t)tcp_receive_length,&tcp_socket_interrupt_status) != 0){
                printf("TCP send failed\r\n");
            }
        }
        else if (tcp_receive_length < 0){
            printf("TCP receive error: %d\r\n", tcp_receive_length);
        }

        Delay_ms(1);
    }
}
