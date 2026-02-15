#include <usart.h>
#include "dma.h"
/**
 * PA9作为TX，PA10作为RX使用
 */

 // 定义全局变量，表示传输完成
uint8_t isFinished = 0;
// 定义全局常量，放在ROM，作为数据源
const uint8_t src[] = {10,20,30,40};
// 定义变量数组，放在RAM，用来存储接收到的数据
uint8_t dest[4] = {0};

int main(){
    // 串口初始化
    init_usart();
    
    // 测试串口是否正常工作
    printf("Serial test: Hello World!\n");
    
    // DMA初始化
    DMA_Init();

    // 打印变量和常量地址
    printf("src: %p\n", src);
    printf("dest: %p\n", dest);
    
    // 打印初始dest值
    printf("Initial dest values: ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", dest[i]);
    }
    printf("\n");

    // 开启DMA传输
    printf("Starting DMA transfer...\n");
    DMA_Transmit((uint32_t)src, (uint32_t)dest, 4);

    int counter = 0;
    while (1)
    {
        if (isFinished)
        {
            printf("DMA transfer completed!\n");
            for (int i = 0; i < 4; i++)
            {
                printf("dest[%d] = %d\n", i, dest[i]);
            }
            isFinished = 0;
            break; // 传输完成后退出循环
        }
        
        // 每100ms打印一次状态
        if (counter++ % 1000000 == 0) {
            printf("Waiting for DMA transfer...\n");
        }
    }
    
    // 死循环
    while (1);
}
