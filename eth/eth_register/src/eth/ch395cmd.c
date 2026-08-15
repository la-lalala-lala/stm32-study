/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH395CMD.C
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : CH395 command interface file
 *******************************************************************************/
#include "ch395.h"
#include "delay/delay.h"

/********************************************************************************
 * Function Name  : ch395CmdGetVersion
 * Description    : Obtain the chip and firmware version number
 * Input          : None
 * Output         : None
 * Return         : Version number
 *******************************************************************************/
uint8_t ch395CmdGetVersion(void)
{
    uint8_t i;
    ch395WriteCommand(CMD01_GET_IC_VER);
    i = ch395ReadData();
    ch395EndCommand();
    return i;
}

/********************************************************************************
 * Function Name  : ch395CmdSetUartBaudRate
 * Description    : Set the baudrate for serial port communication
 * Input          : baudrate
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetUartBaudRate(uint32_t baudrate)
{
    ch395WriteCommand(CMD31_SET_BAUDRATE);
    ch395WriteData((uint8_t)baudrate);
    ch395WriteData((uint8_t)((uint16_t)baudrate >> 8));
    ch395WriteData((uint8_t)(baudrate >> 16));
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdEnterSleep
 * Description    : Enter low-power sleep suspended state
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdEnterSleep(void)
{
    ch395WriteCommand(CMD00_ENTER_SLEEP);
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdReset
 * Description    : Reset the CH395
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdReset(void)
{
    ch395WriteCommand(CMD00_RESET_ALL);
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdCheckExist
 * Description    : Test communication interface and working condition
 * Input          : testdata
 * Output         : None
 * Return         : bitwise inverse of input data
 *******************************************************************************/
uint8_t ch395CmdCheckExist(uint8_t testdata)
{
    uint8_t i;
    ch395WriteCommand(CMD11_CHECK_EXIST);
    ch395WriteData(testdata);
    i = ch395ReadData();
    ch395EndCommand();
    return i;
}

/*******************************************************************************
 * Function Name  : ch395CmdGetGlobalInterruptStatus
 * Description    : Gets the CH395 global interrupt status
 * Input          : None
 * Output         : None
 * Return         : Global interrupt status
 *******************************************************************************/
uint16_t ch395CmdGetGlobalInterruptStatus(void)
{
    uint16_t init_status;
    ch395WriteCommand(CMD02_GET_GLOB_INT_STATUS_ALL);
    init_status = ch395ReadData();
    init_status |= (uint16_t)ch395ReadData() << 8;
    ch395EndCommand();
    return init_status;
}

/*******************************************************************************
 * Function Name  : ch395CmdSetPhy
 * Description    : Set the PHY mode
 * Input          : PHY mode
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetPhy(uint8_t PhyMode)
{
    ch395WriteCommand(CMD10_SET_PHY);
    ch395WriteData(PhyMode);
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdEnsureMacAddress
 * Description    : Set MAC address，If already set, It will be ignored
 * Input          : macaddr
 * Output         : None
 * Return         : Return Setting Result. Success-> 0，Failed-> -1
 *******************************************************************************/
uint8_t ch395CmdEnsureMacAddress(uint8_t *macaddr){
     uint8_t current_mac[6];

    ch395CmdGetMacAddress(current_mac);

    /* 已经是目标 MAC，不再写 EEPROM */
    if (memcmp(current_mac, macaddr, sizeof(current_mac)) == 0)
    {
        printf("MAC already configured\r\n");
        return 0;
    }

    /* 第一次配置，或目标 MAC 已发生变化 */
    ch395CmdSetMacAddress(macaddr);
    Delay_ms(100);

    /* 写入后回读校验 */
    ch395CmdGetMacAddress(current_mac);
    if (memcmp(current_mac, macaddr, sizeof(current_mac)) != 0)
    {
        printf("MAC write verify failed\r\n");
        return -1;
    }

    return 0;
}


