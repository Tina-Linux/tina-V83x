#ifndef __BLUETOOTH_INTERFACE_H__
#define __BLUETOOTH_INTERFACE_H__

#ifndef BT_ADDR_LENTH
#define BT_ADDR_LENTH     6
typedef unsigned char S_BT_ADDR[BT_ADDR_LENTH];
#endif

#ifndef AVK_ELEMENT_ATTR_LENTH
#define AVK_ELEMENT_ATTR_LENTH 102
#endif

typedef struct{
    unsigned char title[AVK_ELEMENT_ATTR_LENTH];
    unsigned char artist[AVK_ELEMENT_ATTR_LENTH];
    unsigned char album[AVK_ELEMENT_ATTR_LENTH];
    unsigned char playing_time[AVK_ELEMENT_ATTR_LENTH];
}s_avk_element_attr_t;

typedef enum{
    BT_DISC_NEW_EVT,
    BT_DISC_CMPL_EVT,
    BT_DISC_DEV_INFO_EVT,
    BT_DISC_REMOTE_NAME_EVT,
}tBT_DISC_EVT;

typedef void (tBT_DISC_CBACK)(tBT_DISC_EVT event);

/* define bt app event */
typedef enum{
    APP_AVK_CONNECTED_EVT = 0,
    APP_AVK_DISCONNECTED_EVT = 1,
    APP_AVK_START_EVT = 2,             /* stream data transfer started */
    APP_AVK_STOP_EVT = 3,	       /* stream data transfer stopped */
    APP_AVK_CONNECT_COMPLETED_EVT = 4,
    APP_AUDIO_OPEN_REQ_EVT = 5,
    APP_HS_CONNECTED_EVT = 6,
    APP_HS_DISCONNECTED_EVT = 7,
    APP_HS_AUDIO_OPEN_EVT = 8,
    APP_HS_AUDIO_CLOSE_EVT = 9,
    APP_MGR_DISC_RESULTS = 10,
    APP_HS_RING_EVT = 11
}APP_BT_EVENT;

typedef struct{

#define BT_UUID_LEN  37

    int server_num;
    int service_num;
    int char_attr_num;
    char attr_uuid128[BT_UUID_LEN];
    unsigned char  *value;
    int            len;
}s_tBleCallbackData;

/* define bt app event */
typedef enum{
    APP_BLE_SE_CONNECTED_EVT = 0,
    APP_BLE_SE_DISCONNECTED_EVT = 1,
    APP_BLE_SE_READ_EVT = 2,
    APP_BLE_SE_WRITE_EVT = 3
}APP_BLE_EVENT;

int bluetooth_init();
int bluetooth_start(void *p, char *p_conf);
void bluetooth_stop();
void start_app_avk();
void start_app_avk_no_avrcp();
void stop_app_avk();
void s_set_bt_name(const char *name);
int  s_get_bd_addr(unsigned char bd_addr[]);
void s_set_discoverable(int enable);
void s_set_connectable(int enable);
void s_start_discovery(int time);
int  s_get_disc_results(char *results, int *len);
void s_set_volume(int volume);
void s_set_volume_up();
void s_set_volume_down();
int  s_connect_auto();
int  s_connect_dev_by_addr(S_BT_ADDR s_bt_addr);
void s_disconnect();
void s_avk_play();
void s_avk_pause();
void s_avk_stop();
void s_avk_close_pcm_alsa();
void s_avk_resume_pcm_alsa();
void s_avk_play_previous();
void s_avk_play_next();
int  s_avk_get_element_attr(s_avk_element_attr_t *p_s_avk_element);
void start_app_hs();
void stop_app_hs();
void s_hs_pick_up();
void s_hs_hung_up();
int s_initial_disc_param();
int s_set_discovery_callback(tBT_DISC_CBACK *u_custom_disc_cback);
int s_set_discovery_bd_addr(unsigned char bd_addr[]);
int s_set_discovery_name(char bt_name[]);
void s_start_discovery_with_get_rssi();
int s_get_disc_results_with_rssi(char *disc_results, int *len);

void start_app_ble();
void stop_app_ble();
int  s_ble_server_register(unsigned short uuid, void *p_cback);
int s_ble_server_deregister(int server_num);
int s_ble_server_create_service(int server_num, const char *service_uuid128, int char_num, int is_primary);
int s_ble_server_add_char(int server_num, int service_num, const char *char_uuid128,
    int is_descript, unsigned short attribute_permission, unsigned short characteristic_property);
int s_ble_server_start_service(int server_num, int srvc_attr_num);
int s_ble_server_config_adv_data(const char *srvc_uuid_array,
        unsigned char app_ble_adv_value[], int data_len,
        unsigned short appearance_data, int is_scan_rsp);
int s_ble_server_send_indication(int server_num, int attr_num, unsigned char data[], int data_len);
int s_ble_server_send_read_rsp(void *p_cb_data, unsigned char data[], int data_len);
void s_set_ble_discoverable(int enable);
void s_set_ble_connectable(int enable);

#endif /* __BLUETOOTH_INTERFACE_H__ */
