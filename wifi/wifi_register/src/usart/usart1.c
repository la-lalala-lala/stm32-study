#include "usart1.h"

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
void Driver_USART1_Init(void){
    // 必须设置时钟频率
    // uint32_t system_clock_freq = 72000000; // Set system clock frequency (72 MHz)
    // SystemClock_Config(system_clock_freq);
    // 1. 配置时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // 复位
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
    // 清除重映射
    AFIO->MAPR &= ~AFIO_MAPR_USART1_REMAP;  // 默认 PA9=TX, PA10=RX

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

/**
 * 发送一个字节
 * uint8_t ch: 需要发送的一个字节
 */
void Driver_USART1_SendChar(uint8_t ch)
{
    // 往DR寄存器直接写 -> 发送数据
    // 需要等待上一个字节发送完成之后  ->  才能发送下一个字节
    // SR_TXE   如果数据正在发 ->  0
    //          如果数据发生完成 -> 1
    while ((USART1->SR & USART_SR_TXE) == 0)
        ;

    USART1->DR = ch;
}

/**
 * 接收一个字节
 * return: uint8_t 接收到的字节
 */
uint8_t Driver_USART1_ReceiveChar(void)
{
    // 接收数据也需要挂起 -> 等待有数据传入
    // SR_RXNE : 有数据进来 -> 1
    //           读取一次之后 -> 0
    while ((USART1->SR & USART_SR_RXNE) == 0)
        ;
    return USART1->DR;
}

/**
 * 发送多个字节
 * uint8_t *str: 一个字符串
 * uint8_t len: 字符串长度
 */
void Driver_USART1_SendString(uint8_t *str, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        Driver_USART1_SendChar(str[i]);
    }
}

/**
 * 接收多个字节  ->  由于不能确定接收数据的长度 ->  推荐buff够大
 * 收到空闲位结束
 * uint8_t buff[]: 存接收到的数据
 * uint8_t * len: 实际接收数据的长度 -> 需要函数中赋值
 */
void Driver_USART1_ReceiveString(uint8_t buff[], uint8_t *len)
{
    uint8_t i = 0;
    while (1)
    {
        // 1. 收到数据  存缓存
        if (USART1->SR & USART_SR_RXNE)
        {
            buff[i] = USART1->DR;
            i++;
        }

        // 2. 收到空闲 停止
        if (USART1->SR & USART_SR_IDLE)
        {

            *len = i;
            break;
        }
        // 3. 挂起等待
        // while ((USART1->SR & USART_SR_RXNE) == 0)
        //     ;
    }
}

// void Driver_USART1_ReceiveString(uint8_t buff[], uint8_t *len)
// {
//     uint8_t i = 0;
//     while (1)
//     {
//         // 3. 需要挂起
//         while ((USART1->SR & USART_SR_RXNE) == 0)
//         {
//             // 2. 接收到空闲位 -> break
//             if (USART1->SR & USART_SR_IDLE)
//             {
//                 // 接收完成
//                 *len = i;
//                 return;
//             }
//         }
//         // 1. 接收到数据 -> 存buff len+1
//         buff[i] = USART1->DR;
//         i++;
//     }
// }



int fputc(int ch,FILE *file)
{
    Driver_USART1_SendChar((uint8_t )ch);
    return ch;
}

//使用STM32开发，想用printf把输出打印到串口，需要重定向printf函数。
//网上一搜全都是重写fpuc的，但这只针对使用了MicroLIB的情况，如果你使用STM32CubeMX配置了CMake或者Makefile项目，这种方法是根本不可行的，重写fputc没有鸟用。
//这个时候需要重写_write函数，如下：

// 最终生效的方法，重写_write，重定向到日志输出串口
int _write(int file, char *ch, int len){
  for (int i = 0; i < len; i++) {
    Driver_USART1_SendChar((uint8_t)ch[i]);
  }
  return len;
}