/********************************************************************************
 * Function Name  : ch395CmdSetMacAddress
 * Description    : Set MAC address
 * Input          : macaddr
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetMacAddress(uint8_t *macaddr)
{
    uint8_t i;
    ch395WriteCommand(CMD60_SET_MAC_ADDR);
    for (i = 0; i < 6; i++)
        ch395WriteData(*macaddr++);
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdSetIpAddress
 * Description    : Set IP address
 * Input          : ipaddr
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetIpAddress(uint8_t *ipaddr)
{
    uint8_t i;
    ch395WriteCommand(CMD40_SET_IP_ADDR);
    for (i = 0; i < 4; i++)
        ch395WriteData(*ipaddr++);
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdSetGatewayIpAddress
 * Description    : Set GWIP address
 * Input          : ipaddr
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetGatewayIpAddress(uint8_t *gwipaddr)
{
    uint8_t i;
    ch395WriteCommand(CMD40_SET_GWIP_ADDR);
    for (i = 0; i < 4; i++)
        ch395WriteData(*gwipaddr++);
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdSetMaskAddress
 * Description    : Set MASK address ,The default is 255.255.255.0
 * Input          : maskaddr
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetMaskAddress(uint8_t *maskaddr)
{
    uint8_t i;
    ch395WriteCommand(CMD40_SET_MASK_ADDR);
    for (i = 0; i < 4; i++)
        ch395WriteData(*maskaddr++);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetMacFilter
 * Description    : Set MAC filtering
 * Input          : filtype     - Preference of Filtering
                    table0      - Hash0
                    table1      - Hash1
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395CmdSetMacFilter(uint8_t filtype, uint32_t table0, uint32_t table1)
{
    ch395WriteCommand(CMD90_SET_MAC_FILT);
    ch395WriteData(filtype);
    ch395WriteData((uint8_t)table0);
    ch395WriteData((uint8_t)((uint16_t)table0 >> 8));
    ch395WriteData((uint8_t)(table0 >> 16));
    ch395WriteData((uint8_t)(table0 >> 24));

    ch395WriteData((uint8_t)table1);
    ch395WriteData((uint8_t)((uint16_t)table1 >> 8));
    ch395WriteData((uint8_t)(table1 >> 16));
    ch395WriteData((uint8_t)(table1 >> 24));
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdGetPhyStatus
 * Description    : Gets the current PHY status
 * Input          : None
 * Output         : None
 * Return         : Current PHY status. See PHY Parameter definition for the status definition
 *******************************************************************************/
uint8_t ch395CmdGetPhyStatus(void)
{
    uint8_t i;
    ch395WriteCommand(CMD01_GET_PHY_STATUS);
    i = ch395ReadData();
    ch395EndCommand();
    return i;
}

/********************************************************************************
 * Function Name  : ch395CmdInitialize
 * Description    : Initialize CH395
 * Input          : None
 * Output         : None
 * Return         : uint8_t s
 *******************************************************************************/
uint8_t ch395CmdInitialize(void)
{
    uint8_t i = 0;
    uint8_t s = 0;
    ch395WriteCommand(CMD0W_INIT_CH395);
    ch395EndCommand();
    while (1)
    {
        Delay_ms(20);               /* Delay query, more than 20MS is recommended*/
        s = ch395CmdGetCommandStatus(); /* Do not query too frequently*/
        if (s != CH395_ERR_BUSY)
            break;
        if (i++ > 200)
            return CH395_ERR_UNKNOW; /* Timeout exits. This function needs more than 400MS to complete */
    }
    return s;
}

/********************************************************************************
 * Function Name  : ch395CmdGetUnreachableIpPortProtocol
 * Description    : Get unreachable information (IP,Port,Protocol Type)
 * Input          : list
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdGetUnreachableIpPortProtocol(uint8_t *list)
{
    uint8_t i;
    ch395WriteCommand(CMD08_GET_UNREACH_IPPORT);
    for (i = 0; i < 8; i++)
    {
        *list++ = ch395ReadData();
    }
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdSetRetransmissionCount
 * Description    : Set retry times
 * Input          : retry times
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetRetransmissionCount(uint8_t time)
{
    ch395WriteCommand(CMD10_SET_RETRAN_COUNT);
    ch395WriteData(time);
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdSetRetransmissionPeriod
 * Description    : Set retry period
 * Input          : retry period
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetRetransmissionPeriod(uint16_t period)
{
    ch395WriteCommand(CMD20_SET_RETRAN_PERIOD);
    ch395WriteData((uint8_t)period);
    ch395WriteData((uint8_t)(period >> 8));
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdGetCommandStatus
 * Description    : Obtain the command execution status
 * Input          : None
 * Output         : None
 * Return         : Command execution status
 *******************************************************************************/
uint8_t ch395CmdGetCommandStatus(void)
{
    uint8_t i;
    ch395WriteCommand(CMD01_GET_CMD_STATUS);
    i = ch395ReadData();
    ch395EndCommand();
    return i;
}

