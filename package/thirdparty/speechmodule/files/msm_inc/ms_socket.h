/*
 *
 * File Name    : ms_osal_FreeRTOS.h
 * Introduction	:
 *
 * Current Version	: 2.0
 * Author			: Humble(yunyun1.wu@midea.com)
 * Create Time	    : Apr 11, 2017
 * Change Log	    :
 *
 * All software, firmware and related documentation here in ("M-Smart Software") are
 * intellectual property of M-SMART Research Institute of Midea Group and protected
 * by law, including, but not limited to, copyright law and international treaties.
 *
 * Any use, modification, reproduction, retransmission, or republication of all
 * or part of M-Smart Software is expressly prohibited, unless prior written
 * permission has been granted by M-Smart.
 *
 */

#if !defined(MS_SOCKET_H)
#define MS_SOCKET_H

#include <ms_common.h>

#define MS_ASSERT(x) do {if(!(x)) while(1);} while(0)								//!< x为0 则进入死循环

/**
 * \brief struct ms_socket_info_t \n
 * 存储socket信息
 */
typedef struct {
	int socket_fd;																		//!< socket的描述符.
	uint32_t last_hb;//tcp-client used: last local heartbeat							//!< tcp连接的最后一次心跳.
	//struct sockaddr_in socket_addr;//tcp-client used
	char	tcp_client_ip_str[16];														//!< tcp连接的的client、的ip地址.
}ms_socket_info_t;

/**
 * \brief struct ms_local_socket_set_t \n
 * 存储局域网socket信息
 */
typedef struct {
	ms_socket_info_t socket_info[MS_TOTAL_SOCKET_NUM];									//!< @see ms_socket_info_t.
}ms_local_socket_set_t;

/**
 * \brief struct T_MSNetAppOps \n
 * socket接口
 */
typedef struct T_MSNetAppOps
{
	ms_socket_info_t *p_net_socket_set;																										//!< 广域网socket描述符集.
	ms_local_socket_set_t *p_local_socket_set;																								//!< 局域网socket描述符集.
	int (*ms_socket_open)(int *socket_fd, int port,char* addr, int *p_server_ip);															//!< 创建并连接msmart云端socket函数指针.
	int (*ms_socket_close)(int fd);																											//!< 关闭并断开msmart云端socket函数指针.
	int (*ms_net_read) (int fd,unsigned char* buf, int length,int timeout);																	//!< 读指针.
	int (*ms_net_write) (int fd,unsigned char* buf, int length,int timeout);																//!< 写指针.
	int (*ms_net_open) (struct T_MSNetAppOps*,int port,char* addr, int *p_server_ip);														//!< 创建并连接msmart云端socket函数指针.
	void (*ms_net_close)(struct T_MSNetAppOps*);																							//!< 关闭并断开msmart云端socket函数指针.
	int (*ms_local_socket_open)(struct T_MSNetAppOps *ops);																					//!< 创建局域网TCP ，UDP socket.
	int (*ms_local_socket_close)(struct T_MSNetAppOps *ops);																				//!< 关闭局域网所有socket.
	int (*ms_local_socket_accept)(struct T_MSNetAppOps *ops, uint32_t tm_ms);																//!< 模块TCP server 监听client的连接请求.
	int (*ms_local_udp_readfrom)(int fd, unsigned char* buf, unsigned short len,char* rip, unsigned short riplen, unsigned short* rport);	//!< 局域网socket的读.
	int (*ms_local_udp_writeto)(int fd, const unsigned char* buf, unsigned short len,const char* rip, unsigned short rport);				//!< 局域网socket的写操作.
} T_MSNetAppOps;



/**
 * \brief      网络操作接口初始化
 * \param handle  网络对象指针
 * \return
 */
void ms_NetworkInit(ST_HANDLE *handle);

#endif
