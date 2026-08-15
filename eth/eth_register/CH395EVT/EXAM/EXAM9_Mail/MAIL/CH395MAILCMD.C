/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395MAILCMD.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CODEC
 **********************************************************************************/
#include "CH395MAILCMD.H"
#include "string.H"
#include "stdio.H"
#include "CH395INC.H"
#include "CH395CMD.H"
#include "ch395.h"
#include <ctype.h>

extern void CH395SocketInitOpen(UINT8 index);

const UINT8 *g_strBoundary = "18ac0781-9ae4-4a2a-b5f7-5479635efb6b";				   /* Boundary, can be modified */
const UINT8 *g_strEncode = "base64";												   /* Encoding method */
const UINT8 *g_strUINT8set = "gb2312";												   /* Windows format (Linux format: "utf-8") */
const UINT8 *g_xMailer = "X-Mailer: X-WCH-Mail Client Sender\r\n";					   /* X-Mailer content */
const UINT8 *g_Encoding = "Content-Transfer-Encoding: quoted-printable\r\nReply-To: "; /* Encoding content */
const UINT8 *g_Custom = "X-Program: CSMTPMessageTester";							   /* X-Program content, can be modified */
const UINT8 *g_FormatMail = "This is a multi-part message in MIME format.";			   /* The email has multiple parts, typically includes attachments */
const UINT8 *g_AttachHead = "\r\nContent-Transfer-Encoding: quoted-printable\r\n";	   /* Encoding method */

