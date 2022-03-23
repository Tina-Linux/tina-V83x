/*
 * =============================================================================
 *
 *       Filename:  wifi_cntrl.h
 *
 *    Description:
 *
 *     Created on:  2016Äê11ÔÂ21ÈÕ
 *
 *     Created by:  liuyitong
 *
 * =============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <memory.h>


#include <sys/socket.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include "def.h"
#include "wifi_cntrl.h"

#define LINUX 1

extern enum loglevel g_debuglevel;
extern char ifName[IFNAMSIZ];
extern char g_wpafile[128];

void xr_system(const char* cmd){
	//if(g_use_file) return;
	LOG(INFO, "shell: %s \n", cmd);
	system(cmd);
	if(errno > 0)
		printf("%s, %d\n", cmd, errno);
}
void clear_wpas(void){
//	xr_system("killall wpa_supplicant 2>/dev/null");
	printf("nothing to do.\n");
	//TODO and also clear the conf file
}

void wifi_disable(const char* ifName){
	char cmd[64];
	sprintf(cmd, "ifconfig %s down", ifName);
	xr_system(cmd);

}

void wifi_enable(const char* ifName){
	char cmd[64];
	sprintf(cmd, "ifconfig %s up", ifName);
	xr_system(cmd);

}

void wifi_set_channel(const int ch){
	char cmd[64];
	sprintf(cmd, "iw %s set channel %d", ifName, ch);
	xr_system(cmd);
}

void wifi_monitor_on(void){
	wifi_disable(ifName);
	printf("wifi monitor start !!!\n");
	char cmd[64];
	sprintf(cmd, "iw %s set type monitor", ifName);
	xr_system(cmd);

	wifi_enable(ifName);
	sleep(1);
}

void wifi_station_on(void){
	wifi_disable(ifName);
	char cmd[64];
	sprintf(cmd, "iw %s set type station", ifName);
	xr_system(cmd);
	wifi_enable(ifName);
	sleep(1);
}

int xr_connect_ap(void){
	char cmd[128];
#if LINUX
	printf("wpa_supplicant start!\n");
	sprintf(cmd, "wpa_supplicant -i%s -Dnl80211 -c %s -B", ifName, g_wpafile);
	//wpa_supplicant -iwlan0 -Dnl80211 -c /tmp/wpa_supplicant.conf -B
	xr_system(cmd);
	sleep(5);
#else
	//sprintf(cmd, "cp -f wpa_conf /data/misc/wifi/wpa_supplicant.conf");
	//xr_system(cmd);
	usleep(1000);
	sprintf(cmd, "svc wifi enable");
	xr_system(cmd);
	sleep(3);
#endif
	return 0;
}

int scan(void){

	char cmd[128];
	sprintf(cmd, "rm -rf /tmp/wifi_list.txt");
	xr_system(cmd);
	usleep(1000*10);
	LOG(ALWAYS, "scan \n");
	sprintf(cmd, "iw %s scan > /tmp/wifi_list.txt", ifName);
	xr_system(cmd);
	usleep(1000*10);
	return 1;
}

int xr_request_ip(void){
	char cmd[128];
	sprintf(cmd, "udhcpc -i %s", ifName);
	xr_system(cmd);
	//sleep(3);
	return 0;
}

int get_addr(int sockfd, char *mac_addr, unsigned *self_ipaddr)
{
	struct ifreq ifopts;

	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifopts)!=0) {
		return -EIO;
	}
	memcpy(mac_addr, ifopts.ifr_hwaddr.sa_data, ETH_ALEN);
	if (ioctl(sockfd, SIOCGIFADDR, &ifopts)!=0) {
		return -EIO;
	}

	*self_ipaddr = ((struct sockaddr_in *)&ifopts.ifr_addr)->sin_addr.s_addr;
	return 0;
}

int xr_sendack(struct ssidpwd_complete *complete)
{
	struct sockaddr_in s;
	int sockfd, addr_len, broadcast = 1;
	unsigned int self_ipaddr = 0, i;
	unsigned char payload = 0;
	char smac[6];

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// this call is what allows broadcast packets to be sent:
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
		perror("setsockopt (SO_BROADCAST)");
		exit(EXIT_FAILURE);
	}

	get_addr(sockfd, smac, &self_ipaddr);
	self_ipaddr |= htonl(0x000000FF);
	bzero(&s,sizeof(struct sockaddr_in));
	s.sin_family = AF_INET;
	s.sin_port = htons(ACK_DEST_PORT);
	s.sin_addr.s_addr = self_ipaddr;   //htonl(INADDR_BROADCAST);
	payload = complete ? complete->round_num : 'A';
	LOG(NOTICE,"Broadcast  UDP ack - %d\n", payload);
	addr_len = sizeof(struct sockaddr);
	for(i=0; i < SEND_ACK_TIMES; i++)
		sendto(sockfd, (unsigned char *)&payload, sizeof(unsigned char), 0,
				(struct sockaddr *)&s, addr_len);

	LOG(NOTICE,"Broadcast  UDP close \n");
	close(sockfd);
	return 0;
}


int check_ip_timeout(const int timeout)
{
	int sockfd, ret = 0;
	unsigned int self_ipaddr=0;
	char smac[6];
	int i;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		return errno;
	}

	for (i=0; i<timeout; i++) {
		ret = get_addr(sockfd, smac, &self_ipaddr);
		if (self_ipaddr != 0) {
			close(sockfd);
			return 0;
		}
		LOG(NOTICE, "Getting IP address(%02ds) ... \n", i);
		sleep(1);
	}
	close(sockfd);
	sleep(1);
	return ret;
}


/*wifi connect file of android or linux*/
int gen_wpafile(struct ssidpwd_complete *complete,  int encrypt_type){
#if LINUX
	FILE *fp;
	xr_system("rm -f /tmp/wpa_supplicant.conf");
	if((fp=fopen(g_wpafile, "w+"))==NULL){
		LOG(ALWAYS, "open file failed %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	char commset[]={"update_config=1\nctrl_interface=/var/log/wpa_supplicant\n"};
	char WPAstr[]={"network={\n\tssid=\"%s\"\n\tpsk=\"%s\"\n\tkey_mgmt=WPA-PSK\n}\n"};
	char OPENstr[]={"ap_scan=1\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\tscan_ssid=1\n}\n"};
	char WEPstr[]={"ap_scan=1\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\twep_key0=\"%s\"\n\twep_tx_keyidx=0\n\tscan_ssid=1\n}\n"};
	char CmdStr[2048] = {0};
	char PswStr[64]={0};
	sprintf(CmdStr, "%s", commset);
	fprintf(fp,"%s", CmdStr);
	if(complete->pwd_size)
	{
			strncpy(PswStr,(char *)complete->pwd,complete->pwd_size);
			printf("%s\n",PswStr);
	}

	switch (encrypt_type) {
	case CFGFILE_OPEN :
		sprintf(CmdStr, OPENstr, complete->ssid);
		break;
	case CFGFILE_WPA :
		sprintf(CmdStr, WPAstr, complete->ssid, PswStr);
		break;
	case CFGFILE_WEP :
		sprintf(CmdStr, WEPstr, complete->ssid, PswStr);
		break;
	default :
	//	return -EINVAL;
		break;
	}
	LOG(INFO, "%s\n",CmdStr);
	fprintf(fp,"%s", CmdStr);
	fclose(fp);
	return 0;
#else
	FILE *fp;
	if((fp=fopen(g_wpafile, "w+"))==NULL){
		LOG(ALWAYS, "open file failed %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	char commset1[]={"ctrl_interface=/data/misc/wifi/sockets\nupdate_config=1\ndevice_name=r16_evb\nmanufacturer=unknown\n"};
	char commset2[]={"model_name=Allwinner-Tablet\nmodel_number=Allwinner-Tablet\nserial_number=03389918d84500000000\n"};
	char commset3[]={"device_type=10-0050F204-5\nconfig_methods=physical_display virtual_push_button\nwmm_enabled=1\nuapsd_enabled=1\n"};
	char WPAstr[]={"\nnetwork={\n\tssid=\"%s\"\n\tpsk=\"%s\"\n\tkey_mgmt=WPA-PSK\n\tpriority=1\n}\n"};
	char OPENstr[]={"\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\tpriority=1\n}\n"};
	char WEPstr[]={"\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\tauth_alg=OPEN SHARED\n\twep_key0=\"%s\"\n\tpriority=1\n}\n"};
	char CmdStr[2048] = {0};
	sprintf(CmdStr, "%s", commset1);
	fprintf(fp,"%s", CmdStr);
	sprintf(CmdStr, "%s", commset2);
	fprintf(fp,"%s", CmdStr);
	sprintf(CmdStr, "%s", commset3);
	fprintf(fp,"%s", CmdStr);
	switch (encrypt_type) {
	case CFGFILE_OPEN :
		LOG(NOTICE, "input file ssid :%s\n", complete->ssid);
		sprintf(CmdStr, OPENstr, complete->ssid);
		break;
	case CFGFILE_WPA :
		LOG(NOTICE, "input file ssid :%s\n", complete->ssid);
		LOG(NOTICE, "input file pwd :%s\n", complete->pwd);
		sprintf(CmdStr, WPAstr, complete->ssid, complete->pwd);
		break;
	case CFGFILE_WEP :
		LOG(NOTICE, "input file ssid :%s\n", complete->ssid);
		LOG(NOTICE, "input file pwd :%s\n", complete->pwd);
		sprintf(CmdStr, WEPstr, complete->ssid, complete->pwd);
		break;
	default :
	//	return -EINVAL;
		break;
	}
	LOG(INFO, "%s\n",CmdStr);
	fprintf(fp,"%s", CmdStr);
	fclose(fp);
	return 0;
#endif
}

int gen_hide_ssid(struct ssidpwd_complete *complete,  int encrypt_type){
	FILE *fp;
	if((fp=fopen(g_wpafile, "w+"))==NULL){
		LOG(ALWAYS, "open file failed %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	char commset1[]={"ctrl_interface=/data/misc/wifi/sockets\nupdate_config=1\ndevice_name=r16_evb\nmanufacturer=unknown\n"};
	char commset2[]={"model_name=Allwinner-Tablet\nmodel_number=Allwinner-Tablet\nserial_number=03389918d84500000000\n"};
	char commset3[]={"device_type=10-0050F204-5\nconfig_methods=physical_display virtual_push_button\nwmm_enabled=1\nuapsd_enabled=1\n"};
	char WPAstr[]={"\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tpsk=\"%s\"\n\tkey_mgmt=WPA-PSK\n}\n"};
	char OPENstr[]={"\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tkey_mgmt=NONE\n}\n"};
	char WEPstr[]={"\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tkey_mgmt=NONE\n\tauth_alg=OPEN SHARED\n\twep_key0=\"%s\"\n}\n"};
	char CmdStr[2048] = {0};
	sprintf(CmdStr, "%s", commset1);
	fprintf(fp,"%s", CmdStr);
	sprintf(CmdStr, "%s", commset2);
	fprintf(fp,"%s", CmdStr);
	sprintf(CmdStr, "%s", commset3);
	fprintf(fp,"%s", CmdStr);
	switch (encrypt_type) {
	case CFGFILE_OPEN :
		LOG(NOTICE, "input file ssid :%s\n", complete->ssid);
		sprintf(CmdStr, OPENstr, complete->ssid);
		break;
	case CFGFILE_WPA :
		LOG(NOTICE, "input file ssid :%s\n", complete->ssid);
		LOG(NOTICE, "input file pwd :%s\n", complete->pwd);
		sprintf(CmdStr, WPAstr, complete->ssid, complete->pwd);
		break;
	case CFGFILE_WEP :
		LOG(NOTICE, "input file ssid :%s\n", complete->ssid);
		LOG(NOTICE, "input file pwd :%s\n", complete->pwd);
		sprintf(CmdStr, WEPstr, complete->ssid, complete->pwd);
		break;
	default :
	//	return -EINVAL;
		break;
	}
	LOG(INFO, "%s\n",CmdStr);
	fprintf(fp,"%s", CmdStr);
	fclose(fp);
	return 0;
}
