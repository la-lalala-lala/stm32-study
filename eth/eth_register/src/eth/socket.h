#ifndef __CH395_SOCKET_H__
#define __CH395_SOCKET_H__

#include <stdint.h>

/**
 * @brief 配置并打开一个 UDP Client Socket。
 * @param socket_index Socket 编号，范围为 0~7。
 * @param remote_ip UDP 服务端 IPv4 地址，按 4 个字节传入。
 * @param remote_port UDP 服务端端口。
 * @param local_port CH395 本地 UDP 源端口。
 * @param socket_interrupt_status 由调用方保存的 Socket 中断状态。
 * @return 0 表示成功，负数表示参数或 CH395 命令错误。
 */
int socket_udp_start(uint8_t socket_index,const uint8_t remote_ip[4],uint16_t remote_port,uint16_t local_port,uint8_t *socket_interrupt_status);

/**
 * @brief 轮询并读取一个 UDP Client 数据报。
 * @param socket_index Socket 编号，范围为 0~7。
 * @param buffer 用于保存 UDP payload 的接收缓冲区。
 * @param buffer_size 接收缓冲区容量，单位为字节。
 * @param socket_interrupt_status 由调用方保存的 Socket 中断状态。
 * @return 正数为收到的 payload 长度，0 表示当前无数据，负数表示错误。
 */
int socket_udp_receive(uint8_t socket_index,uint8_t *buffer,uint16_t buffer_size,uint8_t *socket_interrupt_status);

/**
 * @brief 向 UDP Client 的固定服务端发送一段数据。
 * @param socket_index Socket 编号，范围为 0~7。
 * @param buffer 待读取并发送的数据。
 * @param length 发送数据长度，单位为字节。
 * @param socket_interrupt_status 由调用方保存的 Socket 中断状态。
 * @return 0 表示已提交发送，负数表示参数错误。
 */
int socket_udp_send(uint8_t socket_index,const uint8_t *buffer,uint16_t length,uint8_t *socket_interrupt_status);

/**
 * @brief 配置、打开并连接一个 TCP Client Socket。
 * @param socket_index Socket 编号，范围为 0~7。
 * @param remote_ip TCP 服务端 IPv4 地址，按 4 个字节传入。
 * @param remote_port TCP 服务端端口。
 * @param local_port CH395 本地 TCP 源端口。
 * @param socket_interrupt_status 由调用方保存的 Socket 中断状态。
 * @return 0 表示连接成功，负数表示参数、打开或连接错误。
 */
int socket_tcp_client_start(uint8_t socket_index,const uint8_t remote_ip[4],uint16_t remote_port,uint16_t local_port,uint8_t *socket_interrupt_status);

/**
 * @brief 轮询并读取一个 TCP Client 数据段。
 * @param socket_index Socket 编号，范围为 0~7。
 * @param buffer 用于保存 TCP 数据的接收缓冲区。
 * @param buffer_size 接收缓冲区容量，单位为字节。
 * @param socket_interrupt_status 由调用方保存的 Socket 中断状态。
 * @return 正数为读取长度，0 表示当前无数据，负数表示断开、超时或读取错误。
 */
int socket_tcp_client_receive(uint8_t socket_index,uint8_t *buffer,uint16_t buffer_size,uint8_t *socket_interrupt_status);

/**
 * @brief 向已建立连接的 TCP 服务端发送数据。
 * @param socket_index Socket 编号，范围为 0~7。
 * @param buffer 待读取并发送的数据。
 * @param length 发送数据长度，单位为字节。
 * @param socket_interrupt_status 由调用方保存的 Socket 中断状态。
 * @return 0 表示已提交发送，负数表示参数、连接或发送错误。
 */
int socket_tcp_client_send(uint8_t socket_index,const uint8_t *buffer,uint16_t length,uint8_t *socket_interrupt_status);

#endif
