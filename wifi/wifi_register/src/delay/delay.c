#include "delay.h"

static uint8_t cycle_counter_initialized = 0;

static void Delay_InitCycleCounter(void)
{
	if (cycle_counter_initialized == 0U)
	{
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
		DWT->CYCCNT = 0;
		DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
		cycle_counter_initialized = 1U;
	}
}

uint32_t Delay_GetCycleCount(void)
{
	Delay_InitCycleCounter();
	return DWT->CYCCNT;
}

uint8_t Delay_TimeoutElapsed(uint32_t start_cycles, uint32_t timeout_ms)
{
	const uint32_t timeout_cycles = (SystemCoreClock / 1000U) * timeout_ms;
	return ((uint32_t)(Delay_GetCycleCount() - start_cycles) >= timeout_cycles);
}

void Delay_us(uint16_t us)
{
	const uint32_t start_cycles = Delay_GetCycleCount();
	const uint32_t wait_cycles = (SystemCoreClock / 1000000U) * us;

	while ((uint32_t)(Delay_GetCycleCount() - start_cycles) < wait_cycles)
	{
	}
}

void Delay_ms(uint16_t ms)
{
	while (ms--)
	{
		Delay_us(1000);
	}
}

void Delay_s(uint16_t s)
{
	while (s--)
	{
		Delay_ms(1000);
	}
}
