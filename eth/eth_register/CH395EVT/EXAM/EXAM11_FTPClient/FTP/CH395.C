/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : Basic functions for CH395
 **********************************************************************************/
/* Header files included */
#include <stdio.h>
#include <string.h>
#include "CH395INC.H"
#include "CH395.H"
#include "CH395FTPINC.H"
/* Include command file */
#include "CH395CMD.h"

#define MybufLen 536
#define CH395_DEBG 1
UINT8 MyBuffer[2][MybufLen];
struct _SOCK_INF SockInf[2];
struct _CH395_SYS CH395Inf;
UINT8 PortTmp;

extern const UINT8 CH395MACAddr[6];
extern const UINT8 CH395IPAddr[4];
extern const UINT8 CH395GWIPAddr[4];
extern const UINT8 CH395IPMask[4];
extern const UINT8 DestIPAddr[4];

/*******************************************************************************
 * Function Name  : mStopIfError
 * Description    : Used for debugging, displays error code and halts the system
 * Input          : iError
 * Output         : None
 * Return         : None
 *******************************************************************************/
void mStopIfError(UINT8 iError)
{
    if (iError == CMD_ERR_SUCCESS)
        return;                              /* Operation succeeded */
    printf("Error: %02X\n", (UINT16)iError); /* Display error */
    while (1)
    {
        Delay_Ms(200);
        Delay_Ms(200);
    }
}

/*******************************************************************************
 * Function Name  : InitCH395InfParam
 * Description    : Initialize the CH395Inf parameters
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void InitCH395InfParam(void)
{
    memset(&CH395Inf, 0, sizeof(CH395Inf));                          /* Clear all of CH395Inf */
    memcpy(CH395Inf.IPAddr, CH395IPAddr, sizeof(CH395IPAddr));       /* Write IP address into CH395Inf */
    memcpy(CH395Inf.GWIPAddr, CH395GWIPAddr, sizeof(CH395GWIPAddr)); /* Write gateway IP address into CH395Inf */
    memcpy(CH395Inf.MASKAddr, CH395IPMask, sizeof(CH395IPMask));     /* Write subnet mask into CH395Inf */
}

/*******************************************************************************
 * Function Name  : CH395ClientSocketInitOpen
 * Description    : Configure CH395 socket parameters, initialize and open socket
 * Input          : index
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395ClientSocketInitOpen(UINT8 index)
{
    UINT8 i;

    CH395CMDSetSocketDesIP(index, SockInf[index].IPAddr);       /* Set socket destination IP address */
    CH395CMDSetSocketProtType(index, SockInf[index].ProtoType); /* Set socket protocol type */
    CH395CMDSetSocketDesPort(index, SockInf[index].DesPort);    /* Set socket destination port */
    CH395CMDSetSocketSourPort(index, SockInf[index].SourPort);  /* Set socket source port */
    i = CH395CMDOpenSocket(index);                              /* Open socket */
    mStopIfError(i);                                            /* Check for success */
    i = CH395CMDTCPConnect(index);                              /* TCP connection */
    mStopIfError(i);                                            /* Check for success */
}

/********************************************************************************
 * Function Name  : CH395ServerSocketInitOpen
 * Description    : Configure CH395 socket parameters, initialize and open socket
 * Input          : index
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395ServerSocketInitOpen(UINT8 index)
{
    UINT8 i;

    CH395CMDSetSocketProtType(index, SockInf[index].ProtoType); /* Set socket protocol type */
    CH395CMDSetSocketSourPort(index, SockInf[index].SourPort);  /* Set socket source port */
    i = CH395CMDOpenSocket(index);                              /* Open socket */
    mStopIfError(i);                                            /* Check for success */
    i = CH395CMDTCPListen(index);                               /* TCP connection */
    mStopIfError(i);                                            /* Check for success */
}

/*******************************************************************************
 * Function Name  : CH395_FTPCtlClient
 * Description    : Establish TCP control connection
 * Input          : socketid
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPCtlClient(UINT8 socketid)
{
    memset(&SockInf[socketid], 0, sizeof(SockInf[socketid])); /* Clear corresponding SockInf */
    memcpy(SockInf[socketid].IPAddr, DestIPAddr, 4);          /* Write destination IP address */
    SockInf[socketid].DesPort = 21;                           /* Destination port */
    SockInf[socketid].SourPort = 4000;                        /* Source port */
    SockInf[socketid].ProtoType = PROTO_TYPE_TCP;             /* TCP mode */
    ftp.SocketCtl = socketid;
    CH395ClientSocketInitOpen(socketid);
    memset((void *)MyBuffer[socketid], '\0', sizeof(MyBuffer[socketid]));
}

