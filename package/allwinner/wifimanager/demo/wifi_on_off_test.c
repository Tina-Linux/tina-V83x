#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wifi_intf.h>
#include <pthread.h>
#include <sys/time.h>
#include "wmg_debug.h"
#include "wifi_udhcpc.h"

#define WIFI_ON_OFF_TEST_CNTS  10

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
	wmg_printf(MSG_INFO,"NAME:\n\twifi_on_off_test\n");
	wmg_printf(MSG_INFO,"DESCRIPTION:\n\ttest the stability of wifi module\n");
	wmg_printf(MSG_INFO,"USAGE:\n\twifi_on_off_test <test_times> <level>\n");
	wmg_printf(MSG_INFO,"PARAMS:\n");
	wmg_printf(MSG_INFO,"\ttest_times : test times of the action(wifi_on and wifi_off)\n");
	wmg_printf(MSG_INFO,"\t             default test times: 10\n");
	wmg_printf(MSG_INFO,"\tlevel      : print level(d0~d5).larger value,more info.para is not required,default d2.\n");
	wmg_printf(MSG_INFO,"--------------------------------------MORE---------------------------------------\n");
	wmg_printf(MSG_INFO,"The way to get help information:\n");
	wmg_printf(MSG_INFO,"\twifi_on_off_test --help\n");
	wmg_printf(MSG_INFO,"\twifi_on_off_test -h\n");
	wmg_printf(MSG_INFO,"\twifi_on_off_test -H\n");
	wmg_printf(MSG_INFO,"---------------------------------------------------------------------------------\n");
}
/*
*/
static int set_log_level(int argv, char *argc[])
{
	if(argv >=3 && !strncmp(argc[2],"d",1)){
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
    int onOffTestCnt = WIFI_ON_OFF_TEST_CNTS;
    const aw_wifi_interface_t *p_wifi_interface = NULL;
    unsigned long long int on_sec = 0, on_microsec = 0;
    unsigned long long int off_sec = 0, off_microsec = 0;
    struct timeval tv1;
    struct timeval tv2;

    if(argv == 2 && (!strcmp(argc[1],"--help") || !strcmp(argc[1], "-h") || !strcmp(argc[1], "-H"))){
	    print_help();
	    return -1;
    }

    if(argv < 1 || argv >3){
	    wmg_printf(MSG_ERROR,"ERROR:No need other paras\n");
	    print_help();
	    return -1;
    }
    if(set_log_level(argv,argc))
	    return -1;

    if(NULL == argc[1])
		onOffTestCnt = WIFI_ON_OFF_TEST_CNTS;
    else
		onOffTestCnt = atoi(argc[1]);

    event_label=rand();
	wmg_printf(MSG_INFO,"on off test %d times\n",onOffTestCnt);
    for(i=0; i<onOffTestCnt;i++)
    {
        wmg_printf(MSG_INFO,"\n***************************\n");
        wmg_printf(MSG_INFO,"Do wifi on off test %d times\n", i);
        wmg_printf(MSG_INFO,"****************************\n");


	gettimeofday(&tv1, NULL);
        event_label++;
        /* turn on wifi */
        p_wifi_interface = aw_wifi_on(wifi_state_handle, event_label);
        if(p_wifi_interface == NULL)
        {
	    wmg_printf(MSG_ERROR,"Test failed: wifi on failed \n");
            return -1;
        }
        gettimeofday(&tv2, NULL);
        on_microsec = (tv2.tv_sec*1000000 + tv2.tv_usec)-(tv1.tv_sec*1000000 + tv1.tv_usec);
        on_sec = on_microsec/1000000;
        on_microsec = on_microsec%1000000;
	wmg_printf(MSG_INFO,"==========================================\n");
	wmg_printf(MSG_INFO,"wifi on time: %llu.%-llu seconds\n", on_sec, on_microsec);
	wmg_printf(MSG_INFO,"==========================================\n");

        /* test */
        usleep(10000);

        gettimeofday(&tv1, NULL);
        /* turn off wifi */
        ret = aw_wifi_off(p_wifi_interface);
        if(ret < 0)
        {
            wmg_printf(MSG_ERROR,"Test failed: wifi off error!\n");
            return -1;
        }

        gettimeofday(&tv2, NULL);
        off_microsec = (tv2.tv_sec*1000000 + tv2.tv_usec)-(tv1.tv_sec*1000000 + tv1.tv_usec);
        off_sec = off_microsec/1000000;
        off_microsec = off_microsec%1000000;
	wmg_printf(MSG_INFO,"==========================================\n");
	wmg_printf(MSG_INFO,"wifi off time: %llu.%-llu seconds\n", off_sec, off_microsec);
	wmg_printf(MSG_INFO,"==========================================\n");
    }

    wmg_printf(MSG_INFO,"********************************\n");
    wmg_printf(MSG_INFO,"Test completed: Success!\n");
    wmg_printf(MSG_INFO,"********************************\n");

    return 0;
}
