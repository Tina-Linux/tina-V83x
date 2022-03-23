/**
 * @file msm_adapter.h
 * @brief M-Smart Middleware(中间件)适配
 * @author 叶楚汉
 * @version 1.0.0
 * @par Copyright  (c)
 *		Midea
 */

#ifndef __MSM_ADAPTER_H__
#define __MSM_ADAPTER_H__
/* Includes ,We must include C standard headers*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/**
 * \defgroup msm_adapter M-Smart Middleware(中间件)适配部分
 * \brief M-Smart Middleware适配接口，如license读写、延时设置、互斥体设置、申请内存、网络字节与主机字节转换、socket操作、快连等。
 * \note 此适配层接口仅用于中间件内部调用，客户程序员若需用到延时、互斥体等相关接口，则请使用系统原生接口。 \n
 */

/** 宏定义：广播地址 */
#define MSM_IPADDR_BROADCAST		((unsigned int)0xffffffffUL)
/** 宏定义：监听广播地址  */
#define MSM_IPADDR_ANY			((unsigned int)0x00000000UL)
/** 宏定义：定义无符号char型  */
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

/**
 * @brief	适配接口结果枚举值
 */
typedef enum{
	MSM_RESULT_SUCCESS = 0, /*!< 接口调用成功*/
	MSM_RESULT_ERROR, /*!< 接口调用未成功 */
}msm_result_t;

/**
 * @brief msm_printf(const char* format, ...)
 * 格式化输出函数
 * @param[in]	format	格式控制信息
 * @param[in]	...	可选参数，可以是任何类型的数据
 * @return	小于0	输出失败 \n
 *		大于等于0	输出的长度 \n
 */
int msm_printf(const char* format, ...);

/**
 * @brief	msm_flash_read_license_operation_info(uint8_t *buf, unsigned int len)
 * 读取工作区license函数
 * @param[in/out]	buf	缓存数据区
 * @param[in]	len	读取长度
 * @return	MSM_RESULT_SUCCESS	成功  \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_flash_read_license_operation_info(uint8_t *buf, unsigned int len);
/**
 * @brief	msm_flash_save_license_operation_info(uint8_t *buf, unsigned int len)
 * 保存工作区license函数
 * @param[in]	buf	缓存数据区
 * @param[in]	len	保存长度
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_flash_save_license_operation_info(uint8_t *buf, unsigned int len);
/**
 * @brief	msm_flash_read_license_factory_info(uint8_t *buf, unsigned int len)
 * 读取生产区license函数
 * @param[in/out]	buf	缓存数据区
 * @param[in]	len	读取长度
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_flash_read_license_factory_info(uint8_t *buf, unsigned int len);

/**
 * @brief	msm_timer_delay_ms(int ms)
 * 延时函数
 * @param[in]	ms	设置的延时时间，单位为毫秒
 */
void msm_timer_delay_ms(int ms);
/**
 * @brief	msm_timer_get_systime_ms(void)
 * 处理器开机经过的时间，毫秒级
 * @return	返回时间值
 */
unsigned int msm_timer_get_systime_ms(void);//TimerGetSysTimeMs

/**
 * @brief	msm_timer_tickcount_get(void)
 * 处理器开机经过的tick数
 * @return	返回的tick数
 */
unsigned int msm_timer_tickcount_get(void);

/**
 * @brief	msm_sst_mutex_init(void)
 * 互斥体的初始化
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_sst_mutex_init(void);
/**
 * @brief	msm_sst_mutex_lock(void)
 * 获取互斥体
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_sst_mutex_lock(void);
/**
 * @brief	msm_sst_mutex_unlock(void)
 * 释放互斥体
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_sst_mutex_unlock(void);
/**
 * @brief	msm_sst_mutex_delete(void)
 * 删除互斥体
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_sst_mutex_delete(void);

/**
 * @brief	msm_memory_alloc(unsigned int size)
 * 申请动态内存
 * @param[in]	size	申请动态内存大小
 * @return	返回申请内存的首地址
 */
void *msm_memory_alloc(unsigned int size);
/**
 * @brief	msm_memory_calloc(size_t n, size_t size)
 * 申请动态内存，一般为数组形式的
 * @param[in]	n	申请内存元素的个数
 * @param[in]	size	每个元素大小
 * @return	返回申请内存的首地址
 */
void *msm_memory_calloc(size_t n, size_t size);

/**
 * @brief	msm_memory_free(void *pmem)
 * 释放动态内存
 * @param[in]	pmem	内存的首地址
 */
