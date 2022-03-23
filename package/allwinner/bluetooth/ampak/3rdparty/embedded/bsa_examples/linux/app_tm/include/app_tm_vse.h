
/*****************************************************************************
**
**  Name:           app_tm_vse.h
**
**  Description:    Bluetooth Vendor Specific Event handing
**
**  Copyright (c) 2015, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#ifndef APP_TM_VSE_H
#define APP_TM_VSE_H

/* Self-sufficient */
#include "bt_target.h"
#include "app_tm.h"
#include "bsa_tm_api.h"

/*
 * Definitions
 */
#ifndef APP_TM_VSE_FW_CORE_DUMP_FILENAME
#define APP_TM_VSE_FW_CORE_DUMP_FILENAME    "./fw_core_dump.log"
#endif

/* Vendor Specific Sub-Events */
#define APP_TM_VSE_3D_MODE_CHANGE           0x21    /* 3D Mode Change */
#define APP_TM_VSE_3D_FRAME_PERIOD          0x22    /* 3D Frame Period */
#define APP_TM_VSE_3D_CLOCK_SNAPSHOT        0x23    /* 3D Clock SnapShot */
#define APP_TM_VSE_3D_MULTICAST_LOCK_CPLT   0x24    /* 3D Slave Lock Complete */
#define APP_TM_VSE_3D_MULTICAST_SYNC_TO     0x25    /* 3D Slave Sync Timeout */
#define APP_TM_VSE_3D_EIR_HANDSHAKE_CPLT    0x29    /* EIR Handshake (Connect) */
#define APP_TM_VSE_3D_MULTICAST_RX_DATA     0x30    /* Multicast Rx Data */

#define APP_TM_VSE_NOTI_WLAN_RESET          0x54    /* Notification of WLAN reset */

#define APP_TM_VSE_FW_CORE_DUMP             0xF7    /* FW Coredump Data */


/* 3DG Report Id definitions */
#define APP_3DG_REPORT_BAT_LEVEL            0x01    /* Battery Level Report */
#define APP_3DG_REPORT_RSSI                 0x02    /* RSSI Level Report */
#define APP_3DG_REPORT_MODEL                0x03    /* Model Name/IdReport */
#define APP_3DG_REPORT_VENDOR               0x04    /* Vendor Name/Id Report */
#define APP_3DG_REPORT_FW_VERSION           0x05    /* FW Version Report */
#define APP_3DG_REPORT_BUTTON               0x06    /* Button Press Report */
#define APP_3DG_REPORT_BDADDR               0x07    /* BdAddr Report */
#define APP_3DG_REPORT_ASSOCIATION          0x08    /* Association Report */
#define APP_3DG_REPORT_DEBUG                0xFF    /* Debug Report */

/* Proximity Pairing (EIR_HANDSHAKE_CPLT) Type */
#define APP_TM_VSE_PROX_PAIR_TYPE_3DG              0x00
#define APP_TM_VSE_PROX_PAIR_TYPE_RC               0x01

/* 3DG Proximity Pairing (EIR_HANDSHAKE_CPLT) SubType */
#define APP_TM_VSE_PROX_PAIR_SUB_TYPE_3DG_MCAST    0x00
#define APP_TM_VSE_PROX_PAIR_SUB_TYPE_3DG_MIP      0x01

/* RC Proximity Pairing (EIR_HANDSHAKE_CPLT) SubType */
#define APP_TM_VSE_PROX_PAIR_SUB_TYPE_RC           0x00
#define APP_TM_VSE_PROX_PAIR_SUB_TYPE_RC_VOICE     0x01

/* APP TM custom callback function */
typedef BOOLEAN (tAPP_TM_CUSTOM_CBACK)(tBSA_TM_EVT event, tBSA_TM_MSG *p_data);

typedef struct
{
    tAPP_TM_CUSTOM_CBACK *p_callback;
    FILE *fw_core_dump_file; /* FW Core Dump File Descriptor */
    BOOLEAN erase_fw_core_dump;
} tAPP_TM_VSE_CB;


