/********************************** (C) COPYRIGHT *********************************
 * File Name          : MAILINC.H
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : Header file definitions for ch395MAIL.c
 **********************************************************************************/
#ifndef __MAILINC_H__
#define __MAILINC_H__
#include "debug.h"
extern UINT8 OrderType;   /* Command type */
extern UINT8 CheckType;   /* Handshake signal verification type */
extern UINT8 ReceDatFlag; /* Flag indicating whether data has been received */
extern UINT8 SendDatFlag; /* Flag indicating whether data has been sent */
extern UINT16 ReceLen;    /* Length of data received in the receive buffer */

#define attach_max_len 512   /* Maximum length for sending attachments */
#define SendSocket 0         /* Socket used for sending emails */
#define ReceSocket 1         /* Socket used for receiving emails */
#define POP3_SERVER_PORT 110 /* Port number for receiving emails */
#define SMTP_SERVER_PORT 25  /* Port number for sending emails */
/**************************************************************************************************************************
 * Operation Mode (Default is type 5)
 * 1  Only send email -- After sending, logout and close the socket connection;
 * 2  Only receive email -- After receiving (fetching mail list), logout and close the socket connection;
 * 3  Only receive email -- After receiving (reading and deleting mail), logout and close the socket connection;
 * 4  Send and receive email -- After sending, start receiving; after receiving, logout and close the socket connection;
 * 5  Send and receive email -- After receiving, reply to email; after replying, logout, delete the mail, then logout and close the socket connection.
 **************************************************************************************************************************/
#ifndef mail_work_mode
#define mail_work_mode 5
#endif

#if (mail_work_mode == 1)
#define send_mail 1      /* Send email */
#define send_over_quit 1 /* Send and then quit */
#elif (mail_work_mode == 2)
#define receive_mail 1      /* Receive email */
#define receive_over_quit 1 /* Read email and then quit */
#elif (mail_work_mode == 3)
#define receive_mail 1      /* Receive email */
#define receive_dele_quit 1 /* Delete after reading and then quit */
#elif (mail_work_mode == 4)
#define send_mail 1         /* Send email */
#define send_over_receive 1 /* Send then receive */
#define receive_mail 0      /* Receive email */
#define receive_over_quit 1 /* Read email and then quit */
#elif (mail_work_mode == 5)
#define receive_mail 1       /* Receive email */
#define receive_over_reply 1 /* Read email and then reply */
#define send_mail 0          /* Send email */
#define send_over_quit 1     /* Send and then quit */
#endif

/* ********************************************************************************************************************* *
 * POP Optional Commands
 *
 *********************************************************************************************************************** */
#ifdef receive_mail
#define POP_RTER 1    /* 1 Retrieve all text of server email */
#define POP_DELE 1    /* 2 Mark for deletion */
#define POP_RSET 1    /* 3 Undo all DELE commands */
#define POP_TOP 1     /* 4 Return the first m lines of n-th email */
#define POP_UIDL 1    /* 5 Retrieve unique identifier for the specified email, if not specified, return for all emails */
#define POP_REFRESH 1 /* Continue searching when no email is received */
#endif

/*********************************************************************************************************************** *
 * Command Codes
 * 0x01-0x0f for receive email command codes
 ****************** 0x01-0x07 for common commands
 ****************** 0x09-0x0e for optional POP commands
 ****************** 0x01-0x02-0x03-0x06 for status verification
 ****************** 0x04-0x05 0x09-0x0e for operational status
 * 0x10-0x1f for send email command codes
 ****************** 0x10-0x17 should be executed in order
 ****************** Subsequent ones are optional operations
 * 0x00    No operation
 ********************************************************************************************************************* */
