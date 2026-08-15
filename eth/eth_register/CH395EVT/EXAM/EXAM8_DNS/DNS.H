/********************************** (C) COPYRIGHT *********************************
 * File Name          : DNS.H
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : DNS interface file
 **********************************************************************************/
#ifndef _DNS_H_
#define _DNS_H_

#include "debug.h"

#define MAX_DNS_BUF_SIZE 512 /* Maximum DNS buffer size */

extern UINT8 status;
extern UINT8 dns_buf[MAX_DNS_BUF_SIZE];
extern UINT8 DNS_SERVER_IP[4]; /* DNS server IP */

#define IPPORT_DOMAIN 53 /* Default DNS port */

/*
********************************************************************************
Define Part
********************************************************************************
*/

#define TYPE_A 1      /* Host address */
#define TYPE_NS 2     /* Name server */
#define TYPE_MD 3     /* Mail destination (obsolete) */
#define TYPE_MF 4     /* Mail forwarder (obsolete) */
#define TYPE_CNAME 5  /* Canonical name */
#define TYPE_SOA 6    /* Start of Authority */
#define TYPE_MB 7     /* Mailbox name (experimental) */
#define TYPE_MG 8     /* Mail group member (experimental) */
#define TYPE_MR 9     /* Mail rename name (experimental) */
#define TYPE_NULL 10  /* Null (experimental) */
#define TYPE_WKS 11   /* Well-known sockets */
#define TYPE_PTR 12   /* Pointer record */
#define TYPE_HINFO 13 /* Host information */
#define TYPE_MINFO 14 /* Mailbox information (experimental) */
#define TYPE_MX 15    /* Mail exchanger */
#define TYPE_TXT 16   /* Text strings */
#define TYPE_ANY 255  /* Matches any type */

#define CLASS_IN 1 /* The ARPA Internet */

struct dhdr
{
  UINT16 id; /* Identifier */
  UINT8 qr;  /* Query or response flag */
  UINT8 opcode;
  UINT8 aa;       /* Authoritative answer */
  UINT8 tc;       /* Truncation flag */
  UINT8 rd;       /* Recursion desired */
  UINT8 ra;       /* Recursion available */
  UINT8 rcode;    /* Response code */
  UINT16 qdcount; /* Number of questions */
  UINT16 ancount; /* Number of answers */
  UINT16 nscount; /* Number of authority records */
  UINT16 arcount; /* Number of additional records */
};

UINT16 get16(UINT8 *s);

int ParseName(UINT8 *msg, UINT8 *compressed, char *buf);

UINT8 *DnsQuestion(UINT8 *msg, UINT8 *cp);

UINT8 *DnsAnswer(UINT8 *msg, UINT8 *cp, UINT8 *pSip);

UINT8 parseMSG(struct dhdr *pdhdr, UINT8 *pbuf, UINT8 *pSip);

UINT8 *put16(UINT8 *s, UINT16 i);

UINT16 MakeDnsQueryMsg(UINT16 op, char *name, UINT8 *buf, UINT16 len);

UINT8 DnsQuery(UINT8 s, UINT8 *name, UINT8 *pSip);

#endif
