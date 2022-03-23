/*================================================================
*  Midea CRC speach module user interface demo.
*  Author: jiahui.xie
*  Date: 2018-08-21
================================================================*/
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/msg.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include "poll.h"
#include "m_type.h"
#include <signal.h>
#include "ms_common.h"
#include "AC_json.h"
#include "mcrc_speech_user_intf.h"

int deamon_pid = 0;
int speech_state = MSUI_STATE_INITIALIZING;

bool speech_led_is_sync = false;

bool main_voice_exit = false;
pthread_mutex_t main_voice_lock;
pthread_mutex_t ac_status_lock;
pthread_mutex_t mute_lock;
struct mcrc_speech_user user;

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

#ifdef HARD_VAD
   int wake_lock = 0;
   bool sleep_enable = true;
   pthread_rwlock_t wake_rwlock;
void updat_wakelock(){
       int socket_ser;
       E_MsDownStream wake_request;
       struct sockaddr_in address;
       bzero(&address, sizeof(address));
       address.sin_family = AF_INET;
       address.sin_addr.s_addr = htonl(INADDR_ANY);
       address.sin_port = htons(MS_UDP_WAKEUP_PORT);

       socket_ser = socket(AF_INET, SOCK_DGRAM, 0);
       bind(socket_ser, (struct sockaddr*)&address, sizeof(address));

       int num = 0;
       int add_len = sizeof(address);
       while(1){
           num = recvfrom(socket_ser, &wake_request, sizeof(wake_request), 0, (struct socketaddr*)&address, &add_len);
           MS_TRACE("************wake_request:%d\n",wake_request);
           pthread_rwlock_wrlock(&wake_rwlock);
	   switch(wake_request){
	   case MS_UART_EVENT_WAKE_LOCK:
		   MS_TRACE("************LOCK\n");
		   wake_lock++;
	       break;
	   case MS_UART_EVENT_WAKE_UNLOCK:
		MS_TRACE("************UNLOCK\n");
		   wake_lock--;
		   break;
	   case MS_UART_EVENT_WAKE_ENABLE:
		   MS_TRACE("************ENABLE\n");
		   sleep_enable = true;
		   break;
	   case MS_UART_EVENT_WAKE_DISABLE:
		   MS_TRACE("************DISABLE\n");
		   sleep_enable = false;
		   break;
	   default:
			  break;
	   }
	   pthread_rwlock_unlock(&wake_rwlock);
       }
       close(socket_ser);
       return ;
}
#endif


