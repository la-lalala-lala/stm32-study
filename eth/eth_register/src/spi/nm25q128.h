#ifndef __INF_NM25Q128_H
#define __INF_NM25Q128_H

#include "spi2.h"

#define NM25Q128_MANUFACTURER_ID 0x52U
#define NM25Q128_MEMORY_TYPE     0x21U
#define NM25Q128_CAPACITY_ID     0x18U
#define NM25Q128_DEVICE_ID       0x2118U

// WREN: Write Enable，写/擦除前必须先发送，置位 WEL。
#define NM25Q128_CMD_WRITE_ENABLE  0x06U

// WRDI: Write Disable，清除 WEL，禁止后续写/擦除。
#define NM25Q128_CMD_WRITE_DISABLE 0x04U

// RDSR1: Read Status Register 1，最低位 WIP=1 表示忙。
#define NM25Q128_CMD_READ_STATUS1  0x05U

// READ: Read Data Bytes，发送 3 字节地址后连续读数据。
#define NM25Q128_CMD_READ_DATA     0x03U

// PP: Page Program，页编程，最多一次写入 256 字节。
#define NM25Q128_CMD_PAGE_PROGRAM  0x02U

// SE: Sector Erase，4KB 扇区擦除。
#define NM25Q128_CMD_SECTOR_ERASE  0x20U

// RDID: Read Identification，返回 Manufacturer ID + Memory Type + Memory Density。
#define NM25Q128_CMD_READ_ID       0x9FU

// RDP: Release From Deep Power Down，让 Flash 退出深度掉电模式。
#define NM25Q128_CMD_RELEASE_POWER_DOWN 0xABU

// RSTEN: Reset Enable，允许后续执行软件复位。
#define NM25Q128_CMD_RESET_ENABLE       0x66U

// RST: Reset Memory，执行软件复位，恢复默认 SPI 状态。
#define NM25Q128_CMD_RESET_MEMORY       0x99U

void nm25q128_init(void);

void nm25q128_read_id(uint8_t *mid,uint16_t *did);

// 块 64K一块  一共256块   2位16进制数字 
// 扇区 4k  一共16个扇区  1位16进制数字
// 页  256字节  一共16个页  1位16进制数字
void nm25q128_write_page(uint8_t block,uint8_t sector,uint8_t page,uint8_t data[],uint8_t dataLen);

// 最小单位擦除  =>  一个扇区
void nm25q128_sector_erase(uint8_t block,uint8_t secotr);


// 读取数据  => 可以一次性读取多个扇区的数据
void nm25q128_read_data(uint8_t block,uint8_t sector,uint8_t page,uint8_t buff[],uint16_t dataLen);

#endif
