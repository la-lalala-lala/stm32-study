/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 basic functions
 **********************************************************************************/
/* Header files included */

#include <stdio.h>
#include <string.h>
#include "CH395INC.H"
#include "CH395.H"
#include "mailinc.h"
#include "debug.h"
#include "mail.h"

extern const UINT8 CH395IPAddr[4];   /* CH395 IP address */
extern const UINT8 CH395GWIPAddr[4]; /* CH395 Gateway */
extern const UINT8 CH395IPMask[4];   /* CH395 Subnet Mask */

extern const UINT8 Socket0DesIP[4];  /* Destination IP for Socket 0 */
extern const UINT16 Socket0DesPort;  /* Destination port for Socket 0 */
extern const UINT16 Socket0SourPort; /* Source port for Socket 0 */

extern const UINT8 Socket1DesIP[4];  /* Destination IP for Socket 1 */
extern const UINT16 Socket1DesPort;  /* Destination port for Socket 1 */
extern const UINT16 Socket1SourPort; /* Source port for Socket 1 */

/* Common variable definitions */
#define MybufLen 4096        /* Max buffer size for CH395 receive data */
UINT8 MyBuffer[2][MybufLen]; /* Data buffer */

struct _SOCK_INF SockInf[2]; /* Save socket information */
struct _CH395_SYS CH395Inf;  /* Save CH395 information */
UINT8 tcptimeout_flag;
UINT8 phyDiscont_flag;

/**********************************************************************************
 * Function Name  : mStopIfError
 * Description    : Debug usage, shows error code and halts
 * Input          : iError
 * Output         : None
 * Return         : None
 **********************************************************************************/
void mStopIfError(UINT8 iError)
{
    if (iError == CMD_ERR_SUCCESS)
        return;                              /* Operation successful */
    printf("Error: %02X\n", (UINT16)iError); /* Display error */
    while (1)
    {
        Delay_Ms(200);
        Delay_Ms(200);
    }
}

/**********************************************************************************
 * Function Name  : InitCH395InfParam
 * Description    : Initialize CH395Inf parameters
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void InitCH395InfParam(void)
{
    memset(&CH395Inf, 0, sizeof(CH395Inf));                          /* Clear CH395Inf */
    memcpy(CH395Inf.IPAddr, CH395IPAddr, sizeof(CH395IPAddr));       /* Copy IP address into CH395Inf */
    memcpy(CH395Inf.GWIPAddr, CH395GWIPAddr, sizeof(CH395GWIPAddr)); /* Copy gateway IP address */
    memcpy(CH395Inf.MASKAddr, CH395IPMask, sizeof(CH395IPMask));     /* Copy subnet mask */
}

/*********************************************************************************
 * Function Name  : InitSocketParam
 * Description    : Initialize socket
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void InitSocketParam(void)
{
    memset(&SockInf[0], 0, sizeof(SockInf[0]));                    /* Clear SockInf[0] */
    memcpy(SockInf[0].IPAddr, Socket0DesIP, sizeof(Socket0DesIP)); /* Copy destination IP */
    SockInf[0].DesPort = Socket0DesPort;                           /* Set destination port */
    SockInf[0].SourPort = Socket0SourPort;                         /* Set source port */
    SockInf[0].ProtoType = PROTO_TYPE_TCP;                         /* TCP mode */

    memset(&SockInf[1], 0, sizeof(SockInf[1]));                    /* Clear SockInf[1] */
    memcpy(SockInf[1].IPAddr, Socket1DesIP, sizeof(Socket1DesIP)); /* Copy destination IP */
    SockInf[1].DesPort = Socket1DesPort;                           /* Set destination port */
    SockInf[1].SourPort = Socket1SourPort;                         /* Set source port */
    SockInf[1].ProtoType = PROTO_TYPE_TCP;                         /* TCP mode */
}

/**********************************************************************************
 * Function Name  : CH395SocketInitOpen
 * Description    : Configure CH395 socket parameters, initialize and open socket
 * Input          : index
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395SocketInitOpen(UINT8 index)
{
    UINT8 i;
    /* Socket index in TCP client mode */
    memset(MyBuffer[index], '\0', sizeof(MyBuffer[index]));
    if (index == SendSocket)
        CheckType = SMTP_CHECK_CNNT;
    if (index == ReceSocket)
        CheckType = POP_CHECK_CNNT;
    CH395CMDSetSocketDesIP(index, SockInf[index].IPAddr);       /* Set target IP */
    CH395CMDSetSocketProtType(index, SockInf[index].ProtoType); /* Set protocol type */
    CH395CMDSetSocketDesPort(index, SockInf[index].DesPort);    /* Set destination port */
    CH395CMDSetSocketSourPort(index, SockInf[index].SourPort);  /* Set source port */
    i = CH395CMDOpenSocket(index);                              /* Open socket */
    mStopIfError(i);                                            /* Check success */
    i = CH395CMDTCPConnect(index);                              /* TCP connect */
    mStopIfError(i);                                            /* Check success */
}

