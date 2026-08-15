/********************************** (C) COPYRIGHT *********************************
 * File Name          : ch395FTPCMD.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 chip FTP client application - FTP command code
 **********************************************************************************/
#include "Ch395.h"
#include "CH395FTPINC.H"
#include "string.h"

FTP ftp;
char send_buff[536]; /* Data buffer for sending */
UINT16 ListNum = 0;

/********************************************************************************
 * Function Name  : CH395_FTPLoginUser
 * Description    : Authenticate username
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPLoginUser(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "USER %s\r\n", pUserName);
#if DEBUG
    printf("USER :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPLoginPass
 * Description    : Authenticate password
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPLoginPass(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "PASS %s\r\n", pPassword);
#if DEBUG
    printf("PASS :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPSyst
 * Description    : Query server system type
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPSyst(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "SYST\r\n");
#if DEBUG
    printf("SYST :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPList
 * Description    : Get file list, filenames and timestamps
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPList(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "LIST\r\n");
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPNlst
 * Description    : Get file list
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPNlst(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "NLST\r\n");
#if DEBUG
    printf("NLST :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPType
 * Description    : Set file transfer type
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPType(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "TYPE A\r\n");
#if DEBUG
    printf("TYPE :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPSize
 * Description    : Get file information
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPSize(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "SIZE %s\r\n", FileName);
#if DEBUG
    printf("SIZE :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPRetr
 * Description    : Download file
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPRetr(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "RETR %s\r\n", FileName);
#if DEBUG
    printf("RETR :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPCdup
 * Description    : Go to parent directory
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPCdup(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "CDUP\r\n");
#if DEBUG
    printf("CDUP :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/********************************************************************************
 * Function Name  : CH395_FTPMkd
 * Description    : Create directory
 * Input          : ReName
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPMkd(UINT8 ReName)
{
    memset(send_buff, '\0', sizeof(send_buff));
    if (ReName)
    {
        ListNum++;
        sprintf(send_buff, "MKD %s%d\r\n", ListName, ListNum);
    }
    else
        sprintf(send_buff, "MKD %s\r\n", ListName);
    sprintf(ListMdk, &send_buff[4]); /* Save directory name */
