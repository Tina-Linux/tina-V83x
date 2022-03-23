#if defined(OSTYPE_LINUX)
#include <sys/msg.h>
#include <netdb.h>
#include <sys/time.h>
#endif

#include <ms_common.h>
#include <errno.h>
#include <netdb.h>
#include "msm_adapter.h"
#include "ms_net_app.h"
#include "ms_net_local_app.h"
#include "ms_socket.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int deamon_pid = 0;
/**
 * @brief     骞垮煙缃戞搷浣滃彞鏌�
 */
ST_HANDLE net_handle = INVALID_STHANDLE;
int fd_net_fifo_rd = -1;
ms_stored_info_t g_dev_info;


///TODO
#define CHECK_SDK

#ifdef MS_TEST
#define MS_TEST_SN		"0000B21110TPN36R5188231108350000"
#define MS_TEST_AP_SSID	"MS_TEST_SSID"
#define MS_TEST_AP_PWD  "12345678"
#endif

#ifdef HARD_VAD
    int wakelock_fd = 0 ;
#endif

#if defined(CHECK_SDK)
#define MS_TEST_PRODUCT_KEY		"n0UCsapKnRmbieWTIKWDAYtKiK7w7mQQ" //B1
//#define MS_TEST_PRODUCT_KEY		"BtjJglkcYSsjsVsCbCqO4cDNChxZ0EHE" //E2
//#define MS_TEST_PRODUCT_KEY		"BSxHJEObuZ0kkLl8tmt259RpoR2UGSsV" //AC
//#define MS_TEST_PRODUCT_KEY		"S24okKl7mtYdllcBk8tAMjTF9gnLPyYd" //FA
//#define MS_TEST_PRODUCT_KEY		"dTl3tZUSm36GBFfwSIS7r1xeC0oA4pR9" //FC 04

#endif


bool login_complete = false;
bool need_restart_main = false;
#define  BUF_LENGTH 10
typedef struct
{
	int timestamp;
	int tail;
	int head;
	int size;
	T_MsNetAppFrame msg[BUF_LENGTH];
} msg_queue;
msg_queue g_net_msg;

static void  uart_net_msg_handle(){
	int msgRecvBytes;
	T_MsTransStream inframe;
	T_MsNetAppFrame tframe;
	uint32_t net_msg_id = 0;
	int last_process_time = 0;
	int cur_time = 0;
	while(login_complete == true){
		cur_time = msm_timer_get_systime_ms();
#ifdef HARD_VAD
	ms_sent_wakelock_event(wakelock_fd,MS_UART_EVENT_WAKE_LOCK);
#endif

		if((g_net_msg.size >0)&&((net_msg_id == 0)||((cur_time-last_process_time)>800))){
			last_process_time = msm_timer_get_systime_ms();
		memset(&inframe,0,sizeof(inframe));
		MS_DBG_TRACE(" Net Queue head:%d, tail:%d, g_net_msg.size:%d",g_net_msg.head,g_net_msg.tail,g_net_msg.size);
		inframe.event = MS_SYS_EVENT_TRANSDATA;
		inframe.size = g_net_msg.msg[g_net_msg.tail].size;
		memcpy(inframe.data,g_net_msg.msg[g_net_msg.tail].data,g_net_msg.msg[g_net_msg.tail].size);
		net_msg_id =  g_net_msg.msg[g_net_msg.tail].msg_id;
		PRINT_BUF(inframe.data,inframe.size);
		write_state_to_uart_fifo(MSMART_NET_OUT,&inframe);

		g_net_msg.tail++;
		g_net_msg.size--;
		if(g_net_msg.tail>=BUF_LENGTH){
			g_net_msg.tail = 0;
		}
        }

        memset(&inframe,0,sizeof(inframe));
        memset(&tframe,0,sizeof(tframe));
        if((msgRecvBytes = read(fd_net_fifo_rd, &inframe, sizeof(inframe))) >0){

	        TRACE("Receive data from uart,inframe.size:%d",inframe.size);
	        printf("Receive data from uart,inframe.size:%d\n",inframe.size);
	        PRINT_BUF(inframe.data,inframe.size);

            if(inframe.event == MS_LOCALNET_EVENT_NETWORK_SETTING){
                need_restart_main = true;
            }

            tframe.size = inframe.size;
	        if(inframe.event == MS_UART_EVENT_REPORT_NOACK){
			tframe.event = ENUM_MS_NETAPP_EVENT_REPORT_NOACK;
		    }else if(inframe.event == MS_UART_EVENT_REPORT_ACK){
			tframe.event = ENUM_MS_NETAPP_EVENT_REPORT_ACK;
		    }else if(inframe.event == MS_SYS_EVENT_TRANSDATA){
			tframe.event = ENUM_MS_NETAPP_EVENT_TRANS;
				tframe.msg_id = net_msg_id;
				net_msg_id = 0;
		    }else{

		    }
            memcpy(tframe.data,inframe.data,sizeof(inframe.data));

	        MS_NetApp_Send(&net_handle,&tframe,(void*)0);
	    }
#ifdef HARD_VAD
	ms_sent_wakelock_event(wakelock_fd,MS_UART_EVENT_WAKE_UNLOCK);
#endif
	       if(g_net_msg.size<=0){
	            usleep(100000);
	        }
	}
}

 /**
  * \brief static int ms_cloud_event_cb(T_MsNetAppFrame *p_frame, void *args)
  * 濞村鐦悽銊ф畱閸ョ偠鐨熼崙鑺ユ殶閿涘苯褰叉禒銉﹀ⅵ閸楁澘鍤弨璺哄煂閻ㄥ嫭鏆熼幑顔兼姎缁鐎烽崪宀勬毐鎼达拷.
  * \param p_frame 娴滃娆㈤弫鐗堝祦鐢拷
  * \param args unused
  */
