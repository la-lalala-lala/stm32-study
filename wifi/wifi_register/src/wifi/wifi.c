#include "wifi.h"



/**
 * @description: 开发板连接路由器的热点（Station模式）
 * @return 是否连接成功
 */
uint8_t ESP32_Start_Station(void){
    // 1.设置wifi模式为station模式
    uint8_t *cmd = "AT+CWMODE=1\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    // 2. 连接对应的wifi
    cmd = "AT+CWJAP=\"lalala\",\"lalala.666\"\r\n";
    if (ESP32_Send_CMD(cmd, RESPONSE_OK, 5000) == ESP32_ERROR){
        return ESP32_ERROR;
    }

    // 3. 设置局域网IP子网  =>  192.168.124.67
    cmd = "AT+CIPSTA=\"192.168.124.67\",\"192.168.124.1\",\"255.255.255.0\"\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);
    return ESP32_SUCCESS;
}

/**
 * @description: 开发板作为热点（AP模式），电脑连接开发板的热点（开发板开局域网），只能作为内网
 */
void ESP32_Start_AP(void){
    // 1. 设置WIFI模式为ap模式
    uint8_t *cmd = "AT+CWMODE=2\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    // 2. 打开WIFI热点
    cmd = "AT+CWSAP=\"esp32_c3_mb026\",\"esp32esp32\",5,3\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    // 3. 设置局域网IP子网  =>  192.168.88.1
    cmd = "AT+CIPAP=\"192.168.88.1\",\"192.168.88.1\",\"255.255.255.0\"\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);
}         

/**
 * @description: 开发板开启TCP服务
 */
void Esp32_Start_TCP_Server(void){
    // 1. 打开多连接模式
    uint8_t *cmd = "AT+CIPMUX=1\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    // 2. 创建TCP服务器
    cmd = "AT+CIPSERVER=1,8080\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    // 3. 添加IPDINFO信息  显示对方的IP地址加端口号
    cmd = "AT+CIPDINFO=1\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);
}

/**
 * @description: 开发板连接TCP服务
 * @return 是否连接成功
 */
uint8_t Esp32_Start_TCP_Client(void){
    // 1. 打开单连接模式
    uint8_t *cmd = "AT+CIPMUX=0\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    // 2. 连接到TCP服务端
    cmd = "AT+CIPSTART=\"TCP\",\"192.168.124.12\",9000\r\n";
    if (ESP32_Send_CMD(cmd, RESPONSE_OK, 5000) == ESP32_ERROR){
        return ESP32_ERROR;
    }
    // 0 是普通传输模式，1 是透传模式；透传只适合流式数据，普通模式更容易在 STM32 上做状态机和收包解析。

    // 3. 添加IPDINFO信息  显示对方的IP地址加端口号
    cmd = "AT+CIPDINFO=1\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);
    return ESP32_SUCCESS;
}

/**
 * @description: 开发板连接UDP服务
 * @return 是否连接成功
 */
uint8_t Esp32_Start_UDP_Client(void){
// 1. 打开单连接模式
    uint8_t *cmd = "AT+CIPMUX=0\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    // 2. 连接到UDP服务端
    cmd = "AT+CIPSTART=\"UDP\",\"192.168.124.12\",9000,5000,0\r\n";
    if (ESP32_Send_CMD(cmd, RESPONSE_OK, 5000) == ESP32_ERROR){
        return ESP32_ERROR;
    }
    // 0 是普通传输模式，1 是透传模式；透传只适合流式数据，普通模式更容易在 STM32 上做状态机和收包解析。

    // 3. 添加IPDINFO信息  显示对方的IP地址加端口号
    cmd = "AT+CIPDINFO=1\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);
    return ESP32_SUCCESS;
}
