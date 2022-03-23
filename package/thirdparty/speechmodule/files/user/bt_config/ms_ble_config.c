/**
 * @file msm_ble_adapter.c
 * @brief M-Smart BLE Demo
 * @function ÊµÏÖM-Smart BLEÅäÍø¼°µÇÂ½M-SmartÔÆ
 * @author Ð»½¨¾ü/Ò¶³þºº
 * @version 1.0.0
 * @par Copyright  (c)
 *		Midea
 */

#include <unistd.h>

#include "ms_common.h"

#include "ms_ble_app.h"
#include "bluetooth.h"
#include "msm_ble_adapter.h"
#include "stdbool.h"
#include "wifi_config_api.c"        //CAUTION: INCLUDE .C FILE


int wifi_connect_count = 0;
int wifi_disconnect_count = 0;
unsigned char server_status_change_flag = false; // for ble test

bool bt_config_ok = false;

ms_stored_info_t	ms_stored_info;
int deamo_pid = 0;
#ifdef HARD_VAD
int bt_wakelock_fd = 0;
#endif

static int send_msg_to_mspeech_fifo(const T_MsTransStream *pInStream)
{
    int fd_wr   = -1;
    int iRet    = -1;

    if (-1 == (fd_wr = open(MAISPEECH_IN,  O_WRONLY | O_NONBLOCK)))
    {
        MS_ERR_TRACE("LOCAL_NET Process open /tmp/maipseech_in fail");
        return -1;
    }

    // Write to FIFO
    if ((write(fd_wr, (const void *)pInStream, sizeof(T_MsTransStream)) < 0))
    {
        MS_ERR_TRACE("LOCAL_NET Process write /tmp/maipseech_in fail");
        close(fd_wr);
        return -1;
    }

    iRet = close(fd_wr);

    return iRet;
}