void msm_memory_free (void *pmem);
/**
 * @brief	msm_htons(uint16_t n)
 * 将主机的无符号短整形转换成网络字节顺序
 * @param[in]	n	主机的字节顺序的16位数
 * @return	返回网络字节序数值
 */
uint16_t msm_htons(uint16_t n);
/**
 * @brief	msm_ntohs(uint16_t n)
 * 将无符号整形从网络字节顺序转为主机字节顺序
 * @param[in]	n	网络字节的16位数
 * @return	返回主机字节序数值
 */
uint16_t msm_ntohs(uint16_t n);
/**
 * @brief	msm_inet_addr(const char *cp)
 * 将一个点分十进制的IP转换成长整形
 * @param[in]	cp	为"192.168.1.1"格式的点分十进制IP地址
 * @return	如果正确执行将返回一个无符号网络字节序长整型数
 */
uint32_t msm_inet_addr(const char *cp);
/**
 * @brief	msm_fd_set_init(void)
 * 初始化文件描述符集合
 */
void msm_fd_set_init(void);
/**
 * @brief	msm_fd_zero(void)
 * 清空文件描述符
 */
void	msm_fd_zero(void);
/**
 * @brief	msm_fd_set(int fd)
 * 在文件描述符集合中增加新的文件描述符
 */
void msm_fd_set(int fd);
/**
 * @brief	msm_fd_is_set(int fd)
 * 检测指定文件描述符是否在该集合中
 * @param[in]	fd 指定的文件描述符
 * @return	非0	fd为集合中的一员
 *		0	 fd非集合中的一员
 */
int msm_fd_is_set(int fd);
/**
 * @brief	msm_select(int maxfdp1, uint32_t timeout_ms)
 * 设置socket超时模式，超时不阻塞
 * @param[in]	maxfdp1	socket文件描述符+1
 * @param[in]	timeout_ms	设置超时时间，单位为毫秒
 * @return	0	表示超时，没有可读可写文件 \n
 *			-1	错误				 \n
 *			>0	某些文件可读可写
 */
int msm_select(int maxfdp1, uint32_t timeout_ms);

/**
 * @brief	msm_accept(int s, char* rip, unsigned short riplen)
 * 接收客户端的TCP连接请求
 * @param[in]	s	TCP server 监听文件描述符
 * @param[in]	rip	客户端的地址信息，采用点十法表示，如"192.168.1.2"
 * @param[in]	riplen	客户端地址信息长度，长度为16（含结束符）
 */
int msm_accept(int s, char* rip, unsigned short riplen);
/**
 * @brief	msm_socket_set_rcvtimeo(int s, unsigned int ms)
 * 设置socket接收数据超时时间
 * @param[in]	s	socket文件描述符
 * @param[in]	ms	设置的接收时限，单位为毫秒
 * @return	0	设置成功 \n
 *			-1	设置失败
 */
int msm_socket_set_rcvtimeo(int s, unsigned int ms);
/**
 *	@brief	msm_socket_tcp_server_init(int32_t ip, int16_t port, int ms)
 *	初始化TCP server socket
 *	@param[in]	ip	设置TCP的地址，0.0.0.0代表本机地址
 *	@param[in]	port	TCP server的端口号
 *	@param[in]	ms	用于设置socket接收时限，单位为毫秒
 *	@return	>0	TCP socket的文件描述符 \n
 *			-1	初始化失败
 */
int msm_socket_tcp_server_init(int32_t ip, int16_t port, int ms);

/**
 * @brief	msm_socket_tcp_read(int socket,unsigned char* buffer, int len,int timeout_ms)
 * 接收tcp socket数据
 * @param[in]	s	TCP连接文件描述符
 * @param[out]	buffer	接收数据缓存区
 * @param[in]	len	接收数据长度
 * @param[in]	timeout_ms	设置的接收时限
 * @return	>0	接收到的字节数 \n
 *			0	连接关闭 \n
 *			-1	接收失败
 */
int msm_socket_tcp_read(int socket,unsigned char* buffer, int len,int timeout_ms);
/**
 * @brief	msm_socket_tcp_write(int socket,unsigned char* buffer, int len,int timeout_ms)
 * 发送tcp socket数据
 * @param[in]	socket	TCP连接文件描述符
 * @param[in]	buffer	发送数据缓存区
 * @param[in]	len	发送数据长度
 * @param[in]	timeout_ms(unused)	设置的发送时限
 * @return	>0	写入的字节数 \n
 *			0	当前写缓冲区已满 \n
 *			-1	写失败
 */
