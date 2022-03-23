/*****************************************************************************
**
**  Name:           app_ble_client_otafwdl.h
**
**  Description:    Bluetooth LE OTA fw download header file
**
**  Copyright (c) 2010-2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef  APP_BLE_CLIENT_OTAFWDL_H
#define  APP_BLE_CLIENT_OTAFWDL_H

#define  APP_BLE_CLIENT_OTAFWDL_STRING_DATA_LEN          2
#define  APP_BLE_CLIENT_OTAFWDL_STRING_ADDRESS_LEN       4
#define  APP_BLE_CLIENT_OTAFWDL_STRING_DATA_RECORD_TYPE  2
#define  APP_BLE_CLIENT_OTAFWDL_STRING_CHECKSUM_LENGTH   2
#define  APP_BLE_CLIENT_OTAFWDL_STRING_SKIP_CRLF         2
#define  APP_BLE_CLIENT_OTAFWDL_SKIP_BYTE (APP_BLE_CLIENT_OTAFWDL_STRING_DATA_LEN+APP_BLE_CLIENT_OTAFWDL_STRING_ADDRESS_LEN+APP_BLE_CLIENT_OTAFWDL_STRING_DATA_RECORD_TYPE)
#define  APP_BLE_CLIENT_OTAFWDL_WS_UPGRADE_COMMAND_PREPARE_DOWNLOAD  1
#define  APP_BLE_CLIENT_OTAFWDL_WS_UPGRADE_COMMAND_DOWNLOAD          2
#define  APP_BLE_CLIENT_OTAFWDL_WS_UPGRADE_COMMAND_VERIFY            3
#define  APP_BLE_CLIENT_OTAFWDL_MAX_TX_WRITE_PACKET_LEN           0x14
#define  APP_BLE_CLIENT_OTAFWDL_WIDTH                             32
#define  APP_BLE_CLIENT_OTAFWDL_TOPBIT                            (1 << (APP_BLE_CLIENT_OTAFWDL_WIDTH-1))
#define  APP_BLE_CLIENT_OTAFWDL_POLYNOMIAL                        0x04C11DB7
#define  APP_BLE_CLIENT_OTAFWDL_INITIAL_REMAINDER                 0xFFFFFFFF
#define  APP_BLE_CLIENT_OTAFWDL_FINAL_XOR_VALUE                   0xFFFFFFFF

typedef enum
{
   OTAFU_STATE_PRE_INIT,
   OTAFU_STATE_WAIT_DOWNLOAD_NOTIFY,
   OTAFU_STATE_REC_DOWNLOAD_NOTIFY,
   OTAFU_STATE_WAIT_DOWNLOAD_LENGTH_NOTIFY,
   OTAFU_STATE_REC_DOWNLOAD_LENGTH_NOTIFY,
   OTAFU_STATE_WRITE_DATA,
   OTAFU_STATE_DATA_VERIFY,
   OTAFU_STATE_DATA_VERIFY_CHECK,
   OTAFU_STATE_UPGRADE_SUCCESS,
   OTAFU_STATE_UPGRADE_FAIL,
   OTAFU_STATE_ABORT
}app_ble_client_fwdl_upgrade_state;


typedef struct
{
    unsigned char *otabinary;
    UINT32 ota_data_len;
    UINT32 ota_binary_len;
    UINT32 ota_binary_idx;
    UINT32 ota_need_send_cnt;
    UINT32 ota_send_cnt;
    UINT32 mcrc ;
    UINT32 client_num;
    app_ble_client_fwdl_upgrade_state ota_upgrade_state ;
}app_ble_client_fwdl_fw_upgrade_cb;

/*******************************************************************************
 **
 ** Function        app_ble_client_fw_upgrade
 **
 ** Description     do preparation for fw upgrade
 **
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 ***************************************************************************/
int app_ble_client_fw_upgrade(void);
/*******************************************************************************
**
** Function       app_ble_client_fw_handle_upgrade_process
**
** Description     handle fw upgrade state machine
**
**
**  Returns         None
**
 ***************************************************************************/
void app_ble_client_fw_handle_upgrade_process(tBSA_BLE_EVT event ,tBSA_BLE_MSG *p_data);
#endif
