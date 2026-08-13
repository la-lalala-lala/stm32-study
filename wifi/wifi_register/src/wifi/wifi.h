#ifndef __DRIVER_WIFI_H
#define __DRIVER_WIFI_H
#include "esp32.h"

/**
 * @description: 开发板连接路由器的热点（Station模式）
 * @return 是否连接成功
 */
uint8_t ESP32_Start_Station(void);

/**
 * @description: 开发板作为热点（AP模式），电脑连接开发板的热点（开发板开局域网），只能作为内网
 */
void ESP32_Start_AP(void);            

/**
 * @description: 开发板开启TCP服务
 */
void Esp32_Start_TCP_Server(void);

/**
 * @description: 开发板连接TCP服务
 * @return 是否连接成功
 */
uint8_t Esp32_Start_TCP_Client(void);


/**
 * @description: 开发板连接UDP服务
 * @return 是否连接成功
 */
uint8_t Esp32_Start_UDP_Client(void);

#endif