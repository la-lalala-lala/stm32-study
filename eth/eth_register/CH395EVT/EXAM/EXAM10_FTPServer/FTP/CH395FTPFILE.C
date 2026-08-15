/********************************** (C) COPYRIGHT *********************************
* File Name          : CH395FTPFILE.C
* Author             : WCH
* Version            : V1.1
* Date               : 2014/8/1
* Description        : CH395 chip FTP server application - Data handling part

**********************************************************************************/
const char *Atest = "0123456789abcdefghijklmnopqrstuvwxyz";
const char *Btest = "ch395 FTP server demo";
const char *Ctest = "www.wch.cn 2025-03-18";
const char *FileNature = "03-18-25  10:10AM         ";
const char *ListNature = "03-18-25  11:00AM  <DIR>       ";
#include "debug.h"
#include "CH395FTPINC.H"
#include "CH395INC.H"
#include "string.h"
extern char pFileName[16];
extern char gFileName[16];
extern char SendBuf[SendBufLen];

/* This program simulates files.
 * The root directory initially contains a folder and two text documents;
 * The folder is named "USER" and contains one text document;
 * The root directory supports up to four files; if more than four files are added, no more data can be uploaded;
 * The "USER" directory supports one file, and any uploaded file will overwrite the existing one.
 */
char RootBuf1[64];
char RootBuf2[64];
char RootBuf3[64];
char RootBuf4[64];
char UserBuf[64];
char NameList[200];
char NameFile[100];
char ListUser[50];
char ListRoot[450];
char *pFile;
UINT16 FileLen;

/*******************************************************************************
 * Function Name  : mInitFtpVari
 * Description    : Initialize the list and files, used for simulating files and directory lists
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void mInitFtpList()
{
    memset((void *)RootBuf1, '\0', sizeof(RootBuf1));
    sprintf(RootBuf1, Atest);
    memset((void *)RootBuf2, '\0', sizeof(RootBuf2));
    sprintf(RootBuf2, Btest);
    memset((void *)RootBuf3, '\0', sizeof(RootBuf3));
    memset((void *)RootBuf4, '\0', sizeof(RootBuf4));
    memset((void *)UserBuf, '\0', sizeof(UserBuf));
    sprintf(UserBuf, Ctest);
    memset((void *)ListRoot, '\0', sizeof(ListRoot));
    sprintf(ListRoot, "%s%4d A.txt\r\n%s%4d B.txt\r\n%sUSER\r\n", FileNature, strlen(RootBuf1), FileNature, strlen(RootBuf1), ListNature); // Current directory
    memset((void *)ListUser, '\0', sizeof(ListUser));
    sprintf(ListUser, "%s%4d H.txt\r\n", FileNature, strlen(UserBuf)); /* Sub-directory */
    memset((void *)NameFile, '\0', sizeof(NameFile));
    sprintf(NameFile, "USER\r\n");
    memset((void *)NameList, '\0', sizeof(NameList));
    sprintf(NameList, "A.txt#0B.txt#1");
    ftp.BufStatus = ftp.BufStatus | 0x03; /* buf1 and buf2 are not empty */
}

