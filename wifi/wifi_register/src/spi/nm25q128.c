#include "nm25q128.h"

void nm25q128_init(void){
    // 初始化里唤醒/复位
    // 0xAB：先把 Flash 从 Deep Power-Down 里唤醒
    // 0x66：允许软件复位
    // 0x99：执行软件复位，让 Flash 回到默认 SPI 状态
    spi_init();
    spi_start();
    spi_swap_byte(NM25Q128_CMD_RELEASE_POWER_DOWN);
    spi_stop();
    Delay_us(100);

    spi_start();
    spi_swap_byte(NM25Q128_CMD_RESET_ENABLE);
    spi_stop();
    spi_start();
    spi_swap_byte(NM25Q128_CMD_RESET_MEMORY);
    spi_stop();
    Delay_us(100);
}

void nm25q128_read_id(uint8_t *mid,uint16_t *did){
    // 走读写流程
    /* 1. 片选启动  */
    spi_start();
    /* 2. 交换数据   SPI 虽然是交换数据  实际上 多数情况下都是只有一方发的数据有用 */
    // 2.1 发送9F指令
    spi_swap_byte(NM25Q128_CMD_READ_ID);
    // 2.2 接收数据的时候   填写的交换发送数据是没有意义的
    *mid = spi_swap_byte(0XFF);
    *did = 0;
    *did |= spi_swap_byte(0XFF) << 8;
    *did |= spi_swap_byte(0XFF) << 0;

    /* 3. 片选关闭 */
    spi_stop();
}

void nm25q128_wait_busy(void){
    uint8_t status = 0;
    // 1. 片选开启
    spi_start();
    // 2. 发送读取状态指令
    spi_swap_byte(NM25Q128_CMD_READ_STATUS1);
    // 3. 接收状态寄存器的值
    status = spi_swap_byte(0xff);
    // 如果最低位为1  表示忙
    while (status & 0x01){
        status = spi_swap_byte(0xff);
    }
    // 4. 片选关闭
    spi_stop();
}

void nm25q128_write_enable(void){
    // 1. 片选开启
    spi_start();
    // 2. 开启写使能
    spi_swap_byte(NM25Q128_CMD_WRITE_ENABLE);
    // 3. 片选关闭
    spi_stop();
}

void nm25q128_write_disenable(void){
    // 1. 片选开启
    spi_start();
    // 2. 关闭写使能
    spi_swap_byte(NM25Q128_CMD_WRITE_DISABLE);
    // 3. 片选关闭
    spi_stop();
}


// 块 64K一块  一共256块   2位16进制数字 
// 扇区 4k  一共16个扇区  1位16进制数字
// 页  256字节  一共16个页  1位16进制数字
void nm25q128_write_page(uint8_t block,uint8_t sector,uint8_t page,uint8_t data[],uint8_t dataLen){
    // 1. 判断忙状态
    nm25q128_wait_busy();
    // 2. 开启写使能
    nm25q128_write_enable();
    // 3. 写入数据
    // 3.1 片选使能
    spi_start();
    // 3.2 发送写数据的指令
    spi_swap_byte(NM25Q128_CMD_PAGE_PROGRAM);
    // 3.3 写入数据的起始地址
    // W25Q32的地址是22位   表示的时候使用24位  =  6个16进制表示
    uint32_t addr = block * 0x010000 + sector * 0x001000 + page * 0x000100;
    spi_swap_byte(addr>>16);
    spi_swap_byte((addr >> 8) & 0xff);
    spi_swap_byte((addr >> 0) & 0xff);
    // 3.4 写入数据
    for (uint8_t i = 0; i < dataLen; i++){
        spi_swap_byte(data[i]);
    }
    // 3.5 关闭片选 =>  如果不关 是没有办法再次写入命令的
    spi_stop();
    // 4. 等待页编程完成。完成后芯片会自动清除 WEL。
    nm25q128_wait_busy();
}

// 最小单位擦除  =>  一个扇区
void nm25q128_sector_erase(uint8_t block,uint8_t secotr){
    // 1. 等待忙状态
    nm25q128_wait_busy();
    // 2. 开启写使能
    nm25q128_write_enable();
    // 3. 执行扇区擦除
    // 3.1 开启片选
    spi_start();
    // 3.2 发送擦除命令
    spi_swap_byte(NM25Q128_CMD_SECTOR_ERASE);
    // 3.3 发送擦除地址
    uint32_t addr = block * 0x010000 + secotr * 0x001000;
    spi_swap_byte(addr>>16);
    spi_swap_byte((addr >> 8) & 0xff);
    spi_swap_byte((addr >> 0) & 0xff);
    // 3.4 关闭片选
    spi_stop();
    // 4. 等待扇区擦除完成。完成后芯片会自动清除 WEL。
    nm25q128_wait_busy();
}


// 读取数据  => 可以一次性读取多个扇区的数据
void nm25q128_read_data(uint8_t block,uint8_t sector,uint8_t page,uint8_t buff[],uint16_t dataLen){
    // 1. 等待忙状态
    nm25q128_wait_busy();
    // 2. 开始读数据
    // 2.1 开启片选
    spi_start();
    // 2.2 发送读数据指令
    spi_swap_byte(NM25Q128_CMD_READ_DATA);
    uint32_t addr = block * 0x010000 + sector * 0x001000 + page * 0x000100;
    spi_swap_byte(addr>>16);
    spi_swap_byte((addr >> 8) & 0xff);
    spi_swap_byte((addr >> 0) & 0xff);
    // 2.4 读取数据存储到buff
    for (uint8_t i = 0; i < dataLen; i++){
        buff[i] = spi_swap_byte(0xff);
    }
    // 4. 关闭写使能
    spi_stop();
}
