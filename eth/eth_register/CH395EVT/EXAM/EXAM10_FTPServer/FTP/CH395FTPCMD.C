/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395FTPCMD.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 chip FTP server application - FTP command code
 **********************************************************************************/
#include "Ch395.h"
#include "CH395FTPINC.H"
#include "string.h"

const char FTP_SERVICE_CMD[10][60] =
    {
        "220 Microsoft FTP Service\r\n",                          /* 0 Login */
        "226 Transfer complete\r\n",                              /* 1 Transfer complete */
        "331 access allowed send identity as password.\r\n",      /* 2 Username correct */
        "230 user logged in.\r\n",                                /* 3 Login successful */
        "530 user cannot log in.\r\n",                            /* 4 Login failed */
        "200 PORT command successful.\r\n",                       /* 5 PORT command successful */
        "125 Data connection already open;Transfer starting\r\n", /* 6 Get list command */
        "150 Opening ASCII mode data connection for",             /* 7 Download/Upload file command successful */
        "550 The ch395 cannot find the file specified",           /* 8 File does not exist */
        "221 Goodbye\r\n",                                        /* 9 Exit */
};
const char FTP_SERVICE_CMD1[10][40] =
    {
        "200 Type set to",                     /* 0 TYPE set file transfer type */
        "215 Windows_NT\r\n",                  /* 1 syst system type */
        "257 \"/\" is current directory.\r\n", /* 2 PWD print directory */
        "250 command successful.\r\n",         /* 3 Command successful */
        "227 Entering Passive Mode(",          /* 4 PASV command successful */
        "213 ",                                /* 5 size command successful */
        "250 DELE command success\r\n",        /* 6 */
        "550 Access is denied.\r\n",           /* 7 */
        "200 noop command success.\r\n",       /* 8 */
        "500 command not understood\r\n",      /* 9 Unsupported command */
};
const char FTP_SERVICE_CMD2[2][50] =
    {
        "350 File exists,ready for destination name\r\n", /* 0 */
        "250 RNTO command successful\r\n",                /* 1 */
};

FTP ftp;
char SendBuf[SendBufLen]; /* Used to store the IP address of the destination */
UINT8 CmdPortIP[4];       /* Used to store the IP address converted to characters */
UINT16 TemAddr;           /* Temporary variable for analyzing IP address and port */
UINT16 NumPort;           /* Port number */
UINT32 gLEN;              /* File length */
/* Used to store file name */
char gFileName[16]; /* Used to store the requested download file name */
char pFileName[16]; /* Used to store the requested upload file name */
char ListName[16];
/* Used to store current directory */

/********************************************************************************
 * Function Name  : CH395_FTPGetPortNum
 * Description    : Get the port number and IP address
 * Input          : Received data
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPGetPortNum(char *recv_buff)
{
    UINT8 k, i;

    i = 0;
    memset((void *)CmdPortIP, 0, 4);
    for (k = 0; k < strlen(recv_buff); k++)
    {
        if (recv_buff[k] >= '0' && recv_buff[k] <= '9')
        {
            if (i < 4)
            { /* Get destination IP address */
                TemAddr = 0;
                while (recv_buff[k] != ',')
                {
                    TemAddr += recv_buff[k] - '0';
                    k++;
                    TemAddr *= 10;
                }
                TemAddr /= 10;
                CmdPortIP[i] = TemAddr;
                i++;
            }
            else
            { /* Get port number */
                NumPort = 0;
                while (recv_buff[k] != ',')
                {
                    NumPort += (recv_buff[k] - '0') * 256;
                    k++;
                    NumPort *= 10;
                }
                NumPort /= 10;
                k++;
                TemAddr = 0;
                while (recv_buff[k] >= '0' && recv_buff[k] <= '9')
                {
                    TemAddr += (recv_buff[k] - '0');
                    k++;
                    TemAddr *= 10;
                }
                TemAddr /= 10;
                NumPort += TemAddr;
                break;
            }
        }
    }
