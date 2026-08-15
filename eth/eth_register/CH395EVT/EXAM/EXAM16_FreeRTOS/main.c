/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "debug.h"
#include "CH395INC.h"
#include "CH395.H"
#include "FreeRTOS.h"
#include "semphr.h"
#include "portmacro.h"
#include "task.h"
#include "event_groups.h"

/* Global Variable */
TaskHandle_t CH395_init_Task_Handler;
TaskHandle_t CH395_InterruptPolling_Task_Handler;
TaskHandle_t CH395_Socket0_TCPclient_Loopback_Task_Handler;
TaskHandle_t CH395_Socket1_UDPServer_broadcast_Task_Handler;

SemaphoreHandle_t xCH395cmd_Mutex;
EventGroupHandle_t xCH395_Status;
EventGroupHandle_t xCH395_Socket_Status[8];

#define Event_CH395_Ready (1 << 0)
#define Event_CH395_Link_SUC (1 << 1)

/**********************************************************************************/
/* Include command file */
#include "CH395CMD.h"

/* Common variable definition */
UINT8 MyBuffer[2048];        /* data buffer */
struct _SOCK_INF SockInf[2]; /* Save the socket information */
struct _CH395_SYS CH395Inf;  /* Save the CH395 information */

/* CH395 Related definition */
const UINT8 CH395IPAddr[4] = {192, 168, 1, 101}; /* CH395 IP  */
const UINT8 CH395GWIPAddr[4] = {192, 168, 1, 1}; /* CH395 gateway */
const UINT8 CH395IPMask[4] = {255, 255, 255, 0}; /* CH395 mask */

/* Socket Related definition*/
const UINT8 Socket0DesIP[4] = {192, 168, 1, 123}; /* Socket 0 des IP */
const UINT16 Socket0SourPort = 1000;              /* Socket 0 sour port */
const UINT16 Socket0DesPort = 1000;               /* Socket 0 des port */

/* Socket Related definition*/
const UINT8 Socket1DesIP[4] = {255, 255, 255, 255}; /* Socket 0 des IP */
const UINT16 Socket1SourPort = 2000;                /* Socket 0 sour port */
const UINT16 Socket1DesPort = 2000;                 /* Socket 0 sour port */

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
        vTaskDelay(200);
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
 * Function Name  : CH395Init
 * Description    : Configure IP, GWIP, and MAC for the CH395, and initialize the CH395
 * Input          : None
 * Output         : None
 * Return         : Function execution result
 **********************************************************************************/
UINT8 CH395Init(void)
{
    UINT8 i;

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
 * Function Name  : InitSocketParam
 * Description    : Initialize Socket
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void InitSocket0Param(void)
{
    memset(&SockInf[0], 0, sizeof(SockInf[0]));                    /* Clear all SockInf to zero*/
    memcpy(SockInf[0].IPAddr, Socket0DesIP, sizeof(Socket0DesIP)); /* Enter the DESIP */
    SockInf[0].DesPort = Socket0DesPort;                           /* DesPort */
    SockInf[0].SourPort = Socket0SourPort;                         /* SourPort */
    SockInf[0].ProtoType = PROTO_TYPE_TCP;                         /* TCP mode*/
}

/**********************************************************************************
 * Function Name  : CH395SocketInitOpen
 * Description    : Set CH395 socket parameters to initialize and open the socket
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395Socket0InitOpen(void)
{
    UINT8 i;
    CH395CMDSetSocketDesIP(0, SockInf[0].IPAddr);       /* set socket 0 des IP */
    CH395CMDSetSocketProtType(0, SockInf[0].ProtoType); /* set socket 0 type */
    CH395CMDSetSocketDesPort(0, SockInf[0].DesPort);    /* set socket 0 des port */
    CH395CMDSetSocketSourPort(0, SockInf[0].SourPort);  /* set socket 0 sour port */
    i = CH395CMDOpenSocket(0);                          /* open socket 0 */
    mStopIfError(i);                                    /* check the result */
}

/**********************************************************************************
 * Function Name  : InitSocketParam
 * Description    : Initialize Socket
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void InitSocket1Param(void)
{
    memset(&SockInf[1], 0, sizeof(SockInf[1]));                    /* Clear all SockInf to zero*/
    memcpy(SockInf[1].IPAddr, Socket1DesIP, sizeof(Socket0DesIP)); /* Enter the DESIP */
    SockInf[1].SourPort = Socket1SourPort;                         /* SourPort */
    SockInf[1].DesPort = Socket1DesPort;                           /* DesPort */
    SockInf[1].ProtoType = PROTO_TYPE_UDP;                         /* UDP mode*/
}

