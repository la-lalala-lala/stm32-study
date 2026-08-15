#include "socket.h"
#include "ch395.h"
#include "delay/delay.h"

/*
 * 读取一次 CH395 全局中断，并把指定 Socket 的中断状态保存到调用方。
 * 状态缓存由 main 持有，因此 socket.c 不需要全局变量。
 */
static void socket_update_interrupt(uint8_t socket_index,uint8_t *socket_interrupt_status){
    uint16_t global_status;

    /* INT# 为低电平有效，高电平时不读取中断状态命令。 */
    if (ch395QueryInterrupt() != 0U)
        return;

    global_status = ch395CmdGetGlobalInterruptStatus();
    if ((global_status & (uint16_t)(GINT_STAT_SOCK0 << socket_index)) != 0U){
        *socket_interrupt_status |= ch395CmdGetSocketInterrupt(socket_index);
    }
}

/*
 * 超长报文仍然要从 CH395 接收缓冲区读完，否则下一次报文会被堵在芯片内。
 * 使用局部缓存，不增加 socket.c 的全局存储。
 */
static void socket_discard_receive(uint8_t socket_index,uint16_t length){
    uint8_t discard_buffer[32];

    while (length != 0U){
        uint16_t chunk = (length > (uint16_t)sizeof(discard_buffer)) ? (uint16_t)sizeof(discard_buffer) : length;

        ch395CmdGetReceiveData(socket_index, chunk, discard_buffer);
        length = (uint16_t)(length - chunk);
    }
}

int socket_udp_start(uint8_t socket_index,const uint8_t remote_ip[4],uint16_t remote_port,uint16_t local_port,uint8_t *socket_interrupt_status){
    uint8_t status;

    if (socket_index > 7U || remote_ip == NULL || remote_port == 0U || local_port == 0U || socket_interrupt_status == NULL){
        return -1;
    }

    /* 按网络开发指南和官方 EXAM0 的顺序配置 UDP Client。 */
    ch395CmdSetSocketDestinationIp(socket_index, (uint8_t *)remote_ip);
    ch395CmdSetSocketProtocolType(socket_index, PROTO_TYPE_UDP);
    ch395CmdSetSocketDestinationPort(socket_index, remote_port);
    ch395CmdSetSocketSourcePort(socket_index, local_port);

    status = ch395CmdOpenSocket(socket_index);
    if (status != CMD_ERR_SUCCESS){
        return -(int)status;
    }

    /* 首次发送前没有 SENDBUF_FREE 中断，官方例程要求软件先置位。 */
    *socket_interrupt_status = SINT_STAT_SENDBUF_FREE;
    return 0;
}

int socket_udp_receive(uint8_t socket_index,uint8_t *buffer,uint16_t buffer_size,uint8_t *socket_interrupt_status){
    uint16_t receive_length;

    if (socket_index > 7U || buffer == NULL || buffer_size == 0U || socket_interrupt_status == NULL){
        return -1;
    }

    /* 先读取并缓存 CH395 产生的 Socket 中断。 */
    if ((*socket_interrupt_status & SINT_STAT_RECV) == 0U){
        socket_update_interrupt(socket_index, socket_interrupt_status);
    }

    if ((*socket_interrupt_status & SINT_STAT_RECV) == 0U){
        return 0;
    }

    receive_length = ch395CmdGetReceiveLength(socket_index);
    if (receive_length == 0U){
        return 0;
    }

    /* 当前报文已经开始处理，先清除软件中的 RECV 标志。 */
    *socket_interrupt_status &= (uint8_t)~SINT_STAT_RECV;

    if (receive_length > buffer_size){
        socket_discard_receive(socket_index, receive_length);
        return -2;
    }

    /* UDP Client 接收数据为原始 payload，不包含 UDP Server 信息头。 */
    ch395CmdGetReceiveData(socket_index, receive_length, buffer);
    return (int)receive_length;
}

int socket_udp_send(uint8_t socket_index,const uint8_t *buffer,uint16_t length,uint8_t *socket_interrupt_status){
    if (socket_index > 7U || buffer == NULL || length == 0U || socket_interrupt_status == NULL){
        return -1;
    }

    /* 每次写发送缓冲前，都必须等待上一次发送释放缓冲区。 */
    while ((*socket_interrupt_status & SINT_STAT_SENDBUF_FREE) == 0U){
        socket_update_interrupt(socket_index, socket_interrupt_status);
        if ((*socket_interrupt_status & SINT_STAT_SENDBUF_FREE) == 0U){
            Delay_ms(1);
        }
    }

    *socket_interrupt_status &= (uint8_t)~SINT_STAT_SENDBUF_FREE;

    /* UDP Client 的目标端点已在 socket_udp_start 中配置，直接写发送缓冲。 */
    ch395CmdSendData(socket_index, (uint8_t *)buffer, length);
    return 0;
}