/********************************************************************************
 * Function Name  : ch395CmdGetRemoteIpPort
 * Description    : Obtain the port and IP address of the remote end
 * Input          : sockindex      - sockindex
                    list           - Save the IP address and port number
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395CmdGetRemoteIpPort(uint8_t sockindex, uint8_t *list)
{
    uint8_t i;

    ch395WriteCommand(CMD06_GET_REMOT_IPP_SN);
    ch395WriteData(sockindex);
    for (i = 0; i < 6; i++)
    {
        *list++ = ch395ReadData();
    }
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdClearReceiveBuffer
 * Description    : Clear receive buffer
 * Input          : sockindex
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdClearReceiveBuffer(uint8_t sockindex)
{
    ch395WriteCommand(CMD10_CLEAR_RECV_BUF_SN);
    ch395WriteData((uint8_t)sockindex);
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdGetSocketStatus
 * Description    : Obtain the socket n status
 * Input          : sockindex
 * Output         : None
 * Return         : First byte     - socket n open or closed
                    Second byte    - TCP state, meaningful only if TCP mode and the first byte is open
*******************************************************************************/
uint16_t ch395CmdGetSocketStatus(uint8_t sockindex)
{
    uint16_t status;
    ch395WriteCommand(CMD12_GET_SOCKET_STATUS_SN);
    ch395WriteData(sockindex);
    status = ch395ReadData() << 8;
    status |= ch395ReadData();
    ch395EndCommand();
    return status;
}

/*******************************************************************************
 * Function Name  : ch395CmdGetSocketInterrupt
 * Description    : Gets the interrupt status of socket n
 * Input          : sockindex
 * Output         : None
 * Return         : socket interrupt status
 *******************************************************************************/
uint8_t ch395CmdGetSocketInterrupt(uint8_t sockindex)
{
    uint8_t intstatus;
    ch395WriteCommand(CMD11_GET_INT_STATUS_SN);
    ch395WriteData(sockindex);
    /*In between sending and receiving bytes, a TSC time delay is required.*/
    Delay_us(1);
    intstatus = ch395ReadData();
    ch395EndCommand();
    return intstatus;
}

/*******************************************************************************
 * Function Name  : ch395CmdSetSocketDestinationIp
 * Description    : Set the destination IP address of socket n
 * Input          : sockindex     - socket index
                    ipaddr        - destination IP address
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395CmdSetSocketDestinationIp(uint8_t sockindex, uint8_t *ipaddr)
{
    ch395WriteCommand(CMD50_SET_IP_ADDR_SN);
    ch395WriteData(sockindex);
    ch395WriteData(*ipaddr++);
    ch395WriteData(*ipaddr++);
    ch395WriteData(*ipaddr++);
    ch395WriteData(*ipaddr++);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetSocketDestinationPort
 * Description    : Set the destination port of socket n
 * Input          : sockindex   - socket index
                    desport     - destination port
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395CmdSetSocketDestinationPort(uint8_t sockindex, uint16_t desport)
{
    ch395WriteCommand(CMD30_SET_DES_PORT_SN);
    ch395WriteData(sockindex);
    ch395WriteData((uint8_t)desport);
    ch395WriteData((uint8_t)(desport >> 8));
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetSocketSourcePort
 * Description    : Set the source port of socket n
 * Input          : sockindex     - socket index
                    sorport       - source port
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395CmdSetSocketSourcePort(uint8_t sockindex, uint16_t sorport)
{
    ch395WriteCommand(CMD30_SET_SOUR_PORT_SN);
    ch395WriteData(sockindex);
    ch395WriteData((uint8_t)sorport);
    ch395WriteData((uint8_t)(sorport >> 8));
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetSocketProtocolType
 * Description    : Set the protocol type of socket n
 * Input          : sockindex   - socket index
                    prottype    - protocol type
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395CmdSetSocketProtocolType(uint8_t sockindex, uint8_t prottype)
{
    ch395WriteCommand(CMD20_SET_PROTO_TYPE_SN);
    ch395WriteData(sockindex);
    ch395WriteData(prottype);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdOpenSocket
 * Description    : Open socket n
 * Input          : sockindex
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t ch395CmdOpenSocket(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    ch395WriteCommand(CMD1W_OPEN_SOCKET_SN);
    ch395WriteData(sockindex);
    ch395EndCommand();
    while (1)
    {
        Delay_ms(20);               /* Delay query, more than 20MS is recommended*/
        s = ch395CmdGetCommandStatus(); /* Do not query too frequently*/
        if (s != CH395_ERR_BUSY)
            break;
        if (i++ > 200)
            return CH395_ERR_UNKNOW;
    }
    return s;
}

/******************************************************************************
 * Function Name  : ch395CmdTcpListen
 * Description    : socket n listens, After receiving this command, socket n enters server mode, valid only for TCP mode
 * Input          : sockindex
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t ch395CmdTcpListen(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    ch395WriteCommand(CMD1W_TCP_LISTEN_SN);
    ch395WriteData(sockindex);
    ch395EndCommand();
    while (1)
    {
        Delay_ms(20);               /* Delay query, more than 20MS is recommended*/
        s = ch395CmdGetCommandStatus(); /* Do not query too frequently*/
        if (s != CH395_ERR_BUSY)
            break;
        if (i++ > 200)
            return CH395_ERR_UNKNOW;
    }
    return s;
}

