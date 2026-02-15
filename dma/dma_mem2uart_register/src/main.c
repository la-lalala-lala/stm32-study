#include <usart.h>
#include "dma.h"
#include "delay.h"
/**
 * PA9作为TX，PA10作为RX使用
 */


// 定义变量数组，放在RAM，用来存储接收到的数据
uint8_t src[4] = {'a','b','c','d'};

int main(){
    // 串口初始化
    init_usart();
    
    // 测试串口是否正常工作
    printf("Hello World!\n");
    
    // DMA初始化
    DMA_Init();

    // 开启DMA传输
    printf("Starting DMA transfer...\n");
    delay_ms(1);
    DMA_Transmit((uint32_t)src, (uint32_t)&USART1->DR, 4);
    while (1)
    {

    }
}