#if DEBUG
    printf("CmdPortIP: %d.%d.%d.%d\n", CmdPortIP[0], CmdPortIP[1], CmdPortIP[2], CmdPortIP[3]);
    printf("NumPort = %08d\n", NumPort);
#endif
}

/*******************************************************************************
 * Function Name  : CH395_FTPGetFileName
 * Description    : Get file name
 * Input          : recv_buff   - Received data
                    pBuf        - Buffer to store the name
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395_FTPGetFileName(char *recv_buff, char *pBuf)
{
    UINT8 k, i;

    k = 0;
    while ((recv_buff[k] != ' ') && k < strlen(recv_buff))
        k++; /* Find the position of the content to be retrieved */
    k++;
    i = 0;
    while ((recv_buff[k] != '\r') && k < strlen(recv_buff))
    { /* Save the required content */
        if (i > 14)
            return;
        if (recv_buff[k] != '/')
        {
            pBuf[i] = recv_buff[k];
            i++;
        }
        k++;
    }
#if DEBUG
    printf("Name: %s\n", pBuf);
#endif
}

/******************************************************************************
 * Function Name  : CH395_FTPCmdRespond
 * Description    : Responds with the appropriate reply based on the client's command
 * Input          : rThe command code received from the client
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPCmdRespond(char *recv_buff)
{
    UINT8 s;
    UINT8 len;
    UINT16 port;

    if (strncmp("USER", recv_buff, 4) == 0)
    { /* Username */
        printf("USER\n");
        memset((void *)UserName, '\0', sizeof(UserName));
        CH395_FTPGetFileName(recv_buff, UserName);
        CH395_FTPSendData((char *)FTP_SERVICE_CMD[2], strlen(FTP_SERVICE_CMD[2]), ftp.SocketCtl);
    }
    else if (strncmp("PASS", recv_buff, 4) == 0)
    {
#if Access_Authflag /* If username check is required */
        if (strncmp(pUserName, UserName, strlen(pUserName)) == 0)
        { /* Correct username */
            CH395_FTPSendData((char *)FTP_SERVICE_CMD[3], strlen(FTP_SERVICE_CMD[3]), ftp.SocketCtl);
        }
        else
        { /* Incorrect username */
            CH395_FTPSendData((char *)FTP_SERVICE_CMD[4], strlen(FTP_SERVICE_CMD[4]), ftp.SocketCtl);
        }
#else
        CH395_FTPSendData((char *)FTP_SERVICE_CMD[3], strlen(FTP_SERVICE_CMD[3]), ftp.SocketCtl);
#endif
        memset((void *)UserName, '\0', sizeof(UserName));
        CH395_FTPGetFileName(recv_buff, UserName);
        if (strncmp(pPassWord, UserName, strlen(pPassWord)) == 0)
        { /* Check if the password is correct (if valid, write file permission is granted) */
            ftp.AuthFlag = 1;
            printf("have more authority\n");
        }
    }
    else if (strncmp("PORT", recv_buff, 4) == 0)
    { /* Client sends the TCP server port number and IP address created by the client */
        CH395_FTPGetPortNum(recv_buff);
        CH395_FTPSendData((char *)FTP_SERVICE_CMD[5], strlen(FTP_SERVICE_CMD[5]), ftp.SocketCtl);
        CH395_FTPDatClient(1, NumPort, CmdPortIP);
    }
    else if (strncmp("LIST", recv_buff, 4) == 0)
    { /* Get the current list information */
        CH395_FTPSendData((char *)FTP_SERVICE_CMD[6], strlen(FTP_SERVICE_CMD[6]), ftp.SocketCtl);
        CH395_FTPDataReady("0");
    }
    else if (strncmp("STOR", recv_buff, 4) == 0)
    { /* Client requests to upload data (needs to receive data) */
        if (ftp.AuthFlag == 1)
        { /* If permission granted, support upload */
            ftp.CmdReceDatS = 1;
            ftp.FileFlag = 0;
            memset((void *)pFileName, '\0', sizeof(pFileName));
            CH395_FTPGetFileName(recv_buff, pFileName);
            memset((void *)SendBuf, '\0', sizeof(SendBuf));
            sprintf(SendBuf, "%s %s.\r\n", FTP_SERVICE_CMD[7], pFileName);
            CH395_FTPSendData(SendBuf, strlen(SendBuf), ftp.SocketCtl);
        }
        else
        { /* Command not supported without permission, close the connection */
            CH395_FTPSendData((char *)FTP_SERVICE_CMD1[7], strlen(FTP_SERVICE_CMD1[7]), ftp.SocketCtl);
            CH395CMDCloseSocket(ftp.SocketDat);
            CH395CMDCloseSocket(ftp.SocketCtl);
        }
    }
    else if (strncmp("RETR", recv_buff, 4) == 0)
    { /* Client requests to download data (needs to send data) */
        printf("RETR\n");
        if (recv_buff[5] == '/')
        {
            memset((void *)SendBuf, '\0', sizeof(SendBuf));
            CH395_FTPGetFileName(recv_buff, SendBuf);
            len = strlen(ListName);
            sprintf(gFileName, &SendBuf[len]);
        }
        else
        {
            memset((void *)gFileName, '\0', sizeof(gFileName));
            CH395_FTPGetFileName(recv_buff, gFileName);
        }
        memset((void *)SendBuf, '\0', sizeof(SendBuf));
        s = CH395_FTPFileOpen(gFileName, FTP_MACH_FILE);
        if (s)
        { /* File exists, start data transmission */
            sprintf(SendBuf, "%s %s.\r\n", FTP_SERVICE_CMD[7], gFileName);
            CH395_FTPSendData(SendBuf, strlen(SendBuf), ftp.SocketCtl);
            CH395_FTPDataReady(gFileName);
        }
        else
        { /* File does not exist, send error response */
            sprintf(SendBuf, "%s(%s).\r\n", FTP_SERVICE_CMD[8], gFileName);
            CH395_FTPSendData(SendBuf, strlen(SendBuf), ftp.SocketCtl);
            s = CH395CMDCloseSocket(ftp.SocketDat);
            mStopIfError(s);
            ftp.SocketDat = 255;
        }
    }
    else if (strncmp("QUIT", recv_buff, 4) == 0)
    { /* Sign out */
        CH395_FTPSendData((char *)FTP_SERVICE_CMD[9], strlen(FTP_SERVICE_CMD[9]), ftp.SocketCtl);
    }
    else if (strncmp("TYPE", recv_buff, 4) == 0)
    { /* Type of data transferred */
        memset((void *)SendBuf, '\0', sizeof(SendBuf));
        sprintf(SendBuf, "%s %c.\r\n", FTP_SERVICE_CMD1[0], recv_buff[5]);
        CH395_FTPSendData(SendBuf, strlen(SendBuf), ftp.SocketCtl);
    }
    else if (strncmp("syst", recv_buff, 4) == 0)
    { /* System type */
        CH395_FTPSendData((char *)FTP_SERVICE_CMD1[1], strlen(FTP_SERVICE_CMD1[1]), ftp.SocketCtl);
    }
    else if (strncmp("PWD", recv_buff, 3) == 0)
    { /* Show directory */
        if (strncmp("USER", ListName, strlen("USER")) == 0)
        {
            CH395_FTPSendData((char *)FTP_SERVICE_CMD1[10], strlen(FTP_SERVICE_CMD1[10]), ftp.SocketCtl);
        }
        else
        {
            CH395_FTPSendData((char *)FTP_SERVICE_CMD1[2], strlen(FTP_SERVICE_CMD1[2]), ftp.SocketCtl);
        }
    }
    else if (strncmp("CWD", recv_buff, 3) == 0)
    { /* Open Directory */
        memset((void *)ListName, '\0', sizeof(ListName));
        CH395_FTPGetFileName(recv_buff, ListName);
        s = CH395_FTPFileOpen(ListName, FTP_MACH_LIST);
        if (s)
        {
            CH395_FTPSendData((char *)FTP_SERVICE_CMD1[3], strlen(FTP_SERVICE_CMD1[3]), ftp.SocketCtl);
        }
        else
        {
            memset((void *)SendBuf, '\0', sizeof(SendBuf));
            sprintf(SendBuf, "%s.\r\n", FTP_SERVICE_CMD[8]);
            CH395_FTPSendData(SendBuf, strlen(SendBuf), ftp.SocketCtl);
        }
    }
    else if (strncmp("PASV", recv_buff, 4) == 0)
    { /* The client is used to obtain the TCP port number and IP address created by the server */
        CH395_FTPDatServer(1, &port);
        memset((void *)SendBuf, '\0', sizeof(SendBuf));
        sprintf(SendBuf, "%s%s%02d,%02d).\r\n", FTP_SERVICE_CMD1[4], SourIP, (port / 256), (port % 256));
        CH395_FTPSendData(SendBuf, strlen(SendBuf), ftp.SocketCtl);
    }
    else if (strncmp("SIZE", recv_buff, 4) == 0)
    { /* Get file size */
        if (recv_buff[5] == '/')
        {
            memset((void *)SendBuf, '\0', sizeof(SendBuf));
            CH395_FTPGetFileName(recv_buff, SendBuf);
            len = strlen(ListName);
            sprintf(gFileName, &SendBuf[len]);
        }
        else
        {
            memset((void *)pFileName, '\0', sizeof(gFileName));
            CH395_FTPGetFileName(recv_buff, gFileName);
        }
        memset((void *)SendBuf, '\0', sizeof(SendBuf));
        gLEN = CH395_FTPFileSize(gFileName);
        sprintf(SendBuf, "%s%d\r\n", FTP_SERVICE_CMD1[5], gLEN);
        CH395_FTPSendData((char *)SendBuf, strlen(SendBuf), ftp.SocketCtl);
    }
    else if (strncmp("MKD", recv_buff, 3) == 0)
    { /* Create a directory */
        if ((ftp.AuthFlag == 1) && (ftp.ListState == 0))
        { /* Support upload if you have permission */
            memset((void *)pFileName, '\0', sizeof(pFileName));
            CH395_FTPGetFileName(recv_buff, pFileName);
            memset((void *)SendBuf, '\0', sizeof(SendBuf));
            sprintf(SendBuf, "257 MKD %s.\r\n", pFileName);
            CH395_FTPSendData(SendBuf, strlen(SendBuf), ftp.SocketCtl);
            ftp.ListFlag = 3;
            CH395_FTPListRenew(ftp.ListFlag);
        }
        else
        { /* Unsupported command without permission, close the connection */
            CH395_FTPSendData((char *)FTP_SERVICE_CMD1[7], strlen(FTP_SERVICE_CMD1[7]), ftp.SocketCtl);
            CH395CMDCloseSocket(ftp.SocketDat);
            CH395CMDCloseSocket(ftp.SocketCtl);
        }
    }

    else if (strncmp("RMD", recv_buff, 3) == 0)
    { /* Delete a directory  */
        if (ftp.AuthFlag == 1)
        { /* Support delete if you have permission */
            if (recv_buff[4] == '/')
            {
                memset((void *)SendBuf, '\0', sizeof(SendBuf));
                CH395_FTPGetFileName(recv_buff, SendBuf);
                len = strlen(ListName);
                sprintf(pFileName, &SendBuf[len]);
            }
            else
            {
                memset((void *)pFileName, '\0', sizeof(pFileName));
                CH395_FTPGetFileName(recv_buff, pFileName);
            }
            ftp.ListFlag = 4;
            CH395_FTPListRenew(ftp.ListFlag);
            CH395_FTPSendData((char *)FTP_SERVICE_CMD1[6], strlen(FTP_SERVICE_CMD1[6]), ftp.SocketCtl);
        }
        else
        {
            CH395_FTPSendData((char *)FTP_SERVICE_CMD1[7], strlen(FTP_SERVICE_CMD1[7]), ftp.SocketCtl);
        }
    }
    else if (strncmp("DELE", recv_buff, 4) == 0)
    { /* Create a directory */
        if (ftp.AuthFlag == 1)
        { /* If permission is granted, support deletion */
            if (recv_buff[5] == '/')
            {
                memset((void *)SendBuf, '\0', sizeof(SendBuf));
                CH395_FTPGetFileName(recv_buff, SendBuf);
                len = strlen(ListName);
                sprintf(pFileName, &SendBuf[len]);
                printf("pFileName:%s\r\n", pFileName);
            }
            else
            {

                memset((void *)pFileName, '\0', sizeof(pFileName));
                CH395_FTPGetFileName(recv_buff, pFileName);
            }
            ftp.ListFlag = 2;
            CH395_FTPListRenew(ftp.ListFlag);
            CH395_FTPSendData((char *)FTP_SERVICE_CMD1[6], strlen(FTP_SERVICE_CMD1[6]), ftp.SocketCtl);
        }
        else
        {
            CH395_FTPSendData((char *)FTP_SERVICE_CMD1[7], strlen(FTP_SERVICE_CMD1[7]), ftp.SocketCtl);
        }
    }
    else if (strncmp("noop", recv_buff, 4) == 0)
    {
        CH395_FTPSendData((char *)FTP_SERVICE_CMD1[8], strlen(FTP_SERVICE_CMD1[8]), ftp.SocketCtl);
    }
    else if (strncmp("RNFR", recv_buff, 4) == 0)
    {
        if ((ftp.AuthFlag == 1) && (ftp.ListState == 0))
        { /* If permission is granted, support renaming */
            memset((void *)SendBuf, '\0', sizeof(SendBuf));
            if (recv_buff[5] == '/')
            {
                CH395_FTPGetFileName(recv_buff, SendBuf);
                len = strlen(ListName);
                sprintf(pFileName, &SendBuf[len]);
            }
            else
            {
                memset((void *)pFileName, '\0', sizeof(pFileName));
                CH395_FTPGetFileName(recv_buff, pFileName);
            }
            CH395_FTPSendData((char *)FTP_SERVICE_CMD2[0], strlen(FTP_SERVICE_CMD2[0]), ftp.SocketCtl);
        }
        else
        {
            CH395_FTPSendData((char *)FTP_SERVICE_CMD1[7], strlen(FTP_SERVICE_CMD1[7]), ftp.SocketCtl);
        }
    }
    else if (strncmp("RNTO", recv_buff, 4) == 0)
    {
        if ((ftp.AuthFlag == 1) && (ftp.ListState == 0))
        { /* If permission is granted, support renaming */
            memset((void *)gFileName, '\0', sizeof(gFileName));
            if (recv_buff[5] == '/')
            {
                memset((void *)SendBuf, '\0', sizeof(SendBuf));
                CH395_FTPGetFileName(recv_buff, SendBuf);
                len = strlen(ListName);
                sprintf(gFileName, &SendBuf[len]);
            }
            else
            {
                CH395_FTPGetFileName(recv_buff, gFileName);
            }
            ftp.ListFlag = 5;
            CH395_FTPListRenew(ftp.ListFlag);
            CH395_FTPSendData((char *)FTP_SERVICE_CMD2[1], strlen(FTP_SERVICE_CMD2[1]), ftp.SocketCtl);
        }
        else
        {
            CH395_FTPSendData((char *)FTP_SERVICE_CMD1[7], strlen(FTP_SERVICE_CMD1[7]), ftp.SocketCtl);
        }
    }
    else
    { /* Command not supported */
        CH395_FTPSendData((char *)FTP_SERVICE_CMD1[9], strlen(FTP_SERVICE_CMD1[9]), ftp.SocketCtl);
    }
}