int msm_socket_tcp_write(int socket,unsigned char* buffer, int len,int timeout_ms);
/**
 * @brief	msm_socket_close(int fd)
 * 关闭socket
 * @param[in]	fd 关闭的socket
 * @return	=0	成功 \n
 *			-1	失败
 */
int msm_socket_close(int fd);

/**
 *	@brief	msm_socket_udp_init(unsigned int ip, unsigned short port, int ms, bool broadcast_flag)
 *	初始化UDP socket
 *	@param[in]	ip	设置UDP的地址，广播为ffffffff，监听为00000000
 *	@param[in]	port	设置UDP的端口号
 *	@param[in]	ms	设置UDP的接收的时限，单位为毫秒
 *	@param[in]	broadcast_flag	设置UDP的模式，0表示监听模式，1表示广播模式
 *	@return	>=0	UDP文件描述符 \n
 *			-1	初始化失败
 */
int msm_socket_udp_init(unsigned int ip, unsigned short port, int ms, bool broadcast_flag);
/**
 * @brief	msm_socket_udp_readfrom(int fd, unsigned char* buf, unsigned short len, char* rip, unsigned short riplen, unsigned short* rport)
 * 接收UDP数据
 * @param[in]	fd	UDP文件描述符
 * @param[out]	buf	接收数据缓存区
 * @param[in]	len	接收数据长度
 * @param[in]	rip	对端IP，采用点十法表示，如"192.168.1.2"
 * @param[in]	riplen	IP长度
 * @param[in]	rport	对端端口
 * @return	非0	接收数据长度 \n
 *			-1	接收失败
 */
int msm_socket_udp_readfrom(int fd, unsigned char* buf, unsigned short len,
        char* rip, unsigned short riplen, unsigned short* rport);
/**
 * @brief	msm_socket_udp_writeto(int fd, const unsigned char* buf, unsigned short len, const char* rip, unsigned short rport);
 * 向UDP连接端发送的数据
 * @param[in]	fd	UDP文件描述符
 * @param[in]	buf	发送数据缓存区
 * @param[in]	len	发送数据长度
 * @param[in]	rip	对端IP，采用点十法表示，如"192.168.1.2"
 * @param[in]	riplen	IP长度
 * @param[in]	rport	对端端口
 * @return	非0	发送数据长度 \n
 *			-1	发送失败

 */
int msm_socket_udp_writeto(int fd, const unsigned char* buf, unsigned short len,
        const char* rip, unsigned short rport);

/**
 * @brief	msm_socket_open(int *socket_fd, int port,char* addr, int *p_server_ip)
 * 与服务器建立socket连接
 * @param[out]	socket_fd	返回的TCP socket文件描述符
 * @param[in]	port	服务器端口号
 * @param[in]	port	服务器域名
 * @param[out]	p_server_ip	服务器ip
 * @return	0	成功连接服务器 \n
 *			其它	无法连接服务器
 */
int msm_socket_open(int *socket_fd, int port,char* addr, int *p_server_ip);

/**
 * @brief	msm_sniffer_set_chn(int chn)
 * sniffer模式下设置侦听信道
 * @param[in]	chn	信道
 * @return	MSM_RESULT_SUCCESS	设置成功 \n
 *			MSM_RESULT_ERROR	设置失败
 */
msm_result_t msm_sniffer_set_chn(int chn);

//if platform not support , the return 0; else, return the fixed channel
/**
 * @brief	msm_sniffer_get_fixed_channel(void *fixed_bssid, unsigned char *ssid, int *ssid_length)
 * 根据bssid或ssid获取
 * @param[in]	fixed_bssid	bssid值
 * @param[out]	ssid	bssid所对应的ssid
 * @param[out]	ssid_length	ssid长度
 * @return	大于0	bssid对应热点的channel值 \n
 *			0	无法获取bssid对应热点的channel值
 */
int msm_sniffer_get_fixed_channel(void *fixed_bssid, unsigned char *ssid, int *ssid_length);

/**
* @brief	Get the module version(unsigned char *p_in, int len)
 * 获取集成M-Smart Middleware的设备版本号，长度固定为六个字节
* @parameter[in]	p_in	input data
* @parameter[in]	len	The lenght of input data
* @return	NULL
**/

void msm_get_module_version(unsigned char *p_in, int len);
#endif
