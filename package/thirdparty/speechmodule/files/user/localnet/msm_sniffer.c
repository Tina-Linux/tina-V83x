/*
 * Copyright (c) 2017 M-SMART of Midea Group.
 * All rights reserved.
 *
 * File Name		: sniffer task
 * Introduction	:
 *
 * Current Version	: v0.1
 * Author			: Johann <yech3@midea.com.cn>
 * Create Time	: 2018/04
 * Change Log		: create this file
 *
 * All software, firmware and related documentation here in ("M-Smart Software") are
 * intellectual property of M-SMART of Midea Group and protected
 * by law, including, but not limited to, copyright law and international treaties.
 *
 * Any use, modification, reproduction, retransmission, or republication of all
 * or part of M-Smart Software is expressly prohibited, unless prior written
 * permission has been granted by M-Smart.
 */
#include "ms_common.h"
#include "ms_net_local_app.h"

/******************** for msc **************************/
#include "ms_hal_msc.h"
/******************** for msc **************************/

/*********************Platform related******************************/
#include "wifi_config.h"
/*********************Platform related end**************************/
/** sniffer 信道锁定配网时间*/
#define		MS_SNIFFER_CYCLE_MS			(30 * 1000)
/** sniffer 切换信道的时间*/
#define MS_CHANGE_CHANNEL_TIME 150

/**
 * @brief struct msm_sniffer_info_t
 * sniffer 配网存储参数
 */
typedef struct MS_SNIFFER_INFO_T{
	uint8_t					    sniffer_done;								//!< sniffer完成状态位，1表示sniffer完成，0表示未完成。
	uint8_t				        lock_bssid[6];									//!< 锁定信道 路由的ssid。
	u32				            lock_chn_ms;								//!< 记录锁定信道时间，超时重新锁定。
	ms_enum_lock_chn_info_t     lock_chn_info;									//!< 锁定信道的方式。
	ms_stored_info_t		    ms_stored_info;								//!< 存储路由信息的结构体。
	uint8_t						sniffer_err_flag;							//!< 配网过程是否有err。
}msm_sniffer_info_t;

/** sniffer 线程句柄 */
//ms_hal_os_thread_t	msm_sniffer_thread = NULL;

/** 全局变量  监测sniffer的状态 */
msm_sniffer_info_t g_msm_sniffer_info;
/** 时钟比较函数*/
extern bool ms_sys_api_ticks_compare_result(u32 end_tick, u32 start_tick, u32 interval_tick);

/** 局域网函数 */
E_Err ms_test_event_cb(T_MsNetLocalAppFrame *data);
/**
 * \brief 获取快连配网的result
 *\param p_sniffer_info 指针变量，用于缓存路由信息
 */
void msm_get_result(msm_sniffer_info_t *p_sniffer_info)
{
	ms_msc_info_t ms_msc_info;

	memset(&ms_msc_info, 0, sizeof(ms_msc_info));
	// 美的msc获取结果的接口
	if(ms_hal_msc_get_result(&ms_msc_info))
	{
		MS_ERR_TRACE("get msc result error");
		return ;
	}

	p_sniffer_info->ms_stored_info.pwd_len = ms_msc_info.pwd_len;
	p_sniffer_info->ms_stored_info.ssid_len = ms_msc_info.ssid_len;
	memcpy(p_sniffer_info->ms_stored_info.pwd, ms_msc_info.pwd_data, ms_msc_info.pwd_len);
	memcpy(p_sniffer_info->ms_stored_info.ssid, ms_msc_info.ssid_data, ms_msc_info.ssid_len);
	memcpy(p_sniffer_info->lock_bssid, ms_msc_info.bssid, 6);
	memcpy(p_sniffer_info->ms_stored_info.random_num, ms_msc_info.random, 16);

	MS_TRACE("sniffer done");
	MS_TRACE("ssid:");
	PRINT_BUF(p_sniffer_info->ms_stored_info.ssid, ms_msc_info.ssid_len);
	MS_TRACE("pwd:");
	PRINT_BUF(p_sniffer_info->ms_stored_info.pwd, ms_msc_info.pwd_len);
	MS_TRACE("bssid:");
	PRINT_BUF(p_sniffer_info->lock_bssid, 6);
	MS_TRACE("random:");
	PRINT_BUF(p_sniffer_info->ms_stored_info.random_num, 16);
}

