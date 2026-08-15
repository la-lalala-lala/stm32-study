/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395MAILCMD.H
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : Data structure definitions for ch395mail.c
 **********************************************************************************/
#ifndef __CH395MAILCMD_H__
#define __CH395MAILCMD_H__

#include "debug.h"
#include "mailinc.h"
#include "mail.h"

extern POP m_pop3;
extern POP *p_pop3;
extern SMTP m_smtp;
extern SMTP *p_smtp;
extern UINT8 EncodeName[32];                 /* Stores the encoded sender's name */
extern UINT8 send_buff[512];                 /* Data buffer for sending */
extern UINT8 MailBodyData[128];              /* Email body content, used for demonstration (temporarily stores decoded body content during replies) */
extern UINT8 AttachmentData[attach_max_len]; /* Attachment content, used for demonstration (temporarily stores decoded attachment content during replies) */

void Base64Decode(UINT8 *src, UINT16 src_len, UINT8 *dst);
void ch395mail_xtochar(UINT8 *dat, UINT8 *p, UINT8 len);
void QuotedPrintableEncode(UINT8 *pSrc, UINT8 *pDst, UINT16 nSrcLen, UINT16 nMaxLineLen);
void CH395_MailCmd(u8 choiceorder);
void decode_q_encoded(const char *input, char *output, int max_output_len);
void extract_between(const char *source, const char *start, const char *end, char *out, int max_len);

#endif