/********************************************************************************
 * Function Name  : ch395CmdTcpConnect
 * Description    : socket n connection. After receiving this command, socket n enters the client mode, valid only for TCP mode
 * Input          : sockindex
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t ch395CmdTcpConnect(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    ch395WriteCommand(CMD1W_TCP_CONNECT_SN);
    ch395WriteData(sockindex);
    ch395EndCommand();
    while (1)
    {
        Delay_ms(20);               /* Delay query, more than 20MS is recommended*/
        s = ch395CmdGetCommandStatus(); /* Do not query too frequently*/
        if (s != CH395_ERR_BUSY)
            break;
        if (i++ > 200)
            return CH395_ERR_UNKNOW;
    }
    return s;
}

/********************************************************************************
 * Function Name  : ch395CmdTcpDisconnect
 * Description    : socket n disconnection
 * Input          : sockindex
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t ch395CmdTcpDisconnect(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    ch395WriteCommand(CMD1W_TCP_DISCONNECT_SN);
    ch395WriteData(sockindex);
    ch395EndCommand();
    while (1)
    {
        Delay_ms(20);               /* Delay query, more than 20MS is recommended*/
        s = ch395CmdGetCommandStatus(); /* Do not query too frequently*/
        if (s != CH395_ERR_BUSY)
            break;
        if (i++ > 200)
            return CH395_ERR_UNKNOW;
    }
    return s;
}

/********************************************************************************
 * Function Name  : ch395CmdSendData
 * Description    : Writes data to socket n buffer
 * Input          : sockindex    - socket index
                    data buf     - data buf
                    len          - data length
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395CmdSendData(uint8_t sockindex, uint8_t *databuf, uint16_t len)
{
#if (CH395_SPI_DMA_ENABLE == 0)
    uint16_t i;
    ch395WriteCommand(CMD30_WRITE_SEND_BUF_SN);
    ch395WriteData((uint8_t)sockindex);
    ch395WriteData((uint8_t)len);
    ch395WriteData((uint8_t)(len >> 8));
    for (i = 0; i < len; i++)
    {
        ch395WriteData(*databuf++);
    }
    ch395EndCommand();
#else
    if (!len)
        return;
    ch395WriteCommand(CMD30_WRITE_SEND_BUF_SN);
    ch395WriteData((uint8_t)sockindex);
    ch395WriteData((uint8_t)len);
    ch395WriteData((uint8_t)(len >> 8));

    Delay_us(1);

    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
    DMA_Tx_Init(DMA1_Channel3, (uint32_t)&SPI1->DR, (uint32_t)databuf, len);
    DMA_Rx_Init(DMA1_Channel2, (uint32_t)&SPI1->DR, (uint32_t)databuf, len);
    DMA_Cmd(DMA1_Channel2, ENABLE);
    DMA_Cmd(DMA1_Channel3, ENABLE);
    while (!DMA_GetFlagStatus(DMA1_FLAG_TC3) || !DMA_GetFlagStatus(DMA1_FLAG_TC2))
    {
        if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == RESET)
            break;
    }
    DMA_ClearFlag(DMA1_FLAG_TC3);
    DMA_ClearFlag(DMA1_FLAG_TC2);

    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA_Cmd(DMA1_Channel3, DISABLE);
    ch395EndCommand();
#endif
}

/*******************************************************************************
 * Function Name  : ch395CmdGetReceiveLength
 * Description    : Gets the length of data received by socket n
 * Input          : socket index
 * Output         : None
 * Return         : 2 bytes receive length
 *******************************************************************************/
uint16_t ch395CmdGetReceiveLength(uint8_t sockindex)
{
    uint16_t i;

    ch395WriteCommand(CMD12_GET_RECV_LEN_SN);
    ch395WriteData((uint8_t)sockindex);
    /*In between sending and receiving bytes, a TSC time delay is required.*/
    Delay_us(1);
    i = ch395ReadData();
    i = (uint16_t)(ch395ReadData() << 8) + i;
    ch395EndCommand();
    return i;
}

