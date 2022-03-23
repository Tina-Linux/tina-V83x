#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wifi_intf.h>
#include <pthread.h>
#include <sys/time.h>
#include "wmg_debug.h"
#include "wifi_udhcpc.h"

#define MAX_WAIT_CONNECTION_TIME 3
#define BTMG_GET_CONNECTION_INFO_TEST_TIMES 50

static bool is_wifi_connected = false;

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
			 is_wifi_connected = true;
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
			is_wifi_connected = false;
		    wmg_printf(MSG_ERROR,"Disconnected,the reason:%s\n",wmg_event_txt(w->StaEvt.event));
		    break;
		}
    }
}

void print_help(){
	wmg_printf(MSG_INFO,"---------------------------------------------------------------------------------\n");
	wmg_printf(MSG_INFO,"NAME:\n\twifi_get_connection_info_test\n");
	wmg_printf(MSG_INFO,"DESCRIPTION:\n\ttest the connection status of wifi\n");
	wmg_printf(MSG_INFO,"USAGE:\n\twifi_get_connection_info_test <test_times> <level>\n");
	wmg_printf(MSG_INFO,"SPECIAL NOTICE:\n\tBefore doing this test, please make sure your device is already connected to AP\n");
	wmg_printf(MSG_INFO,"PARAMS:\n");
	wmg_printf(MSG_INFO,"\ttest_times      : total test times to report connection infomation. para is not required, default 50.\n");
	wmg_printf(MSG_INFO,"\tlevel      : print level(d0~d5).larger value,more info.para is not required,default d2.\n");
	wmg_printf(MSG_INFO,"--------------------------------------MORE---------------------------------------\n");
	wmg_printf(MSG_INFO,"The way to get help information:\n");
	wmg_printf(MSG_INFO,"\twifi_get_connection_info_test --help\n");
	wmg_printf(MSG_INFO,"\twifi_get_connection_info_test -h\n");
	wmg_printf(MSG_INFO,"\twifi_get_connection_info_test -H\n");
	wmg_printf(MSG_INFO,"---------------------------------------------------------------------------------\n");
}
/*
*/
static int set_log_level(int argv, char *argc[])
{
	if(argv >= 3 && !strncmp(argc[2],"d",1)){
		char *debug = argc[2];
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
    int ret = 0;
    int i = 0, event_label = 0;
    const aw_wifi_interface_t *p_wifi_interface = NULL;
	int try_conn_time = 0;
	connection_status *connection_info = NULL;
	char ssid[128] = {0};
	int ssid_len = sizeof(ssid);
	char *test_time_chr = NULL;
	int test_time = BTMG_GET_CONNECTION_INFO_TEST_TIMES;

    if(argv == 2 && (!strcmp(argc[1],"--help") || !strcmp(argc[1], "-h") || !strcmp(argc[1], "-H"))){
	    print_help();
	    return -1;
    }

    if(argv > 3){
	    wmg_printf(MSG_ERROR,"ERROR:No need other paras\n");
	    print_help();
	    return -1;
    }


    if(set_log_level(argv,argc))
	    return -1;

	if (argv >= 2) {
		test_time_chr = argc[1];
		if (atoi(test_time_chr) > 0) {
			test_time = atoi(test_time_chr);
	    	wmg_printf(MSG_INFO,"INFO: test times: %d\n", test_time);
		}
	}



    event_label=rand();
	/* turn on wifi */
    p_wifi_interface = aw_wifi_on(wifi_state_handle, event_label);
	if (p_wifi_interface == NULL) {
		wmg_printf(MSG_ERROR,"Test failed: wifi on failed \n");
		return -1;
	}

	connection_info = (connection_status *)calloc(1, sizeof(connection_status));
	if (!connection_info) {
		wmg_printf(MSG_ERROR,"malloc for connection_info failed \n");
		return -1;
	}


	while (test_time) {

		ret = p_wifi_interface->is_ap_connected(ssid, &ssid_len);
		if (ret > 0) {
			wmg_printf(MSG_INFO,"\nWIFI is connected to AP: %s\n", ssid);
			is_wifi_connected = true;
		} else {
			wmg_printf(MSG_INFO,"\nWIFI is disconnected to AP\n");
			is_wifi_connected = false;
		}

    	if (is_wifi_connected) {
			try_conn_time = 0;
			p_wifi_interface->get_connection_info(connection_info);
			printf("\n*******************************************************************************\n");
			printf("wifi_get_connection_info_test: get connection infomation successfully!\n");
			printf("Connected AP: %s\n", connection_info->ssid);
			printf("IP address: %s\n", connection_info->ip_address);
			printf("frequency: %d\n", connection_info->freq);
			printf("RSSI: %d\n", connection_info->rssi);
			printf("link_speed: %d\n", connection_info->link_speed);
			printf("noise: %d\n", connection_info->noise);
			printf("**********************************************************************************\n");
			test_time--;
			printf("******%s: %d times left to test!******\n", __func__, test_time);
			sleep(5);
		} else {
			try_conn_time++;
			if (try_conn_time > MAX_WAIT_CONNECTION_TIME) {
				printf("\n*******************************************************************************\n");
				printf("wifi_get_connection_info_test: WIFI disconnected, reach max waiting time, exit!\n");
				printf("**********************************************************************************\n");
				exit(-1);
			}
			printf("\n*******************************************************************************\n");
			printf("wifi_get_connection_info_test: WIFI disconnected, waiting for connection: %d times\n", try_conn_time);
			printf("**********************************************************************************\n");
			sleep(3);
		}
	}

	printf("\n*******************************************************************************\n");
	printf("wifi_get_connection_info_test: get connection infomation complete!\n");
	printf("\n*******************************************************************************\n");

    return 0;
}
