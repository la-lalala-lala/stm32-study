/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : Main program body.
 *******************************************************************************/
#include "debug.h"
#include "CH395INC.h"
#include "CH395.H"

#define Query395Interrupt() GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8)

/**********************************************************************************/
/* Include command file */
#include "CH395CMD.h"

/* Common variable definition */
UINT8 MyBuffer[8][1024]; /* data buffer */

/*By default, each of the first four sockets is allocated 4 buffer blocks (2048 bytes each) for the send buffer.
If needed, the receive and send buffers can be reallocated using the CMD_SET_RECV_BUF and CMD_SET_SEND_BUF commands.*/
UINT16 SocketSendBufferSizes[8] = {
    1024,
    1024,
    1024,
    1024,
    1024,
    1024,
    1024,
    1024,
};
UINT16 Recv_len[8];

struct _SOCK_INF SockInf[8]; /* Save the socket information */
struct _CH395_SYS CH395Inf;  /* Save the CH395 information */

/* CH395 Related definition */
const UINT8 CH395IPAddr[4] = {192, 168, 1, 101}; /* CH395 IP  */
const UINT8 CH395GWIPAddr[4] = {192, 168, 1, 1}; /* CH395 gateway */
const UINT8 CH395IPMask[4] = {255, 255, 255, 0}; /* CH395 mask */

/* Socket definitions */
const UINT16 Socket0SourPort = 5000; /* Socket 0 Source port */

/*
If using the SINT_STAT_SENDBUF_FREE interrupt as the condition for determining whether to send a data packet,
the socket interrupt value can be set to SINT_STAT_SENDBUF_FREE during initialization,
as there will be no send buffer idle interrupt before any data packet is sent.
*/
UINT8 sock_int_status[8] = {
    SINT_STAT_SENDBUF_FREE,
    SINT_STAT_SENDBUF_FREE,
    SINT_STAT_SENDBUF_FREE,
    SINT_STAT_SENDBUF_FREE,
    SINT_STAT_SENDBUF_FREE,
    SINT_STAT_SENDBUF_FREE,
    SINT_STAT_SENDBUF_FREE,
    SINT_STAT_SENDBUF_FREE,
};

/**********************************************************************************
 * Function Name  : mStopIfError
 * Description    : Display error code, and stop
 * Input          : iError
 * Output         : None
 * Return         : None
 **********************************************************************************/
void mStopIfError(UINT8 iError)
{
    if (iError == CMD_ERR_SUCCESS)
        return;                                /* Success */
    printf("Error: %02X\r\n", (UINT16)iError); /* printf error */
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
    memset(&CH395Inf, 0, sizeof(CH395Inf));                          /* Clear all CH395Inf to zero */
    memcpy(CH395Inf.IPAddr, CH395IPAddr, sizeof(CH395IPAddr));       /* Enter the IP address in the CH395Inf file */
    memcpy(CH395Inf.GWIPAddr, CH395GWIPAddr, sizeof(CH395GWIPAddr)); /* Enter the gateway IP address in the CH395Inf file */
    memcpy(CH395Inf.MASKAddr, CH395IPMask, sizeof(CH395IPMask));     /* Enter the mask IP address in the CH395Inf file */
}

/**********************************************************************************
 * Function Name  : InitSocketParam
 * Description    : Initialize Socket
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void InitSocketParam(void)
{
    memset(&SockInf[0], 0, sizeof(SockInf[0])); /* Clear all bytes of SockInf to zero */
    SockInf[0].SourPort = Socket0SourPort;      /* Set the source port */
    SockInf[0].ProtoType = PROTO_TYPE_TCP6;     /* Set to TCP mode */
}

/**********************************************************************************
 * Function Name  : CH395SocketInitOpen
 * Description    : Set CH395 socket parameters to initialize and open the socket
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395SocketInitOpen(void)
{
    UINT8 i;

    /* Socket 0 is in TCP server mode */
    CH395CMDSetSocketProtType(0, SockInf[0].ProtoType); /* Set the protocol type for socket 0 */
    CH395CMDSetSocketSourPort(0, SockInf[0].SourPort);  /* Set the source port for socket 0 */
    i = CH395CMDOpenSocket(0);                          /* Open socket 0 */
    mStopIfError(i);                                    /* Check if the operation was successful */
    i = CH395CMDTCPListen(0);                           /* TCP listening */
    mStopIfError(i);

    for (i = 1; i < 8; i++)
    {
        CH395CMDSetSocketProtType(i, SockInf[0].ProtoType); /* Set the protocol type for socket i */
        CH395CMDSetSocketSourPort(i, SockInf[0].SourPort);  /* Set the source port for socket i */
    }
}

/**********************************************************************************
 * Function Name  : Data_Loop
 * Description    : Receive data from socket
 * Input          : socket index
 * Output         : None
 * Return         : None
 * *********************************************************************************/
