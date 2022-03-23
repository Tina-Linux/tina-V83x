/*
 * =============================================================================
 *
 *       Filename:  main.c
 *
 *    Description:
 *
 *     Created on:  2016年11月21日
 *
 *     Created by:  liuyitong
 *
 * =============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <memory.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

#include "def.h"
#include "decode.h"
#include "wifi_cntrl.h"
#include "time_out.h"
#include "scan.h"
#include "sm_link_manager.h"
//#include <dlfcn.h>

#define LINUX 1
int ch_idx = 0;
////////////////global //////////////

const u8 multicast_mac[6] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x01};	//TODO confirm this
char ifName[IFNAMSIZ] = "wlan0";
char g_wpafile[128] = "/tmp/wpa_supplicant.conf";

int g_stop = 0;
int g_ap_channel = -1;
int while_test = 0;
#ifdef USE_CTRL_
extern int g_channel_locked;
extern int g_dump_packets;
extern int delay_set;
extern int ctrl_display_pkt;
void wsm_dump_switch(int dump);
#endif
static int f_send_finished;
static char server_state[5]="yes";

enum loglevel g_debuglevel = NOTICE;

int chan_list[13] = {1,6,11,2,3,4,5,7,8,9,10,12,13};

////////////////end global////////////

int parse_argv(int argc, char** argv){
	int opt;
	while ((opt = getopt(argc, argv, "dvpwtsc:")) != -1) {
		switch (opt) {
			case 'd':
				if(g_debuglevel<DETAIL)
				g_debuglevel++;
				break;
			case 'v':
				printf("ver %s\n", VERSION);
				exit(0);
#ifdef USE_CTRL_
			case 'p':
				g_dump_packets = 1;
				break;
			case 's':
				scanf("%d", &delay_set);
				break;
			case 'w':
				wsm_dump_switch(1);
				break;
#endif
			case 't':
				while_test = 1;
				break;
			case 'c':
				strncpy(server_state,optarg,sizeof(server_state));
                break;
			default:
				// usage();
				return 0;

		}
	}

	strcpy(g_wpafile, "/tmp/wpa_supplicant.conf");
	return 0;
}

static int get_rtheader_len(u8 *buf, size_t len){
	struct ieee80211_radiotap_header *rt_header;
	u16 rt_header_size;

	rt_header = (struct ieee80211_radiotap_header *)buf;
	/* check the radiotap header can actually be present */
	if (len < sizeof(struct ieee80211_radiotap_header))
		return -EILSEQ;
	 /* Linux only supports version 0 radiotap format */
	 if (rt_header->it_version)
		return -EILSEQ;
	rt_header_size = le16toh(rt_header->it_len);
	 /* sanity check for allowed length and radiotap length field */
	if (len < rt_header_size)
		return -EILSEQ;
	return rt_header_size;
}

#ifdef USE_CTRL_
extern void* ctrlThread(void*);
#endif