#define COMMAND_UNUSEFULL 0x00
// POP RECEIVE CODE
#define POP_RECEIVE_USER 0x01  /* Authenticate user */
#define POP_RECEIVE_PASS 0x02  /* Authenticate password */
#define POP_RECEIVE_STAT 0x03  /* Mailbox statistics */
#define POP_RECEIVE_LIST 0x04  /* Return size of specified email */
#define POP_RECEIVE_RTER 0x05  /* Retrieve entire email text */
#define POP_RECEIVE_QUIT 0x06  /* Exit login */
#define POP_CLOSE_SOCKET 0x07  /* Close POP socket */
#define POP_RECEIVE_START 0x08 /* Start receiving email */
#define POP_RECEIVE_DELE 0x09  /* Mark for deletion */
#define POP_RECEIVE_RSET 0x0a  /* Undo all DELE commands */
#define POP_RECEIVE_TOP 0x0b   /* Retrieve the first m lines of n-th email (m must be a natural number) */
#define POP_RECEIVE_UIDL 0x0e  /* Return unique identifier for specified email */
#define POP_ERR_CHECK 0x0F     /* POP handshake error, exit login and close socket */
// SMTP SEND CODE
#define SMTP_SEND_HELO 0x10    /* Send HELO command */
#define SMTP_SEND_AUTH 0x11    /* Send login command */
#define SMTP_SEND_USER 0x12    /* Send username */
#define SMTP_SEND_PASS 0x13    /* Send password */
#define SMTP_SEND_MAIL 0x14    /* Send sender's address */
#define SMTP_SEND_RCPT 0x15    /* Send recipient's address */
#define SMTP_SEND_DATA 0x16    /* Send data command */
#define SMTP_DATA_OVER 0x17    /* Send email content */
#define SMTP_SEND_QUIT 0x18    /* Exit login */
#define SMTP_CLOSE_SOCKET 0x19 /* Close SMTP socket */
#define SMTP_SEND_START 0x1E   /* Start sending email */
#define SMTP_ERR_CHECK 0x1F    /* SMTP handshake error, exit login and close socket */

/* ********************************************************************************************************************* *
 * Handshake Signal Verification Error Codes
 * ********************************************************************************************************************* */
// POP ERR CHECK CODE
#define POP_ERR_CNNT 0x20 /* POP connection error ！ possibly port number, server IP, etc. */
#define POP_ERR_USER 0x21 /* User authentication error ！ username format error or USER command error */
#define POP_ERR_PASS 0x22 /* Login failed ！ username or password error or PASS command error */
#define POP_ERR_STAT 0x23 /* Error retrieving email information ！ STAT command error */
#define POP_ERR_LIST 0x24 /* Error retrieving email list ！ LIST command error */
#define POP_ERR_RETR 0x25 /* Error retrieving specified email ！ email does not exist or RETR command error */
#define POP_ERR_DELE 0x26 /* Error deleting email ！ email does not exist or DELE command error */
#define POP_ERR_QUIT 0x27 /* Error exiting login ！ QUIT command error */
#define POP_ERR_RSET 0x28 /* Error restoring deleted email ！ RSET command error */
#define POP_ERR_TOP 0x2B  /* Error retrieving the first m lines of email ！ email does not exist or TOP command error */
#define POP_ERR_UIDL 0x2C /* Error retrieving email ID ！ email does not exist or UIDL command error */
#define POP_ERR_UNKW 0x2F /* Unknown POP error */
#define POP_ERR_DROP 0x30
// SMTP ERR CHECK CODE
#define SMTP_ERR_CNNT 0x31     /* SMTP connection error ！ possibly port number, server IP, etc. */
#define SMTP_ERR_HELO 0x32     /* HELO command failure ！ HELO command error or hostname format error */
#define SMTP_ERR_AUTH 0x33     /* LOGIN command failure ！ AUTH command error */
#define SMTP_ERR_USER 0x34     /* Username not recognized ！ USER command error */
#define SMTP_ERR_PASS 0x35     /* Login failure ！ username or password error or PASS command error */
#define SMTP_ERR_MAIL 0x36     /* Sender address failure ！ MAIL command error */
#define SMTP_ERR_RCPT 0x37     /* Recipient address error ！ RCPT command error */
#define SMTP_ERR_DATA 0x38     /* DATA command failure */
#define SMTP_ERR_DATA_END 0x39 /* Data sending error */
#define SMTP_ERR_QUIT 0x3A     /* Exit login failure */
#define SMTP_ERR_UNKNOW 0x3F   /* Unknown SMTP error */

#define send_data_timeout 0x10 /* Data sending timeout */
#define send_data_success 0x14 /* Data sending success */

