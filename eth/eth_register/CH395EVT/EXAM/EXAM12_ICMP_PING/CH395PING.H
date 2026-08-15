/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395PING.H
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : Function descriptions
 **********************************************************************************/
#ifndef __CH395PING_H__
#define __CH395PING_H__

#define ICMP_SOKE_CON 0  /* Socket connection status */
#define ICMP_SEND_ERR 1  /* Error sending ICMP packet */
#define ICMP_SEND_SUC 2  /* Successfully sent ICMP packet */
#define ICMP_RECV_ERR 3  /* Error receiving ICMP packet */
#define ICMP_RECV_SUC 4  /* Successfully received ICMP packet */
#define ICMP_UNRECH 5    /* Destination unreachable */
#define ICMP_REPLY 6     /* ICMP reply type */
#define ICMP_REPLY_SUC 7 /* Successful ICMP reply */
#define ICMP_KEEP_NO 10  /* No keep-alive response */

#define ICMP_HEAD_TYPE 8   /* ICMP header type */
#define ICMP_HEAD_REPLY 0  /* ICMP reply type */
#define ICMP_HEAD_CODE 0   /* Code number within the type */
#define ICMP_HEAD_ID 512   /* Unique identifier */
#define ICMP_HEAD_SEQ 100  /* Sequence number */
#define ICMP_DATA_BYTES 32 /* ICMP data length in bytes */

/* ICMP header field data structure */
typedef struct _icmphdr
{
    UINT8 i_type;     /* ICMP message type */
    UINT8 i_code;     /* Code number within the type */
    UINT16 i_cksum;   /* Checksum */
    UINT16 i_id;      /* Unique identifier */
    UINT16 i_seq;     /* Sequence number */
    UINT8 i_data[32]; /* ICMP data */
} IcmpHeader, *IcmpHead;

extern UINT8 IcmpResp; /* ICMP response status */
extern UINT8 IcmpSuc;  /* ICMP success status */
extern UINT8 IcmpCont; /* ICMP continuation status */

void InitParameter(void); /* Initialize parameters */

void CH395PINGCmd(void); /* Query status, execute sub-command */

void InitPing(void); /* Initialize PING functionality */

void CH395IcmpRecvData(UINT32 len, UINT8 *pDat); /* Receive ICMP data */
#endif
