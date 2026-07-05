#include "usart3.h"

#define USART3_TX_TIMEOUT          1000000U
#define USART3_RX_FIRST_TIMEOUT    3000000U
#define USART3_RX_NEXT_TIMEOUT      300000U

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

void usart3_init(void){
    // USART3 使用默认映射：PB10=TX，PB11=RX
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST;

    AFIO->MAPR &= ~AFIO_MAPR_USART3_REMAP;

    // PB10: USART3_TX 复用推挽输出，CNF-10，MODE-11
    GPIOB->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOB->CRH |= (GPIO_CRH_MODE10 | GPIO_CRH_CNF10_1);

    // PB11: USART3_RX 浮空输入，CNF-01，MODE-00
    GPIOB->CRH &= ~(GPIO_CRH_MODE11 | GPIO_CRH_CNF11);
    GPIOB->CRH |= GPIO_CRH_CNF11_0;

    USART3->CR1 &= ~USART_CR1_UE;
    USART3->BRR = calculate_bpr(36,115200);
    USART3->CR1 &= ~(USART_CR1_M | USART_CR1_PCE);
    USART3->CR2 &= ~USART_CR2_STOP;
    USART3->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
}

uint8_t usart3_send(uint8_t ch){
    uint32_t timeout = USART3_TX_TIMEOUT;

    while ((USART3->SR & USART_SR_TXE) == 0)
    {
        if (timeout-- == 0)
        {
            return 0;
        }
    }

    USART3->DR = ch;
    return 1;
}

uint8_t usart3_receive(uint8_t *ch, uint32_t timeout){
    while ((USART3->SR & USART_SR_RXNE) == 0)
    {
        if (timeout-- == 0)
        {
            return 0;
        }
    }

    *ch = USART3->DR;
    return 1;
}

uint8_t usart3_send_string(const uint8_t *str,uint16_t size){
    for (uint16_t i = 0; i < size; i++)
    {
        if (usart3_send(str[i]) == 0)
        {
            return 0;
        }
    }

    return 1;
}

uint16_t usart3_receive_string(uint8_t buffer[],uint16_t size){
    uint16_t i = 0;
    uint8_t ch = 0;
    uint32_t timeout = USART3_RX_FIRST_TIMEOUT;

    if (size == 0)
    {
        return 0;
    }

    while (i < (size - 1))
    {
        if (usart3_receive(&ch, timeout) == 0)
        {
            break;
        }

        buffer[i++] = ch;
        timeout = USART3_RX_NEXT_TIMEOUT;
    }

    buffer[i] = '\0';
    return i;
}

uint32_t usart3_debug_status(void){
    return USART3->SR;
}

uint8_t usart3_rx_pin_level(void){
    return (GPIOB->IDR & GPIO_IDR_IDR11) ? 1U : 0U;
}

uint8_t usart3_tx_pin_level(void){
    return (GPIOB->IDR & GPIO_IDR_IDR10) ? 1U : 0U;
}
