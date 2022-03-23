/*
 * =============================================================================
 *
 *       Filename:  decode.h
 *
 *    Description:
 *
 *     Created on:  2016年11月21日
 *
 *     Created by:  liuyitong
 *
 * =============================================================================
 */

#ifndef __DECODE__
#define __DECODE__


#include "def.h"

typedef enum{
	XRSC_STATUS_SEARCHING,
	XRSC_STATUS_SRC_LOCKED,
	XRSC_STATUS_COMPLETE
} xrsc_status_t;

struct ssidpwd_complete{
	u8 *ssid ;
	u8 *pwd ;
	int ssid_size ;
	int pwd_size ;
	int round_num;
};

struct ssid_pwd{
	u8 *ssid ;
	u8 *pwd ;
	int ssid_size ;
	int pwd_size ;
	int ssid_right ;
	int pwd_right ;
};

struct lead_info{
	int channel ;
	int ssidpwd_size ;
	int pwd_size ;
	int round_num ;

	int packet_sum ;
	int lead_complete ;
	int packet_count ;
	u8 *count_pkt ;

	u8 *ssidpwd_data ;
	u8 *source_mac ;
};

void printADDR(char *name, u8 *data, int level);
void set_status(xrsc_status_t status);
xrsc_status_t get_status(void);
int packet_filter(u8 * packet, struct lead_info* lead_code);
int check_leadcode(struct lead_info *lead_code);
void locked_sourcemac (u8 *source_mac, struct lead_info *lead_code);
int get_leadcode(u8 *packet, struct lead_info *lead_code);
struct ssid_pwd *p_ssid_pwd_data(void);
struct lead_info *p_lead_code(void);
int packet_deoced (u8 *packet, struct lead_info *lead_code);
int get_channel(void);
int packet_deoced (u8 *packet, struct lead_info *lead_code);
struct ssidpwd_complete *p_ssidpwd_complete_(void);
void restart_packet_decoed(struct lead_info *lead_code , struct ssid_pwd *ssid_pwd_data);

#endif //__DECODE__