#ifdef send_mail
const UINT8 *base64_map = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#endif
#ifdef receive_mail
const UINT8 base64_decode_map[256] =
	{
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 62, 255, 255, 255, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255,
		255, 0, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255, 255, 26, 27, 28,
		29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
		49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
#endif
POP m_pop3;
POP *p_pop3;
SMTP m_smtp;
SMTP *p_smtp;
UINT8 EncodeName[32];														   /* Stores the encoded sender's name */
UINT8 send_buff[512];														   /* Sending data buffer */
UINT8 MailBodyData[128] = "Demonstration test ch395 mail function wch.cn";	   /* Email body content, used for demonstration (temporarily stores decoded body content when replying) */
UINT8 AttachmentData[attach_max_len] = "0123456789abcdefghijklmnopqrstuvwxyz"; /* Attachment content, used for demonstration (temporarily stores decoded attachment content when replying) */

/****************************************************************************
 * Function Name  : hex_to_int
 * Description    : hex to int
 * Input          : hex data
 * Output         : None
 * Return         : int data
 *****************************************************************************/
int hex_to_int(char ch)
{
	if ('0' <= ch && ch <= '9')
		return ch - '0';
	if ('A' <= ch && ch <= 'F')
		return ch - 'A' + 10;
	if ('a' <= ch && ch <= 'f')
		return ch - 'a' + 10;
	return -1;
}

/****************************************************************************
 * Function Name  : decode_q_encoded
 * Description    : q4 encoded
 * Input          : input            - The string to be encoded
					output 		     - The encoded string
					max_output_len   - max len
 * Output         : None
 * Return         : None
*****************************************************************************/
void decode_q_encoded(const char *input, char *output, int max_output_len)
{
	const char *start = strstr(input, "?q?");
	if (!start)
	{
		strncpy(output, input, max_output_len); // fallback
		return;
	}

	start += 3; // skip "?q?"
	const char *end = strstr(start, "?=");
	if (!end)
	{
		strncpy(output, input, max_output_len); // fallback
		return;
	}

	int i = 0;
	while (start < end && i < max_output_len - 1)
	{
		if (*start == '_')
		{
			output[i++] = ' ';
		}
		else if (*start == '=' && isxdigit(start[1]) && isxdigit(start[2]))
		{
			int high = hex_to_int(start[1]);
			int low = hex_to_int(start[2]);
			output[i++] = (char)((high << 4) | low);
			start += 2;
		}
		else
		{
			output[i++] = *start;
		}
		start++;
	}

	output[i] = '\0';
}

/****************************************************************************
 * Function Name  : extract_between
 * Description    : extract_between
 * Input          : source   - src data
					start    - start point
					end      - end point
					max_len  - max len
 * Output         : None
 * Return         : None
*****************************************************************************/
void extract_between(const char *source, const char *start, const char *end, char *out, int max_len)
{
	const char *p1 = strstr(source, start);
	if (!p1)
		return;
	p1 += strlen(start);
	const char *p2 = strstr(p1, end);
	if (!p2)
		return;
	int len = p2 - p1;
	if (len > max_len - 1)
		len = max_len - 1;
	strncpy(out, p1, len);
	out[len] = '\0';
}

/****************************************************************************
 * Function Name  : Base64Encode
 * Description    : Base64 encoding
 * Input          : src      - The string to be encoded
 *                  src_len  - The length of the string to be encoded
 *                  dst      - The encoded string
 * Output         : None
 * Return         : None
 *****************************************************************************/
#ifdef send_mail
void Base64Encode(UINT8 *src, UINT16 src_len, UINT8 *dst)
{
	UINT16 i = 0;
	UINT16 j = 0;

	for (; i < src_len - src_len % 3; i += 3)
	{
		dst[j++] = base64_map[(src[i] >> 2) & 0x3f];
		dst[j++] = base64_map[((src[i] << 4) | (src[i + 1] >> 4)) & 0x3f];
		dst[j++] = base64_map[((src[i + 1] << 2) | (src[i + 2] >> 6)) & 0x3f];
		dst[j++] = base64_map[src[i + 2] & 0x3f];
	}

	if (src_len % 3 == 1)
	{
		dst[j++] = base64_map[(src[i] >> 2) & 0x3f];
		dst[j++] = base64_map[(src[i] << 4) & 0x3f];
		dst[j++] = '=';
		dst[j++] = '=';
	}
	else if (src_len % 3 == 2)
	{
		dst[j++] = base64_map[(src[i] >> 2) & 0x3f];
		dst[j++] = base64_map[((src[i] << 4) | (src[i + 1] >> 4)) & 0x3f];
		dst[j++] = base64_map[(src[i + 1] << 2) & 0x3f];
		dst[j++] = '=';
	}

	dst[j] = '\0';
}
#endif
/*****************************************************************************
 * Function Name  : Base64Decode
 * Description    : Base64 decoding
 * Input          : src       - The string to be decoded
 *                  src_len   - The length of the string to be decoded
 *                  dst       - The decoded string
 * Output         : None
 * Return         : None
 *****************************************************************************/

#ifdef receive_mail
void Base64Decode(UINT8 *src, UINT16 src_len, UINT8 *dst)
{
	int i = 0, j = 0;

	for (; i < src_len; i += 4)
	{
		if (strncmp((const char *)&src[i], (const char *)"\r\n", 2) == 0)
			i += 2;
		dst[j++] = base64_decode_map[src[i]] << 2 |
				   base64_decode_map[src[i + 1]] >> 4;
		dst[j++] = base64_decode_map[src[i + 1]] << 4 |
				   base64_decode_map[src[i + 2]] >> 2;
		dst[j++] = base64_decode_map[src[i + 2]] << 6 |
				   base64_decode_map[src[i + 3]];
	}
	if (src_len % 4 == 3)
	{
		dst[strlen((const char *)dst) - 1] = '\0';
	}
	else if (src_len % 4 == 2)
	{
		dst[strlen((const char *)dst) - 1] = '\0';
		dst[strlen((const char *)dst) - 1] = '\0';
	}
	else
		dst[j] = '\0';
}
#endif

/*****************************************************************************
 * Function Name  : QuotedPrintableEncode
 * Description    : Quoted-printable encoding
 * Input          : pSrc        - The string to be encoded
 *                  pDst        - The encoded string
 *                  nSrcLen     - The length of the string to be encoded
 *                  nMaxLineLen - The maximum line length
 * Output         : None
 * Return         : None
 *****************************************************************************/

#ifdef send_mail
void QuotedPrintableEncode(UINT8 *pSrc, UINT8 *pDst, UINT16 nSrcLen, UINT16 nMaxLineLen)
{
	UINT16 nDstLen = 0;
	UINT16 nLineLen = 0;
	UINT16 i = 0;

	for (i = 0; i < nSrcLen; i++, pSrc++)
	{
		if ((*pSrc >= '!') && (*pSrc <= '~') && (*pSrc != '='))
		{
			*pDst++ = (char)*pSrc;
			nDstLen++;
			nLineLen++;
		}
		else
		{
			sprintf((char *)pDst, "=%02x", *pSrc);
			pDst += 3;
			nDstLen += 3;
			nLineLen += 3;
		}
		if (nLineLen >= nMaxLineLen - 3)
		{
			sprintf((char *)pDst, "=\r\n");
			pDst += 3;
			nDstLen += 3;
			nLineLen = 0;
		}
	}
	*pDst = '\0';
}
#endif

/*****************************************************************************
 * Function Name  : ch395mail_xtochar
 * Description    : Convert hexadecimal to string
 * Input          : dat     - The hexadecimal data to be converted
 *                  p       - The resulting string after conversion
 *                  len     - The length of the data to be converted
 * Output         : None
 * Return         : None
 *****************************************************************************/

#ifdef receive_over_reply
void ch395mail_xtochar(UINT8 *dat, UINT8 *p, UINT8 len)
{
	UINT8 k;
	for (k = 0; k < len; k++)
	{
		*p = (((dat[k] & 0xf0) >> 4) / 10) ? (((dat[k] & 0xf0) >> 4) + 'A' - 10) : (((dat[k] & 0xf0) >> 4) + '0');
		p++;
		*p = ((dat[k] & 0x0f) / 10) ? ((dat[k] & 0x0f) + 'A' - 10) : ((dat[k] & 0x0f) + '0');
		p++;
		if (k < len - 1)
		{
			*p = '.';
			p++;
		}
	}
}
#endif

/********************************send_mail***************************************/
#ifdef send_mail
/*****************************************************************************
 * Function Name  : ch395mail_IsMIME
 * Description    : Check if there are attachments
 * Input          : None
 * Output         : None
 * Return         : 0 - No attachments
 *                  1 - Has attachments
 *****************************************************************************/

void ch395mail_IsMIME(void)
{
	if (strlen((const char *)p_smtp->m_strFile) <= 0)
		p_smtp->g_MIME = 0;
	else
		p_smtp->g_MIME = 1;
}

/*****************************************************************************
 * Function Name  : ch395mail_GetAttachHeader
 * Description    : Used to build the attachment envelope
 * Input          : pFileName     - The name of the attachment
 *                  pAttachHeader - The assembled envelope content
 * Output         : None
 * Return         : None
 *****************************************************************************/

void ch395mail_GetAttachHeader(UINT8 *pFileName, UINT8 *pAttachHeader)
{
	const UINT8 *strContentType = "application/octet-stream";
	sprintf((char *)pAttachHeader, "\r\n\r\n--%s\r\nContent-Type: %s;\r\n name=\"%s\"%sContent-Disposition: \
	attachment;\r\n filename=\"%s\"\r\n\r\n",
			g_strBoundary, strContentType, pFileName, g_AttachHead, pFileName);
}
/*****************************************************************************
 * Function Name  : ch395mail_GetAttachEnd
 * Description    : Assemble the attachment end content
 * Input          : EndSize    - size of the end content,
					pAttachEnd - the buffer to store the end content
 * Output         : None
 * Return         : None
 *****************************************************************************/

void ch395mail_GetAttachEnd(UINT16 *EndSize, UINT8 *pAttachEnd)
{
	strcat((char *)pAttachEnd, (const char *)"\r\n--");
	strcat((char *)pAttachEnd, (const char *)g_strBoundary);
	strcat((char *)pAttachEnd, (const char *)"--");
	*EndSize = strlen((const char *)pAttachEnd);
}

/*****************************************************************************
 * Function Name  : ch395mail_SendHeader
 * Description    : Send the mail header
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_SendHeader(void)
{
	UINT8 s;
	const UINT8 *strContentType = "multipart/mixed";
	/* "FROM: "*/
	memset((char *)send_buff, (int)'\0', sizeof(send_buff));
	strcat((char *)send_buff, (const char *)"From: \"");
	strcat((char *)send_buff, (const char *)p_smtp->m_strSenderName);
	strcat((char *)send_buff, (const char *)"\" <");
	strcat((char *)send_buff, (const char *)p_smtp->m_strSendFrom);
	strcat((char *)send_buff, (const char *)">\r\n");
	/* "TO: " */
	strcat((char *)send_buff, (const char *)"To: <");
	strcat((char *)send_buff, (const char *)p_smtp->m_strSendTo);
	strcat((char *)send_buff, (const char *)">\r\n");
	/* "Subject: " */
	strcat((char *)send_buff, (const char *)"Subject: ");
#ifdef receive_over_reply
	strcat((char *)send_buff, (const char *)"Re: ");
#endif
	strcat((char *)send_buff, (const char *)p_smtp->m_strSubject);
	strcat((char *)send_buff, (const char *)"\r\n");
	/*"Date: "*/
	strcat((char *)send_buff, (const char *)"Date: ");
	strcat((char *)send_buff, ""); /* Time*/
	strcat((char *)send_buff, (const char *)"\r\n");
	/* "X-Mailer: " */
	strcat((char *)send_buff, (const char *)g_xMailer);

	if (p_smtp->g_MIME == 1)
	{ /* With attachment*/
		strcat((char *)send_buff, (const char *)"MIME-Version: 1.0\r\nContent-Type: ");
		strcat((char *)send_buff, (const char *)strContentType);
		strcat((char *)send_buff, (const char *)";\r\n\tboundary=\"");
		strcat((char *)send_buff, (const char *)g_strBoundary);
		strcat((char *)send_buff, (const char *)"\"\r\n");
	}
	/* Encoding information*/
	strcat((char *)send_buff, (const char *)g_Encoding);
	strcat((char *)send_buff, (const char *)p_smtp->m_strSenderName);
	strcat((char *)send_buff, (const char *)" <");
	strcat((char *)send_buff, (const char *)p_smtp->m_strSendFrom);
	strcat((char *)send_buff, (const char *)">\r\n");
	/* Add custom-tailor*/
	strcat((char *)send_buff, (const char *)g_Custom);
	/* End of mail header*/
	strcat((char *)send_buff, (const char *)"\r\n\r\n");
#if DEBUG
	printf("Mail Header:%s\n", send_buff);
#endif
	s = ch395mail_SendData(send_buff, strlen((const char *)send_buff), uncheck, SendSocket);
#if DEBUG
	if (s != send_data_success)
		printf("ERROR: %02x\n", (UINT16)s);
#endif
}

