/*****************************************************************************
**
**  Name:           app_ble_client.h
**
**  Description:    Bluetooth BLE include file
**
**  Copyright (c) 2015, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef APP_BLE_CLIENT_H
#define APP_BLE_CLIENT_H

#include "bsa_api.h"
#include "app_ble.h"
#include "app_ble_client_db.h"

/*******************************************************************************
 **
 ** Function         app_ble_client_find_free_space
 **
 ** Description      find free space for BLE client application
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_client_find_free_space(void);

/*******************************************************************************
 **
 ** Function         app_ble_client_display_attr
 **
 ** Description      Display BLE client's attribute list
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_client_display_attr(tAPP_BLE_CLIENT_DB_ELEMENT *p_blecl_db_elmt);

/*******************************************************************************
 **
 ** Function         app_ble_client_display
 **
 ** Description      Display BLE client
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_client_display(int detail);

/*******************************************************************************
 **
 ** Function         app_ble_client_find_reg_pending_index
 **
 ** Description      find registration pending index
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_client_find_reg_pending_index(void);

/*******************************************************************************
 **
 ** Function         app_ble_client_find_index_by_interface
 **
 ** Description      find BLE client index by interface
 **
 ** Parameters    if_num: interface number
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_client_find_index_by_interface(tBSA_BLE_IF if_num);

/*******************************************************************************
 **
 ** Function         app_ble_client_find_conn_id_by_interface
 **
 ** Description      find BLE client conn index by interface
 **
 ** Parameters    if_num: interface number
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_client_find_conn_id_by_interface(tBSA_BLE_IF if_num);

/*******************************************************************************
 **
 ** Function         app_ble_client_find_index_by_conn_id
 **
 ** Description      find BLE client index by connection ID
 **
 ** Parameters       conn_id: Connection ID
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_client_find_index_by_conn_id(UINT16 conn_id);

/*
 * BLE Client functions
 */
/*******************************************************************************
 **
 ** Function        app_ble_client_register
 **
 ** Description     This is the ble client register command
 **
 ** Parameters      uuid: uuid number of application
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_register(UINT16 uuid);

/*******************************************************************************
 **
 ** Function        app_ble_client_service_search
 **
 ** Description     This is the ble search service to ble server
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_service_search(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_read
 **
 ** Description     This is the read function to read a characteristec or characteristic descriptor from BLE server
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_write
 **
 ** Description     This is the write function to write a characteristic or characteristic discriptor to BLE server.
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_write(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_register_notification
 **
 ** Description     This is the register function to receive a notification
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_register_notification(void);


/*******************************************************************************
 **
 ** Function        app_ble_client_open
 **
 ** Description     This is the ble open connection to ble server
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_open(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_close
 **
 ** Description     This is the ble close connection
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_close(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_unpair
 **
 ** Description     Unpair LE device
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_unpair(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_deregister
 **
 ** Description     This is the ble deregister app
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_deregister(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_deregister_all
 **
 ** Description     Deregister all apps
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_deregister_all(void);


/*******************************************************************************
 **
 ** Function        app_ble_client_deregister_notification
 **
 ** Description     This is the deregister function for a notification
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_deregister_notification(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_load_attr
 **
 ** Description     This is the cache load function
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_load_attr(tAPP_BLE_CLIENT_DB_ELEMENT *p_blecl_db_elmt,
                             BD_ADDR bd_addr, UINT16 conn_id);

/*******************************************************************************
**
** Function         app_ble_client_profile_cback
**
** Description      BLE Client Profile callback.
**
** Returns          void
**
*******************************************************************************/
void app_ble_client_profile_cback(tBSA_BLE_EVT event,  tBSA_BLE_MSG *p_data);

/*******************************************************************************
 **
 ** Function        app_ble_client_service_search_execute
 **
 ** Description     This is the ble search service to ble server
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_service_search_execute(UINT16 service);

/*******************************************************************************
 **
 ** Function        app_ble_client_read_battery_level
 **
 ** Description     Read BLE Device battery level
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_battery_level(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_read_mfr_name
 **
 ** Description     Read BLE Device manufacturer name
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_mfr_name(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_read_model_number
 **
 ** Description     Read BLE Device model number
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_model_number(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_read_serial_number
 **
 ** Description     Read BLE Device serial number
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_serial_number(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_read_hardware_revision
 **
 ** Description     Read BLE Device hardware revision
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_hardware_revision(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_read_firmware_revision
 **
 ** Description     Read BLE Device firmware revision
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_firmware_revision(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_read_software_revision
 **
 ** Description     Read BLE Device software revision
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_software_revision(void);

/*******************************************************************************
 **
 ** Function        app_ble_client_read_system_id
 **
 ** Description     Read BLE Device system ID
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_system_id(void);

/*******************************************************************************
**
** Function         app_ble_client_handle_read_evt
**
** Description      Function to handle callback for read events
**
** Returns          void
**
*******************************************************************************/
void app_ble_client_handle_read_evt(tBSA_BLE_CL_READ_MSG *cli_read);

/*******************************************************************************
**
** Function         app_ble_client_handle_indication
**
** Description      Function to handle callback for indications
**
** Returns          void
**
*******************************************************************************/
void app_ble_client_handle_indication(tBSA_BLE_CL_NOTIF_MSG *cli_notif);

/*******************************************************************************
**
** Function         app_ble_client_handle_notification
**
** Description      Function to handle callback for notification events
**
** Returns          void
**
*******************************************************************************/
void app_ble_client_handle_notification(tBSA_BLE_CL_NOTIF_MSG *cli_notif);

#endif