void* recvThread(void * p){
	static struct timeval start;
	static struct timeval end;
	double average_time = 0, max_time = 0;
	int complete_count = 0, than_1s = 0, than_3s = 0, max_count = 0;
	/*****************************************/
	int sockfd, sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;	/* set promiscuous mode */
	unsigned char buf[BUF_SIZ], *pkt;
	int rt_header_len = 0;		//rt for radiotap

	//Create a raw socket that shall sniff
	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		LOG(ALWAYS,"socket - %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	/* Set interface to promiscuous mode - do we need to do this every time? */
	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);
	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		LOG(ALWAYS,"setsockopt - %s\n", strerror(errno));
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		LOG(ALWAYS,"setsocket SO_BINDTODEVICE - %s\n", strerror(errno));
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	gettimeofday(&start,NULL);
	while(1) {
		static int success = 0;
		double time_use = 0;
		if(success == 0) {
			numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
			if (numbytes < 0) {
				LOG(NOTICE, "recvfrom - %s\n", strerror(errno));
				printf("UDP error\n");
				goto thr_exit;
			}
			if (g_stop) goto thr_exit;
			/*et length of Radiotap header*/
			if ((rt_header_len = get_rtheader_len(buf, (size_t)numbytes)) < 1){
				LOG(DETAIL, "skip packet with rt_header_len = %d\n", rt_header_len);
				continue;
			}

			/*ow process the packet*/
			pkt = (u8*)buf + rt_header_len;

#ifdef USE_CTRL_
			if(ctrl_display_pkt == 1) {
				/*print seize pacekt*/
				printADDR("A1", (pkt + 4), ALWAYS);
				printADDR("A2", (pkt + 10),ALWAYS);
				printADDR("A3", (pkt + 16), ALWAYS);
				LOG(ALWAYS, "________________________________\n");
			}
#endif

			if (packet_filter(pkt, p_lead_code()) == 0)
				continue;

			/*if time receive time out,the function will be restart*/
			clear_out_count();/*clear the time out count*/

#ifdef USE_CTRL_

			if(ctrl_display_pkt == 2) {
				/*print the packet that it is filter*/
				printADDR("A1", (pkt + 4), ALWAYS);
				printADDR("A2", (pkt + 10),ALWAYS);
				printADDR("A3", (pkt + 16), ALWAYS);
				LOG(ALWAYS, "________________________________\n");
			}
#endif
			if(packet_deoced (pkt, p_lead_code()) == 1){
				success = 1;
				/*those is debug info*/
				gettimeofday(&end,NULL);
				time_use = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_sec - start.tv_sec);//ms
				time_use /= 1000.0;
				LOG(NOTICE, "time d is : %lf ms\n", time_use);
				start = end;

				complete_count += 1;
				average_time += time_use;

				if(time_use == max_time)
					max_count += 1;
				if(time_use > max_time){
					max_time = time_use;
					max_count = 0;
				}
				if(time_use >= 1000 && time_use <= 2000)
					than_1s += 1;

				if(time_use >= 3000)
					than_3s += 1;

				LOG(ALWAYS, "SUCCESS!!!!!!\n");
				LOG(ALWAYS, "________________________________\n");

				struct ssidpwd_complete *data = p_ssidpwd_complete_();
				LOG(ALWAYS, "SSID_SIZE : %d\n", data->ssid_size);
				LOG(ALWAYS, "SSID : %s\n",data->ssid);

				LOG(ALWAYS, "PWD_SIZE : %d\n", data->pwd_size);
				LOG(ALWAYS, "PWD : %s\n",data->pwd);

				LOG(ALWAYS, "=====================================\n");
				LOG(ALWAYS, "=====================================\n");
				LOG(ALWAYS, "complete_count : %d\n", complete_count);
				LOG(ALWAYS, "max_count : %d\n", max_count);
				LOG(ALWAYS, "than_1s : %d\n", than_1s);
				LOG(ALWAYS, "than_3s : %d\n", than_3s);
				LOG(ALWAYS, "now_time : %lf\n", time_use);
				LOG(ALWAYS, "average_time : %lf\n", average_time/complete_count);
				LOG(ALWAYS, "max_time : %lf\n", max_time);
				if (while_test == 1) {
					success = 0;
					restart_packet_decoed(p_lead_code(), p_ssid_pwd_data());
					ch_idx = 0;
					g_ap_channel = -1;
					LOG(ALWAYS, "RESTART END\n");
					if(complete_count == 1000)
					exit(1);

				LOG(ALWAYS, "=====================================\n");
				LOG(ALWAYS, "=====================================\n");
				/*debug info end*/
				}
			}
		}
		else
			break;
	}

thr_exit:
	close(sockfd);
	LOG(ALWAYS, "recv thread exitting\n");
	return NULL;
}

