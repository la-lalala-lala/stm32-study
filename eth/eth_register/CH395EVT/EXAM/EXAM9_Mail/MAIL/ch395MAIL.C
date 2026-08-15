/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395MAIL.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 chip for sending and receiving emails
 **********************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "CH395INC.H"
#include "CH395CMD.H"
#include "mailinc.h"
#include "ch395.h"
#include "mail.h"
#include "debug.h"
#include "CH395MAILCMD.H"
#include <ctype.h>

#ifdef receive_over_reply
const UINT8 *g_mailbody = "welcome.";                                                   /* Reply email body content, can be modified */
const UINT8 *g_autograph1 = "\r\n\r\nBest Regards!\r\n----- Original Message ----\r\n"; /* Reply email signature part 1 */
const UINT8 *g_autograph2 = "\r\nweb:http://www.wch.cn\r\n";                            /* Reply email signature part 2 */
UINT8 macaddr[6];                                                                       /* Store MAC address (hex data) */
UINT8 macaddrchar[18];                                                                  /* Store converted MAC address as string */
UINT8 R_argv[3][32];                                                                    /* Store parsed email information */
#endif
char mail_buf[4096]; /* Data buffer */

const UINT8 SMTP_CLIENT_CMD[5][11] = /*SMTP CMD Codes*/
    {
        "EHLO",       /* 0 Exit */
        "AUTH LOGIN", /* 1 Login */
        "MAIL FROM:", /* 2 Sender address */
        "RCPT TO:",   /* 3 Recipient address */
        "DATA"        /* 4 Start sending data */
};
const UINT8 POP3_CLIENT_CMD[12][5] = /*POP3 CMD Codes*/
    {
        /* basic POP3 commands */
        "QUIT", /* 0 Exit */
        "USER", /* 1 Username */
        "PASS", /* 2 Password */
        "STAT", /* 3 Mailbox statistics */
        "LIST", /* 4 Return size of specified email */
        "RETR", /* 5 Retrieve full text of email */
        "DELE", /* 6 Mark email for deletion */
        "RSET", /* 7 Undo all DELE commands */
        "NOOP", /* 8 Return a positive response */
        /* alternative POP3 commands */
        "APOP", /* 9  Authenticated secure password transmission, changes state on success */
        "TOP",  /* 10 Return the top m lines of the nth email, m must be a natural number */
        "UIDL"  /* 11 Return unique identifier for specified email */
};
/**********************************************************************************
 * Function Name  : ch395mail_pop3init
 * Description    : Initialize email reception
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
#ifdef receive_mail
void ch395mail_pop3init(void)
{
    p_pop3 = &m_pop3;
    memset(p_pop3, '\0', sizeof(POP));
    strcpy((char *)p_pop3->pPop3Server, (const char *)p_Server);     /* Server name */
    strcpy((char *)p_pop3->pPop3UserName, (const char *)p_UserName); /* Username */
    strcpy((char *)p_pop3->pPop3PassWd, (const char *)p_PassWord);   /* Password */
}
#endif
/******************************************************************************/
#ifdef send_mail
/**********************************************************************************
 * Function Name  : ch395mail_smtpinit
 * Description    : Initialize email sending
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void ch395mail_smtpinit(void)
{
    p_smtp = &m_smtp;
    p_smtp->g_MIME = 0;
    memset(p_smtp, '\0', sizeof(SMTP));
    strcpy((char *)p_smtp->m_strSMTPServer, (const char *)m_Server);   /* Server name */
    strcpy((char *)p_smtp->m_strUSERID, (const char *)m_UserName);     /* Username */
    strcpy((char *)p_smtp->m_strPASSWD, (const char *)m_PassWord);     /* Password */
    strcpy((char *)p_smtp->m_strSendFrom, (const char *)m_SendFrom);   /* Sender email address */
    strcpy((char *)p_smtp->m_strSenderName, (const char *)m_SendName); /* Sender name */
