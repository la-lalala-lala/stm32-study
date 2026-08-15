/********************************** (C) COPYRIGHT *********************************
 * File Name          : MAIL.H
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : Data structure definitions for ch395mail.c
 **********************************************************************************/
#ifndef __MAIL_H__
#define __MAIL_H__

#define Query395Interrupt() GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8)

typedef struct
{
    UINT8 g_MIME;              /* Attachment flag (whether there is an attachment) */
    UINT8 m_strFile[48];       /* Attachment name */
    UINT8 m_strSendFrom[48];   /* Sender's address */
    UINT8 m_strSendTo[48];     /* Recipient's address */
    UINT8 m_strSMTPServer[32]; /* SMTP server name */
    UINT8 m_strUSERID[32];     /* Username */
    UINT8 m_strPASSWD[32];     /* Password */
    UINT8 m_strSubject[32];    /* Subject */
    UINT8 m_strSenderName[32]; /* Sender's name */
} SMTP;

typedef struct
{
    UINT8 analysemailflag;   /* Flag indicating whether email analysis is complete */
    UINT8 identitycheck;     /* Flag for verifying attachment content */
    UINT8 RefreshTime;       /* Flag to continue searching for emails if none are found */
    UINT8 sBufTime[40];      /* Stores the time when the email is received */
    UINT8 Ccname[48];        /* Name for CC recipients */
    UINT8 DecodeRName[32];   /* Stores the decoded sender's name for the received email */
    UINT8 pPop3Server[32];   /* POP server */
    UINT8 pPop3UserName[32]; /* POP login username */
    UINT8 pPop3PassWd[32];   /* POP login password */
} POP;

extern const UINT8 SMTP_CLIENT_CMD[5][11];
extern const UINT8 POP3_CLIENT_CMD[12][5];

/* SMTP email sending parameters */
extern const char *m_Server;   /* Server name */
extern const char *m_UserName; /* Username */
extern const char *m_PassWord; /* Password */
extern const char *m_SendFrom; /* Sender's address */
extern const char *m_SendName; /* Sender's name */
extern const char *m_SendTo;   /* Recipient's address */
extern const char *m_Subject;  /* Subject */
extern const char *m_FileName; /* Attachment name (if no attachment, set to "\0") */

/* POP email receiving parameters */
extern const char *p_Server;   /* POP server */
extern const char *p_UserName; /* POP login username */
extern const char *p_PassWord; /* POP login password */
#endif
