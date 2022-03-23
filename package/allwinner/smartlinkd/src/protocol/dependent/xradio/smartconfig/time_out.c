/*
 * =============================================================================
 *
 *       Filename:  main.c
 *
 *    Description:
 *
 *     Created on:  2016Äê11ÔÂ28ÈÕ
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

#include "decode.h"
#include "time_out.h"

#ifdef USE_CTRL_
extern int delay_set;/*the time of device stay in channel*/
#endif
static int out_count = 0;
static int out_s = 3;

void set_timeout(int s){
	out_s = s;
}

void clear_out_count(void){
	out_count = 0;
}

/*FIXME TODO find a better way to check out time out ,like pthread_cond_timewait();*/
void *time_out(void *arg){
	while(1) {

		usleep(1000*1000);
		out_count ++;
		if(get_status() == XRSC_STATUS_COMPLETE)
				return NULL;
		if(out_count > out_s) {
			printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			printf("!!!!!!TIME IS OUT ,RESET!!!!!\n");
			printf("!!!!!!TIME IS OUT ,RESET!!!!!\n");
			printf("!!!!!!TIME IS OUT ,RESET!!!!!\n");
			printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			/*add channel delay time to make the device get more data in single channle*/
#ifdef USE_CTRL_
			delay_set += 100;
			if(delay_set > 220) {
				delay_set = 220;
			}
#endif
			if(get_status() != XRSC_STATUS_COMPLETE)
				restart_packet_decoed(p_lead_code(), p_ssid_pwd_data());/*restart the receive*/
			else
				return NULL;
			out_count = 0;
		}
	}
	return NULL;
}
