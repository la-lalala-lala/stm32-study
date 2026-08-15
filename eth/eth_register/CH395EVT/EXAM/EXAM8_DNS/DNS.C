/********************************** (C) COPYRIGHT *********************************
 * File Name          : DNS.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : DNS interface file
 **********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dns.h"
#include "CH395CMD.h"
#include "CH395.h"
#include "CH395INC.h"
#include "debug.h"

extern UINT8 DNS_SERVER_IP[4];
UINT16 MSG_ID = 0x1100;
UINT8 dns_buf[MAX_DNS_BUF_SIZE];
UINT8 status;
UINT16 count = 0;
extern UINT16 Socket0SourPort;
extern struct _SOCK_INF SockInf[2];
extern struct _CH395_SYS CH395Inf;

/*********************************************************************************
 * Function Name  : get16
 * Description    : The buffer UINT8 data is converted to UINT16 format
 * Input          : UINT8 date
 * Output         : None
 * Return         : UINT16 date
 **********************************************************************************/
UINT16 get16(UINT8 *s)
{
    UINT16 i;
    i = *s++ << 8;
    i = i + *s;
    return i;
}

/**********************************************************************************
 * Function Name  : ParseName
 * Description    : Analyze the full domain name
 * Input          : msg          - pointer to the message
                    compressed   - Pointer to the primary domain name in the packet
                    buf          - Buffer pointer to the converted domain name
 * Output         : None
 * Return         : Compressed message length
**********************************************************************************/
int ParseName(UINT8 *msg, UINT8 *compressed, char *buf)
{
    UINT16 slen; /* Current segment length */
    UINT8 *cp;
    int clen = 0; /* Compressed domain length  */
    int indirect = 0;
    int nseg = 0; /* The total number of domain name segments that are split */
    cp = compressed;
    for (;;)
    {
        slen = *cp++; /* The count value of the first byte */
        if (!indirect)
            clen++;
        if ((slen & 0xc0) == 0xc0)
        {
            if (!indirect)
                clen++;
            indirect = 1;
            cp = &msg[((slen & 0x3f) << 8) + *cp];
            slen = *cp++;
        }
        if (slen == 0) /* If the count is 0, end */
            break;
        if (!indirect)
            clen += slen;
        while (slen-- != 0)
            *buf++ = (char)*cp++;
        *buf++ = '.';
        nseg++;
    }
    if (nseg == 0)
    {
        *buf++ = '.';
    }
    *buf++ = '\0';
    return clen; /* Compressed message length */
}

/*********************************************************************************
 * Function Name  : DnsQuestion
 * Description    : Analyze the problem record portion of the response message
 * Input          : msg         - Pointer to the response message
                    cp          - A pointer to the count problem
 * Output         : None
 * Return         : A pointer to the next record
**********************************************************************************/
UINT8 *DnsQuestion(UINT8 *msg, UINT8 *cp)
{
    int len;
    char name[MAX_DNS_BUF_SIZE];
    len = ParseName(msg, cp, name);
    cp += len;
    cp += 2; /* type */
    cp += 2; /* class */
    return cp;
}