/********************************************************************************
 * Function Name  : ch395CmdGetReceiveData
 * Description    : Gets socket n receive buffer data
 * Input          : sockindex    - socket index
                    len          - data length
                    pbuf         - data buf
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395CmdGetReceiveData(uint8_t sockindex, uint16_t len, uint8_t *pbuf)
{
#if (CH395_SPI_DMA_ENABLE == 0)
    uint16_t i;
    if (!len)
        return;
    ch395WriteCommand(CMD30_READ_RECV_BUF_SN);
    ch395WriteData(sockindex);
    ch395WriteData((uint8_t)len);
    ch395WriteData((uint8_t)(len >> 8));
    for (i = 0; i < len; i++)
    {
        *pbuf = ch395ReadData();
        pbuf++;
    }
    ch395EndCommand();

#else
    if (!len)
        return;
    ch395WriteCommand(CMD30_READ_RECV_BUF_SN);
    ch395WriteData(sockindex);
    ch395WriteData((uint8_t)len);
    ch395WriteData((uint8_t)(len >> 8));

    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
    DMA_Tx_Init(DMA1_Channel3, (uint32_t)&SPI1->DR, (uint32_t)pbuf, len);
    DMA_Rx_Init(DMA1_Channel2, (uint32_t)&SPI1->DR, (uint32_t)pbuf, len);
    DMA_Cmd(DMA1_Channel2, ENABLE);
    DMA_Cmd(DMA1_Channel3, ENABLE);
    while (!DMA_GetFlagStatus(DMA1_FLAG_TC3) || !DMA_GetFlagStatus(DMA1_FLAG_TC2))
    {
        if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == RESET)
            break;
    }
    DMA_ClearFlag(DMA1_FLAG_TC3);
    DMA_ClearFlag(DMA1_FLAG_TC2);

    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA_Cmd(DMA1_Channel3, DISABLE);
    ch395EndCommand();
#endif
}

/*******************************************************************************
 * Function Name  : ch395CmdCloseSocket
 * Description    : Close socket n
 * Input          : sockindex
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t ch395CmdCloseSocket(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    ch395WriteCommand(CMD1W_CLOSE_SOCKET_SN);
    ch395WriteData(sockindex);
    ch395EndCommand();
    while (1)
    {
        Delay_ms(20);               /* Delay query, more than 20MS is recommended*/
        s = ch395CmdGetCommandStatus(); /* Do not query too frequently*/
        if (s != CH395_ERR_BUSY)
            break;
        if (i++ > 200)
            return CH395_ERR_UNKNOW;
    }
    return s;
}

/******************************************************************************
 * Function Name  : ch395CmdSetSocketIpRawProtocol
 * Description    : In IP mode, configure the IP packet protocol field.
 * Input          : sockindex     - SocketIndex
                    prototype     - 1 byte protocol field
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395CmdSetSocketIpRawProtocol(uint8_t sockindex, uint8_t prototype)
{
    ch395WriteCommand(CMD20_SET_IPRAW_PRO_SN);
    ch395WriteData(sockindex);
    ch395WriteData(prototype);
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdEnablePing
 * Description    : On/off PING
 * Input          : 1 Enable PING ; 0  Disable PING
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdEnablePing(uint8_t enable)
{
    ch395WriteCommand(CMD10_PING_ENABLE);
    ch395WriteData(enable);
    ch395EndCommand();
}

/********************************************************************************
 * Function Name  : ch395CmdGetMacAddress
 * Description    : Gets the MAC address
 * Input          : MAC address pointer
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdGetMacAddress(uint8_t *macaddr)
{
    uint8_t i;
    ch395WriteCommand(CMD06_GET_MAC_ADDR);
    for (i = 0; i < 6; i++)
        *macaddr++ = ch395ReadData();
    ch395EndCommand();
}

/******************************************************************************
 * Function Name  : ch395CmdEnableDhcp
 * Description    : Enable DHCP
 * Input          : flag : 1 enable DHCP, 0 disable DHCP
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t ch395CmdEnableDhcp(uint8_t flag)
{
    uint8_t i = 0;
    uint8_t s;
    ch395WriteCommand(CMD10_DHCP_ENABLE);
    ch395WriteData(flag);
    ch395EndCommand();
    while (1)
    {
        Delay_ms(20);               /* Delay query, more than 20MS is recommended*/
        s = ch395CmdGetCommandStatus(); /* Do not query too frequently*/
        if (s != CH395_ERR_BUSY)
            break;
        if (i++ > 200)
            return CH395_ERR_UNKNOW;
    }
    return s;
}

/******************************************************************************
 * Function Name  : ch395CmdGetDhcpStatus
 * Description    : Obtain DHCP status
 * Input          : None
 * Output         : None
 * Return         : 1 byte status code, 0 indicates success, other values fail
 *******************************************************************************/
uint8_t ch395CmdGetDhcpStatus(void)
{
    uint8_t status;
    ch395WriteCommand(CMD01_GET_DHCP_STATUS);
    status = ch395ReadData();
    ch395EndCommand();
    return status;
}

/*******************************************************************************
 * Function Name  : ch395CmdGetIpInfo
 * Description    : Get IP, subnet mask, gateway
 * Input          : None
 * Output         : 20 bytes, respectively 4 bytes IP, 4 bytes gateway, 4 bytes mask, 4 bytes DNS1, 4 bytes DNS2
 * Return         : None
 *******************************************************************************/
