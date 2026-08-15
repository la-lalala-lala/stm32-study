#include "usart3.h"

/*
 * USART3 挂载在 APB1。SystemCoreClock 表示 HCLK，因此还需要解析 PPRE1
 * 才能得到实际外设时钟，不能固定假设为 36 MHz。系统回退到 HSI 时，
 * 该计算也能让波特率跟随实际时钟调整。
 */
static uint32_t USART3_GetPeripheralClock(void)
{
    SystemCoreClockUpdate();
    const uint32_t ppre1 = (RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;
    // PPRE1 编码：0xx=/1，100=/2，101=/4，110=/8，111=/16。
    const uint32_t divider = (ppre1 < 4U) ? 1U : (1U << (ppre1 - 3U));
    return SystemCoreClock / divider;
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
    const uint32_t pclk1 = USART3_GetPeripheralClock();
    // 16 倍过采样时 BRR 编码等价于 PCLK1/baud；加 baud/2 完成整数四舍五入。
    USART3->BRR = (pclk1 + (115200U / 2U)) / 115200U;
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
    // TXE=1 仅表示发送数据寄存器为空、可以写入下一字节，不表示线路发送已完成；
    // 如需确认最后一个停止位已经发出，应另外等待 TC=1。
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
 * @brief 发送指定长度的数据
 * @param str 数据地址；由 len 指定长度，因此不要求以 '\0' 结尾
 * @param len 发送字节数
 */
void Driver_USART3_SendString(const char *str, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        Driver_USART3_SendChar((uint8_t)str[i]);
    }
}

void Driver_USART3_FlushReceive(void)
{
    volatile uint32_t discard;

    /*
     * STM32F1 通过“先读 SR、再读 DR”清除 IDLE 和接收错误标志。
     * 循环同时排空当前 RXNE 数据；本函数会主动丢弃尚未处理的接收字节。
     */
    do
    {
        discard = USART3->SR;
        discard = USART3->DR;
    } while ((USART3->SR & USART_SR_RXNE) != 0U);

    (void)discard;
}

uint16_t Driver_USART3_ReceiveString(uint8_t buff[], uint16_t capacity,
                                    uint32_t first_byte_timeout_ms,
                                    uint32_t inter_byte_timeout_ms)
{
    uint16_t length = 0;
    // 首字节超时从函数进入时开始；字节间超时在每次读取 DR 后重新计时。
    uint32_t wait_start = Delay_GetCycleCount();
    uint32_t last_byte = wait_start;

    while (length < capacity)
    {
        // 先保存 SR，使错误状态与随后从 DR 读出的字节保持对应。
        const uint32_t status = USART3->SR;

        if ((status & USART_SR_RXNE) != 0U)
        {
            const uint8_t byte = (uint8_t)USART3->DR;
            if ((status & (USART_SR_FE | USART_SR_NE | USART_SR_PE)) == 0U)
            {
                buff[length++] = byte;
            }
            last_byte = Delay_GetCycleCount();
            continue;
        }

        /*
         * 未启用 IDLE 中断时不必专门清除 IDLE。若在此额外执行一次 SR->DR
         * 清除序列，可能误读并丢弃恰好在两次读取之间到达的新字节。
         */
        if ((status & (USART_SR_ORE | USART_SR_NE |
                       USART_SR_FE | USART_SR_PE)) != 0U)
        {
            volatile uint32_t discard = USART3->DR;
            (void)discard;
        }

        // 尚未保存有效字节时，只应用首字节等待超时。
        if ((length == 0U) && Delay_TimeoutElapsed(wait_start, first_byte_timeout_ms))
        {
            break;
        }
        // 已有数据后，以连续静默时间判断本批不定长数据结束。
        if ((length > 0U) && Delay_TimeoutElapsed(last_byte, inter_byte_timeout_ms))
        {
            break;
        }
    }

    return length;
}