/*********************************************************************************
 * Function Name  : DnsAnswer
 * Description    : Analyze the answer record portion of the response message
 * Input          : msg           - Pointer to the response message
                    cp            - A pointer to the count problem
                    psip          - search result
 * Output         : None
 * Return         : A pointer to the next record
**********************************************************************************/
UINT8 *DnsAnswer(UINT8 *msg, UINT8 *cp, UINT8 *pSip)
{
    int len, type;
    char name[MAX_DNS_BUF_SIZE];
    len = ParseName(msg, cp, name);
    cp += len;
    type = get16(cp);
    cp += 2; /* type */
    cp += 2; /* class */
    cp += 4; /* ttl */
    cp += 2; /* Resource data length */
    switch (type)
    {
    case TYPE_A:
        pSip[0] = *cp++;
        pSip[1] = *cp++;
        pSip[2] = *cp++;
        pSip[3] = *cp++;
        break;
    case TYPE_CNAME:
    case TYPE_MB:
    case TYPE_MG:
    case TYPE_MR:
    case TYPE_NS:
    case TYPE_PTR:
        len = ParseName(msg, cp, name);
        cp += len;
        break;
    case TYPE_HINFO:

    case TYPE_MX:

    case TYPE_SOA:

    case TYPE_TXT:
        break;
    default:
        break;
    }
    return cp;
}
/*********************************************************************************
 * Function Name  : parseMSG
 * Description    : Analyze the resource record portion of the response message
 * Input          : msg           - Pointer to DNS packet header
                    cp            - Pointer to the response message
                    pSip          - search result
 * Output         : None
 * Return         : Returns 1 on success, 0 otherwise
**********************************************************************************/
UINT8 parseMSG(struct dhdr *pdhdr, UINT8 *pbuf, UINT8 *pSip)
{
    UINT16 tmp;
    UINT16 i;
    UINT8 *msg;
    UINT8 *cp;
    msg = pbuf;
    memset(pdhdr, 0, sizeof(pdhdr));
    pdhdr->id = get16(&msg[0]);
    tmp = get16(&msg[2]);
    if (tmp & 0x8000)
        pdhdr->qr = 1;
    pdhdr->opcode = (tmp >> 11) & 0xf;
    if (tmp & 0x0400)
        pdhdr->aa = 1;
    if (tmp & 0x0200)
        pdhdr->tc = 1;
    if (tmp & 0x0100)
        pdhdr->rd = 1;
    if (tmp & 0x0080)
        pdhdr->ra = 1;
    pdhdr->rcode = tmp & 0xf;
    pdhdr->qdcount = get16(&msg[4]);
    pdhdr->ancount = get16(&msg[6]);
    pdhdr->nscount = get16(&msg[8]);
    pdhdr->arcount = get16(&msg[10]);
    /* Analyze the variable data length portion */
    cp = &msg[12];
    for (i = 0; i < pdhdr->qdcount; i++) /* Query question */
    {
        cp = DnsQuestion(msg, cp);
    }
    for (i = 0; i < pdhdr->ancount; i++) /* answer */
    {
        cp = DnsAnswer(msg, cp, pSip);
    }
    for (i = 0; i < pdhdr->nscount; i++) /*authorization */
    {
        /* To be resolved */;
    }
    for (i = 0; i < pdhdr->arcount; i++) /* additional information */
    {
        /* To be resolved */;
    }
    if (pdhdr->rcode == 0)
        return 1; /*rcode = 0: success */
    else
        return 0;
}

/*********************************************************************************
 * Function Name  : put16
 * Description    : The data UINT16 format is stored in the buffer as UINT8
 * Input          : s     - The first address of the buffer
                    i     - UINT16 date
 * Output         : None
 * Return         : Offset pointer
**********************************************************************************/
UINT8 *put16(UINT8 *s, UINT16 i)
{
    *s++ = i >> 8;
    *s++ = i;
    return s;
}

/**********************************************************************************
 * Function Name  : MakeDnsQuery
 * Description    : Creating DNS query packets
 * input          : op   - recursion
 *                  name - Pointer to the domain name to be searched
 *                  buf  - DNS buffer
 *                  len  - Maximum buffer length
 * Output         : None
 * Return         : Pointer to DNS packet
**********************************************************************************/
UINT16 MakeDnsQueryMsg(UINT16 op, char *name, UINT8 *buf, UINT16 len)
{
    UINT8 *cp;
    char *cp1;
    char tmpname[MAX_DNS_BUF_SIZE];
    char *dname;
    UINT16 p;
    UINT16 dlen;
    cp = buf;
    MSG_ID++;
    cp = put16(cp, MSG_ID); /* identification */
    p = (op << 11) | 0x0100;
    cp = put16(cp, p); /* 0x0100:Recursion desired */
    cp = put16(cp, 1); /* Number of questions: 1 */
    cp = put16(cp, 0); /* Number of resource records: 0 */
    cp = put16(cp, 0); /* Number of resource records: 0 */
    cp = put16(cp, 0); /* Number of additional resource records: 0 */
    strcpy(tmpname, name);
    dname = tmpname;
    dlen = strlen(dname);
    for (;;)
    { /* The URI is written into buf according to the DNS request packet domain name format*/
        cp1 = strchr(dname, '.');
        if (cp1 != NULL)
            len = cp1 - dname;
        else
            len = dlen;
        *cp++ = len;
        if (len == 0)
            break;
        strncpy((char *)cp, dname, len);
        cp += len;
        if (cp1 == NULL)
        {
            *cp++ = 0;
            break;
        }
        dname += len + 1; /* Move dname first address backward*/
        dlen -= len + 1;  /* dlen length decreases*/
    }
    cp = put16(cp, 0x0001); /* type:1------ip address */
    cp = put16(cp, 0x0001); /* class:1-------eth addr*/
    return ((UINT16)(cp - buf));
}

