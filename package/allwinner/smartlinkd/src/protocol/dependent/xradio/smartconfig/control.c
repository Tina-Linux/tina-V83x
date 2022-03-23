/*
 * =============================================================================
 *
 *       Filename:  control.c
 *
 *    Description:
 *
 *     Created on:  2015年12月15日
 *
 *     Created by:
 *
 * =============================================================================
 */

#ifdef USE_CTRL_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "decode.h"
#include "def.h"
#include "time_out.h"

int g_channel_locked = 0;
int g_dump_packets = 0;
int delay_set = 20;
int ctrl_display_pkt = 0;
extern int g_stop;
extern enum loglevel g_debuglevel;
extern int xr_system(char*);
extern void clear_wpas();

void wsm_dump_switch(int dump){
	char syscmd[128];
	int onoff = dump ? 1 : 0;
	sprintf(syscmd, "echo %d > `ls /sys/kernel/debug/ieee80211/phy*/xradio/wsm_dumps`", onoff);
	xr_system(syscmd);

}

void sig_handler(int signo)
{
	  if (signo == SIGINT) {

		  clear_wpas();
		  wsm_dump_switch(0);
		  //wifi_down();
		  exit(0);
	  }
}


void* ctrlThread(void* p){
	  if (signal(SIGINT, sig_handler) == SIG_ERR)
		    LOG(ALWAYS, "\ncan't catch SIGINT\n");
	LOG(INFO, "waiting cmd ...\n");
	char cmd[128] = {0};
	return NULL;

	while(scanf("%s", cmd)!=0) {
		if(!strcmp(cmd, "s") || !strcmp(cmd, "stop")) {
			g_stop = 1;
		}else if(!strcmp(cmd, "d")) {
			if(g_debuglevel<DETAIL)
				g_debuglevel++;
		}else if(!strcmp(cmd, "r")) {
			if(g_debuglevel>ALWAYS)
				g_debuglevel--;
		}else if(!strcmp(cmd, "cc")) {
			int channel = -1;
			scanf("%d", &channel);
			char syscmd[128];
			sprintf(syscmd, "iw wlan0 set channel %d", channel);
			if(channel > 0 && channel < 14)
				xr_system(syscmd);

		}else if(!strcmp(cmd, "lc")) {
			g_channel_locked = 1;

		}else if(!strcmp(cmd, "ul")) {
			g_channel_locked = 0;

		}else if(!strcmp(cmd, "fr")) {
			char syscmd[128];
			sprintf(syscmd, "cat /sys/kernel/debug/ieee80211/phy*/frequency");
			xr_system(syscmd);

		}else if(!strcmp(cmd, "n")) {
			char syscmd[128];
			sprintf(syscmd, "cat /sys/kernel/debug/ieee80211/phy*/xradio/counters");
			xr_system(syscmd);

		}else if(!strcmp(cmd, "dp")) {
			g_dump_packets = 1;

		}else if(!strcmp(cmd, "pd")) {
			g_dump_packets = 0;

		}else if(!strcmp(cmd, "dw")) {
			wsm_dump_switch(1);

		}else if(!strcmp(cmd, "wd")) {
			wsm_dump_switch(0);

		}else if(!strcmp(cmd, "sf")) {
			char syscmd[128];
			sprintf(syscmd, "echo 0x7fff 0x7fff > `ls /sys/kernel/debug/ieee80211/phy*/xradio/parse_flags`");
			xr_system(syscmd);
		}else if(!strcmp(cmd, "fs")){
			char syscmd[128];
			sprintf(syscmd, "echo 0x0 0x0 > `ls /sys/kernel/debug/ieee80211/phy*/xradio/parse_flags`");
			xr_system(syscmd);
		}else if(!strcmp(cmd, "ex")) {
			g_stop = 1;
		}else if(!strcmp(cmd, "dis")) {
			scanf("%d", &ctrl_display_pkt);
		}else if(!strcmp(cmd, "de")) {
			scanf("%d", &delay_set);
		}else if(!strcmp(cmd, "rest")) {
			restart_packet_decoed(p_lead_code(), p_ssid_pwd_data());
		}else if(!strcmp(cmd, "out")) {
			int out  = 0;
			scanf("%d", &out);
			set_timeout(out);
		}else if(!strcmp(cmd, "help")) {
			LOG(ALWAYS, "d - debug level add\n"
				"r - debug level reduce\n"
				"s - stop receiving\n"
				"n - show dbg conuters \n"
				"cc <ch> - change channel\n"
				"fr - frequnecy show\n"
				"lc - lock channel \n"
				"ul - unlock channel \n"
				"dp - dump packets \n"
				"pd - dump packets cancel\n"
				"dw - dump wsm\n"
				"wd - dump wsm cancel \n"
				"sf - set parse_flag \n"
				"fs - unset parse_flag \n"
				"ex - exit(1) \n"
				"dis 1/0 - 1: display pkt , 0: off \n"
				"out - set time out reset time\n"
				);
		}

	}
	return NULL;
}

#endif //USE_CTRL_
