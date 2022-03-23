#ifndef __MS_UART_PROCESS_H__
#define	__MS_UART_PROCESS_H__
#include "stdbool.h"
#include "stdint.h"

/******************************************************************
				GLOBAL	DEFINATIONS	 DEFINATE
*******************************************************************/
#define MS_UPGRADE_PASSTHROUGH_MODE			0x00
#define MSMART_APPLIANCE_UPGRADE_MODE		MS_UPGRADE_PASSTHROUGH_MODE
#define MS_UART_MSG_MAX_LEN					265
#define MS_MSMART_UART_HEAD_BYTE			0
#define MS_MSMART_UART_HEAD				0xAA
#define MS_MSMART_UART_MSG_HEAD_LEN		1
#define MS_MSMART_UART_MSG_LEN_BYTE		1
#define MS_MSMART_UART_MSG_MIN_LEN		10
#define MS_MSMART_UART_MSG_CMD_TYPE_BYTE	9
#define MS_UART_MSG_DATA_BEGIN_BYTE			10

typedef enum
{
	MS_UART_CTL_CMD							= 0x02,
	MS_UART_QUERY_CMD						= 0x03,
	MS_UART_REPORT_RUNNING_PARA_NOACK	= 0x04,
	MS_UART_REPORT_RUNNING_PARA_ACK			= 0x05,
	MS_UART_REPORT_ERROR_NOACK			= 0x06,
	MS_UART_SN_ACQUIRE						= 0x07,
	MS_UART_REPORT_ERROR_ACK				= 0x0a,
	MS_UART_ONLINE_NOTIFY						= 0x0d,
	MS_UART_GET_MAC							= 0x13,
	MS_UART_ENTER_SELF_DETECT_MODE			= 0x14,
	MS_UART_SYNCHRONIZE_TIME				= 0x61,
    MS_UART_DEV_QUERY_NETWORK_STATE		= 0x63,
	MS_UART_AIRCONDITION_RESTORE			= 0x64,
	MS_UART_AIRCONDITION_GET_INFO			= 0x65,
	MS_UART_CONFIG_MODULE_ROUTER			= 0X6A,
	MS_UART_SWITCH_MODULE_MODE			= 0X81,
	MS_UART_REBOOT_WIFI						= 0x82,
	MS_UART_RESTORE_WIFI_CFG				= 0x83,
	MS_UART_UNBIND                                             = 0x84,
	MS_UART_WIFI_VERSION_QUERY			= 0X87,
	MS_UART_QUERY_DEV_INFO					= 0xa0,
	MS_UART_RENT_CTL_CMD                                 = 0xbb,
	MS_UART_DEV_SERVICE_QUERY				= 0xe1,
	MS_UART_DEV_TRD_QUERY					= 0xe2,
	MS_UART_DEV_UUID_QUERY					= 0xe3,
	MS_UART_EXT_THIRD_PARTY_INFO_QUERY      = 0xe5,
	MS_UART_EXT_AUDIO_PALY                  = 0xCB,
	MS_UART_EXT_AUDIO_PALY_CONTINUE         = 0xCC,
	MS_UART_EXT_WDG_REBOOT                  = 0x86,
}ms_uart_up_type_t;


typedef struct MS_UART_DO_PACKET_PARA_T
{
	uint8_t  *data;
	uint16_t *data_len;
	uint8_t  device_type;
	uint8_t  msg_id;
	uint8_t  cmd;
}ms_uart_do_packet_para_t;

typedef struct MS_UART_MSG_HEAD_T
{
	uint8_t head;
    uint8_t msg_length;
	uint8_t dev_type;
	uint8_t frame_crc;
	uint8_t reserve[2];
	uint8_t msg_id;
	uint8_t frame_version;
	uint8_t dev_version;
	uint8_t cmd_type;
} ms_uart_msg_head_t;

typedef enum
{
	MS_UART_SM_GET_SN	= 0x00,
	MS_UART_SM_GET_E1	= 0x01,
	MS_UART_SM_GET_A0	= 0x02,
	MS_UART_SM_GET_E2	= 0x03,
	MS_UART_SM_GET_E3	= 0x04,
	MS_UART_SM_GET_E5,
	MS_UART_SM_START_OK,
	MS_UART_SM_IDLE,
	MS_UART_SM_SELFCHECK,		//handle with 0x14
}ms_uart_sm_t;


typedef enum
{
	MS_TIMER_ONE_SHOT,
	MS_TIMER_PERIODIC,
}ms_timer_reload_t;

typedef enum MS_FW_UART_SELF_CHECK_STATUS_T
{
	MS_SELF_DETECT_STATE_GET_SN	= 0x00,
	MS_SELF_DETECT_STATE_SCAN_AP	= 0x01,
	MS_SELF_DETECT_STATE_GET_SN_2S	= 0x02,
	MS_SELF_DETECT_STATE_SELF_CHECK_FINISH = 0x03,
	MS_SELF_DETECT_STATE_RESTART	= 0x04,
	MS_SELF_DETECT_STATE_INIT_PRINT	= 0x05,
	MS_SELF_DETECT_STATE_IDLE,
}ms_fw_uart_self_check_status_t;

typedef struct MS_FW_UART_SELF_CHECK_T
{
	uint8_t auth_type;
	uint8_t ssid_length;
	uint8_t ssid[32];
	ms_fw_uart_self_check_status_t self_check_status;
	uint8_t reference_sn[32];
	bool get_sn_flag;
	bool get_ssid_flag;
}ms_fw_uart_self_check_t;

#define ms_networking_status_t ms_sys_status_t

/******************************************************************
				INTERFACE	 DECLEAR
*******************************************************************/
typedef void (*callBackFun) (void *);//???????l??????

void ms_fw_uart_debug_hex(char *info, uint8_t *data, uint16_t len);

void ms_uart_do_package(ms_uart_do_packet_para_t *para);
void ms_uart_state_machine(void);

#endif
