#ifndef _M_TYPE_H__
#define _M_TYPE_H__


#include <stdlib.h>
#include "cJSON.h"

#define ETH "wlan0" // change to your device
typedef int (*m_cloud_callback)(void *usrdata);

typedef enum {
    M_OUT_NONE = 0,
    M_OUT_HEX_CMD,
    M_OUT_ASR_TXT,
    M_OUT_TTS_URL,
    M_OUT_BROADCAST,
    M_OUT_NLU_RESULT,
    M_OUT_DM_RESULT,
    M_OUT_OUTER_DATA,/*reserved*/
    M_OUT_NATIVE_CALL,/*reserved*/
    M_OUT_ERROR, /*reserved*/
	M_OUT_OTA,
    M_CLOUD_STAT,
    M_SYSTEM_CONFIG,
	M_GET_STATUS,
	M_GET_HWINFO,
	M_ORDER_CONTROL,
}M_EVENT;

struct m_usr_data{
    M_EVENT event;
	cJSON *req_data;
	void *user_data;
};


struct m_cloud_st {
    int running;
	char *url;
	char *sn;
	char *model;
	char *fw_version;  /* const version */
	char *sw_version;  /* software version */
	char *hardwarever;
	char *hardwareFullVer;    /* see below */
	char *hardwareCategory;   /* to happay AI cloud */
	char *hardwareModel;      /* make AI cloud happy */
	char *firmwarever;	  /* AI cloud version */
	char *hardwareplat;
	char *firmwareurl;
	char *firmwaremd5;
	char *brand;
	char *category;
	char *id;         /* device id */
	char *clientId;	 /* AI cloud client ID */
	char *mac;
	char *otaStatus;
	char confirm;
	char img_exist;
	char downloading;
    m_cloud_callback resq_cb_handler; /* resquest callback handler */
	void *priv_data;
};

typedef enum {
	M_REPORT_LOG = 0,
	M_REPORT_STATUS,
	M_ORDER_TRANS_REPLY,
	M_ORDER_CONTROL_REPLY,
	M_ORDER_CONFIG_REPLY,
	M_ORDER_PROFILE_REPLY,
	M_ORDER_STATUS_REPLY,
	M_ORDER_CAP_REPLY, /* capability */
	M_SPEECH_TRANS,
	M_SPEECH_ACTIVE,
	M_SPEECH_BROADCAST_REPLY,
	M_SPEECH_ASR,
	M_SPEECH_NLU,
	M_NOTIFY_EVENT,
	M_OTA_CHECK,
	M_OTA_UPGRADE_REPLY,
	M_OTA_STATUS,

}M_CLOUD_TOPIC_E;


typedef enum{
    M_CLOUD_OFFLINE = 0,
	M_CLOUD_ONLINE,
	M_CLOUD_TIMEOUT,
	M_CLOUD_THINKING,
	M_CLOUD_CONNECTING
}M_CLOUD_STATUS_E;

#endif