static int ms_cloud_event_cb(T_MsNetAppFrame *p_frame, void *args){
	ST_HANDLE owner = (ST_HANDLE)args;
    char *out_msmart = "/tmp/out_msmart";
	E_MsNetAppEvent event = p_frame->event;
	MS_TRACE("%p get event(0x%x:%ld)", owner, p_frame->event, p_frame->size);
	printf("%p get event(0x%x:%ld)\n", owner, p_frame->event, p_frame->size);
	PRINT_BUF(p_frame->data,p_frame->size);

	switch(event){
		//閻ц缍嶉張宥呭閸ｏ拷
	    case ENUM_MS_NETAPP_EVENT_LOGIN:

		login_complete = true;
		pthread_t uart_msg_handle_tid;
		if(0 !=  pthread_create(&uart_msg_handle_tid,NULL,(void*)&uart_net_msg_handle,NULL)){
			MS_ERR_TRACE("uart_down_msg pthread create fail");
			printf("uart_down_msg pthread create fail\n");
			exit(-1);
		}
            {
                ms_sys_status_t sys_status;
                uint8_t aIp[4];

                ms_stored_info_t temp_dev_info;
		if(-1 == msm_read_config(MS_STORED_INFO, (char *)&temp_dev_info, sizeof(temp_dev_info)))
		{
			MS_ERR_TRACE("Local net read store info faild");
			return E_ERR_UNKNOWN;
		}

                if (1 == ms_hal_getIpAddr(aIp, MS_STA_POINT)){
			 MS_DBG_TRACE("aquire IP success");
                }else{
			MS_DBG_TRACE("aquire IP fail");
                }
                memset(&sys_status,0,sizeof(sys_status));
                if(-1==(msm_read_config(MS_SYS_STATUS_PATH,(char*)&sys_status,sizeof(sys_status)))){
                    MS_TRACE("Read info fail");
                }
                sys_status.cloud_status = 0;
                sys_status.config_method = 0;
                sys_status.wifi_mode = 0x01;
                sys_status.module_type =0x01;
                sys_status.wifi_signal_level = ms_hal_getWifiSignal(temp_dev_info.ssid);;
                sys_status.multi_cloud_status = 0x00;
                sys_status.lan_status = 0x00;
                sys_status.multi_cloud_status = 0x01;
                sys_status.tcp_connected_number = 0;
                memcpy(sys_status.ip_v4,aIp,sizeof(aIp));

                system_state_change(MSMART_NET_OUT,&sys_status);
            }
		 /*\TODO: Application code*/
	         break;
	    //閺堝秴濮熼崳銊ь瀲缁撅拷
	    case ENUM_MS_NETAPP_EVENT_LOGOUT:
			login_complete = false;
			memset(&g_net_msg,0,sizeof(g_net_msg));
			{
				T_MsTransStream tframe;
				ms_sys_status_t sys_status;
                if(-1==(msm_read_config(MS_SYS_STATUS_PATH,(char*)&sys_status,sizeof(sys_status)))){
                    MS_TRACE("Read system info fail");
                }
                sys_status.multi_cloud_status = 0x00;

                if(-1 == msm_write_config(MS_SYS_STATUS_PATH, (char *)&sys_status, sizeof(sys_status)))
                {
                    MS_TRACE("stored system info fail");
                }
                tframe.event = MS_SYS_EVENT_SYS_STATUS_CHANGED;
                write_state_to_uart_fifo(MSMART_NET_OUT,&tframe);
			}
		 /*\TODO: Application code*/
	        break;
	     //閺堝秴濮熼崳銊ュ瀻闁板秷顔曟径鍢擠閿涘本娲块弬鎷岊啎婢跺様D
	    case ENUM_MS_NETAPP_EVENT_REFRESH_VID:
			if(-1 == msm_write_config(MS_DEVICE_ID, ms_get_device_id(), 6))
			{
				MS_TRACE("stored info fail");
				printf("stored info fail\n");
			}
		    TRACE("Note:\r\nTODO : refresh and save device id, print byte [low -> high]");
		    printf("Note:\r\nTODO : refresh and save device id, print byte [low -> high]\n");
			break;
		//閺佺増宓侀柅蹇庣炊
	    case ENUM_MS_NETAPP_EVENT_TRANS:
	        {
			if(g_net_msg.size < BUF_LENGTH){
				MS_DBG_TRACE("TRANS EVENT");
	                memset(&(g_net_msg.msg[g_net_msg.head]),0,sizeof(T_MsNetAppFrame));

	                g_net_msg.msg[g_net_msg.head].event = p_frame->event;
	                g_net_msg.msg[g_net_msg.head].msg_id = p_frame->msg_id;
	                g_net_msg.msg[g_net_msg.head].size = p_frame->size;
	                memcpy((g_net_msg.msg[g_net_msg.head].data),p_frame->data,p_frame->size);

	                g_net_msg.size++;
	                g_net_msg.head++;
	                if(g_net_msg.head>=BUF_LENGTH){
			    g_net_msg.head = 0;
	                }
			}
#ifdef HARD_VAD
			ms_sent_wakelock_event(wakelock_fd,MS_UART_EVENT_WAKE_ENABLE);
#endif
	        }
            break;
	   case ENUM_MS_NETAPP_EVENT_GET_SST_R3:
		{/// Important to do when ms_net_local_app is used
			if(0 == p_frame->data[0] && p_frame->size >= 65)
			{
				ms_dev_info_t dev_info;

				MS_TRACE("get r3");
				printf("get r3\n");
				///TODO
				///Step1: Init the dev_info
				memset(&dev_info, 0, sizeof(dev_info));
				dev_info.network_status = E_NETWORK_STATUS_STA;
				memcpy(dev_info.ssid, g_dev_info.ssid, strlen(g_dev_info.ssid));
				memcpy(dev_info.pwd, g_dev_info.pwd, strlen(g_dev_info.pwd));
				//Step2: Call the function
				ms_net_local_app_update_udp_key_id(p_frame->data + 1, &dev_info);
			}
			else
			{
				MS_ERR_TRACE("get wrong r3");
				printf("get wrong r3\n");
			}
		}
		break;
	    case ENUM_MS_NETAPP_EVENT_REBOOT:
		/*\TODO: Application code*/
	         break;
	    case ENUM_MS_NETAPP_EVENT_RESTORE:
		/*\TODO: Application code*/
	         break;
	    case ENUM_MS_NETAPP_EVENT_GET_TIME:
		/*\TODO: Application code*/
	         break;
	    case ENUM_MS_NETAPP_EVENT_REPORT_NOACK:
		/*\TODO: Application code*/
	         break;
	    case ENUM_MS_NETAPP_EVENT_REPORT_ACK:
		/*\TODO: Application code*/
	         break;
	    default:
		/*\TODO: Application code*/
		break;
	}
	return 0;
}

