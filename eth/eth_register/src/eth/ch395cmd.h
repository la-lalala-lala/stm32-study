/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH395CMD.H
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 command interface file
 *******************************************************************************/
#ifndef __CH395CMD_H__
#define __CH395CMD_H__
#include "ch395inc.h"

uint8_t ch395CmdGetVersion(void); /* Obtain the chip and firmware version number */

void ch395CmdEnterSleep(void); /* Enter low-power sleep suspended state */

void ch395CmdSetUartBaudRate(uint32_t baudrate); /* Set the baudrate for serial port communication */

void ch395CmdReset(void); /* Reset */

uint8_t ch395CmdCheckExist(uint8_t testdata); /* Test communication interface and working condition */

uint16_t ch395CmdGetGlobalInterruptStatus(void); /* Gets the CH395 global interrupt status */

void ch395CmdSetPhy(uint8_t PhyMode); /* Set the PHY mode */

uint8_t ch395CmdEnsureMacAddress(uint8_t *macaddr); /* Set MAC address，If already set, It will be ignored */

void ch395CmdSetMacAddress(uint8_t *macaddr); /* Set MAC address */

void ch395CmdSetIpAddress(uint8_t *ipaddr); /* Set IP address */

void ch395CmdSetGatewayIpAddress(uint8_t *gwipaddr); /* Set GWIP address */

void ch395CmdSetMaskAddress(uint8_t *maskaddr); /* Set MASK address */

void ch395CmdSetMacFilter(uint8_t filtype, uint32_t table0, uint32_t table1); /* Set MAC filtering. */

uint8_t ch395CmdGetPhyStatus(void); /* Gets the current PHY status */

uint8_t ch395CmdInitialize(void); /* Initialize CH395 */

void ch395CmdGetUnreachableIpPortProtocol(uint8_t *list); /* Get unreachable information (IP,Port,Protocol Type) */

void ch395CmdSetRetransmissionCount(uint8_t time); /* Set retry times */

void ch395CmdSetRetransmissionPeriod(uint16_t period); /* Set retry period */

uint8_t ch395CmdGetCommandStatus(void); /* Obtain the command execution status */

void ch395CmdGetRemoteIpPort(uint8_t sockindex, uint8_t *list); /*Obtain the port and IP address of the remote end */

void ch395CmdClearReceiveBuffer(uint8_t sockindex); /* Clear receive buffer  */

uint16_t ch395CmdGetSocketStatus(uint8_t sockindex); /* Obtain the socket n status */

uint8_t ch395CmdGetSocketInterrupt(uint8_t sockindex); /* Gets the interrupt status of socket n  */

void ch395CmdSetSocketDestinationIp(uint8_t sockindex, uint8_t *ipaddr); /* Set the destination IP address of socket n  */

void ch395CmdSetSocketDestinationPort(uint8_t sockindex, uint16_t desport); /* Set the destination port of socket n */

void ch395CmdSetSocketSourcePort(uint8_t sockindex, uint16_t sorport); /* Set the source port of socket n */

void ch395CmdSetSocketProtocolType(uint8_t sockindex, uint8_t prottype); /* Set the protocol type of socket n  */

uint8_t ch395CmdOpenSocket(uint8_t sockindex); /* Open socket n */

uint8_t ch395CmdTcpListen(uint8_t sockindex); /* TCP Listen */

uint8_t ch395CmdTcpConnect(uint8_t sockindex); /* TCP Connect */

uint8_t ch395CmdTcpDisconnect(uint8_t sockindex); /* TCP Disconnect */

void ch395CmdSendData(uint8_t sockindex, uint8_t *databuf, uint16_t len); /* Writes data to socket n buffer */

uint16_t ch395CmdGetReceiveLength(uint8_t sockindex); /* Gets the length of data received by socket n */

void ch395CmdGetReceiveData(uint8_t sockindex, uint16_t len, uint8_t *pbuf); /* Gets socket n receive buffer data */

uint8_t ch395CmdCloseSocket(uint8_t sockindex); /* Close socket n*/

