#include "usart/usart.h"
#include <string.h>
#include <stm32f1xx.h>
#include <spi/nm25q128.h>

int main(void)
{
	init_usart();
	nm25q128_init();
	printf("串口通信测试\r\n");
	printf("fw=min-spi-pb12-pb14-v1\r\n");
	uint8_t mid =0;
	uint16_t did = 0;
	nm25q128_read_id(&mid,&did);
	printf("mid=0x%02x,did=0x%04x\r\n", mid, did);
	printf("id %s\r\n",
	       (mid == NM25Q128_MANUFACTURER_ID && did == NM25Q128_DEVICE_ID) ? "ok" : "error");

	/* 1. 擦除扇区  块 0-255 扇区 0-15 */
	// nm25q128_sector_erase(0,1);

	// 2. 写入数据 page 0-15
	uint8_t text[] = "hello mengge";
	nm25q128_write_page(0,1,1,text,strlen((char *)text));
	
	/* 3. 读取数据 */
	uint8_t flash_buff[20] = {0};
	nm25q128_read_data(0,1,1,flash_buff,strlen((char *)text));
	printf("---------\r\n");
	printf("%s\r\n",flash_buff);
	while(1)
	{

	}
}
