#if defined(OSTYPE_LINUX)
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#endif

#include "ms_socket.h"
#include "msm_adapter.h"
#include <ms_common.h>
#include "ms_net_local_app.h"
#include "wifi_config.h"

unsigned char ms_test_device_mac[6] = {0x2C, 0x3A, 0xE8, 0x08, 0x10, 0xDA};
#ifdef TEST_MAC
unsigned char ms_test_device_type[4] = {0x00,0xb1,0x01,0x00};
#define MS_TEST_SN              "0000B11110TVN50R61892100Z1230000"
#endif
/**
 * \brief 婵☆垱鐟ョ换鏃�鎷呭蹇曠闁绘稒褰冪欢閬嶆煀瀹ュ洦浼�闁告瑥鍟弳鈺冪磾閿燂拷?,闁糕晝鏌夐、鎲乻_net_local_app_exit闁挎稑鐗炵槐姘舵晬瀹�鍕舵嫹閿熶粙宕欓搹鍏夋煠閹艰揪鎷�
 */

int g_cur_id;
int deamo_pid = 0;
int local_msgid = 0;
ST_HANDLE local_net_handle = INVALID_STHANDLE;
int fd_local_net_fifo_rd = -1;
bool config_ok = false;
int g_client_socket_fd;
bool client_login_complete = false;

#ifdef HARD_VAD
    int wakelock_fd = 0;
#endif

#define  BUF_LENGTH 10
typedef struct
{
	int timestamp;
	int tail;
	int head;
	int size;
	T_MsNetLocalAppFrame msg[BUF_LENGTH];
} msg_queue;

msg_queue g_local_msg;

static void  uart_localnet_msg_handle(){
	int msgRecvBytes;
	T_MsTransStream inframe;
    T_MsNetLocalAppFrame tframe;
	uint32_t msg_id = 0;
	int last_process_time = 0;
	int cur_time = 0;
	while(client_login_complete == true){
		cur_time = msm_timer_get_systime_ms();
#ifdef HARD_VAD
	ms_sent_wakelock_event(wakelock_fd,MS_UART_EVENT_WAKE_LOCK);
#endif
        if((g_local_msg.size >0)&&((msg_id == 0)||(cur_time -last_process_time)>800)){
		last_process_time = msm_timer_get_systime_ms();
		memset(&inframe,0,sizeof(inframe));
		MS_DBG_TRACE(" Local Net Queue head:%d, tail:%d, size:%d",g_local_msg.head,g_local_msg.tail,g_local_msg.size);
		msg_id = g_local_msg.msg[g_local_msg.tail].msg_id;
		memcpy(inframe.data,g_local_msg.msg[g_local_msg.tail].data,sizeof(inframe.data));
		inframe.event = MS_SYS_EVENT_TRANSDATA;
		inframe.size = g_local_msg.msg[g_local_msg.tail].size;
		g_client_socket_fd = g_local_msg.msg[g_local_msg.tail].fd;

		MS_DBG_TRACE("uart_localnet_msg_handle,msg down to uart");

		write_state_to_uart_fifo(MSMART_LOCAL_NET_OUT,&inframe);

		g_local_msg.tail++;
		g_local_msg.size--;

		if(g_local_msg.tail>=BUF_LENGTH){
			g_local_msg.tail = 0;
		}
        }

        memset(&inframe,0,sizeof(inframe));
        memset(&tframe,0,sizeof(tframe));
        if((msgRecvBytes = read(fd_local_net_fifo_rd, &inframe, sizeof(inframe))) >0){
	        TRACE("Receive data from uart,inframe.size:%d",inframe.size);
	        printf("Receive data from uart,inframe.size:%d\n",inframe.size);
	        PRINT_BUF(inframe.data,inframe.size);

		    tframe.size = inframe.size;
	        if(inframe.event == MS_UART_EVENT_REPORT_NOACK){
			tframe.event = ENUM_MS_NETLOCALAPP_EVENT_REPORT_NOACK;

	        }else if(inframe.event == MS_UART_EVENT_REPORT_ACK){
			tframe.event = ENUM_MS_NETLOCALAPP_EVENT_REPORT_ACK;
	        }else if(inframe.event == MS_SYS_EVENT_TRANSDATA){
			tframe.event = ENUM_MS_NETLOCALAPP_EVENT_TRANS;
				tframe.msg_id = msg_id;
				msg_id = 0;
	        }else{

	        }

	        tframe.fd = g_client_socket_fd;
            memcpy(tframe.data,inframe.data,sizeof(inframe.data));

            ms_net_local_app_send(&local_net_handle, &tframe, (void*)0);
	    }
#ifdef HARD_VAD
	ms_sent_wakelock_event(wakelock_fd,MS_UART_EVENT_WAKE_UNLOCK);
#endif
        if(g_local_msg.size<=0){
            usleep(10000);
        }
	}
}


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