/* *********************************************************************************************************************
 * Common Command Codes
 *
 ********************************************************************************************************************* */
// POP3 CODE
#define OPEN_SOCKET_RECEIVE 0xA0    /* Open receiving socket */
#define LOGIN_MAILBOX_RECEIVE 0xA1  /* Log in to receiving mailbox */
#define READ_MAIL_STAT 0xA2         /* Read total list of emails */
#define READ_MAIL_LIST 0xA3         /* Read email list */
#define READ_MAIL_MAILBODYDATA 0xA4 /* Read email body content */
#define READ_MAIL_ATTACHMENT 0xA5   /* Read email attachment content */
#define READ_MAIL_ADDRESS 0xA6      /* Read sender's address */
#define READ_MAIL_TIME 0xA7         /* Read sending time */
#define READ_MAIL_SUBJECT 0xA8      /* Read email subject */
#define QUIT_RECEIVE_MAIL 0xA9      /* Exit login and close socket */
// SMTP CODE
#define OPEN_SOCKET_SEND 0xB0     /* Open sending socket */
#define LOGIN_MAILBOX_SEND 0xB1   /* Log in to sending mailbox */
#define SEND_MAIL_UNATTACH 0xB2   /* Send email without attachment */
#define SEND_MAIL_CONTATTACH 0xB3 /* Send email with attachment */
#define QUIT_SEND_MAIL 0xB4       /* Exit login and close socket */

/* *********************************************************************************************************************
 * Check Handshake Information Error Types
 * Success returns CHECK_SUCCESS, and the command operation code is set to the next required command
 * Error returns corresponding error code
 * Some commands do not require handshake signals, so no check is needed
 ********************************************************************************************************************* */
#define CHECK_SUCCESS 0x00
#define uncheck 0x00
// POP CHECK CODE
// Check response information, return "+OK" if correct, return "-ERR" if incorrect
#define POP_CHECK_CNNT 0x50
#define POP_CHECK_USER 0x51
#define POP_CHECK_PASS 0x52
#define POP_CHECK_STAT 0x53
#define POP_CHECK_LIST 0x54
#define POP_CHECK_RETR 0x55
#define POP_CHECK_DELE 0x56
#define POP_CHECK_SEAR 0x57
#define POP_CHECK_QUIT 0x58
#define POP_CHECK_RSET 0x59
#define POP_CHECK_APOP 0x5A
#define POP_CHECK_TOP 0x5B
#define POP_CHECK_UIDL 0x5C
// SMTP CHECK CODE
#define SMTP_CHECK_CNNT 0x60
#define SMTP_CHECK_HELO 0x61
#define SMTP_CHECK_AUTH 0x62
#define SMTP_CHECK_USER 0x63
#define SMTP_CHECK_PASS 0x64
#define SMTP_CHECK_MAIL 0x65
#define SMTP_CHECK_RCPT 0x66
#define SMTP_CHECK_DATA 0x67
#define SMTP_CHECK_DATA_END 0x68
#define SMTP_CHECK_QUIT 0x69
/*************************************************************************/
UINT8
ch395mail_SendData(UINT8 *PSend, UINT16 Len, UINT8 type, UINT8 index); /* Send data */

void ch395mail_Quit(UINT8 index); /* Exit mailbox login */

void ch395mail_MailCommand(UINT8 choiceorder); /* Execute corresponding command based on command type, automatically complete the entire send/receive process */

void ch395mail_pop3init(void); /* Initialize receiving mail */

void ch395mail_pop3Retr(UINT8 num); /* Read a specific email */

void ch395mail_pop3Dele(UINT8 num); /* Delete a specific email */

void ch395mail_pop3Rset(void); /* Undo all delete commands */

void ch395mail_pop3Top(UINT8 num, UINT8 m); /* Read the first m lines of a specific email */

void ch395mail_pop3Uidl(UINT8 num); /* Get the ID of a specific email */

void ch395mail_smtpinit(void); /* Initialize sending mail */

UINT8 ch395mail_CheckResponse(UINT8 *recv_buff, UINT8 check_type); /* Check response information */

void ch395mail_replybody(void); /* Construct reply email body */

#endif