void ms_net_app_dead(int handle){
    printf("********************************************************net exit");
    if(INVALID_STHANDLE != net_handle){
        if(E_ERR_SUCCESS==MS_NetApp_Exit(&net_handle))
        {
            net_handle = INVALID_STHANDLE;
            TRACE("net app exit OK");
        }else
        {
           TRACE("net app exit fail");
        }
    }
    if(fd_net_fifo_rd > 0){
        close(fd_net_fifo_rd);
        fd_net_fifo_rd = -1;
    }
}


//add by 926 @20190625
static void print_debug_buf(char *name, char *buf, int buf_len) {
    printf("%s:\n", name);
    for (int i = 0; i < buf_len; i++) {
        printf("%#02x ", *(buf+i));
    }
    printf("\n");
}


/**
 * \brief static int ms_net_app(void) \n
 * 濞村鐦疢-Smart婢舵牜缍夐柈銊ュ瀻閻ㄥ嫭绁寸拠鏇炲毐閺侊拷.
 * \param void  閺冪姾绶崗銉ュ棘閺侊拷.
 * \return 鏉╂柨娲栧ù瀣槸閹笛嗩攽缂佹挻鐏夐敍灞惧灇閸旂喕绻戦崶锟�0閿涘矂鏁婄拠顖濈箲閸ョ偠绀嬮弫锟�.
 * \note 濮濄倕鍤遍弫棰佺窗闂冭顢�
 */