#if 0

 void *monitor_device_routine(void *user) {
    struct mcrc_speech_user *proc = (struct mcrc_speech_user *)user;
    struct epoll_event ev, events[10];
    int fd_home_in = -1;
	int epfd  =-1;
	int nfds = -1;
	int i;
    int ret;
	T_MsNetAppFrame transtream;
	MS_TRACE("monitor_device_routineddd\r\n");

	if(access(MAISPEECH_IN,F_OK)==-1){
		MS_TRACE("fifo file is empty\r\n");
	   ret=mkfifo(MAISPEECH_IN,0777);
	   if(ret!=0){
		   MS_TRACE("Could not create fifo %s\n",MAISPEECH_IN);
	   }
	}


  if(-1 == (fd_home_in = open(MAISPEECH_IN,O_RDONLY|O_NONBLOCK))){
	  MS_TRACE("/tmp/maipseech_in OPEN ERROR");
	  return NULL;
  }
  epfd = epoll_create(1);
  ev.data.fd=fd_home_in;
  ev.events=EPOLLIN|EPOLLERR;
  epoll_ctl(epfd,EPOLL_CTL_ADD,fd_home_in,&ev);
  MS_TRACE("monitor_device_routine\r\n");
    while (1) {
		nfds=epoll_wait(epfd,events,1,500);

			if( events[0].events&EPOLLIN ){
				//have data in
				if(-1 == (ret = read(fd_home_in, &transtream,sizeof(transtream)))){
					 MS_TRACE("READ DEVICE ERROR");
					 break;
				 }else{
					 //msg handle
					    unsigned char *data=malloc((transtream.size+1)*2);
					    if(data){
						    memset(data,0,(transtream.size+1)*2);

							HexToStr(data, transtream.data, transtream.size);


							if(proc->is_online_process ==0){
								cJSON *device=cJSON_CreateObject();
								cJSON * deviceinfo= cJSON_CreateObject();
								cJSON * status= cJSON_CreateObject();

								cJSON  *msg = cJSON_CreateObject();

								MS_TRACE("local:\n%s\n", data);
								cJSON_AddNumberToObject(deviceinfo, "deviceSubType",1);
								 cJSON_AddStringToObject(msg, "data",data);
								 cJSON_AddItemToObject(device,"deviceinfo",deviceinfo);
								 cJSON_AddItemToObject(device,"status",status);
								  cJSON_AddItemToObject(device,"msg",msg);
								  char *out = cJSON_Print(device);
									MS_TRACE("local:\n%s\n", out);

								char *result = data2json(out,strlen(out));
								MS_TRACE("result=%s\r\n",result);
								free(out);

							}
							else{
								//online msg handle
								int msui_ret = MSUI_CB_RET_SUCCESS;
					            struct speech_usrdata msui_data;
					            msui_data.event = MSUI_EV_IN_DEVICE_DATA;
					            strcpy(msui_data.out_res, ret);

					            msui_ret = proc->_handler(&msui_data);
							}



							free(data);
					    }

				 }


			}
			else if( events[0].events&EPOLLERR ){
				//have err pol

			}
			else {
				//unkown data
			}

        usleep(20 * 1000);
    }

    close(epfd);
	close(fd_home_in);
	return NULL;
}
int monitor_device_init(struct mcrc_speech_user *proc)
{
   int ret=-1;

		 ret = pthread_create(&proc->devicemonitor_tid, NULL, monitor_device_routine, proc);
	   if (ret < 0){
		    MS_TRACE("creat pthread err\r\n");
		    return -1;
		}

	   proc->running = 1;


    return ret;
}
#endif
static void StrToHex(char   *pbDest, char *pszSrc, int nLen)
{
	char h1, h2;
	char s1, s2;
	for (int i = 0; i < nLen; i++)
	{
		h1 = pszSrc[2 * i];
		h2 = pszSrc[2 * i + 1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
			s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;

		pbDest[i] = s1 * 16 + s2;
	}
}

void HexToStr(char *pszDest, char *pbSrc, int nLen)
{
	char	ddl, ddh;
	for (int i = 0; i < nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pszDest[i * 2] = ddh;
		pszDest[i * 2 + 1] = ddl;
	}

	pszDest[nLen * 2] = '\0';
}

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


int device_0xB0_0xB1_process(uint8_t *buff, struct mcrc_speech_user *user)
{
	int i;
	int ret = 0;
	int property_num = buff[11];
	uint8_t *p = &buff[12];
	for(i=0; i<property_num; i++){
		if(p[0] == 0x20 && p[1] == 0x0){
			ret = 1;
			switch(p[4]){
                printf("\n\n !!!!delete by 926 just for debug !!\n\n");
            #if 0 //delete by 926 for debug ,should chang to raw
				case SPEECH_OFF:
					pthread_mutex_lock(&mute_lock);
					//midea_setSpeakerMute(true);
					//midea_setMicMute(user, true);
					pthread_mutex_unlock(&mute_lock);
					break;
				case SPEECH_CONTROL_ONLY:
					pthread_mutex_lock(&mute_lock);
					//midea_setSpeakerMute(true);
					//midea_setMicMute(user, false);
					pthread_mutex_unlock(&mute_lock);
					break;
				case SPEECH_REPORT_ONLY:
					pthread_mutex_lock(&mute_lock);
					//midea_setSpeakerMute(false);
					//midea_setMicMute(user, true);
					pthread_mutex_unlock(&mute_lock);
					break;
				case SPEECH_ON:
					pthread_mutex_lock(&mute_lock);
					//midea_setSpeakerMute(false);
					//midea_setMicMute(user, false);
					pthread_mutex_unlock(&mute_lock);
					break;
                #endif
				default:
					break;
			}
			speech_led_is_sync = true;
		}
		p = p + p[3] + 4;
	}
	return ret;
}

int device_0xB5_process(uint8_t *buff, struct mcrc_speech_user *user)
{
	int i;
	int ret = 0;
	int property_num = buff[11];
	uint8_t *p = &buff[12];

	for(i=0; i<property_num; i++){
		if(p[0] == 0x20 && p[1] == 0x0){
			ret = 1;
			switch(p[3]){
                printf("\n\n !!!!delete by 926 just for debug !!\n\n");
                #if 0
				case SPEECH_OFF:
					pthread_mutex_lock(&mute_lock);
					//midea_setSpeakerMute(true);
					//midea_setMicMute(user, true);
					pthread_mutex_unlock(&mute_lock);
					break;
				case SPEECH_CONTROL_ONLY:
					pthread_mutex_lock(&mute_lock);
					//midea_setSpeakerMute(true);
					//midea_setMicMute(user, false);
					pthread_mutex_unlock(&mute_lock);
					break;
				case SPEECH_REPORT_ONLY:
					pthread_mutex_lock(&mute_lock);
					//midea_setSpeakerMute(false);
					//midea_setMicMute(user, true);
					pthread_mutex_unlock(&mute_lock);
					break;
				case SPEECH_ON:
					pthread_mutex_lock(&mute_lock);
					//midea_setSpeakerMute(false);
					//midea_setMicMute(user, false);
					pthread_mutex_unlock(&mute_lock);
					break;
                #endif
				default:
					break;
			}
			speech_led_is_sync = true;
		}
		p = p + p[2] + 3;
	}
	return ret;
}

void get_speech_led_status(void)
{
	int data_len = 0;
	uint8_t *data = NULL;
	uint8_t uart_data[40];
	uint8_t msg_data[256];

	memset(uart_data, 0, 40);
	data = &uart_data[10];
	data[data_len++] = 0xB1;
	data[data_len++] = 0x01;
	data[data_len++] = 0x20;
	data[data_len++] = 0x00;
	data[data_len++] = 0x00;
	data[data_len] = CRC8(data, data_len-1);
	uart_data[0] = 0xAA;
	uart_data[1] = data_len + 10;
	uart_data[2] = 0xAC;
	uart_data[9] = 0x03;
	uart_data[uart_data[1]] = ms_uart_pro_check_sum(uart_data+1, uart_data[1]-1);
	HexToStr(msg_data, uart_data, uart_data[1]+1);
	MS_TRACE("%s\n\n\n",msg_data);
	send_msg_to_uart(msg_data, strlen(msg_data), 0);
}

void speech_led_status_sync(void)
{
	while(1){
		get_speech_led_status();
		sleep(30);
	}
}
// get ip address
static int _get_local_ip(const char *eth_inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        MS_TRACE("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        MS_TRACE("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, 32/*sizeof(ip)*/, "%s", inet_ntoa(sin.sin_addr));
    MS_TRACE("[LJH]IP SIZE %d\n", sizeof(ip));

    close(sd);
    return 0;
}

int send_msg_to_uart(char *data, size_t len, int ack)
{
	int fd = -1;
	int ret;
	struct T_MsTransStream frame;
	int length = 0;
	char buf[256];

	fd = open(MAISPEECH_OUT, O_RDWR | O_NONBLOCK);
	if(fd < 0){
		MS_TRACE("open %s error(%s)\n", MAISPEECH_OUT, strerror(errno));
        close(fd);
		return fd;
	}

	memset(&frame, 0, sizeof(frame));
	MS_TRACE("len:%d data:%s\n", len, data);

	StrToHex(frame.data, data, len);
	frame.size = len / 2;
	if(ack){
		frame.event = MS_SYS_EVENT_TRANSDATA;
	}
	else {
		frame.event = MS_SYS_EVENT_TRANSDATA;
	}
	ret = write(fd, &frame, sizeof(frame));
	if(ret < 0){
		MS_TRACE("read message error(%s)\n", strerror(errno));
                close(fd);
		return ret;
	}

    close(fd);
	return ret;
}

static void * net_check_routine(void *user)
{
    char aIp[64];
    int iRet = -1;
    struct mcrc_speech_user *pUser = user;

    MS_TRACE("net_check_routine start ...");

    sleep(30);

    while (pUser->running)
    {
        memset(aIp, 0, sizeof(aIp));
        iRet = _get_local_ip("wlan0", aIp);
        if (0 != iRet)
        {
            MS_TRACE("GET IP ERROR, play url: /mnt/app/tts/network_config_start.mp3");
            //audio_player_play(speaker->aplayer, "/mnt/app/tts/network_config_start.mp3");
            mcrc_speech_play_request(NULL, "/mnt/app/tts/network_config_start.mp3", MSUI_PLAY_SEQUENTIALLY, 1, -1);

            //sleep(5);
            //continue;
        }
        MS_TRACE("GET IP : %s\n", aIp);

        break;
    }

    return NULL;
}


void *read_fifo_routine(void *user){

	int fd_maispeech_in = -1;
    struct mcrc_speech_user *puser=user;
	int retval;;
	int readcnt;
    int ne, nevents, ret, fd;
	struct T_MsTransStream  transtream;
	char data_buf[256];
	play_flag_t ePlayflag = MSUI_PLAY_SEQUENTIALLY;
    int mEpollFd = epoll_create(1);
    struct epoll_event events[1];
    struct epoll_event  ev;

	MS_TRACE("read fifo routine start ...");

        //memset(rd,0,sizeof(rd));
	if(access(MAISPEECH_IN,F_OK)==-1){
		MS_TRACE("fifo file is empty\r\n");
	   ret=mkfifo(MAISPEECH_IN,0777);
	   if(ret!=0){
		   MS_TRACE("Could not create fifo %s\n",MAISPEECH_IN);
	   }
	}

	if(-1 == (fd_maispeech_in = open(MAISPEECH_IN,O_RDWR |O_NONBLOCK))){
		MS_TRACE("/tmp/maipseech_in OPEN ERROR\n");
                close(fd_maispeech_in);
                return NULL;
		//break;
	}
	//fcntl(fd_maispeech_in, F_SETFL, fcntl(fd_maispeech_in, F_GETFL) | O_NONBLOCK);
	ev.events  = EPOLLIN | EPOLLET |EPOLLERR|EPOLLRDHUP;
	ev.data.fd = fd_maispeech_in;
	while (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd_maispeech_in, &ev) < 0 && errno == EINTR);

	while(1){
		nevents = epoll_wait(mEpollFd, events, 1, 200);  // 20ms timeout
		if (nevents < 0) {
			//if (errno != EINTR){
				MS_TRACE("epoll_wait() unexpected error: %s", strerror(errno));
			//}
			usleep(100*1000);
			continue;
		}

		for (ne = 0; ne < nevents; ne++) {
			if (events[ne].events & (EPOLLERR|EPOLLRDHUP)) {
				#if 0
				close(fd_maispeech_in);
				while (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd_maispeech_in, NULL) < 0 && errno == EINTR);
				if(-1 == (fd_maispeech_in = open(MAISPEECH_IN, O_RDONLY | O_NONBLOCK))){
					MS_ERR_TRACE("/tmp/msmart_net_out OPEN ERROR");
					break;
				}
				fcntl(fd_maispeech_in, F_SETFL, fcntl(fd_maispeech_in, F_GETFL) | O_NONBLOCK);
				ev.events  = EPOLLIN | EPOLLET |EPOLLERR|EPOLLRDHUP;
				ev.data.fd = fd_maispeech_in;
				while (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd_maispeech_in, &ev) < 0 && errno == EINTR);
				MS_TRACE("-----------%s--------\n", __FUNCTION__);
				#endif
			}
			else if(events[ne].events & EPOLLIN) {
				fd = events[ne].data.fd;
				if(fd == fd_maispeech_in) {
				if(-1 == (readcnt = read(fd, &transtream,sizeof(struct T_MsTransStream)))){
			MS_TRACE("READ DEVICE ERROR");
					//sleep(3);
			break;
		}
			else{
                  if (MS_LOCALNET_EVENT_NETWORK_SETTING == transtream.event)
                  {
                        ePlayflag = (1 == transtream.msg_id) ? MSUI_PLAY_LOCAL_RIGHTNOW : MSUI_PLAY_SEQUENTIALLY;
                        mcrc_speech_play_request(NULL, transtream.data /*"/mnt/app/tts/network_config_start.mp3"*/, MSUI_PLAY_LOCAL_RIGHTNOW, 1, -1);
                        MS_TRACE("network config begin, start play %s play flag(1-now 0-later):%d(%d)\n", transtream.data, transtream.msg_id, ePlayflag);

                        if (0 == strncmp((char *)transtream.data, "/mnt/app/tts/network_config_start.mp3", transtream.size  - 1))
                        {
                            mcrc_speech_netstatus_play_onoff(0);  //close play net status change music when net config
                        }
                        else if (   (0 == strncmp((char *)transtream.data, "/mnt/app/tts/network_config_success.mp3", transtream.size  - 1)) \
                                 || (0 == strncmp((char *)transtream.data, "/mnt/app/tts/network_config_fail.mp3", transtream.size  - 1)))
                        {
                            mcrc_speech_netstatus_play_onoff(1);  //close play net status change music when net config
                        }
                        else
                        {

                        }
                        continue;
                  }

                        HexToStr(data_buf, transtream.data, transtream.size);
						unsigned char cmdtype=transtream.data[9];
                        unsigned char dev_cmdtype=transtream.data[10];
						if((cmdtype == 0x02 && transtream.data[10] == 0xB0)||(cmdtype == 0x03 && transtream.data[10] == 0xB1)){
							if(device_0xB0_0xB1_process(transtream.data, puser))
								continue;
						}else if(cmdtype == 0x05 && transtream.data[10] == 0xB5){
							if(device_0xB5_process(transtream.data, puser))
								continue;
						}
#if defined(LOCAL_ASR)&&defined(LOCAL_ASR_AC)


						if(puser->is_online_process ==0){
							if((cmdtype ==0x05 && dev_cmdtype == 0xA0)||(cmdtype ==0x03 && (dev_cmdtype == 0xC0 || dev_cmdtype == 0xB1))||(cmdtype ==0x02 && (dev_cmdtype == 0xC0 || dev_cmdtype == 0xB0))){
								cJSON *device=cJSON_CreateObject();
								cJSON * deviceinfo= cJSON_CreateObject();
								cJSON * status= cJSON_CreateObject();

								cJSON  *msg = cJSON_CreateObject();

							//	MS_TRACE("local:\n%s\n", data_buf);
								cJSON_AddNumberToObject(deviceinfo, "deviceSubType",1);
								ms_stored_info_t dev_info;
								if(get_device_info(&dev_info) != -1)
									cJSON_AddStringToObject(deviceinfo, "deviceSN", dev_info.sn);
								 cJSON_AddStringToObject(msg, "data",data_buf);
								 cJSON_AddItemToObject(device,"deviceinfo",deviceinfo);
								 cJSON_AddItemToObject(device,"status",status);
								  cJSON_AddItemToObject(device,"msg",msg);
								  char *out = cJSON_Print(device);
									MS_TRACE("local:\n%s\n", out);

								char *result = data2json(out,cmdtype);
								MS_TRACE("result=%s\r\n",result);
								free(out);
								cJSON_Delete(device);
							}
						}
						else
#endif
						{
						    if(cmdtype !=0x04){
								m_cloud_trans(M_ORDER_TRANS_REPLY, data_buf, strlen(data_buf));
						    }
						}
                   }
				}
			}
		}
	}

    MS_TRACE("thread end, failure state\n");
    close(fd_maispeech_in);
}


