#include "usart1.h"

#define USART1_LEGACY_RX_CAPACITY             255U
#define USART1_LEGACY_FIRST_BYTE_TIMEOUT_MS   1000U
#define USART1_LEGACY_INTER_BYTE_TIMEOUT_MS   10U

/*
 * USART1 挂载在 APB2。SystemCoreClock 表示 HCLK，因此还需要解析 PPRE2
 * 才能得到实际外设时钟，不能固定假设为 72 MHz。系统回退到 HSI 时，
 * 该计算也能让波特率跟随实际时钟调整。
 */
static uint32_t USART1_GetPeripheralClock(void)
{
    SystemCoreClockUpdate();
    const uint32_t ppre2 = (RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos;
    // PPRE2 编码：0xx=/1，100=/2，101=/4，110=/8，111=/16。
    const uint32_t divider = (ppre2 < 4U) ? 1U : (1U << (ppre2 - 3U));
    return SystemCoreClock / divider;
}

// 初始化
void Driver_USART1_Init(void){
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
    // 每个引脚在 CRH 中占 4 位；先清除完整字段，避免残留配置影响最终模式。
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
    GPIOA->CRH |= GPIO_CRH_MODE9 | GPIO_CRH_CNF9_1;

    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOA->CRH |= GPIO_CRH_CNF10_0;

    // 3. 串口配置
    // 3.1 波特率设置
    const uint32_t pclk2 = USART1_GetPeripheralClock();
    // 16 倍过采样时 BRR 编码等价于 PCLK2/baud；加 baud/2 完成整数四舍五入。
    USART1->BRR = (pclk2 + (115200U / 2U)) / 115200U;

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

void Driver_USART1_FlushReceive(void)
{
    volatile uint32_t discard;

    /*
     * STM32F1 通过“先读 SR、再读 DR”清除 IDLE 和接收错误标志。
     * 循环同时排空当前 RXNE 数据；本函数会主动丢弃尚未处理的接收字节。
     */
    do
    {
        discard = USART1->SR;
        discard = USART1->DR;
    } while ((USART1->SR & USART_SR_RXNE) != 0U);

    (void)discard;
}

uint16_t Driver_USART1_ReceiveString(
    uint8_t buff[], uint16_t capacity,
    uint32_t first_byte_timeout_ms, uint32_t inter_byte_timeout_ms)
{
    if ((buff == NULL) || (capacity == 0U))
    {
        return 0U;
    }

    uint16_t length = 0U;
    uint32_t wait_start = Delay_GetCycleCount();
    uint32_t last_byte = wait_start;

    while (length < capacity)
    {
        const uint32_t status = USART1->SR;

        if ((status & USART_SR_RXNE) != 0U)
        {
            const uint8_t byte = (uint8_t)USART1->DR;
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
            volatile uint32_t discard = USART1->DR;
            (void)discard;
        }

        if ((length == 0U) &&
            Delay_TimeoutElapsed(wait_start, first_byte_timeout_ms))
        {
            break;
        }
        if ((length > 0U) &&
            Delay_TimeoutElapsed(last_byte, inter_byte_timeout_ms))
        {
            break;
        }
    }

    return length;
}


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
