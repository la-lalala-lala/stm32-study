#include "system.h"

#define HSE_STARTUP_TIMEOUT_MS 100U

void SystemClock_Config(void) {
    if ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) {
        SystemCoreClockUpdate();
        return;
    }

    SystemCoreClockUpdate();
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    const uint32_t hse_timeout_cycles =
        (SystemCoreClock / 1000U) * HSE_STARTUP_TIMEOUT_MS;
    const uint32_t hse_start_cycles = DWT->CYCCNT;

    // 启动外部高速晶振（HSE）
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY) &&
           ((uint32_t)(DWT->CYCCNT - hse_start_cycles) < hse_timeout_cycles));

    // HSE启动失败处理，fallback到内部高速晶振（HSI）
    if (!(RCC->CR & RCC_CR_HSERDY)) {
        // 启动内部高速晶振（HSI）
        RCC->CR |= RCC_CR_HSION;
        // 等待HSI稳定
        while (!(RCC->CR & RCC_CR_HSIRDY));
        // 切换系统时钟源到HSI
        RCC->CFGR &= ~RCC_CFGR_SW;
        // 等待HSI成为系统时钟源
        while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

        RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    } else {
        // HSE启动成功，配置PLL
        // 清除PLL输入源位
        RCC->CFGR &= ~RCC_CFGR_PLLSRC;
        // 设置PLL输入源为HSE
        RCC->CFGR |= RCC_CFGR_PLLSRC;

        // 清除PLL倍频系数位
        RCC->CFGR &= ~RCC_CFGR_PLLMULL;
        // 配置PLL倍频系数为9（8MHz * 9 = 72MHz）
        RCC->CFGR |= RCC_CFGR_PLLMULL9;

        // 启动PLL
        RCC->CR |= RCC_CR_PLLON;
        // 等待PLL稳定
        while (!(RCC->CR & RCC_CR_PLLRDY));

        // 根据系统时钟频率设置Flash等待周期
        FLASH->ACR &= ~FLASH_ACR_LATENCY;
        if (SYSCLK_FREQ > 24000000U) {
            // 系统时钟>24MHz时，设置2个等待状态
            FLASH->ACR |= FLASH_ACR_LATENCY_2;
        } else {
            // 系统时钟≤24MHz时，设置1个等待状态
            FLASH->ACR |= FLASH_ACR_LATENCY_1;
        }

        // 配置总线分频器
        RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
        // AHB总线不分频（HCLK = SYSCLK = 72MHz）
        RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
        // APB1总线2分频（PCLK1 = HCLK/2 = 36MHz，STM32F103最大支持36MHz）
        RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
        // APB2总线不分频（PCLK2 = HCLK = 72MHz）
        RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

        // 清除系统时钟源选择位
        RCC->CFGR &= ~RCC_CFGR_SW;
        // 切换系统时钟源到PLL
        RCC->CFGR |= RCC_CFGR_SW_PLL;
        // 等待PLL成为系统时钟源
        while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
    }

    SystemCoreClockUpdate();
}