void Data_Loop(UINT8 sockindex)
{
    if (Recv_len[sockindex] == 0)
    {
        if (sock_int_status[sockindex] & SINT_STAT_RECV)
        {
            sock_int_status[sockindex] &= ~SINT_STAT_RECV;

            Recv_len[sockindex] = CH395CMDGetRecvLength(sockindex); /* Gets the length of data in the current buffer */

            /*By default, each of the first four sockets is allocated 4 buffer blocks (2048 bytes each) for the send buffer.
            If needed, the receive and send buffers can be reallocated using the CMD_SET_RECV_BUF and CMD_SET_SEND_BUF commands.*/
            if (Recv_len[sockindex] > SocketSendBufferSizes[sockindex])
            {
                Recv_len[sockindex] = SocketSendBufferSizes[sockindex];
            }
            printf("Socket %d receive len : %d\r\n", sockindex, Recv_len[sockindex]);
            CH395CMDGetRecvData(sockindex, Recv_len[sockindex], MyBuffer[sockindex]);
        }
    }
    else
    {
        if (sock_int_status[sockindex] & SINT_STAT_SENDBUF_FREE)
        {
            sock_int_status[sockindex] &= ~SINT_STAT_SENDBUF_FREE;
            CH395CMDSendData(sockindex, MyBuffer[sockindex], Recv_len[sockindex]);
            Recv_len[sockindex] = 0;
        }
    }
}

/**********************************************************************************
 * Function Name  : CH395SocketInterrupt
 * Description    : CH395 socket interrupt, called in global interrupt
 * Input          : sockindex
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395SocketInterrupt(UINT8 sockindex)
{
    sock_int_status[sockindex] |= CH395CMDGetSocketInt(sockindex); /* Gets the socket interrupt status */

    if (sock_int_status[sockindex] & SINT_STAT_RECV) /* Receive interruption */
    {
        // Handle it in the main program
    }
    if (sock_int_status[sockindex] & SINT_STAT_SENDBUF_FREE) /* The send buffer is free and can continue writing data to be sent */
    {
        // Handle it in the main program
    }
    if (sock_int_status[sockindex] & SINT_STAT_SEND_OK) /* Send completion interrupt */
    {
        sock_int_status[sockindex] &= ~SINT_STAT_SEND_OK;
    }
    if (sock_int_status[sockindex] & SINT_STAT_CONNECT) /* The connection is interrupted, valid only in TCP mode */
    {
        sock_int_status[sockindex] &= ~SINT_STAT_CONNECT;
        CH395CMDSetKeepLive(sockindex, 1); /* Enable the KEEPALIVE timer */
        printf("SINT_STAT_CONNECT\r\n");
    }
    if (sock_int_status[sockindex] & SINT_STAT_DISCONNECT) /* Disconnect interrupt, valid only in TCP mode */
    {
        sock_int_status[sockindex] &= ~SINT_STAT_DISCONNECT;
        printf("SINT_STAT_DISCONNECT \r\n");
    }
    if (sock_int_status[sockindex] & SINT_STAT_TIM_OUT) /* Timeout interrupt, valid only in TCP mode */
    {
        sock_int_status[sockindex] &= ~SINT_STAT_TIM_OUT;
        printf("SINT_STAT_TIM_OUT\r\n");
    }
}

/**********************************************************************************
 * Function Name  : CH395GlobalInterrupt
 * Description    : CH395 Global interrupt function
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395GlobalInterrupt(void)
{
    UINT16 init_status;

    init_status = CH395CMDGetGlobIntStatus();

    if (init_status & GINT_STAT_UNREACH) /* Unreachable interrupt,to read unreachable information */
    {
    }
    if (init_status & GINT_STAT_IP_CONFLI) /* IP address conflict interrupted,advice to change the IP address of the CH395 again and initialize the CH395 */
    {
    }
    if (init_status & GINT_STAT_PHY_CHANGE) /* PHY change interrupt */
    {
        printf("Init status : GINT_STAT_PHY_CHANGE\r\n");
    }
    if (init_status & GINT_STAT_DHCP)
    {
    }
    if (init_status & GINT_STAT_SOCK0)
    {
        CH395SocketInterrupt(0); /* socket 0 interrupt */
    }
    if (init_status & GINT_STAT_SOCK1)
    {
        CH395SocketInterrupt(1); /* socket 1 interrupt */
    }
    if (init_status & GINT_STAT_SOCK2)
    {
        CH395SocketInterrupt(2); /* socket 2 interrupt */
    }
    if (init_status & GINT_STAT_SOCK3)
    {
        CH395SocketInterrupt(3); /* socket 3 interrupt */
    }
    if (init_status & GINT_STAT_SOCK4)
    {
        CH395SocketInterrupt(4); /* socket 4 interrupt */
    }
    if (init_status & GINT_STAT_SOCK5)
    {
        CH395SocketInterrupt(5); /* socket 5 interrupt */
    }
    if (init_status & GINT_STAT_SOCK6)
    {
        CH395SocketInterrupt(6); /* socket 6 interrupt */
    }
    if (init_status & GINT_STAT_SOCK7)
    {
        CH395SocketInterrupt(7); /* socket 7 interrupt */
    }
    if (init_status & GINT_STAT_DHCPV6)
    {
    }
}

