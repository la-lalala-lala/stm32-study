/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395FTPINC
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395FTPINC header file declaration
 **********************************************************************************/
#ifndef __CH395FTPINC_H__
#define __CH395FTPINC_H__

typedef struct _FTP
{
  UINT8 TcpStatus;   /* TCP connection status */
  UINT8 CmdStatus;   /* Command execution status */
  UINT8 InterCmdS;   /* Interface command execution flag */
  UINT8 CmdFirtS;    /* First execution flag for the command */
  UINT8 FindList;    /* Found the specified folder */
  UINT8 FindFile;    /* Found the specified filename */
  UINT8 CmdDataS;    /* Data transmission status */
  UINT8 SocketCtl;   /* FTP control connection socket index */
  UINT8 DatMonitor;  /* FTP data connection listening socket index */
  UINT8 DatTransfer; /* FTP data connection transmission socket index */
  UINT8 FileCmd;     /* Interface command type  */
  UINT16 SouPort;    /* Port number */
  UINT16 RecDatLen;  /* Received data length */
} FTP;

extern FTP ftp;
extern char send_buff[536];
extern char ListName[24];      /* Used to store the directory name */
extern char ListMdk[24];       /* Used to store the newly created directory name */
extern char FileName[24];      /* Used to store the filename */
extern char SourIP[17];        /* Used to store the IP address in string format */
extern const UINT8 *pUserName; /* Anonymous login */
extern const UINT8 *pPassword; /* e-mail username as the login password */

/* TCP connection status */
#define FTP_MACH_DISCONT 0x00 /* TCP disconnected */
#define FTP_MACH_CONNECT 0x01 /* TCP connected */

/* Error status */
#define FTP_MACH_QUIT 0x01  /* Error before login */
#define FTP_MACH_CLOSE 0x02 /* Error after login */

/* Data transmission status */
#define FTP_MACH_SENDDATA 0x01 /* Need to send data */
#define FTP_MACH_RECEDATA 0x02 /* Need to receive data */
#define FTP_MACH_DATAOVER 0x04 /* Data transmission complete */

/* Command execution status */
#define FTP_MACH_LOGINSUC 0x01  /* Login successful */
#define FTP_MACH_SEARCHSUC 0x02 /* File search complete */
#define FTP_MACH_GETSUC 0x04    /* File download successful */
#define FTP_MACH_PUTSUC 0x08    /* File upload successful */

/* Command execution flags */
#define FTP_MACH_FINDLIST 0x01 /* Directory search flag */
#define FTP_MACH_FINDFILE 0x02 /* File search flag */
#define FTP_MACH_GETFILE 0x04  /* File download flag */
#define FTP_MACH_MKDLIST 0x08  /* Directory creation flag */
#define FTP_MACH_PUTFILE 0x10  /* File upload flag */

/* Return status information */
#define FTP_CHECK_NULL 0x00       /* No handshake signal returned, no need to check return information */
#define FTP_CHECK_SUCCESS 0x00    /* Handshake information correct */
#define FTP_COMMAND_SUCCESS 0x00  /* Interface command executed successfully */
#define FTP_COMMAND_CONTINUE 0x01 /* Interface command is still in progress */

/* Main program command types */
#define FTP_CMD_LOGIN 0x01   /* Login */
#define FTP_CMD_SEARCH 0x02  /* Search file */
#define FTP_CMD_GETFILE 0x03 /* Download file */
#define FTP_CMD_PUTFILE 0x04 /* Upload file */

/* *********************************************************************************************************************
 * Handshake signal verification error codes
 ************************************************************************************************************************/
#define FTP_ERR_LOGIN 0x20   /* Login */
#define FTP_ERR_SEARCH 0x21  /* Search file */
#define FTP_ERR_GETFILE 0x22 /* Download file */
#define FTP_ERR_PUTFILE 0x23 /* Upload file */
#define FTP_ERR_UNLIST 0x2a  /* Directory not found */
#define FTP_ERR_UNFILE 0x2b  /* File not found */
#define FTP_ERR_UNKW 0x2f    /* Unknown error */

UINT8 CH395_FTPFileWrite(char *recv_buff, UINT16 len);
UINT8 CH395_FTPFileOpen(char *pFileName);
void CH395_FTPFileRead(void);

void CH395_FTPClientCmd(void); /* Query status, execute subcommands */

void CH395_FTPInterCmd(void); /* Execute corresponding commands, the execution order can be adjusted */

void CH395_SocketSendData(char *PSend, UINT32 Len, UINT8 index); /* Send data */

void CH395_FTPConnect(void); /* 395 initialization */

void CH395_FTPInitVari(void); /* Variable initialization */

void CH395_FTPProcessReceDat(char *recv_buff, UINT8 check_type, UINT8 socketid); /* Check response information */

UINT8 CH395_FTPCmdRespond(char *recv_buff, UINT8 check_type); /* Check response information */

void CH395_FTPCtlClient(UINT8 socketid); /* Create FTP control connection */

void CH395_FTPDatServer(UINT8 socketid, UINT16 *port); /* Create FTP data connection */

void CH395_FTPCommand(UINT8 choiceorder); /* Execute corresponding command based on command type */

UINT8 CH395_FTPLogin(void); /* Login */

UINT8 CH395_FTPSearch(char *pListNAme, char *pFileName); /* Search file */

UINT8 CH395_FTPGetFile(char *pFileName); /* Download file */

UINT8 CH395_FTPPutFile(char *pListNAme, char *pFileName); /* Upload file */

UINT8 CH395_FTPFindFile(char *pReceiveData); /* Verify filename */

UINT8 CH395_FTPFindList(char *pReceiveData); /* Verify directory name */

void CH395_FTPQuit(void); /* Quit */

#endif