msui_cb_return_t midea_speech_event_cb(struct speech_usrdata *usrdata) {
    int ret = 0;
    int event = usrdata->event;
    switch (event) {
        case MSUI_EV_OUT_CMD:
            {
                MS_TRACE("[msui]: MSUI_EV_OUT_CMD\n");
                cJSON *root = cJSON_Parse(usrdata->out_res);
				cJSON *cmd = cJSON_GetObjectItem(root->child, "cmd");
                char *out = cJSON_Print(root);
                MS_TRACE("[msui] out cmd is :%s\n",out);
				ret = send_msg_to_uart(cmd->valuestring, strlen(cmd->valuestring), 0);
				if(ret > 0){
					ret = MSUI_CB_RET_SUCCESS;
				}
				else {
					ret = MSUI_CB_RET_FAILED;
				}
                cJSON_Delete(root);
                free(out);
                break;
            }
        case MSUI_EV_OUT_NLU:
            {
                MS_TRACE("[msui]: MSUI_EV_OUT_NLU\n");
                ret = MSUI_CB_RET_SUCCESS;
                break;
            }

        case MSUI_EV_IN_DEVICE_STATUS:
            {
                //get request cmd
                cJSON *root = cJSON_Parse(usrdata->out_res);
                char *out = cJSON_Print(root);
                MS_TRACE("[msui]: MSUI_EV_IN_DEVICE_STATUS request is : %s\n", out);
				#if 1
					cJSON *cmd = cJSON_GetObjectItem(root->child, "cmd");
					ret = send_msg_to_uart(cmd->valuestring, strlen(cmd->valuestring), 1);
					MS_TRACE("send_msg_to_uart:%d\n", ret);
                    ret = MSUI_CB_RET_SUCCESS;
				#else
                    //char *cmd = "AA20B100000000000003310300000011001D3A4C00B400000000000000B40000DC";//reply device state cmd
                    char *cmd = "aa2de200000000000003010105001a0000048000304100001b0200000000000000100000003c000000000000006f";//reply device state cmd
                    strcpy(usrdata->in_req, cmd);
				#endif

                cJSON_Delete(root);
                free(out);
                break;
            }

        case MSUI_EV_OUT_SPEECH_STATE:
        {
            speech_state = usrdata->speech_state_now;
            if(speech_state == MSUI_STATE_WAIT_WAKEUP)
                MS_TRACE("[msui]: MSUI_EV_OUT_SPEECH_STATE is WAIT_WAKEUP\n");
            else if(speech_state == MSUI_STATE_IN_DIALOGUE)
                MS_TRACE("[msui]: MSUI_EV_OUT_SPEECH_STATE is IN_DIALOGUE\n");
            else if(speech_state == MSUI_STATE_ONLINE_THINK)
                MS_TRACE("[msui]: MSUI_EV_OUT_SPEECH_STATE is ONLINE_THINK\n");
            else if(speech_state == MSUI_STATE_TTS_SPEAK)
                MS_TRACE("[msui]: MSUI_EV_OUT_SPEECH_STATE is TTS_SPEAK\n");
            break;
        }

        case MSUI_EV_OUT_OUTER_DATA:
            {
                MS_TRACE("[msui]: MSUI_EV_OUT_OUTER_DATA\n");
                break;
            }

        case MSUI_EV_OUT_NATIVE_CALL:
            {
                MS_TRACE("[msui]: MSUI_EV_OUT_NATIVE_CALL\n");
                break;
            }

        case MSUI_EV_OUT_ERROR:
            {
                MS_TRACE("[msui]: MSUI_EV_OUT_ERROR\n");
                break;
            }
    }
    return ret;
}
int check_vad_suspend(void *state){
    char buf[64];
    long frames = -1;
    FILE *fd = NULL;
    fd = fopen("/sys/module/snd_soc_rockchip_vad/parameters/voice_inactive_frames", "r");
    if(fd){
        if(fgets(buf, sizeof(buf), fd)){
           frames = atol(buf);
        }
        fclose(fd);
    }else{
        debug_local(LOG_LEVEL_ERROR, "Can not get hardware vad state!!\n");
        return -1;
    }

    if (frames > 80000/*5sec * 16000*/){
        return 1;
    }

    return 0;
}



