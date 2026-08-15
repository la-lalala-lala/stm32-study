/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395FTPINC.H
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : Description of CH395FTP.c header file
 **********************************************************************************/
#ifndef __CH395FTPINC_H__
#define __CH395FTPINC_H__
/**********************************************************************************/
#ifndef Access_Authflag
#define Access_Authflag 0 /* Set specific username access (other usernames cannot access the server) */
#endif
/**********************************************************************************/
#define SendBufLen 500 /* Send buffer length */
typedef struct _FTP
{
  UINT8 TimeCount; /* Timer count */
  UINT8 FileFlag;  /* Flag to determine if file length is greater than 64 bytes (this program supports a maximum upload file size of 64) */
  UINT8 BufStatus; /* Buffer status flags (bits 0-3 represent the status of four buffers, 0 is empty, 1 is in use) */
  UINT8 AuthFlag;  /* User permission flag */
  UINT8 ListFlag;
  UINT8 CmdDataS;    /* Data transfer status */
  UINT8 CmdReceDatS; /* Receive data status */
  UINT8 DataOver;    /* Data reading completed */
  UINT8 CmdPortS;
  UINT8 CmdPasvS;
  UINT8 ListState; /* 0 for current directory, 1 for next level directory */
  UINT8 TcpCtlSta; /* TCP control connection status */
  UINT8 TcpDatSta; /* TCP data connection status */
  UINT8 SocketCtl; /* FTP control connection */
  UINT8 SocketDat; /* FTP data connection */
} FTP;
extern FTP ftp;
extern char SendBuf[SendBufLen];
extern const char FTP_SERVICE_CMD[10][60];
extern const char FTP_SERVICE_CMD1[10][40];
extern const char FTP_SERVICE_CMD2[2][50];

extern char UserName[16]; /* Used to store the username */
extern char SourIP[17];   /* Used to store the converted character IP address */
#if Access_Authflag
extern char *pUserName; /* Username with access permission (other usernames cannot access the server) */
#endif
extern char *pPassWord; /* Authorized password (other passwords only support read) */
/* TCP connection status */
#define FTP_MACH_DISCONT 0x01 /* TCP disconnected */
#define FTP_MACH_CONNECT 0x02 /* TCP connected */
#define FTP_MACH_KEPCONT 0x04 /* TCP keep connection */
/*  Data transfer status */
#define FTP_MACH_SENDDATA 0x01 /* Need to send data */
#define FTP_MACH_RECEDATA 0x02 /* Need to receive data */
#define FTP_MACH_DATAOVER 0x04 /* Data transfer completed */
#define FTP_MACH_CLOSECTL 0x08 /* Command sent */
#define FTP_MACH_LIST 0x00     /* Get list */
#define FTP_MACH_FILE 0x01     /* Operate on file */
#define FTP_CMD_SUCCESS 0x00   /* Get list */
#define FTP_CMD_ERROR 0x01     /* Operate on file */

/**********************************************************************************/
void CH395_FTPSendData(char *PSend, UINT16 Len, UINT8 index); /* Send data */

void CH395_FTPConnect(void); /* Initialize 395 */

void CH395_FTPInitVari(void); /* Variable initialization */

void mInitFtpList(void); /* Initialize the list and files, used for simulating files and directory lists */

void CH395_FTPListRenew(UINT8 index); /* Update directory */

void CH395_FTPProcessReceDat(char *recv_buff, UINT8 index); /* Process received data */

void CH395_FTPCmdRespond(char *recv_buff); /* Detect received command and respond accordingly */

void CH395_FTPDataReady(char *pSendDat); /* Prepare data to send */

void CH395_FTPGetPortNum(char *recv_buff); /* Get port for connection */

void CH395_FTPGetFileName(char *recv_buff, char *pBuf);

UINT8 CH395_FTPFileOpen(char *pName, UINT8 index); /* Open file */

UINT32 CH395_FTPFileSize(char *pName); /* Get file size */

void CH395_FTPFileRead(char *pName); /* Read file */

void CH395_FTPFileWrite(char *recv_buff, UINT16 LEN); /* Write file */

void CH395_FTPCtlServer(UINT8 socketid); /* Create FTP control connection */

void CH395_FTPDatClient(UINT8 socketid, UINT16 port, UINT8 *pAddr); /* Create FTP client data connection */

void CH395_FTPDatServer(UINT8 socketid, UINT16 *port); /* Create FTP server data connection */

void CH395_FTPGetSocketID(UINT8 socketid); /* Get data connection socket index */

void CH395_FTPServerCmd(void); /* Query status and execute corresponding subroutine */

#endif
/**********************************************************************************/
