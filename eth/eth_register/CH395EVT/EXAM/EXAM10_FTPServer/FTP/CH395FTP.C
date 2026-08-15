/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395FTP.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 chip FTP server application - FTP command code
 **********************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "CH395INC.H"
#include "CH395CMD.H"
#include "CH395FTPINC.H"
#include "CH395.H"

/*******************************************************************************
 * Function Name  : CH395_FTPSendData
 * Description    : Prepare data to be sent
 * Input          : pName - file name (if requesting list information, it sends character '0')
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPDataReady(char *pName)
{
     ftp.CmdDataS = FTP_MACH_SENDDATA;
     CH395_FTPFileRead(pName); /* Write the data to be sent into the send buffer */
}

/******************************************************************************
 * Function Name  : CH395_FTPProcessReceDat
 * Description    : Process received data
 * Input          : recv_buff   - handshake information
                    sockeid     - socket index
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395_FTPProcessReceDat(char *recv_buff, UINT8 sockeid)
{
     if (sockeid == ftp.SocketDat)
     {
          if (ftp.CmdDataS == FTP_MACH_RECEDATA)
          { /* Receive file data */
               CH395_FTPFileWrite(recv_buff, (UINT16)strlen(recv_buff));
          }
     }
     if (sockeid == ftp.SocketCtl)
     { /* Receive command */
          CH395_FTPCmdRespond(recv_buff);
     }
}

/******************************************************************************
 * Function Name  : CH395_FTPServerCmd
 * Description    : Query status and execute corresponding commands
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPServerCmd()
{
     UINT8 i;

     if (ftp.TcpCtlSta == FTP_MACH_CONNECT)
     { /* Establish TCP FTP control connection */
          ftp.TcpCtlSta = FTP_MACH_KEPCONT;
          CH395_FTPSendData((char *)FTP_SERVICE_CMD[0], strlen(FTP_SERVICE_CMD[0]), ftp.SocketCtl);
     }
     if (ftp.TcpDatSta == FTP_MACH_CONNECT)
     { /* CH395 establishes data connection */
          if (ftp.CmdPortS == 1)
          {
               ftp.CmdPortS = 0;
               CH395_FTPSendData((char *)FTP_SERVICE_CMD[5], strlen(FTP_SERVICE_CMD[5]), ftp.SocketCtl);
          }
          else if (ftp.CmdPasvS == 1)
          {
               ftp.CmdPasvS = 0;
          }
     }
     if (ftp.TcpDatSta == FTP_MACH_DISCONT)
     { /* Data reception is complete */
          if (ftp.CmdDataS == FTP_MACH_RECEDATA)
          {
               ftp.CmdDataS = FTP_MACH_DATAOVER;
               CH395_FTPListRenew(ftp.ListFlag); /* Update directory */
          }
     }
     if (ftp.CmdDataS == FTP_MACH_DATAOVER)
     { /* Data transmission completed */
          ftp.CmdDataS = FTP_MACH_CLOSECTL;
          CH395_FTPSendData((char *)FTP_SERVICE_CMD[1], strlen(FTP_SERVICE_CMD[1]), ftp.SocketCtl);
     }
     if (ftp.CmdDataS == FTP_MACH_SENDDATA)
     { /* Send data */
          if (ftp.TcpDatSta >= FTP_MACH_CONNECT)
          {
               ftp.TcpDatSta = FTP_MACH_KEPCONT;
               CH395_FTPSendData(SendBuf, strlen(SendBuf), ftp.SocketDat);
               if (ftp.DataOver)
               {                                            /* Detect end of sending */
                    ftp.CmdDataS = FTP_MACH_DATAOVER;       /* Set send completion flag */
                    i = CH395CMDCloseSocket(ftp.SocketDat); /* Close data connection */
                    mStopIfError(i);
               }
          }
     }
}
