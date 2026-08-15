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

#define CH395_DHCP 1

/* Common variable definition */
UINT8 MyBuffer[1][2048]; /* data buffer */

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
UINT16 Recv_len[1];

struct _SOCK_INF SockInf[1]; /* Save the socket information */
struct _CH395_SYS CH395Inf;  /* Save the CH395 information */

/* CH395 Related definition */
const UINT8 CH395IPAddr[4] = {192, 168, 1, 101}; /* CH395 IP  */
const UINT8 CH395GWIPAddr[4] = {192, 168, 1, 1}; /* CH395 gateway */
const UINT8 CH395IPMask[4] = {255, 255, 255, 0}; /* CH395 mask */

/* Socket 0 is UDP broadcast mode */
const UINT8 Socket0DesIP[4] = {255, 255, 255, 255};
const UINT16 Socket0DesPort = 1000;
const UINT16 Socket0SourPort = 5000;

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

UINT8 flag = 0; /* DHCP success flag */

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
    CH395CMDCloseSocket(0);
    /* Socket 0 in UDP mode */
    CH395CMDSetSocketDesIP(0, SockInf[0].IPAddr);       /* Set destination IP address for socket 0 */
    CH395CMDSetSocketProtType(0, SockInf[0].ProtoType); /* Set protocol type for socket 0 */
    CH395CMDSetSocketDesPort(0, SockInf[0].DesPort);    /* Set destination port for socket 0 */
    CH395CMDSetSocketSourPort(0, SockInf[0].SourPort);  /* Set source port for socket 0 */
    i = CH395CMDOpenSocket(0);                          /* Open socket 0 */
    mStopIfError(i);
}

/**********************************************************************************
 * Function Name  : UDP_Server_Data_Loop
 * Description    : Receive data from socket
 * Input          : socket index
 * Output         : None
 * Return         : None
 * *********************************************************************************/
void UDP_Server_Data_Loop(UINT8 sockindex)
{
    u16 port;
    if (Recv_len[sockindex] == 0)
    {
        if (sock_int_status[sockindex] & SINT_STAT_RECV)
        {
            sock_int_status[sockindex] &= ~SINT_STAT_RECV;

            Recv_len[sockindex] = CH395CMDGetRecvLength(sockindex); /* Gets the length of data in the current buffer */
            CH395CMDGetRecvData(sockindex, Recv_len[sockindex], MyBuffer[sockindex]);
            port = ((UINT16)MyBuffer[sockindex][3] << 8) + MyBuffer[sockindex][2];
            printf("Socket %d receive len : %d\r\n", sockindex, Recv_len[sockindex]);
            printf("Port = %d\n", port);
            printf("IP = %2d.%2d.%2d.%2d.\n", (UINT16)MyBuffer[sockindex][4], (UINT16)MyBuffer[sockindex][5], (UINT16)MyBuffer[sockindex][6], (UINT16)MyBuffer[sockindex][7]);
        }
    }
    else
    {
        if (sock_int_status[sockindex] & SINT_STAT_SENDBUF_FREE)
        {
            sock_int_status[sockindex] &= ~SINT_STAT_SENDBUF_FREE;
            port = ((UINT16)MyBuffer[sockindex][3] << 8) + MyBuffer[sockindex][2];
            CH395UDPSendTo(&MyBuffer[sockindex][8], (Recv_len[sockindex] - 8), &MyBuffer[sockindex][4], port, sockindex);
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
    UINT16 i;
    UINT8 buf[20];

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
#if CH395_DHCP
        i = CH395CMDGetDHCPStatus();

        if (i == 0)
        {
            flag = 1;
            CH395CMDGetIPInf(buf);

            printf("IP:%02d.%02d.%02d.%02d\n", (UINT16)buf[0], (UINT16)buf[1], (UINT16)buf[2], (UINT16)buf[3]);
            printf("GWIP:%02d.%02d.%02d.%02d\n", (UINT16)buf[4], (UINT16)buf[5], (UINT16)buf[6], (UINT16)buf[7]);
            printf("Mask:%02d.%02d.%02d.%02d\n", (UINT16)buf[8], (UINT16)buf[9], (UINT16)buf[10], (UINT16)buf[11]);
            printf("DNS1:%02d.%02d.%02d.%02d\n", (UINT16)buf[12], (UINT16)buf[13], (UINT16)buf[14], (UINT16)buf[15]);
            printf("DNS2:%02d.%02d.%02d.%02d\n", (UINT16)buf[16], (UINT16)buf[17], (UINT16)buf[18], (UINT16)buf[19]);
        }
#endif
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
        return CH395_ERR_UNKNOW; /* Test cmd , return 0XFA if it fails*/
                                 /* The value 0XFA is usually a hardware error or incorrect read/write timing */

#if !CH395_DHCP                             /* If DHCP is enabled, the IP, gateway, and subnet mask do not need to be set */
    CH395CMDSetIPAddr(CH395Inf.IPAddr);     /* Set the CH395's IP address */
    CH395CMDSetGWIPAddr(CH395Inf.GWIPAddr); /* Set the gateway address */
    CH395CMDSetMASKAddr(CH395Inf.MASKAddr); /* Set the subnet mask, default is 255.255.255.0 */
#endif
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

#if CH395_DHCP
    printf("Start DHCP\n");
    i = CH395CMDDHCPEnable(1);
    mStopIfError(i);
#endif

    while (1)
    {
        if (Query395Interrupt() == 0)
        {
            CH395GlobalInterrupt();
        }
        if (flag)
        {
            flag = 0;
            InitSocketParam();     /* Initialize socket related variables */
            CH395SocketInitOpen(); /* Wait for DHCP to succeed before initializing the socket */
        }
        UDP_Server_Data_Loop(0);
    }
}