E_Err ms_dev_info_init(ms_dev_info_t *p_dev_info)
{
	int loop = 0;
	int temp_ip[4];
	char ap_ssid[15];
    T_MsTransStream tframe;
    uint8_t aIp[4];
	ms_stored_info_t temp_dev_info;

	memset(&temp_dev_info, 0, sizeof(temp_dev_info));

	//濡絾鐗曢崢娑氭嫚鐠囨彃绲块柡鍕靛灠閹線寮垫径濠勬憼闁稿被鍔忛惌楣冩偨閸欐ü绻嗛柟顓у灲缁辨繈寮垫径濠傜仧閺夆晜绋戦崣鍞杢a婵☆垪锟藉磭纭�闁挎稑鑻幆渚�宕氬▎鎺旂ap婵☆垪锟藉磭纭�
	if(-1 == msm_read_config(MS_STORED_INFO, (char *)&temp_dev_info, sizeof(temp_dev_info)))
	{
		MS_ERR_TRACE("Local net read store info faild");
		return E_ERR_UNKNOWN;
	}

    MS_DBG_TRACE("BOOT MODE: %d ", temp_dev_info.boot_mode);
    printf("BOOT MODE(0-sta,1-idle,2-ap): %d\n", temp_dev_info.boot_mode);
	switch(temp_dev_info.boot_mode){
	case MS_BOOT_MODE_CONFIG_MS_SNIFFER:
		break;
	    case MS_BOOT_MODE_CONFIG_MS_BT:
	    case MS_BOOT_MODE_CONFIG_MIX_SNIFFER:
	    case MS_BOOT_MODE_CONFIG_MS_AP:
	    {
#ifdef HARD_VAD
		ms_sent_wakelock_event(wakelock_fd,MS_UART_EVENT_WAKE_DISABLE);
#endif
		    p_dev_info->network_status = E_NETWORK_STATUS_AP;
		    break;
	    }
		case MS_BOOT_MODE_IDLE:
	    case MS_BOOT_MODE_STA:
	    {
		MS_DBG_TRACE("ENTER STA MODE:%s",temp_dev_info.ssid);
		printf("ENTER STA MODE:%s\n",temp_dev_info.ssid);

		    p_dev_info->network_status = E_NETWORK_STATUS_STA;
		    if((0 != temp_dev_info.ssid_len)){
#if 0
                /*send play network connect start msg to mspeech fifo*/
                memset(&tframe, 0, sizeof(tframe));
                tframe.event = MS_LOCALNET_EVENT_NETWORK_SETTING;
                strncpy((char *)tframe.data, "/oem/tts/do_connect.mp3", sizeof(tframe.data) - 1);
                tframe.size = strlen((char *)tframe.data) + 1;
                tframe.msg_id = 1;  // 1-play RIGHTNOW, 0-play SEQUENTIALLY
                send_msg_to_mspeech_fifo(&tframe);
                MS_DBG_TRACE("NETWORK CONNECT START, play url: /oem/tts/do_connect.mp3");
#endif

#if 1
                memset(aIp, 0, sizeof(aIp));
                while (loop < 80)
                {
                    if (1 == ms_hal_getIpAddr(aIp, MS_STA_POINT))
                    {
                        MS_DBG_TRACE("get STA Ipsuccess, IP: %d.%d.%d.%d", aIp[0], aIp[1], aIp[2], aIp[3]);
                        printf("get STA Ipsuccess, IP: %d.%d.%d.%d\n", aIp[0], aIp[1], aIp[2], aIp[3]);
                        loop = 0;
                        break;
                    }

                    MS_DBG_TRACE("wait STA Ip: %d", loop++);
                    printf("wait STA Ip: %d\n", loop++);
                     sleep(1);
                }
#endif
	        if(-1 == msm_read_config(MS_DEVICE_ID, p_dev_info->device_id, 6))
				{
					MS_TRACE("read deviceID info fail,use temp deviceID:");
					printf("read deviceID info fail,use temp deviceID:\n");
				}
                memset(aIp, 0, sizeof(aIp));
				memset(&tframe, 0, sizeof(tframe));
                if (1 == ms_hal_getIpAddr(aIp, MS_STA_POINT))
                {
                    MS_DBG_TRACE("NETWORK CONNECT success, IP address:%d.%d.%d.%d", aIp[0], aIp[1], aIp[2], aIp[3]);
                    printf("NETWORK CONNECT success, IP address:%d.%d.%d.%d\n", aIp[0], aIp[1], aIp[2], aIp[3]);
                    //strncpy((char *)tframe.data, "/oem/tts/network_config_success.mp3", sizeof(tframe.data) - 1);
                    //delete by 926 @20190619
                    strncpy((char *)tframe.data, "/mnt/app/tts/network_config_success.mp3", sizeof(tframe.data) - 1);
                }
                else
                {
                    MS_ERR_TRACE("NETWORK CONNECT fail, IP address:%d.%d.%d.%d", aIp[0], aIp[1], aIp[2], aIp[3]);
                    printf("NETWORK CONNECT fail, IP address:%d.%d.%d.%d\n", aIp[0], aIp[1], aIp[2], aIp[3]);
                    //strncpy((char *)tframe.data, "/oem/tts/network_config_fail.mp3", sizeof(tframe.data) - 1);
                    //delete by 926 @20190619
                    strncpy((char *)tframe.data, "/mnt/app/tts/network_config_fail.mp3", sizeof(tframe.data) - 1);
                }
#if 0
                /*send play network connect success msg to mspeech fifo*/
                MS_DBG_TRACE("NETWORK CONNECT play url: %s", tframe.data);
                tframe.event = MS_LOCALNET_EVENT_NETWORK_SETTING;
                tframe.size = strlen((char *)tframe.data) + 1;
                tframe.msg_id = 0;  // 1-play RIGHTNOW, 0-play SEQUENTIALLY
                send_msg_to_mspeech_fifo(&tframe);
#endif

		        //sleep(1);
                memcpy(p_dev_info->ip,aIp,sizeof(aIp));
		memcpy(p_dev_info->random, temp_dev_info.random_num, sizeof(temp_dev_info.random_num));
                memcpy(p_dev_info->ssid, temp_dev_info.ssid, sizeof(p_dev_info->ssid));
                memcpy(p_dev_info->pwd, temp_dev_info.pwd, sizeof(p_dev_info->pwd));
		    }
		    break;
	    }
        default:
        {
            MS_DBG_TRACE("no boot_mode config, ENTER IDEL MODE!");
            p_dev_info->network_status = E_NETWORK_STATUS_IDLE;
            break;
        }
	}
    if((temp_dev_info.sn[4] == 0)&&((temp_dev_info.sn[5] == 0))){
	return E_ERR_UNKNOWN;
    }

	memcpy(p_dev_info->device_type,temp_dev_info.device_type,sizeof(temp_dev_info.device_type));
	memcpy(p_dev_info->sn,temp_dev_info.sn,sizeof(temp_dev_info.sn));
//	memcpy(p_dev_info->mac_addr,temp_dev_info.mac_addr,sizeof(temp_dev_info.mac_addr));
	ms_hal_wlan_get_mac_address(p_dev_info->mac_addr);
#ifdef TEST_MAC
        p_dev_info->network_status = E_NETWORK_STATUS_AP;
        memcpy(p_dev_info->sn, MS_TEST_SN, sizeof(p_dev_info->sn));
        memcpy(p_dev_info->device_type, ms_test_device_type, sizeof(ms_test_device_type));
        memcpy(p_dev_info->mac_addr, ms_test_device_mac, sizeof(ms_test_device_mac));
        memcpy(p_dev_info->device_id,"123456", 6);
#endif

	if(p_dev_info->network_status == E_NETWORK_STATUS_AP)
	{
	    memset(p_dev_info->device_id,0,sizeof(p_dev_info->device_id));
		ms_sys_api_get_ap_ssid((unsigned char *)ap_ssid,p_dev_info);
	    wlan_accesspoint_start(ap_ssid,"12345678");
		TRACE("softap ssid: %s", ap_ssid);
		//while(!ms_hal_getIpAddr(p_dev_info->ip, MS_SOFTAP_POINT)){  //delete by 926 @20190625 wlan1 -> wlan0
		while(!ms_hal_getIpAddr(p_dev_info->ip, MS_STA_POINT)){
			MS_TRACE("Localnet busy wait for AP Ip Address");
			printf("Localnet busy wait for AP Ip Address\n");
			sleep(1);
		}
	}

	MS_DBG_TRACE("SN:");
	printf("SN:");
    for (int i=0; i<sizeof(temp_dev_info.sn); i++) {
        printf("%#02x ", *(p_dev_info->sn + i));
    }
    printf("\n");
    PRINT_BUF(p_dev_info->sn,sizeof(temp_dev_info.sn));

    MS_DBG_TRACE("device type:");
    printf("device type:");
    for (int i=0; i<sizeof(temp_dev_info.device_type); i++) {
        printf("%#02x ", *(p_dev_info->device_type + i));
    }
    printf("\n");
    PRINT_BUF(p_dev_info->device_type,sizeof(temp_dev_info.device_type));

	MS_TRACE("p_dev_info Mac address:");
	printf("p_dev_info Mac address:");
    for (int i=0; i<sizeof(p_dev_info->mac_addr); i++) {
        printf("%#02x ", *(p_dev_info->mac_addr + i));
    }
    printf("\n");
	PRINT_BUF(p_dev_info->mac_addr,sizeof(p_dev_info->mac_addr));

    MS_DBG_TRACE("device id:");
    printf("device id:");
    for (int i=0; i<sizeof(temp_dev_info.device_id); i++) {
        printf("%#02x ", *(p_dev_info->device_id + i));
    }
    printf("\n");
    PRINT_BUF(p_dev_info->device_id,sizeof(temp_dev_info.device_id));
	return E_ERR_SUCCESS;
}

