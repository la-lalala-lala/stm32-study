#include "ch395.h"
#include "delay/delay.h"
#include "spi/spi1.h"
#include "stm32f1xx.h"

/* 战舰 V4：RST=PD7，INT#=PG6，CS=PG9，SPI1=PA5/PA6/PA7。 */
#define CH395_INT_PIN GPIO_IDR_IDR6

/**
 * @brief 初始化 CH395Q 的 SPI1、复位引脚和中断引脚
 * @return 无
 * @note RST 使用 PD7，空闲为高电平；INT# 使用 PG6，低电平表示有中断。
 */
void ch395PortInit(void)
{
    RCC->APB2ENR |= (RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPGEN);

    /* PD7 RST：推挽输出，空闲为高电平。先置高可避免切换模式时误复位。 */
    GPIOD->BSRR = GPIO_BSRR_BS7;
    GPIOD->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
    GPIOD->CRL |= GPIO_CRL_MODE7;

    /* PG6 INT#：上拉输入，CH395Q 中断为低电平有效。 */
    GPIOG->BSRR = GPIO_BSRR_BS6;
    GPIOG->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_CNF6);
    GPIOG->CRL |= GPIO_CRL_CNF6_1;

    spi1_init();
    spi1_stop();
}

/**
 * @brief 通过 PD7 输出低电平脉冲，对 CH395Q 执行硬件复位
 * @return 无
 * @note 复位脉冲持续 10 ms；释放复位后，调用方应按芯片要求继续等待初始化完成。
 */
void ch395Reset(void)
{
    GPIOD->BRR = GPIO_BRR_BR7;
    Delay_ms(10);
    GPIOD->BSRR = GPIO_BSRR_BS7;
}

/**
 * @brief 轮询读取 CH395Q 的 INT# 引脚电平
 * @return 引脚原始电平；0 表示存在低电平有效中断，1 表示当前无中断
 * @note 战舰 V4 的 INT# 为低电平有效，处理中断时应判断返回值是否为 0。
 */
uint8_t ch395QueryInterrupt(void)
{
    return ((GPIOG->IDR & CH395_INT_PIN) != 0U) ? 1U : 0U;
}

/**
 * @brief 拉低片选并向 CH395Q 写入一个命令字节
 * @param cmd CH395Q 命令码
 * @return 无
 * @note 函数返回后 CS 仍保持低电平，直到调用 ch395EndCommand()。
 */
void ch395WriteCommand(uint8_t cmd)
{
    spi1_start();
    (void)spi1_swap_byte(cmd);
    Delay_us(1);
}

/**
 * @brief 在当前命令事务中向 CH395Q 写入一个数据字节
 * @param mdata 待写入的数据
 * @return 无
 * @note 应在 ch395WriteCommand() 之后、ch395EndCommand() 之前调用。
 */
void ch395WriteData(uint8_t mdata)
{
    (void)spi1_swap_byte(mdata);
    Delay_us(1);
}

/**
 * @brief 发送占位字节并从 CH395Q 读取一个数据字节
 * @return 从 CH395Q 读取到的数据
 * @note SPI 读取使用 0x00 作为发送占位字节，读取后保留约 1 us 间隔。
 */
uint8_t ch395ReadData(void)
{
    const uint8_t data = spi1_swap_byte(0x00U);
    Delay_us(1);
    return data;
}

/**
 * @brief 拉高片选并结束当前 CH395Q 命令事务
 * @return 无
 * @note 必须在一条命令的所有参数和返回数据完成交换后调用。
 */
void ch395EndCommand(void)
{
    spi1_stop();
}

/**
 * @brief 使用调用方提供的静态网络参数初始化 CH395Q
 * @param ip 4 字节本机 IPv4 地址，按网络字节顺序排列
 * @param gateway 4 字节默认网关 IPv4 地址，按网络字节顺序排列
 * @param netmask 4 字节子网掩码，按网络字节顺序排列
 * @param mac 6 字节 MAC 地址；仅当 CH395Q 当前地址不一致时写入并校验
 * @return 0 初始化成功；-1 SPI 通信检查失败；-2 CH395Q 初始化命令失败；
 *         -3 等待 PHY 链路建立超时；-4 MAC 地址配置或校验失败
 * @note 四个数组均由调用方提供，函数只在本次调用期间读取其内容。
 *       函数会依次完成硬件初始化、静态参数配置、PHY 链路等待，并打印
 *       当前生效的 IP、网关、子网掩码和 MAC 信息；链路等待最长约 10 秒。
 */
int ch395StaticInit(uint8_t ip[4],uint8_t gateway[4],uint8_t netmask[4],uint8_t mac[6]){
    uint8_t status;
    uint8_t check;
    uint8_t phy;

    ch395PortInit();

    ch395Reset();
    Delay_ms(100);

    check = ch395CmdCheckExist(0x65);
    if (check != 0x9AU){
        printf("CH395 SPI check failed: 0x%02X\r\n", check);
        return -1;
    }

    if (ch395CmdEnsureMacAddress(mac) != 0){
        printf("MAC configuration failed\r\n");
        return -4;
    }

    ch395CmdSetIpAddress(ip);
    ch395CmdSetGatewayIpAddress(gateway);
    ch395CmdSetMaskAddress(netmask);

    status = ch395CmdInitialize();
    if (status != CMD_ERR_SUCCESS)
    {
        printf("CH395 init failed: 0x%02X\r\n", status);
        return -2;
    }

    /* 允许 CH395 响应 PC 发来的 ICMP Echo 请求。 */
    ch395CmdEnablePing(1);

    for (uint8_t retry = 0; retry < 50; retry++)
    {
        phy = ch395CmdGetPhyStatus();
        if (phy != PHY_DISCONN)
        {
            uint8_t ip_info[20] = {0};
            uint8_t mac_addr[6] = {0};

            printf("CH395 link up, PHY status: 0x%02X\r\n", phy);

            /* 读取并打印 CH395 当前生效的网络参数。 */
            ch395CmdGetIpInfo(ip_info);
            ch395CmdGetMacAddress(mac_addr);

            printf("CH395 network information:\r\n");
            printf("  IP:   %u.%u.%u.%u\r\n",(unsigned int)ip_info[0],(unsigned int)ip_info[1],(unsigned int)ip_info[2],(unsigned int)ip_info[3]);
            printf("  Gateway: %u.%u.%u.%u\r\n",(unsigned int)ip_info[4],(unsigned int)ip_info[5],(unsigned int)ip_info[6],(unsigned int)ip_info[7]);
            printf("  Netmask: %u.%u.%u.%u\r\n",(unsigned int)ip_info[8],(unsigned int)ip_info[9],(unsigned int)ip_info[10],(unsigned int)ip_info[11]);
            printf("  MAC:  %02X:%02X:%02X:%02X:%02X:%02X\r\n",(unsigned int)mac_addr[0],(unsigned int)mac_addr[1],(unsigned int)mac_addr[2],(unsigned int)mac_addr[3],(unsigned int)mac_addr[4],(unsigned int)mac_addr[5]);

            /* 链路建立会产生 PHY_CHANGE，全局状态读取后 INT# 才恢复为空闲电平。 */
            (void)ch395CmdGetGlobalInterruptStatus();
            return 0;
        }

        Delay_ms(200);
    }

    printf("CH395 link timeout\r\n");
    return -3;
}