/*
 * Global variables
 */
extern tAPP_TM_VSE_CB app_tm_vse_cb;


/*******************************************************************************
 **
 ** Function       app_tm_vse_init
 **
 ** Description    Initialize VSE module
 **
 ** Parameters     None
 **
 ** Returns        Void
 **
 *******************************************************************************/
int app_tm_vse_init (void);

/*******************************************************************************
 **
 ** Function        app_tm_vse_register
 **
 ** Description     This function is used to Register Vendor Specific Events
 **
 ** Parameters      sub_event: sub_event to register
 **                 p_vse_callback: VSE Callback function to call
 **
 ** Returns         status
 **
 *******************************************************************************/
int app_tm_vse_register(int sub_event, tAPP_TM_CUSTOM_CBACK p_vse_callback);

/*******************************************************************************
 **
 ** Function        app_tm_vse_deregister
 **
 ** Description     This function is used to Deregister Vendor Specific Events
 **
 ** Parameters      sub_event: sub_event to register
 **                 deregister_callback: indicates if the VSE callback must be deregistered
 **
 ** Returns         status
 **
 *******************************************************************************/
int app_tm_vse_deregister(int sub_event, BOOLEAN deregister_callback);

/*******************************************************************************
 **
 ** Function       app_tm_vse_get_prox_pairing_type_desc
 **
 ** Description    Get the name of a Proximity Pairing type Event
 **
 ** Parameters     type: Proximity Pairing type
 **
 ** Returns        Type Description
 **
 *******************************************************************************/
char *app_tm_vse_get_prox_pairing_type_desc (UINT8 type);

/*******************************************************************************
 **
 ** Function       app_tm_vse_get_prox_pairing_type_desc
 **
 ** Description    Get the name of a Proximity Pairing type Event
 **
 ** Parameters     type: Proximity Pairing type
 **                sub_type: Proximity Pairing sub-type
 **
 ** Returns        Type Description
 **
 *******************************************************************************/
char *app_tm_vse_get_prox_pairing_sub_type_desc (UINT8 type, UINT8 sub_type);

/*******************************************************************************
 **
 ** Function        app_tm_vse_decode_association
 **
 ** Description     Decode Association message
 **
 ** Parameters      p_data: Pointer on the buffer to decode
 **                 data_length: Length of the buffer
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_decode_association(UINT8 *p_data, UINT8 data_length);


/*******************************************************************************
 **
 ** Function        app_tm_vse_fw_core_dump_erase
 **
 ** Description     Erase last FW core dump file
 **
 ** Parameters      None
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_fw_core_dump_erase(void);

/*******************************************************************************
 **
 ** Function        app_tm_vse_fw_core_dump_open
 **
 ** Description     Open FW core dump file in Append mode
 **
 ** Parameters      None
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_fw_core_dump_open(void);

/*******************************************************************************
 **
 ** Function        app_tm_vse_fw_core_dump_close
 **
 ** Description     Close FW core dump file
 **
 ** Parameters      None
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_fw_core_dump_close(void);

/*******************************************************************************
 **
 ** Function        app_tm_vse_fw_core_dump_write_header
 **
 ** Description     Write header into FW core dump file
 **
 ** Parameters      None
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_fw_core_dump_write_header(void);

/*******************************************************************************
 **
 ** Function        app_tm_vse_fw_core_dump_write_data
 **
 ** Description     Write data into FW core dump file
 **
 ** Parameters      p_buf:pointer on the VSE data
 **                 buf_length: data length
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_fw_core_dump_write_data(UINT8 *p_buf, int buf_length);

/*******************************************************************************
 **
 ** Function        app_tm_vse_decode_mcast_param
 **
 ** Description     Decode VSE Parameters
 **
 ** Parameters      p_vse: VSE data to decode
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_decode_mcast_param(tBSA_TM_VSE_MSG *p_vse);
#endif /* APP_TM_VSE_H */