/**
 * \brief E_Err ms_test_event_cb(T_MsNetLocalAppFrame *data) \n
 * 濞存嚎鍊撶花浼村矗閵夛妇鍔撮柨娑樼灱閺侀亶鎮介妸锕�鐓曢悗瑙勭煯閿燂拷?
 * \param data  閻忕儑鎷烽柛鈺冨枔缂嶅娼婚弬鎸庣濞存粌顑勫▎銏ゅ极閻楀牆绁﹂悽顖ゆ嫹
 */
E_Err ms_test_event_cb(T_MsNetLocalAppFrame *data)
{
    T_MsTransStream tframe;
    uint8_t aIp[4];

    memset(&tframe,0,sizeof(tframe));
	switch(data->event)
	{
		//閻忕儑鎷烽柛鈺冨枔缂嶅CP閺夆晝鍋炵敮鎾矗閹寸姵鏅搁柛娆惷敓锟�?
		case ENUM_MS_NETLOCALAPP_EVENT_CLIENT_NUM_CHANGE:
			{
				ms_client_num_change_t client_num;
				memcpy(&client_num, data->data, sizeof(client_num));
				TRACE("event:client num change:%d", client_num.alive_num);
				printf("event:client num change:%d\n", client_num.alive_num);
				if(client_num.alive_num > 0){
					g_client_socket_fd = data->fd;
					client_login_complete = true;
				pthread_t uart_msg_handle_tid;
				if(0 !=  pthread_create(&uart_msg_handle_tid,NULL,(void*)&uart_localnet_msg_handle,NULL)){
					MS_ERR_TRACE("uart_down_msg pthread create fail");
					printf("uart_down_msg pthread create fail\n");
					exit(-1);
				}
				}else{
					memset(&g_local_msg,0,sizeof(g_local_msg));
					client_login_complete = false;
				}
				{
					ms_sys_status_t sys_status;
	                memset(&sys_status,0,sizeof(sys_status));
	                if(-1==(msm_read_config(MS_SYS_STATUS_PATH,(char*)&sys_status,sizeof(sys_status)))){
	                    MS_TRACE("Read info fail");
			   printf("Read info fail\n");
	                }
	                sys_status.tcp_connected_number = client_num.alive_num;
	                system_state_change(MSMART_LOCAL_NET_OUT,&sys_status);
				}
				///TODO : need to notify to dev
			}
		    break;
		//閻忕儑鎷烽柛鈺冨枔缂嶅鏌呰箛搴ｇ倞
		case ENUM_MS_NETLOCALAPP_EVENT_TRANS:
                {
			if(g_local_msg.size < BUF_LENGTH){
				    MS_DBG_TRACE("TRANS Data ,zise:%d",data->size);
				    PRINT_BUF(data->data,data->size);
	                    memset(&(g_local_msg.msg[g_local_msg.head]),0,sizeof(T_MsNetLocalAppFrame));
	                    memcpy(&(g_local_msg.msg[g_local_msg.head]),data,sizeof(T_MsNetLocalAppFrame));
	                    g_local_msg.size++;
	                    g_local_msg.head ++;
	                    if(g_local_msg.head>=BUF_LENGTH){
			    g_local_msg.head = 0;
	                }
			}
#ifdef HARD_VAD
		        ms_sent_wakelock_event(wakelock_fd,MS_UART_EVENT_WAKE_ENABLE);
#endif
            }
	        break;
			//ap闂佹澘绉剁紞澶岋拷鐟版湰閸ㄦ岸鏁嶅畝鍐ㄧ闁告瑦鐗為惌楣冩偨閸欐ü绻嗛柟顓ㄦ嫹
		case ENUM_MS_NETLOCALAPP_EVENT_CONFIG:
			{
				ms_stored_info_t	ms_stored_info;
				ms_stored_info_t	*p_ms_stored_info = NULL;

				CLR_STRUCT(&ms_stored_info);

				#if defined(MS_DEBUG)
                MS_TRACE("dump...:%d",data->size);
				PRINT_BUF(data->data, data->size);
				#endif

				if(NULL == data->data)
					return E_ERR_UNKNOWN;

				p_ms_stored_info = (ms_stored_info_t *)data->data;
				if(-1==(msm_read_config(MS_STORED_INFO,(char*)&ms_stored_info,sizeof(ms_stored_info)))){
					MS_TRACE("Read info fail");
					printf("Read info fail\n");
				}
				ms_stored_info.boot_mode = MS_BOOT_MODE_STA;
				ms_stored_info.ssid_len = p_ms_stored_info->ssid_len;
				ms_stored_info.pwd_len = p_ms_stored_info->pwd_len;
				memcpy(ms_stored_info.random_num, p_ms_stored_info->random_num, sizeof(p_ms_stored_info->random_num));
                memcpy(ms_stored_info.device_id, p_ms_stored_info->device_id, sizeof(p_ms_stored_info->device_id));    //ATTENTION: SHOULD NOT SAVE device_id
				memset(ms_stored_info.ssid,0,sizeof(ms_stored_info.ssid));
				memset(ms_stored_info.pwd,0,sizeof(ms_stored_info.pwd));
				memcpy(ms_stored_info.ssid, p_ms_stored_info->ssid, ms_stored_info.ssid_len);
				memcpy(ms_stored_info.pwd, p_ms_stored_info->pwd, ms_stored_info.pwd_len);
				if(p_ms_stored_info->pwd_len == 0){
					ms_stored_info.ak_security_type = CFGFILE_OPEN;
				}else{
					ms_stored_info.ak_security_type = FLAG_WPA2;
				}

#if 0
                /*send play network connect start msg to mspeech fifo*/
                memset(&tframe, 0, sizeof(tframe));
                tframe.event = MS_LOCALNET_EVENT_NETWORK_SETTING;
                //strncpy((char *)tframe.data, "/oem/tts/do_connect.mp3", sizeof(tframe.data) - 1); //delete by 926 @20190619
                strncpy((char *)tframe.data, "/mnt/app/tts/do_connect.mp3", sizeof(tframe.data) - 1);
                tframe.size = strlen((char *)tframe.data) + 1;
                tframe.msg_id = 1;  // 1-play RIGHTNOW, 0-play SEQUENTIALLY
                send_msg_to_mspeech_fifo(&tframe);
                //MS_DBG_TRACE("NETWORK CONNECT START, play url: /oem/tts/do_connect.mp3"); //delete by 926 @20190619
                MS_DBG_TRACE("NETWORK CONNECT START, play url: /mnt/app/tts/do_connect.mp3");
                printf("NETWORK CONNECT START, play url: /mnt/app/tts/do_connect.mp3\n");
#endif

#if 0
                MS_TRACE("event: soft-ap config %s[%d], %s[%d]", ms_stored_info.ssid, ms_stored_info.ssid_len, ms_stored_info.pwd, ms_stored_info.pwd_len);
                wlan_sta_start(ms_stored_info.ssid, ms_stored_info.pwd,temp_dev_info.ak_security_type);

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
                /*send play network connect success msg to mspeech fifo*/
                tframe.event = MS_LOCALNET_EVENT_NETWORK_SETTING;
                tframe.size = strlen((char *)tframe.data) + 1;
                tframe.msg_id = 0;  // 1-play RIGHTNOW, 0-play SEQUENTIALLY
                send_msg_to_mspeech_fifo(&tframe);
#endif

				///TODO : switch to sta mode and reboot

				if(-1 == msm_write_config(MS_STORED_INFO, (char *)&ms_stored_info, sizeof(ms_stored_info)))
				{
					MS_TRACE("stored info fail");
					printf("stored info fail\n");
				}
				else
				{
					MS_TRACE("stored info ok");
					printf("stored info ok\n");
				}
				config_ok = true;
			}
		break;
		case ENUM_MS_NETLOCALAPP_EVENT_REPORT_ACK:
			TRACE("event:msg report ack, need to send to device");
			printf("event:msg report ack, need to send to device\n");
			break;
		default:
			break;
	}
	return 0;
}
void ms_net_local_app_dead(int handle){
    printf("********************************************************net_locla_app exit");
    if(INVALID_STHANDLE != local_net_handle){
        if(E_ERR_SUCCESS==ms_net_local_app_exit(&local_net_handle))
        {
            local_net_handle = INVALID_STHANDLE;
            TRACE("net local app exit OK");
            printf("net local app exit OK\n");
        }else
        {
            TRACE("net local app exit fail");
            printf("net local app exit fail\n");
        }
    }

    if(fd_local_net_fifo_rd > 0) {
        close(fd_local_net_fifo_rd);
        fd_local_net_fifo_rd = -1;
    }
    exit(0);
}
/**
 * \brief int ms_net_local_app(void) \n
 * 婵炴潙顑堥惁鐤�-Smart闂佹澘绉剁紞澶愬矗婵犲倸鏁剁紓鍐╁灴閸庢挳宕氶崱娆愮暠婵炴潙顑堥惁顖炲礄閼恒儲娈�.
 * \param void  闁哄啰濮剧欢顓㈠礂閵夈儱妫橀柡渚婃嫹.
 * \return 閺夆晜鏌ㄥú鏍圭�ｎ厾妲搁柟绗涘棭鏀界紓浣规尰閻忓鏁嶇仦鎯х亣闁告梻鍠曠换鎴﹀炊閿燂拷?闁挎稑鐭傞弫濠勬嫚椤栨繄绠查柛銉у仩缁�瀣极閿燂拷?
 */

