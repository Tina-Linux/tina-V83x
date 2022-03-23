/*
 * =============================================================================
 *
 *       Filename:  wifi_cntrl.h
 *
 *    Description:
 *
 *     Created on:  2016Äê11ÔÂ21ÈÕ
 *
 *     Created by:  liuyitong
 *
 * =============================================================================
 */

#ifndef __WIFI_CNTRL_H__
#define __WIFI_CNTRL_H__
#include "decode.h"

void xr_system(const char* cmd);
void clear_wpas(void);
void wifi_disable(const char* ifName);
void wifi_enable(const char* ifName);
void wifi_set_channel(const int ch);
void wifi_monitor_on(void);
void wifi_station_on(void);
int xr_connect_ap(void);
int xr_request_ip(void);
int xr_sendack(struct ssidpwd_complete *complete);
int check_ip_timeout(const int timeout);
int scan(void);
int gen_wpafile(struct ssidpwd_complete *complete,  int encrypt_type);
int gen_hide_ssid(struct ssidpwd_complete *complete,  int encrypt_type);
#endif//__WIFI_CNTRL_H__