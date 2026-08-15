/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395PING.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 chip Ping-related functions
 **********************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "CH395INC.H"
#include "CH395CMD.H"
#include "CH395.H"
#include "CH395PING.H"

UINT8 UNREACH_COUNT;
UINT8 TIMOUT_COUNT;
UINT8 SUCRECV_COUNT;
UINT8 CH395INF_BUF[20];
UINT8 SEND_BUF[100];
UINT8 IcmpCont;
UINT8 IcmpSeq;
UINT8 IcmpSuc;
UINT8 icmp_tmp;
UINT32 TimeCount;

extern const UINT8 DestIPAddr[4];

/**********************************************************************************
 * Function Name  : InitPing
 * Description    : Ping initialization
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void InitParameter(void)
{
    UNREACH_COUNT = 0;
    TIMOUT_COUNT = 0;
    SUCRECV_COUNT = 0;
    IcmpCont = 0;
    IcmpSeq = 0;
    IcmpSuc = 0;
    icmp_tmp = 0;
    CH395CMDGetIPInf(CH395INF_BUF);
}

/**********************************************************************************
 * Function Name  : calculate_checksum
 * Description    : Calculate the checksum of the ICMP header
 * Input          : buf         - Pointer to the buffer containing the ICMP header
                    length      - Length of the ICMP header
 * Output         : None
 * Return         : Checksum value
 **********************************************************************************/
UINT16 calculate_checksum(UINT8 *buf, UINT32 length)
{
    UINT32 sum = 0;
    UINT16 *data = (UINT16 *)buf;

    // Add 16-bit values one by one
    while (length > 1)
    {
        sum += *data++;
        length -= 2;
    }

    // If the data length is odd, handle the last byte separately
    if (length > 0)
    {
        sum += *(UINT8 *)data;
    }

    // Handle the carry
    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // Return the one's complement
    return (UINT16)~sum;
}

/**********************************************************************************
 * Function Name  : InitPing
 * Description    : Ping initialization
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void InitPing(void)
{
    IcmpHeader head;
    UINT8 i;

    IcmpCont++;
    IcmpSeq += 10;
    head.i_type = ICMP_HEAD_TYPE;
    head.i_code = ICMP_HEAD_CODE;
    head.i_id = ICMP_HEAD_ID;
    head.i_seq = ICMP_HEAD_SEQ + IcmpSeq;
    memset(head.i_data, 0, sizeof(head.i_data));
    for (i = 0; i < ICMP_DATA_BYTES; i++)
    {
        if (i < 26)
            head.i_data[i] = i + 'a';
        else
            head.i_data[i] = i + 'a' - 26;
    }
    head.i_cksum = 0;
    head.i_cksum = calculate_checksum((UINT8 *)&head, sizeof(head));
    memset(SEND_BUF, 0, sizeof(SEND_BUF));
    memcpy(SEND_BUF, &head, sizeof(head));
}

/**********************************************************************************
 * Function Name  : InitPing
 * Description    : Ping initialization
 * Input          : Pointer to the buffer containing the ICMP header
 * Output         : None
 * Return         : None
 **********************************************************************************/
void Respond_Ping(UINT8 *pDat)
{
    IcmpHeader head;
    UINT8 i;

    head.i_type = ICMP_HEAD_REPLY;
    head.i_code = pDat[1];
    head.i_id = (pDat[5] << 8) + pDat[4];
    head.i_seq = (pDat[7] << 8) + pDat[6];
    for (i = 0; i < 32; i++)
    {
        head.i_data[i] = pDat[i + 8];
    }
    head.i_cksum = 0;
    head.i_cksum = calculate_checksum((UINT8 *)&head, sizeof(head));
    memset(SEND_BUF, 0, sizeof(SEND_BUF));
    memcpy(SEND_BUF, &head, sizeof(head));
}

/**********************************************************************************
 * Function Name  : InitPing
 * Description    : Ping initialization
 * Input          : len         - Length of the ICMP header
                    pDat        - Pointer to the buffer containing the ICMP header
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395IcmpRecvData(UINT32 len, UINT8 *pDat)
{
    UINT16 tmp = 0;

    icmp_tmp = IcmpSuc;
    IcmpSuc = ICMP_RECV_ERR;
    if (len == 40)
    {
        if (pDat[0] == ICMP_HEAD_REPLY)
        {
            if (pDat[1] == ICMP_HEAD_CODE)
            {
                tmp = pDat[5];
                tmp = tmp << 8;
                tmp += pDat[4];
                if (tmp == ICMP_HEAD_ID)
                {
                    tmp = pDat[7];
                    tmp = (tmp << 8);
                    tmp += pDat[6] - IcmpSeq;
                    if (tmp == ICMP_HEAD_SEQ)
                    {
                        IcmpSuc = ICMP_RECV_SUC;
                    }
                }
            }
        }
        if (pDat[0] == ICMP_HEAD_TYPE)
        {
            if (pDat[1] == ICMP_HEAD_CODE)
            {
                Respond_Ping(pDat);
                IcmpSuc = ICMP_REPLY;
            }
        }
    }
    else
    {
        if (pDat[0] == 3)
        {
            if (pDat[1] == 1)
            {
                IcmpSuc = ICMP_UNRECH;
            }
        }
    }
}
/**********************************************************************************
 * Function Name  : CH395PINGCmd
 * Description    : Query the status and execute the corresponding command
 * Input          : None
 * Output         : None
 * Return         : None
 **********************************************************************************/
