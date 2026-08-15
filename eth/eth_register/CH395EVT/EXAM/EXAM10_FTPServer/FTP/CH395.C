/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : Basic functions of CH395
 **********************************************************************************/
/* Header file inclusion */
#include <stdio.h>
#include <string.h>
#include "CH395INC.H"
#include "CH395.H"
#include "CH395FTPINC.H"
/* Include command file */
#include "CH395CMD.h"
/***********************************************************************************/
/* Common variable definitions */
#define MybufLen 536 /* Define the maximum length of the CH395 receive buffer */
#define CH395_DEBG 1
UINT8 MyBuffer[2][MybufLen]; /* Data buffer */
struct _SOCK_INF SockInf[2]; /* Store socket information */
struct _CH395_SYS CH395Inf;  /* Store CH395 information */

UINT8 SerPort;
UINT32 TMP_LEN;
extern const UINT8 CH395MACAddr[6];
extern const UINT8 CH395IPAddr[4];
extern const UINT8 CH395GWIPAddr[4];
extern const UINT8 CH395IPMask[4];
/*******************************************************************************
 * Function Name  : mStopIfError
 * Description    : Used for debugging, displays the error code and halts execution
 * Input          : iError
 * Output         : None
 * Return         : None
 *******************************************************************************/
void mStopIfError(UINT8 iError)
{
     if (iError == CMD_ERR_SUCCESS)
          return;                             /* Operation successful */
     printf("Error: %02X\n", (UINT16)iError); /* Display error */
     while (1)
     {
          Delay_Ms(200);
          Delay_Ms(200);
     }
}

/********************************************************************************
 * Function Name  : InitCH395InfParam
 * Description    : Initialize CH395Inf parameters
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void InitCH395InfParam(void)
{
     memset(&CH395Inf, 0, sizeof(CH395Inf));                          /* Clear CH395Inf structure */
     memcpy(CH395Inf.IPAddr, CH395IPAddr, sizeof(CH395IPAddr));       /* Write IP address to CH395Inf */
     memcpy(CH395Inf.GWIPAddr, CH395GWIPAddr, sizeof(CH395GWIPAddr)); /* Write gateway IP address to CH395Inf */
     memcpy(CH395Inf.MASKAddr, CH395IPMask, sizeof(CH395IPMask));     /* Write subnet mask to CH395Inf */
     // memcpy(CH395Inf.MacAddr,CH395MACAddr,sizeof(CH395MACAddr));
}

/*******************************************************************************
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
     mStopIfError(i);                                            /* Check if successful */
     i = CH395CMDTCPListen(index);                               /* Start TCP listening */
     mStopIfError(i);                                            /* Check if successful */
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
     mStopIfError(i);                                            /* Check if successful */
     i = CH395CMDTCPConnect(index);                              /* Establish TCP connection */
     mStopIfError(i);                                            /* Check if successful */
}

/*******************************************************************************
 * Function Name  : CH395_FTPCtlServer
 * Description    : Establish a TCP control connection
 * Input          : socketid
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPCtlServer(UINT8 socketid)
{
     memset(&SockInf[socketid], 0, sizeof(SockInf[socketid])); /* Clear the corresponding SockInf */
     SockInf[socketid].SourPort = 21;                          /* Source port */
     SockInf[socketid].ProtoType = PROTO_TYPE_TCP;             /* TCP mode */
     CH395ServerSocketInitOpen(socketid);
     memset((void *)MyBuffer[socketid], '\0', sizeof(MyBuffer[socketid]));
}

/*******************************************************************************
 * Function Name  : CH395_FTPDatServer
 * Description    : Initialize socket
 * Input          : socketid      - socketid
                    port          - destination port
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPDatServer(UINT8 socketid, UINT16 *port)
{
     if (SerPort < 100 || SerPort > 200)
          SerPort = 100;
     SerPort++;
     memset(&SockInf[socketid], 0, sizeof(SockInf[socketid])); /* Clear the corresponding SockInf */
     SockInf[socketid].SourPort = 256 * 5 + SerPort;           /* Source port */
     *port = SockInf[socketid].SourPort;
     SockInf[socketid].ProtoType = PROTO_TYPE_TCP; /* TCP mode */
     CH395ServerSocketInitOpen(socketid);
     memset((void *)MyBuffer[socketid], '\0', sizeof(MyBuffer[socketid]));
}

