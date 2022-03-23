/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_main.c
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/04/11
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

#include "stdio.h"
#include "stdint.h"

#include "elog.h"

#include "ms2m_hal_chip.h"

#include "ms2m_hal_wlan.h"

#include "ms2m_EII.h"

#include "ms2m_hal_common.h"
#include "ms2m_demo_uart.h"

#include "ms2m_demo_main.h"
#include "ms_common.h"
#include "ms_net_app.h"
#include <semaphore.h>
#include <sys/epoll.h>
#include <signal.h>
#include <time.h>

bool m_eii_lua_dl_fail = false;
static T_MsTransStream         *m_send_frame = NULL;
static timer_t localtimer=0;
pthread_mutex_t m_lock;
pthread_mutex_t send_lock;
pthread_mutex_t devinfo_list_lock;
pthread_mutex_t asr_ctrl_list_lock;
pthread_mutex_t asr_lock;
sem_t m_sem;
LIST_HEAD(ms2m_devinfo_list);
ms2m_asr_ctrl_info asr_ctrl_info;

static const MS_MS2M_HANDLE EII_CMD_ARRAY[]={
	{MS2M_CLOUD_N2M_LKG_EXEC_ISSUED,         EII_CLOUD_LKG_EXEC},
	{MS2M_CLOUD_N2M_LKG_WHEN_SET,            EII_CLOUD_LKG_WHEN_SET},
	{MS2M_CLOUD_N2M_LKG_THEN_SET,            EII_CLOUD_LKG_THEN_SET},
	{MS2M_CLOUD_N2M_LKG_JUDGE,               EII_CLOUD_LKG_JUDGE},
	{MS2M_CLOUD_N2M_LKG_DEL,                 EII_CLOUD_LKG_DEL},
	{MS2M_CLOUD_N2M_LKG_OPT,                 EII_CLOUD_LKG_OPT},
	{MS2M_CLOUD_N2M_LKG_STORE,               EII_CLOUD_LKG_STORE},
};