#ifdef receive_over_reply
    strcpy((char *)p_smtp->m_strSendTo, (const char *)R_argv[0]);  /* Recipient email address */
    strcpy((char *)p_smtp->m_strSubject, (const char *)R_argv[1]); /* Subject */
    strcpy((char *)p_smtp->m_strFile, (const char *)R_argv[2]);    /* Attachment file name (leave uninitialized if no attachment) */
#else
    strcpy((char *)p_smtp->m_strSendTo, (const char *)m_SendTo);   /* Recipient email address */
    strcpy((char *)p_smtp->m_strSubject, (const char *)m_Subject); /* Subject */
    strcpy((char *)p_smtp->m_strFile, (const char *)m_FileName);   /* Attachment file name (leave uninitialized if no attachment) */
#endif
}

/**********************************************************************************
 * Function Name  : ch395mail_replybody
 * Description    : Construct reply email body; content can be modified
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
#ifdef receive_over_reply
void ch395mail_replybody(void)
{
    memset(MailBodyData, '\0', sizeof(MailBodyData));
    QuotedPrintableEncode((UINT8 *)"Hello", MailBodyData, strlen("Hello"), 76);
    strcat((char *)send_buff, (const char *)MailBodyData);
    strcat((char *)send_buff, (const char *)("\r\n    "));
    strcat((char *)send_buff, (const char *)(p_pop3->DecodeRName));
    strcat((char *)send_buff, (const char *)("!"));
    memset(MailBodyData, '\0', sizeof(MailBodyData));
    QuotedPrintableEncode((UINT8 *)g_mailbody, MailBodyData, strlen((const char *)g_mailbody), 76);
    strcat((char *)send_buff, (const char *)MailBodyData);
    if (p_pop3->identitycheck == 1)
    {
        memset(macaddr, '\0', sizeof(macaddr));
        memset(macaddrchar, '\0', sizeof(macaddrchar));
        CH395CMDGetMACAddr(macaddr);
        ch395mail_xtochar(macaddr, &macaddrchar[0], strlen((const char *)macaddr));
#if DEBUG
        printf("B len= %02X\n", (UINT16)strlen((const char *)macaddrchar));
#endif
        if (strlen((const char *)macaddrchar) > 17)
            macaddrchar[17] = '\0';
        strcat((char *)send_buff, (const char *)("\r\n"));
        memset(MailBodyData, '\0', sizeof(MailBodyData));
        QuotedPrintableEncode((UINT8 *)"I am ", MailBodyData, strlen((const char *)("I am ")), 76);
        strcat((char *)send_buff, (const char *)MailBodyData);
        memset(MailBodyData, '\0', sizeof(MailBodyData));
        QuotedPrintableEncode((UINT8 *)macaddrchar, MailBodyData, strlen((const char *)macaddrchar), 76);
        strcat((char *)send_buff, (const char *)MailBodyData);
        strcat((char *)send_buff, (const char *)(".\r\n"));
    }
    strcat((char *)send_buff, (const char *)g_autograph1);
    strcat((char *)send_buff, (const char *)("From: \""));
    strcat((char *)send_buff, (const char *)(p_pop3->DecodeRName));
    strcat((char *)send_buff, (const char *)("\" <"));
    strcat((char *)send_buff, (const char *)(R_argv[0]));
    strcat((char *)send_buff, (const char *)(">\r\n"));
    strcat((char *)send_buff, (const char *)("To: "));
    strcat((char *)send_buff, (const char *)(p_smtp->m_strSendFrom));
    strcat((char *)send_buff, (const char *)("\r\n"));
    if (strlen((const char *)(p_pop3->Ccname)))
    {
        strcat((char *)send_buff, (const char *)("Cc: "));
        strcat((char *)send_buff, (const char *)(p_pop3->Ccname));
        strcat((char *)send_buff, (const char *)("\r\n"));
    }
    strcat((char *)send_buff, "Sent: ");
    strcat((char *)send_buff, (const char *)(p_pop3->sBufTime));
    strcat((char *)send_buff, (const char *)("\r\nSubject: "));
    strcat((char *)send_buff, (const char *)(p_smtp->m_strSubject));
    strcat((char *)send_buff, (const char *)g_autograph2);
}
#endif
/******************************************************************************/
#endif
/*********************************************************************************
 * Function Name  : ch395mail_AnalyseMailData
 * Description    : Analyzes the received email
 * Input          : Email content
 * Output         : None
 * Return         : None
 **********************************************************************************/