/*******************************************************************************
 * Function Name  : CH395_FTPDatClient
 * Description    : Initialize socket
 * Input          : socketid    - socketid
                    port        - destination port
                    pAddr       - destination IP address
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPDatClient(UINT8 socketid, UINT16 port, UINT8 *pAddr)
{
     memset(&SockInf[socketid], 0, sizeof(SockInf[socketid])); /* Clear the corresponding SockInf */
     memcpy(SockInf[socketid].IPAddr, pAddr, 4);               /* Write destination IP address */
     SockInf[socketid].DesPort = port;                         /* Destination port */
     SockInf[socketid].SourPort = 20;                          /* Source port */
     SockInf[socketid].ProtoType = PROTO_TYPE_TCP;             /* TCP mode */
     CH395ClientSocketInitOpen(socketid);
     memset((void *)MyBuffer[socketid], '\0', sizeof(MyBuffer[socketid]));
}

/*******************************************************************************
 * Function Name  : CH395SocketInterrupt
 * Description    : CH395 socket interrupt, called within the global interrupt
 * Input          : sockindex
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395SocketInterrupt(UINT8 sockindex)
{
     UINT8 sock_int_socket;
     UINT16 len;

#if CH395_DEBG
     printf("sockindex =%d\n", (UINT16)sockindex);
#endif
     sock_int_socket = CH395CMDGetSocketInt(sockindex); /* Get the socket interrupt status */
     if (sock_int_socket & SINT_STAT_SENDBUF_FREE)      /* Send buffer is free, can continue writing data to send */
     {
     }
     if (sock_int_socket & SINT_STAT_SEND_OK) /* Send completion interrupt */
     {
#if CH395_DEBG
          printf("send data success\n");
#endif
          if (ftp.CmdDataS == FTP_MACH_CLOSECTL)
          {
               if (sockindex == ftp.SocketCtl)
               {
                    ftp.CmdDataS = 0;
                    CH395CMDCloseSocket(ftp.SocketCtl);
               }
          }
     }
     if (sock_int_socket & SINT_STAT_CONNECT) /* Connection interrupt, valid only in TCP mode */
     {
#if CH395_DEBG
          printf("TCP Connect\n");
#endif
          CH395_FTPGetSocketID(sockindex);
          if (ftp.CmdReceDatS == 1)
          { /* Data reception needed */
               if (ftp.TcpDatSta == FTP_MACH_CONNECT)
               {
                    ftp.CmdReceDatS = 0;
                    ftp.CmdDataS = FTP_MACH_RECEDATA;
               }
          }
     }
     if (sock_int_socket & SINT_STAT_RECV) /* Receive interrupt */
     {
          printf("ftp sockid:%d\r\n", sockindex);
          len = CH395CMDGetRecvLength(sockindex); /* Get the current buffer data length */
          if (len)
          {
               if (len > MybufLen)
                    len = MybufLen;
               CH395CMDGetRecvData(sockindex, len, MyBuffer[sockindex]); /* Read data */
               SockInf[sockindex].RemLen = len;                          /* Save length */
#if CH395_DEBG
               printf("len:%d\n", len);
               printf("MyBuffer:\n%s\n", MyBuffer[sockindex]);
#endif

               CH395_FTPProcessReceDat((char *)MyBuffer[sockindex], sockindex);
               memset((void *)MyBuffer[sockindex], '\0', sizeof(MyBuffer[sockindex]));
          }
     }
     if (sock_int_socket & SINT_STAT_DISCONNECT) /* Disconnection interrupt, valid only in TCP mode */
     {
#if CH395_DEBG
          printf("TCP Disconnect\n");
#endif
          CH395CMDCloseSocket(sockindex);
          if (sockindex == ftp.SocketCtl)
          {
               ftp.TcpCtlSta = FTP_MACH_DISCONT;
               CH395_FTPCtlServer(0);
          }
          else if (sockindex == ftp.SocketDat)
          {
               ftp.TcpDatSta = FTP_MACH_DISCONT;
          }
     }
     if (sock_int_socket & SINT_STAT_TIM_OUT) /* Timeout interrupt, valid only in TCP mode */
     {
#if CH395_DEBG
          printf("TCP Timeout\n");
#endif
          CH395CMDCloseSocket(sockindex);
          if (sockindex == ftp.SocketCtl)
          {
               ftp.TcpCtlSta = FTP_MACH_DISCONT;
               CH395_FTPCtlServer(0);
          }
          else if (sockindex == ftp.SocketDat)
          {
               ftp.TcpDatSta = FTP_MACH_DISCONT;
          }
     }
}

