/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395FTP.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 FTP server application - FTP command code
 **********************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "CH395INC.H"
#include "CH395CMD.H"
#include "CH395FTPINC.H"
#include "CH395.H"

/******************************************************************************
 * Function Name  : CH395_FTPProcessReceDat
 * Description    : Process received data
 * Input          : recv_buff  - Handshake information
                    check_type - Command type to check
                    socketid   - Socket index
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395_FTPProcessReceDat(char *recv_buff, UINT8 check_type, UINT8 socketid)
{
    UINT8 S;
    if (socketid == ftp.DatMonitor)
    {
        if (ftp.CmdDataS == FTP_MACH_RECEDATA)
        { /* Receive file data */
            if (ftp.InterCmdS == FTP_MACH_GETFILE)
            {
                S = CH395_FTPFileWrite(recv_buff, strlen(recv_buff));
            }
            else if (ftp.InterCmdS == FTP_MACH_FINDLIST)
            {
                S = CH395_FTPFindList(recv_buff); /* Verify transmitted data for searching a specific directory */
                if (S == FTP_CHECK_SUCCESS)
                    ftp.FindList = 1; /* Directory name found */
            }
            else if (ftp.InterCmdS == FTP_MACH_FINDFILE)
            {
                S = CH395_FTPFindFile(recv_buff); /* Find file */

                if (S == FTP_CHECK_SUCCESS)
                    ftp.FindFile = 1; /* File found */
                printf("FindFile: %d\r\n", ftp.FindFile);
            }
        }
    }
    else if (socketid == ftp.SocketCtl)
    { /* Receive command response */
        char *token;
        // Split message using newline "\n"
        token = strtok(recv_buff, "\n");
        while (token != NULL)
        {
            printf("ftp recv:%s\r\n", token);
            // Parse each message
            S = CH395_FTPCmdRespond(token, check_type);
            // Get the next segment
            token = strtok(NULL, "\n");
        }
    }
}

/*******************************************************************************
 * Function Name  : CH395_FTPSendFile
 * Description    : Send file data
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPSendFile()
{
    UINT8 S;
    S = CH395_FTPFileOpen(FileName);
    if (S == FTP_CHECK_SUCCESS)
        CH395_FTPFileRead();
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.DatMonitor);
    Delay_Ms(10);
    if (ftp.CmdDataS == FTP_MACH_DATAOVER)
    {
        CH395CMDCloseSocket(ftp.DatMonitor);
    }
}

/*******************************************************************************
 * Function Name  : CH395_FTPClientCmd
 * Description    : Query status and execute corresponding command
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPClientCmd()
{
    if (ftp.CmdDataS == FTP_MACH_SENDDATA)
    {
        if (ftp.TcpStatus == FTP_MACH_CONNECT)
            CH395_FTPSendFile(); /* Send data to server */
        return;
    }
    if (ftp.FileCmd)
    {
        CH395_FTPInterCmd(); /* Execute corresponding interface command */
    }
}

/*******************************************************************************
 * Function Name  : CH395_FTPInterCmd
 * Description    : Execute corresponding command, order can be adjusted
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPInterCmd()
{
    switch (ftp.FileCmd)
    {
    case FTP_CMD_LOGIN: /* Login */
        if (CH395_FTPLogin() == FTP_COMMAND_SUCCESS)
        {                                            /* Login successful, other operations can proceed */
            CH395_FTPSearch("USER", "FILELIST.TXT"); /* Start searching for files in the specified directory */
        }
        break;
    case FTP_CMD_SEARCH: /* Search for file (parameters: directory name, file name) */
        if (CH395_FTPSearch("USER", "FILELIST.TXT") == FTP_COMMAND_SUCCESS)
        { /* Search command completed, other operations can proceed */
            if (ftp.FindFile)
                CH395_FTPGetFile("FILELIST.TXT"); /* If file found in the specified directory, start downloading the file */
            else
                CH395_FTPQuit(); /* If file not found, quit (or other operations like uploading) */
        }
        break;
    case FTP_CMD_GETFILE: /* Download file (parameter: file name) */
        if (CH395_FTPGetFile("FILELIST.TXT") == FTP_COMMAND_SUCCESS)
        {                                        /* File download successful, other operations can proceed */
            CH395_FTPPutFile("TEXT", "abc.txt"); /* Upload file */
        }
        break;
    case FTP_CMD_PUTFILE: /* Upload file (parameters: directory name, file name) */
        if (CH395_FTPPutFile("TEXT", "abc.txt") == FTP_COMMAND_SUCCESS)
        {                    /* File upload successful, other operations can proceed */
            CH395_FTPQuit(); /* Quit */
        }
        break;
    default:
        break;
    }
}
