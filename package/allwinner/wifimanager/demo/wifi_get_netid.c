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
			 wmg_printf(MSG_INFO,"Connecting to the network......\n");
			 break;
		 }
		 case CONNECTED:
		 {
			 wmg_printf(MSG_INFO,"Connected to the AP\n");
			 start_udhcpc();
			 break;
		 }

		 case OBTAINING_IP:
		 {
			 wmg_printf(MSG_INFO,"Getting ip address......\n");
			 break;
		 }

		 case NETWORK_CONNECTED:
		 {
			 wmg_printf(MSG_DEBUG,"Successful network connection\n");
			 break;
		 }
		case DISCONNECTED:
		{
		    wmg_printf(MSG_ERROR,"Disconnected,the reason:%s\n",wmg_event_txt(w->StaEvt.event));
		    break;
		}
    }
}

void *app_scan_task(void *args)
{
    const aw_wifi_interface_t *p_wifi = (aw_wifi_interface_t *)args;
    char scan_results[4096];
    int len = 0;
    int event_label = 0;

    while(1){
        len = 4096;
        p_wifi->get_scan_results(scan_results, &len);
    }
}


void print_help(){
	wmg_printf(MSG_INFO,"---------------------------------------------------------------------------------\n");
	wmg_printf(MSG_INFO,"NAME:\n\twifi_get_netid_test\n");
	wmg_printf(MSG_INFO,"DESCRIPTION:\n\tget the netid of the AP.\n");
	wmg_printf(MSG_INFO,"USAGE:\n\twifi_get_netid_test <ssid> <key_mgmt> <level>\n");
	wmg_printf(MSG_INFO,"PARAMS:\n\tssid     : ssid of the AP\n");
	wmg_printf(MSG_INFO,"\tkey_mgmt : encryption method of the AP\n");
	wmg_printf(MSG_INFO,"\t\t0 : NONE\n");
	wmg_printf(MSG_INFO,"\t\t1 : key_mgmt = WPA_PSK\n");
	wmg_printf(MSG_INFO,"\t\t2 : key_mgmt = WPA2_PSK\n");
	wmg_printf(MSG_INFO,"\t\t3 : key_mgmt = WEP\n");
	wmg_printf(MSG_INFO,"\tlevel : print level(d0~d5).larger value,more info.para is not required,default d2.\n");
	wmg_printf(MSG_INFO,"--------------------------------------MORE---------------------------------------\n");
	wmg_printf(MSG_INFO,"The way to get help information:\n");
	wmg_printf(MSG_INFO,"\twifi_get_netid_test --help\n");
	wmg_printf(MSG_INFO,"\twifi_get_netid_test -h\n");
	wmg_printf(MSG_INFO,"\twifi_get_netid_test -H\n");
	wmg_printf(MSG_INFO,"---------------------------------------------------------------------------------\n");
}


int check_paras(int num, char *str[]){
	if(num < 3 || num >4){
		wmg_printf(MSG_ERROR,"ERROR: paras more or less!\n");
		return -1;
	}

	int mgmt = atoi(str[2]);
	if(mgmt >= 0 && mgmt <= 3){
		return 0;
	}else{
		wmg_printf(MSG_ERROR,"ERROR: key_mgmt not allowed\n");
		return -1;
	}

}

/*
 *argc[1]   ap ssid
 *argc[2]   ap key_mgmt:"0"--WIFIMG_NONE;"1"--WIFIMG_WPA_PSK;"2"--WIFIMG_WPA2_PSK;"3"--WIFIMG_WEP
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
    int ret = -1, switch_int = -1;
    int event_label = 0;
    const aw_wifi_interface_t *p_wifi_interface = NULL;
    tKEY_MGMT key_mgmt = WIFIMG_NONE;
    char net_id[10]={0};
    int id_len = sizeof(net_id);

    if(argv == 2 && (!strcmp(argc[1],"--help") || !strcmp(argc[1], "-h") || !strcmp(argc[1], "-H"))){
	    print_help();
	    return -1;
    }

    if(check_paras(argv, argc)){
	    print_help();
	    return -1;
    }

    if(set_log_level(argv,argc))
	    return -1;

    event_label = rand();
    p_wifi_interface = aw_wifi_on(wifi_state_handle, event_label);
    if(p_wifi_interface == NULL){
        wmg_printf(MSG_ERROR,"wifi on failed \n");
        return -1;
    }

    wmg_printf(MSG_INFO,"\n*********************************\n");
    wmg_printf(MSG_INFO,"***Start wifi get netid test!***\n");
    wmg_printf(MSG_INFO,"*********************************\n");

	switch_int= atoi(argc[2]);
	wmg_printf(MSG_DEBUG,"The switch_int is %d\n",switch_int);

	switch(switch_int)
	{
		case 0: key_mgmt = WIFIMG_NONE; break;
		case 1: key_mgmt = WIFIMG_WPA_PSK; break;
		case 2: key_mgmt = WIFIMG_WPA2_PSK; break;
		case 3: key_mgmt = WIFIMG_WEP; break;
		default: ; break;
	}


    ret = p_wifi_interface->get_netid(argc[1], key_mgmt, net_id, &id_len);
    if(ret == 0){
	wmg_printf(MSG_INFO,"The netid of your network is %s\n",net_id);
	wmg_printf(MSG_INFO,"The lenght of netid is %d\n",id_len);
	wmg_printf(MSG_INFO,"******************************\n");
	wmg_printf(MSG_INFO,"Wifi get netid test: Success!\n");
	wmg_printf(MSG_INFO,"******************************\n");
    }else{
	wmg_printf(MSG_ERROR,"get netid test failed!\n");

    }


    return 0;
}