/******************************************************************************
 * Function Name  : CH395_FTPListRenew
 * Description    : Update directory
 * Input          : index - the status to be updated (0 for adding a new file, other values update file size information)
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPListRenew(UINT8 index)
{
    UINT8 i, k;

#if DEBUG
    printf("pFileName:\n%s\n", pFileName);
#endif
    if (ftp.ListState == 0)
    { /* Root directory */
        if (index == 2 || index == 4)
        { /* Delete file */
            i = 0;
            while ((strncmp(pFileName, &ListRoot[i], strlen(pFileName))) && (i < strlen(ListRoot)))
                i++;
            if (i >= strlen(ListRoot))
                return; /* File not found, exit */
            k = i;
            while ((ListRoot[k] != '\n') && (k < strlen(ListRoot)))
                k++; /* Move to end of file attributes */
            if (k >= strlen(ListRoot))
                return; /* Error, exit */
            k++;
            while ((ListRoot[i] != '\n') && i)
                i--; /* Move to start of previous file attributes */
            if (i)
                ListRoot[i + 1] = '\0';
            else
                ListRoot[i] = '\0'; /* This file is the first file */
            memset(SendBuf, '\0', sizeof(SendBuf));
            sprintf(SendBuf, "%s%s", ListRoot, &ListRoot[k]); /* Delete file attribute info */
            memset(ListRoot, '\0', sizeof(ListRoot));
            sprintf(ListRoot, SendBuf);
            if (index == 2)
            {
                i = 0;
                while ((strncmp(pFileName, &NameList[i], strlen(pFileName))) && (i < strlen(NameList)))
                    i++;
                if (i >= strlen(NameList))
                    return;
                NameList[i] = '\0';
                i += strlen(pFileName);
                switch (NameList[i + 1])
                { /* Clear buffer */
                case '0':
                    ftp.BufStatus = ftp.BufStatus & 0xfe;
                    break;
                case '1':
                    ftp.BufStatus = ftp.BufStatus & 0xfd;
                    break;
                case '2':
                    ftp.BufStatus = ftp.BufStatus & 0xfb;
                    break;
                case '3':
                    ftp.BufStatus = ftp.BufStatus & 0xf7;
                    break;
                }
                i += 2;
                sprintf(SendBuf, "%s%s", NameList, &NameList[i]);
                memset(NameList, '\0', sizeof(NameList));
                sprintf(NameList, SendBuf);
            }
            else
            {
                i = 0;
                while ((strncmp(pFileName, &NameFile[i], strlen(pFileName))) && (i < strlen(NameFile)))
                    i++;
                if (i >= strlen(NameFile))
                    return;
                NameFile[i] = '\0';
                i += strlen(pFileName);
                i += 2;
                sprintf(SendBuf, "%s%s", NameFile, &NameFile[i]);
                memset(NameFile, '\0', sizeof(NameFile));
                sprintf(NameFile, SendBuf);

#if DEBUG
                printf("NameFile:\n%s\n", NameFile);
#endif
            }
        }
        else if (index == 1 || index == 3)
        { /* Add new file */
            memset(SendBuf, '\0', sizeof(SendBuf));
            if (index == 1)
            {
                sprintf(SendBuf, "%s%s%4d %s\r\n", ListRoot, FileNature, FileLen, pFileName);
            }
            else
            {
                sprintf(SendBuf, "%s%s%s\r\n", ListRoot, ListNature, pFileName);
                strcat(NameFile, pFileName);
                strcat(NameFile, "\r\n");
            }
            memset(ListRoot, '\0', sizeof(ListRoot));
            sprintf(ListRoot, SendBuf);
        }
        else if (index == 0 || index == 5)
        { /* Replace or rename file */
            i = 0;
            while ((strncmp(pFileName, &ListRoot[i], strlen(pFileName))) && i < strlen(ListRoot))
                i++;
            if (i >= strlen(ListRoot))
                return;
            if (index == 0)
            { /* Overwrite file */
                k = i;
                i -= 2;
                while (ListRoot[i] != ' ')
                {
                    i--;
                    if (i < 2)
                        return;
                }
                ListRoot[i] = '\0';
                memset(SendBuf, '\0', sizeof(SendBuf));
                sprintf(SendBuf, "%s %4d %s", ListRoot, FileLen, &ListRoot[k]);
                memset(ListRoot, '\0', sizeof(ListRoot));
                sprintf(ListRoot, SendBuf);
            }
            else
            { /* Rename */
                k = i;
                while (ListRoot[k] != '\r')
                {
                    k++;
                    if (k >= strlen(ListRoot))
                        return;
                }
                ListRoot[i] = '\0';
                memset(SendBuf, '\0', sizeof(SendBuf));
                sprintf(SendBuf, "%s%s%s", ListRoot, gFileName, &ListRoot[k]);
                memset(ListRoot, '\0', sizeof(ListRoot));
                sprintf(ListRoot, SendBuf);
                i = 0;
                while (pFileName[i] != '.')
                {
                    i++;
                    if (i >= strlen(pFileName))
                        break;
                }
                if (i < strlen(pFileName))
                { /* Rename to a file */
                    i = 0;
                    while ((strncmp(pFileName, &NameList[i], strlen(pFileName))))
                    {
                        i++;
                        if (i >= strlen(NameList))
                            return;
                    }
                    k = i;
                    while (NameList[k] != '#')
                    {
                        k++;
                        if (k >= strlen(NameList))
                            return;
                    }
                    NameList[i] = '\0';
                    sprintf(SendBuf, "%s%s%s", NameList, gFileName, &NameList[k]);
                    memset(NameList, '\0', sizeof(NameList));
                    sprintf(NameList, SendBuf);
                }
                else
                { /* Rename to a folder */
                    i = 0;
                    while ((strncmp(pFileName, &NameFile[i], strlen(pFileName))))
                    {
                        i++;
                        if (i >= strlen(NameFile))
                            return;
                    }
                    NameFile[i] = '\0';
                    strcat(NameFile, gFileName);
                    strcat(NameFile, "\r\n");
                }
            }
        }
    }
    else
    { /* "USER" subdirectory */
        if (index == 2 || index == 4)
        { /* Delete file */
            i = 0;
            while ((strncmp(pFileName, &ListUser[i], strlen(pFileName))) && (i < strlen(ListUser)))
                i++;
            if (i >= strlen(ListUser))
                return; /* File not found, exit */
            k = i;
            while ((ListUser[k] != '\n') && (k < strlen(ListUser)))
                k++; /* Move to end of file attributes */
            if (k >= strlen(ListUser))
                return; /* Error, exit */
            k++;
            while ((ListUser[i] != '\n') && i)
                i--; /* Move to start of previous file attributes */
            if (i)
                ListUser[i + 1] = '\0';
            else
                ListUser[i] = '\0'; /* This file is the first file */
            memset(SendBuf, '\0', sizeof(SendBuf));
            sprintf(SendBuf, "%s%s", ListUser, &ListUser[k]); /* Delete file attribute info */
            memset(ListUser, '\0', sizeof(ListUser));
            sprintf(ListUser, SendBuf);
        }
        else if (index == 1 || index == 3)
        { /* Add new file */
            memset(SendBuf, '\0', sizeof(SendBuf));
            if (index == 1)
            {
                sprintf(SendBuf, "%s%s%4d %s\r\n", ListUser, FileNature, FileLen, pFileName);
            }
            else
            {
                sprintf(SendBuf, "%s%s%s\r\n", ListUser, ListNature, pFileName);
                strcat(NameFile, pFileName);
                strcat(NameFile, "\r\n");
            }
            memset(ListUser, '\0', sizeof(ListUser));
            sprintf(ListUser, SendBuf);
        }
        else if (index == 0 || index == 5)
        { /* Replace or rename file */
            i = 0;
            while ((strncmp(pFileName, &ListUser[i], strlen(pFileName))) && i < strlen(ListUser))
                i++;
            if (i >= strlen(ListUser))
                return;
            if (index == 0)
            { /* Overwrite file */
                k = i;
                i -= 2;
                while (ListUser[i] != ' ')
                {
                    i--;
                    if (i < 2)
                        return;
                }
                ListUser[i] = '\0';
                memset(SendBuf, '\0', sizeof(SendBuf));
                sprintf(SendBuf, "%s %4d %s", ListUser, FileLen, &ListUser[k]);
                memset(ListUser, '\0', sizeof(ListUser));
                sprintf(ListUser, SendBuf);
            }
            else
            { /* Rename */
                k = i;
                while (ListUser[k] != '\r')
                {
                    k++;
                    if (k >= strlen(ListUser))
                        return;
                }
                ListUser[i] = '\0';
                memset(SendBuf, '\0', sizeof(SendBuf));
                sprintf(SendBuf, "%s%s%s", ListUser, gFileName, &ListUser[k]);
                memset(ListUser, '\0', sizeof(ListUser));
                sprintf(ListUser, SendBuf);
            }
        }
    }
}
/*******************************************************************************
 * Function Name  : CH395_FTPFileSize
 * Description    : Get file size
 * Input          : File name
 * Output         : None
 * Return         : None
 *******************************************************************************/