int ms_net_local_app(void)
{
	E_Err ret;
	uint32_t tm_ms;
	ms_dev_info_t dev_info;
	ms_stored_info_t store_dev_info;
	ST_HANDLE ops_handle;
        int waitreply = 0;
	T_MsNetLocalAppFrame mframe;
    T_MsTransStream inframe;

	CLR_STRUCT(&dev_info);
    signal(SIGINT, ms_net_local_app_dead);

    memset(&g_local_msg,0,sizeof(g_local_msg));

    if(-1==(fd_local_net_fifo_rd = open(MSMART_LOCAL_NET_IN,  O_RDONLY|O_NONBLOCK))){
	    ret = E_ERR_FD;
	    MS_ERR_TRACE("NET Process open /tmp/msmart_local_net_in fail");
	    printf("NET Process open /tmp/msmart_local_net_in fail\n");
	    goto LAST;
    }

    if(E_ERR_SUCCESS !=(ret=ms_dev_info_init(&dev_info))){
		MS_ERR_TRACE("Init local net fail");
		printf("Init local net fail\n");
		goto LAST;
	}

//    while((dev_info.network_status == E_NETWORK_STATUS_IDLE || dev_info.network_status == E_NETWORK_STATUS_STA)&&(!ms_hal_getIpAddr(dev_info.ip, MS_STA_POINT))){
//	MS_DBG_TRACE("Localnet Busy wait for IP address");
//        if((read(fd_local_net_fifo_rd, &inframe, sizeof(inframe))) > 0){
//		    if(inframe.event == MS_LOCALNET_EVENT_NETWORK_SETTING){
//		        goto LAST;
//	        }
//        }
//        sleep(1);
//    }


    MS_TRACE("IP address:%d.%d.%d.%d",dev_info.ip[3],dev_info.ip[2],dev_info.ip[1],dev_info.ip[0]);
    printf("IP address:%d.%d.%d.%d\n",dev_info.ip[3],dev_info.ip[2],dev_info.ip[1],dev_info.ip[0]);

//	if(dev_info.network_status == E_NETWORK_STATUS_STA){
//
//		kill(deamo_pid,SIG_SYS_EVENT_WIFI_READY);
//	}
	ms_NetworkInit(&ops_handle);
	ret = ms_net_local_app_init(&local_net_handle, ms_test_event_cb, ops_handle, &dev_info);
	if(E_ERR_SUCCESS != ret)
	{
		TRACE("net local app init err, %d", ret);
		printf("net local app init err, %d\n", ret);
		return -1;
	}

	while(1)
	{
		tm_ms = msm_timer_get_systime_ms();

        if ((INVALID_STHANDLE == local_net_handle) || (fd_local_net_fifo_rd < 0)) {
            TRACE("INVALID HANDLE, EXIT!  local_net_handle:%d fd_local_net_fifo_rd:%d ", local_net_handle, fd_local_net_fifo_rd);
            printf("INVALID HANDLE, EXIT!  local_net_handle:%d fd_local_net_fifo_rd:%d\n", local_net_handle, fd_local_net_fifo_rd);
            break;
        }

		ret = ms_net_local_app_process(&local_net_handle, tm_ms);
		if(E_ERR_SUCCESS != ret)
		{
			TRACE("net local app process, %d", ret);
			printf("net local app process, %d\n", ret);
			break;
		}
        if(config_ok == true){
            waitreply++;
            if(waitreply == 10){
                waitreply = 0;
                msm_timer_delay_ms(10);
                break;
            }
        }
//		msm_timer_delay_ms(10);
	}

LAST:
    MS_TRACE("ms_net_local_app_exit enter, return %d", ret);
    printf("ms_net_local_app_exit enter, return %d\n", ret);
    client_login_complete = false;
    if(local_net_handle!=INVALID_STHANDLE){
        if(E_ERR_SUCCESS==(ret = ms_net_local_app_exit(&local_net_handle)))
        {
            local_net_handle = INVALID_STHANDLE;
            TRACE("net local app exit OK");
            printf("net local app exit OK\n");
        }else
        {
		TRACE("net local app exit fail");
		printf("net local app exit fail\n");
        }
    }
    if(fd_local_net_fifo_rd > 0)
    {
        close(fd_local_net_fifo_rd);
        fd_local_net_fifo_rd = -1;
    }

    if(config_ok == true){
		ms_stored_info_t temp_dev_info;
		memset(&temp_dev_info, 0, sizeof(temp_dev_info));
		if(-1 == msm_read_config(MS_STORED_INFO, (char *)&temp_dev_info, sizeof(temp_dev_info)))
		{
			MS_ERR_TRACE("Local net read store info faild");
			printf("Local net read store info faild\n");
			return E_ERR_UNKNOWN;
		}
         wlan_sta_start(temp_dev_info.ssid,temp_dev_info.pwd,temp_dev_info.ak_security_type);
         while(!ms_hal_getIpAddr(dev_info.ip, MS_STA_POINT)){
              sleep(1);
         }
         MS_TRACE("IP address:%d.%d.%d.%d",dev_info.ip[3],dev_info.ip[2],dev_info.ip[1],dev_info.ip[0]);
	     kill(deamo_pid,SIG_SYS_EVENT_WIFI_READY);
         config_ok = false;
    }
    return ret;
}

/**
 * @brief AP闂佹澘绉剁紞澶愭儍閸戠mple
 */
int main(int argc,char **argv)
{
    if(argc == 2){
        sscanf(argv[1],"%d\n",&deamo_pid);
    }
    printf("deamo_pid:%d\n",deamo_pid);
    sleep(1);

	int ret = 0;
#ifdef HARD_VAD
    wakelock_fd = init_wakelock_client();
#endif

    ret = ms_net_local_app();

#ifdef HARD_VAD
	close_wakelock_client(wakelock_fd);
#endif
	return ret;
}
