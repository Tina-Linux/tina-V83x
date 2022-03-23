/*
  * Copyright (c) 2019 Allwinner Technology. All Rights Reserved.
  * Author        laumy <liumingyuan@allwinnertech.com>
  * Version       0.0.1
  *
  * Author:        Kevin
  * Version:       1.0.0
  * Update: first version of definition for Allwinner BLE standard APIs
  */

#ifndef __AW_GATT_SERVER_H
#define __AW_GATT_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum {
	BT_GATT_CHAR_PROPERTY_BROADCAST = 0x01,
	BT_GATT_CHAR_PROPERTY_READ = 0x02,
	BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE = 0x04,
	BT_GATT_CHAR_PROPERTY_WRITE = 0x08,
	BT_GATT_CHAR_PROPERTY_NOTIFY = 0x10,
	BT_GATT_CHAR_PROPERTY_INDICATE = 0x20,
	BT_GATT_CHAR_PROPERTY_AUTH_SIGNED_WRITE = 0x40
} gatt_char_properties_t;
//}tGattCharProperties;

typedef enum {
	BT_GATT_DESC_PROPERTY_BROADCAST = 0x01,
	BT_GATT_DESC_PROPERTY_READ = 0x02,
	BT_GATT_DESC_PROPERTY_WRITE_NO_RESPONSE = 0x04,
	BT_GATT_DESC_PROPERTY_WRITE = 0x08,
	BT_GATT_DESC_PROPERTY_NOTIFY = 0x10,
	BT_GATT_DESC_PROPERTY_INDICATE = 0x20,
	BT_GATT_DESC_PROPERTY_AUTH_SIGNED_WRITE = 0x40
} gatt_desc_properties_t;
//}tGattDescProperties;

typedef enum {
	BT_GATT_PERM_READ = 0x01,
	BT_GATT_PERM_WRITE = 0x02,
	BT_GATT_PERM_READ_ENCYPT = 0x04,
	BT_GATT_PERM_WRITE_ENCRYPT = 0x08,
	BT_GATT_PERM_ENCRYPT = 0x04 | 0x08,
	BT_GATT_PERM_READ_AUTHEN = 0x10,
	BT_GATT_PERM_WRITE_AUTHEN = 0x20,
	BT_GATT_PERM_AUTHEN = 0x10 | 0x20,
	BT_GATT_PERM_AUTHOR = 0x40,
	BT_GATT_PERM_NONE = 0x80
} gatt_permissions_t;
//}tGattPermissions;

/* codes for Success or Error response PDU */
typedef enum {
	BT_GATT_SUCCESS  = 0x00,
	BT_GATT_ERROR_INVALID_HANDLE   = 0x01, /*Invalid Handle*/
	BT_GATT_ERROR_READ_NOT_PERMITTED = 0x02, /*Read not Permitted*/
	BT_GATT_ERROR_WRITE_NOT_PERMITTED = 0x03, /*Writed Not Permitted*/
	BT_GATT_ERROR_INVALID_PDU = 0x04, /*Invalid PDU*/
	BT_GATT_ERROR_AUTHENTICATION  = 0x05, /*Insufficient Authentication*/
	BT_GATT_ERROR_REQUEST_NOT_SUPPORTED  = 0x06, /*Request Not Supported*/
	BT_GATT_ERROR_INVALID_OFFSET  = 0x07, /*Invalid Offset*/
	BT_GATT_ERROR_AUTHORIZATION   = 0x08, /*Insufficient Authorization*/
	BT_GATT_ERROR_PREPARE_QUEUE_FULL  = 0x09, /*Prepare Queue Full*/
	BT_GATT_ERROR_ATTRIBUTE_NOT_FOUND  = 0x0A, /*Attribute Not Found*/
	BT_GATT_ERROR_ATTRIBUTE_NOT_LONG  = 0x0B, /*Attribute Not Long*/
	BT_GATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE = 0x0C, /*Insufficient Encryption Key Size*/
	BT_GATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN  = 0x0D, /*Invalid Attribute Value Length*/
	BT_GATT_ERROR_UNLIKELY  = 0x0E, /*Unlikely Error*/
	BT_GATT_ERROR_INSUFFICIENT_ENCRYPTION  = 0x0F, /*Insufficient Encryption*/
	BT_GATT_ERROR_UNSUPPORTED_GROUP_TYPE  = 0x10, /*Unsupported Group Type*/
	BT_GATT_ERROR_INSUFFICIENT_RESOURCES  = 0x11, /*Insufficient Resources*/
	BT_GATT_ERROR_DB_OUT_OF_SYNC  = 0x12, /*Database Out of Sync*/
	BT_GATT_ERROR_VALUE_NOT_ALLOWED  = 0x13 /*Value Not Allowed*/
	/*0x80-0x9F Application Error*/
	/*0xE0-0xFF Common Profile and Service Error Codes*/
} gatt_attr_res_code_t;

typedef enum {
	BT_GATT_CONNECTION,
	BT_GATT_DISCONNECT,
} gatt_connection_event_t;
//}tGattConnectionEvent;