/*
 * 连接断开或连接超时后清理软件状态，并把错误交给 main 处理。
 * 返回值：-2 表示断开，-3 表示超时，0 表示没有连接错误。
 */
static int socket_tcp_client_connection_error(uint8_t *socket_interrupt_status){
    if ((*socket_interrupt_status & SINT_STAT_DISCONNECT) != 0U){
        *socket_interrupt_status &= (uint8_t)~(
            SINT_STAT_DISCONNECT | SINT_STAT_CONNECT);
        return -2;
    }

    if ((*socket_interrupt_status & SINT_STAT_TIM_OUT) != 0U){
        *socket_interrupt_status &= (uint8_t)~(
            SINT_STAT_TIM_OUT | SINT_STAT_CONNECT);
        return -3;
    }

    return 0;
}

int socket_tcp_client_start(uint8_t socket_index,const uint8_t remote_ip[4],uint16_t remote_port,uint16_t local_port,uint8_t *socket_interrupt_status){
    uint8_t status;

    if (socket_index > 7U || remote_ip == NULL || remote_port == 0U || local_port == 0U || socket_interrupt_status == NULL){
        return -1;
    }

    /* TCP Client 的 Socket 参数配置顺序与网络开发指南一致。 */
    ch395CmdSetSocketDestinationIp(socket_index, (uint8_t *)remote_ip);
    ch395CmdSetSocketProtocolType(socket_index, PROTO_TYPE_TCP);
    ch395CmdSetSocketDestinationPort(socket_index, remote_port);
    ch395CmdSetSocketSourcePort(socket_index, local_port);

    /* 先打开 Socket，再执行 TCP Client 连接命令。 */
    status = ch395CmdOpenSocket(socket_index);
    if (status != CMD_ERR_SUCCESS){
        return -(int)status;
    }

    status = ch395CmdTcpConnect(socket_index);
    if (status != CMD_ERR_SUCCESS){
        (void)ch395CmdCloseSocket(socket_index);
        return -(int)status;
    }

    /* 首次连接前没有发送缓冲空闲中断，按官方例程预置该状态。 */
    *socket_interrupt_status = SINT_STAT_SENDBUF_FREE;
    return 0;
}

int socket_tcp_client_receive(uint8_t socket_index,uint8_t *buffer,uint16_t buffer_size,uint8_t *socket_interrupt_status){
    uint16_t receive_length;
    int connection_error;

    if (socket_index > 7U || buffer == NULL || buffer_size == 0U || socket_interrupt_status == NULL){
        return -1;
    }

    socket_update_interrupt(socket_index, socket_interrupt_status);
    connection_error = socket_tcp_client_connection_error(socket_interrupt_status);
    if (connection_error != 0){
        return connection_error;
    }

    if ((*socket_interrupt_status & SINT_STAT_RECV) == 0U){
        return 0;
    }

    receive_length = ch395CmdGetReceiveLength(socket_index);
    if (receive_length == 0U){
        return 0;
    }

    *socket_interrupt_status &= (uint8_t)~SINT_STAT_RECV;
    if (receive_length > buffer_size){
        socket_discard_receive(socket_index, receive_length);
        return -4;
    }

    /* TCP 接收缓冲区直接保存有效数据，不需要 UDP 信息头解析。 */
    ch395CmdGetReceiveData(socket_index, receive_length, buffer);
    return (int)receive_length;
}

int socket_tcp_client_send(uint8_t socket_index,const uint8_t *buffer,uint16_t length,uint8_t *socket_interrupt_status){
    int connection_error;

    if (socket_index > 7U || buffer == NULL || length == 0U || socket_interrupt_status == NULL){
        return -1;
    }

    /* TCP 必须先收到 CONNECT 中断，不能像 UDP 一样直接发送。 */
    while ((*socket_interrupt_status & SINT_STAT_CONNECT) == 0U){
        socket_update_interrupt(socket_index, socket_interrupt_status);
        connection_error = socket_tcp_client_connection_error(socket_interrupt_status);
        if (connection_error != 0){
            return connection_error;
        }
        Delay_ms(1);
    }

    /* 等待发送缓冲区空闲，再写入当前 TCP 数据段。 */
    while ((*socket_interrupt_status & SINT_STAT_SENDBUF_FREE) == 0U){
        socket_update_interrupt(socket_index, socket_interrupt_status);
        connection_error = socket_tcp_client_connection_error(socket_interrupt_status);
        if (connection_error != 0){
            return connection_error;
        }
        Delay_ms(1);
    }

    *socket_interrupt_status &= (uint8_t)~SINT_STAT_SENDBUF_FREE;
    ch395CmdSendData(socket_index, (uint8_t *)buffer, length);
    return 0;
}