void ch395CmdGetIpInfo(uint8_t *addr)
{
    uint8_t i;
    ch395WriteCommand(CMD014_GET_IP_INF);
    for (i = 0; i < 20; i++)
    {
        *addr++ = ch395ReadData();
    }
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetArp
 * Description    : Set ARP retransmission period and number of times
 *  Input         : period   - 1 byte ARP retransmission period
                    cnt      - 1 byte ARP retransmission number
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetArp(uint8_t period, uint8_t cnt)
{
    ch395WriteCommand(CMD20_SET_ARP);
    ch395WriteData(period);
    ch395WriteData(cnt);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetTcpMss
 * Description    : Set TCP MSS
 * Input          : mss
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetTcpMss(uint16_t mss)
{
    ch395WriteCommand(CMD20_TCP_MSS);
    ch395WriteData((uint8_t)mss);
    ch395WriteData((uint8_t)(mss >> 8));
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetTtl
 * Description    : Set the TTL
 * Input          : sockindex   - SocketIndex
 *                  TTLnum      - 1 byte TTL
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetTtl(uint8_t sockindex, uint8_t TTLnum)
{
    ch395WriteCommand(CMD20_SET_TTL);
    ch395WriteData(sockindex);
    ch395WriteData(TTLnum);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetSocketReceiveBuffer
 * Description    : Sets the SOCKET receive buffer
 * Input          : sockindex    - sockindex
                    startblk     - starting block index
                    blknum       - the number of blocks
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395CmdSetSocketReceiveBuffer(uint8_t sockindex, uint8_t startblk, uint8_t blknum)
{
    ch395WriteCommand(CMD30_SET_RECV_BUF);
    ch395WriteData(sockindex);
    ch395WriteData(startblk);
    ch395WriteData(blknum);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetSocketSendBuffer
 * Description    : Sets the SOCKET send buffer
 * Input          : sockindex    - sockindex
                    startblk     - starting block index
                    blknum       - the number of blocks
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395CmdSetSocketSendBuffer(uint8_t sockindex, uint8_t startblk, uint8_t blknum)
{
    ch395WriteCommand(CMD30_SET_SEND_BUF);
    ch395WriteData(sockindex);
    ch395WriteData(startblk);
    ch395WriteData(blknum);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetFunctionParameters
 * Description    : Sets function parameter
 * Input          : Four-byte function parameter
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetFunctionParameters(uint8_t PARA1, uint8_t PARA2, uint8_t PARA3, uint8_t PARA4)
{
    ch395WriteCommand(CMD40_SET_FUN_PARA);
    ch395WriteData(PARA1);
    ch395WriteData(PARA2);
    ch395WriteData(PARA3);
    ch395WriteData(PARA4);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetKeepAliveIdle
 * Description    : Set KEEPLIVE idle time
 * Input          : idle time (ms)
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetKeepAliveIdle(uint32_t idle)
{
    ch395WriteCommand(CMD40_SET_KEEP_LIVE_IDLE);
    ch395WriteData((uint8_t)idle);
    ch395WriteData((uint8_t)((uint16_t)idle >> 8));
    ch395WriteData((uint8_t)(idle >> 16));
    ch395WriteData((uint8_t)(idle >> 24));
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetKeepAliveInterval
 * Description    : Set KEEPLIVE interval time
 * Input          : timeout interval (ms)
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetKeepAliveInterval(uint32_t intvl)
{
    ch395WriteCommand(CMD40_SET_KEEP_LIVE_INTVL);
    ch395WriteData((uint8_t)intvl);
    ch395WriteData((uint8_t)((uint16_t)intvl >> 8));
    ch395WriteData((uint8_t)(intvl >> 16));
    ch395WriteData((uint8_t)(intvl >> 24));
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetKeepAliveCount
 * Description    : Set KEEPLIVE retries times
 * Input          : retry times
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetKeepAliveCount(uint8_t cnt)
{
    ch395WriteCommand(CMD10_SET_KEEP_LIVE_CNT);
    ch395WriteData(cnt);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetKeepAlive
 * Description    : Set the socket n keeplive function
 * Input          : sockindex   - sockindex
 *                  cmd         - 0: close 1:open
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetKeepAlive(uint8_t sockindex, uint8_t cmd)
{
    ch395WriteCommand(CMD20_SET_KEEP_LIVE_SN);
    ch395WriteData(sockindex);
    ch395WriteData(cmd);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdEepromErase
 * Description    : EEPROM erase
 * Input          : None
 * Output         : None
 * Return         : executing state
 *******************************************************************************/
uint8_t ch395CmdEepromErase(void)
{
    uint8_t i = 0;
    uint8_t s;
    ch395WriteCommand(CMD0W_EEPROM_ERASE);
    ch395EndCommand();
    while (1)
    {
        Delay_ms(20);               /* Delay query, more than 20MS is recommended*/
        s = ch395CmdGetCommandStatus(); /* Do not query too frequently*/
        if (s != CH395_ERR_BUSY)
            break;
        if (i++ > 200)
            return CH395_ERR_UNKNOW;
    }
    return s;
}

/*******************************************************************************
 * Function Name  : ch395CmdEepromWrite
 * Description    : Write EEPROM
 * Input          : eepaddr   - EEPROM addr
 *                ：buf       - buffer address
 *                ：len       - len
 * Output         : None
 * Return         : executing state
 *******************************************************************************/
uint8_t ch395CmdEepromWrite(uint16_t eepaddr, uint8_t *buf, uint8_t len)
{
    uint8_t i = 0;
    uint8_t s;
    ch395WriteCommand(CMD30_EEPROM_WRITE);
    ch395WriteData((uint8_t)(eepaddr));
    ch395WriteData((uint8_t)(eepaddr >> 8));
    ch395WriteData(len);
    while (len--)
        ch395WriteData(*buf++);
    ch395EndCommand();
    while (1)
    {
        Delay_ms(20);               /* Delay query, more than 20MS is recommended*/
        s = ch395CmdGetCommandStatus(); /* Do not query too frequently*/
        if (s != CH395_ERR_BUSY)
            break;
        if (i++ > 200)
            return CH395_ERR_UNKNOW;
    }
    return s;
}

/*******************************************************************************
 * Function Name  : ch395CmdEepromRead
 * Description    : Read EEPROM
 * Input          : eepaddr   - EEPROM addr
 *                ：buf       - buffer address
 *                ：len       - len
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdEepromRead(uint16_t eepaddr, uint8_t *buf, uint8_t len)
{
    ch395WriteCommand(CMD30_EEPROM_READ);
    ch395WriteData((uint8_t)(eepaddr));
    ch395WriteData((uint8_t)(eepaddr >> 8));
    ch395WriteData(len);
    Delay_us(30);
    while (len--)
        *buf++ = ch395ReadData();
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdReadGpioRegister
 * Description    : Read GPIO register
 * Input          : register addr
 * Output         : None
 * Return         : register value
 *******************************************************************************/
uint8_t ch395CmdReadGpioRegister(uint8_t regadd)
{
    uint8_t i;
    ch395WriteCommand(CMD11_READ_GPIO_REG);
    ch395WriteData(regadd);
    i = ch395ReadData();
    ch395EndCommand();
    return i;
}

/*******************************************************************************
 * Function Name  : ch395CmdWriteGpioRegister
 * Description    : Write GPIO register
 * Input          : regadd    - register addr
 *                ：regval    - register value
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdWriteGpioRegister(uint8_t regadd, uint8_t regval)
{
    ch395WriteCommand(CMD20_WRITE_GPIO_REG);
    ch395WriteData(regadd);
    ch395WriteData(regval);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395SetUartBaudRate
 * Description    : Set 395 Uart BaudRate
 * Input          : BaudRate
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t ch395SetUartBaudRate(uint32_t baudrate)
{
    uint8_t s = CH395_ERR_UNKNOW;
#if (CH395_OP_INTERFACE_MODE == CH395_UART_MODE)
    ch395CmdSetUartBaudRate(baudrate); /* Set BaudRate */
    Delay_ms(1);
    Set_MCU_BaudRate(baudrate);
    s = ch395ReadData(); /* If the setting is successful CH395 return CMD_ERR_SUCCESS */
    if (s == CMD_ERR_SUCCESS)
        printf("Set Success\r\n");
#endif
    return s;
}

/*******************************************************************************
 * Function Name  : ch395UdpSendTo
 * Description    : UDP sends data to the specified IP address and port
 * Input          : buf     - Send data buffer
                    len     - Send data length
                    ip      - DES IP
                    port    - DES Port
                    sockeid - socket index
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395UdpSendTo(uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t port, uint8_t sockindex)
{
    ch395CmdSetSocketDestinationIp(sockindex, ip);
    ch395CmdSetSocketDestinationPort(sockindex, port);
    ch395CmdSendData(sockindex, buf, len);
}

/*******************************************************************************
 * Function Name  : ch395CmdEnableIpv6AutoLinkLocalAddress
 * Description    : Enable automatic generation of Link-Local Address
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdEnableIpv6AutoLinkLocalAddress(void)
{
    ch395WriteCommand(CMD00_IPV6_AUTO_LLA);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetIpv6LinkLocalAddress
 * Description    : Set IPv6 Link-Local Address
 * Input          : pointer to 16-byte Link-Local address
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetIpv6LinkLocalAddress(uint8_t *pLLA)
{
    uint8_t i;
    ch395WriteCommand(CMD100_SET_LLA);
    for (i = 0; i < 16; i++)
        ch395WriteData(pLLA[i]);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetIpv6GlobalAddress
 * Description    : Set IPv6 Global Unicast Address
 * Input          : pointer to 16-byte Global Unicast address
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetIpv6GlobalAddress(uint8_t *pGUA)
{
    uint8_t i;
    ch395WriteCommand(CMD100_SET_GUA);
    for (i = 0; i < 16; i++)
        ch395WriteData(pGUA[i]);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdSetSocketDestinationIpv6Address
 * Description    : Set destination IPv6 address for socket n
 * Input          : socket index, pointer to 16-byte IPv6 address
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ch395CmdSetSocketDestinationIpv6Address(uint8_t sockindex, uint8_t *pIPv6)
{
    uint8_t i;
    ch395WriteCommand(CMD110_SET_DES_IPV6_ADDR_SN);
    ch395WriteData(sockindex);
    for (i = 0; i < 16; i++)
        ch395WriteData(pIPv6[i]);
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdGetIpv6UnreachableInfo
 * Description    : Get IPv6 unreachable information
 * Input          : pointer to 20-byte buffer (store code+proto+port+IPv6)
 * Output         : 20 bytes data
 * Return         : None
 *******************************************************************************/
void ch395CmdGetIpv6UnreachableInfo(uint8_t *pBuf)
{
    uint8_t i;
    ch395WriteCommand(CMD014_GET_IPV6_UNREACH);
    for (i = 0; i < 20; i++)
        pBuf[i] = ch395ReadData();
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdGetRemoteIpv6
 * Description    : Get remote IPv6 address and port of socket n
 * Input          : socket index
 * Output         : pointer to 18-byte buffer (16-byte IPv6 + 2-byte port)
 * Return         : None
 *******************************************************************************/
void ch395CmdGetRemoteIpv6(uint8_t sockindex, uint8_t *pBuf)
{
    uint8_t i;
    ch395WriteCommand(CMD112_GET_REMOT_IPV6_SN);
    ch395WriteData(sockindex);
    for (i = 0; i < 18; i++)
        pBuf[i] = ch395ReadData();
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395CmdEnableDhcpv6
 * Description    : Enable or disable DHCPv6
 * Input          : 1 byte flag (1: enable, 0: disable)
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t ch395CmdEnableDhcpv6(uint8_t flag)
{
    uint8_t i = 0;
    uint8_t s;
    ch395WriteCommand(CMD10_DHCPV6_ENABLE);
    ch395WriteData(flag);
    ch395EndCommand();
    while (1)
    {
        Delay_ms(20);               /* Delay query, more than 20MS is recommended*/
        s = ch395CmdGetCommandStatus(); /* Do not query too frequently*/
        if (s != CH395_ERR_BUSY)
            break;
        if (i++ > 200)
            return CH395_ERR_UNKNOW;
    }
    return s;
}

/*******************************************************************************
 * Function Name  : ch395CmdGetDhcpv6Status
 * Description    : Get DHCPv6 status
 * Input          : None
 * Output         : None
 * Return         : 1 byte status
 *******************************************************************************/
uint8_t ch395CmdGetDhcpv6Status(void)
{
    uint8_t status;
    ch395WriteCommand(CMD01_GET_DHCPV6_STATUS);
    status = ch395ReadData();
    ch395EndCommand();
    return status;
}

/*******************************************************************************
 * Function Name  : ch395CmdGetIpv6Address
 * Description    : Get IPv6 Link-Local and Unicast addresses
 * Input          : pointer to 32-byte buffer (store LLA + GUA)
 * Output         : 32 bytes address data
 * Return         : None
 *******************************************************************************/
void ch395CmdGetIpv6Address(uint8_t *pBuf)
{
    uint8_t i;
    ch395WriteCommand(CMD020_GET_IPV6_ADDR);
    for (i = 0; i < 32; i++)
        pBuf[i] = ch395ReadData();
    ch395EndCommand();
}

/*******************************************************************************
 * Function Name  : ch395Udp6SendTo
 * Description    : UDP sends data to the specified IP address and port
 * Input          : buf     - Send data buffer
                    len     - Send data length
                    ip      - DES IP
                    port    - DES Port
                    sockeid - socket index
 * Output         : None
 * Return         : None
*******************************************************************************/
void ch395Udp6SendTo(uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t port, uint8_t sockindex)
{
    ch395CmdSetSocketDestinationIpv6Address(sockindex, ip);
    ch395CmdSetSocketDestinationPort(sockindex, port);
    ch395CmdSendData(sockindex, buf, len);
}
