#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <wmg_debug.h>
#include <wifi_udhcpc.h>
#include <wifi_intf.h>
#include <smg_log.h>
#include <sm_link_manager.h>

#include <wmg_debug.h>
#include <wifi_udhcpc.h>
#include <wifi_intf.h>


const aw_wifi_interface_t *p_wifi_interface = NULL;

enum wmgState state;

static int event_label;

static void wifi_state_handle(struct Manager *w, int event_label)
{
    wmg_printf(MSG_DEBUG,"event_label 0x%x\n", event_label);

    switch(w->StaEvt.state)
    {
		 case CONNECTING:
		 {
			 wmg_printf(MSG_INFO,"Connecting to the network(%s)......\n",w->ssid);
			 state = CONNECTING;
			 break;
		 }
		 case CONNECTED:
		 {
			 wmg_printf(MSG_INFO,"Connected to the AP(%s)\n",w->ssid);
			 state = CONNECTED;
			 start_udhcpc();
			 break;
		 }

		 case OBTAINING_IP:
		 {
			 wmg_printf(MSG_INFO,"Getting ip address(%s)......\n",w->ssid);
			 state = OBTAINING_IP;
			 break;
		 }

		 case NETWORK_CONNECTED:
		 {
			 wmg_printf(MSG_DEBUG,"Successful network connection(%s)\n",w->ssid);
			 state = NETWORK_CONNECTED;
			 break;
		 }
		case DISCONNECTED:
		{
		    wmg_printf(MSG_ERROR,"Disconnected,the reason:%s\n",wmg_event_txt(w->StaEvt.event));
		    state = DISCONNECTED;
		    break;
		}
    }
}
void print_help()
{
	printf("---------------------------------------------------------------------------------\n");
	printf("example:  smartlink_demo V\n\n");
	printf("V:take the following value\n");
	printf("\t0-softap\n");
	printf("\t1-soundwave\n");
	printf("\t2-xradio smartconfig\n");
	printf("\t3-xradio airkiss\n");
	printf("\t4-ampark (ap6212...) airkiss\n");
	printf("\t5-ampark (ap6212...) cooee\n");
	printf("\t6-composite (softap & soudwave)\n");
	printf("---------------------------------------------------------------------------------\n");
}
int main(int argc, char* argv[])
{
	struct net_info network_info;
	int ret = -1;
    int proto = 0;
	void *sound_wave_argv[] = {
		"1",
		"default",
		"0",
		"16000",
		"60",
	};

	void *xr_airkiss_argv[] = {
		"1",
		"60",
		"1234567890123456",
		"wlan0"
	};

    if(argc == 2 && (!strcmp(argv[1],"--help") || !strcmp(argv[1], "-h") || !strcmp(argv[1], "-H"))){
	    print_help();
	    return -1;
    }

    if(argc > 1){
        proto = atoi(argv[1]);
    }

	smg_set_debug_level(SMG_MSGDUMP);

	if(proto <6 && proto >= 0) {
		struct proto_params params;

		ret = sm_link_init(1);
		if(ret < 0)
			return 0;

		switch(proto) {
		case 0 :
			sm_link_protocol_enable(SM_LINK_SOFTAP,&params,1);
			ret = sm_link_wait_get_results(SM_LINK_SOFTAP,&network_info);
			break;

		case 1 :

			params.argv = sound_wave_argv;

			sm_link_protocol_enable(SM_LINK_SOUND_WAVE,&params,1);
			ret = sm_link_wait_get_results(SM_LINK_SOUND_WAVE,&network_info);
			break;

		case 2 :
			sm_link_protocol_enable(SM_LINK_XR_SMARTCONFIG,&params,1);
			ret = sm_link_wait_get_results(SM_LINK_XR_SMARTCONFIG,&network_info);
			break;

		case 3 :

			params.argv = xr_airkiss_argv;
			sm_link_protocol_enable(SM_LINK_XR_AIRKISS,&params,1);
			ret = sm_link_wait_get_results(SM_LINK_XR_AIRKISS,&network_info);
			break;
		case 4 :
			smg_printf(SMG_INFO,"future support!!!\n");
			break;
		case 5 :
			smg_printf(SMG_INFO,"future support!!!\n");
			break;
		}
	}

	if(proto == 6) {
		#define WORK_PROTOCOL_NUM  2
		#define COM_PROTOCOL      (SM_LINK_SOFTAP|SM_LINK_SOUND_WAVE)
		struct proto_params params_com[WORK_PROTOCOL_NUM];

		params_com[0].argv = sound_wave_argv;
		params_com[1].argv = NULL;

		ret = sm_link_init(WORK_PROTOCOL_NUM);
		if(ret < 0)
			return 0;
		sm_link_protocol_enable(COM_PROTOCOL,params_com,WORK_PROTOCOL_NUM);
		ret = sm_link_wait_get_results(COM_PROTOCOL,&network_info);
	}

	event_label = rand();
//	while(1) {
//		sleep(1);
//	}
	if(ret > 0) {
		printf("ssid:%s,pwd:%s\n",network_info.ssid,network_info.password);

		p_wifi_interface = aw_wifi_on(wifi_state_handle, event_label);

		if(p_wifi_interface == NULL) {
			printf("wifi on failed \n");
		} else {
			if(state == NETWORK_CONNECTED) {
				smg_printf(SMG_INFO,"auto connected Successful  !!!!\n");
				smg_printf(SMG_INFO,"==================================\n");
		    }else {
				p_wifi_interface->connect_ap(network_info.ssid,network_info.password,event_label);
				if(state == NETWORK_CONNECTED) {
					smg_printf(SMG_INFO,"connected Successful  !!!!\n");
					smg_printf(SMG_INFO,"==================================\n");
				}else
					smg_printf(SMG_INFO,"connected failed  !!!!\n");
			}
		}
	} else {
		smg_printf(SMG_ERROR,"receive failed\n");
	}

end:
	sm_link_deinit();

	return 0;
}