/*********************************************************************************
 * Function Name  : UDPSocketParamInit
 * Description    : Initialize socket
 * Input          : sockindex     - socket index  
                    addr          - des ip
                    SourPort      - sour ip
                    DesPort       - des port
 * Output         : None
 * Return         : None
**********************************************************************************/
void UDPSocketParamInit(UINT8 sockindex, UINT8 *addr, UINT16 SourPort, UINT16 DesPort)
{
    UINT8 i;
    memset(&SockInf[sockindex], 0, sizeof(SockInf[sockindex])); /* Clear all SockInf [0] */
    memcpy(SockInf[sockindex].IPAddr, addr, sizeof(addr));      /* Write the destination IP address */
    SockInf[sockindex].DesPort = DesPort;                       /* Des port */
    SockInf[sockindex].SourPort = SourPort;                     /* Sour port */
    SockInf[sockindex].ProtoType = PROTO_TYPE_UDP;

    CH395CMDSetSocketDesIP(sockindex, SockInf[sockindex].IPAddr);      /* Set socket 0 des ip */
    CH395CMDSetSocketProtType(sockindex, PROTO_TYPE_UDP);              /* Set socket 0 protocol type */
    CH395CMDSetSocketDesPort(sockindex, SockInf[sockindex].DesPort);   /* Set socket 0 des port */
    CH395CMDSetSocketSourPort(sockindex, SockInf[sockindex].SourPort); /* Set socket 0 sour port */
    i = CH395CMDOpenSocket(sockindex);                                 /* Open socket 0 */
    mStopIfError(i);
}

/*********************************************************************************
 * Function Name  : UDPSendData
 * Description    : UDP Send data
 * Input          : sockindex     - socket index  
                    databuf       - send buffer
                    len           - send len
 * Output         : None
 * Return         : None
 **********************************************************************************/
UINT16 UDPSendData(UINT8 sockindex, UINT8 *databuf, UINT16 len)
{
    UINT16 ret;
    ret = len;
    CH395CMDSendData(sockindex, databuf, len);
    return ret;
}

/***********************************************************************************
 * Function Name  : DnsQuery
 * Description    : Dns query
 *input           : s    - socket indexes
                    name - Pointer to the domain name to be queried
                    pSip - search result
 * Output         : None
 * Return         : Query results. Return 1 if successful, return - 1 if failed
**********************************************************************************/
UINT8 DnsQuery(UINT8 s, UINT8 *name, UINT8 *pSip)
{
    struct dhdr dhp;
    UINT16 len, cnt;
    if (status > 1)
    {
        count++;
        printf("count = %2d\n", (UINT16)count);
        Delay_Ms(2);
        if (count > 2000)
        {
            printf("DNS Fail!!!!\n");
            count = 0;
            status = 0;
        }
    }
    if (status == 1)
    {
        UDPSocketParamInit(s, DNS_SERVER_IP, Socket0SourPort, IPPORT_DOMAIN);
        status = 2;
        printf("status = 2!\n");
    }
    if (status == 2)
    {
        Delay_Ms(200);
        len = MakeDnsQueryMsg(0, (char *)name, dns_buf, MAX_DNS_BUF_SIZE);
        cnt = UDPSendData(s, dns_buf, len);
        if (cnt == 0)
            return (0);
        else
        {
            status = 3;
            printf("status = 3!\n");
        }
    }
    if (status == 4)
    {
        return (parseMSG(&dhp, dns_buf, pSip));
    }
    return 0;
}