/*******************************************************************************
 * Function Name  : CH395_FTPDatServer
 * Description    : Initialize socket
 * Input          : socketid    - socket index
                    port        - socket source port
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPDatServer(UINT8 socketid, UINT16 *port)
{
    if (PortTmp < 100 || PortTmp > 200)
        PortTmp = 100;
    PortTmp++;
    memset(&SockInf[socketid], 0, sizeof(SockInf[socketid])); /* Clear corresponding SockInf */
    SockInf[socketid].SourPort = 4000 + PortTmp;              /* Source port */
    *port = SockInf[socketid].SourPort;
    SockInf[socketid].ProtoType = PROTO_TYPE_TCP; /* TCP mode */
    ftp.DatMonitor = socketid;
    CH395ServerSocketInitOpen(socketid);
    memset((void *)MyBuffer[socketid], '\0', sizeof(MyBuffer[socketid]));
}

/******************************************************************************
 * Function Name  : CH395SocketInterrupt
 * Description    : CH395 socket interrupt, called in the global interrupt
 * Input          : sockindex
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395SocketInterrupt(UINT8 sockindex)
{
    UINT8 sock_int_socket;
    UINT16 len;

    sock_int_socket = CH395CMDGetSocketInt(sockindex); /* Get socket interrupt status */
#if CH395_DEBG
    printf("socket%d = %x\n", (UINT16)sockindex, sock_int_socket);
#endif
    if (sock_int_socket & SINT_STAT_SENDBUF_FREE) /* Send buffer is free, data can be written */
    {
    }
    if (sock_int_socket & SINT_STAT_SEND_OK) /* Send complete interrupt */
    {
#if CH395_DEBG
        printf("send data success\n");
#endif
    }
    if (sock_int_socket & SINT_STAT_CONNECT) /* Connection interrupt, only valid in TCP mode */
    {
#if CH395_DEBG
        printf("TCP Connect\n");
#endif
        if (sockindex == ftp.DatMonitor)
        {
            ftp.TcpStatus = FTP_MACH_CONNECT;
        }
    }
    if (sock_int_socket & SINT_STAT_RECV) /* Receive interrupt */
    {

        len = CH395CMDGetRecvLength(sockindex); /* Get current buffer data length */
#if CH395_DEBG
        printf("len:%d\n", len);
#endif
        if (len)
        {
            if (len > MybufLen)
                len = MybufLen;
            memset(MyBuffer[sockindex], 0, MybufLen);
            CH395CMDGetRecvData(sockindex, len, MyBuffer[sockindex]); /* Read data */
            SockInf[sockindex].RemLen = len;                          /* Save length */
            ftp.RecDatLen = len;
#if CH395_DEBG
            printf("MyBuffer:\n%s\n", MyBuffer[sockindex]);
#endif
            CH395_FTPProcessReceDat((char *)MyBuffer[sockindex], ftp.FileCmd, sockindex);
            memset((void *)MyBuffer[sockindex], '\0', sizeof(MyBuffer[sockindex]));
        }
    }
    if (sock_int_socket & SINT_STAT_DISCONNECT) /* Disconnect interrupt, only valid in TCP mode */
    {
#if CH395_DEBG
        printf("TCP Disconnect\n"); /* Application may need to reconnect */
#endif
        if (sockindex == ftp.DatMonitor)
        {
            ftp.TcpStatus = 0;
            ftp.CmdDataS = FTP_MACH_DATAOVER;
        }
        printf("close socket%d : %d\n", sockindex, CH395CMDCloseSocket(sockindex));
    }
    if (sock_int_socket & SINT_STAT_TIM_OUT) /* Timeout interrupt, only valid in TCP mode */
    {
#if CH395_DEBG
        printf("TCP Timout\n"); /* Application may need to reconnect */
#endif
        /* A timeout interrupt indicates a connection/send/receive timeout or failure. Upon timeout, CH395 internally closes the socket. */
        /* In some cases, CH395 may not retry the connection, for example, if the remote port is not open. */
        if (sockindex == ftp.DatMonitor)
        {
            ftp.TcpStatus = 0;
            ftp.CmdDataS = FTP_MACH_DATAOVER;
        }
        CH395CMDCloseSocket(sockindex);
    }
}