typedef struct {
	int num_handle;
	int svc_handle;
} gatt_add_svc_msg_t;
//}tGattAddSvcMsg;

typedef struct {
	char *uuid;
	int char_handle;
} gatt_add_char_msg_t;
//}tGattAddCharMsg;

typedef struct {
	int desc_handle;
} gatt_add_desc_msg_t;
//}tGattAddDescMsg;

typedef struct {
	unsigned int trans_id;
	int attr_handle;
	int offset;
	bool is_blob_req;
} gatt_char_read_req_t;

#define AG_GATT_MAX_ATTR_LEN 600

typedef struct {
	unsigned int trans_id;
	int attr_handle;
	int offset;
	char value[AG_GATT_MAX_ATTR_LEN];
	int value_len;
	bool need_rsp;
} gatt_char_write_req_t;

typedef struct {
	bool state;
} gatt_notify_req_t;

typedef struct {
	unsigned int trans_id;
	int attr_handle;
	int offset;
	bool is_blob_req;
} gatt_desc_read_req_t;

typedef struct {
	unsigned int trans_id;
	int attr_handle;
	int offset;
	char value[AG_GATT_MAX_ATTR_LEN];
	int value_len;
	bool need_rsp;
} gatt_desc_write_req_t;

typedef struct {

} gatt_send_indication_t;

typedef void (*bt_gatt_add_service_cb)(gatt_add_svc_msg_t *pData);
typedef void (*bt_gatt_add_char_cb)(gatt_add_char_msg_t *pData);
typedef void (*bt_gatt_add_desc_cb)(gatt_add_desc_msg_t *pData);
typedef void (*bt_gatt_sevice_ready_cb)(int state);
typedef void (*bt_gatt_connection_event_cb)(char *addr, gatt_connection_event_t event);

typedef void (*bt_gatt_char_read_req_cb)( gatt_char_read_req_t *char_read);
typedef void (*bt_gatt_char_write_req_cb)(gatt_char_write_req_t *char_write);
typedef void (*bt_gatt_char_notify_req_cb)(gatt_desc_read_req_t *char_notify);

typedef void (*bt_gatt_desc_read_req_cb)(gatt_desc_read_req_t *desc_read);
typedef void (*bt_gatt_desc_write_req_cb)(gatt_desc_write_req_t *desc_write);

typedef void (*bt_gatt_send_indication_cb)(gatt_send_indication_t *send_ind);

typedef struct {

/*gatt add ... callback*/
	bt_gatt_add_service_cb gatt_add_svc_cb;
	bt_gatt_add_char_cb gatt_add_char_cb;
	bt_gatt_add_desc_cb gatt_add_desc_cb;

/*gatt event callback*/
	bt_gatt_connection_event_cb gatt_connection_event_cb;

	bt_gatt_sevice_ready_cb gatt_service_ready_cb;

/*gatt characteristic request callback*/
	bt_gatt_char_read_req_cb gatt_char_read_req_cb;
	bt_gatt_char_write_req_cb gatt_char_write_req_cb;
	bt_gatt_char_notify_req_cb gatt_char_notify_req_cb;

/*gatt descriptor request callback*/
	bt_gatt_desc_read_req_cb gatt_desc_read_req_cb;
	bt_gatt_desc_write_req_cb gatt_desc_write_req_cb;

	bt_gatt_send_indication_cb gatt_send_indication_cb;
} gatt_server_cb_t;
//}tGattServerCb;


typedef struct {
	char *uuid;      /*128-bit service UUID*/
	bool primary;    /* If true, this GATT service is a primary service */
	int number;
} gatt_add_svc_t;

typedef struct {
	char *uuid;      /*128-bit characteristic UUID*/
	int properties;        /*The GATT characteristic properties*/
	int permissions;       /*The GATT characteristic permissions*/
	int svc_handle;          /*the service atr handle*/
} gatt_add_char_t;

typedef struct {
	char *uuid;     /*128-bit descriptor UUID*/
	int properties;       /*The GATT descriptor properties*/
	int permissions;      /*he GATT descriptor  permissions*/
	int svc_handle;
} gatt_add_desc_t;

typedef struct {
	int svc_handle;
} gatt_star_svc_t;

typedef struct {
	int svc_handle;
} gatt_stop_svc_t;

typedef struct {
	int svc_handle;
} gatt_del_svc_t;

typedef struct {
	unsigned int trans_id;
	int status;
	int svc_handle;
	char *value;
	int value_len;
	int auth_req;
} gatt_send_read_rsp_t;

typedef enum {
	BT_GATT_REQ_FAILED,
	BT_GATT_REQ_IN_PROGRESS,
	BT_GATT_REQ_PERMITTED,
	BT_GATT_REQ_INVAILD_VALUE_LENGTH,
	BT_GATT_REQ_NOT_AUTHORIZED,
	BT_GATT_REQ_NOT_SUPPORT
} gatt_state_t;

typedef struct {
	unsigned int trans_id;
	int attr_handle;
	gatt_attr_res_code_t state;
} gatt_write_rsp_t;