/*****************************************************************************
 * Function Name  : ch395mail_SendAttach
 * Description    : Send the attachment
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_SendAttach(void)
{
	UINT16 EndSize;
	UINT8 s;

	memset(send_buff, '\0', sizeof(send_buff));
	ch395mail_GetAttachHeader(p_smtp->m_strFile, send_buff);
#if DEBUG
	printf("Attach Header:%s\n", send_buff);
#endif
	s = ch395mail_SendData(send_buff, strlen((const char *)send_buff), uncheck, SendSocket);
#if DEBUG
	if (s != send_data_success)
		printf("ERROR: %02x\n", (UINT16)s);
#endif

	/* Send attachment content */
	/* GetAttachedFileBody(&FileSize, m_smtp->m_strFile, pAttachedFileBody );*/
	memset(send_buff, '\0', sizeof(send_buff));
	QuotedPrintableEncode(AttachmentData, send_buff, strlen((const char *)AttachmentData), 200);
#if DEBUG
	printf("Attach Data send_buff:\n%s\n", send_buff);
#endif
	s = ch395mail_SendData(send_buff, strlen((const char *)send_buff), uncheck, SendSocket);
#if DEBUG
	if (s != send_data_success)
		printf("ERROR: %02x\n", (UINT16)s);
#endif
	memset(send_buff, '\0', sizeof(send_buff));
	ch395mail_GetAttachEnd(&EndSize, send_buff);
