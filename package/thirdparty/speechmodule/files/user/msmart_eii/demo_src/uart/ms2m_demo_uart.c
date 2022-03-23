/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_demo_uart.c
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/04/24
 * Change Log           :
 *
 * All software, firmware and related documentation here in ("M-Smart Software") are
 * intellectual property of M-SMART Research Institute of Midea Group and protected
 * by law, including, but not limited to, copyright law and international treaties.
 *
 * Any use, modification, reproduction, retransmission, or republication of all
 * or part of M-Smart Software is expressly prohibited, unless prior written
 * permission has been granted by M-Smart.
 */

#include "stdint.h"
#include "string.h"

#include "elog.h"

#include "ms2m_hal_config.h"
//#include "ms2m_queue.h"
#include "ms2m_hal_os.h"
#include "ms2m_hal_chip.h"
#include "ms2m_hal_uart.h"

//#include "ms2m_info.h"

//#include "ms2m_uart_msmart.h"
#include "ms2m_EII.h"

#include "ms2m_demo_uart_msmart.h"
#include "ms2m_demo_uart.h"
#include "ms2m_demo_main.h"
#include <ms_common.h>

#define _TASK_STACK_SIZE MS2M_DEMO_UART_STACK_SIZE
#define _TASK_PRIORITY MS2M_DEMO_UART_PRIORITY

extern int g_asr_cmd;
extern ms2m_devinfo_t asr_last_ctrl_dev;
extern pthread_mutex_t asr_lock;
extern pthread_mutex_t asr_ctrl_list_lock;
extern ms2m_asr_ctrl_info asr_ctrl_info;

typedef struct ms2m_uart_msmart_head_s
{
	uint8_t head;
	uint8_t len;
	uint8_t appliance_type;
	uint8_t reserve[6];
	uint8_t cmd;
	uint8_t msg_body[1];
}ms2m_uart_msmart_head_t;

static ms2m_hal_os_thread_t gs_uart_thread = NULL;

typedef enum
{
	MS2M_UART_STATUS_SN_GET = 0,
	MS2M_UART_STATUS_TYPE_SET,
	MS2M_UART_STATUS_TYPE_GET,
	MS2M_UART_STATUS_START
}ms2m_uart_status_machine_e;

static ms2m_uart_status_machine_e gs_uart_status = MS2M_UART_STATUS_SN_GET;
//static ms2m_uart_status_machine_e gs_uart_status = MS2M_UART_STATUS_START;

static MS2M_STATUS _status_sn_get()
{
	uint8_t tmp[20] = {0};
	static int timestamp = 0;
	static char ac_flag = 0;
	uint8_t buff[255] = {0};
	uint16_t buff_len = 0;
	uint8_t recv[255] = {0};
	uint16_t recv_len = 0;
	MS2M_STATUS sn_ret = M_ERROR;
	MS2M_STATUS type_ret = M_ERROR;
	int now = ms2m_hal_get_ticks();
	if(ms2m_hal_ticks_compare(timestamp,now,300) == M_OK)
	{
		if(ac_flag)
		{
			ms2m_uart_msmart_pack(MS2M_UART_CMD_SN_QUERY, MS2M_UNKNOWN_APPLIANCE_TYPE,\
					tmp, 20, \
					buff, &buff_len);
			ac_flag = 0;
		}
		else
		{
			ms2m_uart_msmart_pack(MS2M_UART_CMD_AC_SN_QUERY, MS2M_UNKNOWN_APPLIANCE_TYPE,\
					tmp, 1, \
					buff, &buff_len);
			ac_flag = 1;
		}
		ms2m_hal_uart_write(buff,buff_len);
		timestamp = now;
	}
	if(M_OK == ms2m_uart_msmart_recv(recv, &recv_len))
	{
		ms2m_uart_msmart_head_t *head = (ms2m_uart_msmart_head_t *)recv;
		if(head->cmd != MS2M_UART_CMD_SN_QUERY &&  head->cmd != MS2M_UART_CMD_AC_SN_QUERY)
		{
			return M_CONTINUE;
		}
		if(ms2m_uart_msmart_checksum_check(recv) != M_OK)
		{
			return M_CONTINUE;
		}
		if(head->cmd == MS2M_UART_CMD_SN_QUERY)
		{
			sn_ret = eii_sn_set(head->msg_body);
			type_ret = eii_appliance_type_set(head->appliance_type);
			if(sn_ret == M_DIRTY || type_ret == M_DIRTY)
			{
				//Write to Flash by Zac
			}
		}
		else if(head->cmd == MS2M_UART_CMD_AC_SN_QUERY)
		{
			ms2m_uart_msmart_ac_sn_t *ac = (ms2m_uart_msmart_ac_sn_t *)head->msg_body;
			sn_ret = eii_sn_set(ac->sn);
			type_ret = eii_appliance_type_set(ac->type);
			if(sn_ret == M_DIRTY || type_ret == M_DIRTY)
			{
				//Write to Flash by Zac
			}
		}
		return M_OK;
	}
	return M_CONTINUE;
}

