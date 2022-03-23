#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wifi_intf.h>
#include <pthread.h>
#include "wmg_debug.h"
#include "wifi_udhcpc.h"

static void wifi_state_handle(struct Manager *w, int event_label)
{
    wmg_printf(MSG_DEBUG,"event_label 0x%x\n", event_label);

    switch(w->StaEvt.state)
    {
		 case CONNECTING:
		 {
			 wmg_printf(MSG_INFO,"Connecting to the network(%s)......\n",w->ssid);
			 break;
		 }
		 case CONNECTED:
		 {
			 wmg_printf(MSG_INFO,"Connected to the AP(%s)\n",w->ssid);
			 start_udhcpc();
			 break;
		 }

		 case OBTAINING_IP:
		 {
			 wmg_printf(MSG_INFO,"Getting ip address(%s)......\n",w->ssid);
			 break;
		 }

		 case NETWORK_CONNECTED:
		 {
			 wmg_printf(MSG_DEBUG,"Successful network connection(%s)\n",w->ssid);
			 break;
		 }
		case DISCONNECTED:
		{
		    wmg_printf(MSG_ERROR,"Disconnected,the reason:%s\n",wmg_event_txt(w->StaEvt.event));
		    break;
		}
    }
}
void print_help(){
	wmg_printf(MSG_INFO,"---------------------------------------------------------------------------------\n");
	wmg_printf(MSG_INFO,"NAME:\n\twifi_connect_ap_test\n");
	wmg_printf(MSG_INFO,"DESCRIPTION:\n\tconnect the AP when don't konw the key_mgmt.\n");
	wmg_printf(MSG_INFO,"USAGE:\n\twifi_connect_ap_test <ssid> <passwd> <level>\n");
	wmg_printf(MSG_INFO,"PARAMS:\n\tssid   : ssid of the AP\n");
	wmg_printf(MSG_INFO,"\tpasswd : password of the AP, don't need it when key_mgmt is NONE\n");
	wmg_printf(MSG_INFO,"\tlevel  : print level(d0~d5).larger value,more info.para is not required,default d2.\n");
	wmg_printf(MSG_INFO,"--------------------------------------MORE---------------------------------------\n");
	wmg_printf(MSG_INFO,"The way to get help information:\n");
	wmg_printf(MSG_INFO,"\twifi_connect_ap_test --help\n");
	wmg_printf(MSG_INFO,"\twifi_connect_ap_test -h\n");
	wmg_printf(MSG_INFO,"\twifi_connect_ap_test -H\n");
	wmg_printf(MSG_INFO,"---------------------------------------------------------------------------------\n");
}


int check_paras(int num, char *str[]){
	if(num <2 || num >4){
		wmg_printf(MSG_ERROR,"ERROR: paras more or less!\n");
		return -1;
	}

	return 0;
}

/*
 *argc[1]   ap ssid
 *argc[2]   ap passwd
*/
static int set_log_level(int argv, char *argc[])
{
	if(argv >=4 && !strncmp(argc[3],"d",1)){
		char *debug = argc[3];
		if(strlen(debug) >=2 && debug[1] >= '0' &&
			debug[1] <= '5'){
			wmg_set_debug_level(debug[1] - '0');
			return 0;
		}else{
			printf("Illegal level\n");
			printf("Level range 0~5\n");
			return -1;
		}
	}
	return 0;
}

int main(int argv, char *argc[]){
    int ret = 0, len = 0;
    int times = 0, event_label = 0;;
    char ssid[256] = {0}, scan_results[4096] = {0};
    const aw_wifi_interface_t *p_wifi_interface = NULL;

	if(argv == 2 && (!strcmp(argc[1],"--help") || !strcmp(argc[1], "-h") || !strcmp(argc[1], "-H"))){
		print_help();
		return -1;
	}

	if(check_paras(argv, argc)){
		print_help();
		return -1;
	}
	if(set_log_level(argv, argc) == -1)
		return -1;

    wmg_printf(MSG_DEBUG,"\n*********************************\n");
    wmg_printf(MSG_DEBUG,"***Start wifi connect ap test\n");
	wmg_printf(MSG_DEBUG,"***ssid    :%s\n",argc[1]);
	wmg_printf(MSG_DEBUG,"***password:%s\n",argc[2]);
    wmg_printf(MSG_DEBUG,"*********************************\n");
	wmg_printf(MSG_INFO,"==================================\n");

    event_label = rand();
    p_wifi_interface = aw_wifi_on(wifi_state_handle, event_label);
    if(p_wifi_interface == NULL){
        wmg_printf(MSG_ERROR,"wifi on failed\n");
        return -1;
    }

	while(aw_wifi_get_wifi_state() == CONNECTING) {
		sleep(1);
		wmg_printf(MSG_INFO,"waiting connected......");
	}

	if(aw_wifi_get_wifi_state() == NETWORK_CONNECTED){
		wmg_printf(MSG_INFO,"auto connected Successful  !!!!\n");
		wmg_printf(MSG_INFO,"==================================\n");
	}

	p_wifi_interface->connect_ap(argc[1], argc[2], event_label);
	if(aw_wifi_get_wifi_state() == NETWORK_CONNECTED)
		wmg_printf(MSG_INFO,"Wifi connect ap : Success!\n");
	else {
		wmg_printf(MSG_ERROR,"Wifi connect ap : Failure!\n");
		return -1; //flag for tinatest
	}
	wmg_printf(MSG_INFO,"==================================\n");
    return 0;
}
