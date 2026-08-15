/********************************** (C) COPYRIGHT *********************************
 * File Name          : CH395.H
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 structure description
 **********************************************************************************/
#ifndef __CH395_H__
#define __CH395_H__

#include "stdio.h"
#include "string.h"
#include "ch395inc.h"
#include "ch395cmd.h"

/* CH395Q 板级接口。 */
void ch395PortInit(void);          /* 初始化 SPI1、PD7 RST 和 PG6 INT#。 */
void ch395Reset(void);               /* 通过 PD7 低电平脉冲复位 CH395Q。 */
uint8_t ch395QueryInterrupt(void);     /* 读取 INT#：0 为有中断，1 为无中断。 */

/* CH395Q 命令传输接口，供 ch395cmd.c 使用。 */
void ch395WriteCommand(uint8_t cmd);      /* 开始事务并写入命令码。 */
void ch395WriteData(uint8_t mdata);   /* 写入一个数据字节。 */
uint8_t ch395ReadData(void);          /* 读取一个数据字节。 */
void ch395EndCommand(void);             /* 结束命令事务。 */

// CH395Q使用静态ip初始化
int ch395StaticInit(uint8_t ip[4],uint8_t gateway[4],uint8_t netmask[4],uint8_t mac[6]);

struct _CH395_SYS
{
    uint8_t IPAddr[4];        /* CH395 IP addr 32bit*/
    uint8_t GWIPAddr[4];      /* CH395 GW IP addr 32bit*/
    uint8_t MASKAddr[4];      /* CH395 MASK addr 32bit*/
    uint8_t MacAddr[6];       /* CH395 MAC addr 48bit*/
    uint8_t PHYStat;          /* CH395 PHY state 8bit*/
    uint8_t IntfMode;         /* Interface mode */
    uint8_t UnreachIPAddr[4]; /* Unreachable IP */
    uint16_t UnreachPort;     /* Unreachable Port */
};

struct _SOCK_INF
{
    uint8_t IPAddr[4];  /* Socket des IP addr 32bit*/
    uint8_t IPv6Addr[16];  /* Socket des IPv6 addr 128bit,Only CH395X is supported*/
    uint8_t ProtoType;  /* Proto type */
    uint8_t ScokStatus; /* Socket status */
    uint16_t DesPort;   /* DesPort */
    uint16_t SourPort;  /* SourPort */
    uint16_t SendLen;   /* Send date len */
    uint16_t RemLen;    /* Remain len */
    uint8_t *pSend;     /* Send point */
};

#endif