void timeout_handler (int signum)
{
	local_timer_stop();
	MS_TRACE("timeout_handler\r\n");
	pthread_mutex_lock(&asr_lock);
	if(asr_ctrl_info.asr_cmd != ASR_CMD_NULL){
		ms2m_dev_info_clean(&(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock);
		asr_ctrl_info.asr_cmd = ASR_CMD_NULL;
		asr_ctrl_info.list_len = 0;
		ms2m_dev_play_audio(CONTROL_FAIL_AUDIO);
	}
	pthread_mutex_unlock(&asr_lock);
}

void local_timer_init(void)
{
	struct sigevent evp;
	int ret;

	if(localtimer >0)
		local_timer_stop();
	evp.sigev_value.sival_int = 0;
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = SIGUSR1;
	signal(SIGUSR1, timeout_handler);
	ret = timer_create(CLOCK_REALTIME, &evp, &localtimer);
	if(ret){
		MS_TRACE("timer_create");
	}
}

void local_timer_start(void)
{
	int ret;
	struct itimerspec ts;
	MS_TRACE("local_timer_start\r\n");
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;
	ts.it_value.tv_sec = 5;
	ts.it_value.tv_nsec = 0;
	ret = timer_settime(localtimer, 0, &ts, NULL);
	if(ret)
		MS_TRACE("timer_settime");

}

void local_timer_stop(void)
{
	int ret;
	struct itimerspec ts;

	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;
	ts.it_value.tv_sec = 0;
	ts.it_value.tv_nsec = 0;
	ret = timer_settime(localtimer, 0, &ts, NULL);
	if( ret )
		MS_TRACE("timer_settime");

	timer_delete(localtimer);
	localtimer =0;

}

bool msmart_cloud_is_login_success(void)
{
	uint8_t cloud_status = CLOUD_DISCONNECT;

	if(-1==(msm_read_config(MS_CLOUD_STATUS_PATH,(char*)&cloud_status,sizeof(uint8_t)))){
		return false;
	}
	return (cloud_status == CLOUD_CONNECTED);
}

static MS2M_STATUS ms_net_app_send(T_MsTransStream *request)
{
	int fd = -1;
	int ret;
	pthread_mutex_lock(&send_lock);

	fd = open(MSMART_NET_IN, O_RDWR | O_NONBLOCK);
	if(fd < 0){
		MS_TRACE("open %s error(%s)\n", strerror(errno));
		close(fd);
		return M_ERROR;
	}

	ret = write(fd, request, sizeof(T_MsTransStream));
	if(ret < 0){
		MS_TRACE("write message error(%s)\n", strerror(errno));
		close(fd);
		return M_ERROR;
	}

	close(fd);
	pthread_mutex_unlock(&send_lock);
	return M_OK;
}

MS2M_STATUS msmart_cloud_send_request(T_MsTransStream *request, T_MsTransStream **p_response, int msec)
{
	struct timespec ts;
	struct timeval tt;
	int ret;

	if (!IS_NULL(p_response) && (0 == msec)) {
		MS_TRACE("input invalid");
		return M_ERROR;
	}
	if (!msmart_cloud_is_login_success()) {
		MS_TRACE("cloud not login yet");
		return M_ERROR;
	}
	do{
		pthread_mutex_lock(&m_lock);
		if(!IS_NULL(m_send_frame)){
			pthread_mutex_unlock(&m_lock);
			usleep(100*1000);
		}else{
			break;
		}
	}while(1);
	m_send_frame = request;
	ret = ms_net_app_send(request);
	if(ret != M_OK){
		goto func_exit;
	}
	if(!IS_NULL(p_response)){
		pthread_mutex_unlock(&m_lock);
		gettimeofday(&tt,NULL);
		ts.tv_sec = tt.tv_sec + msec/1000;
		ts.tv_nsec = (tt.tv_usec + (msec%1000)*1000)* 1000;
		ret = sem_timedwait(&m_sem, &ts);
		pthread_mutex_lock(&m_lock);
		if(ret == M_OK){
			*p_response = (T_MsTransStream *)malloc(sizeof(T_MsTransStream));
			if (IS_NULL(*p_response))
				goto func_exit;
			memcpy(*p_response, m_send_frame, sizeof(T_MsTransStream));
		}
	}

func_exit:
	m_send_frame = NULL;
	pthread_mutex_unlock(&m_lock);
	return ret;
}

MS2M_STATUS ms2m_upload_data(MS2M_CLOUD_MSG_YTPE cmd,
		uint8_t isAck, uint8_t needResponse,
		uint8_t *buf, uint16_t len,
		T_MsTransStream **p_resp)
{
	T_MsTransStream *request = NULL;
	T_MsTransStream *response = NULL;
	int ret;

	request = (T_MsTransStream *)malloc(sizeof(T_MsTransStream));
	if (!request)
		return M_NO_MEM;
	if(isAck){
		request->event = MS_EII_M2M_ACK_CLOUD;
	}
	else{
		request->event = MS_EII_M2M_CLOUD_TRANS;
	}
	request->data[0] = cmd&0x00FF;
	request->data[1] = ((cmd&0xFF00)>>8);
	memcpy(&request->data[2], buf, len);
	request->size = len+2;
	request->msg_id = 0;
	if(needResponse)
		ret = msmart_cloud_send_request(request, &response, 5000);
	else
		ret = msmart_cloud_send_request(request, NULL, 0);
	if(ret != M_OK)
		goto func_exit;
	if(response != NULL){
		if((response->data[0] == cmd&0x00FF) &&
				(response->data[1] == (cmd&0xFF00)>>8)){
			if(!IS_NULL(p_resp)){
				*p_resp = response;//外部释放
			}else{
				free(response);
			}
			ret = M_OK;
		}
		else{
			free(response);
			ret = M_ERROR;
		}
	}
func_exit:
	free(request);
	return ret;
}

MS2M_STATUS eii_demo_cb(EII_COMMAND cmd, uint8_t *databuf, uint16_t len)
{
	//MS_DBG_TRACE("CMD:%x\r\n:%s",cmd,data);
	unsigned int script_addr = 0;
	MS2M_STATUS ret = M_ERROR;
	T_MsTransStream *resp = NULL;
	uint32_t    real_size = 0;

	MS_TRACE("cmd:0x%04x\r\n", cmd);
	switch(cmd)
	{
		case EII_STATUS_NO_SCRIPT:
		case EII_STATUS_SCRIPT_ERROR:
			{
				char *tmp_buf = NULL;
				uint32_t coap_len;
				uint8_t *coap_str;
				char    *ver;
				uint16_t ver_len;
				char    *md5;
				uint16_t md5_len;
				char    *url;
				uint16_t url_len;
				MS2M_STATUS ret;

				while (!msmart_cloud_is_login_success()) {
					MS_TRACE("cloud not login yet");
					sleep(1);
				}
				tmp_buf = malloc(128);
				if(tmp_buf == NULL)
					break;
				ver = malloc(128);
				if(ver == NULL)
					goto exit_tmp_buf;
				md5 = malloc(128);
				if(md5 == NULL)
					goto exit_ver;
				url = malloc(256);
				if(url == NULL)
					goto exit_md5;
				//LUA请求
				ms2m_sn_devicetype_set();
				script_addr = eii_script_addr_get();
				MS_TRACE("script addr is 0x%08x\n",script_addr);
				coap_str = eii_cloud_script_request_package(&coap_len);

				ret = ms2m_upload_data(MS2M_CLOUD_M2N_REQUEST_LUA, 0, 1, coap_str, coap_len, &resp);
				if(ret != M_OK){
					m_eii_lua_dl_fail = true;
					MS_TRACE("Req Lua failed.");
					goto exit_url;
				}
				ret = eii_script_download_get(&resp->data[2], strlen(&resp->data[2]),
						ver, &ver_len, md5, &md5_len,
						url, &url_len);
				MS_TRACE("get lua info ret:%d, info:%s", ret, &resp->data[2]);
				if(ret != M_OK && ret != M_DIRTY){
					m_eii_lua_dl_fail = true;
					goto exit_url;
				}

				url[url_len] = '\0';
				ret = ms2m_hal_flash_http_get(script_addr, url, &real_size);
				free(resp);
				if(ret != M_OK){
					m_eii_lua_dl_fail = true;
					MS_TRACE("Download lua failed.");
				}
exit_url:
				free(url);
exit_md5:
				free(md5);
exit_ver:
				free(ver);
exit_tmp_buf:
				free(tmp_buf);
				if(m_eii_lua_dl_fail == false){
					eii_cloud_cmd(EII_CMD_SCRIPT_READY, NULL, real_size, NULL, 0);
					MS_TRACE("Download lua script complete.");
				}
			}
			break;

		case EII_CLOUD_DEV_CNTL_ACK:
			if(databuf != NULL){
				ret = ms2m_upload_data(MS2M_CLOUD_M2N_DEV_CNTL_ACK, 1, 0, databuf, len, NULL);
			}
			break;

		case EII_CLOUD_DEV_STATUS_CHANGE:
			if(databuf != NULL){
				ret = ms2m_upload_data(MS2M_CLOUD_M2N_DEV_STATUS_UPDATE, 0, 1, databuf, len, NULL);
			}
			break;

		case EII_LAN_LKG_EXEC:
			if(databuf != NULL){
				ret = ms2m_upload_data(MS2M_CLOUD_M2N_LKG_EXEC_UPDATA, 0, 1, databuf, len, NULL);
			}
			break;

		case EII_LAN_LKG_JUDGE:
			if(databuf != NULL){
				ret = ms2m_upload_data(MS2M_CLOUD_M2N_LKG_EXEC_FIN_UPDATA, 0, 1, databuf, len, NULL);
			}
			break;

		case EII_LAN_LKG_ACT:
			if(databuf != NULL){
				ret = ms2m_upload_data(MS2M_CLOUD_M2N_LKG_WHEN_UPDATA, 0, 1, databuf, len, NULL);
			}
			break;

		default:
			break;
	}

	return M_OK;
}

MS2M_STATUS ms_store_dev_position(char *str, int len)
{
	if(-1 == msm_write_config(EII_POSITION, str, len)){
		MS_TRACE("read eii position faild");
		return M_ERROR;
	}
	return M_OK;
}

MS2M_STATUS ms_get_dev_position(char *str, int len)
{
	if(-1 == msm_read_config(EII_POSITION, str, len)){
		MS_TRACE("read eii position faild");
		return M_ERROR;
	}
	str[len-1] = 0;
	return M_OK;
}

uint16_t ms2m_pase_cloud(uint8_t *buff, uint32_t len)
{
	uint16_t sub_cmd = 0;
	uint16_t index = 0;
	EII_COMMAND eii_cmd = 0;
	uint8_t *ack = NULL;
	uint16_t ack_len = 0;
	unsigned int ret = 0;

	unsigned int eii_retcode = 0;
	MS_TRACE("enter");

	sub_cmd = buff[index++];
	sub_cmd += ((uint16_t)buff[index++] & 0x00FF) <<8;
	MS_TRACE("eii recv sub_cmd 0x%04x is %s", sub_cmd, &buff[index]);
	switch(sub_cmd)
	{
		case MS2M_CLOUD_N2M_LKG_EXEC_ISSUED: //0003
		case MS2M_CLOUD_N2M_LKG_WHEN_SET: //0004
		case MS2M_CLOUD_N2M_LKG_THEN_SET:  //0005
		case MS2M_CLOUD_N2M_LKG_JUDGE: //0006
		case MS2M_CLOUD_N2M_LKG_DEL: //0007
		case MS2M_CLOUD_N2M_LKG_OPT: //0008
		case MS2M_CLOUD_N2M_LKG_STORE: //0009
			eii_cmd = EII_CMD_ARRAY[sub_cmd - 3].eii_cmd;
			ret = eii_cloud_cmd(eii_cmd, &buff[index], strlen(&buff[index]), &ack, &ack_len);
			MS_TRACE("eii_cloud_cmd ret is %d,acklen is %d",
					ret, ack_len);
			break;

		case MS2M_CLOUD_N2M_DEV_CNTL: //0011,设备控制必定为异步应答
			eii_cmd = EII_CLOUD_DEV_CNTL;
			ret = eii_cloud_cmd(eii_cmd, &buff[index], strlen(&buff[index]), &ack, &ack_len);
			MS_TRACE("eii return ret is %d,acklen is %d",
					ret,ack_len);
			break;

		default:
			break;
	}
	if((ack_len > 0) && (ack != NULL)){
		memcpy(&buff[index], ack, ack_len);
		MS_TRACE("Ack msg:%s", ack);
		eii_cloud_ack_free(ack);
	}
	else{
		MS_TRACE("response is null\n");
		ack_len = 0;
	}
	MS_TRACE("leave");
	return ack_len;
}

static const uint8_t CRC8TABLE[] = {
    0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
    157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
    35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
    190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
    70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
    219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
    101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
    248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
    87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
    233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
    116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

uint8_t CRC8(const uint8_t * data, uint32_t size)
{
    uint8_t result = 0;

    while (size--) {
            result = CRC8TABLE[result ^ *data];
            data++;
    }

    return(result);
}

uint8_t ms_uart_pro_check_sum(const uint8_t *offset_rx, uint8_t rx_len)
{
	uint8_t check_sum = 0;
	uint8_t i = 0;

	for (i = 0; i < rx_len; i++){
		check_sum += offset_rx[i];
	}
	check_sum = ~(check_sum);
	check_sum += 1;

	return check_sum;
}

MS2M_STATUS ms2m_del_dev_info(uint8_t *info, struct list_head *list, pthread_mutex_t *lock)
{
	ms2m_devinfo_node *p_devinfo = NULL;

	pthread_mutex_lock(lock);
	list_for_each_entry(p_devinfo, list, node, ms2m_devinfo_node){
		if(!memcmp(&(p_devinfo->info), info, sizeof(p_devinfo->info))){
			list_del(&(p_devinfo->node));
			free(p_devinfo);
			pthread_mutex_unlock(lock);
			return M_OK;
		}
	}
	pthread_mutex_unlock(lock);
	return M_ERROR;
}

MS2M_STATUS ms2m_add_dev_info(uint8_t *info, struct list_head *list, pthread_mutex_t *lock)
{
	ms2m_devinfo_node *p_devinfo = NULL;

	p_devinfo = (ms2m_devinfo_node *)malloc(sizeof(ms2m_devinfo_node));
	if(!p_devinfo){
		MS_TRACE("malloc ms2m_devinfo_node error!!\n");
		return M_ERROR;
	}
	memcpy(&(p_devinfo->info), info, sizeof(ms2m_devinfo_t));
	pthread_mutex_lock(lock);
	list_add(&(p_devinfo->node), list);
	pthread_mutex_unlock(lock);
    free(p_devinfo);
	return M_OK;
}

void ms2m_dev_info_clean(struct list_head *list, pthread_mutex_t *lock)
{
	ms2m_devinfo_node *p_devinfo = NULL;
	ms2m_devinfo_node *n = NULL;

	pthread_mutex_lock(lock);
	list_for_each_entry_safe(p_devinfo, n, list, node, ms2m_devinfo_node){
		list_del(&(p_devinfo->node));
		free(p_devinfo);
	}
	pthread_mutex_unlock(lock);
}

int find_device_info(uint8_t roomtype, uint8_t devtype, struct list_head *src_list, pthread_mutex_t *src_lock,\
		struct list_head *dst_list, pthread_mutex_t *dst_lock)
{
	ms2m_devinfo_node *p_devinfo = NULL;
	int count = 0;

	if(src_list == NULL || dst_list == NULL)
		goto out;

	pthread_mutex_lock(src_lock);
	list_for_each_entry(p_devinfo, src_list, node, ms2m_devinfo_node){
		if((roomtype == ROOM_TYPE_ALL || p_devinfo->info.roomtype == roomtype) && p_devinfo->info.devtype == devtype){
			ms2m_add_dev_info(&(p_devinfo->info), dst_list, dst_lock);
			count++;
		}
	}
	pthread_mutex_unlock(src_lock);
out:
	return count;
}

MS2M_STATUS ms2m_load_dev_info(void)
{
	int fd = -1;
	ms2m_devinfo_t info;
	int info_len = sizeof(ms2m_devinfo_t);

	fd = open(EII_DEVINFO, O_RDONLY);
	if(fd == -1){
		MS_TRACE("open %s fail!\n",EII_DEVINFO);
		return M_ERROR;
	}

	while(read(fd, &info, info_len) == info_len){
		ms2m_add_dev_info(&info, &ms2m_devinfo_list, &devinfo_list_lock);
	}

	return M_OK;
}

MS2M_STATUS ms2m_save_dev_info(uint8_t *data, int size)
{
	int len = size-14;
	uint8_t *p_info;

	if(len < 0){
		MS_TRACE("have no dev info!!");
		return M_ERROR;
	}
/*
	if(data[data[1]] == ms_uart_pro_check_sum(data+1, data[1]-1)){
		MS_TRACE("check sum fail!!");
		return M_ERROR;
	}
*/
	if(-1 == msm_write_config(EII_DEVINFO, data+12, size-14)){
		MS_TRACE("write eii devinfo faild");
		return M_ERROR;
	}
	p_info = data+12;

	while(len >= sizeof(ms2m_devinfo_t)){
		ms2m_add_dev_info(p_info, &ms2m_devinfo_list, &devinfo_list_lock);
		len -= sizeof(ms2m_devinfo_t);
		p_info += sizeof(ms2m_devinfo_t);
	}

	return M_OK;
}

MS2M_STATUS ms2m_do_ctrl(uint8_t cmd, ms2m_devinfo_t *p_info, uint8_t subcmd)
{
	uint8_t msg[256];
	int data_len = 0;
	uint8_t *data = NULL;
	uint8_t uart_data[40];
	uint8_t msg_data[256];

	memset(msg, 0, 256);
	data = &msg[10];
	data[data_len++] = cmd;
	if(cmd == CMD_POWER_CTRL || cmd == CMD_FUNC_CTRL){
		memcpy(data+data_len, p_info, sizeof(ms2m_devinfo_t));
		data_len += sizeof(ms2m_devinfo_t);
	}
	data[data_len++] = subcmd;
	data[data_len] = CRC8(data, data_len-1);
	msg[0] = 0xAA;
	msg[1] = data_len + 10;
	msg[2] = 0xAC;
	msg[9] = 0x21;
	msg[msg[1]] = ms_uart_pro_check_sum(msg+1, msg[1]-1);

	MS_TRACE("EII uart put msg:");
	PRINT_BUF(msg, msg[1]+1);
	eii_uart_put(msg, msg[1]+1);
	return M_OK;
}

MS2M_STATUS ms2m_other_device_ctrl(uint8_t cmd, uint8_t roomtype, uint8_t devtype, uint8_t subcmd)
{
	MS2M_STATUS ret = M_ERROR;
	ms2m_devinfo_node *p_devinfo = NULL;
	ms2m_devinfo_node *n = NULL;

	switch(devtype){
		case DEV_TYPE_WM:
			pthread_mutex_lock(&asr_lock);
			asr_ctrl_info.list_len += find_device_info(roomtype, DEV_TYPE_PWM, &ms2m_devinfo_list, \
					&devinfo_list_lock, &(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock);
			asr_ctrl_info.list_len += find_device_info(roomtype, DEV_TYPE_DWM, &ms2m_devinfo_list, \
					&devinfo_list_lock, &(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock);
			asr_ctrl_info.list_len += find_device_info(roomtype, DEV_TYPE_CWM, &ms2m_devinfo_list, \
					&devinfo_list_lock, &(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock);
			pthread_mutex_unlock(&asr_lock);
			break;
		case DEV_TYPE_WH:
			pthread_mutex_lock(&asr_lock);
			asr_ctrl_info.list_len += find_device_info(roomtype, DEV_TYPE_AWH, &ms2m_devinfo_list, \
					&devinfo_list_lock, &(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock);
			asr_ctrl_info.list_len += find_device_info(roomtype, DEV_TYPE_EWH, &ms2m_devinfo_list, \
					&devinfo_list_lock, &(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock);
			asr_ctrl_info.list_len += find_device_info(roomtype, DEV_TYPE_GWH, &ms2m_devinfo_list, \
					&devinfo_list_lock, &(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock);
			pthread_mutex_unlock(&asr_lock);
			break;
		default:
			pthread_mutex_lock(&asr_lock);
			asr_ctrl_info.list_len += find_device_info(roomtype, devtype, &ms2m_devinfo_list, \
					&devinfo_list_lock, &(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock);
			pthread_mutex_unlock(&asr_lock);
			break;
	}
	if(asr_ctrl_info.list_len > 0){
		pthread_mutex_lock(&asr_ctrl_list_lock);
		list_for_each_entry_safe(p_devinfo, n, &(asr_ctrl_info.asr_ctrl_list), node, ms2m_devinfo_node){
			ms2m_do_ctrl(cmd, &(p_devinfo->info), subcmd);
		}
		pthread_mutex_unlock(&asr_ctrl_list_lock);
		ret = M_OK;
	}

	return ret;
}

void ASR_process(char *asr, int len)
{
	int ret = M_NO_LKG;

	pthread_mutex_lock(&asr_lock);
	if(asr_ctrl_info.asr_cmd != ASR_CMD_NULL){
		ms2m_dev_info_clean(&(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock);
		asr_ctrl_info.asr_cmd = ASR_CMD_NULL;
		asr_ctrl_info.list_len = 0;
	}
	pthread_mutex_unlock(&asr_lock);

	if(!strncmp(asr,"washing_machine_on", len)){
		ret = ms2m_other_device_ctrl(CMD_POWER_CTRL, ROOM_TYPE_BEDROOM, DEV_TYPE_WM, ON);
		if(ret == M_OK){
			pthread_mutex_lock(&asr_lock);
			asr_ctrl_info.asr_cmd = ASR_CMD_DWM_ON;
			pthread_mutex_unlock(&asr_lock);
		}
	}
	else if(!strncmp(asr,"washing_machine_off", len)){
		ret = ms2m_other_device_ctrl(CMD_POWER_CTRL, ROOM_TYPE_BEDROOM, DEV_TYPE_WM, OFF);
		if(ret == M_OK){
			pthread_mutex_lock(&asr_lock);
			asr_ctrl_info.asr_cmd = ASR_CMD_DWM_OFF;
			pthread_mutex_unlock(&asr_lock);
		}
	}
	else if(!strncmp(asr,"water_heater_on", len)){
		ret = ms2m_other_device_ctrl(CMD_POWER_CTRL, ROOM_TYPE_BEDROOM, DEV_TYPE_WH, ON);
		if(ret == M_OK){
			pthread_mutex_lock(&asr_lock);
			asr_ctrl_info.asr_cmd = ASR_CMD_GWH_ON;
			pthread_mutex_unlock(&asr_lock);
		}
	}
	else if(!strncmp(asr,"water_heater_off", len)){
		ret = ms2m_other_device_ctrl(CMD_POWER_CTRL, ROOM_TYPE_BEDROOM, DEV_TYPE_WH, OFF);
		if(ret == M_OK){
			pthread_mutex_lock(&asr_lock);
			asr_ctrl_info.asr_cmd = ASR_CMD_GWH_OFF;
			pthread_mutex_unlock(&asr_lock);
		}
	}
	else if(!strncmp(asr,"bedroom_AC_on", len)){
		ret = ms2m_other_device_ctrl(CMD_POWER_CTRL, ROOM_TYPE_BEDROOM, DEV_TYPE_RAC, ON);
		if(ret == M_OK){
			pthread_mutex_lock(&asr_lock);
			asr_ctrl_info.asr_cmd = ASR_CMD_RAC_ON;
			pthread_mutex_unlock(&asr_lock);
		}
	}
	else if(!strncmp(asr,"bedroom_AC_off", len)){
		ret = ms2m_other_device_ctrl(CMD_POWER_CTRL, ROOM_TYPE_BEDROOM, DEV_TYPE_RAC, OFF);
		if(ret == M_OK){
			pthread_mutex_lock(&asr_lock);
			asr_ctrl_info.asr_cmd = ASR_CMD_RAC_OFF;
			pthread_mutex_unlock(&asr_lock);
		}
	}
	else if(!strncmp(asr,"back", len)){
		/*
		ret = ms2m_other_device_ctrl(CMD_SCENE_CTRL, ROOM_TYPE_ALL, DEV_TYPE_ALL, BACK);
		pthread_mutex_lock(&asr_lock);
		asr_ctrl_info.asr_cmd = ASR_CMD_BACK;
		pthread_mutex_unlock(&asr_lock);
		*/
	}
	else if(!strncmp(asr,"leave", len)){
		/*
		ret = ms2m_other_device_ctrl(CMD_SCENE_CTRL, ROOM_TYPE_ALL, DEV_TYPE_ALL, LEAVE);
		pthread_mutex_lock(&asr_lock);
		asr_ctrl_info.asr_cmd = ASR_CMD_LEAVE;
		pthread_mutex_unlock(&asr_lock);
		*/
	}
	else if(!strncmp(asr,"humidifier_on", len)){
		ret = ms2m_other_device_ctrl(CMD_POWER_CTRL, ROOM_TYPE_ALL, DEV_TYPE_HUM, ON);
		if(ret == M_OK){
			pthread_mutex_lock(&asr_lock);
			asr_ctrl_info.asr_cmd = ASR_CMD_HUM_ON;
			pthread_mutex_unlock(&asr_lock);
		}
	}
	else if(!strncmp(asr,"humidifier_off", len)){
		ret = ms2m_other_device_ctrl(CMD_POWER_CTRL, ROOM_TYPE_ALL, DEV_TYPE_HUM, OFF);
		if(ret == M_OK){
			pthread_mutex_lock(&asr_lock);
			asr_ctrl_info.asr_cmd = ASR_CMD_HUM_OFF;
			pthread_mutex_unlock(&asr_lock);
		}
	}
	if(ret == M_OK){
		local_timer_init();
		local_timer_start();
	}else{
		ms2m_dev_play_audio(DEV_NOT_FOUND_AUDIO);
	}
}

void *read_fifo_routine(void *user){

	int fd_eii_in = -1;
	int readcnt;
	int ne, nevents, ret, fd;
	struct T_MsTransStream  transtream;
	int mEpollFd = epoll_create(1);
	struct epoll_event events[1];
	struct epoll_event  ev;

	MS_TRACE("read fifo routine start ...");

	if(access(EII_IN,F_OK)==-1){
		MS_TRACE("fifo file is empty\r\n");
		ret=mkfifo(EII_IN,0777);
		if(ret!=0){
			MS_TRACE("Could not create fifo %s\n",EII_IN);
		}
	}

	if(-1 == (fd_eii_in = open(EII_IN,O_RDWR |O_NONBLOCK))){
		MS_TRACE("/tmp/eii_in OPEN ERROR\n");
		close(fd_eii_in);
		return NULL;
	}
	ev.events  = EPOLLIN | EPOLLET |EPOLLERR|EPOLLRDHUP;
	ev.data.fd = fd_eii_in;
	while (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd_eii_in, &ev) < 0 && errno == EINTR);

	while(1){
		nevents = epoll_wait(mEpollFd, events, 1, 200);  // 20ms timeout
		if (nevents < 0) {
			MS_TRACE("epoll_wait() unexpected error: %s", strerror(errno));
			usleep(100*1000);
			continue;
		}

		for (ne = 0; ne < nevents; ne++) {
			if(events[ne].events & EPOLLIN) {
				fd = events[ne].data.fd;
				if(fd == fd_eii_in) {
					if(-1 == (readcnt = read(fd, &transtream,sizeof(struct T_MsTransStream)))){
						MS_TRACE("READ DEVICE ERROR");
					}else{
						switch(transtream.event){
							case MS_EII_M2M_CLOUD_TRANS:
								pthread_mutex_lock(&m_lock);
								if(m_send_frame && m_send_frame->event == MS_EII_M2M_CLOUD_TRANS){
									memcpy(m_send_frame->data, transtream.data, transtream.size);
									m_send_frame->size = transtream.size;
									sem_post(&m_sem);
								}
								pthread_mutex_unlock(&m_lock);
								break;
							case MS_EII_MSM_PASE_CLOUD:
								transtream.size = ms2m_pase_cloud(transtream.data, transtream.size);
								if(transtream.size > 0)
									ms_net_app_send(&transtream);
								break;
							case MS_UART_EVENT_REPORT_NOACK:
							case MS_UART_EVENT_REPORT_ACK:
							case MS_SYS_EVENT_TRANSDATA:
								eii_uart_put(transtream.data, transtream.size);
								break;
							case MS_EII_ASR_RESULT:
								MS_TRACE("ASR result: %s\n\n\n",transtream.data);
								ASR_process(transtream.data, transtream.size);
								break;
							case MS_EII_CLOUD_MSG:
								MS_TRACE("EII cmd: %02X\n",transtream.data[10]);
								switch(transtream.data[10]){
									case CMD_DEV_INFO_LIST:
										ms2m_dev_info_clean(&ms2m_devinfo_list, &devinfo_list_lock);
										ms2m_save_dev_info(transtream.data, transtream.size);
										break;
									default:
										break;
								}
								break;
							default:
								break;
						}
					}
				}
			}
		}
	}

	MS_TRACE("thread end, failure state\n");
	close(fd_eii_in);
}

#if 0
void test_ctrl(void)
{
	int data_len = 0;
	uint8_t *data = NULL;
	uint8_t uart_data[40];
	uint8_t msg_data[256];

	memset(uart_data, 0, 40);
	data = &uart_data[10];
	data[data_len++] = 0x01;
	data[data_len++] = 0xFA;
	uart_data[0] = 0xAA;
	uart_data[1] = data_len + 10;
	uart_data[2] = 0xAC;
	uart_data[6] = 0x21;
	uart_data[9] = 0x02;
	uart_data[uart_data[1]] = ms_uart_pro_check_sum(uart_data+1, uart_data[1]-1);
}
#endif

void ms2m_main()
{
#if 0
	int tmp = 0;
	int cnt = 0;
#endif
	//ms2m_hal_wlan_start_sta();
	//ms2m_hal_msleep(50);
	char coap_str[256] = {0};
	uint8_t * new_coap_str = NULL;
	uint16_t coap_len;
	int ret;
	uint8_t aIp[4];
	int count = 0;
	pthread_t read_fifo_tid;
	bool get_scene_flag = false;
	T_MsTransStream *resp = NULL;

	MS_OPEN_TRACE("msmart_eii");
	pthread_mutex_init(&m_lock, NULL);
	pthread_mutex_init(&send_lock, NULL);
	pthread_mutex_init(&devinfo_list_lock, NULL);
	pthread_mutex_init(&asr_ctrl_list_lock, NULL);
	pthread_mutex_init(&asr_lock, NULL);
	mkfifo(EII_OUT, 0777);
	mkfifo(EII_IN, 0777);
	sem_init(&m_sem, 0, 0);
	pthread_mutex_lock(&asr_lock);
	INIT_LIST_HEAD(&(asr_ctrl_info.asr_ctrl_list));
	asr_ctrl_info.asr_cmd = ASR_CMD_NULL;
	asr_ctrl_info.list_len = 0;
	pthread_mutex_unlock(&asr_lock);
	if(0 != (ret = pthread_create(&read_fifo_tid,NULL,read_fifo_routine,NULL))){
		MS_DBG_TRACE("read_fifo_routine pthread create fail\n");
		ms2m_hal_reboot();
	}

	while(!ms_hal_getIpAddr(aIp, MS_STA_POINT)){
		MS_DBG_TRACE("Net busy wait for IP address");
		sleep(2);
	}

	eii_config_t config;

	if(M_ERROR == ms2m_demo_uart_init())
	{
		MS2M_PRINTF("UART DEMO INIT ERROR");
		ms2m_hal_reboot();
	}
	ms2m_load_dev_info();

	memset(&config,0,sizeof(eii_config_t));
	config.base_addr = MS2M_HAL_BASEADDR;
	config.script_block_size = MS2M_HAL_SCRIPT_SIZE;
	config.when_block_size = MS2M_HAL_LKG_WHEN_SIZE;
	config.then_block_size = MS2M_HAL_LKG_THEN_SIZE;
	eii_cloud_cb_set(eii_demo_cb);

	if(eii_init(&config) == M_ERROR)
	{
		MS_ERR_TRACE("EII init error");
		ms2m_hal_reboot();
	}

	eii_cloud_dev_enable(0);
	eii_security_random_reset();

	ret = ms_get_dev_position(coap_str, 256);
	if(ret != M_ERROR){
		ret = eii_dev_position_set(coap_str, strlen(coap_str));
		MS_TRACE("set pos:%s, ret:%d",coap_str, ret);
	}
	MS_TRACE("enter loop eii_process()");
	while(!m_eii_lua_dl_fail)
	{
		eii_process();
		ms2m_hal_msleep(100);
		if(msmart_cloud_is_login_success() &&
				get_scene_flag == false){
			MS_TRACE("Get scene and pos.");
			//登陆云后刷新全部场景并获取位置，仅执行一次，后续云端Save请求要考虑Flash的寿命问题
			get_scene_flag = true;
			eii_cloud_lkg_check(0);//关闭场景检验
			new_coap_str = eii_cloud_linkage_request_package(&coap_len);
			ms2m_upload_data(MS2M_CLOUD_M2N_REQUEST_SCENE_OPT, 0, 0, new_coap_str, coap_len, NULL);

			//刷新位置信息
			new_coap_str = eii_cloud_position_request_package(&coap_len);
			ms2m_upload_data(MS2M_CLOUD_M2N_REQUEST_LOCATION, 0, 1, new_coap_str, coap_len, &resp);
			if(resp != NULL){
				ms_store_dev_position(&resp->data[2], strlen(&resp->data[2]));
				ret = eii_dev_position_set(&resp->data[2], strlen(&resp->data[2]));
				MS_TRACE("set pos:%s, ret:%d",&resp->data[2], ret);
				free(resp);
			}
		}
	}
	sem_destroy(&m_sem);
	pthread_mutex_destroy(&m_lock);
	pthread_mutex_destroy(&send_lock);
	pthread_mutex_destroy(&devinfo_list_lock);
	pthread_mutex_destroy(&asr_ctrl_list_lock);
	pthread_mutex_destroy(&asr_lock);
	ms2m_dev_info_clean(&ms2m_devinfo_list, &devinfo_list_lock);
	ms2m_dev_info_clean(&(asr_ctrl_info.asr_ctrl_list), &asr_ctrl_list_lock);
	MS_CLOSE_TRACE();
}