int ms_ble_sdk_cb_to_user(ms_ble_event_frame_t *pframe)
{
	MS_BLE_DBG_TRACE("sdk->user");
	switch(pframe->event)
	{
		case MS_BLE_EVENT_CONFIG://get config feedback
		{
			ms_op_code_t cb_code = (ms_op_code_t)pframe->data[0];
			switch(cb_code)//which kind of result
			{
				case MS_OP_CONFIG_SUCCESS://get ssid and pwd
				{
					ms_ble_config_ret_cb_t *ret_info = NULL;
					ret_info = pframe->data;

					if(-1==(msm_read_config(MS_STORED_INFO,(char*)&ms_stored_info,sizeof(ms_stored_info)))){
						MS_TRACE("Read info fail");
					}

					memset(ms_stored_info.ssid, 0, sizeof(ms_stored_info.ssid));
					memset(ms_stored_info.pwd, 0, sizeof(ms_stored_info.pwd));
					memcpy(ms_stored_info.ssid, ret_info->ssid, ret_info->ssid_len);
					ms_stored_info.ssid_len = ret_info->ssid_len;

					memcpy(ms_stored_info.pwd, ret_info->pwd, ret_info->pwd_len);
					ms_stored_info.pwd_len = ret_info->pwd_len;

					memcpy(ms_stored_info.random_num, ret_info->random, 16);

					ms_stored_info.boot_mode = MS_BOOT_MODE_STA;

					if(ret_info->pwd_len == 0){
						ms_stored_info.ak_security_type = CFGFILE_OPEN;
					}else{
						ms_stored_info.ak_security_type = FLAG_WPA2;
					}

					MS_BLE_TRACE("ble config done\r\n[TODO]need to save ssid and password");
					MS_BLE_TRACE("ssid: %s, password: %s", ms_stored_info.ssid, ms_stored_info.pwd);
					MS_BLE_TRACE("random_num:");

					PRINT_BUF(ms_stored_info.random_num, 16);

#if 0
                    T_MsTransStream tframe;
                    uint8_t aIp[4];

                    /*send play network connect start msg to mspeech fifo*/
                    memset(&tframe, 0, sizeof(tframe));
                    tframe.event = MS_LOCALNET_EVENT_NETWORK_SETTING;
                    strncpy((char *)tframe.data, "/oem/tts/do_connect.mp3", sizeof(tframe.data) - 1);
                    tframe.size = strlen((char *)tframe.data) + 1;
                    tframe.msg_id = 1;  // 1-play RIGHTNOW, 0-play SEQUENTIALLY
                    send_msg_to_mspeech_fifo(&tframe);
                    MS_DBG_TRACE("NETWORK CONNECT START, play url: /oem/tts/do_connect.mp3");
#endif

#if 0
                    MS_TRACE("event: soft-ap config %s[%d], %s[%d]", ms_stored_info.ssid, ms_stored_info.ssid_len, ms_stored_info.pwd, ms_stored_info.pwd_len);
                    wlan_sta_start(ms_stored_info.ssid, ms_stored_info.pwd,ms_stored_info.ak_security_type);

                    memset(aIp, 0, sizeof(aIp));
                    memset(&tframe, 0, sizeof(tframe));
                    if (1 == ms_hal_getIpAddr(aIp, MS_STA_POINT))
                    {
                        MS_TRACE("network config success, IP address:%d.%d.%d.%d, play url: /oem/tts/network_config_success.mp3", aIp[0], aIp[1], aIp[2], aIp[3]);
                        strncpy((char *)tframe.data, "/oem/tts/network_config_success.mp3", sizeof(tframe.data) - 1);
                    }
                    else
                    {
                        MS_TRACE("network config fail, IP address:%d.%d.%d.%d, play url: /oem/tts/network_config_fail.mp3", aIp[0], aIp[1], aIp[2], aIp[3]);
                        strncpy((char *)tframe.data, "/oem/tts/network_config_fail.mp3", sizeof(tframe.data) - 1);
                    }
                    /*seng play network connect success msg to mspeech fifo*/
                    tframe.event = MS_LOCALNET_EVENT_NETWORK_SETTING;
                    tframe.size = strlen((char *)tframe.data) + 1;
                    tframe.msg_id = 0;  // 1-play RIGHTNOW, 0-play SEQUENTIALLY
                    send_msg_to_mspeech_fifo(&tframe);
#endif

					if(-1 == msm_write_config(MS_STORED_INFO, (char *)&ms_stored_info, sizeof(ms_stored_info)))
					{
						MS_TRACE("stored info fail");
					}
					else
					{
						MS_TRACE("stored info ok");
					}

					bt_config_ok = true;
					if(deamo_pid > 0){
					    kill(deamo_pid,SIG_SYS_EVENT_WIFI_READY);
					}
					sleep(2);
                    wlan_sta_start(ms_stored_info.ssid,ms_stored_info.pwd,ms_stored_info.ak_security_type);
                    server_status_change_flag == true;
					break;
				}
				case MS_OP_CONFIG_TIMEOUT:
				{
					MS_BLE_TRACE("config timeout");
					break;
				}
				case MS_OP_EXCEPTION:
				{
					MS_BLE_ERR_TRACE("op exception");
					break;
				}
				default:
				{
					MS_BLE_TRACE("other op code");//suggest to retry
					break;
				}
			}
			break;
		}
		case MS_BLE_EVENT_TRANS:
		{
			unsigned char buf[260] = { 0 };
			unsigned char msgid = pframe->msgId;
			memcpy(buf, pframe->data, pframe->dataSize);

			PRINT_BUF(pframe->data, pframe->dataSize);
			MS_BLE_TRACE("[TODO]get ble trans through data, msgid = %d, len = %d", pframe->msgId, pframe->dataSize);//ble stack is powered on
			break;
		}

		case MS_BLE_EVENT_BLE_STACK_READY:
		{
			MS_BLE_TRACE("ble stack is ready[just notify]");
			break;
		}

		case MS_BLE_EVENT_CONNECTED:
		{
			MS_BLE_TRACE("phone connected[just notify]");//ble connection is established
			break;
		}

		case MS_BLE_EVENT_DISCONNECT:
		{
			MS_BLE_TRACE("phone disconnected[just notify]");//ble connection is shut down
			break;
		}

		case MS_BLE_EVENT_PT_READY:
		{
			MS_BLE_TRACE("pt ready[just notify]");
			break;
		}

		default:
		{
			break;
		}
	}

	return 0;
}