#ifdef receive_over_reply
void ch395mail_AnalyseMailData(char *recv_buff)
{
    UINT16 i;
    char *from_pos;
    ReceLen = strlen(recv_buff);
    printf("%s\r\n", recv_buff);

    from_pos = strstr(recv_buff, "\nFrom: ");
    if (from_pos)
    {
        from_pos += 7;
        i = 0;
        while ((from_pos[i] != '\r') && (i < 32))
        {
            R_argv[0][i] = from_pos[i];
            i++;
        }
        printf("From: %s\n", R_argv[0]);
    }
    else
    {
        printf("Could not find 'From:'\n");
    }

    from_pos = strstr(recv_buff, "\nDate: ");
    if (from_pos)
    {
        from_pos += 7;
        i = 0;
        while ((from_pos[i] != '\r') && (i < 32))
        {
            p_pop3->sBufTime[i] = from_pos[i];
            i++;
        }
        printf("Date: %s\n", p_pop3->sBufTime);
    }
    else
    {
        printf("Could not find 'Date:'\n");
    }

    from_pos = strstr(recv_buff, "\nSubject: ");
    if (from_pos)
    {
        from_pos += 10;
        i = 0;
        memset(send_buff, 0, 512);
        while ((from_pos[i] != '\r') && (i < 512))
        {
            send_buff[i] = from_pos[i];
            i++;
        }
        from_pos = strstr((const char *)send_buff, "?q?");
        if (from_pos)
        {
            decode_q_encoded((const char *)from_pos, (char *)send_buff, sizeof(send_buff));
            memcpy(&R_argv[1], send_buff, strlen((const char *)send_buff));
        }
        else
        {
            printf("Other encoding\n");
        }
    }
    else
    {
        printf("Could not find 'Subject:'\n");
    }

    extract_between((const char *)recv_buff, "Content-Transfer-Encoding: ", "\n", (char *)send_buff, sizeof(send_buff));
    printf("Text Encoding: %s\n", send_buff);

    // Extract text content
    from_pos = strstr(recv_buff, "Content-Type: text/plain");
    if (from_pos)
    {
        from_pos = strstr(from_pos, "\r\n\r\n");
        if (from_pos)
        {
            from_pos += 2;
            const char *end = strstr(from_pos, "\n--");
            if (end)
            {
                int len = end - from_pos;
                if (len > sizeof(send_buff) - 1)
                    len = sizeof(send_buff) - 1;
                strncpy((char *)send_buff, (const char *)from_pos, len);
                send_buff[len] = '\0';
            }
        }
    }
    Base64Decode(send_buff, strlen((const char *)send_buff), send_buff);
    printf("Text Content: %s\n", send_buff);
    if (strlen((const char *)send_buff) < 128)
    {
        memset(MailBodyData, 0, sizeof(MailBodyData));
        memcpy(MailBodyData, send_buff, strlen((const char *)send_buff));
    }
    else
    {
        printf("Mail body too long\n");
    }

    extract_between((const char *)recv_buff, "filename=\"", "\"", (char *)send_buff, sizeof(send_buff));

    memcpy(R_argv[2], send_buff, strlen((const char *)send_buff));

    from_pos = strstr(recv_buff, "Content-Type: application/octet-stream");
    if (from_pos)
    {
        extract_between((const char *)recv_buff, "Content-Transfer-Encoding: ", "\n", (char *)send_buff, sizeof(send_buff));
        printf("Attachment Encoding: %s\n", send_buff);

        from_pos = strstr(from_pos, "\r\n\r\n");
        if (from_pos)
        {
            from_pos += 2;
            const char *end = strstr(from_pos, "\n--");
            if (end)
            {
                int len = end - from_pos;
                if (len > sizeof(send_buff) - 1)
                    len = sizeof(send_buff) - 1;
                strncpy((char *)send_buff, (const char *)from_pos, len);
                send_buff[len] = '\0';
            }
        }
        Base64Decode(send_buff, strlen((const char *)send_buff), send_buff);
        printf("Text Content: %s\n", send_buff);
        if (strlen((const char *)send_buff) < attach_max_len)
        {
            memset(AttachmentData, 0, sizeof(AttachmentData));
            memcpy(AttachmentData, send_buff, strlen((const char *)send_buff));
        }
        else
        {
            printf("Attachment body too long\n");
        }
    }
    /* In some cases, the user name is not displayed and the email name is used instead */
    memcpy((char *)p_pop3->DecodeRName, (const char *)R_argv[0], strlen((const char *)R_argv[0]));

#if DEBUG
    printf("addr: %s\n", R_argv[0]);                /* Sender's address */
    printf("send name: %s\n", p_pop3->DecodeRName); /* Sender's name */
    printf("subject: %s\n", R_argv[1]);             /* Email subject */
    printf("attach name: %s\n", R_argv[2]);         /* Attachment name */
    printf("send time: %s\n", p_pop3->sBufTime);    /* Send time */
    printf("attach text: %s\n", AttachmentData);    /* Attachment content */
#endif

    p_pop3->identitycheck = 1;

    ch395mail_smtpinit();
}
#endif
/**********************************************************************************
 * Function Name  : ch395mail_CheckResponse
 * Description    : Verify the handshake signal
 * Input          : recv_buff   - Handshake information
                    check_type  - Type of check
 * Output         : None
 * Return         : None
**********************************************************************************/
UINT8 ch395mail_CheckResponse(UINT8 *recv_buff, UINT8 check_type)
{
#ifdef receive_mail
    static u16 mail_len;
#endif
    switch (check_type)
    {
#ifdef send_mail
    case SMTP_CHECK_CNNT:
        if (strncmp("220", (const char *)recv_buff, 3) == 0)
        {
            OrderType = SMTP_SEND_HELO;
            return (CHECK_SUCCESS);
        }
        return SMTP_ERR_CNNT;
    case SMTP_CHECK_HELO:
        if (strncmp("250", (const char *)recv_buff, 3) == 0)
        {
            OrderType = SMTP_SEND_AUTH;
            return (CHECK_SUCCESS);
        }
        return SMTP_ERR_HELO;
    case SMTP_CHECK_AUTH: /* Login command */
        if (strncmp("250", (const char *)recv_buff, 3) == 0)
        {
            OrderType = COMMAND_UNUSEFULL;
            return (CHECK_SUCCESS);
        }
        if (strncmp("334", (const char *)recv_buff, 3) == 0)
        {
            OrderType = SMTP_SEND_USER;
            return (CHECK_SUCCESS);
        }
        return SMTP_ERR_AUTH;
    case SMTP_CHECK_USER: /* Username */
        if (strncmp("334", (const char *)recv_buff, 3) == 0)
        {
            OrderType = SMTP_SEND_PASS;
            return (CHECK_SUCCESS);
        }
        return SMTP_ERR_USER;
    case SMTP_CHECK_PASS: /* Password */
        if (strncmp("235", (const char *)recv_buff, 3) == 0)
        {
            OrderType = SMTP_SEND_MAIL;
            return (CHECK_SUCCESS);
        }
        return SMTP_ERR_PASS;
    case SMTP_CHECK_MAIL: /* Sender */
        if (strncmp("250", (const char *)recv_buff, 3) == 0)
        {
            OrderType = SMTP_SEND_RCPT;
            return (CHECK_SUCCESS);
        }
        return SMTP_ERR_MAIL;
    case SMTP_CHECK_RCPT: /* Recipient */
        if (strncmp("250", (const char *)recv_buff, 3) == 0)
        {
            OrderType = SMTP_SEND_DATA;
            return (CHECK_SUCCESS);
        }
        return SMTP_ERR_RCPT;
    case SMTP_CHECK_DATA: /* data command */
        if (strncmp("354", (const char *)recv_buff, 3) == 0)
        {
            OrderType = SMTP_DATA_OVER;
            return (CHECK_SUCCESS);
        }
        return SMTP_ERR_DATA;
    case SMTP_CHECK_DATA_END: /* Data sending completed */
        if (strncmp("250", (const char *)recv_buff, 3) == 0)
        {
#ifdef send_over_receive
            OrderType = POP_RECEIVE_START;
#endif
#ifdef send_over_quit
            OrderType = SMTP_SEND_QUIT;
#endif
            return (CHECK_SUCCESS);
        }
        return SMTP_ERR_DATA_END;
    case SMTP_CHECK_QUIT: /* Logout */
        if (strncmp("220", (const char *)recv_buff, 3) == 0 || strncmp("221", (const char *)recv_buff, 3) == 0)
        {
            OrderType = SMTP_CLOSE_SOCKET;
            return (CHECK_SUCCESS);
        }
        return SMTP_ERR_QUIT;
#endif
#ifdef receive_mail
    case POP_CHECK_CNNT: /* Username */
        if (strncmp("+OK", (const char *)recv_buff, 3) == 0)
        {
            OrderType = POP_RECEIVE_USER;
            return (CHECK_SUCCESS);
        }
        return POP_ERR_CNNT;
    case POP_CHECK_USER: /* Username */
        if (strncmp("+OK", (const char *)recv_buff, 3) == 0)
        {
            OrderType = POP_RECEIVE_PASS;
            return (CHECK_SUCCESS);
        }
        return POP_ERR_USER;
    case POP_CHECK_PASS: /* Password */
        if (strncmp("+OK", (const char *)recv_buff, 3) == 0)
        {
            OrderType = POP_RECEIVE_STAT;
            return (CHECK_SUCCESS);
        }
        return POP_ERR_PASS;
    case POP_CHECK_STAT: /* Total mail information */
        if (strncmp("+OK", (const char *)recv_buff, 3) == 0)
        {
            OrderType = POP_RECEIVE_LIST;
            return (CHECK_SUCCESS);
        }
        return POP_ERR_STAT;
    case POP_CHECK_LIST: /* Mail list */
        if ((strncmp("+OK", (const char *)recv_buff, 3) == 0))
        {
            p_pop3->RefreshTime = 0;
#ifdef receive_over_reply
            OrderType = POP_RECEIVE_RTER;
#endif
#ifdef receive_over_quit
            OrderType = POP_RECEIVE_QUIT;
#endif
#ifdef receive_dele_quit
            OrderType = POP_RECEIVE_RTER;
#endif
            return (CHECK_SUCCESS);
        }
        return POP_ERR_LIST;
    case POP_CHECK_QUIT: /* Logout */
        if (strncmp("+OK", (const char *)recv_buff, 3) == 0)
        {
#ifdef POP_REFRESH
            if (p_pop3->RefreshTime)
            {
                Delay_Ms(200);
                Delay_Ms(200);
                OrderType = POP_RECEIVE_START;
            }
            else
                OrderType = POP_CLOSE_SOCKET;
#else
            OrderType = POP_CLOSE_SOCKET;
#endif
            return (CHECK_SUCCESS);
        }
        return POP_ERR_QUIT;
    case POP_CHECK_RETR: /* Retrieve mail content */
        if (strncmp("+OK", (const char *)recv_buff, 3) == 0)
        {
            u8 tvar = 0;
            // Skip the first space
            while (recv_buff[tvar] != ' ')
                tvar++;
            tvar++;
            while (isdigit(recv_buff[tvar]))
            {
                mail_len = mail_len * 10 + (recv_buff[tvar] - '0'); // Build number
                tvar++;
            }
            printf("mail len = %d\n", mail_len);
            mail_len += 20;
            // The mail is too long. Do not read this mail
            if (mail_len > 4096)
            {
                printf("mail len too long\r\n");
                return (POP_ERR_DROP);
            }
            memset(mail_buf, 0, 4096);

            OrderType = COMMAND_UNUSEFULL;
#ifdef receive_over_reply
            memset(AttachmentData, '\0', sizeof(AttachmentData));
            memset(R_argv, '\0', sizeof(R_argv));
            memset(p_pop3, '\0', sizeof(POP));
#endif
        }
        if (strncmp("-ERROR", (const char *)recv_buff, 6) != 0)
        {
            if (mail_len != 0)
            {
                strcat(mail_buf, (const char *)recv_buff);
                printf("recv_len = %d\n", strlen((const char *)mail_buf));
                mail_len -= strlen((const char *)recv_buff);
                printf("remain len = %d\r\n", mail_len);
            }
            if (mail_len == 0)
            {
                if (strncmp("\r\n.\r\n", (const char *)(&recv_buff[ReceLen - 5]), 5) == 0)
                {
#ifdef receive_dele_quit
                    OrderType = POP_RECEIVE_DELE;
                    return CHECK_SUCCESS;
#endif
#ifdef receive_over_quit
                    OrderType = POP_RECEIVE_QUIT;
                    return CHECK_SUCCESS;
#endif
#ifdef receive_over_reply
                    ch395mail_AnalyseMailData((char *)mail_buf); /* Analyze mail content */
                    OrderType = SMTP_SEND_START;
                    return CHECK_SUCCESS;
#endif
                }
                else
                {
                    return POP_ERR_RETR;
                }
            }
            else
            {
                return COMMAND_UNUSEFULL;
            }
        }
        if (OrderType == COMMAND_UNUSEFULL)
            return (CHECK_SUCCESS);
        return POP_ERR_RETR;
    case POP_CHECK_DELE: /* Delete */
        if (strncmp("+OK", (const char *)recv_buff, 3) == 0)
        {
            OrderType = POP_RECEIVE_QUIT; /* After execution, do not execute other commands, select the next command as needed */
            return (CHECK_SUCCESS);
        }
        return POP_ERR_DELE;
    case POP_CHECK_RSET: /* Undo delete */
        if (strncmp("+OK", (const char *)recv_buff, 3) == 0)
        {
            OrderType = POP_RECEIVE_QUIT; /* After execution, do not execute other commands, select the next command as needed */
            return (CHECK_SUCCESS);
        }
        return POP_ERR_RSET;
    case POP_CHECK_TOP: /* Get the first n lines of mail */
        if (strncmp("+OK", (const char *)recv_buff, 3) == 0)
        {
            OrderType = POP_RECEIVE_QUIT; /* After execution, do not execute other commands, select the next command as needed */
            return (CHECK_SUCCESS);
        }
        if (strncmp("Return", (const char *)recv_buff, 6) == 0)
        {
            OrderType = POP_RECEIVE_QUIT; /* After execution, do not execute other commands, select the next command as needed */
            return (CHECK_SUCCESS);
        }
        return POP_ERR_TOP;
    case POP_CHECK_UIDL: /* Get the unique identifier of the mail */
        if (strncmp("+OK", (const char *)recv_buff, 3) == 0)
        {
            OrderType = POP_RECEIVE_QUIT; /* After execution, do not execute other commands, select the next command as needed */
            return (CHECK_SUCCESS);
        }
        return POP_ERR_UIDL;
#endif
    default:
        return SMTP_ERR_UNKNOW;
    }
}