int xr_server_thread_handle(char *buf, int length)
{
    int ret;

    ret = strcmp(buf,"OK");
    if(0 == ret)
    {
	f_send_finished = 1;
	printf("thread_handle of xrsc: recieve OK from server!\n");
	return 0;
    }
    return -1;
}
int xradio_smartconfig_protocol_resource_free(void *arg)
{
	system("ifconfig wlan0 down");
}
int xradio_smartconfig_protocol(void *arg)
{
	int ret;

//	parse_argv(argc, argv);
	struct pro_feedback info;

	bool is_receive = false;

	struct pro_worker *_worker = (struct pro_worker *)arg;
	struct net_info netInfo;

	info.force_quit_sm = false;
	info.protocol = _worker->type;

	_worker->enable = true;

	_worker->free_cb = xradio_smartconfig_protocol_resource_free;

#ifdef USE_CTRL_
	pthread_t ctrl_tid;
	ret = pthread_create(&ctrl_tid, NULL, (void *)& ctrlThread, NULL);
	if(ret != 0){
		LOG(ALWAYS,"create thread faild : %s\n", strerror(ret));
		goto end;
	}else LOG(INFO, "new thread: %d created \n", (int)ctrl_tid);
#endif

	printf("start!!!\n");
	clear_wpas();
	wifi_monitor_on();

	pthread_t recv_tid;
	ret = pthread_create(&recv_tid, NULL, (void *)& recvThread, NULL);
	if(ret != 0) {
		LOG(ALWAYS,"create thread faild : %s\n", strerror(ret));
		goto end;
	}else LOG(INFO, "new thread: %d created \n", (int)recv_tid);

	pthread_t time_out_tid;
	ret = pthread_create(&time_out_tid, NULL, (void *)& time_out, NULL);
	if(ret != 0) {
		LOG(ALWAYS,"create thread faild : %s\n", strerror(ret));
		goto end;
	}else LOG(INFO, "new thread: %d created \n", (int)time_out_tid);

	while(_worker->enable){

#ifdef USE_CTRL_
		usleep(1000 * delay_set);	//TODO find a best interval, for example the delay time is self-adaptation;
#else
		usleep(1000 * 20);
#endif
		static int curr_ch = 0;
		if(g_stop || !_worker->enable){
			LOG(NOTICE, "exitting main...\n");
			break;
		}
#ifdef USE_CTRL_
		if(get_status() == XRSC_STATUS_SRC_LOCKED || g_channel_locked == 1) {
		/*device is locked the channel, now it's geting data*/
			if(g_channel_locked == 1)
				continue;
#else
		if(get_status() == XRSC_STATUS_SRC_LOCKED){
#endif
			if(g_ap_channel == -1) {
				g_ap_channel = get_channel();
				LOG(ALWAYS, "now channel %d\n", curr_ch);
				LOG(ALWAYS, "stop in channel %d\n", g_ap_channel);
				if(g_ap_channel != curr_ch && g_ap_channel > 0 && g_ap_channel<=13)
					wifi_set_channel(g_ap_channel);
			}
			continue;
		}else if(get_status() == XRSC_STATUS_SEARCHING) {
		/*device is sourch leadcode, now need change channel*/
			curr_ch = chan_list[ch_idx];
			wifi_set_channel(curr_ch);
			if(ch_idx == 12) ch_idx = 0;
			else ch_idx++;
			continue;
		}else if(get_status() == XRSC_STATUS_COMPLETE) {
		/*device is get ssid and password, now need to connect ap*/
			LOG(INFO, "seems completed \n");
			if (while_test == 0) {
				struct ssidpwd_complete *complete;
				int i = 0, j = 5;
				complete = p_ssidpwd_complete_();
				g_stop = 1;
				wifi_station_on();

				strcpy(netInfo.ssid,(char *)complete->ssid);
				strcpy(netInfo.password, (char *)complete->pwd);
				is_receive = true;

			}
		}else {
			LOG(ALWAYS, "invalid status return \n");
		}
	}

end:
	_worker->enable = false;
	_worker->cb(&info,is_receive,&netInfo);

	LOG(ALWAYS, " main return 0\n");
	return 0;
}
