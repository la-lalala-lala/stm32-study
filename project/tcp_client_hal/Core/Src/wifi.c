#include "wifi.h"

/**
 * @description: 初始化WIFI
 * @return {*}
 */
void WIFI_Init(void){
    /* 1. 初始化 ESP32 芯片 */
    ESP32_Init();

    // WIFI_AP_MODE();
    WIFI_STA_MODE();
}

/**
 * @description: wifi设置为STAT模式
 */
void WIFI_STA_MODE(void){
    /* 1. 设置为基站模式 */
    printf("设置基站模式\r\n");
    uint8_t *cmd = "AT+CWMODE=1\r\n";
    ESP32_Send_CMD(cmd, strlen((char *)cmd),3000);

    /* 2. 指定连接到哪个AP(wifi路由器) */
    printf("设置要连接到的AP\r\n");
    cmd = "AT+CWJAP=\"uino_office\",\"uinnova123\"\r\n";
    ESP32_Send_CMD(cmd, strlen((char *)cmd),5000);

    /* 3. 设置局域网IP子网 */
    printf("设置局域网IP子网\r\n");
    cmd = "AT+CIPSTA=\"192.168.1.100\",\"192.168.1.1\",\"255.255.255.0\"\r\n";
    ESP32_Send_CMD(cmd, strlen((char *)cmd),5000);
}

/**
 * @description: wifi 设置为AP模式
 * @return {*}
 */
void WIFI_AP_MODE(void){
    /* 2. 设置 WIFI 为AP模式 */
    /* 2.1 设置为 WIFI 模式
        0: 无 Wi-Fi 模式，并且关闭 Wi-Fi RF
        1: Station 模式
        2: SoftAP 模式
        3: SoftAP+Station 模式
    */
    printf("设置AP模式\r\n");
    uint8_t *cmd = "AT+CWMODE=2\r\n";
    ESP32_Send_CMD(cmd, strlen((char *)cmd),2000);

    /* 2.2 设置AP的
        - SSID,
        - 密码
        - 信道
        - 加密方式 0: OPEN 2: WPA_PSK 3: WPA2_PSK 4: WPA_WPA2_PSK
        - 广播ssid 0: 广播 SSID（默认）1: 不广播 SSID  注意:如果设置默认值，就不要添加这个参数
    */
    printf("设置AP的参数\r\n");
    cmd = "AT+CWSAP=\"atguigu-esp\",\"12345678\",5,3\r\n";
    ESP32_Send_CMD(cmd, strlen((char *)cmd),2000);

    /* 2.3 设置AP的ip地址（不确定的情况下，先不设置） */
    //printf("设置AP的IP地址\r\n");
    //cmd = "AT+CIPAP=\"192.168.1.1\",\"192.168.1.1\",\"255.255.255.0\"\r\n";
    //ESP32_Send_CMD(cmd, strlen((char *)cmd),4000);
}

/**
 * @description: 开启一个udp客户端。用来收发udp数据
 * @return {*}
 */
void WIFI_StartUdpClient(void){
    /* 1. 设置单连接模式。 只有单连接模式才能开启UDP客户端 */
    printf("设置单连接模式,用于开启UDP客户端\r\n");
    uint8_t *cmd = "AT+CIPMUX=0\r\n";
    ESP32_Send_CMD(cmd, strlen((char *)cmd),4000);

    /*
        <mode>：在 UDP Wi-Fi 透传下，本参数的值必须设为 0
        – 0: 接收到 UDP 数据后，不改变对端 UDP 地址信息（默认）
        – 1: 仅第一次接收到与初始设置不同的对端 UDP 数据时，改变对端 UDP 地址信息为发送数据设
        备的 IP 地址和端口
        – 2: 每次接收到 UDP 数据时，都改变对端 UDP 地址信息为发送数据的设备的 IP 地址和端口
    */
    printf("建立服务器\r\n");
    cmd = "AT+CIPSTART=\"UDP\",\"10.203.1.167\",9000,5000,0\r\n";
    ESP32_Send_CMD(cmd, strlen((char *)cmd),5000);

    /*
        3. 设置 IPD(Incoming Packet Data)数据格式
            0: 在 “+IPD” 和 “+CIPRECVDATA” 消息中，不提示对端 IP 地址和端口信息
            1: 在 “+IPD” 和 “+CIPRECVDATA” 消息中，提示对端 IP 地址和端口信息
        +IPD,0,4,"192.168.8.3",56241:abcd
    */
    printf("设置IPD数据格式\r\n");
    cmd = "AT+CIPDINFO=1\r\n";
    ESP32_Send_CMD(cmd, strlen((char *)cmd),3000);
}

/**
 * @description: 使用阻塞式方式从串口读取UDP数据。 利用到了空闲帧
 * @param {uint8_t} rxBuff 存储读取到的实际数据。
 * @param {uint16_t} maxSize 最多读取多少
 * @param {uint8_t} ip 对方ip地址
 * @param {uint16_t} port 对方端口
 * @param {uint16_t} *realReceveSize 接收的数据量
 * @return {*}
 */

void WIFI_ReadUdpData(uint8_t rxBuff[], uint16_t maxSize, uint8_t *ip,uint16_t *port, uint16_t *realReceveSize){
    uint8_t tempBuff[128] = {0};
    *realReceveSize = 0U;

    uint16_t receivedSize = 0U;
    uint16_t payloadSize = 0U;

    HAL_UARTEx_ReceiveToIdle(&huart3, tempBuff, sizeof(tempBuff) - 1U,&receivedSize, 10000);
    if (receivedSize == 0U){
        return;
    }
    tempBuff[receivedSize] = '\0';
    char *ipd = strstr((char *)tempBuff, "+IPD");
    char *data = strchr(ipd, ':');

    if (ipd == NULL || data == NULL) {
        return;
    }

    data++;
    if (sscanf(ipd, "+IPD,%hu,\"%15[^\"]\",%hu:", &payloadSize, ip, port) != 3) {
        return;
    }
    if (*realReceveSize > maxSize) {
        *realReceveSize = maxSize;
    }

    memcpy(rxBuff, data, payloadSize);
    *realReceveSize = payloadSize;
}

/**
 * @description: 通过TCP协议给对方发数据
 * @param {uint8_t} *msg 具体信息
 * @param {uint16_t} size 信息长度
 * @return {*}
 */

void WIFI_SendUdpData(uint8_t *msg,uint16_t size){
    // 先发送一个AT指令  => 告诉ESP32  我要发送数据了 => 再发送数据
    uint8_t sendCmd[100];
    // 发送send指令 单连接
    /* 1. 发送AT指令 AT+CIPSEND=<link ID>,<length>*/
    sprintf((char *)sendCmd, "AT+CIPSEND=%d\r\n", size);
    ESP32_Send_CMD(sendCmd, strlen((char *)sendCmd),2000);
    /* 2. 把要发送的数据通过串口发出 */
    HAL_UART_Transmit(&huart3, msg, size, 1000);
}