static MS2M_STATUS _status_model_get()
{
	static int timestamp = 0;
	uint8_t buff[255] = {0};
	uint16_t buff_len = 0;
	uint8_t recv[255] = {0};
	uint16_t recv_len = 0;
	int now = ms2m_hal_get_ticks();
	if(ms2m_hal_ticks_compare(timestamp,now,300) == M_OK)
	{
		ms2m_uart_msmart_pack(MS2M_UART_CMD_TYPE_QUERY, eii_appliance_type_get(),\
				recv, 20, \
				buff, &buff_len);
		ms2m_hal_uart_write(buff,buff_len);
		timestamp = now;
	}
	if(M_OK == ms2m_uart_msmart_recv(recv, &recv_len))
	{
		ms2m_uart_msmart_head_t *head = (ms2m_uart_msmart_head_t *)recv;
		if(head->cmd != MS2M_UART_CMD_TYPE_QUERY)
		{
			return M_CONTINUE;
		}
		ms2m_uart_msmart_type_t *payload = (ms2m_uart_msmart_type_t *)head->msg_body;
		if(ms2m_uart_msmart_checksum_check(recv) != M_OK)
		{
			return M_CONTINUE;
		}
		if(eii_model_set(payload->model_l,payload->model_h) == M_DIRTY)
		{
			//Write to Flash by Zac
		}
		return M_OK;
	}
	return M_CONTINUE;
}

//-----------------------------------------------------------------------//
static MS2M_STATUS _uart_want_send_cb(uint8_t *payload, uint16_t len)
{
	int ret = 0;

	ret = ms2m_hal_uart_write(payload,len);
	if(ret < 0)
	{
		MS_ERR_TRACE("Uart Write error");
		return M_ERROR;
	}
	return M_OK;
}

static void _uart_task(void *arg)
{
	//arg = arg;
	uint8_t buff[255] = {0};
	uint16_t buff_len = 0;

	eii_uart_cb_set(_uart_want_send_cb);

	while(1)
	{
		if(M_OK == ms2m_uart_msmart_recv(buff, &buff_len))
		{
			if(M_OK != eii_uart_put(buff, buff_len))
			{
				continue;
			}
		}
		ms2m_hal_msleep(1);
	}
}

static MS2M_STATUS _uart_start()
{
	if(NULL == gs_uart_thread)
	{
		if(M_OK != ms2m_hal_os_thread_create(&gs_uart_thread,
					"UART_thread",
					_uart_task,
					NULL,
					_TASK_STACK_SIZE,
					_TASK_PRIORITY))
		{
			MS_ERR_TRACE("ms2m_uart_task fail\r\n");
			return M_ERROR;
		}
	}

	MS_TRACE("ms2m_factory thread has started, %p\r\n", gs_uart_thread);
	return M_OK;
}

MS2M_STATUS ms2m_sn_devicetype_set(void)
{
	MS2M_STATUS lRetVal;
	ms_stored_info_t temp_dev_info;

	memset(&temp_dev_info, 0, sizeof(temp_dev_info));
	if(-1 == msm_read_config(MS_STORED_INFO, (char *)&temp_dev_info, sizeof(temp_dev_info)))
	{
		return M_ERROR;
	}
	lRetVal = eii_sn_set(temp_dev_info.sn);
	if(lRetVal != M_OK)
		goto LAST;
	lRetVal = eii_appliance_type_set(temp_dev_info.device_type[1]);
	if(lRetVal != M_OK)
		goto LAST;
	lRetVal = eii_model_set(temp_dev_info.device_type[2],temp_dev_info.device_type[3]);
LAST:
	return lRetVal;
}

static MS2M_STATUS ms2m_uart_send(uint8_t *data, uint16_t len)
{
	int fd = -1;
	int ret;
	struct T_MsTransStream frame;
	int length = 0;
	char buf[256];

	fd = open(EII_OUT, O_RDWR | O_NONBLOCK);
	if(fd < 0){
		MS_TRACE("open %s error(%s)\n", strerror(errno));
		close(fd);
		return M_ERROR;
	}

	memset(&frame, 0, sizeof(frame));

	if(len > MAX_UART_LEN){
		MS_TRACE("data is too long!!\n");
		return M_ERROR;
	}
	memcpy(frame.data, data, len);
	frame.size = len;
	frame.event = MS_SYS_EVENT_TRANSDATA;
	ret = write(fd, &frame, sizeof(frame));
	if(ret < 0){
		MS_TRACE("read message error(%s)\n", strerror(errno));
		close(fd);
		return M_ERROR;
	}

	close(fd);

	return M_OK;
}