/**********************************************************************************
 * Function Name  : CH395Init
 * Description    : Configure IP, GWIP, and MAC for the CH395, and initialize the CH395
 * Input          : None
 * Output         : None
 * Return         : Function execution result
 **********************************************************************************/
UINT8 CH395Init(void)
{
    UINT8 i;
#if (CH395_OP_INTERFACE_MODE == CH395_UART_MODE)
    CH395SetUartBaudRate(921600);
#endif

    i = CH395CMDCheckExist(0x65);
    if (i != 0x9a)
        return CH395_ERR_UNKNOW;            /* Test cmd , return 0XFA if it fails*/
                                            /* The value 0XFA is usually a hardware error or incorrect read/write timing */
    CH395CMDSetIPAddr(CH395Inf.IPAddr);     /* Set CH395 IP */
    CH395CMDSetGWIPAddr(CH395Inf.GWIPAddr); /* Set GW IP */
    CH395CMDSetMASKAddr(CH395Inf.MASKAddr); /* Set MASK */
    CH395CMDSetFunPapr(0x02, 0, 0, 0);
    i = CH395CMDInitCH395(); /* initialize the CH395 */
    return i;
}

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

/*********************************************************************************
 * Function Name  : Socket_R_S_Buf_Modify
 * Description    : Defines the socket receive and send buffers
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void Socket_R_S_Buf_Modify(void)
{

    CH395CMDSetSocketRecvBuf(0, 0, 4); /* Socket 0 ，Receive buffer: 4 * 512 = 2K，Send buffer: 2 * 512 = 1K*/
    CH395CMDSetSocketSendBuf(0, 4, 2);

    CH395CMDSetSocketRecvBuf(1, 6, 4); /* Socket 1 */
    CH395CMDSetSocketSendBuf(1, 10, 2);

    CH395CMDSetSocketRecvBuf(2, 12, 4); /* Socket 2 */
    CH395CMDSetSocketSendBuf(2, 16, 2);

    CH395CMDSetSocketRecvBuf(3, 18, 4); /* Socket 3 */
    CH395CMDSetSocketSendBuf(3, 22, 2);

    CH395CMDSetSocketRecvBuf(4, 24, 4); /* Socket 4 */
    CH395CMDSetSocketSendBuf(4, 28, 2);

    CH395CMDSetSocketRecvBuf(5, 30, 4); /* Socket 5 */
    CH395CMDSetSocketSendBuf(5, 34, 2);

    CH395CMDSetSocketRecvBuf(6, 36, 4); /* Socket 6 */
    CH395CMDSetSocketSendBuf(6, 40, 2);

    CH395CMDSetSocketRecvBuf(7, 42, 4); /* Socket 7 */
    CH395CMDSetSocketSendBuf(7, 46, 2);
}

/**********************************************************************************
 * Function Name  : main
 * Description    : main
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
int main(void)
{
    UINT8 i;
    UINT8 MyIPv6Addr[32];
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

    CH395CMDIPv6AutoLLA(); /* Enable IPv6 Auto LLA address */
    InitCH395InfParam();   /* Initialize CH395Inf parameters */
    i = CH395Init();       /* Initialize the CH395 */
    mStopIfError(i);
    CH395CMDGetIPv6Addr(MyIPv6Addr); /* Get the IPv6 LLA address */
    printf("CH395 IPv6 LLA Address : ");
    for (i = 0; i < 8; i++)
    {
        printf("%02X%02X", MyIPv6Addr[i * 2], MyIPv6Addr[i * 2 + 1]);
        if (i != 7)
            printf(":");
    }
    printf("\r\n");

    Socket_R_S_Buf_Modify(); /* Define the socket send and receive buffers */

    while (1)
    {                                              /* Wait for the CH395 Connect Ethernet*/
        if (CH395CMDGetPHYStatus() == PHY_DISCONN) /* Example Query whether the CH395 is connected */
        {
            Delay_Ms(200); /* If no, wait for 200MS and query again */
        }
        else
        {
            printf("CH395 Connect Ethernet\r\n"); /*When the CH395 is connected to the Ethernet, an interruption occurs */
            break;
        }
    }
    InitSocketParam(); /* Initialize socket related variables */
    CH395SocketInitOpen();

    while (1)
    {
        if (Query395Interrupt() == 0)
        {
            CH395GlobalInterrupt();
        }
        Data_Loop(1);
        Data_Loop(2);
        Data_Loop(3);
        Data_Loop(4);
        Data_Loop(5);
        Data_Loop(6);
        Data_Loop(7);
    }
}
