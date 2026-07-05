#include "usart3.h"

// 波特率计算，MHZ，波特率
static uint16_t calculate_bpr(uint32_t pclk2,uint32_t bound){
    float temp;
	uint16_t mantissa;                
	uint16_t fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分	 
    mantissa<<=4;
	mantissa+=fraction;
    return mantissa;
}

// 初始化
void usart3_init(void){
    // 必须设置时钟频率
    // 1. 配置时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    // 2.GPIO 工作模式
    // PB10: USART3_TX 复用推挽输出，CNF-10，MODE-11
    // PB11: USART3_RX 浮空输入，CNF-01，MODE-00
    GPIOB->CRH |= GPIO_CRH_MODE10;
    GPIOB->CRH |= GPIO_CRH_CNF10_1;
    GPIOB->CRH &= ~GPIO_CRH_CNF10_0;

    GPIOB->CRH &= ~GPIO_CRH_MODE11;
    GPIOB->CRH &= ~GPIO_CRH_CNF11_1;
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

// 发送一个字符
void usart3_send(uint8_t ch){
    // 等待发送寄存器为空 判断SR里面的TXE 是否为1
    while ((USART3->SR & USART_SR_TXE) == 0)
    {}
    // 向DR写入新的要发送的数据
    USART3->DR = ch;
}

// 接收一个字符
uint8_t usart3_receive(void){
    while ((USART3->SR & USART_SR_RXNE) == 0)
    {
        // 增加判断空闲帧的条件
        if (USART3->SR & USART_SR_IDLE)
        {
            return 0;
        }    
    }
    // 读取已经接收到的数据，等待接收下一个数据
    return USART3->DR;
}


// 发送字符串
void usart3_send_string(uint8_t *str,uint8_t size){
    for (uint8_t i = 0; i < size; i++)
    {
       usart3_send(str[i]);
    }
    
}

// 接收字符串
// void usart3_receive_string(uint8_t buffer[],uint8_t *size){
//     // 定义一个变量，用来保存已经接收到的字符个数
//     uint8_t i = 0;
//     while ((USART3->SR && USART_SR_IDLE) == 0)
//     {
//         buffer[i] = usart3_receive();
//         i++;
//     }
//     *size = i;
// }

void usart3_receive_string(uint8_t buffer[],uint8_t *size){
    // 定义一个变量，用来保存已经接收到的字符个数
    uint8_t i = 0;
    while ((USART3->SR & USART_SR_IDLE) == 0)
    {
        buffer[i] = usart3_receive();
        i++;
    }
    // 清除IDLE位
    // USART3->SR;
    USART3->DR;
    *size = --i;
}