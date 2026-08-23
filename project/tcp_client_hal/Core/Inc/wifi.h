#ifndef __WIFI_H
#define __WIFI_H

#include "esp32.h"

void WIFI_Init(void);

void WIFI_STA_MODE(void);

void WIFI_AP_MODE(void);

void WIFI_StartUdpClient(void);

void WIFI_ReadUdpData(uint8_t rxBuff[],uint16_t maxSize,uint8_t *ip,uint16_t *port,uint16_t *realReceveSize);

void WIFI_SendUdpData(uint8_t *msg,uint16_t size);

#endif