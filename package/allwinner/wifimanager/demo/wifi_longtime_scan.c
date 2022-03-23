#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wifi_intf.h>
#include <pthread.h>
#include "wmg_debug.h"
#include "wifi_udhcpc.h"

#define TEST_TIMES 1000

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

void print_help(){
    wmg_printf(MSG_INFO,"---------------------------------------------------------------------------------\n");
    wmg_printf(MSG_INFO,"NAME:\n\twifi_longtime_scan_test\n");
    wmg_printf(MSG_INFO,"DESCRIPTION:\n\ttest the stability of wifi scanning.\n");
    wmg_printf(MSG_INFO,"USAGE:\n\twifi_longtime_scan_test <test_times> <level>\n");
    wmg_printf(MSG_INFO,"\ttest_times : test times of the action(wifi scan).\n");
    wmg_printf(MSG_INFO,"\tlevel: print level(d0~d5).larger value,more info.para is not required,default d2.\n");
    wmg_printf(MSG_INFO,"--------------------------------------MORE---------------------------------------\n");
    wmg_printf(MSG_INFO,"The way to get help information:\n");
    wmg_printf(MSG_INFO,"\twifi_longtime_scan_test --help\n");
    wmg_printf(MSG_INFO,"\twifi_longtime_scan_test -h\n");
    wmg_printf(MSG_INFO,"\twifi_longtime_scan_test -H\n");
    wmg_printf(MSG_INFO,"---------------------------------------------------------------------------------\n");
}

static int set_log_level(int argv, char *argc[])
{
    if(argv >=3 && !strncmp(argc[2],"d",1)){
	char *debug = argc[2];
	printf("debug[1] = %c\n",debug[1]);
	if(strlen(debug) >=2 && debug[1] >= '0' &&
	    debug[1] <= '9'){
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
    int ret = 0, len = 0, i = 0;
    int test_times = 0, event_label = 0;
    char ssid[256] = {0}, scan_results[4096] = {0}, reply[4069]= {0};
    const aw_wifi_interface_t *p_wifi_interface = NULL;

    if(argv == 2 && (!strcmp(argc[1],"--help") || !strcmp(argc[1], "-h") || !strcmp(argc[1], "-H"))){
	print_help();
	return -1;
    }

    if(argv < 1 || argv >3){
	wmg_printf(MSG_ERROR,"ERROR: No need other paras!\n");
	print_help();
	return -1;
    }

    if(NULL == argc[1])
	test_times = TEST_TIMES;
    else
	test_times = atoi(argc[1]);

    wmg_printf(MSG_INFO,"\n*********************************\n");
    wmg_printf(MSG_INFO,"***Start wifi long_time scan test!***\n");
    wmg_printf(MSG_INFO,"*********************************\n");

    wmg_printf(MSG_INFO,"Test times: %d\n",test_times);

    if(set_log_level(argv,argc))
	return -1;

    event_label = rand();
    p_wifi_interface = aw_wifi_on(wifi_state_handle, event_label);
    if(p_wifi_interface == NULL){
        wmg_printf(MSG_ERROR,"wifi on failed\n");
        return -1;
    }

    for(i = 0; i < test_times; i++) {
	event_label++;
        wmg_printf(MSG_INFO,"\n***************************\n");
        wmg_printf(MSG_INFO,"Do wifi scan test %d times\n", i);
        wmg_printf(MSG_INFO,"****************************\n");

	wmg_printf(MSG_INFO,"\n*********************************\n");
	wmg_printf(MSG_INFO,"***Start scan!***\n");
	wmg_printf(MSG_INFO,"*********************************\n");

	len = 4096;
	ret=( p_wifi_interface->get_scan_results(scan_results, &len));
	wmg_printf(MSG_DEBUG,"ret of get_scan_results is %d\n", ret);
	if(ret==0)
	{
	    wmg_printf(MSG_INFO,"%s\n",scan_results);
	    wmg_printf(MSG_INFO,"******************************\n");
	    wmg_printf(MSG_INFO,"Wifi get_scan_results: Success!\n");
	    wmg_printf(MSG_INFO,"******************************\n");
	}else{
	    wmg_printf(MSG_ERROR,"Get_scan_results failed!\n");
	    return -1;
	}
    }

    wmg_printf(MSG_INFO,"******************************\n");
    wmg_printf(MSG_INFO,"Wifi long_time scan test: Success!\n");
    wmg_printf(MSG_INFO,"******************************\n");

    return 0;
}