#if DEBUG
	printf("Attach End :%s\n", send_buff);
#endif
	s = ch395mail_SendData(send_buff, strlen((const char *)send_buff), uncheck, SendSocket); /* Send attached file end */
#if DEBUG
	if (s != send_data_success)
		printf("ERROR: %02x\n", (UINT16)s);
#endif
}

/*****************************************************************************
 * Function Name  : ch395mail_SendAttachHeader
 * Description    : Send the attachment envelope
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_SendAttachHeader(void)
{
	UINT8 s;
	const UINT8 *strContentType = "text/plain";

	s = ch395mail_SendData((UINT8 *)g_FormatMail, strlen((const char *)g_FormatMail), uncheck, SendSocket);
#if DEBUG
	if (s != send_data_success)
		printf("ERROR: %02x\n", (UINT16)s);
#endif
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "\r\n--%s\r\nContent-Type: %s;\r\n\tcharset=\"%s\"%s\r\n", g_strBoundary, strContentType, g_strUINT8set, g_AttachHead);
#if DEBUG
	printf("MIME Header:\n%s\n", send_buff);
#endif
	s = ch395mail_SendData(send_buff, strlen((const char *)send_buff), uncheck, SendSocket);
#if DEBUG
	if (s != send_data_success)
		printf("ERROR: %02x\n", (UINT16)s);
#endif
}

/*****************************************************************************
 * Function Name  : ch395mail_smtpEhlo
 * Description    : Enter the send mail state
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_smtpEhlo(void)
{
	memset(EncodeName, '\0', sizeof(EncodeName));
	QuotedPrintableEncode((UINT8 *)p_smtp->m_strSenderName, EncodeName, strlen((const char *)(p_smtp->m_strSenderName)), 76);
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s %s\r\n", SMTP_CLIENT_CMD[0], EncodeName);
#if DEBUG
	printf("EHLO :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), SMTP_CHECK_HELO, SendSocket);
}

/*****************************************************************************
 * Function Name  : ch395mail_smtpLoginAuth
 * Description    : Enter the send mail state
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_smtpLoginAuth(void)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s\r\n", SMTP_CLIENT_CMD[1]);
#if DEBUG
	printf("AUTH :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), SMTP_CHECK_AUTH, SendSocket); /* send "AUTH LOGIN" command */
}