#if DEBUG
    printf("MKD :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPRmd
 * Description    : Remove directory
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPRmd(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "RMD %s\r\n", ListName);
#if DEBUG
    printf("RMD :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPAppe
 * Description    : Upload file
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPAppe(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "APPE %s\r\n", FileName);
#if DEBUG
    printf("STOR :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/********************************************************************************
 * Function Name  : CH395_FTPAllo
 * Description    : Download file
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPAllo(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "ALLO 500000\r\n");
#if DEBUG
    printf("ALLO :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/********************************************************************************
 * Function Name  : CH395_FTPNoop
 * Description    : Send no-op command
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPNoop(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "NOOP\r\n");
#if DEBUG
    printf("NOOP :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPPort
 * Description    : Set connection port number
 * Input          : port
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPPort(UINT16 port)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "PORT %s%02d,%02d\r\n", SourIP, port / 256, port % 256);
#if DEBUG
    printf("PORT :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/*********************************************************************************
 * Function Name  : CH395_FTPCwd
 * Description    : Open directory
 * Input          : index
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPCwd(UINT8 index)
{
    memset(send_buff, '\0', sizeof(send_buff));
    if (index == 1)
        sprintf(send_buff, "CWD /%s\r\n", ListMdk);
    else
        sprintf(send_buff, "CWD /%s\r\n", ListName);
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
    Delay_Ms(500);
}

/*********************************************************************************
 * Function Name  : CH395_FTPQuit
 * Description    : Logout
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void CH395_FTPQuit(void)
{
    memset(send_buff, '\0', sizeof(send_buff));
    sprintf(send_buff, "QUIT\r\n");
#if DEBUG
    printf("QUIT :%s\n", send_buff);
#endif
    CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
}

/********************************************************************************
 * Function Name  : CH395_FTPCheckLogin
 * Description    : Verify login handshake signal
 * Input          : recv_buff   - Handshake information
                    check_type  - Type of check
 * Output         : None
 * Return         : Status
*********************************************************************************/
UINT8 CH395_FTPCheckLogin(char *recv_buff)
{
    if (strncmp("220", recv_buff, 3) == 0)
    { /* Connection successful */
        CH395_FTPLoginUser();
    }
    else if (strncmp("331", recv_buff, 3) == 0)
    { /* Username correct */
        CH395_FTPLoginPass();
    }
    else if (strncmp("230", recv_buff, 3) == 0)
    { /* Login successful */
        ftp.CmdStatus = FTP_MACH_LOGINSUC;
    }
    else if (strncmp("530", recv_buff, 3) == 0)
    { /* Login failed */
        CH395CMDCloseSocket(ftp.SocketCtl);
    }
    else if (strncmp("221", recv_buff, 3) == 0)
    {                                       /* Logout */
        CH395CMDCloseSocket(ftp.SocketCtl); /* Close connection */
    }
    return FTP_ERR_LOGIN;
}

/********************************************************************************
 * Function Name  : CH395_FTPCheckSearch
 * Description    : Verify search file handshake signal
 * Input          : recv_buff   - Handshake information
 * Output         : None
 * Return         : Status
*********************************************************************************/
UINT8 CH395_FTPCheckSearch(char *recv_buff)
{
    printf("#####");

    if (strncmp("200 Type", recv_buff, 8) == 0)
    { /* File type */
        CH395_FTPDatServer(1, &ftp.SouPort);
        CH395_FTPPort(ftp.SouPort);
    }
    else if (strncmp("200 PORT", recv_buff, 8) == 0)
    {
        CH395_FTPList();
        ftp.CmdDataS = FTP_MACH_RECEDATA;
    }
    else if (strncmp("150", recv_buff, 3) == 0)
    { /* File list retrieval successful */
        ftp.CmdDataS = FTP_MACH_RECEDATA;
    }
    else if (strncmp("125", recv_buff, 3) == 0)
    { /* File list retrieval successful */
        ftp.CmdDataS = FTP_MACH_RECEDATA;
    }
    else if (strncmp("226", recv_buff, 3) == 0)
    { /* List information complete */
        if (ftp.InterCmdS == FTP_MACH_FINDFILE)
            ftp.CmdStatus = FTP_MACH_SEARCHSUC;
        if (ftp.FindList == 1)
        {
            ftp.FindList = 0;
            CH395_FTPCwd(0);
        }
        ftp.InterCmdS = 0;
    }
    else if (strncmp("425", recv_buff, 3) == 0)
    { /* Unable to establish data connection */
        CH395_FTPQuit();
    }
    else if (strncmp("250", recv_buff, 3) == 0)
    { /* Open directory */
        ftp.InterCmdS = FTP_MACH_FINDFILE;
        CH395_FTPType();
    }
    else if (strncmp("221", recv_buff, 3) == 0)
    {                                       /* Logout */
        CH395CMDCloseSocket(ftp.SocketCtl); /* Close connection */
    }
    else
        return FTP_ERR_SEARCH;
    return FTP_CHECK_SUCCESS;
}

/*********************************************************************************
 * Function Name  : CH395_FTPCheckGetfile
 * Description    : Verify file download handshake signal
 * Input          : recv_buff   - Handshake information
 * Output         : None
 * Return         : Status
*********************************************************************************/
UINT8 CH395_FTPCheckGetfile(char *recv_buff)
{
    if (strncmp("200 Type", recv_buff, 8) == 0)
    { /* File type */
        CH395_FTPSize();
    }
    else if (strncmp("213", recv_buff, 3) == 0)
    { /* File information */
        CH395_FTPDatServer(1, &ftp.SouPort);
        CH395_FTPPort(ftp.SouPort);
    }
    else if (strncmp("200 PORT", recv_buff, 8) == 0)
    {
        CH395_FTPRetr();
    }
    else if (strncmp("150", recv_buff, 3) == 0)
    {
        ftp.CmdDataS = FTP_MACH_RECEDATA;
        memset(send_buff, '\0', sizeof(send_buff));
    }
    else if (strncmp("550", recv_buff, 3) == 0)
    { /* File not found */
        CH395_FTPQuit();
    }
    else if (strncmp("226", recv_buff, 3) == 0)
    { /* Transfer complete */
        ftp.CmdStatus = FTP_MACH_GETSUC;
    }
    else if (strncmp("221", recv_buff, 3) == 0)
    {                                       /* Logout */
        CH395CMDCloseSocket(ftp.SocketCtl); /* Close connection */
    }
    else
        return FTP_ERR_GETFILE;
    return (FTP_CHECK_SUCCESS);
}

/*********************************************************************************
 * Function Name  : CH395_FTPCheckPutfile
 * Description    : Verify file upload handshake signal
 * Input          : recv_buff   - Handshake information
 * Output         : None
 * Return         : Status
 *********************************************************************************/
UINT8 CH395_FTPCheckPutfile(char *recv_buff)
{
    if (strncmp("250", recv_buff, 3) == 0)
    { /* Return to the previous directory */
        if (ftp.InterCmdS == FTP_MACH_MKDLIST)
        {
            CH395_FTPMkd(0);
        }
        else if (ftp.InterCmdS == FTP_MACH_PUTFILE)
        {
            CH395_FTPType();
        }
    }
    else if (strncmp("257", recv_buff, 3) == 0)
    {                                     /* Directory created successfully */
        ftp.InterCmdS = FTP_MACH_PUTFILE; /* Upload file */
        memset(send_buff, '\0', sizeof(send_buff));
        sprintf(send_buff, "CWD %s", ListMdk);
        CH395_SocketSendData(send_buff, strlen(send_buff), ftp.SocketCtl);
    }
    else if (strncmp("550", recv_buff, 3) == 0)
    { /* Directory name already exists */
#if 1
        CH395_FTPMkd(1); /* If the directory name exists, create another directory with a new name (e.g., TXET1) */
#else
        CH395_FTPCwd(0); /* If the directory name exists, just open it */
#endif
    }
    if (strncmp("200 Type", recv_buff, 8) == 0)
    { /* Format type */
        CH395_FTPDatServer(1, &ftp.SouPort);
        CH395_FTPPort(ftp.SouPort);
    }
    else if (strncmp("200 PORT", recv_buff, 8) == 0)
    {
        CH395_FTPAppe();
    }
    else if (strncmp("125", recv_buff, 3) == 0)
    {                                     /* Upload file request successful */
        ftp.CmdDataS = FTP_MACH_SENDDATA; /* Data needs to be sent */
    }
    else if (strncmp("150", recv_buff, 3) == 0)
    {                                     /* Upload file request successful */
        ftp.CmdDataS = FTP_MACH_SENDDATA; /* Data needs to be sent */
    }
    else if (strncmp("226", recv_buff, 3) == 0)
    { /* Upload finished */
        ftp.CmdStatus = FTP_MACH_PUTSUC;
    }
    else if (strncmp("221", recv_buff, 3) == 0)
    {                                       /* Logout */
        CH395CMDCloseSocket(ftp.SocketCtl); /* Close connection */
    }
    else
        return FTP_ERR_PUTFILE;
    return (FTP_CHECK_SUCCESS);
}

/********************************************************************************
 * Function Name  : CH395_FTPCmdRespond
 * Description    : Verify login handshake signal
 * Input          : recv_buff    - Handshake information
                    check_type   - Type of check
 * Output         : None
 * Return         : Status
 *********************************************************************************/
UINT8 CH395_FTPCmdRespond(char *recv_buff, UINT8 check_type)
{
    UINT8 s;

    if (strncmp("421", recv_buff, 3) == 0)
    { /* Server unexpectedly disconnected */
        return (FTP_CHECK_SUCCESS);
    }
    switch (check_type)
    {
    case FTP_CMD_LOGIN: /* Verify return information during login process */
        s = CH395_FTPCheckLogin(recv_buff);
        return s;
    case FTP_CMD_SEARCH: /* Verify return information during file search process */
        s = CH395_FTPCheckSearch(recv_buff);
        return s;
    case FTP_CMD_GETFILE: /* Verify return information during file download process */
        s = CH395_FTPCheckGetfile(recv_buff);
        return s;
    case FTP_CMD_PUTFILE: /* Verify return information during file upload process */
        s = CH395_FTPCheckPutfile(recv_buff);
        return s;
    default:
        return FTP_ERR_UNKW;
    }
}

/********************************************************************************
 * Function Name  : CH395_FTPFindList
 * Description    : Verify directory name
 * Input          : Received list data
 * Output         : None
 * Return         : Status
 *********************************************************************************/
UINT8 CH395_FTPFindList(char *pReceiveData)
{
    UINT32 len, i, j;

    len = ftp.RecDatLen;

    for (i = 0; i < len; i++)
    {
        if (strncmp(ListName, &pReceiveData[i], strlen(ListName)) == 0)
        {
            j = i;
            while (strncmp("<DIR>", &pReceiveData[j], strlen("<DIR>")))
            {
                j--;
                if (pReceiveData[j] == 'M')
                    return FALSE;
            }
#if DEBUG
            printf("*********\nfind list\n*********\n");
#endif
            return FTP_CHECK_SUCCESS; /* Found the specified directory name */
        }
    }
    return FTP_ERR_UNLIST;
}

/********************************************************************************
 * Function Name  : CH395_FTPFindFile
 * Description    : Verify file name
 * Input          : Received file list data
 * Output         : None
 * Return         : Status
 *********************************************************************************/
UINT8 CH395_FTPFindFile(char *pReceiveData)
{
    // Check if FileName exists in pReceiveData
    if (strstr((char *)pReceiveData, FileName) != NULL)
    {
#if DEBUG
        printf("*********\nfind file\n*********\n");
#endif
        return FTP_CHECK_SUCCESS; // Found the specified file name
    }
#if DEBUG
    printf("%s\n%s\n", FileName, pReceiveData);
#endif
    return FTP_ERR_UNFILE;
}

/*********************************************************************************
 * Function Name  : CH395_FTPLogin
 * Description    : Login
 * Input          : None
 * Output         : None
 * Return         : Function execution result
 *********************************************************************************/
UINT8 CH395_FTPLogin(void)
{
    if (ftp.CmdFirtS == 0)
    {
        ftp.CmdFirtS = 1;
        ftp.FileCmd = FTP_CMD_LOGIN; /* Enter file search state */
        CH395_FTPCtlClient(0);
    }
    if (ftp.CmdStatus == FTP_MACH_LOGINSUC)
    { /* Login successful */
#if DEBUG
        printf("************\nlogin success\n*********\n");
#endif
        ftp.CmdFirtS = 0;
        ftp.CmdStatus = 0;
        ftp.FileCmd = 0;
        return FTP_COMMAND_SUCCESS;
    }
    return FTP_COMMAND_CONTINUE;
}

/*********************************************************************************
 * Function Name  : CH395_FTPSearch
 * Description    : Search for a file
 * Input          : pListName - Directory name where the file is located
 *                  pFileName - File name to search in the directory
 * Output         : None
 * Return         : Status
 *********************************************************************************/
UINT8 CH395_FTPSearch(char *pListName, char *pFileName)
{
    if (ftp.CmdFirtS == 0)
    {
        ftp.CmdFirtS = 1;
        ftp.FileCmd = FTP_CMD_SEARCH;      /* Enter file search state */
        ftp.InterCmdS = FTP_MACH_FINDLIST; /* Find directory */
        memset((void *)ListName, '\0', sizeof(ListName));
        sprintf(ListName, pListName); /* Input folder name */
        memset((void *)FileName, '\0', sizeof(FileName));
        sprintf(FileName, pFileName); /* Input file name to search */
        CH395_FTPType();
    }
    if (ftp.TcpStatus == FTP_MACH_DISCONT)
    {
        if (ftp.CmdStatus == FTP_MACH_SEARCHSUC)
        { /* File search completed */
#if DEBUG
            printf("**********\nsearch success\n*********\n");
#endif
            ftp.CmdFirtS = 0;
            ftp.CmdStatus = 0;
            ftp.FileCmd = 0;
            return FTP_COMMAND_SUCCESS;
        }
    }
    return FTP_COMMAND_CONTINUE;
}

/*********************************************************************************
 * Function Name  : CH395_FTPGetFile
 * Description    : Download file
 * Input          : File name to download
 * Output         : None
 * Return         : Status
 *********************************************************************************/
UINT8 CH395_FTPGetFile(char *pFileName)
{
    if (ftp.CmdFirtS == 0)
    {
        ftp.CmdFirtS = 1;
        ftp.FileCmd = FTP_CMD_GETFILE;    /* Enter file download state */
        ftp.InterCmdS = FTP_MACH_GETFILE; /* Download file */
        memset((void *)FileName, '\0', sizeof(FileName));
        sprintf(FileName, pFileName); /* Input file name to download */
        CH395_FTPType();
    }
    if (ftp.TcpStatus == FTP_MACH_DISCONT)
    {
        if (ftp.CmdStatus == FTP_MACH_GETSUC)
        { /* File download successful */
#if DEBUG
            printf("*********\ngetfile success\n*********\n");
#endif
            ftp.CmdFirtS = 0;
            ftp.CmdStatus = 0;
            ftp.FileCmd = 0;
            return FTP_COMMAND_SUCCESS;
        }
    }
    return FTP_COMMAND_CONTINUE;
}

/*********************************************************************************
 * Function Name  : CH395_FTPPutFile
 * Description    : Upload file
 * Input          : pListNAme - Directory name where the file should be saved
                    pFileName - File name to upload
 * Output         : None
 * Return         : Status
 *********************************************************************************/
UINT8 CH395_FTPPutFile(char *pListNAme, char *pFileName)
{
    if (ftp.CmdFirtS == 0)
    {
        ftp.CmdFirtS = 1;
        ftp.FileCmd = FTP_CMD_PUTFILE;    /* Enter file upload state */
        ftp.InterCmdS = FTP_MACH_MKDLIST; /* Create directory */
        memset((void *)ListName, '\0', sizeof(ListName));
        sprintf(ListName, pListNAme); /* Input directory name for upload */
        memset((void *)FileName, '\0', sizeof(FileName));
        sprintf(FileName, pFileName); /* Input file name for upload */
        CH395_FTPCdup();
    }
    if (ftp.TcpStatus == FTP_MACH_DISCONT)
    {
        if (ftp.CmdStatus == FTP_MACH_PUTSUC)
        { /* File upload successful */
#if DEBUG
            printf("*********\nputfile success\n*********\n");
#endif
            ftp.CmdFirtS = 0;
            ftp.CmdStatus = 0;
            ftp.FileCmd = 0;
            return FTP_COMMAND_SUCCESS;
        }
    }
    return FTP_COMMAND_CONTINUE;
}
