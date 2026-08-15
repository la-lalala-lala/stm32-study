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
#include "HTTPS.H"

#define Query395Interrupt() GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8)

/**********************************************************************************/
/* Include command file */
#include "CH395CMD.h"

st_http_request *http_request;
extern UINT8 WebServer_flag;
extern UINT16 WebServer_len;
extern UINT8 WebServer_RecvBuffer[4096];
extern UINT16 WebServer_SendLen;
extern UINT8 *WebServer_Send_point;

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

struct _SOCK_INF SockInf[1]; /* Save the socket information */
struct _CH395_SYS CH395Inf;  /* Save the CH395 information */

/* CH395 Related definition */
UINT8 CH395IPAddr[4] = {192, 168, 1, 101};       /* CH395 IP  */
const UINT8 CH395GWIPAddr[4] = {192, 168, 1, 1}; /* CH395 gateway */
const UINT8 CH395IPMask[4] = {255, 255, 255, 0}; /* CH395 mask */

UINT16 Socket0SourPort = 80;

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

extern UINT8 WebServer_handle_step;

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
    memset(&SockInf[0], 0, sizeof(SockInf[0])); /* Clear all fields of SockInf[0] */
    SockInf[0].SourPort = Socket0SourPort;      /* Source port */
    SockInf[0].ProtoType = PROTO_TYPE_TCP;      /* TCP mode */
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

    /* Socket 0 in TCP Server mode, no need to set destination IP and source port */
    CH395CMDSetSocketProtType(0, SockInf[0].ProtoType); /* Set protocol type for socket 0 */
    CH395CMDSetSocketSourPort(0, SockInf[0].SourPort);  /* Set source port for socket 0 */
    i = CH395CMDOpenSocket(0);                          /* Open socket 0 */
    mStopIfError(i);                                    /* Check if successful */
    i = CH395CMDTCPListen(0);                           /* Start listening */
    mStopIfError(i);                                    /* Check if successful */
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
        UINT8 buf[10];
        sock_int_status[sockindex] &= ~SINT_STAT_CONNECT;
        printf("SINT_STAT_CONNECT\r\n");
        CH395CMDGetRemoteIPP(sockindex, buf);
        printf("IP address = %d.%d.%d.%d\n", (UINT16)buf[0], (UINT16)buf[1], (UINT16)buf[2], (UINT16)buf[3]);
    }
    if (sock_int_status[sockindex] & SINT_STAT_DISCONNECT) /* Disconnect interrupt, valid only in TCP mode */
    {
        /*Socket 0 as Web Server, handles the disconnection in the main program*/
        if (sockindex != 0)
        {
            sock_int_status[sockindex] &= ~SINT_STAT_DISCONNECT;
        }
        printf("SINT_STAT_DISCONNECT \r\n");
    }
    if (sock_int_status[sockindex] & SINT_STAT_TIM_OUT) /* Timeout interrupt, valid only in TCP mode */
    {
        sock_int_status[sockindex] &= ~SINT_STAT_TIM_OUT;
        printf("SINT_STAT_TIM_OUT\r\n");
    }
}

/**********************************************************************************
 * Function Name  : WedServer_Socket_handle
 * Description    : handle WedServer socket
 * Input          : None
 * Output         : None
 * Return         : None
 * *********************************************************************************/
void WedServer_Socket_handle(void)
{
    UINT8 s;
    if (sock_int_status[0] & SINT_STAT_CONNECT)
    {
        UINT8 buf[6];
        sock_int_status[0] &= ~SINT_STAT_CONNECT;
        printf("Tcp Connect\n");
        CH395CMDGetRemoteIPP(0, buf);
        printf("IP address = %d.%d.%d.%d\n", (UINT16)buf[0], (UINT16)buf[1], (UINT16)buf[2], (UINT16)buf[3]);
    }
    if (WebServer_handle_step == WEB_SERVER_STEP_IDLE)
    {
        if ((WebServer_flag & RECVDATA) == 0)
        {
            if (sock_int_status[0] & SINT_STAT_RECV)
            {
                sock_int_status[0] &= ~SINT_STAT_RECV;
                WebServer_len = CH395CMDGetRecvLength(0); /* Gets the length of the data in the current buffer */
                if (WebServer_len == 0)
                    return;
                CH395CMDGetRecvData(0, WebServer_len, WebServer_RecvBuffer); /* Read data */
                WebServer_flag |= RECVDATA;
            }
        }
    }
    if (WebServer_flag & SENDDATA)
    {
        if (sock_int_status[0] & SINT_STAT_SENDBUF_FREE)
        {
            if (WebServer_SendLen > 0)
            {
                sock_int_status[0] &= ~SINT_STAT_SENDBUF_FREE;
                if (WebServer_SendLen > SocketSendBufferSizes[0]) /* Calculates the length that can be sent */
                {
                    CH395CMDSendData(0, WebServer_Send_point, SocketSendBufferSizes[0]);
                    WebServer_SendLen -= SocketSendBufferSizes[0];
                    WebServer_Send_point += SocketSendBufferSizes[0];
                }
                else
                {
                    CH395CMDSendData(0, WebServer_Send_point, WebServer_SendLen);
                    WebServer_SendLen = 0;
                    WebServer_flag &= ~SENDDATA;
                }
            }
        }
    }
    if (sock_int_status[0] & SINT_STAT_DISCONNECT)
    {
        sock_int_status[0] &= ~SINT_STAT_DISCONNECT;
        s = CH395CMDOpenSocket(0);
        mStopIfError(s);
        s = CH395CMDTCPListen(0);
        mStopIfError(s);
    }
    if (sock_int_status[0] & SINT_STAT_TIM_OUT)
    {
        sock_int_status[0] &= ~SINT_STAT_TIM_OUT;
        printf("Tcp Time out\n");
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
        sock_int_status[0] |= CH395CMDGetSocketInt(0);
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

    /*LED IO*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_11);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_12);
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
    st_http_request http_request_data;
    http_request = &http_request_data;
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

restart:
    CH395_PORT_INIT();
    CH395_GPIO_INIT();
    printf("CH395EVT Test Demo\r\n");

    CH395Reset();
    Delay_Ms(100);
    i = CH395CMDGetVer();
    printf("CH395VER : %2x\r\n", (UINT16)i);

    InitCH395InfParam(); /* Initialize CH395Inf parameters */
    i = CH395Init();     /* Initialize the CH395 */
    mStopIfError(i);

    WebServer_handle_step = WEB_SERVER_STEP_IDLE;
    sock_int_status[0] = SINT_STAT_SENDBUF_FREE;

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
        WedServer_Socket_handle();
        WebServer();
        if ((WebServer_flag & IPCHANGE) | (WebServer_flag & PORTCHANGE)) /* If the IP or Port changes, the CH395 is reset and reinitialized */
        {
            WebServer_flag &= ~(IPCHANGE | PORTCHANGE);
            printf("reset all!\n");
            CH395CMDReset();
            Delay_Ms(100);
            printf("restart\n");
            goto restart;
        }
    }
}
