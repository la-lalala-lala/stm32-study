#include "ble.h"

void ESP32_BLE_Init(void){

    /* 1. 初始化ESP32 */
    ESP32_Init();

    /* 2. 初始化BLE角色 */
    uint8_t *cmd = "AT+BLEINIT=2\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    /* 3. 创建BLE服务 */
    cmd = "AT+BLEGATTSSRVCRE\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    /* 4. 开启BLE服务 */
    cmd = "AT+BLEGATTSSRVSTART\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    /* 5. 设置广播参数 */
    cmd = "AT+BLEADVPARAM=50,50,0,0,7,0,,\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    /* 6. 自动设置广播数据 */
    cmd = "AT+BLEADVDATAEX=\"esp32-c3-ble\",\"A002\",\"0102030405\",1\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    /* 7. 开启广播 */
    cmd = "AT+BLEADVSTART\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);


    /* 8. 配置SPP */
    cmd = "AT+BLESPPCFG=1,1,7,1,5\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);

    //  不能直接在初始化BLE的时候使能SPP 
    /* 9. 配置打印蓝牙连接信息 */
    cmd = "AT+SYSMSG=5\r\n";
    ESP32_Send_CMD(cmd, RESPONSE_OK, 1000);
}