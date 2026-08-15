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
UINT8 MyBuffer[4][2048]; /* data buffer */

/*By default, each of the first four sockets is allocated 4 buffer blocks (2048 bytes each) for the send buffer.
If needed, the receive and send buffers can be reallocated using the CMD_SET_RECV_BUF and CMD_SET_SEND_BUF commands.*/
UINT16 SocketSendBufferSizes[8] = {
    2048,
    2048,
    2048,
    2048,
    0,
    0,
    0,
    0,
};
UINT16 Recv_len[4];

struct _SOCK_INF SockInf[4]; /* Save the socket information */
struct _CH395_SYS CH395Inf;  /* Save the CH395 information */

/* CH395 Related definition */
const UINT8 CH395IPAddr[4] = {192, 168, 1, 101}; /* CH395 IP  */
const UINT8 CH395GWIPAddr[4] = {192, 168, 1, 1}; /* CH395 gateway */
const UINT8 CH395IPMask[4] = {255, 255, 255, 0}; /* CH395 mask */

/* Socket 0 is UDP unicast, Socket 1 is a TCP server, Socket 2 is a TCP client, and Socket 3 is in IP RAW mode.*/
const UINT8 Socket0DesIP[4] = {192, 168, 1, 123};
const UINT16 Socket0DesPort = 1000;
const UINT16 Socket0SourPort = 5555;

const UINT16 Socket1SourPort = 6000;

const UINT8 Socket2DesIP[4] = {192, 168, 1, 123};
const UINT16 Socket2DesPort = 1000;
const UINT16 Socket2SourPort = 7000;

const UINT8 Socket3DesIP[4] = {192, 168, 1, 123};
const UINT8 IPRawProto = 0xE2;
/* IP packet protocol type */

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
    memset(&SockInf[0], 0, sizeof(SockInf[0]));                    /* Clear all fields of SockInf[0] */
    memcpy(SockInf[0].IPAddr, Socket0DesIP, sizeof(Socket0DesIP)); /* Write destination IP address */
    SockInf[0].DesPort = Socket0DesPort;                           /* Destination port */
    SockInf[0].SourPort = Socket0SourPort;                         /* Source port */
    SockInf[0].ProtoType = PROTO_TYPE_UDP;                         /* UDP mode */

    memset(&SockInf[1], 0, sizeof(SockInf[1])); /* Clear all fields of SockInf[1] */
    SockInf[1].SourPort = Socket1SourPort;      /* Source port */
    SockInf[1].ProtoType = PROTO_TYPE_TCP;      /* TCP mode */

    memset(&SockInf[2], 0, sizeof(SockInf[2]));                    /* Clear all fields of SockInf[2] */
    memcpy(SockInf[2].IPAddr, Socket2DesIP, sizeof(Socket2DesIP)); /* Write destination IP address */
    SockInf[2].DesPort = Socket2DesPort;                           /* Destination port */
    SockInf[2].SourPort = Socket2SourPort;                         /* Source port */
    SockInf[2].ProtoType = PROTO_TYPE_TCP;                         /* TCP mode */

    memset(&SockInf[3], 0, sizeof(SockInf[3]));                    /* Clear all fields of SockInf[3] */
    memcpy(SockInf[3].IPAddr, Socket3DesIP, sizeof(Socket3DesIP)); /* Write destination IP address */
    SockInf[3].ProtoType = PROTO_TYPE_IPRAW;                       /* IP RAW mode */
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

    /* Socket 0 in UDP mode */
    CH395CMDSetSocketDesIP(0, SockInf[0].IPAddr);       /* Set destination IP address for socket 0 */
    CH395CMDSetSocketProtType(0, SockInf[0].ProtoType); /* Set protocol type for socket 0 */
    CH395CMDSetSocketDesPort(0, SockInf[0].DesPort);    /* Set destination port for socket 0 */
    CH395CMDSetSocketSourPort(0, SockInf[0].SourPort);  /* Set source port for socket 0 */
    i = CH395CMDOpenSocket(0);                          /* Open socket 0 */
    mStopIfError(i);

    /* Socket 1 in TCP Server mode, no need to set destination IP and source port */
    CH395CMDSetSocketProtType(1, SockInf[1].ProtoType); /* Set protocol type for socket 1 */
    CH395CMDSetSocketSourPort(1, SockInf[1].SourPort);  /* Set source port for socket 1 */
    i = CH395CMDOpenSocket(1);                          /* Open socket 1 */
    mStopIfError(i);                                    /* Check if successful */
    i = CH395CMDTCPListen(1);                           /* Start listening */
    mStopIfError(i);                                    /* Check if successful */

    /* Socket 2 in TCP Client mode */
    CH395CMDSetSocketDesIP(2, SockInf[2].IPAddr);       /* Set destination IP address for socket 2 */
    CH395CMDSetSocketProtType(2, SockInf[2].ProtoType); /* Set protocol type for socket 2 */
    CH395CMDSetSocketDesPort(2, SockInf[2].DesPort);    /* Set destination port for socket 2 */
    CH395CMDSetSocketSourPort(2, SockInf[2].SourPort);  /* Set source port for socket 2 */
    i = CH395CMDOpenSocket(2);                          /* Open socket 2 */
    mStopIfError(i);                                    /* Check if successful */
    i = CH395CMDTCPConnect(2);                          /* Start connection */
    mStopIfError(i);                                    /* Check if successful; if failed, the socket will be closed */

    /* Socket 3 in IP RAW mode */
    CH395CMDSetSocketDesIP(3, SockInf[3].IPAddr);       /* Set destination IP address for socket 3 */
    CH395CMDSetSocketProtType(3, SockInf[3].ProtoType); /* Set the IP packet protocol field in IP RAW mode */
    CH395CMDSetSocketIPRAWProto(3, IPRawProto);         /* Set protocol field */
    i = CH395CMDOpenSocket(3);                          /* Open socket 3 */
    mStopIfError(i);                                    /* Check if successful */
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
    i = CH395CMDInitCH395();                /* initialize the CH395 */
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

    InitCH395InfParam(); /* Initialize CH395Inf parameters */
    i = CH395Init();     /* Initialize the CH395 */
    mStopIfError(i);

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
        Data_Loop(0);
        Data_Loop(1);
        Data_Loop(2);
        Data_Loop(3);
    }
}
