#ifndef __BT_MANAGER_H
#define __BT_MANAGER_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#if __cplusplus
extern "C" {
#endif

#define BTMGVERSION "Version:2.0.3.Ieccaadd7"

#define BT_LAST_CONNECT_FILE "/etc/bluetooth/last_connected"

#ifndef  CONFIG_FILE_PATH
#define CONFIG_FILE_PATH "/etc/bluetooth/aw_bluetooth"
#endif


/*log devel in control of bt_manager*/
typedef enum btmg_log_level_t {
	BTMG_LOG_LEVEL_NONE = 0,
	BTMG_LOG_LEVEL_ERROR,
	BTMG_LOG_LEVEL_WARNG,
	BTMG_LOG_LEVEL_INFO,
	BTMG_LOG_LEVEL_DEBUG,
} btmg_log_level_t;

/*BT state*/
typedef enum {
    BTMG_STATE_OFF,
    BTMG_STATE_ON,
    BTMG_STATE_TURNING_ON,
    BTMG_STATE_TURNING_OFF,
} btmg_state_t;

/*BT discovery state*/
typedef enum {
    BTMG_DISC_STARTED,
    BTMG_DISC_STOPPED_AUTO,
    BTMG_DISC_START_FAILED,
    BTMG_DISC_STOPPED_BY_USER,
} btmg_discovery_state_t;

/*BT discovery mode*/
typedef enum {
    BTMG_SCAN_MODE_NONE,
    BTMG_SCAN_MODE_CONNECTABLE,
    BTMG_SCAN_MODE_CONNECTABLE_DISCOVERABLE,
} btmg_discovery_mode_t;

/*BT bond state*/
typedef enum {
    BTMG_BOND_STATE_NONE,
    BTMG_BOND_STATE_BONDING,
    BTMG_BOND_STATE_BONDED,
} btmg_bond_state_t;

/*AVRCP commands*/
typedef enum {
    BTMG_AVRCP_PLAY,
    BTMG_AVRCP_PAUSE,
    BTMG_AVRCP_STOP,
    BTMG_AVRCP_FASTFORWARD,
    BTMG_AVRCP_REWIND,
    BTMG_AVRCP_FORWARD,
    BTMG_AVRCP_BACKWARD,
    BTMG_AVRCP_VOL_UP,
    BTMG_AVRCP_VOL_DOWN,
} btmg_avrcp_command_t;

/*A2DP_SINK connection state for callback*/
typedef enum {
    BTMG_A2DP_SINK_DISCONNECTED,
    BTMG_A2DP_SINK_CONNECTING,
    BTMG_A2DP_SINK_CONNECTED,
    BTMG_A2DP_SINK_DISCONNECTING,
} btmg_a2dp_sink_connection_state_t;

/*A2DP_SINK audio state for callback*/
typedef enum {
    BTMG_A2DP_SINK_AUDIO_SUSPENDED,
    BTMG_A2DP_SINK_AUDIO_STOPPED,
    BTMG_A2DP_SINK_AUDIO_STARTED,
} btmg_a2dp_sink_audio_state_t;

/*AVRCP play state for callback*/
/*The A2DP_SINK audio state may not be updated in time due to the BT stack implementation
* of different smartphone, while the AVRCP play state is always updated in time. So it is
*recommended to take the reported ARVCP state to judge the playing status of BT music*/
typedef enum {
    BTMG_AVRCP_PLAYSTATE_STOPPED,
    BTMG_AVRCP_PLAYSTATE_PLAYING,
    BTMG_AVRCP_PLAYSTATE_PAUSED,
    BTMG_AVRCP_PLAYSTATE_FWD_SEEK,
    BTMG_AVRCP_PLAYSTATE_REV_SEEK,
    BTMG_AVRCP_PLAYSTATE_ERROR,
} btmg_avrcp_play_state_t;

typedef struct btmg_track_info_t {
	char title[256];
	char artist[256];
	char album[256];
	char track_num[64];
	char num_tracks[64];
	char genre[256];
	char playing_time[256];
} btmg_track_info_t;

struct paired_dev {
	 char *remote_address;
	 char *remote_name;
	 short rssi;
	 bool is_bonded;
	 bool is_connected;
	 struct paired_dev *next;
};

typedef struct paired_dev bt_paried_device;


#define MAX_BT_NAME_LEN 248
#define MAX_BT_ADDR_LEN 17

/*callback functions for bt_manager itself*/
typedef void (*bt_manager_cb)(int event);

/*bt manager callback*/
typedef struct btmg_manager_callback_t {
    bt_manager_cb bt_mg_cb;
} btmg_manager_callback_t;

/*callback functions for GAP profile*/
typedef void (*bt_gap_status_cb)(btmg_state_t status);
typedef void (*bt_gap_discovery_status_cb)(btmg_discovery_state_t status);
typedef void (*bt_gap_dev_found_cb)(const char *address,const char *name, unsigned int bt_class, int rssi);
typedef void (*bt_gap_update_rssi_cb)(const char *address,const char *name, int rssi);
typedef void (*bt_gap_bond_state_cb)(btmg_bond_state_t state,const char *bd_addr,const char *name);
typedef void (*bt_gap_ssp_request_cb)(const char *bd_addr,const char *name, uint32_t pass_key);
typedef void (*bt_gap_pin_request_cb)(const char *bd_addr,const char *name);
/*gap callback*/
typedef struct btmg_gap_callback_t {
    bt_gap_status_cb gap_status_cb; /*used for return results of bt_manager_enable and status of BT*/
    bt_gap_discovery_status_cb gap_disc_status_cb; /*used for return discovery status of BT*/
    bt_gap_dev_found_cb gap_dev_found_cb; /*used for device found event*/
	bt_gap_update_rssi_cb gap_update_rssi_cb; /*update rssi for discovered and bonded devices*/
    bt_gap_bond_state_cb gap_bond_state_cb; /*used for bond state event*/
    bt_gap_ssp_request_cb gap_ssp_request_cb; /*used for ssp request*/
    bt_gap_pin_request_cb gap_pin_request_cb; /*used for pin request*/
} btmg_gap_callback_t;

/*callback functions for a2dp_sink profile*/
typedef void (*bt_a2dp_sink_connection_state_cb)(const char *bd_addr, btmg_a2dp_sink_connection_state_t state);
typedef void (*bt_a2dp_sink_audio_state_cb)(const char *bd_addr, btmg_a2dp_sink_audio_state_t state);

/*a2dp_sink callback*/
typedef struct btmg_a2dp_sink_callback_t {
    bt_a2dp_sink_connection_state_cb a2dp_sink_connection_state_cb;/*used to report the a2dp_sink connection state*/
    bt_a2dp_sink_audio_state_cb a2dp_sink_audio_state_cb;/*used to report the a2dp_sink audio state, not recommended as mentioned before*/
} btmg_a2dp_sink_callback_t;

/*callback functions for avrcp profile*/
typedef void (*bt_avrcp_play_state_cb)(const char *bd_addr, btmg_avrcp_play_state_t state); /*used to report play state of avrcp, recommended*/
//typedef void (*bt_avrcp_track_changed_cb)(char *bd_addr, char *title, char *artist, char *album, char *track_num, char *num_tracks,
 //                       char *genre, char *playing_time); /*used to report track information*/
typedef void (*bt_avrcp_track_changed_cb)(const char *bd_addr, btmg_track_info_t track_info); /*used to report track information*/
typedef void (*bt_avrcp_play_position_cb)(const char *bd_addr, int song_len, int song_pos);/*used to report the progress of playing music*/
typedef void (*bt_avrcp_audio_volume_cb)(const char *bd_addr, unsigned int volume);

/*avrcp callback*/
typedef struct btmg_avrcp_callback_t {
    bt_avrcp_play_state_cb avrcp_play_state_cb;
    bt_avrcp_track_changed_cb avrcp_track_changed_cb;
    bt_avrcp_play_position_cb avrcp_play_position_cb;
    bt_avrcp_audio_volume_cb avrcp_audio_volume_cb;/*used to report the avrcp volume, range: 0~16*/
} btmg_avrcp_callback_t;

/*bt_manager callback struct to be registered when calling bt_manager_init to report various event*/
typedef struct btmg_callback_t {
    btmg_manager_callback_t btmg_manager_cb;
    btmg_gap_callback_t btmg_gap_cb;
    btmg_a2dp_sink_callback_t btmg_a2dp_sink_cb;
    btmg_avrcp_callback_t btmg_avrcp_cb;
}btmg_callback_t;


/*bt_manager APIs*/
/* set the bt_manager printing level*/
int bt_manager_set_loglevel(btmg_log_level_t log_level);
/* get the bt_manager printing level*/
btmg_log_level_t bt_manager_get_loglevel(void);
/*preinit function, to allocate room for callback struct, which will be free by bt_manager_deinit*/
int bt_manager_preinit(btmg_callback_t **btmg_cb);
/*deinit function, must be called before exit*/
int bt_manager_deinit(btmg_callback_t *btmg_cb);
/*init function, the callback functions will be registered*/
int bt_manager_init(btmg_callback_t *btmg_cb);
/*GAP APIs*/
/*set BT discovery mode*/
int bt_manager_set_discovery_mode(btmg_discovery_mode_t mode);
/*enable BT*/
int bt_manager_enable(bool enable);
/*set the BT to be enable defaultly when calling bt_manager_init*/
int bt_manager_set_enable_default(bool is_default);
/*set auto ssp reply when pairing request recieved, without user input comfirmation*/
int bt_manager_set_auto_ssp_reply(bool auto_reply);
/*set auto ssp reply when pairing request recieved, without user input comfirmation*/
int bt_manager_set_auto_pin_reply(bool auto_reply);
/*return BT state, is enabled or not*/
bool bt_manager_is_enabled(void);
/*start discovery, will return immediately*/
int bt_manager_start_discovery(void);
/*cancel discovery, will return immediately*/
int bt_manager_cancel_discovery(void);
/*judge the discovery is in process or not*/
bool bt_manager_is_discovering();
/*pair*/
int bt_manager_pair(char *addr);
/*unpair*/
int bt_manager_unpair(char *addr);
/*get bt state*/
btmg_state_t bt_manager_get_state();
/*get BT name*/
int bt_manager_get_name(char *name, int size);
/*set BT name*/
int bt_manager_set_name(const char *name);
/*get local device address*/
int bt_manager_get_address(char *addr, int size);
/*ssp reply to accept or reject bonding request*/
int bt_manager_ssp_reply(bool yes);
/*pin reply to pair or not*/
int bt_manager_pin_reply(bool accept, char *pin_code);
/*a2dp sink APIs*/
/*request a2dp_sink connection*/
int bt_manager_a2dp_sink_connect(char *addr);
/*request a2dp_sink disconnection*/
int bt_manager_a2dp_sink_disconnect(char *addr);
/*used to send avrcp command, refer to the struct btmg_avrcp_command_t for the supported commands*/
int bt_manager_avrcp_command(char *addr, btmg_avrcp_command_t command);
/*used to set absolute volume, range: 0~16*/
int bt_manager_vol_changed_noti(char *vol_level);

/* Get the paired device,need to call <bt_manager_free_paired_devices>  to free data*/
int bt_manager_get_paired_devices(bt_paried_device **dev_list,int *count);

/* free paird device data resource*/
int bt_manager_free_paired_devices(bt_paried_device *dev_list);

#if __cplusplus
};  // extern "C"
#endif

#endif
