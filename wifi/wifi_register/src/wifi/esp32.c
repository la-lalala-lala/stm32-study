#include "esp32.h"


static uint8_t esp32_read_buffer[1024] = {0};

#define ESP32_RESET_PIN              4U
#define ESP32_AT_RETRY_COUNT         10U
#define ESP32_AT_TIMEOUT_MS          2000U
#define ESP32_RX_POLL_TIMEOUT_MS     20U
#define ESP32_RX_INTER_BYTE_MS       10U

static void ESP32_HardwareReset(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    GPIOA->BSRR = (1U << ESP32_RESET_PIN);
    GPIOA->CRL &= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4);
    GPIOA->CRL |= GPIO_CRL_MODE4;

    GPIOA->BRR = (1U << ESP32_RESET_PIN);
    Delay_ms(100);
    GPIOA->BSRR = (1U << ESP32_RESET_PIN);
    Delay_ms(500);
}

/**
 * @description: 初始化ESP32
 */
uint8_t ESP32_Init(void){
    ESP32_HardwareReset();
    Driver_USART3_Init();

    for (uint8_t retry = 0; retry < ESP32_AT_RETRY_COUNT; retry++)
    {
        if (ESP32_Send_CMD("AT\r\n", RESPONSE_OK, ESP32_AT_TIMEOUT_MS) == ESP32_SUCCESS)
        {
            return ESP32_SUCCESS;
        }
    }

    return ESP32_ERROR;
}

/**
 * @description: 发送AT指令
 * @param cmd 要发送的AT指令(指令必须以 \r\n结束)
 * @param expect_result 期待的响应字符串
 * @param timeout_ms 等待响应的总超时时间
 */
uint8_t ESP32_Send_CMD(const char *cmd, const char *expect_result,
                       uint32_t timeout_ms){
    uint16_t total_length = 0;

    memset(esp32_read_buffer, 0, sizeof(esp32_read_buffer));
    Driver_USART3_FlushReceive();
    printf("发送指令：%s",cmd);
    Driver_USART3_SendString(cmd, (uint16_t)strlen(cmd));
    const uint32_t wait_start = Delay_GetCycleCount();

    while (!Delay_TimeoutElapsed(wait_start, timeout_ms))
    {
        const uint16_t remaining =
            (uint16_t)(sizeof(esp32_read_buffer) - 1U - total_length);
        if (remaining == 0U)
        {
            break;
        }

        const uint16_t received = Driver_USART3_ReceiveString(
            &esp32_read_buffer[total_length], remaining,
            ESP32_RX_POLL_TIMEOUT_MS, ESP32_RX_INTER_BYTE_MS);
        total_length += received;
        esp32_read_buffer[total_length] = '\0';

        if (strstr((char *)esp32_read_buffer, expect_result) != NULL)
        {
            printf("%s", esp32_read_buffer);
            printf("\r\n=====================\r\n");
            return ESP32_SUCCESS;
        }
    }

    printf("%s", esp32_read_buffer);
    printf("\r\n等待响应 %s 超时\r\n=====================\r\n", expect_result);
    return ESP32_ERROR;
}

uint16_t ESP32_ReadResponse(uint8_t response_buffer[], uint16_t size,
                            uint32_t timeout_ms){
    if (size == 0U)
    {
        return 0U;
    }

    memset(response_buffer, 0, size);
    const uint16_t received = Driver_USART3_ReceiveString(
        response_buffer, (uint16_t)(size - 1U), timeout_ms,
        ESP32_RX_INTER_BYTE_MS);
    response_buffer[received] = '\0';
    return received;
}


/**
 * @description: 接收TCP服务端的数据，串口3将数据从esp32 c3发送给stm32
 * @param rxbuff 缓冲区
 * @param max_size 缓冲区最大长度
 * @param real_receive_len 实际接收的长度
 * @param id 连接id
 * @param ip 客户端IP
 * @param port 客户端端口
 */