/*********************************************************************************
 * Function Name  : CH395SocketInterrupt
 * Description    : CH395 socket interrupt, called from global interrupt
 * Input          : sockindex
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395SocketInterrupt(UINT8 sockindex)
{
    UINT8 sock_int_socket;
    UINT8 i;
    UINT16 len;
    sock_int_socket = CH395CMDGetSocketInt(sockindex); /* Get socket interrupt status */
    if (sock_int_socket & SINT_STAT_SENDBUF_FREE)      /* Send buffer free, can continue writing data */
    {
    }
    if (sock_int_socket & SINT_STAT_SEND_OK) /* Send complete interrupt */
    {
        if (sockindex == SendSocket)
            SendDatFlag = 1;
    }
    if (sock_int_socket & SINT_STAT_RECV) /* Receive interrupt */
    {
        len = CH395CMDGetRecvLength(sockindex); /* Get length of data in buffer */
        if (len == 0)
            return;
        if (len > MybufLen)
            len = MybufLen;
        ReceDatFlag = 1;
        CH395CMDGetRecvData(sockindex, len, MyBuffer[sockindex]); /* Read data */
        SockInf[sockindex].RemLen = len;                          /* Save length */
        ReceLen = len;
#if DEBUG
        printf("ReceLen= %d\n", ReceLen);
        printf("CheckType= %02x\n", (UINT16)CheckType);
        printf("MyBuffer = %s\n", MyBuffer[sockindex]);
#endif
        if (CheckType != uncheck)
        {
            i = ch395mail_CheckResponse(MyBuffer[sockindex], CheckType);
            memset(MyBuffer[sockindex], 0, sizeof(MyBuffer[sockindex]));
            if (i == POP_ERR_DROP) // Drop current data
            {
                u8 tempvar = 200;
                while (tempvar--)
                {
                    Delay_Ms(2);
                    if (CH395CMDGetRecvLength(sockindex) != 0)
                    {
                        tempvar = 200;
                        CH395CMDClearRecvBuf(sockindex);
                    }
                }
                i = CHECK_SUCCESS;
            }

            if (i != CHECK_SUCCESS)
            {
                if (sockindex == SendSocket)
                    OrderType = SMTP_ERR_CHECK;
                if (sockindex == ReceSocket)
                    OrderType = POP_ERR_CHECK;
#if DEBUG
                printf("ERROR: %02x\n", (UINT16)i);
#endif
            }
        }
    }
    if (sock_int_socket & SINT_STAT_CONNECT) /* Connection interrupt (TCP only) */
    {
#if DEBUG
        printf("Tcp Connect\n");
#endif
    }
    if (sock_int_socket & SINT_STAT_DISCONNECT) /* Disconnection interrupt (TCP only) */
    {
    }
    if (sock_int_socket & SINT_STAT_TIM_OUT) /* Timeout interrupt (TCP only) */
    {
#if DEBUG
        printf("Tcp Time out\n");
#endif
        tcptimeout_flag = 1;
        Delay_Ms(200); /* Delay 200ms before retrying to avoid frequent connections */
        i = CH395CMDOpenSocket(sockindex);
        mStopIfError(i);
        CH395CMDTCPConnect(sockindex); /* Start connection */
        mStopIfError(i);
    }
}

/*********************************************************************************
 * Function Name  : CH395GlobalInterrupt
 * Description    : CH395 global interrupt handler
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395GlobalInterrupt(void)
{
    UINT16 init_status;
    UINT16 i;
    UINT8 buf[10];

    init_status = CH395CMDGetGlobIntStatus();
    if (init_status & GINT_STAT_UNREACH) /* Unreachable interrupt, read info */
    {
        CH395CMDGetUnreachIPPT(buf);
    }
    if (init_status & GINT_STAT_IP_CONFLI) /* IP conflict, suggest updating IP and reinitializing */
    {
    }
    if (init_status & GINT_STAT_PHY_CHANGE) /* PHY change interrupt */
    {
#if DEBUG
        printf("Init status : GINT_STAT_PHY_CHANGE\n");
#endif
        i = CH395CMDGetPHYStatus(); /* Get PHY status */
        if (i == PHY_DISCONN)
        {
            phyDiscont_flag = 1;
#if DEBUG
            printf("Ethernet Disconnect\n"); /* If PHY_DISCONN, CH395 will automatically close all sockets */
#endif
        }
    }
    if (init_status & GINT_STAT_SOCK0)
    {
        CH395SocketInterrupt(0); /* Handle socket 0 interrupt */
    }
    if (init_status & GINT_STAT_SOCK1)
    {
        CH395SocketInterrupt(1); /* Handle socket 1 interrupt */
    }
    if (init_status & GINT_STAT_SOCK2)
    {
        CH395SocketInterrupt(2); /* Handle socket 2 interrupt */
    }
    if (init_status & GINT_STAT_SOCK3)
    {
        CH395SocketInterrupt(3); /* Handle socket 3 interrupt */
    }
    if (init_status & GINT_STAT_SOCK4)
    {
        CH395SocketInterrupt(4); /* Handle socket 4 interrupt */
    }
    if (init_status & GINT_STAT_SOCK5)
    {
        CH395SocketInterrupt(5); /* Handle socket 5 interrupt */
    }
    if (init_status & GINT_STAT_SOCK6)
    {
        CH395SocketInterrupt(6); /* Handle socket 6 interrupt */
    }
    if (init_status & GINT_STAT_SOCK7)
    {
        CH395SocketInterrupt(7); /* Handle socket 7 interrupt */
    }
}
/*********************************************************************************
 * Function Name  : CH395Init
 * Description    : Configure CH395 parameters such as IP, GWIP, MAC, and initialize
 * Input          : None
 * Output         : None
 * Return         : Function execution result
 **********************************************************************************/