/*****************************************************************************
 * Function Name  : ch395mail_smtpLoginUser
 * Description    : Authenticate the username
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_smtpLoginUser(void)
{
	memset(send_buff, '\0', sizeof(send_buff));
	Base64Encode((UINT8 *)p_smtp->m_strUSERID, strlen((const char *)(p_smtp->m_strUSERID)), send_buff);
	sprintf((char *)send_buff, "%s\r\n", send_buff);
#if DEBUG
	printf("USER :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), SMTP_CHECK_USER, SendSocket);
}

/*****************************************************************************
 * Function Name  : ch395mail_smtpLoginPass
 * Description    : Login password
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_smtpLoginPass(void)
{
	memset((char *)send_buff, '\0', sizeof(send_buff));
	Base64Encode((UINT8 *)p_smtp->m_strPASSWD, strlen((const char *)(p_smtp->m_strPASSWD)), send_buff);
	sprintf((char *)send_buff, "%s\r\n", send_buff);
#if DEBUG
	printf("PASS :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), SMTP_CHECK_PASS, SendSocket);
}

/*****************************************************************************
 * Function Name  : ch395mail_smtpCommandMail
 * Description    : Send sender's name
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_smtpCommandMail(void)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s <%s>\r\n", SMTP_CLIENT_CMD[2], p_smtp->m_strSendFrom);
#if DEBUG
	printf("MAIL :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), SMTP_CHECK_MAIL, SendSocket);
}

/*****************************************************************************
 * Function Name  : ch395mail_smtpCommandRcpt
 * Description    : Recipient address
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_smtpCommandRcpt(void)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s <%s>\r\n", SMTP_CLIENT_CMD[3], p_smtp->m_strSendTo);
#if DEBUG
	printf("RCPT :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), SMTP_CHECK_RCPT, SendSocket);
}

/*****************************************************************************
 * Function Name  : ch395mail_smtpCommandData
 * Description    : Send DATA command
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_smtpCommandData(void)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s\r\n", SMTP_CLIENT_CMD[4]);
#if DEBUG
	printf("DATA :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), SMTP_CHECK_DATA, SendSocket);
}

/*****************************************************************************
 * Function Name  : ch395mail_smtpSendData
 * Description    : Send email content
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_smtpSendData(void)
{
	UINT8 s;

	ch395mail_IsMIME();		/* Check if there are attachments */
	ch395mail_SendHeader(); /* Send email header */
	if (p_smtp->g_MIME == 1)
	{
		ch395mail_SendAttachHeader(); /* Send attachment header */
	}
	else
	{
		ch395mail_SendData("\r\n", strlen((const char *)"\r\n"), uncheck, SendSocket);
#if DEBUG
		if (s != send_data_success)
			printf("ERROR: %02x\n", (UINT16)s);
#endif
	}
	memset(send_buff, '\0', sizeof(send_buff));