UINT32 CH395_FTPFileSize(char *pName)
{
    UINT8 i;

    i = 0;
    if (ftp.ListState == 0)
    {
        while ((strncmp(pName, &NameList[i], strlen(pName))))
        {
            i++;
            if (i >= strlen(NameList))
            {
                return FALSE;
            }
        }
        while (NameList[i] != '#')
        {
            i++;
            if (i >= strlen(NameList))
            {
                return FALSE;
            }
        }
        switch (NameList[i + 1])
        {
        case '0':
            pFile = RootBuf1;
            break;
        case '1':
            pFile = RootBuf2;
            break;
        case '2':
            pFile = RootBuf3;
            break;
        case '3':
            pFile = RootBuf4;
            break;
        default:
            return FALSE;
        }
    }
    else
    {
        while ((strncmp(pName, &ListUser[i], strlen(pName))))
        {
            i++;
            if (i >= strlen(ListUser))
            {
                return FALSE;
            }
        }
        pFile = UserBuf;
    }
    return strlen(pFile);
}

/******************************************************************************
 * Function Name  : CH395_FTPFileOpen
 * Description    : Open file
 * Input          : pName - File name
                    index - Index
 * Output         : None
 * Return         : None
*******************************************************************************/
UINT8 CH395_FTPFileOpen(char *pName, UINT8 index)
{
    UINT8 k;
    char *pList;

    if (index == FTP_MACH_LIST)
    { /* Open directory and check if the directory name exists */
        if (strlen(pName) == 0)
        {
            ftp.ListState = 0; /* Open root directory */
            return TRUE;
        }
        else if (strncmp("USER", pName, strlen(pName)) == 0)
        {
            ftp.ListState = 1; /* Open directory named USER */
            return TRUE;
        }
        else
        {
            k = 0;
            while (strncmp(pName, &NameFile[k], strlen(pName)))
            {
                k++;
                if (k >= strlen(NameFile))
                {
                    ftp.ListState = 0xFF; /* Directory does not exist */
                    return FALSE;         /* Directory does not exist */
                }
            }
            ftp.ListState = 2; /* Open directory named USER */
            return TRUE;
        }
    }
    else if (index == FTP_MACH_FILE)
    { /* Open file and check if the file exists */
        if (ftp.ListState == 0)
            pList = NameList;
        else
            pList = ListUser;
        for (k = 0; k < strlen(pList); k++)
        {
            if (strncmp(pName, &pList[k], strlen(pName)) == 0)
            {
                return TRUE;
            }
        }
        return FALSE;
    }
    else
        return FALSE;
}