UINT8 CH395Init(void)
{
    UINT8 i;
    UINT8 macaddr[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
#if (CH395_OP_INTERFACE_MODE == CH395_UART_MODE)
    CH395SetUartBaudRate(921600);
#endif
    i = CH395CMDCheckExist(0x65);
    printf("check ======%2x\n", (UINT16)i);
    if (i != 0x9a)
        return CH395_ERR_UNKNOW;            /* Test command, if it fails, return 0xFA */
                                            /* Return value 0xFA usually indicates hardware error or incorrect read/write timing */
    CH395CMDSetIPAddr(CH395Inf.IPAddr);     /* Set CH395 IP address */
    CH395CMDSetGWIPAddr(CH395Inf.GWIPAddr); /* Set gateway address */
    CH395CMDSetMASKAddr(CH395Inf.MASKAddr); /* Set subnet mask, default is 255.255.255.0 */
    CH395CMDSetMACAddr(macaddr);
    i = CH395CMDInitCH395(); /* Initialize CH395 chip */
    return i;
}

/*********************************************************************************
 * Function Name  : SendData
 * Description    : Send data
 * Input          : PSend    -  Data to be sent
                    Len      - Length of data to be sent
                    type     -  Check type
                    index    -  Socket index
 * Output         : None
 * Return         : Status
 **********************************************************************************/
UINT8 ch395mail_SendData(UINT8 *PSend, UINT16 Len, UINT8 type, UINT8 index)
{
    UINT16 count;
    CheckType = type;
    memset(MyBuffer[index], '\0', sizeof(MyBuffer[index]));
    CH395CMDSendData(index, PSend, Len);
    SendDatFlag = 0;
    phyDiscont_flag = 0;
    tcptimeout_flag = 0;
    count = 0;
    if (CheckType == uncheck)
    {
        while (SendDatFlag == 0)
        {
            if (Query395Interrupt() == 0)
                CH395GlobalInterrupt();
            if (phyDiscont_flag)
            {
                OrderType = COMMAND_UNUSEFULL;
                return 0;
            }
            if (tcptimeout_flag)
            {
                OrderType = COMMAND_UNUSEFULL;
                return 0;
            }
            Delay_Ms(1);
            count++;
            if (count > 10000)
            {
#if DEBUG
                printf("wait send Mail Header timeout\n");
#endif
                OrderType = SMTP_ERR_CHECK;
                return send_data_timeout;
            }
        }
        return send_data_success;
    }
    return send_data_success;
}

/*********************************************************************************
 * Function Name  : ch395mail
 * Description    : ch395mail
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void ch395mail(void)
{
    UINT8 i;
    CH395_PORT_INIT();
    printf("CH395EVT Test Demo\r\n");
    CH395Reset();
    Delay_Ms(100);
    i = CH395CMDGetVer();
    printf("CH395VER : %2x\r\n", (UINT16)i);

    InitCH395InfParam(); /* Initialize CH395Inf parameters */
    i = CH395Init();     /* Initialize the CH395 */
    mStopIfError(i);

    while (1)
    {                                              /* Wait for CH395 to connect to Ethernet */
        if (CH395CMDGetPHYStatus() == PHY_DISCONN) /* Example: query whether CH395 is connected */
        {
            Delay_Ms(200); /* If not, wait 200 ms and query again */
        }
        else
        {
            printf("CH395 Connect Ethernet\r\n"); /* When CH395 is connected to Ethernet, an interrupt occurs */
            break;
        }
    }
    InitSocketParam(); /* Initialize socket-related variables */
}