static MS2M_STATUS ms2m_mspeech_send(uint8_t *data, uint16_t len, E_MsDownStream event)
{
	int fd = -1;
	int ret;
	struct T_MsTransStream frame;
	int length = 0;
	char buf[256];

	fd = open(MAISPEECH_IN, O_RDWR | O_NONBLOCK);
	if(fd < 0){
		MS_TRACE("open %s error(%s)\n", strerror(errno));
		close(fd);
		return M_ERROR;
	}

	memset(&frame, 0, sizeof(frame));

	if(len > MAX_UART_LEN){
		MS_TRACE("data is too long!!\n");
		return M_ERROR;
	}
	memcpy(frame.data, data, len);
	frame.size = len;
	frame.event = event;
	ret = write(fd, &frame, sizeof(frame));
	if(ret < 0){
		MS_TRACE("read message error(%s)\n", strerror(errno));
		close(fd);
		return M_ERROR;
	}

	close(fd);

	return M_OK;
}

MS2M_STATUS ms2m_dev_play_audio(char *path)
{
	return ms2m_mspeech_send(path, strlen(path)+1, MS_EVENT_TO_MSPEECH_PLAY_AUDIO);
}

#define EII_UPLOAD_MSG_ID	0x21
#define EII_DOWNLOAD_MSG_ID	0x22

static MS2M_STATUS ms2m_uart_send_cb(uint8_t *data, uint16_t len)
{
	ms2m_dev_state_repo *p_repo;
	uint8_t *devid;

	MS_TRACE("ms2m_uart_send_cb recv: ");
	PRINT_BUF(data, len);
	switch(data[9]){
		case EII_UPLOAD_MSG_ID:
		case EII_DOWNLOAD_MSG_ID:
			switch(data[10]){
				case CMD_DEV_CTRL_SUCCESS:
					pthread_mutex_lock(&asr_lock);
					if(M_OK == ms2m_del_dev_info(data+11, &(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock)){
						//asr_ctrl_info.list_len--;
						asr_ctrl_info.list_len = 0;
						ms2m_dev_info_clean(&(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock);
						if(asr_ctrl_info.asr_cmd != ASR_CMD_NULL){
							local_timer_stop();
							switch(asr_ctrl_info.asr_cmd){
								case ASR_CMD_RAC_ON:
									ms2m_dev_play_audio(RAC_ON_AUDIO);
									break;
								case ASR_CMD_RAC_OFF:
									ms2m_dev_play_audio(RAC_OFF_AUDIO);
									break;
								case ASR_CMD_HUM_ON:
									ms2m_dev_play_audio(HUM_ON_AUDIO);
									break;
								case ASR_CMD_HUM_OFF:
									ms2m_dev_play_audio(HUM_OFF_AUDIO);
									break;
								case ASR_CMD_GWH_ON:
									ms2m_dev_play_audio(GWH_ON_AUDIO);
									break;
								case ASR_CMD_GWH_OFF:
									ms2m_dev_play_audio(GWH_OFF_AUDIO);
									break;
								case ASR_CMD_DWM_ON:
									ms2m_dev_play_audio(DWM_ON_AUDIO);
									break;
								case ASR_CMD_DWM_OFF:
									ms2m_dev_play_audio(DWM_OFF_AUDIO);
									break;
								default:
									break;
							}
							asr_ctrl_info.asr_cmd = ASR_CMD_NULL;
						}
					}
					pthread_mutex_unlock(&asr_lock);
					break;
				case CMD_DEV_STATE_REPO:
					//TODO
					break;
				default:
					break;
			}
			break;
		default:
			return ms2m_uart_send(data, len);
	}
	return M_OK;

}

MS2M_STATUS ms2m_demo_uart_init()
{
#if 0
	MS2M_STATUS ret = ms2m_hal_uart_init();
	if(ret == M_ERROR)
	{
		return M_ERROR;
	}
#endif
	while(1)
	{
		switch(gs_uart_status)
		{
			case MS2M_UART_STATUS_SN_GET:
				if(ms2m_sn_devicetype_set() == M_OK)
				{
					gs_uart_status = MS2M_UART_STATUS_TYPE_GET;
				}
				break;
			case MS2M_UART_STATUS_TYPE_GET:
				MS_TRACE("eii_appliance_type:0x%02x",eii_appliance_type_get());
				gs_uart_status = MS2M_UART_STATUS_START;
				break;
			case MS2M_UART_STATUS_START:
				goto step2;
				break;
			default:
				break;
		}
		ms2m_hal_msleep(10);
	}
step2:
	if(M_OK != eii_uart_cb_set(ms2m_uart_send_cb)){
		MS_TRACE("failed to call eii_uart_cb_set()");
		return M_ERROR;
	}
	return M_OK;
}