/**********************************************************************************
 * Function Name  : CH395SocketInitOpen
 * Description    : Set CH395 socket parameters to initialize and open the socket
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395Socket1InitOpen(void)
{
    UINT8 i;

    CH395CMDSetSocketDesIP(1, SockInf[1].IPAddr);       /* set socket 1 des IP */
    CH395CMDSetSocketProtType(1, SockInf[1].ProtoType); /* set socket 1 type */
    CH395CMDSetSocketDesPort(1, SockInf[1].DesPort);    /* set socket 1 des port */
    CH395CMDSetSocketSourPort(1, SockInf[1].SourPort);  /* set socket 1 sour port */
    i = CH395CMDOpenSocket(1);                          /* open socket 1 */
    mStopIfError(i);                                    /* check the result */
}

/**********************************************************************************
 * Function Name  : CH395_GPIO_INIT
 * Description    : initialize GPIO
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395_GPIO_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 395Interrupt
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 395Rset
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

/**********************************************************************************
 * Function Name  : CH395_Init_Task
 * Description    : initialize CH395
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395_Init_Task()
{
    UINT8 i;
    CH395Reset();
    vTaskDelay(200);
#if (CH395_OP_INTERFACE_MODE == CH395_UART_MODE)
    CH395SetUartBaudRate(921600);
#endif
    printf("CH395 initialize start\r\n");
    i = CH395CMDGetVer();
    printf("CH395VER : %2x\r\n", (UINT16)i);
    InitCH395InfParam(); /* Initialize CH395Inf parameters */
    i = CH395Init();     /* Initialize the CH395 */
    mStopIfError(i);
    xEventGroupSetBits(xCH395_Status, Event_CH395_Ready);
    printf("CH395 initialize success\r\n");
    vTaskDelete(NULL);
}

/**********************************************************************************
 * Function Name  : CH395_InterruptPolling_Task
 * Description    : CH395 Global interrupt function
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395_InterruptPolling_Task()
{
    xEventGroupWaitBits(xCH395_Status, Event_CH395_Ready, pdFALSE, pdFALSE, portMAX_DELAY);
    while (1)
    {
        vTaskDelay(1);
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0)
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
                if (CH395CMDGetPHYStatus() != 1)
                {
                    printf("PHY Link Suc\r\n");
                    xEventGroupSetBits(xCH395_Status, Event_CH395_Link_SUC);
                }
                else
                {
                    printf("PHY Link Down\r\n");
                    xEventGroupClearBits(xCH395_Status, Event_CH395_Link_SUC);
                }
            }
            if (init_status & GINT_STAT_DHCP)
            {
            }
            if (init_status & GINT_STAT_SOCK0)
            {
                xEventGroupSetBits(xCH395_Socket_Status[0], CH395CMDGetSocketInt(0));
            }
            if (init_status & GINT_STAT_SOCK1)
            {
                xEventGroupSetBits(xCH395_Socket_Status[1], CH395CMDGetSocketInt(1));
            }
            if (init_status & GINT_STAT_SOCK2)
            {
                xEventGroupSetBits(xCH395_Socket_Status[2], CH395CMDGetSocketInt(2));
            }
            if (init_status & GINT_STAT_SOCK3)
            {
                xEventGroupSetBits(xCH395_Socket_Status[3], CH395CMDGetSocketInt(3));
            }
            if (init_status & GINT_STAT_SOCK4)
            {
                xEventGroupSetBits(xCH395_Socket_Status[4], CH395CMDGetSocketInt(4));
            }
            if (init_status & GINT_STAT_SOCK5)
            {
                xEventGroupSetBits(xCH395_Socket_Status[5], CH395CMDGetSocketInt(5));
            }
            if (init_status & GINT_STAT_SOCK6)
            {
                xEventGroupSetBits(xCH395_Socket_Status[6], CH395CMDGetSocketInt(6));
            }
            if (init_status & GINT_STAT_SOCK7)
            {
                xEventGroupSetBits(xCH395_Socket_Status[7], CH395CMDGetSocketInt(7));
            }
        }
    }
}

/**********************************************************************************
 * Function Name  : CH395_Socket0_TCPclient_Loopback_Task
 * Description    : initialize CH395
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395_Socket0_TCPclient_Loopback_Task()
{
    u8 i = 0;
    u16 totallen = 0, len = 0;
    // Wait for CH395 initialization to complete
    xEventGroupWaitBits(xCH395_Status, Event_CH395_Ready, pdFALSE, pdFALSE, portMAX_DELAY);
    printf("Socket0 create\n");
    InitSocket0Param();
    CH395Socket0InitOpen();

    xEventGroupWaitBits(xCH395_Status, Event_CH395_Link_SUC, pdFALSE, pdFALSE, portMAX_DELAY);
    i = CH395CMDTCPConnect(0);
    mStopIfError(i);
    printf("Socket0 create SUC\n");
    while (1)
    {
        xEventGroupWaitBits(xCH395_Socket_Status[0], SINT_STAT_RECV, pdTRUE, pdTRUE, portMAX_DELAY);
        totallen = CH395CMDGetRecvLength(0); /* Gets the length of data in the current buffer */
        printf("Socket0 receive\n");
        while (totallen > 0)
        {
            len = (totallen > SocketSendBufferSizes[0]) ? SocketSendBufferSizes[0] : totallen;
            totallen -= len;
            CH395CMDGetRecvData(0, len, MyBuffer);
            xEventGroupWaitBits(xCH395_Socket_Status[0], SINT_STAT_SENDBUF_FREE, pdTRUE, pdTRUE, portMAX_DELAY);
            CH395CMDSendData(0, MyBuffer, len);
            vTaskDelay(1);
        }
        vTaskDelay(1);
    }
}