void* mcrc_speech_routine(struct mcrc_speech_user *user) {
    if (!user) return -1;
    mcrc_speech_process(user);
    pthread_mutex_lock(&main_voice_lock);
    main_voice_exit = true;
    pthread_mutex_unlock(&main_voice_lock);
	return NULL;
}
static int cur_time(){
	struct timeval current_tm;
	gettimeofday(&current_tm, NULL);
	return (current_tm.tv_sec<<10) + (current_tm.tv_usec>>10);
}

void factory_test_turn_on_Mic_and_Speaker(int signum){
        pthread_mutex_lock(&mute_lock);
	////midea_setSpeakerMute(false);
	//midea_setMicMute(&user, false);
	exit(0);
        pthread_mutex_unlock(&mute_lock);
}
#if 0
void SystemErrorHandler(int signum)
{
    const int len=1024;
    void *func[len];
    size_t size;
    int i;
    char **funs;

    signal(signum,SIG_DFL);
    size=backtrace(func,len);
    funs=(char**)backtrace_symbols(func,size);
    fMS_TRACE(stderr,"System error, Stack trace:\n");
    for(i=0;i<size;++i) fMS_TRACE(stderr,"%d %s\n",i,funs[i]);
    free(funs);
    //exit(1);
}
#endif
int main (int argc,char **argv) {

    int fpid = 0;
    int count = 0;
    if(argc == 2){
        sscanf(argv[1],"%d\n",&deamon_pid);
    }
	printf("mspeech\n");
    int ret = 0;
	//signal(SIGSEGV,SystemErrorHandler);
	//signal(SIGABRT,SystemErrorHandler);
    signal(SIGINT, factory_test_turn_on_Mic_and_Speaker);
	pthread_mutex_init(&main_voice_lock,NULL);
	pthread_mutex_init(&ac_status_lock, NULL);
	pthread_mutex_init(&mute_lock, NULL);
#ifdef HARD_VAD
    int last_sleep_time = cur_time();;
#endif
    //MCRC speech init
    char config[1024 * 5];
    FILE *fp = NULL;
	#if defined(AISPEECH_2MIC)
	fp = fopen("/mnt/app/res/config.json", "r");
	#elif defined(AISPEECH_A_4MIC)
    fp = fopen("/mnt/app/res/config_a_4mic.json", "r");
	#else
	#endif
    if (fp) {
        fread(config, 1, 1024 * 5, fp);
        fclose(fp);
    }
    else{
        MS_TRACE("Can't not open config.json\n!!!!");
        return -1;
    }
    strcpy(user.config, config);
    user._handler = midea_speech_event_cb;
    user.running = 1;
	user.is_online_process =0;
    user.is_mute = 0;
	snprintf(user.fwver, 47, "%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x",
				MS_HW_PLATFORM,
				MS_HW_CATEGORY,
				MS_HW_BIG_VERSION,
				MS_HW_SMALL_VERSION,
				MS_HW_FUNC_VERSION,
				MS_SW_VERSION_Y,
				MS_SW_VERSION_M,
				MS_SW_BIG_VERSION,
				MS_SW_SMALL_VERSION,
				MS_SW_FUNC_VERSION);
    pthread_t read_fifo_tid;
    pthread_t net_check_tid;
    pthread_t update_wakelock_tid;
    pthread_t speech_led_sync_tid;
	midea_initVolume(config);
#ifdef HARD_VAD
    pthread_rwlock_init(&wake_rwlock, NULL);
#endif
	if(0 != (ret = pthread_create(&read_fifo_tid,NULL,read_fifo_routine,&user))){
		MS_TRACE("read_fifo_routine pthread create fail\n");
		return -1;
	}
	if(0 != (ret = pthread_create(&speech_led_sync_tid,NULL,speech_led_status_sync,NULL))){
		MS_TRACE("speech_led_status_sync pthread create fail\n");
		return -1;
	}
	while(!speech_led_is_sync){
		count++;
		if(count > 1000)
			break;
		usleep(5000);
	}

#if 0
    if(0 != (ret = pthread_create(&net_check_tid, NULL, net_check_routine, &user))){
        MS_TRACE("net_connect_routine pthread create fail\n");
    }
#endif

#ifdef HARD_VAD
    if(0 != (ret = pthread_create(&update_wakelock_tid, NULL, updat_wakelock, NULL))){
        MS_TRACE("updat_wakelock pthread create fail\n");
    }
#endif
    if (0 != pthread_create(&user.tid, NULL, mcrc_speech_routine, &user)){
	MS_TRACE("mcrc_speech_routine creat failed\n");
	return -1;
    }

    //Run MCRC speech
#if defined(LOCAL_ASR)&&defined(LOCAL_ASR_AC)
	local_asr_register(asr_callback,local_fail_AC);
#endif
    while(1){
        pthread_mutex_lock(&main_voice_lock);
        if(main_voice_exit == true){
		main_voice_exit = false;
		pthread_mutex_unlock(&main_voice_lock);
		break;
        }
        pthread_mutex_unlock(&main_voice_lock);
#ifdef HARD_VAD
            // Timeout
            if((cur_time() -last_sleep_time) >600000){
		pthread_rwlock_wrlock(&wake_rwlock);
		wake_lock = 0;
		sleep_enable = true;
		pthread_rwlock_unlock(&wake_rwlock);
            }
            if(speech_state == MSUI_STATE_WAIT_WAKEUP/* && TBD*/){
                //Please get everything ready before call this intf.

                int v_ok = check_vad_suspend(NULL);
                pthread_rwlock_rdlock(&wake_rwlock);
                if((sleep_enable)&&v_ok&&(wake_lock<=0)){
                    last_sleep_time = cur_time();
                    ret = mcrc_speech_vad_suspend_system();
                    if(ret < 0){
                        user.running = 0;
                        pthread_rwlock_unlock(&wake_rwlock);
                        break;
                    }
                }
                pthread_rwlock_unlock(&wake_rwlock);
            }
#endif
		sleep(3);
	}
    pthread_mutex_destroy(&main_voice_lock);
    pthread_mutex_destroy(&ac_status_lock);
    pthread_mutex_destroy(&mute_lock);
#ifdef HARD_VAD
    pthread_rwlock_destroy(&wake_rwlock);
#endif
    return ret;
}
