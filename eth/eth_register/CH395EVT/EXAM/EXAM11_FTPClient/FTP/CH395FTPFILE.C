/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395FTPFILE.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 chip FTP client application - File read/write code
 *
 **********************************************************************************/
#include "debug.h"
#include "CH395FTPINC.H"
#include "CH395INC.H"
#include "string.h"

const UINT8 *pTest = "abcdefghijklmnopqrstuvwxyz0123456789"; /* Content of the document to be uploaded */

/**********************************************************************************
 * Function Name  : CH395_FTPFileOpen
 * Description    : Open file
 * Input          : pFileName - file name
 * Output         : None
 * Return         : Status
 *********************************************************************************/
UINT8 CH395_FTPFileOpen(char *pFileName)
{
     return FTP_CHECK_SUCCESS;
}

/*********************************************************************************
 * Function Name  : CH395_FTPFileWrite
 * Description    : Read downloaded data
 * Input          : recv_buff - data
                    len       - length
 * Output         : None
 * Return         : Status
*********************************************************************************/
UINT8 CH395_FTPFileWrite(char *recv_buff, UINT16 len)
{
     memcpy(send_buff, recv_buff, len);
     return (FTP_CHECK_SUCCESS);
}

/*********************************************************************************
 * Function Name  : CH395_FTPFileRead
 * Description    : Transfer data
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPFileRead(void)
{
     memset(send_buff, '\0', sizeof(send_buff));
     if (strlen((char *)pTest) < 536)
     {
          sprintf(send_buff, "%s\r\n", pTest); /* Write data to the send buffer. If the data is too large, write in multiple times */
          ftp.CmdDataS = FTP_MACH_DATAOVER;    /* If data is finished reading, set the data over flag */
     }
}