void CH395PINGCmd(void)
{
    if (IcmpSuc <= ICMP_KEEP_NO)
    {
        switch (IcmpSuc)
        {
        case ICMP_SOKE_CON:
            IcmpSuc = ICMP_SEND_ERR;
            CH395CMDSendData(0, SEND_BUF, 40);
            printf("\nPinging %d.%d.%d.%d with %d bytes of data:\n\r\n", (UINT16)DestIPAddr[0], (UINT16)DestIPAddr[1],
                   (UINT16)DestIPAddr[2], (UINT16)DestIPAddr[3], (UINT16)ICMP_DATA_BYTES);
            TimeCount = 0;
            break;
        case ICMP_SEND_ERR:
            if (TimeCount > 250)
            {
                printf("send data fail!\n");
                CH395CMDSendData(0, SEND_BUF, 40);
                TimeCount = 0;
            }
            break;
        case ICMP_SEND_SUC:
            if (TimeCount > 2000)
            {
                printf("Request time out.\n");
                TIMOUT_COUNT++;
                if (IcmpCont < 4)
                {
                    IcmpSuc = 1;
                    InitPing();
                    CH395CMDSendData(0, SEND_BUF, 40);
                    TimeCount = 0;
                }
                else
                {
                    printf("\r\nPing statistics for %d.%d.%d.%d:\n    Packets: Sent = 4,Received = %d,Lost = %d<%d%% loss>.\n\r\n", (UINT16)DestIPAddr[0],
                           (UINT16)DestIPAddr[1], (UINT16)DestIPAddr[2], (UINT16)DestIPAddr[3], (UINT16)(4 - TIMOUT_COUNT), (UINT16)TIMOUT_COUNT,
                           (UINT16)(TIMOUT_COUNT * 25));
                    IcmpSuc = ICMP_KEEP_NO;
                }
            }
            break;
        case ICMP_RECV_ERR:
            printf("receive data unknown.\n\r\n");
            IcmpSuc = ICMP_KEEP_NO;
            break;
        case ICMP_RECV_SUC:
            printf("Reply from %d.%d.%d.%d: bytes=%d time<4ms\n", (UINT16)DestIPAddr[0], (UINT16)DestIPAddr[1], (UINT16)DestIPAddr[2],
                   (UINT16)DestIPAddr[3], (UINT16)ICMP_DATA_BYTES);
            SUCRECV_COUNT++;
            if (IcmpCont < 4)
            {
                IcmpSuc = 1;
                InitPing();
                TimeCount = 0;
                CH395CMDSendData(0, SEND_BUF, 40);
            }
            else
            {
                printf("\r\nPing statistics for %d.%d.%d.%d:\n    Packets: Sent = 4,Received = %d,Lost = %d<%d%% loss>.\n\r\n", (UINT16)DestIPAddr[0],
                       (UINT16)DestIPAddr[1], (UINT16)DestIPAddr[2], (UINT16)DestIPAddr[3], (UINT16)SUCRECV_COUNT, (UINT16)(4 - SUCRECV_COUNT),
                       (UINT16)((4 - SUCRECV_COUNT) * 25));
                IcmpSuc = ICMP_KEEP_NO;
            }
            break;
        case ICMP_UNRECH:
            printf("Reply from %d.%d.%d.%d: Destination host unreachable.\n", (UINT16)DestIPAddr[0], (UINT16)DestIPAddr[1],
                   (UINT16)DestIPAddr[2], (UINT16)DestIPAddr[3]);
            UNREACH_COUNT++;
            if (IcmpCont < 4)
            {
                IcmpSuc = 1;
                InitPing();
                TimeCount = 0;
                CH395CMDSendData(0, SEND_BUF, 40);
            }
            else
            {
                printf("\r\nPing statistics for %d.%d.%d.%d:\n    Packets: Sent = 4,Received = %d,Lost = %d<%d%% loss>.\n\r\n", (UINT16)DestIPAddr[0],
                       (UINT16)DestIPAddr[1], (UINT16)DestIPAddr[2], (UINT16)DestIPAddr[3], (UINT16)UNREACH_COUNT, (UINT16)(4 - UNREACH_COUNT),
                       (UINT16)((4 - UNREACH_COUNT) * 25));
                IcmpSuc = ICMP_KEEP_NO;
            }
            break;
        case ICMP_REPLY:
            CH395CMDSendData(0, SEND_BUF, 40);
            printf("REPLY ping.\n");
            IcmpSuc = ICMP_KEEP_NO;
            break;
        case ICMP_REPLY_SUC:
            printf("Reply ping.\n\r\n");
            IcmpSuc = icmp_tmp;
            break;
        case ICMP_KEEP_NO:
            IcmpSuc = 0xff;
            break;
        default:
            break;
        }
    }
}