static int ms_net_app(void){
	E_Err ret = E_ERR_SUCCESS;
	ST_HANDLE  me = (void *)1, ops_handle = NULL;
    uint8_t aIp[4];

	int time_out = 60;
	uint32_t tm_ms;
	ms_cloud_init_info_t ms_cloud_init_info;
	T_MsTransStream inframe;
#if defined(MS_TEST)
	///TODO
	uint8_t		mac_addr[6] = {0x2C, 0x3A, 0xE8, 0x08, 0x10, 0xDA};
	char	random_num[16] = {0x01};
	char		device_type[4] = {0X00, 0XFA, 0X01, 0X00};
#endif
	//閻€劍绉烽幁顖炴Е閸掓膩閹风儺O

	signal(SIGINT, ms_net_app_dead);
    memset(&g_net_msg,0,sizeof(g_net_msg));
	memset(&g_dev_info, 0, sizeof(g_dev_info));
    CLR_STRUCT(&g_dev_info);
	memset(&ms_cloud_init_info, 0, sizeof(ms_cloud_init_info));
	memset(&inframe,0,sizeof(inframe));

	g_dev_info.boot_mode = 0xff;

	while(g_dev_info.boot_mode != MS_BOOT_MODE_STA){
	MS_DBG_TRACE("Wait for boot mode");
	printf("Wait for boot mode\n");
        if(-1 != msm_read_config(MS_STORED_INFO, (char *)&g_dev_info, sizeof(g_dev_info)))
        {
		MS_DBG_TRACE("Read boot mode");
	printf("Read boot mode\n");
        }else{
	    goto LAST;
        }
	sleep(2);
    }
    while(!ms_hal_getIpAddr(aIp, MS_STA_POINT)){
	MS_DBG_TRACE("Net busy wait for IP address");
	printf("Net busy wait for IP address\n");
	sleep(2);
    }
    MS_TRACE("IP address:%d.%d.%d.%d", aIp[3], aIp[2], aIp[1], aIp[0]);
    printf("IP address:%d.%d.%d.%d\n", aIp[3], aIp[2], aIp[1], aIp[0]);

    if(-1 != msm_read_config(MS_STORED_INFO, (char *)&g_dev_info, sizeof(g_dev_info)))
    {
        memcpy(ms_cloud_init_info.sn, g_dev_info.sn, sizeof(ms_cloud_init_info.sn));
        memcpy(ms_cloud_init_info.random_num, g_dev_info.random_num, sizeof(ms_cloud_init_info.random_num));
        memcpy(ms_cloud_init_info.device_type,g_dev_info.device_type, sizeof(ms_cloud_init_info.device_type));
        memcpy(ms_cloud_init_info.mac_addr, g_dev_info.mac_addr, sizeof(ms_cloud_init_info.mac_addr));
        MS_DBG_TRACE("The config SN:");

        //add by 926 @20190625
        //printf("The config mac:%s\n", ms_cloud_init_info.mac_addr);
        print_debug_buf("mac", ms_cloud_init_info.mac_addr, sizeof(ms_cloud_init_info.mac_addr));
        //printf("The config random num:%s\n", ms_cloud_init_info.random_num);
        print_debug_buf("random num", ms_cloud_init_info.random_num, sizeof(ms_cloud_init_info.random_num));
        //printf("The config product_key:%s\n", ms_cloud_init_info.product_key);
        print_debug_buf("product_key", ms_cloud_init_info.product_key, sizeof(ms_cloud_init_info.product_key));
        //printf("The config product_secret:%s\n", ms_cloud_init_info.product_secret);
        print_debug_buf("product_secret", ms_cloud_init_info.product_secret, sizeof(ms_cloud_init_info.product_secret));
        printf("The config SN:%s\n", ms_cloud_init_info.sn);
        /*
        for (int i=0; i<sizeof(ms_cloud_init_info.sn); i++) {
            printf("%#02x ", *(ms_cloud_init_info.sn + i));
        }
        printf("\n"); //add end
        */
        PRINT_BUF(ms_cloud_init_info.sn,sizeof(ms_cloud_init_info.sn));

        MS_DBG_TRACE("The config deviceType:");
        //add by 926 @20190625
        printf("The config deviceType:");
        for (int i=0; i<sizeof(ms_cloud_init_info.device_type); i++) {
            printf("%#02x ", *(ms_cloud_init_info.device_type + i));
        }
        printf("\n");//add end

        //*(ms_cloud_init_info.device_type + 0) = 0x00; //add by 926 @20190625
        //*(ms_cloud_init_info.device_type + 1) = 0xb1;
        //*(ms_cloud_init_info.device_type + 2) = 0x01;
        //*(ms_cloud_init_info.device_type + 3) = 0x00;
        printf("new config deviceType:");
        for (int i=0; i<sizeof(ms_cloud_init_info.device_type); i++) {
            printf("%#02x ", *(ms_cloud_init_info.device_type + i));
        }
        printf("\n");//add end

        PRINT_BUF(ms_cloud_init_info.device_type,sizeof(ms_cloud_init_info.device_type));
    }else{
#if defined(MS_TEST)
	    memcpy(ms_cloud_init_info.sn, MS_TEST_SN, sizeof(ms_cloud_init_info.sn));
	    memcpy(ms_cloud_init_info.random_num, random_num, sizeof(ms_cloud_init_info.random_num));
	    memcpy(ms_cloud_init_info.device_type, device_type, sizeof(ms_cloud_init_info.device_type));
	    memcpy(ms_cloud_init_info.mac_addr, mac_addr, sizeof(ms_cloud_init_info.mac_addr));
#else
	    goto LAST;
#endif
    }


	if(-1==(fd_net_fifo_rd = open(MSMART_NET_IN,  O_RDONLY|O_NONBLOCK))){
		ret = E_ERR_FD;
		MS_ERR_TRACE("NET Process open /tmp/msmart_net_in fail");
		printf("NET Process open /tmp/msmart_net_in fail\n");
		goto LAST;
	}

	//楠炲灝鐓欑純鎲噊cket閻ㄥ嫭鏁归崣鎴ｇТ閺冭埖妞傞梻锟�
	ms_cloud_init_info.timeout_ms = time_out;
	#if defined(CHECK_SDK)
	memcpy(ms_cloud_init_info.product_key, MS_TEST_PRODUCT_KEY, 32);
    print_debug_buf("new product_key", ms_cloud_init_info.product_key, sizeof(ms_cloud_init_info.product_key));
	#endif
	//閹稿洤鎮滃ù瀣槸閺堝秴濮熼崳顭掔礉鐠囥儲鐖ｇ拠鍡楀涧閺堝elease閻ㄥ嫬绨遍幍宥嗘箒閺佸牞绱滵ebug閻ㄥ嫬绨卞鍝勫煑娑撶儤瀵氶崥鎴炵ゴ鐠囨洘婀囬崝鈥虫珤
	ms_cloud_init_info.server_flag = SERVER_UNOFFICIAL;  //SERVER_UNOFFICIAL

	/*Init Network operation function*/
	ms_NetworkInit(&ops_handle);

    /*Init M-smart WAN object logic*/
	ret = MS_NetApp_Init(&net_handle,ms_cloud_event_cb, ops_handle, (void *)me, &ms_cloud_init_info);
	if(E_ERR_SUCCESS != ret){
		MS_TRACE("MS_NetApp_Init failed, return %d", ret);
		printf("MS_NetApp_Init failed, return %d\n", ret);
		return (-1);
	}

	///TODO : clear random_num
	memset(ms_cloud_init_info.random_num, 0, 16);

	while(1){
		tm_ms = msm_timer_get_systime_ms();

        if ((INVALID_STHANDLE == net_handle) || (fd_net_fifo_rd < 0)) {
            TRACE("INVALID HANDLE, EXIT!  net_handle:%d fd_net_fifo_rd:%d ", net_handle, fd_net_fifo_rd);
            printf("INVALID HANDLE, EXIT!  net_handle:%d fd_net_fifo_rd:%d\n", net_handle, fd_net_fifo_rd);
            break;
        }

		ret = MS_NetApp_Process(&net_handle, tm_ms);
		if(need_restart_main == true){
			break;
		}

//		uart_net_msg_handle();
//        if((read(fd_net_fifo_rd, &inframe, sizeof(inframe))) >0){
//            if(inframe.event == MS_LOCALNET_EVENT_NETWORK_SETTING){
//                break;
//            }
//	    }
		if(E_ERR_SUCCESS != ret)
		{
			MS_TRACE("MS_NetApp_Process failed, return %d", ret);
			printf("MS_NetApp_Process failed, return %d\n", ret);
			if(E_ERR_INVALID_PARAM == ret || E_ERR_NOAUTH == ret)
			{
				break;
			}
			else if(E_ERR_FD == ret)
			{
				msm_timer_delay_ms(10);
				goto LAST;
			}
		}

	}

LAST:
    MS_TRACE("MS_NetApp_Exit enter, return %d", ret);
    printf("MS_NetApp_Exit enter, return %d\n", ret);
    need_restart_main = false;
    login_complete =false;
    if (INVALID_STHANDLE != net_handle){
        if (E_ERR_SUCCESS == MS_NetApp_Exit(&net_handle))
        {
            net_handle = INVALID_STHANDLE;
            TRACE("net app exit OK");
            printf("net app exit OK\n");
        }else
        {
           TRACE("net app exit fail");
           printf("net app exit fail\n");
        }
    }

    if(fd_net_fifo_rd > 0) {
        close(fd_net_fifo_rd);
        fd_net_fifo_rd = -1;
    }

    return ret;
}


/**
 * @brief 楠炲灝鐓欑純鎱mple
 */
int main(int argc,char **argv){
	int ret = 0;
    if(argc == 2){
        sscanf(argv[1],"%d\n",&deamon_pid);
    }
    printf("msmart_net\n");

#ifdef HARD_VAD
    printf("msmart_net support HARD_VAD\n");
    wakelock_fd = init_wakelock_client();
#endif

    ms_sys_status_t sys_status;
    memset(&sys_status,0,sizeof(sys_status));
    sys_status.cloud_status = 1;
    sys_status.config_method = 0;
    sys_status.wifi_mode = 0x01;
    sys_status.module_type =0x01;
    sys_status.wifi_signal_level = 0;
    sys_status.multi_cloud_status = 0x00;
    sys_status.lan_status = 0x01;
    sys_status.multi_cloud_status = 0x00;
    sys_status.tcp_connected_number = 0;
    system_state_change(MSMART_NET_OUT,&sys_status);


	ret = ms_net_app();
    sleep(2);

#ifdef HARD_VAD
	close_wakelock_client(wakelock_fd);
#endif

	return ret;
}