#ifdef receive_over_reply
	ch395mail_replybody();
#else  /* receive_over_reply */
	QuotedPrintableEncode((UINT8 *)MailBodyData, send_buff, strlen((const char *)MailBodyData), 76);
#endif /* receive_over_reply */
#if DEBUG
	printf("text data %s\n", send_buff);
#endif
	s = ch395mail_SendData(send_buff, strlen((const char *)send_buff), uncheck, SendSocket);
#if DEBUG
	if (s != send_data_success)
		printf("ERROR: %02x\n", (UINT16)s);
#endif
	if (1 == p_smtp->g_MIME)
		ch395mail_SendAttach(); /* Send attached file */
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "\r\n.\r\n");
#if DEBUG
	printf("OVER :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), SMTP_CHECK_DATA_END, SendSocket); /* Send end flag of email */
}
#endif

#ifdef receive_mail
/*****************************************************************************
 * Function Name  : ch395mail_pop3LoginUser
 * Description    : Authenticate username
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_pop3LoginUser(void)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s %s\r\n", POP3_CLIENT_CMD[1], p_pop3->pPop3UserName);
#if DEBUG
	printf("USER :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), POP_CHECK_USER, ReceSocket);
}

/*****************************************************************************
 * Function Name  : ch395mail_pop3LoginPass
 * Description    : Password
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_pop3LoginPass(void)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s %s\r\n", POP3_CLIENT_CMD[2], p_pop3->pPop3PassWd);
#if DEBUG
	printf("PASS :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), POP_CHECK_PASS, ReceSocket);
}

/*****************************************************************************
 * Function Name  : ch395mail_pop3Stat
 * Description    : Return mailbox statistics
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_pop3Stat(void)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s\r\n", POP3_CLIENT_CMD[3]);
#if DEBUG
	printf("STAT :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), POP_CHECK_STAT, ReceSocket);
}

/*****************************************************************************
 * Function Name  : ch395mail_pop3List
 * Description    : Process the size of the specified email returned by the server
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_pop3List(void)
{
#if 0 /* Set to 1 if a specific email is needed */
	UINT8 num;
	num = '1';  /* Modify the email number as needed */
	memset( send_buff, '\0', sizeof(send_buff) );
	sprintf( send_buff, "%s %c\r\n", POP3_CLIENT_CMD[4], num );
#else
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s\r\n", POP3_CLIENT_CMD[4]);
#endif
#if DEBUG
	printf("LIST :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), POP_CHECK_LIST, ReceSocket);
}

/*****************************************************************************
 * Function Name  : ch395mail_pop3Retr
 * Description    : Process the full text of the email returned by the server
 * Input          : email number
 * Output         : None
 * Return         : None
 *****************************************************************************/
