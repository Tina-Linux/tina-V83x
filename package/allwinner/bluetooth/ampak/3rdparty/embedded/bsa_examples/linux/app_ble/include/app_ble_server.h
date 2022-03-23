/*****************************************************************************
**
**  Name:           app_ble_server.h
**
**  Description:    Bluetooth BLE include file
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef APP_BLE_SERVER_H
#define APP_BLE_SERVER_H

#include "bsa_api.h"
#include "app_ble.h"

/*******************************************************************************
 **
 ** Function         app_ble_server_find_free_space
 **
 ** Description      find free space for BLE server application
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_free_space(void);

/*******************************************************************************
 **
 ** Function         app_ble_server_display
 **
 ** Description      display BLE server
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_server_display(void);

/*******************************************************************************
 **
 ** Function         app_ble_server_find_reg_pending_index
 **
 ** Description      find registration pending index
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_reg_pending_index(void);

/*******************************************************************************
 **
 ** Function         app_ble_server_find_index_by_interface
 **
 ** Description      find BLE server index by interface
 **
 ** Parameters    if_num: interface number
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_index_by_interface(tBSA_BLE_IF if_num);

/*******************************************************************************
 **
 ** Function         app_ble_server_find_index_by_conn_id
 **
 ** Description      find BLE server index by connection ID
 **
 ** Parameters       conn_id: Connection ID
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_index_by_conn_id(UINT16 conn_id);

/*
 * BLE Server functions
 */
/*******************************************************************************
 **
 ** Function        app_ble_server_register
 **
 ** Description     Register server app
 **
 ** Parameters      uuid: uuid number of application
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_register(UINT16 uuid, tBSA_BLE_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function        app_ble_server_deregister
 **
 ** Description     Deregister server app
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_deregister(void);

/*******************************************************************************
 **
 ** Function        app_ble_server_create_service
 **
 ** Description     create service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_create_service(void);

/*******************************************************************************
 **
 ** Function        app_ble_server_add_char
 **
 ** Description     Add charcter to service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_add_char(void);

/*******************************************************************************
 **
 ** Function        app_ble_server_start_service
 **
 ** Description     Start Service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_start_service(void);

/*******************************************************************************
 **
 ** Function        app_ble_server_send_indication
 **
 ** Description     Send indication to client
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_send_indication(void);

/*******************************************************************************
**
** Function         app_ble_server_profile_cback
**
** Description      BLE Server Profile callback.
**
** Returns          void
**
*******************************************************************************/
void app_ble_server_profile_cback(tBSA_BLE_EVT event,  tBSA_BLE_MSG *p_data);

/*******************************************************************************
 **
 ** Function        app_ble_server_open
 **
 ** Description     This is the ble open connection to ble client
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_open(void);

/*******************************************************************************
 **
 ** Function        app_ble_server_close
 **
 ** Description     This is the ble close connection
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_close(void);

/*******************************************************************************
 **
 **
 ** Function        app_ble_server_ext funcitons
 **
 **
 *******************************************************************************/
int app_ble_server_register_ext(UINT16 uuid, tBSA_BLE_CBACK *p_cback);
int app_ble_server_deregister_ext(int server_num);
int app_ble_server_create_service_uuid128(int server_num, UINT8 service_uuid128[], int char_num, int is_primary);
int app_ble_server_add_char_uuid128(int server_num, int srvc_attr_num,
    UINT8 char_uuid128[], int is_descript, UINT16 attribute_permission, UINT16 characteristic_property);
int app_ble_server_start_service_ext(int server_num, int srvc_attr_num);
int app_ble_server_config_adv_data(UINT8 srvc_uuid128_array[],
        UINT8 app_ble_adv_value[], int data_len,
        UINT16 appearance_data, int is_scan_rsp);
int app_ble_server_send_indication_ext(int server_num, int attr_num, UINT8 data[], int data_len);
int app_ble_server_send_read_rsp(void *p_cb_data, UINT8 rsp_data[], int len);
int app_ble_server_find_servc_num_by_handle(int server_num, UINT16 handle);
int app_ble_server_find_char_uuid_by_handle(int server_num, UINT16 handle, UINT8 attr_uuid128[]);

#endif
