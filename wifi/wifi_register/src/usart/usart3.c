#include "usart3.h"

// 波特率计算，MHz，波特率
static uint16_t calculate_bpr(uint32_t pclk,uint32_t bound){
    float temp;
    uint16_t mantissa;
    uint16_t fraction;

    temp=(float)(pclk*1000000)/(bound*16);//得到USARTDIV
    mantissa=temp;               //得到整数部分
    fraction=(temp-mantissa)*16; //得到小数部分
    mantissa<<=4;
    mantissa+=fraction;

    return mantissa;
}

void Driver_USART3_Init(void){
    // 1. 配置时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    // 复位
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST;

    // 清除重映射
    AFIO->MAPR &= ~AFIO_MAPR_USART3_REMAP;

    // 2.GPIO 工作模式
    // PB10: USART3_TX 复用推挽输出，CNF-10，MODE-11
    GPIOB->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOB->CRH |= (GPIO_CRH_MODE10 | GPIO_CRH_CNF10_1);

    // PB11: USART3_RX 浮空输入，CNF-01，MODE-00
    GPIOB->CRH &= ~(GPIO_CRH_MODE11 | GPIO_CRH_CNF11);
    GPIOB->CRH |= GPIO_CRH_CNF11_0;

    // 3. 串口配置
    // 3.1 波特率设置
    USART3->BRR = calculate_bpr(36,115200);
    // 3.2 收发使能及模块使能
    USART3->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);

    // 3.3 配置一个字的长度 8位
    USART3->CR1 &= ~USART_CR1_M;
    // 3.4 配置不需要校验位
    USART3->CR1 &= ~USART_CR1_PCE;
    // 3.5 配置停止位的长度
    USART3->CR2 &= ~USART_CR2_STOP;
    
}

/**
 * 发送一个字节
 * uint8_t ch: 需要发送的一个字节
 */
void Driver_USART3_SendChar(uint8_t ch)
{
    // 往DR寄存器直接写 -> 发送数据
    // 需要等待上一个字节发送完成之后  ->  才能发送下一个字节
    // SR_TXE   如果数据正在发 ->  0
    //          如果数据发生完成 -> 1
    while ((USART3->SR & USART_SR_TXE) == 0)
        ;

    USART3->DR = ch;
}

/**
 * 接收一个字节
 * return: uint8_t 接收到的字节
 */
uint8_t Driver_USART3_ReceiveChar(void)
{
    // 接收数据也需要挂起 -> 等待有数据传入
    // SR_RXNE : 有数据进来 -> 1
    //           读取一次之后 -> 0
    while ((USART3->SR & USART_SR_RXNE) == 0)
        ;
    return USART3->DR;
}

/**
 * 发送多个字节
 * uint8_t *str: 一个字符串
 * uint8_t len: 字符串长度
 */
void Driver_USART3_SendString(uint8_t *str, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        Driver_USART3_SendChar(str[i]);
    }
}

/**
 * 接收多个字节  ->  由于不能确定接收数据的长度 ->  推荐buff够大
 * 收到空闲位结束
 * uint8_t buff[]: 存接收到的数据
 * uint8_t * len: 实际接收数据的长度 -> 需要函数中赋值
 */
void Driver_USART3_ReceiveString(uint8_t buff[], uint8_t *len)
{
    uint8_t i = 0;
    while (1)
    {
        // 1. 收到数据  存缓存
        if (USART3->SR & USART_SR_RXNE)
        {
            buff[i] = USART3->DR;
            i++;
        }

        // 2. 收到空闲 停止
        if (USART3->SR & USART_SR_IDLE)
        {

            *len = i;
            break;
        }
        // 3. 挂起等待
        // while ((USART3->SR & USART_SR_RXNE) == 0)
        //     ;
    }
}