void ch395CmdSetSocketIpRawProtocol(uint8_t sockindex, uint8_t prototype); /* In IP mode, configure the IP packet protocol field. */

void ch395CmdEnablePing(uint8_t enable); /* On/off PING*/

void ch395CmdGetMacAddress(uint8_t *macaddr); /* Gets the MAC address */

uint8_t ch395CmdEnableDhcp(uint8_t flag); /* Enable DHCP */

uint8_t ch395CmdGetDhcpStatus(void); /* Obtain DHCP status */

void ch395CmdGetIpInfo(uint8_t *addr); /* Get IP, subnet mask, gateway */

void ch395CmdSetArp(uint8_t period, uint8_t cnt); /* Set ARP retransmission period and number of times */

void ch395CmdSetTcpMss(uint16_t mss); /* Set TCP MSS */

void ch395CmdSetTtl(uint8_t sockindex, uint8_t TTLnum); /* Set the TTL */

void ch395CmdSetSocketReceiveBuffer(uint8_t sockindex, uint8_t startblk, uint8_t blknum); /* Sets the SOCKET receive buffer */

void ch395CmdSetSocketSendBuffer(uint8_t sockindex, uint8_t startblk, uint8_t blknum); /* Sets the SOCKET send buffer */

void ch395CmdSetFunctionParameters(uint8_t PARA1, uint8_t PARA2, uint8_t PARA3, uint8_t PARA4); /*Setup parameters*/

void ch395CmdSetKeepAliveIdle(uint32_t idle); /* Set KEEPLIVE idle time */

void ch395CmdSetKeepAliveInterval(uint32_t intvl); /* Set KEEPLIVE interval time */

void ch395CmdSetKeepAliveCount(uint8_t cnt); /* Set KEEPLIVE retries times */

void ch395CmdSetKeepAlive(uint8_t sockindex, uint8_t cmd); /* Set the socket n keeplive function */

uint8_t ch395CmdEepromErase(void); /* Erasure EEPROM */

uint8_t ch395CmdEepromWrite(uint16_t eepaddr, uint8_t *buf, uint8_t len); /* Write EEPROM */

void ch395CmdEepromRead(uint16_t eepaddr, uint8_t *buf, uint8_t len); /* Read EEPROM */

uint8_t ch395CmdReadGpioRegister(uint8_t regadd); /* Read GPIO register */

void ch395CmdWriteGpioRegister(uint8_t regadd, uint8_t regval); /* Write GPIO register */

void ch395UdpSendTo(uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t port, uint8_t sockindex); /*UDP mode Send to assign IP */

uint8_t ch395SetUartBaudRate(uint32_t baudrate); /* Set 395 Uart BaudRate */

void ch395CmdEnableIpv6AutoLinkLocalAddress(void); /* Enable automatic generation of Link-Local Address */

void ch395CmdSetIpv6LinkLocalAddress(uint8_t *pLLA); /* Set IPv6 Link-Local Address */

void ch395CmdSetIpv6GlobalAddress(uint8_t *pGUA); /* Set IPv6 Global Unicast Address */

void ch395CmdSetSocketDestinationIpv6Address(uint8_t sockindex, uint8_t *pIPv6); /* Set destination IPv6 address for socket n */

void ch395CmdGetIpv6UnreachableInfo(uint8_t *pBuf); /* Get IPv6 unreachable information (20 bytes) */

void ch395CmdGetRemoteIpv6(uint8_t sockindex, uint8_t *pBuf); /* Get remote IPv6 address and port of socket n (18 bytes) */

uint8_t ch395CmdEnableDhcpv6(uint8_t flag); /* Enable or disable DHCPv6 (1: enable, 0: disable) */

uint8_t ch395CmdGetDhcpv6Status(void); /* Get DHCPv6 status */

void ch395CmdGetIpv6Address(uint8_t *pBuf); /* Get IPv6 Link-Local and Global Unicast addresses (32 bytes) */

void ch395Udp6SendTo(uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t port, uint8_t sockindex); /* UDPv6 send data to specified IP and port */

#endif
/**************************** endfile *************************************/
