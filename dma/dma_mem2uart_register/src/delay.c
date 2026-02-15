#include "delay.h"


/**
 * 2、宏定义的说明
 * #define SysTick_CTRL_ENABLE_Pos             0                                             < SysTick CTRL: ENABLE Position 
 * #define SysTick_CTRL_ENABLE_Msk            (1ul << SysTick_CTRL_ENABLE_Pos)               < SysTick CTRL: ENABLE Mask 
 * 对于SysTick_CTRL_ENABLE_Pos，其中Pos应该是position的缩写，也就是位置，在文件定义位置为0；
 * 对于SysTick_CTRL_ENABLE_Msk，1ul << SysTick_CTRL_ENABLE_Pos，
 * 意思就是将1左移0位，为0000 0000 0000 0001；其中的1ul说明这个常量1是unsigned long，32bit的数据，
 * 因为stm32寄存器是32位的。根据SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk可知，
 * 替换后为SysTick->CTRL |= 1 <<0， 相当于把CTRL最低位置1，也就是打开SysTick定时器。
 */
void delay_us(uint16_t us){
// 定时器重装值
SysTick->LOAD = 72 * us;
// 清除当前计数值
SysTick->VAL = 0;
// 设置内部时钟源（2位->1），不需要中断（1位->0），并启动定时器(0位->1)
SysTick->CTRL = 0x5;
// 等待计数到0，如果计数到0则16位会置位1
while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
// 关闭定时器
SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void delay_ms(uint16_t ms){
    while (ms--)
    {
        delay_us(1000);
    }
}

void delay_s(uint16_t s){
    while (s--)
    {
        delay_ms(1000);
    }
}