/**********************************************************************************
 * Function Name  : CH395_Socket0_TCPclient_Loopback_Task
 * Description    : initialize CH395
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395_Socket1_UDPServer_broadcast_Task()
{
    // Wait for CH395 initialization to complete
    xEventGroupWaitBits(xCH395_Status, Event_CH395_Ready, pdFALSE, pdFALSE, portMAX_DELAY);
    printf("Socket1 create\n");
    InitSocket1Param();
    CH395Socket1InitOpen();
    printf("Socket1 create SUC\n");
    while (1)
    {
        vTaskDelay(500);
        xEventGroupWaitBits(xCH395_Socket_Status[1], SINT_STAT_SENDBUF_FREE, pdTRUE, pdTRUE, portMAX_DELAY);
        printf("Socket1 send\n");
        CH395CMDSendData(1, MyBuffer, 100);
    }
}

/*******************************************************************************
 * Function Name  : main
 * Description    : Main program.
 * Input          : None
 * Return         : None
 *******************************************************************************/
int main(void)
{
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("FreeRTOS Kernel Version:%s\r\n", tskKERNEL_VERSION_NUMBER);

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
    CH395_GPIO_INIT();

    xCH395cmd_Mutex = xSemaphoreCreateMutex();
    xCH395_Status = xEventGroupCreate();
    for (u8 val = 0; val < 8; val++)
    {
        xCH395_Socket_Status[val] = xEventGroupCreate();
        /*
        The socket interrupt value should be set to SINT_STAT_SENDBUF_FREE during initialization,
        as there will be no send buffer idle interrupt before any data packet is sent.
        */
        xEventGroupSetBits(xCH395_Socket_Status[val], SINT_STAT_SENDBUF_FREE);
    }
    /* create two task */
    xTaskCreate((TaskFunction_t)CH395_Init_Task,
                (const char *)"CH395_Init",
                (uint16_t)128,
                (void *)NULL,
                (UBaseType_t)6,
                (TaskHandle_t *)&CH395_init_Task_Handler);

    xTaskCreate((TaskFunction_t)CH395_InterruptPolling_Task,
                (const char *)"CH395_InterruptPolling",
                (uint16_t)128,
                (void *)NULL,
                (UBaseType_t)1,
                (TaskHandle_t *)&CH395_InterruptPolling_Task_Handler);

    xTaskCreate((TaskFunction_t)CH395_Socket0_TCPclient_Loopback_Task,
                (const char *)"CH395_Socket0_TCPclient_Loopback",
                (uint16_t)128,
                (void *)NULL,
                (UBaseType_t)4,
                (TaskHandle_t *)&CH395_Socket0_TCPclient_Loopback_Task_Handler);

    xTaskCreate((TaskFunction_t)CH395_Socket1_UDPServer_broadcast_Task,
                (const char *)"CH395_Socket1_UDPServer_broadcast",
                (uint16_t)128,
                (void *)NULL,
                (UBaseType_t)3,
                (TaskHandle_t *)&CH395_Socket1_UDPServer_broadcast_Task_Handler);

    vTaskStartScheduler();

    while (1)
    {
        printf("shouldn't run at here!!\n");
    }
}
