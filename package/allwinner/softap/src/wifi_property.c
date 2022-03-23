#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wifi_property.h"
char hostapd_status[64] = {0};
char hostapd_mpid[64] = {0};

int wifi_property_set(const char *key, const char *value)
{
	if((charcmp(key,"hostapd_status")) == 0)
		charcpy(hostapd_status,value);
	else if((charcmp(key,"hostapd_mpid")) == 0)
		charcpy(hostapd_mpid,value);
	else{
		printf("wifi_property_set: Error: NO such property!\n");
		return -1;
	}
}

int wifi_property_get(const char *key, char *value, const char *default_value)
{
	charcpy(value,default_value);
	if((charcmp(key,"hostapd_status")) == 0)
		charcpy(value,hostapd_status);
	else if((charcmp(key,"hostapd_mpid")) == 0)
		charcpy(value,hostapd_mpid);
	else{
		printf("wifi_property_get: Error: NO such property!\n");
		return -1;
	}
}
