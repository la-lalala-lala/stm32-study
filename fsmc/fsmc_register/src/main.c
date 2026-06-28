#include "usart/usart.h"
#include <string.h>
#include <stm32f1xx.h>
#include "fsmc/fsmc.h"

// GCC 下 __attribute__((at(addr))) 不会把变量放到绝对地址。
// 访问 FSMC 外部 SRAM 时，直接使用映射地址指针。
#define FSMC_SRAM_BASE ((uint32_t)0x68000000)
#define V1             ((volatile uint8_t *)(FSMC_SRAM_BASE + 0x00))
#define V2             ((volatile uint8_t *)(FSMC_SRAM_BASE + 0x04))
#define V3             ((volatile uint8_t *)(FSMC_SRAM_BASE + 0x08))
#define V4             ((volatile uint8_t *)(FSMC_SRAM_BASE + 0x0C))


// 声明在全局变量
const uint8_t buff1[1024] = {0};

const char *name = "hello";


int main(void)
{
	init_usart();
	printf("串口通信测试\r\n");
	fsmc_init();

	// 初始化完成之后  就已经有了对应的内存
	uint8_t v = 10;
	printf("v= %p , %d\n",(void *)&v,v);

	*V1 = 11;
	printf("v1= %p , %d\n",(void *)V1,*V1);

	*V2 = 12;
	printf("v2= %p , %d\n",(void *)V2,*V2);

	*V3 = 13;
	printf("v3= %p , %d\n",(void *)V3,*V3);

	// 直接定义指针
	volatile uint8_t *int8_addr = (volatile uint8_t *)(FSMC_SRAM_BASE + 0x01);
	*int8_addr = 14;
	*(volatile uint8_t *)(FSMC_SRAM_BASE + 0x01) = 15;
	printf("0x68000001:%d\n",*(volatile uint8_t *)(FSMC_SRAM_BASE + 0x01));

	// 使用宏定义声明指针
	*V4 = 24;
	printf("v4:%p,%d\n",(void *)V4,*V4);

	// 函数中声明的内容存储到栈中  => SRAM
	uint8_t buff[1024] = {0};
	printf("buff:%p %s\n",(void *)buff,buff);

	printf("buff1:%p %s\n",(void *)buff1,buff1);

	printf("name:%p %s\n",(void *)name,name);

	while(1){

	}
}
