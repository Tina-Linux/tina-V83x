#ifndef _M_CLOUD_H__
#define _M_CLOUD_H__
/*================================================================
*  Midea Cloud user interface.
*  Author: Liu YH
*  Date: 2018-09-01
================================================================*/

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "m_type.h"
//#include "maudio.h"


/*
开始云端连接前，初始化连接参数，调用一次
*/
int m_cloud_init(struct m_cloud_st *cloud);

/*
释放资源，退出程序
*/
int m_cloud_release(void);

/*
更新会话ID，用于会话管理
*/
void m_create_session(void);

/*
发送音频数据前，需发送确认一条信息
*/
int m_cloud_begin(void);

/*
发送同一轮对话的重传音频数据前，需发送确认一条信息
*/
int m_cloud_retrans_begin(void);

/*
发送音频数据，格式为pcm，16000Hz,16bit，mono， 长度<=3200字节(即100ms音频数据)
*/
int m_cloud_send(const char *data, size_t length);


/*
发送json数据
*/
int m_cloud_trans(M_CLOUD_TOPIC_E event, const char *data, size_t len);


/*
结束音频数据发送，需发送结束信息
*/
int m_cloud_end(void);

#endif
