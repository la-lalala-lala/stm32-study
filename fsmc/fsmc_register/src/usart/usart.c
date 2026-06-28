#include "usart.h"

// 波特率计算，MHZ，波特率
uint16_t calculate_bpr(uint32_t pclk2,uint32_t bound){
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
void init_usart(void){
    // 必须设置时钟频率
    // uint32_t system_clock_freq = 72000000; // Set system clock frequency (72 MHz)
    // SystemClock_Config(system_clock_freq);
    // 1. 配置时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // 2.GPIO 工作模式
    // PA9: TX 复用推挽输出，CNF-10，MODE-11
    // PA10: RX 浮空输入，CNF-01，MODE-00
    GPIOA->CRH |= GPIO_CRH_MODE9;
    GPIOA->CRH |= GPIO_CRH_CNF9_1;
    GPIOA->CRH &= ~GPIO_CRH_CNF9_0;

    GPIOA->CRH &= ~GPIO_CRH_MODE10;
    GPIOA->CRH &= ~GPIO_CRH_CNF10_1;
    GPIOA->CRH |= GPIO_CRH_CNF10_0;

    // 3. 串口配置
    // 3.1 波特率设置
    USART1->BRR = calculate_bpr(72,115200);

    // 3.2 收发使能及模块使能
    USART1->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);

    // 3.3 配置一个字的长度 8位
    USART1->CR1 &= ~USART_CR1_M;
    // 3.4 配置不需要校验位
    USART1->CR1 &= ~USART_CR1_PCE;
    // 3.5 配置停止位的长度
    USART1->CR2 &= ~USART_CR2_STOP;
}

// 发送一个字符
void usart_send(uint8_t ch){
    // 等待发送寄存器为空 判断SR里面的TXE 是否为1
    while ((USART1->SR & USART_SR_TXE) == 0)
    {}
    // 向DR写入新的要发送的数据
    USART1->DR = ch;
}

// 接收一个字符
uint8_t usart_receive(void){
    while ((USART1->SR & USART_SR_RXNE) == 0)
    {
        // 增加判断空闲帧的条件
        if (USART1->SR & USART_SR_IDLE)
        {
            return 0;
        }    
    }
    // 读取已经接收到的数据，等待接收下一个数据
    return USART1->DR;
}


// 发送字符串
void usart_send_string(uint8_t *str,uint8_t size){
    for (uint8_t i = 0; i < size; i++)
    {
       usart_send(str[i]);
    }
    
}

// 接收字符串
// void usart_receive_string(uint8_t buffer[],uint8_t *size){
//     // 定义一个变量，用来保存已经接收到的字符个数
//     uint8_t i = 0;
//     while ((USART1->SR && USART_SR_IDLE) == 0)
//     {
//         buffer[i] = usart_receive();
//         i++;
//     }
//     *size = i;
// }

void usart_receive_string(uint8_t buffer[],uint8_t *size){
    // 定义一个变量，用来保存已经接收到的字符个数
    uint8_t i = 0;
    while ((USART1->SR & USART_SR_IDLE) == 0)
    {
        buffer[i] = usart_receive();
        i++;
    }
    // 清除IDLE位
    // USART1->SR;
    USART1->DR;
    *size = --i;
}

/* USER CODE BEGIN 1 */
int fputc(int ch, FILE *file){
  usart_send(ch);
  return ch;
}

//使用STM32开发，想用printf把输出打印到串口，需要重定向printf函数。
//网上一搜全都是重写fpuc的，但这只针对使用了MicroLIB的情况，如果你使用STM32CubeMX配置了CMake或者Makefile项目，这种方法是根本不可行的，重写fputc没有鸟用。
//这个时候需要重写_write函数，如下：

// 最终生效的方法，重写_write，重定向到日志输出串口
int _write(int file, char *ch, int len){
  for (int i = 0; i < len; i++) {
    usart_send((uint8_t)ch[i]);
  }
  return len;
}