/******************************************************************************
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
     if (init_status & GINT_STAT_IP_CONFLI) /* IP conflict interrupt, recommended to change CH395 IP and reinitialize */
     {
     }
     if (init_status & GINT_STAT_PHY_CHANGE) /* PHY status change interrupt */
     {
#if CH395_DEBG
          printf("Init status : GINT_STAT_PHY_CHANGE\n");
#endif
          i = CH395CMDGetPHYStatus(); /* Get PHY status */
          if (i == PHY_DISCONN)
          {
#if CH395_DEBG
               printf("Ethernet Disconnect\n");
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

/*******************************************************************************
 * Function Name  : CH395Init
 * Description    : Configure CH395 parameters such as IP, GWIP, MAC, and initialize it
 * Input          : None
 * Output         : None
 * Return         : Function execution result
 *******************************************************************************/
UINT8 CH395Init(void)
{
     UINT8 i;

     i = CH395CMDCheckExist(0x65);
     if (i != 0x9a)
          return CH395_ERR_UNKNOW; /* Test command, return 0xFA if it fails */
                                   /* Returning 0xFA usually indicates a hardware error or incorrect read/write timing */
#if (CH395_OP_INTERFACE_MODE == CH395_UART_MODE)
     CH395SetUartBaudRate(921600);
#endif
     CH395CMDSetIPAddr(CH395Inf.IPAddr);     /* Set CH395 IP address */
     CH395CMDSetGWIPAddr(CH395Inf.GWIPAddr); /* Set gateway address */
     CH395CMDSetMASKAddr(CH395Inf.MASKAddr); /* Set subnet mask, default is 255.255.255.0 */
     i = CH395CMDInitCH395();                /* Initialize CH395 chip */
     return i;
}

/*******************************************************************************
 * Function Name  : CH395_FTPSendData
 * Description    : Send data
 * Input          : PSend   - Data content to be sent
                    Len     - Length
                    index   - Socket index
 * Output         : None
 * Return         : None
*******************************************************************************/

void CH395_FTPSendData(char *PSend, UINT16 Len, UINT8 index)
{
     CH395CMDSendData(index, (UINT8 *)PSend, Len);
}

/*******************************************************************************
 * Function Name  : CH395_FTPGetSocketID
 * Description    : Get the data connection socket ID
 * Input          : socketid - socket index
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPGetSocketID(UINT8 socketid)
{
     UINT8 i;

#if DEBUG
     printf("get socketid\n");
#endif
     if (SockInf[socketid].SourPort == 21)
     {
          if (ftp.TcpCtlSta <= FTP_MACH_DISCONT)
          {
               ftp.TcpCtlSta = FTP_MACH_CONNECT;
               ftp.SocketCtl = socketid;
#if DEBUG
               printf("ftp.SocketCtl = %d\n", (UINT16)ftp.SocketCtl);
#endif
          }
          else
          {
               i = CH395CMDCloseSocket(socketid); /* Accept only one client connection */
               mStopIfError(i);
#if DEBUG
               printf("ERROR: only support a control socket connected\n");
#endif
          }
     }
     if (SockInf[socketid].SourPort == 20 || SockInf[socketid].SourPort == (256 * 5 + SerPort))
     {
          if (ftp.TcpDatSta <= FTP_MACH_DISCONT)
          {
               ftp.TcpDatSta = FTP_MACH_CONNECT;
               ftp.SocketDat = socketid;
#if DEBUG
               printf("ftp.SocketDat = %d\n", (UINT16)ftp.SocketDat);
#endif
          }
          else
          {
               i = CH395CMDCloseSocket(socketid); /* Accept only one client connection */
               mStopIfError(i);
#if DEBUG
               printf("ERROR: only support a data socket connected\n");
#endif
          }
     }
}

/*******************************************************************************
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
     memset((void *)pFTP, 0, sizeof(ftp));
     ftp.SocketDat = 255;
     ftp.SocketCtl = 255;
     mInitFtpList();
     memset((void *)SourIP, '\0', sizeof(SourIP));
     j = 0;
     for (i = 0; i < 4; i++)
     { /* Convert a decimal IP address to the required character format */
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
}

/*******************************************************************************
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
#if CH395_DEBG
     printf("CH395FTP Demo\n");
#endif

     InitCH395InfParam(); /* Initialize CH395-related variables */
     i = CH395Init();     /* Initialize the CH395 chip */
     mStopIfError(i);

     while (1)
     {                                               /* Wait for the Ethernet connection to be established */
          if (CH395CMDGetPHYStatus() == PHY_DISCONN) /* Check if CH395 is connected */
          {
               Delay_Ms(200); /* If not connected, wait for 200ms before checking again */
          }
          else
          {
#if CH395_DEBG
               printf("CH395 Connect Ethernet\n"); /* CH395 chip is connected to Ethernet, an interrupt will be triggered */
#endif
               break;
          }
     }
     CH395_FTPCtlServer(0);
}