/*******************************************************************************
 * Function Name  : CH395_FTPFileRead
 * Description    : Read file
 * Input          : File name
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395_FTPFileRead(char *pName)
{
    UINT32 len;

    memset((void *)SendBuf, '\0', sizeof(SendBuf));
    if (pName[0] == '0')
    { /* Open list information */
        if (ftp.ListState == 0)
        {
            ftp.DataOver = TRUE;
            sprintf(SendBuf, "%s", ListRoot);
        }
        else if (ftp.ListState == 1)
        {
            ftp.DataOver = TRUE;
            sprintf(SendBuf, "%s", ListUser);
        }
        else if (ftp.ListState == 2)
        {
            ftp.DataOver = TRUE;
        }
    }
    else
    {
        len = CH395_FTPFileSize(pName); /* Get the length of the file to read */
        if (len < SendBufLen)
        {
            memset(SendBuf, '\0', sizeof(SendBuf));
            sprintf(SendBuf, pFile);
        }
    }
}

/*******************************************************************************
 * Function Name  : CH395_FTPFileWrite
 * Description    : Receive file data and store it in the buffer
 * Input          : recv_buff - Data
                    len - Length
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395_FTPFileWrite(char *recv_buff, UINT16 len)
{
    UINT8 i, k;

    FileLen = len;
    i = 0;
    if (ftp.FileFlag)
        return;
    if (FileLen > 64)
    {
        ftp.FileFlag = 1;
        memset(recv_buff, '\0', strlen(recv_buff));
        sprintf(recv_buff, "only support the size of file less than 64 bytes");
        FileLen = strlen(recv_buff);
    }
    if (ftp.ListState == 0)
    {
        while ((strncmp(pFileName, &NameList[i], strlen(pFileName))) && (i < strlen(NameList)))
            i++;
        if (i >= strlen(NameList))
        { /* New file */
            ftp.ListFlag = 1;
            for (k = 0; k < 4; k++)
            {
                if (((ftp.BufStatus >> k) & 0x01) == 0)
                    break;
            }
        }
        else
        {
            ftp.ListFlag = 0; /* Overwrite file */
            i = i + strlen(pFileName) + 1;
            k = NameList[i] - '0';
        }
        switch (k)
        {
        case 0:
            memset((void *)RootBuf1, '\0', sizeof(RootBuf1));
            sprintf(RootBuf1, recv_buff);
            ftp.BufStatus = ftp.BufStatus | 0x01;
            if (ftp.ListFlag)
            {
                strcat(NameList, pFileName);
                strcat(NameList, "#0");
            }
            break;
        case 1:
            memset((void *)RootBuf2, '\0', sizeof(RootBuf2));
            sprintf(RootBuf2, recv_buff);
            ftp.BufStatus = ftp.BufStatus | 0x02;
            if (ftp.ListFlag)
            {
                strcat(NameList, pFileName);
                strcat(NameList, "#1");
            }
            break;
        case 2:
            memset((void *)RootBuf3, '\0', sizeof(RootBuf3));
            sprintf(RootBuf3, recv_buff);
            ftp.BufStatus = ftp.BufStatus | 0x04;
            if (ftp.ListFlag)
            {
                strcat(NameList, pFileName);
                strcat(NameList, "#2");
            }
            break;
        case 3:
            memset((void *)RootBuf4, '\0', sizeof(RootBuf4));
            sprintf(RootBuf4, recv_buff);
            ftp.BufStatus = ftp.BufStatus | 0x08;
            if (ftp.ListFlag)
            {
                strcat(NameList, pFileName);
                strcat(NameList, "#3");
            }
            break;
        default:
            ftp.ListFlag = 0xff;
            break;
        }
    }
    else
    {
        ftp.ListFlag = 1; /* Directly replace the original file in the next directory level */
        memset((void *)UserBuf, '\0', sizeof(UserBuf));
        sprintf(UserBuf, recv_buff);
    }
}
