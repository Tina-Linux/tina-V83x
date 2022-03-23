/*
 * =============================================================================
 *
 *       Filename:  wifi_cntrl.h
 *
 *    Description:
 *
 *     Created on:  2016年11月21日
 *
 *     Created by:  liuyitong
 *
 * =============================================================================
 */
#ifndef __SCAN_H__
#define __SCAN_H__

struct wifi_info{
	unsigned char *source_ssid;
	char *find_ssid;
	int channel;
	int encryption; /*0: open, 1: wpa, 2: wep*/
};

int get_wifi(struct wifi_info *wifi, unsigned char *source_ssid);
struct wifi_info *p_wifi(void);

#endif
