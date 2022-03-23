#ifndef __SOFTAP_INTF_H

#define __SOFTAP_INTF_H

#if __cplusplus
extern "C"{
#endif

#ifndef IW_UP_BROADCOM_WLAN1
#define IW_UP_BROADCOM_WLAN1 0
#endif

typedef enum {
SOFTAP_NONE = 0,
SOFTAP_WPA_PSK,
SOFTAP_WPA2_PSK,
}tSOFTAP_KEY_MGMT;


int aw_softap_reload_firmware(char *ap_sta);
int aw_is_softap_started();
int aw_softap_init();
int aw_softap_deinit();
int aw_softap_config(char *ssid, char *psk, tSOFTAP_KEY_MGMT key_mgmt, char *interface, char *channel, char *broadcast_hidden);
int aw_softap_save_config();
int aw_softap_enable();
int aw_softap_disable();
int aw_softap_router_config(char *ip, char *netmask);
int aw_softap_start_udhcp_dns_server();
int aw_softap_enable_data_forward(char *interface);


#if __cplusplus
} /*extern "c"*/
#endif

#endif