/**
 * \brief 快连配网的广播侦听回调。
 *\param buf 侦听的数据包
 *\param len 数据包长度
 *\param user_data 侦听的数据包
 */
void msm_sniffer_callback(unsigned char *buf, int len,void* user_data)
{
	//msc配网完成则退出回调函数。
	if(true == g_msm_sniffer_info.sniffer_done)
	{
		MS_TRACE("sniffer_done");
		return ;
	}

	//msc处理函数
	int msc_handle_result = ms_hal_msc_handle(buf, len);
	//信道锁定
	if(MSC_CHANNEL_LOCK == msc_handle_result)
	{
		g_msm_sniffer_info.lock_chn_info = MS_ENUM_LOCK_MSC;
		g_msm_sniffer_info.lock_chn_ms = msm_timer_get_systime_ms();

		MS_TRACE("msc locked");
	}
	//配网完成
	else if(MSC_FIN == msc_handle_result)
	{
		g_msm_sniffer_info.sniffer_done = true;
		g_msm_sniffer_info.lock_chn_info = MS_ENUM_LOCK_MSC;

		MS_TRACE("MSC_FIN");
		return ;
	}
	//配网出错，信道锁定超时或者密码错误
	else if(MSC_ERR == msc_handle_result)
	{
		g_msm_sniffer_info.sniffer_err_flag = 1;
		MS_TRACE("msc crc err, restart...");
	}
}

/**
 * @brief sniffer 参数的初始化
 */
static void msm_sniffer_config_init(void)
{
	CLR_STRUCT(&g_msm_sniffer_info);

	//msc参数初始化接口
	ms_hal_msc_init();
	MS_TRACE("start msc");
}

/**
 * @brief sniffer配网线程
 */
static void msm_sniffer_task()
{
	//切换信道变量
	uint8_t ms_ch = 0;

	//init msc
	msm_sniffer_config_init();

	///TODO  Modify by custom developer
	//start sniffer mode
	wifi_enter_promisc_mode();
	//设置sniffer的回调函数
	wifi_set_promisc(RTW_PROMISC_ENABLE_2, msm_sniffer_callback, 1);
    /// end

	MS_TRACE("%s %d", __FUNCTION__, __LINE__);

	while(1)
	{
		//快连配网完成，退出while循环
		if(true == g_msm_sniffer_info.sniffer_done)
		{
			MS_TRACE("%s %d", __FUNCTION__, __LINE__);

			break;
		}
		//信道未锁定时信道的切换方式
		else if(MS_ENUM_NO_LOCK == g_msm_sniffer_info.lock_chn_info)
		{
			ms_ch++;
			if(ms_ch > 13)
				ms_ch = 1;

			///TODO add by custom developer
			//设置信道
			if(0 != wifi_set_channel(ms_ch))
			{
				MS_TRACE("set chn error\r\n");
				goto QUIT;
			}

			MS_TRACE("%d", ms_ch);
			//信道切换的时间
			msm_timer_delay_ms(MS_CHANGE_CHANNEL_TIME);
		}
		//信道锁定后，30s未能成功完成配网，则重新init，重新开始配网
		else if(g_msm_sniffer_info.lock_chn_info > MS_ENUM_NO_LOCK)
		{
			if(1 == g_msm_sniffer_info.sniffer_err_flag || ((msm_timer_get_systime_ms()-g_msm_sniffer_info.lock_chn_ms) > MS_SNIFFER_CYCLE_MS))
			{
				MS_TRACE("%s %d,Wait for 30 seconds when channel locked", __FUNCTION__, __LINE__);
				msm_sniffer_config_init();
			}
		}
	}
	///TODO add by custom developer
	//配网完成，停止广播
	wifi_set_promisc(RTW_PROMISC_DISABLE, NULL, 0);

	//快连配网完成，获取result，并启动局域网
	if(true == g_msm_sniffer_info.sniffer_done)
	{
		if(MS_ENUM_LOCK_MSC == g_msm_sniffer_info.lock_chn_info)
		{
			msm_get_result(&g_msm_sniffer_info);
		}
	}

QUIT:
	//free msc
	ms_msc_free();

	MS_DBG_TRACE("sniffer quit");
}

/**
 * @brief sniffer配网入口
 */
msm_result_t main_smart_config(void)
{
	msm_sniffer_task();
}
