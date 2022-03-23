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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "scan.h"
#include "decode.h"

#define NUM_NULL -1

enum enc {
	OPEN,
	WPA,
	WEP
};

static char *p_data = NULL;
static const char *ssid_sign = "SSID: ";
static const char *channel_sign = "* primary channel: ";
struct wifi_info wifi_list = {NULL, NULL, NUM_NULL, NUM_NULL};


struct wifi_info *p_wifi(void){
	return &wifi_list;
}

int get_wifi(struct wifi_info *wifi_list, unsigned char *source_ssid){

	wifi_list->source_ssid = source_ssid;
	if(wifi_list->source_ssid == NULL){
		printf("wifi_list.sourec_ssid = NULL\n");
		return 0;
	}
	FILE *fp = fopen("/tmp/wifi_list.txt", "r");
	if(fp == NULL) {
		printf("can't find %s\n" ,"/tmp/wifi_list.txt");
		return 0;
	}
	p_data = (char *)malloc(200);
	while(1) {
		memset(p_data, 0, 200);
		char *data = fgets(p_data, 200, fp);
		if(data == NULL) {
			printf("file is end \n");
			if(wifi_list->find_ssid != NULL) {
				free(p_data);
				fclose(fp);
				return 1;
			}
			free(p_data);
			fclose(fp);
			return 0;
		}

		if(wifi_list->find_ssid == NULL) {
			char *s = strstr(p_data, ssid_sign);
			if(s == NULL)
				continue;
			s = s + 6;
			int len = strlen((const char *)s) - 1;
			if(len != strlen((const char *)wifi_list->source_ssid))
				continue;
			if(strncmp((const char *)s, (const char *)wifi_list->source_ssid, len) == 0) {
				wifi_list->find_ssid = s;
				printf("%s", wifi_list->find_ssid);
			}
		}else if(wifi_list->channel <= 0) {
			char *s = strstr(p_data, ssid_sign);
			if(s != NULL) {
				free(p_data);
				fclose(fp);
				return 1;
			}
			s = strstr(p_data, channel_sign);
			if(s == NULL)
				continue;
			s = s + 19;
			wifi_list->channel = atoi(s);
			printf("channel is %d\n", wifi_list->channel);
		}else if(wifi_list->encryption == NUM_NULL) {
			char *s = strstr(p_data, ssid_sign);
			if(s !=NULL) {
				wifi_list->encryption = OPEN;
				printf("encryption mode is OPEN\n");
				free(p_data);
				fclose(fp);
				return 1;
			}
			s = strstr(p_data, "WPA:");
			if(s !=NULL) {
				wifi_list->encryption = WPA;
				printf("encryption mode is WPA\n");
				free(p_data);
				fclose(fp);
				return 1;
			}
			s = strstr(p_data, "WEP:");
			if(s !=NULL) {
				wifi_list->encryption = WEP;
				printf("encryption mode is WEP\n");
				free(p_data);
				fclose(fp);
				return 1;
			}
		}
	}
}