extern void msm_ble_stack_callback(ms_ble_stack_event_t event);
extern pthread_rwlock_t g_ring_lock;
int ms_demo_ms_ble()
{
	ms_ble_init_para_t init_para;
	ms_ble_event_frame_t frame;
	uint8_t		ip[4];

        memset(&ms_stored_info,0,sizeof(ms_stored_info));
	memset(&frame, 0, sizeof(frame));
	memset(&init_para, 0, sizeof(init_para));

	if(-1 == msm_read_config(MS_STORED_INFO, (char *)&ms_stored_info, sizeof(ms_stored_info)))
	{
		MS_ERR_TRACE("Local net read store info faild");
		return E_ERR_UNKNOWN;
	}

	memcpy(init_para.sn, ms_stored_info.sn, strlen(ms_stored_info.sn));


	init_para.ble_init_state = ENUM_MS_BLE_INIT_STATE_UNCONFIG_USER_AUTH;

	init_para.device_type = ms_stored_info.device_type[1];
	init_para.sub_device_type_low = ms_stored_info.device_type[2];

	init_para.cb = ms_ble_sdk_cb_to_user;
	init_para.cloud_logined_state = ENUM_MS_CLOUD_STATE_LOGOUT;
	uint8_t mac[6] = { 0 };

	memcpy(init_para.mac, mac, 6);

	//extern void ms_ble_debug();
	//ms_ble_debug();
	//vTaskDelay(1000*60*1);
	//MS_BLE_DBG_TRACE("================ sleep for 1min debug....");

	pthread_rwlock_init(&g_ring_lock, NULL);

    if(MSM_RESULT_ERROR == ms_ble_app_init(&init_para))
	{
		MS_BLE_ERR_TRACE("ms ble app init error");
		while(1)
			;
	}
#ifdef HARD_VAD
	ms_sent_wakelock_event(bt_wakelock_fd,MS_UART_EVENT_WAKE_DISABLE);
#endif
	bluetooth_control(BLE_OPEN_SERVER,NULL,0);

	int x=0;
	while(1)
	{
		ms_ble_app_process();

//         if(true == bt_config_ok){
//
//		 bluetooth_control(BLE_CLOSE_SERVER,NULL,0);
//		 sleep(1);
//		 break;
//         }

         if(-1 == msm_read_config(MS_STORED_INFO, (char *)&ms_stored_info, sizeof(ms_stored_info)))
         {
                MS_ERR_TRACE("Local net read store info faild");
                return E_ERR_UNKNOWN;
         }
         if((ms_hal_getIpAddr(ip, "wlan0"))&&(ms_stored_info.boot_mode == MS_BOOT_MODE_STA)){
		if(true == server_status_change_flag)
		{
			server_status_change_flag = false;

			ms_ble_wifi_info_t test_wifi_info;

			memset(&test_wifi_info, 0, sizeof(ms_ble_wifi_info_t));

			test_wifi_info.login_cloud_state = ENUM_LOGIN_CLOUD_STATE_LOGINING_CLOUD;
			#ifdef MS_ENABLE_ERRCODE
			test_wifi_info.login_cloud_state = ENUM_LOGIN_CLOUD_STATE_LOGINING_CLOUD;

			test_wifi_info.login_cloud_errcode = ENUM_LOGIN_CLOUD_ERR_HB_TIMEOUT;
			MS_BLE_TRACE("TEST................ in HB TIMEOUT .........");
			#else
			test_wifi_info.login_cloud_errcode = ENUM_LOGIN_CLOUD_ERR_SUCCESS;
			#endif

			frame.event = MS_BLE_EVENT_WIFI_STATE_CHANGE;
			memcpy(frame.data, (unsigned char*)&test_wifi_info, sizeof(ms_ble_wifi_info_t));
			frame.dataSize = sizeof(ms_ble_wifi_info_t);
			ms_ble_app_usr_msg_handle(&frame);
		}
			sleep(1);
			{
				char data[4]={0xff,0xff,0xff,0xff};
				bluetooth_control(BLE_SERVER_SEND,data,4);
			}
		sleep(1);

		bluetooth_control(BLE_CLOSE_SERVER,NULL,0);
		break;
         }else{
		 wifi_disconnect_count++;
		 if(wifi_disconnect_count>600){
			 bluetooth_control(BLE_CLOSE_SERVER,NULL,0);
			 if(-1==(msm_read_config(MS_STORED_INFO,(char*)&ms_stored_info,sizeof(ms_stored_info)))){
									MS_TRACE("Read info fail");
			 }
                         RK_SYSTEM("ifconfig wlan1 down");
	                 RK_SYSTEM("killall hostapd");
			 ms_stored_info.boot_mode = MS_BOOT_MODE_STA;
				 if(-1 == msm_write_config(MS_STORED_INFO, (char *)&ms_stored_info, sizeof(ms_stored_info)))
				 {
				     MS_TRACE("stored info fail");
				 }else{
					 MS_TRACE("stored info ok");
				 }
				 if(deamo_pid > 0){
			        kill(deamo_pid,SIG_SYS_EVENT_WIFI_READY);
				 }
			 sleep(1);
			 break;
		 }
         }
		sleep(1);
		MS_TRACE("%d\r\n", x++);
	}
}


int main(int argc,char **argv)
{
    if(argc == 2){
        sscanf(argv[1],"%d\n",&deamo_pid);
    }
    printf("deamo_pid:%d\n",deamo_pid);
    sleep(1);

    MS_TRACE("start ble demo ...");
#ifdef HARD_VAD
    bt_wakelock_fd = init_wakelock_client();
#endif
    ms_demo_ms_ble();
#ifdef HARD_VAD
    close_wakelock_client(bt_wakelock_fd);
#endif
    MS_TRACE("end ble demo ...");
}