#ifdef POP_RTER
void ch395mail_pop3Retr(UINT8 num)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s %c\r\n", POP3_CLIENT_CMD[5], num);
#if DEBUG
	printf("RTER :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), POP_CHECK_RETR, ReceSocket);
}
#endif

/*****************************************************************************
 * Function Name  : ch395mail_pop3Dele
 * Description    : Process the cancel delete command from the server
 * Input          : num - email number
 * Output         : None
 * Return         : None
 *****************************************************************************/
#ifdef POP_DELE
void ch395mail_pop3Dele(UINT8 num)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, (const char *)"%s %c\r\n", POP3_CLIENT_CMD[6], num);
#if DEBUG
	printf("DELE :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), POP_CHECK_DELE, ReceSocket);
}
#endif

/*****************************************************************************
 * Function Name  : ch395mail_pop3Rset
 * Description    : Pop3 Rset
 * Input          : None
 * Output         : None
 * Return         : None
 *****************************************************************************/
#ifdef POP_RSET
void ch395mail_pop3Rset(void)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, (const char *)"%s \r\n", POP3_CLIENT_CMD[7]);
#if DEBUG
	printf("RSET :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), POP_CHECK_RSET, ReceSocket);
}
#endif

/*****************************************************************************
 * Function Name  : ch395mail_pop3Top
 * Description    : Process the unique identifier of the specified email returned by the server
 * Input          : num - email number
					m - number of lines
 * Output         : None
 * Return         : None
 *****************************************************************************/
#ifdef POP_TOP
void ch395mail_pop3Top(UINT8 num, UINT8 m)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s %c %c\r\n", POP3_CLIENT_CMD[10], num, m);
#if DEBUG
	printf("TOP :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, strlen((const char *)send_buff), POP_CHECK_TOP, ReceSocket);
}
#endif

/*****************************************************************************
 * Function Name  : ch395mail_pop3Uidl
 * Description    : Process the unique identifier of the specified email returned by the server
 * Input          : email number
 * Output         : None
 * Return         : None
 *****************************************************************************/
#ifdef POP_UIDL
void ch395mail_pop3Uidl(UINT8 num)
{
	memset(send_buff, '\0', sizeof(send_buff));
	sprintf((char *)send_buff, "%s %c\r\n", (const char *)POP3_CLIENT_CMD[11], num);
#if DEBUG
	printf("UIDL :%s\n", send_buff);
#endif
	ch395mail_SendData(send_buff, sizeof(send_buff), POP_CHECK_UIDL, ReceSocket);
}
#endif

/********************************send_mail***************************************/
#endif

/*****************************************************************************
 * Function Name  : Quit
 * Description    : Exit the login
 * Input          : index
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_Quit(UINT8 index)
{
	memset(send_buff, '\0', sizeof((const char *)send_buff));
	sprintf((char *)send_buff, "%s\r\n", (const char *)POP3_CLIENT_CMD[0]);
	if (index == SendSocket)
	{
#if DEBUG
		printf("smtp QUIT\n");
#endif
		ch395mail_SendData(send_buff, strlen((const char *)send_buff), SMTP_CHECK_QUIT, index);
	}
	if (index == ReceSocket)
	{
#if DEBUG
		printf("pop3 QUIT\n");
#endif
		ch395mail_SendData(send_buff, strlen((const char *)send_buff), POP_CHECK_QUIT, index);
	}
}

/*****************************************************************************
 * Function Name  : ch395mail_MailCommand
 * Description    : Judge the command and enter the corresponding subroutine
 * Input          : command type
 * Output         : None
 * Return         : None
 *****************************************************************************/
