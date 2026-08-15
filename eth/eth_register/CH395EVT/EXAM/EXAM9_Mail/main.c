/********************************** (C) COPYRIGHT *********************************
 * File Name          : MAIN
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 Mail Function Demo
 **********************************************************************************/
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "CH395INC.H"
#include "CH395CMD.H"
#include "ch395.h"
#include "MAIL.H"
#include "mailinc.h"
#include "CH395MAILCMD.H"
/**************************************************************************************************************************
 * Operation Mode (Default is type 5)
 * 1  Only send email -- After sending, logout and close the socket connection;
 * 2  Only receive email -- After receiving (fetching mail list), logout and close the socket connection;
 * 3  Only receive email -- After receiving (reading and deleting mail), logout and close the socket connection;
 * 4  Send and receive email -- After sending, start receiving; after receiving, logout and close the socket connection;
 * 5  Send and receive email -- After receiving, reply to email; after replying, logout, delete the mail, then logout and close the socket connection.
 **************************************************************************************************************************/
#include "mailinc.h"
/**********************************************************************************/

/* CH395 related definitions */
const UINT8 CH395IPAddr[4] = {192, 168, 1, 100};  /* CH395 IP address */
const UINT8 CH395GWIPAddr[4] = {192, 168, 1, 1}; /* CH395 Gateway */
const UINT8 CH395IPMask[4] = {255, 255, 255, 0};  /* CH395 Subnet Mask */

/* socket0 related definitions, for SMTP email sending */
const UINT8 Socket0DesIP[4] = {111, 124, 203, 45}; /* Socket 0 destination IP address */
const UINT16 Socket0DesPort = SMTP_SERVER_PORT;    /* Socket 0 destination port */
const UINT16 Socket0SourPort = 61214;               /* Socket 0 source port */
/*  {123,58,178,201}   "smtp.126.com"    */
/*  {111,124,203,45}   "smtp.163.com"    */
/*  {113,108,16,44}    "smtp.qq.com"     */
/*  {74,125,129,108}   "smtp.gmail.com"  */

/* socket1 related definitions, for POP mail receiving */
const UINT8 Socket1DesIP[4] = {111, 124, 203, 45}; /* Socket 1 destination IP address */
const UINT16 Socket1DesPort = POP3_SERVER_PORT;    /* Socket 1 destination port */
const UINT16 Socket1SourPort = 8152;               /* Socket 1 source port */
/*  {220,181,15,128}   "pop.126.com", */
/*  {111,124,203,45}   "pop.163.com", */
/*  {113,108,16,116}   "pop.qq.com",  */
/*  {74,125,141,108}   "pop.gmail.com"*/

/* SMTP send email related parameters */
const char *m_Server = "smtp.163.com";       // SMTP server name
const char *m_UserName = "00000000"; // User name
const char *m_PassWord = "00000000"; // Password
const char *m_SendFrom = "00000000"; // Sender address
const char *m_SendName = "00000000"; // Sender name
const char *m_SendTo = "00000000";  // Recipient address
const char *m_Subject = "text";              // Email subject
const char *m_FileName = "m_file.txt";       // Attachment name (use "\0" if no attachment)

/* POP receive email related parameters */
const char *p_Server = "pop.163.com";        // POP server
const char *p_UserName = "00000000"; // POP login username
const char *p_PassWord = "00000000"; // POP login password

/***********************************************************************************/
UINT8 ReceDatFlag = 0; /* Data received flag */
UINT8 SendDatFlag = 0;
UINT8 CheckType; /* Handshake check type */
UINT8 OrderType; /* Command type */
UINT16 ReceLen;  /* Length of received data in buffer */

extern void CH395GlobalInterrupt(void);
extern void CH395SocketInitOpen(UINT8 index);
extern void ch395mail(void);

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

    /*CH395 Interrupt IO*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*CH395 Reset IO*/
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
int main()
{
    Delay_Init();
    USART_Printf_Init(115200);
    CH395_GPIO_INIT();

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

    ch395mail();

#ifdef send_mail
    ch395mail_smtpinit();
    if (send_mail)
        CH395SocketInitOpen(SendSocket);
#endif
#ifdef receive_mail
    ch395mail_pop3init();
    if (receive_mail)
        CH395SocketInitOpen(ReceSocket);
#endif
    while (1)
    {
        if (Query395Interrupt() == 0)
            CH395GlobalInterrupt();
        if (ReceDatFlag)
        {
            ReceDatFlag = 0;
            CH395_MailCmd(OrderType);
        }
    }
}
