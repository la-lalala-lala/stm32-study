#include <usart.h>

/**
 * 用于指定所需的系统时钟频率。根据参数配置 HSE 或者 HSI 作为 PLL 的输入时钟源，并计算 PLL 的倍频系数以及 Flash 延迟设置。
 * 在 STM32F103C8T6 微控制器中，当未经过任何配置时，默认情况下系统时钟的来源是内部 RC 振荡器（HSI），其频率为 8 MHz。
 * 这意味着，如果你的 STM32F103C8T6 微控制器没有经过任何外部晶振或 PLL 的配置，它将以 8 MHz 的频率运行。
 */
void SystemClock_Config(uint32_t system_clock_freq) {
    uint32_t hse_startup_timeout = 10000; // Startup timeout for HSE (in ms)
    RCC->CR |= RCC_CR_HSEON; // Enable HSE
    while (!(RCC->CR & RCC_CR_HSERDY) && hse_startup_timeout--); // Wait until HSE is ready or timeout
    if (!(RCC->CR & RCC_CR_HSERDY)) {
        // HSE failed to start
        // Handle error or default to other clock source
        // For example, default to HSI:
        RCC->CR |= RCC_CR_HSION; // Enable HSI
        while (!(RCC->CR & RCC_CR_HSIRDY)); // Wait until HSI is ready
        RCC->CFGR &= ~RCC_CFGR_SW; // Select HSI as system clock source
        while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI); // Wait until HSI is used as system clock
    } else {
        // HSE started successfully
        RCC->CFGR &= ~RCC_CFGR_PLLSRC; // Clear PLL source
        RCC->CFGR |= RCC_CFGR_PLLSRC; // PLL source is HSE

        // Configure PLLMULL
        RCC->CFGR &= ~RCC_CFGR_PLLMULL; // Clear PLLMULL bits
        RCC->CFGR |= RCC_CFGR_PLLMULL9; // PLL multiplication factor = 9 (for 72 MHz with 8 MHz HSE)
        
        RCC->CR |= RCC_CR_PLLON; // Enable PLL
        while (!(RCC->CR & RCC_CR_PLLRDY)); // Wait until PLL is ready

        // Set Flash latency (depends on your system clock frequency)
        if (system_clock_freq > 24000000) {
            FLASH->ACR |= FLASH_ACR_LATENCY_2; // Two wait states
        } else {
            FLASH->ACR |= FLASH_ACR_LATENCY_1; // One wait state
        }

        // Configure AHB, APB1, APB2 prescalers
        RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // AHB = SYSCLK / 1
        RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 = HCLK / 2
        RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB2 = HCLK / 1

        // Select PLL as system clock source
        RCC->CFGR &= ~RCC_CFGR_SW; // Clear SW bits
        RCC->CFGR |= RCC_CFGR_SW_PLL; // Select PLL as system clock
        while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); // Wait until PLL is used as system clock
    }
}

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
    uint32_t system_clock_freq = 72000000; // Set system clock frequency (72 MHz)
    SystemClock_Config(system_clock_freq);
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