void ch395mail_MailCommand(UINT8 choiceorder)
{
	UINT8 i;
	switch (choiceorder)
	{
#ifdef send_mail
	case SMTP_SEND_HELO:
		ch395mail_smtpEhlo();
		break;
	case SMTP_SEND_AUTH:
		ch395mail_smtpLoginAuth();
		break;
	case SMTP_SEND_USER:
		ch395mail_smtpLoginUser();
		break;
	case SMTP_SEND_PASS:
		ch395mail_smtpLoginPass();
		break;
	case SMTP_SEND_MAIL:
		ch395mail_smtpCommandMail();
		break;
	case SMTP_SEND_RCPT:
		ch395mail_smtpCommandRcpt();
		break;
	case SMTP_SEND_DATA:
		ch395mail_smtpCommandData();
		break;
	case SMTP_DATA_OVER:
		ch395mail_smtpSendData();
		break;
	case SMTP_ERR_CHECK:
		ch395mail_Quit(SendSocket);
		i = CH395CMDCloseSocket(SendSocket);
		mStopIfError(i);
		break;
	case SMTP_SEND_QUIT:
		ch395mail_Quit(SendSocket);
		break;
	case SMTP_CLOSE_SOCKET:
		CheckType = uncheck;
#if DEBUG
		printf("clost smtp socket\n");
#endif
		i = CH395CMDCloseSocket(SendSocket);
		mStopIfError(i);
#ifdef receive_over_reply
		OrderType = POP_RECEIVE_DELE;
#endif
		break;
	case SMTP_SEND_START:
		CH395SocketInitOpen(SendSocket);
		break;
#endif
#ifdef receive_mail
	case POP_RECEIVE_USER:
		ch395mail_pop3LoginUser();
		break;
	case POP_RECEIVE_PASS:
		ch395mail_pop3LoginPass();
		break;
	case POP_RECEIVE_STAT:
		ch395mail_pop3Stat();
		break;
	case POP_RECEIVE_LIST:
		ch395mail_pop3List();
		break;
	case POP_ERR_CHECK:
		ch395mail_Quit(ReceSocket);
		i = CH395CMDCloseSocket(ReceSocket);
		mStopIfError(i);
		break;
	case POP_RECEIVE_QUIT:
		ch395mail_Quit(ReceSocket);
		break;
	case POP_CLOSE_SOCKET:
		CheckType = uncheck;
#if DEBUG
		printf("close pop3 socket\n");
#endif
		i = CH395CMDCloseSocket(ReceSocket);
		mStopIfError(i);
#ifdef send_over_receive
		OrderType = SMTP_SEND_QUIT;
#endif
		break;
	case POP_RECEIVE_START:
		CH395SocketInitOpen(ReceSocket);
		break;
#endif
	default:
#if DEBUG
		printf("COMMAND UNUSEFULL %x\n", choiceorder);
#endif
		break;
	}
}

/*****************************************************************************
 * Function Name  : CH395_MailCmd
 * Description    : CH395 Mail Cmd handle
 * Input          : command type
 * Output         : None
 * Return         : None
 *****************************************************************************/
void CH395_MailCmd(u8 choiceorder)
{

	ch395mail_MailCommand(choiceorder);
#ifdef POP_RTER
	if (choiceorder == POP_RECEIVE_RTER)
	{
		ch395mail_pop3Retr('1'); /* Process the entire text content of the server's email (input the email number) */
	}
#endif
#ifdef POP_DELE
	if (choiceorder == POP_RECEIVE_DELE)
		ch395mail_pop3Dele('1'); /* Mark the email for deletion on the server; the deletion will be finalized only when the QUIT command is executed (input the email number) */
#endif
#ifdef POP_RSET
	if (choiceorder == POP_RECEIVE_RSET)
		ch395mail_pop3Rset(); /* Undo all DELE commands */
#endif
#ifdef POP_TOP
	if (choiceorder == POP_RECEIVE_TOP)
		ch395mail_pop3Top('1', '3'); /* Return the first m lines of the n-th email's content (input the email number, line number (must be a natural number)) */
#endif
#ifdef POP_UIDL
	if (choiceorder == POP_RECEIVE_UIDL)
		ch395mail_pop3Uidl('1'); /* Process the server's return of the unique identifier for the specified email; if not specified, return all. (input the email number) */
#endif
}
