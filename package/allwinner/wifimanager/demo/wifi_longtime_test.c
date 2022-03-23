#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wifi_intf.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#include "wmg_debug.h"
#include "wifi_udhcpc.h"

#define TEST_TIMES 1001
int terminate_flag = 0;
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
	wmg_printf(MSG_INFO,"NAME:\n\twifi_longtime_test\n");
	wmg_printf(MSG_INFO,"DESCRIPTION:\n\tTest the stability of wifi module(see the test result in file:/etc/test_result).\n");
	wmg_printf(MSG_INFO,"USAGE:\n\twifi_longtime_test <ssid> <passwd> <test_times> <level>\n");
	wmg_printf(MSG_INFO,"PARAMS:\n\tssid       : ssid of the AP\n");
	wmg_printf(MSG_INFO,"\tpasswd     : Password of the AP\n");
	wmg_printf(MSG_INFO,"\ttest_times : test times of the action(connect AP and disconnect AP)\n");
	wmg_printf(MSG_INFO,"\t             default test times: %d\n",TEST_TIMES);
	wmg_printf(MSG_INFO,"\tlevel      : print level(d0~d5).larger value,more info.para is not required,default d2.\n");
	wmg_printf(MSG_INFO,"--------------------------------------MORE---------------------------------------\n");
	wmg_printf(MSG_INFO,"The way to get help information:\n");
	wmg_printf(MSG_INFO,"\twifi_longtime_test --help\n");
	wmg_printf(MSG_INFO,"\twifi_longtime_test -h\n");
	wmg_printf(MSG_INFO,"\twifi_longtime_test -H\n");
	wmg_printf(MSG_INFO,"---------------------------------------------------------------------------------\n");
}

static void signal_handler(int signo)
{
    wmg_printf(MSG_DEBUG,"Recieve signal: SIGINT\n");
    terminate_flag = 1;
}