typedef struct {
	int attr_handle;
	char *value;
	int value_len;
} gatt_notify_rsp_t;

typedef struct {
	int attr_handle;
	char *value;
	int value_len;
} gatt_indication_rsp_t;

typedef struct gatt_adv_data_t {
	uint8_t data[31];
	uint8_t data_len;
} gatt_adv_data_t;

typedef struct gatt_rsp_data_t {
	uint8_t data[31];
	uint8_t data_len;
} gatt_rsp_data_t;

/* LE address type */
typedef enum {
	BTMG_LE_PUBLIC_ADDRESS = 0x00,
	BTMG_LE_RANDOM_ADDRESS = 0x01,
	BTMG_LE_IRK_OR_PUBLIC_ADDRESS = 0x02,
	BTMG_LE_IRK_OR_RANDOM_ADDRESS = 0x03,
} gatt_le_addr_type_t;

/*LE advertising type*/
typedef enum {
	BTMG_LE_ADV_IND = 0x00, /*connectable and scannable undirected advertising*/
	BTMG_LE_ADV_DIRECT_HIGH_IND = 0x01, /*connectable high duty cycle directed advertising*/
	BTMG_LE_ADV_SCAN_IND = 0x02, /*scannable undirected advertising*/
	BTMG_LE_ADV_NONCONN_IND = 0x03, /*non connectable undirected advertising*/
	BTMG_LE_ADV_DIRECT_LOW_IND = 0x04, /*connectable low duty cycle directed advertising*/
} gatt_le_advertising_type_t;

/*LE advertising filter policy*/
typedef enum {
	BTMG_LE_PROCESS_ALL_REQ = 0x00, /*process scan and connection requests from all devices*/
	BTMG_LE_PROCESS_CONN_REQ = 0x01, /*process connection request from all devices and scan request only from white list*/
	BTMG_LE_PROCESS_SCAN_REQ = 0x02, /*process scan request from all devices and connection request only from white list*/
	BTMG_LE_PROCESS_WHITE_LIST_REQ = 0x03, /*process requests only from white list*/
} gatt_le_advertising_filter_policy_t;

/*LE peer address type*/
typedef enum {
	BTMG_LE_PEER_PUBLIC_ADDRESS = 0x00, /*public device address(default) or public indentiy address*/
	BTMG_LE_PEER_RANDOM_ADDRESS = 0x01, /*random device address(default) or random indentiy address*/
} gatt_le_peer_addr_type_t;


#define BTMG_LE_ADV_CHANNEL_NONE 0x00
#define BTMG_LE_ADV_CHANNEL_37 0x01
#define BTMG_LE_ADV_CHANNEL_38 (0x01 << 1)
#define BTMG_LE_ADV_CHANNEL_39 (0x01 << 2)
#define BTMG_LE_ADV_CHANNEL_ALL (BTMG_LE_ADV_CHANNEL_NONE | BTMG_LE_ADV_CHANNEL_37 | BTMG_LE_ADV_CHANNEL_38 | BTMG_LE_ADV_CHANNEL_39)

typedef struct {
	uint16_t	min_interval;
	uint16_t	max_interval;
	gatt_le_advertising_type_t	adv_type;
	gatt_le_addr_type_t		own_addr_type;
	gatt_le_peer_addr_type_t	peer_addr_type;
	char		peer_addr[18];
	uint8_t		chan_map;
	gatt_le_advertising_filter_policy_t	filter;
} gatt_le_advertising_parameters_t;

int bt_manager_gatt_create_service(gatt_add_svc_t *svc);

int bt_manager_gatt_add_characteristic(gatt_add_char_t *chr);

int bt_manager_gatt_add_descriptor(gatt_add_desc_t *desc);

int bt_manager_gatt_start_service(gatt_star_svc_t *start_svc);

int bt_manager_gatt_stop_service(gatt_stop_svc_t *stop_svc);

int bt_manager_gatt_delete_service(gatt_del_svc_t *del_svc);

int bt_manager_gatt_send_read_response(gatt_send_read_rsp_t *pData);

int bt_manager_gatt_send_write_response(gatt_write_rsp_t *pData);

int bt_manager_gatt_send_notification(gatt_notify_rsp_t *pData);

int bt_manager_gatt_send_indication(gatt_indication_rsp_t *pData);

int bt_manager_gatt_server_init(gatt_server_cb_t *cb);

void bt_manager_gatt_server_deinit();

int bt_manager_gatt_server_disconnect(unsigned int handle, unsigned char reason);

int bt_manager_gatt_server_clear_white_list();

int bt_manager_gatt_enable_adv(bool enable, gatt_le_advertising_parameters_t *adv_param);

int bt_manager_gatt_set_adv_data(gatt_adv_data_t *adv_data);

int bt_manager_gatt_set_local_name(int flag,char *ble_name);

int bt_manager_gatt_set_scan_rsp_data(gatt_rsp_data_t *rsp_data);

extern gatt_server_cb_t *gatt_server_cb;

#ifdef __cplusplus
}
#endif

#endif
