/********************************** (C) COPYRIGHT *********************************
 * File Name          : Moduleconfig.H
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395.C Data Structure Definitions
 *
 **********************************************************************************/

#ifndef __MODULECONFIG_H__
#define __MODULECONFIG_H__

#define NET_MODULE_DATA_LENGTH 60 // Maximum data length when communicating with the module.

#define MODULE_CFG_LEN 42 // Length of the module configuration structure.

// Communication command codes
#define NET_MODULE_CMD_SET 0X01    // Configure a module in the network
#define NET_MODULE_CMD_GET 0X02    // Retrieve the configuration of a specific module
#define NET_MODULE_CMD_RESET 0X03  // Reset the configuration of a specific module
#define NET_MODULE_CMD_SEARCH 0X04 // Search for modules in the network

// Response command codes
#define NET_MODULE_ACK_SET 0X81    // Response to the configuration command
#define NET_MODULE_ACK_GET 0X82    // Response to the retrieval command
#define NET_MODULE_ACK_RESET 0X83  // Response to the reset command
#define NET_MODULE_ACK_SEARCH 0X84 // Response to the search command

#define NET_MODULE_NAME "CH395NET_MODULE" // Identifier for communication

#define SynCode1 0x55
#define SynCode2 0xaa
// Default configuration

// Network communication structure
typedef struct NET_COMM
{
  unsigned char flag[16];                    // Communication identifier. Since communication is broadcast-based, a fixed value is added here.
  unsigned char cmd;                         // Command header
  unsigned char id[6];                       // Identifier (CH395 MAC address)
  unsigned char len;                         // Length of the data section
  unsigned char dat[NET_MODULE_DATA_LENGTH]; // Data buffer
} net_comm, *pnet_comm;

// Module identification
#define NET_MODULE_TYPE_TCP_S 0X00 // Module as a TCP SERVER
#define NET_MODULE_TYPE_TCP_C 0X01 // Module as a TCP CLIENT
#define NET_MODULE_TYPE_UDP_S 0X02 // Module as a UDP SERVER
#define NET_MODULE_TYPE_UDP_C 0X03 // Module as a UDP CLIENT

// Module configuration structure
typedef struct MODULE_CFG
{
  unsigned char module_name[21]; // Module's name
  unsigned char type;            // Indicates the module's mode (TCP/UDP server/client)
  unsigned char src_ip[4];       // Module's IP address
  unsigned char mask[4];         // Module's subnet mask
  unsigned char getway[4];       // Module's gateway address
  unsigned char src_port[2];     // Module's source port
  unsigned char dest_ip[4];      // Destination IP address
  unsigned char dest_port[2];    // Destination port
} module_cfg, *pmodule_cfg;

// Default configuration
extern UINT8 DefaultSrcIp[4];
extern UINT8 DefaultMask[4];
extern UINT8 DefaultGetway[4];
extern UINT16 DefaultSrcPort;
extern UINT8 ParseConfigbuf(u8 *buf);
extern UINT8 DefaultDesIp[4];
extern UINT16 DefaultDesPort;
extern UINT8 MODULE_NAME[21];

#endif