/********************************************************************************
 * Function Name  : CH395GlobalInterrupt
 * Description    : CH395 global interrupt function
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395GlobalInterrupt(void)
{
    UINT16 init_status;
    UINT16 i;
    UINT8 buf[10];

    init_status = CH395CMDGetGlobIntStatus();
    if (init_status & GINT_STAT_UNREACH) /* Unreachable interrupt, read unreachable information */
    {
        CH395CMDGetUnreachIPPT(buf);
    }
    if (init_status & GINT_STAT_IP_CONFLI) /* IP conflict interrupt, it is recommended to modify the CH395 IP and initialize CH395 */
    {
    }
    if (init_status & GINT_STAT_PHY_CHANGE) /* PHY change interrupt */
    {
#if CH395_DEBG
        printf("Init status : GINT_STAT_PHY_CHANGE\n");
#endif
        i = CH395CMDGetPHYStatus(); /* Get PHY status */
        if (i == PHY_DISCONN)
        {
#if CH395_DEBG
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

/********************************************************************************
 * Function Name  : CH395Init
 * Description    : Configures the CH395's IP, GWIP, MAC, and other parameters, and initializes it
 * Input          : None
 * Output         : None
 * Return         : Function execution result
 *******************************************************************************/
UINT8 CH395Init(void)
{
    UINT8 i;
#if (CH395_OP_INTERFACE_MODE == CH395_UART_MODE)
    CH395SetUartBaudRate(921600);
#endif

    i = CH395CMDCheckExist(0x65);
    if (i != 0x9a)
        return CH395_ERR_UNKNOW;            /* Test command, returns 0XFA if it fails */
                                            /* A return of 0XFA typically indicates a hardware error or incorrect read/write timing */
    CH395CMDSetIPAddr(CH395Inf.IPAddr);     /* Set CH395's IP address */
    CH395CMDSetGWIPAddr(CH395Inf.GWIPAddr); /* Set gateway address */
    CH395CMDSetMASKAddr(CH395Inf.MASKAddr); /* Set subnet mask, default is 255.255.255.0 */
    CH395CMDSetFunPapr(0x08, 0x00, 0x00, 0x00);
    i = CH395CMDInitCH395(); /* Initialize CH395 chip */
    return i;
}

/********************************************************************************
 * Function Name  : CH395_SocketSendData
 * Description    : Send data
 * Input          : PSend - Data to send
                    Len   - Length
                    index - Socket index
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395_SocketSendData(char *PSend, UINT32 Len, UINT8 index)
{
    CH395CMDSendData(index, (UINT8 *)PSend, Len);
    printf("Send:%s", PSend);
}

/********************************************************************************
 * Function Name  : CH395_FTPInitVari
 * Description    : Variable initialization
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPInitVari()
{
    FTP *pFTP;
    UINT8 i, j;

    pFTP = &ftp;
    memset((void *)pFTP, '\0', sizeof(ftp));
    ftp.SocketCtl = 255;
    ftp.DatMonitor = 255;
    ftp.FileCmd = FTP_CMD_LOGIN; /* Execute login command */
    memset((void *)SourIP, '\0', sizeof(SourIP));
    j = 0;
    for (i = 0; i < 4; i++)
    { /* Convert decimal IP address to the required character format */
        if (CH395IPAddr[i] / 100)
        {
            SourIP[j++] = CH395IPAddr[i] / 100 + '0';
            SourIP[j++] = (CH395IPAddr[i] % 100) / 10 + '0';
            SourIP[j++] = CH395IPAddr[i] % 10 + '0';
        }
        else if (CH395IPAddr[i] / 10)
        {
            SourIP[j++] = CH395IPAddr[i] / 10 + '0';
            SourIP[j++] = CH395IPAddr[i] % 10 + '0';
        }
        else
            SourIP[j++] = CH395IPAddr[i] % 10 + '0';
        SourIP[j++] = ',';
    }
#if CH395_DEBG
    printf("SourIP: %s\n", SourIP);
#endif
}

/******************************************************************************
 * Function Name  : CH395_FTPConnect
 * Description    : Initialize CH395
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPConnect()
{
    UINT8 i;
    CH395_FTPInitVari();
    InitCH395InfParam(); /* Initialize CH395-related variables */
    i = CH395Init();     /* Initialize CH395 chip */
    mStopIfError(i);

    while (1)
    {                                              /* Wait for Ethernet connection */
        if (CH395CMDGetPHYStatus() == PHY_DISCONN) /* Check if CH395 is connected */
        {
            Delay_Ms(200); /* If not connected, wait 200ms before checking again */
        }
        else
        {
#if CH395_DEBG
            printf("CH395 Connect Ethernet\n"); /* CH395 is connected to Ethernet, interrupt will occur */
#endif
            break;
        }
    }
}
