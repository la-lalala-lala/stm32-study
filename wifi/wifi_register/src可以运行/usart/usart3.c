#include "usart3.h"

#define USART3_ATK_DIRECT_MODE     0
#define USART3_BAUD                115200U
#define USART3_RX_FIRST_TIMEOUT    3000000U
#define USART3_RX_NEXT_TIMEOUT      300000U

#ifndef SYSCLK_FREQ
#define SYSCLK_FREQ                72000000U
#endif

#if USART3_ATK_DIRECT_MODE

#define SOFT_UART_TX_PIN           11U
#define SOFT_UART_RX_PIN           10U
#define SOFT_UART_TX_MASK          (1U << SOFT_UART_TX_PIN)
#define SOFT_UART_RX_MASK          (1U << SOFT_UART_RX_PIN)
#define SOFT_UART_BIT_CYCLES       ((SYSCLK_FREQ + (USART3_BAUD / 2U)) / USART3_BAUD)
#define SOFT_UART_HALF_BIT_CYCLES  (SOFT_UART_BIT_CYCLES / 2U)

static void soft_uart_delay_cycles(uint32_t cycles){
    uint32_t start = DWT->CYCCNT;

    while ((uint32_t)(DWT->CYCCNT - start) < cycles)
    {
    }
}

static void soft_uart_dwt_init(void){
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint8_t soft_uart_rx_is_high(void){
    return (GPIOB->IDR & SOFT_UART_RX_MASK) ? 1U : 0U;
}

static void soft_uart_tx_high(void){
    GPIOB->BSRR = SOFT_UART_TX_MASK;
}

static void soft_uart_tx_low(void){
    GPIOB->BRR = SOFT_UART_TX_MASK;
}

void usart3_init(void){
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // PB10: 软件 RX，上拉输入；PB11: 软件 TX，推挽输出
    GPIOB->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOB->CRH |= GPIO_CRH_CNF10_1;
    GPIOB->ODR |= SOFT_UART_RX_MASK;

    GPIOB->CRH &= ~(GPIO_CRH_MODE11 | GPIO_CRH_CNF11);
    GPIOB->CRH |= GPIO_CRH_MODE11;
    soft_uart_tx_high();

    soft_uart_dwt_init();
}

uint8_t usart3_send(uint8_t ch){
    uint32_t primask = __get_PRIMASK();

    __disable_irq();

    soft_uart_tx_low();
    soft_uart_delay_cycles(SOFT_UART_BIT_CYCLES);

    for (uint8_t i = 0; i < 8; i++)
    {
        if (ch & (1U << i))
        {
            soft_uart_tx_high();
        }
        else
        {
            soft_uart_tx_low();
        }

        soft_uart_delay_cycles(SOFT_UART_BIT_CYCLES);
    }

    soft_uart_tx_high();
    soft_uart_delay_cycles(SOFT_UART_BIT_CYCLES);

    if (primask == 0)
    {
        __enable_irq();
    }

    return 1;
}

uint8_t usart3_receive(uint8_t *ch, uint32_t timeout){
    uint8_t data = 0;
    uint32_t primask;

    while (soft_uart_rx_is_high())
    {
        if (timeout-- == 0)
        {
            return 0;
        }
    }

    primask = __get_PRIMASK();
    __disable_irq();

    soft_uart_delay_cycles(SOFT_UART_BIT_CYCLES + SOFT_UART_HALF_BIT_CYCLES);

    for (uint8_t i = 0; i < 8; i++)
    {
        if (soft_uart_rx_is_high())
        {
            data |= (1U << i);
        }

        soft_uart_delay_cycles(SOFT_UART_BIT_CYCLES);
    }

    if (primask == 0)
    {
        __enable_irq();
    }

    *ch = data;
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

void usart3_flush_rx(void){
}

uint32_t usart3_debug_status(void){
    return 0;
}

uint8_t usart3_rx_pin_level(void){
    return soft_uart_rx_is_high();
}

uint8_t usart3_tx_pin_level(void){
    return (GPIOB->IDR & SOFT_UART_TX_MASK) ? 1U : 0U;
}

const char *usart3_debug_mode(void){
    return "soft:PB11-TX/PB10-RX";
}

#else

#define USART3_TX_TIMEOUT          1000000U

// 波特率计算，MHZ，波特率
static uint16_t calculate_bpr(uint32_t pclk2,uint32_t bound){
    float temp;
    uint16_t mantissa;
    uint16_t fraction;
    temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
    mantissa=temp;               //得到整数部分
    fraction=(temp-mantissa)*16; //得到小数部分
    mantissa<<=4;
    mantissa+=fraction;
    return mantissa;
}

void usart3_init(void){
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    AFIO->MAPR &= ~AFIO_MAPR_USART3_REMAP;

    GPIOB->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOB->CRH |= (GPIO_CRH_MODE10 | GPIO_CRH_CNF10_1);

    GPIOB->CRH &= ~(GPIO_CRH_MODE11 | GPIO_CRH_CNF11);
    GPIOB->CRH |= GPIO_CRH_CNF11_0;

    USART3->CR1 &= ~USART_CR1_UE;
    USART3->BRR = calculate_bpr(36,115200);
    USART3->CR1 &= ~USART_CR1_M;
    USART3->CR1 &= ~USART_CR1_PCE;
    USART3->CR2 &= ~USART_CR2_STOP;
    USART3->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);

    usart3_flush_rx();
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
        if (USART3->SR & (USART_SR_ORE | USART_SR_FE | USART_SR_NE))
        {
            usart3_flush_rx();
        }

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

void usart3_flush_rx(void){
    volatile uint32_t tmp;

    tmp = USART3->SR;
    tmp = USART3->DR;
    (void)tmp;
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

const char *usart3_debug_mode(void){
    return "hw:PB10-TX/PB11-RX";
}

#endif
