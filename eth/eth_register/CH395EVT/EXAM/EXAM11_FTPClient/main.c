/********************************** (C) COPYRIGHT *********************************
 * File Name          : MAIN.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 Chip FTP Client Application - Main Program
 **********************************************************************************/
#include "debug.h"
#include "CH395INC.h"
#include "CH395.H"
#include "CH395FTPINC.H"

#define Query395Interrupt() GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8)
extern void CH395GlobalInterrupt(void);

/**********************************************************************************/
#define CH395_INT_WIRE INT1 /* CH395's INT# Pin  */
/* CH395-related Definitions */
const UINT8 CH395MACAddr[6] = {0x02, 0x03, 0x04, 0x05, 0x06, 0x07}; /* CH395 MAC Address */
const UINT8 CH395IPAddr[4] = {192, 168, 1, 10};                     /* CH395 IP Address */
const UINT8 CH395GWIPAddr[4] = {192, 168, 1, 1};                    /* CH395 Gateway */
const UINT8 CH395IPMask[4] = {255, 255, 255, 0};                    /* CH395 Subnet Mask */
const UINT8 DestIPAddr[4] = {192, 168, 1, 123};                     /* Destination IP */
/* FTP-related Definitions */
const UINT8 *pUserName = "000"; /* Anonymous login */
const UINT8 *pPassword = "000";
char ListName[24]; /* Used to store directory name */
char ListMdk[24];  /* Used to store the newly created directory name */
char FileName[24]; /* Used to store the file name for searching */
char SourIP[17];   /* Used to store the converted IP address in string format */

/**********************************************************************************
 * Function Name  : CH395_GPIO_INIT
 * Description    : Initialize GPIO
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395_GPIO_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* CH395 Interrupt IO */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* CH395 Reset IO */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

/***********************************************************************************
 * Function Name  : main
 * Description    : Main function
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
int main(void)
{
    UINT8 i;
    Delay_Init();
    USART_Printf_Init(115200);

    /*
    CH395 supports three interface modes: SPI, serial, and parallel.
    When using different interfaces, refer to the interface configuration instructions in 'CH395DS1.pdf'
    and ensure that the corresponding .c file is included in the compilation.
    Additionally, define CH395_OP_INTERFACE_MODE in 'CH395INC.h' to match the selected interface.
    For example, when using the SPI interface, the TXD pin should be grounded, the SEL pin should be floating or pulled high,
    and the 'CH395SPI_HW.c' file should be included in the compilation.
    Furthermore,CH395_OP_INTERFACE_MODE should be set to CH395_SPI_MODE in 'CH395INC.h'.
    */

#if (CH395_OP_INTERFACE_MODE == CH395_UART_MODE)
    printf("CH395 UART Interface Mode\r\n");
#endif
#if (CH395_OP_INTERFACE_MODE == CH395_SPI_MODE)
    printf("CH395 SPI Interface Mode\r\n");
#endif
#if (CH395_OP_INTERFACE_MODE == CH395_PARA_MODE)
    printf("CH395 Parallel Interface Mode\r\n");
#endif

    CH395_PORT_INIT();
    printf("CH395EVT Test Demo\r\n");
    CH395_GPIO_INIT();
    CH395Reset();
    Delay_Ms(100);
    i = CH395CMDGetVer();
    printf("CH395VER : %2x\r\n", (UINT16)i);
    CH395_FTPConnect(); /* Establish TCP FTP control connection */
    while (1)
    {
        if (Query395Interrupt() == 0)
            CH395GlobalInterrupt();
        CH395_FTPClientCmd(); /* Query status and execute corresponding commands */
    }
}
