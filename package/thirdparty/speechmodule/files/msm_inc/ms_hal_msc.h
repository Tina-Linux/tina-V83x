/*
 * Copyright (c) 2015 - 2018 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name		: ms_hal_msc.c
 * Introduction	: msc2.0 operation
 *
 * Author			: Weishan Lu
 * Create Time	: 2017/10/13
 *
 * All software, firmware and related documentation herein ("M-Smart Software") are
 * intellectual property of M-SMART Research Institute of Midea Group and protected
 * by law, including, but not limited to, copyright law and international treaties.
 *
 * Any use, modification, reproduction, retransmission, or republication of all
 * or part of M-Smart Software is expressly prohibited, unless prior written
 * permission has been granted by M-Smart.
 *
 */

#ifndef __MS_HAL_MSC_H__
#define __MS_HAL_MSC_H__

/**
 * \defgroup ms_hal_msc msmart-sniffer通用部分
 * \brief msmart sdk sniffer 部分的接口，如sniffer部分数据单元初始化、解析广播/组播包的处理、获取解析的结果、资源的释放。
 * \par
 * 具体请参考: ms_hal_msc.h \n
 */

#include "msm_adapter.h"
/**
 * \brief enum md_smart_config \n
 * 美的快联的模式。
 */
typedef enum{
	MSC_RECE_OK= 0,												/*!< 默认状态. */
	MSC_CHANNEL_LOCK,											/*!< 美的快联锁定信道. */
	MSC_FIN,													/*!< 美的快联完成. */
	MSC_ERR,													/*!< 美的快联出错. */
}md_smart_config;

/**
 * \brief struct ms_msc_info_t \n
 * 美的快联的模式。
 */
typedef struct
{
	uint8_t pwd_len;											/*!< 美的快联获取的pwd是长度. */
	uint8_t pwd_data[65];										/*!< 美的快联获取的pwd. */
	uint8_t ssid_len;											/*!< 美的快联获取的ssid长度. */
	uint8_t ssid_data[33];										/*!< 美的快联获取的ssid. */
	uint8_t bssid[6];											/*!< 美的快联获取的bssid. */
	uint8_t random[16];										/*!< 获取的随机数 */
	uint8_t config_way;											/*!< 记录配网的方式. */

}ms_msc_info_t;



//intial MSC
/**
* @brief	ms_hal_msc_init(void)
*  init the msc.   When changing the channel, this function should be called.
* @param  null
*
* returun: NULL
**/
void ms_hal_msc_init(void);

//intial MSC
/**
* @brief	ms_hal_msc_handle(uint8_t *in_buff,int in_len)
* handle the receive packet
* @param: buff: the packet pointer
*			   len: the packet len
*
*return:
*                  MSC_RECE_OK:            handle OK \n
*                  MSC_CHANNEL_LOCK:   channel lock	\n
*                  MSC_FIN:			    MSC finish,  FUN: ms_hal_msc_get_result should be called to get the result \n
*		       MSC_ERROR:		    handle ERROR
**/

int ms_hal_msc_handle(uint8_t *in_buff,int in_len);


/**
* @brief	ms_hal_msc_get_result(ms_msc_info_t * result)
* get the msc result
* @Param: the pointer of the result
*
* return:
*		0: OK \n
*		!0: ERROR
**/
int ms_hal_msc_get_result(ms_msc_info_t * result);

/**
* @brief	ms_msc_free()
* free the memory
* @Parameter: NULL
*
* return: NULL
*
**/
void ms_msc_free();



#endif