void Esp32_Read_Data(uint8_t rxbuff[],
                                uint16_t max_size,
                                uint16_t *real_receive_len,
                                uint8_t *id,
                                uint8_t ip[],
                                uint16_t *port){

    uint8_t temp_buff[128] = {0};
    *real_receive_len = 0U;
    const uint16_t received_size = Driver_USART3_ReceiveString(temp_buff,(uint16_t)(sizeof(temp_buff) - 1U),10000U,ESP32_RX_INTER_BYTE_MS);
    if (received_size == 0U){
        return;
    }
    temp_buff[received_size] = '\0';

    // 对数据进行处理
    // if ((strstr((char *)temp_buff, "+IPD")) == NULL){
    //     return;
    // }
    // if (id == NULL){
    //     // 单连接
    //     // +IPD,<数据长度>,"<远端IP>",<远端端口>:<实际数据>
    //     // +IPD,10,"192.168.124.12",9000:你是谁
    //     sscanf((char *)temp_buff, "%*[\r\n]+IPD,%d,\"%[^\"]\",%d:",
    //     (uint16_t *)real_receive_len,
    //     ip,
    //     (uint16_t *)port);
    //     strtok((char *)temp_buff, ":");
    //     memcpy(rxbuff, strtok(NULL, "\r\n"), *real_receive_len);
    // }else{
    //     // 判断是否为别人发送的信息
    //     // 多连接
    //     // +IPD,<连接ID>,<数据长度>,"<远端IP>",<远端端口>:<实际数据>
    //     // +IPD,0,9,"192.168.124.12",9000:hello1234
    //     sscanf((char *)temp_buff, "%*[\r\n]+IPD,%d,%d,\"%[^\"]\",%d:",
    //             (uint8_t *)id,
    //             (uint16_t *)real_receive_len,
    //             ip,
    //             (uint16_t *)port);
    //     strtok((char *)temp_buff, ":");
    //     memcpy(rxbuff, strtok(NULL, "\r\n"), *real_receive_len);
    // }
    char *ipd = strstr((char *)temp_buff, "+IPD");
    char *data = strchr((char *)temp_buff, ':');

    if (ipd == NULL || data == NULL) {
        return;
    }

    data++;

    if (id == NULL) {
        if (sscanf(ipd, "+IPD,%hu,\"%15[^\"]\",%hu:",
                real_receive_len, ip, port) != 3) {
            return;
        }
    } else {
        if (sscanf(ipd, "+IPD,%hhu,%hu,\"%15[^\"]\",%hu:",
                id, real_receive_len, ip, port) != 4) {
            return;
        }
    }

    if (*real_receive_len > max_size) {
        *real_receive_len = max_size;
    }

    memcpy(rxbuff, data, *real_receive_len);
}


/**
 * @description: 发送TCP服务端的数据，stm32将数据从串口3发送给esp32 c3
 * @param txbuff 缓冲区
 * @param tx_len 缓冲区长度
 * @param id TCP连接id
 */
void Esp32_Send_Data(uint8_t txbuff[],
                                uint16_t tx_len,
                                uint8_t *id){
    // 规定 不能使用串口2直接发送数据 => 需要通过AT指令发送数据
    // 先发送一个AT指令  => 告诉ESP32  我要发送数据了 => 再发送数据
    char temp_cmd[100];
    // 1. 发送send指令
    if(id == NULL){
        // 单连接
        sprintf((char *)temp_cmd, "AT+CIPSEND=%d\r\n", tx_len);
        ESP32_Send_CMD(temp_cmd, RESPONSE_OK, 1000);
    }else{
        // 多连接
        sprintf((char *)temp_cmd, "AT+CIPSEND=%d,%d\r\n", *id, tx_len);
        ESP32_Send_CMD(temp_cmd, RESPONSE_OK, 1000);
    }
    // 2. 通过串口发送数据
    Driver_USART3_SendString(txbuff, tx_len);
}