static int set_log_level(int argv, char *argc[])
{
	if(argv >=5 && !strncmp(argc[4],"d",1)){
		char *debug = argc[4];
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

/*
 *argc[1]   ap ssid
 *argc[2]   ap passwd
 *agrc[3]   [test times]
 *see the test result in file:/etc/test_result
*/
int main(int argv, char *argc[]){
    int i = 0, ret = 0, len = 0;
    int times = 0, event_label = 0;;
    char ssid[256] = {0}, scan_results[4096] = {0};
    const aw_wifi_interface_t *p_wifi_interface = NULL;
    unsigned long long int total_con_sec = 0, total_con_microsec = 0;
    unsigned long long int con_sec = 0, con_microsec = 0;
    unsigned long long int total_disc_sec = 0, total_disc_microsec = 0;
    unsigned long long int discon_sec = 0, discon_microsec = 0;
	int flag = 0;
    struct timeval tv1;
    struct timeval tv2;
    int success_times = 0, fail_times = 0, timeout_times = 0;
    char prt_buf[256] = {0};
    int ttest_times = 0;
	int may_be_failed_count = 0;
    if(argv == 2 && (!strcmp(argc[1],"--help") || !strcmp(argc[1], "-h") || !strcmp(argc[1], "-H"))){
	print_help();
	return -1;
    }

    if(NULL == argc[1]){
	print_help();
	return -1;
    }
    if(NULL == argc[3])
	ttest_times = TEST_TIMES;
    else
	ttest_times = atoi(argc[3]);
	if(set_log_level(argv,argc))
		return -1;
	wmg_debug_open_file("/etc/wifi_long_time_test.log");

    wmg_printf(MSG_INFO,"\n*********************************\n");
    wmg_printf(MSG_INFO,"***Start wifi long test!***\n");
    wmg_printf(MSG_INFO,"*********************************\n");

	wmg_printf(MSG_INFO,"Test times: %d\n",ttest_times);

	sleep(3);

    if(signal(SIGINT, signal_handler) == SIG_ERR)
        wmg_printf(MSG_ERROR,"signal(SIGINT) error\n");

    event_label = rand();
    p_wifi_interface = aw_wifi_on(wifi_state_handle, event_label);
    if(p_wifi_interface == NULL){
        wmg_printf(MSG_ERROR,"wifi on failed\n");
        return -1;
    }

	if(aw_wifi_get_wifi_state() == NETWORK_CONNECTED){
		wmg_printf(MSG_INFO,"auto connected Successful  !!!!\n");
		wmg_printf(MSG_INFO,"==================================\n");
	}

    //pthread_create(&app_scan_tid, NULL, &app_scan_task,(void *)p_wifi_interface);
    for(i=0;i<ttest_times;i++){
	    gettimeofday(&tv1, NULL);
		event_label++;

#if 0
		if(flag == 0){
			p_wifi_interface->connect_ap("AWTest", "1qaz@WSX", event_label);
			flag = 1;
		}else {
			p_wifi_interface->connect_ap("AW-TEST-BU1-PSW", "Aa123456", event_label);
			flag = 0;
		}
#else
		p_wifi_interface->connect_ap(argc[1], argc[2], event_label);
#endif

try_connect:
		if(aw_wifi_get_wifi_state() == NETWORK_CONNECTED){
			wmg_printf(MSG_INFO,"Wifi connect ap : Success!\n");
			success_times ++;
		}
		else{
			p_wifi_interface->remove_all_networks();
			sleep(1);
			may_be_failed_count ++;
			if(may_be_failed_count < 2)
				goto try_connect;

			may_be_failed_count = 0;
			fail_times ++ ;
			wmg_printf(MSG_ERROR,"Wifi connect ap : Failure!\n");
		}
		wmg_printf(MSG_INFO,"==================================\n");


        gettimeofday(&tv2, NULL);
        con_microsec = (tv2.tv_sec*1000000 + tv2.tv_usec)-(tv1.tv_sec*1000000 + tv1.tv_usec);
        con_sec = con_microsec/1000000;
        con_microsec = con_microsec%1000000;
        total_con_sec += con_sec;
        total_con_microsec += con_microsec;
		wmg_printf(MSG_INFO,"==========================================\n");
		wmg_printf(MSG_INFO,"Test Times: %d\nSuccess Times: %d\nFailed Times: %d\n",i+1,success_times,fail_times);
		wmg_printf(MSG_INFO,"Connecting time: %llu.%-llu seconds\n", con_sec, con_microsec);
		wmg_printf(MSG_INFO,"==========================================\n");

        sleep(3);

		event_label++;

        gettimeofday(&tv1, NULL);
		p_wifi_interface->disconnect_ap(event_label);

        gettimeofday(&tv2, NULL);
        discon_microsec = (tv2.tv_sec*1000000 + tv2.tv_usec)-(tv1.tv_sec*1000000 + tv1.tv_usec);
        discon_sec = discon_microsec/1000000;
        discon_microsec = discon_microsec%1000000;
        total_disc_sec += discon_sec;
        total_disc_microsec += discon_microsec;

		wmg_printf(MSG_INFO,"==========================================\n");
		wmg_printf(MSG_INFO,"Disconnecting time: %llu.%-llu seconds\n", discon_sec, discon_microsec);
		wmg_printf(MSG_INFO,"==========================================\n");

        sleep(3);

        if(1 == terminate_flag)
            break;
    }

    total_con_microsec = total_con_sec*1000000+total_con_microsec;
    total_disc_microsec = total_disc_sec*1000000+total_disc_microsec;
    wmg_printf(MSG_INFO,"Test Times:%d, Success Times:%d(including get IP timeout times:%d), Failed Times:%d\n",i,success_times,timeout_times,fail_times);
    sprintf(prt_buf,"echo \"Test Times:%d, Success Times:%d(including get IP timeout times:%d), Failed Times:%d\" > /etc/test_results",i,success_times,timeout_times,fail_times);
    system(prt_buf);
    wmg_printf(MSG_INFO,"Connecting mean time: %llu.%-llu seconds\n",(total_con_microsec/1000000)/i,(total_con_microsec/i)%1000000);
    sprintf(prt_buf,"echo \"Connecting mean time: %llu.%-llu seconds\" >> /etc/test_results",(total_con_microsec/1000000)/i,(total_con_microsec/i)%1000000);
    system(prt_buf);
    wmg_printf(MSG_INFO,"Disconnecting mean time: %llu.%-llu seconds\n",(total_disc_microsec/1000000)/i,(total_disc_microsec/i)%1000000);
    wmg_printf(MSG_INFO,prt_buf,"echo \"Disconnecting mean time: %llu.%-llu seconds\" >> /etc/test_results",(total_disc_microsec/1000000)/i,(total_disc_microsec/i)%1000000);
    system(prt_buf);
    if(success_times == ttest_times)
    {
        sprintf(prt_buf,"echo Congratulations! >> /etc/test_results");
	system(prt_buf);
    }

    wmg_printf(MSG_INFO,"******************************\n");
    wmg_printf(MSG_INFO,"Wifi connect ap test: Success!\n");
    wmg_printf(MSG_INFO,"******************************\n");
	wmg_debug_close_file();

    return 0;
}
