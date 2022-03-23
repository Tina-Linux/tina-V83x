/*****************************************************************************
 **
 **  Name:           bsa_status.h
 **
 **  Description:    Contains the BSA status definitions
 **
 **  Copyright (c) 2015, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_STATUS_H
#define BSA_STATUS_H

#include "data_types.h"

typedef UINT16 tBSA_STATUS;

/* Generic part */
#define BSA_SUCCESS                         0

/*
 * Client Error definition
 */
#define BSA_ERROR_CLI_BAD_RSP_SIZE          100 /* Client received Bad response Size */
#define BSA_ERROR_CLI_NYI                   101 /* Client API not Yet Implemented */
#define BSA_ERROR_CLI_DISC_BUSY             102 /* Discovery Client API Already running */
#define BSA_ERROR_CLI_RES_ERROR             103 /* Client API Resource error (e.g. Mutex) */
#define BSA_ERROR_CLI_BAD_PARAM             104 /* Client BSA API parameter */
#define BSA_ERROR_CLI_NOT_CONNECTED         105 /* Client Not connected to Server */
#define BSA_ERROR_CLI_BUSY                  106 /* Client API Busy Already running */
#define BSA_ERROR_CLI_UIPC_OPEN             107 /* Client Enable to open UIPC */
#define BSA_ERROR_CLI_UIPC_SEND             108 /* Client UIPC send error */
#define BSA_ERROR_CLI_BAD_MSG               109 /* Client received bad message */
#define BSA_ERROR_CLI_BAD_PING_RSP          110 /* Client received bad ping response */
#define BSA_ERROR_CLI_NAME_TOO_LONG         111 /* Client Name too long */
#define BSA_ERROR_CLI_ALREADY_WAITING       112 /* Client is trying to send a new request while already waiting for response */
#define BSA_ERROR_CLI_TASK_ERROR            113 /* Client could not create a task */
#define BSA_ERROR_CLI_INTERNAL              114 /* Internal Client error */
#define BSA_ERROR_CLI_ALREADY_CONNECTED     115 /* Client was already connected */
#define BSA_ERROR_CLI_MEM_FULL              116 /* Client No more memory */

/*
 * Server Error definition
 */

/*
 * Generic Server errors
 */
#define BSA_ERROR_SRV_BAD_PARAM             200 /* Server Bad parameter */
#define BSA_ERROR_SRV_BAD_REQ_SIZE          201 /* Server Bad Request Size */
#define BSA_ERROR_SRV_BAD_MSG_ID            202 /* Server Bad message identifier */
#define BSA_ERROR_SRV_ALREADY_ACTIVE        203 /* Server this profile is already active */
#define BSA_ERROR_SRV_NYI                   204 /* Server API not Yet Implemented */
#define BSA_ERROR_SRV_BAD_CLIENT            205 /* Server Bad client number */
#define BSA_ERROR_SRV_MEM_FULL              206 /* Server No more memory */
#define BSA_ERROR_SRV_HW_ERROR              207 /* Server HardWare error */
#define BSA_ERROR_SRV_BLUETOOTH_DISABLE     208 /* Bluetooth is disabled */
#define BSA_ERROR_SRV_INTERNAL              209 /* Internal error */
#define BSA_ERROR_SRV_NOT_COMPILED          210 /* Server Functionality not compiled */
#define BSA_ERROR_SRV_WLAN_RESET            211 /* Common USB is not available. WLAN RESET */

/*
 * Security error
 */
#define BSA_ERROR_SRV_SEC_BOND_ACTIVE       300 /* Bond already ongoing */
#define BSA_ERROR_SRV_SEC_RM_DEV            301 /* Remove device fail: Device is connected */
#define BSA_ERROR_SRV_SEC_BOND_CANCEL_ERROR 302 /* Cancel bonding procedure fail */

/*
 * HH (Hid Host) Errors
 */
#define BSA_ERROR_SRV_HH_DSCP_TOO_BIG       400 /* DSCPINFO too big to fit in buffer */
#define BSA_ERROR_SRV_HH_DSCP_PENDING       401 /* DSCPINFO request already pending */
#define BSA_ERROR_SRV_HH_NOT_ENABLED        403 /* HH is not enabled */
#define BSA_ERROR_SRV_HH_NOT_SUPPORTED      404 /* Feature not supported */
#define BSA_ERROR_SRV_HH_NOT_ALLOWED        405 /* Command not allowed */
#define BSA_ERROR_SRV_HH_UNKNOWN_DEVICE     406 /* Unknown device */
#define BSA_ERROR_SRV_HH_ALREADY_ADDED      407 /* Already added */


/* Insert new HH errors here and don't forget to update the following line (offset) */
#define BSA_ERROR_SRV_HH_OFFSET             BSA_ERROR_SRV_HH_ALREADY_ADDED

#define BSA_ERROR_SRV_HH_HS_HID_NOT_READY  (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_HS_HID_NOT_READY)  /* handshake error : device not ready */
#define BSA_ERROR_SRV_HH_HS_INVALID_RPT_ID (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_HS_INVALID_RPT_ID) /* handshake error : invalid report ID */
#define BSA_ERROR_SRV_HH_HS_TRANS_NOT_SPT  (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_HS_TRANS_NOT_SPT)  /* handshake error : transaction not spt */
#define BSA_ERROR_SRV_HH_HS_INVALID_PARAM  (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_HS_INVALID_PARAM)  /* handshake error : invalid paremter */
#define BSA_ERROR_SRV_HH_HS_ERROR          (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_HS_ERROR)          /* handshake error : unspecified HS error */
#define BSA_ERROR_SRV_HH_ERR               (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_ERR)               /* general BSA HH error */
#define BSA_ERROR_SRV_HH_ERR_SDP           (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_ERR_SDP)           /* SDP error */
#define BSA_ERROR_SRV_HH_ERR_PROTO         (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_ERR_PROTO)         /* SET_Protocol error: only used in BSA_ERROR_SRV_HH_OPEN_EVT callback */
#define BSA_ERROR_SRV_HH_ERR_DB_FULL       (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_ERR_DB_FULL)       /* device database full error, used in BSA_ERROR_SRV_HH_OPEN_EVT */
#define BSA_ERROR_SRV_HH_ERR_TOD_UNSPT     (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_ERR_TOD_UNSPT)     /* type of device not supported */
#define BSA_ERROR_SRV_HH_ERR_NO_RES        (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_ERR_NO_RES)        /* out of system resources */
#define BSA_ERROR_SRV_HH_ERR_AUTH_FAILED   (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_ERR_AUTH_FAILED)   /* authentication fail */
#define BSA_ERROR_SRV_HH_ERR_HDL           (BSA_ERROR_SRV_HH_OFFSET + BTA_HH_ERR_HDL)           /* Bad Handle */

/*
 * OPS (Object Push Server) Errors
 */
#define BSA_ERROR_SRV_OPS_NO_ACCESS_PENDING 500 /* OPS Access Response sent with no access req pending */

/* OPC Error code */
#define BSA_ERROR_OPC_FAIL                  520   /* Object push failed. */
#define BSA_ERROR_OPC_NOT_FOUND             521   /* Object not found. */
#define BSA_ERROR_OPC_NO_PERMISSION         522   /* Operation not authorized. */
#define BSA_ERROR_OPC_SRV_UNAVAIL           523   /* Service unavaliable */
#define BSA_ERROR_SRV_OPC_ALREADY_ACTIVE    524   /* OP Server is already active */


/*
 * File System Errors
 */
#define BSA_ERROR_SRV_FS_PERMISSION         600   /* Permission Error */
#define BSA_ERROR_SRV_FS_NO_FILE            601   /* File doesn't exist */


/*
 * FTC (File Transfer Client) Errors
 */

#define BSA_ERROR_FTC_XML_PARS      650   /* XML folder parsing Failure  */
#define BSA_ERROR_FTC_FIRST         651   /* Offset for BTA/BSA error conversion */
#define BSA_ERROR_FTC_FAIL          (BTA_FTC_FAIL          + BSA_ERROR_FTC_FIRST) /* Generic Failure */
#define BSA_ERROR_FTC_NO_PERMISSION (BTA_FTC_NO_PERMISSION + BSA_ERROR_FTC_FIRST) /* Permision denied */
#define BSA_ERROR_FTC_NOT_FOUND     (BTA_FTC_NOT_FOUND     + BSA_ERROR_FTC_FIRST) /* File not found */
#define BSA_ERROR_FTC_FULL          (BTA_FTC_FULL          + BSA_ERROR_FTC_FIRST) /* File too big or FS full */
#define BSA_ERROR_FTC_BUSY          (BTA_FTC_BUSY          + BSA_ERROR_FTC_FIRST) /* Another operating ongoing */
#define BSA_ERROR_FTC_ABORTED       (BTA_FTC_ABORTED       + BSA_ERROR_FTC_FIRST) /* Operation was aborted */
#define BSA_ERROR_FTC_SERVICE_UNAVL (BTA_FTC_SERVICE_UNAVL + BSA_ERROR_FTC_FIRST) /* Service not available on peer */
#define BSA_ERROR_FTC_SDP_ERR       (BTA_FTC_SDP_ERR       + BSA_ERROR_FTC_FIRST) /* SDP error */
#define BSA_ERROR_FTC_OBX_ERR       (BTA_FTC_OBX_ERR       + BSA_ERROR_FTC_FIRST) /* OBEX error */
#define BSA_ERROR_FTC_OBX_TOUT      (BTA_FTC_OBX_TOUT      + BSA_ERROR_FTC_FIRST) /* OBEX timeout */

/*
* AV (Audio Video source) Errors
 */
#define BSA_ERROR_SRV_AV_NOT_ENABLED           700  /* AV is not enabled */
#define BSA_ERROR_SRV_AV_FEEDING_NOT_SUPPORTED 701  /* Requested Feeding not supported */
#define BSA_ERROR_SRV_AV_BUSY                  702  /* Another operation ongoing */
#define BSA_ERROR_SRV_AV_NOT_OPENED            703  /* No AV link opened */
#define BSA_ERROR_SRV_AV_NOT_STARTED           704  /* AV is not Started */
#define BSA_ERROR_SRV_AV_NOT_STOPPED           705  /* AV is not Stopped */
#define BSA_ERROR_SRV_AV_CP_NOT_SUPPORTED      706  /* Content protection is not supported by all headsets */
#define BSA_ERROR_SRV_AV_BCST_NOT_SUPPORTED    707  /* Broadcast AV not supported */
#define BSA_ERROR_SRV_AV_BR_CT_NOT_SUPPORTED   708  /* Bit Rate Control not supported */

/* Insert new AV errors here and don't forget to update the following line (offset) */
#define BSA_ERROR_SRV_AV_OFFSET                 BSA_ERROR_SRV_AV_BR_CT_NOT_SUPPORTED

#define BSA_ERROR_SRV_AV_FAIL           (BSA_ERROR_SRV_AV_OFFSET + BTA_AV_FAIL)                     /* Generic failure */
#define BSA_ERROR_SRV_AV_FAIL_SDP       (BSA_ERROR_SRV_AV_OFFSET + BTA_AV_FAIL_SDP)       /* Service not found */
#define BSA_ERROR_SRV_AV_FAIL_STREAM    (BSA_ERROR_SRV_AV_OFFSET + BTA_AV_FAIL_STREAM)    /* Stream connection failed */
#define BSA_ERROR_SRV_AV_FAIL_RESOURCES (BSA_ERROR_SRV_AV_OFFSET + BTA_AV_FAIL_RESOURCES) /* No resources */
#define BSA_ERROR_SRV_AV_FAIL_ROLE      (BSA_ERROR_SRV_AV_OFFSET + BTA_AV_FAIL_ROLE)      /* Failed due to role management issues */

/*
 * AVK (Audio Video Sink) Errors
 */
#define BSA_ERROR_SRV_AVK_NOT_ENABLED             800    /* AVK is not enabled */
#define BSA_ERROR_SRV_AVK_RECEIVING_NOT_SUPPORTED 801    /* Bad Format */
#define BSA_ERROR_SRV_AVK_BUSY                    802    /* Another operation ongoing */
#define BSA_ERROR_SRV_AVK_CMD_NOT_ALLOWED         803    /* BSA could not perform the requested operation */

/* Insert new AV errors here and don't forget to update the following line (offset) */
#define BSA_ERROR_SRV_AVK_OFFSET            BSA_ERROR_SRV_AVK_CMD_NOT_ALLOWED

#define BSA_ERROR_SRV_AV_FAIL      (BSA_ERROR_SRV_AV_OFFSET + BTA_AV_FAIL)           /* Generic failure */
#define BSA_ERROR_SRV_AV_SDP       (BSA_ERROR_SRV_AV_OFFSET + BTA_AV_FAIL_SDP)       /* service not found (SDP) */
#define BSA_ERROR_SRV_AV_STREAM    (BSA_ERROR_SRV_AV_OFFSET + BTA_AV_FAIL_STREAM)    /* stream connection failed */
#define BSA_ERROR_SRV_AV_RESOURCES (BSA_ERROR_SRV_AV_OFFSET + BTA_AV_FAIL_RESOURCES) /* no resources */

/*
 * Discovery error
 */
#define BSA_ERROR_SRV_DEV_DISC_BUSY         900 /* Device Discovery already ongoing */
#define BSA_ERROR_SRV_DEV_INFO_BUSY         901 /* Device Info already ongoing */
#define BSA_ERROR_SRV_DISC_ABORT_ERROR      902 /* Device Discovery Cancel not allowed */

/*
 * AG (Audio Gateway) Errors
 */
/* Insert new AG errors here and don't forget to update the following line (offset) */
#define BSA_ERROR_SRV_AG_OFFSET            1000
#define BSA_ERROR_SRV_AG_FAIL_SDP          (BSA_ERROR_SRV_AG_OFFSET+BTA_AG_FAIL_SDP)       /* Open failed due to SDP */
#define BSA_ERROR_SRV_AG_FAIL_RFCOMM       (BSA_ERROR_SRV_AG_OFFSET+BTA_AG_FAIL_RFCOMM)    /* Open failed due to RFCOMM */
#define BSA_ERROR_SRV_AG_FAIL_RESOURCES    (BSA_ERROR_SRV_AG_OFFSET+BTA_AG_FAIL_RESOURCES) /* register failed due to resources issue */
#define BSA_ERROR_SRV_AG_BUSY              (BSA_ERROR_SRV_AG_FAIL_RESOURCES+1)

/*
 * TM (Test Mode) Errors
 */
#define BSA_ERROR_SRV_TM_VSC_BUSY          1100 /* VSC Busy (pending VSC) */
#define BSA_ERROR_SRV_TM_HCI_ERROR         1101 /* HCI error (chip rejected cmd) */
#define BSA_ERROR_SRV_TM_HCI_CMD_ERROR     1102 /* Incorrect HCI Cmd error (Stack rejected HCI cmd) */

/*
 * DM Errors
 */
#define BSA_ERROR_SRV_DM_3DTV_LOCK_FAIL     1200 /* Not able to synchronize with peer Master 3D device */
#define BSA_ERROR_SRV_DM_TBFC_UNSUPPORTED   1201 /* TBFC Not supported by FW */
#define BSA_ERROR_SRV_DM_TBFC_NOT_COMPILED  1202 /* TBFC Not compiled in BSA */
#define BSA_ERROR_SRV_DM_SWITCH_STACK_FAIL  1203 /* Switch Stack Failed */

/*
 * PBS (Phone Book Server) Errors
 */
#define BSA_ERROR_SRV_PBS_OFFSET           1300
#define BSA_ERROR_SRV_PBS_FAIL            (BSA_ERROR_SRV_PBS_OFFSET+BTA_PBS_FAIL)  /* any PBS obex error */

/*
 * PBC (Phone Book Client) Errors
 */
#define BSA_ERROR_PBC_XML_PARS      1350   /* XML folder parsing Failure  */
#define BSA_ERROR_PBC_FIRST         1351   /* Offset for BTA/BSA error conversion */
#define BSA_ERROR_PBC_FAIL          (BTA_PBC_FAIL          + BSA_ERROR_PBC_FIRST) /* Generic Failure */
#define BSA_ERROR_PBC_NO_PERMISSION (BTA_PBC_NO_PERMISSION + BSA_ERROR_PBC_FIRST) /* Permision denied */
#define BSA_ERROR_PBC_NOT_FOUND     (BTA_PBC_NOT_FOUND     + BSA_ERROR_PBC_FIRST) /* File not found */
#define BSA_ERROR_PBC_FULL          (BTA_PBC_FULL          + BSA_ERROR_PBC_FIRST) /* File too big or FS full */
#define BSA_ERROR_PBC_BUSY          (BTA_PBC_BUSY          + BSA_ERROR_PBC_FIRST) /* Another operating ongoing */
#define BSA_ERROR_PBC_ABORTED       (BTA_PBC_ABORTED       + BSA_ERROR_PBC_FIRST) /* Operation was aborted */


/*
 * HL Errors
 */
#define BSA_ERROR_SRV_HL_NOT_ENABLED        1400 /* HL Has not been enabled */
#define BSA_ERROR_SRV_HL_NOT_APP_REGISTERED 1401 /* This application/service has not been registered */
#define BSA_ERROR_SRV_HL_BUSY               1402 /* HL is busy (e.g initiating an outgoing connection) */
#define BSA_ERROR_SRV_HL_ALREADY_CONNECTED  1403 /* This peer end point is already connected */


/* Insert new A errors here and don't forget to update the following line (offset) */
#define BSA_ERROR_SRV_HL_OFFSET             BSA_ERROR_SRV_HL_ALREADY_CONNECTED

#define BSA_ERROR_SRV_HL_FAIL               (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_FAIL) /* generic failure */
#define BSA_ERROR_SRV_HL_ABORTED            (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_ABORTED) /* Connection has been aborted */
#define BSA_ERROR_SRV_HL_NO_RESOURCE        (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_NO_RESOURCE) /* No resource to complete */
#define BSA_ERROR_SRV_HL_LAST_ITEM          (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_LAST_ITEM) /* Last item reached */
#define BSA_ERROR_SRV_HL_DUPLICATE_APP_ID   (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_DUPLICATE_APP_ID) /* Duplicated Application Id */
#define BSA_ERROR_SRV_HL_INVALID_APP_HANDLE (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_INVALID_APP_HANDLE) /* Invalid Application Handle */
#define BSA_ERROR_SRV_HL_INVALID_MCL_HANDLE (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_INVALID_MCL_HANDLE) /* Invalid Control handle */
#define BSA_ERROR_SRV_HL_MCAP_REG_FAIL      (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_MCAP_REG_FAIL) /* Registration fail */
#define BSA_ERROR_SRV_HL_MDEP_CO_FAIL       (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_MDEP_CO_FAIL) /* MDEP Call-Out Registration fail */
#define BSA_ERROR_SRV_HL_ECHO_CO_FAIL       (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_ECHO_CO_FAIL) /* Echo Call-Out Registration fail */
#define BSA_ERROR_SRV_HL_MDL_CFG_CO_FAIL    (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_MDL_CFG_CO_FAIL) /* Load Call-Out Registration fail */
#define BSA_ERROR_SRV_HL_SDP_NO_RESOURCE    (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_SDP_NO_RESOURCE) /* No resource to perform SDP request */
#define BSA_ERROR_SRV_HL_SDP_FAIL           (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_SDP_FAIL) /* SDP Failure */
#define BSA_ERROR_SRV_HL_NO_CCH             (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_NO_CCH) /* No Control Connection */
#define BSA_ERROR_SRV_HL_NO_MCL             (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_NO_MCL) /* No Control Handle (for Echo test) */
#define BSA_ERROR_SRV_HL_NO_FIRST_RELIABLE  (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_NO_FIRST_RELIABLE) /* The first Data channel is not Reliable */
#define BSA_ERROR_SRV_HL_INVALID_DCH_CFG    (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_INVALID_DCH_CFG) /* Invalid Data channel configuration */
#define BSA_ERROR_SRV_HL_INVALID_MDL_HANDLE (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_INVALID_MDL_HANDLE) /* Invalid Control Handle */
#define BSA_ERROR_SRV_HL_INVALID_BD_ADDR    (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_INVALID_BD_ADDR) /* Invalid BdAddr */
#define BSA_ERROR_SRV_HL_INVALID_RECONNECT_CFG  (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_INVALID_RECONNECT_CFG) /* Invalid Reconnect Configuration */
#define BSA_ERROR_SRV_HL_ECHO_TEST_BUSY     (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_ECHO_TEST_BUSY) /* An echo test is already pending */
#define BSA_ERROR_SRV_HL_INVALID_LOCAL_MDEP_ID  (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_INVALID_LOCAL_MDEP_ID) /* Invalid Local MDEP Id */
#define BSA_ERROR_SRV_HL_INVALID_MDL_ID     (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_INVALID_MDL_ID) /* Invalid MDLD ID */
#define BSA_ERROR_SRV_HL_NO_MDL_ID_FOUND    (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_NO_MDL_ID_FOUND) /* MDLD ID not found */
#define BSA_ERROR_SRV_HL_DCH_BUSY           (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_DCH_BUSY) /* DCH is congested*/
#define BSA_ERROR_SRV_HL_INVALID_CTRL_PSM   (BSA_ERROR_SRV_HL_OFFSET + BTA_HL_STATUS_INVALID_CTRL_PSM) /* Invalid Control PSM */


/*
 * DG (DataGateway) errors
 */
#define BSA_ERROR_SRV_DG_OPEN_FAIL          1500 /* DG Open fail */
#define BSA_ERROR_SRV_DG_CONNECTED          1501 /* DG Server connected */
#define BSA_ERROR_SRV_DG_FIND_SERVICE_FAIL  1502 /* Generic Find Service Failure */

/*
 * BLE errors
 */
#define BSA_ERROR_SRV_BLE_FAIL              1600 /* BLE Open fail */
#define BSA_ERROR_SRV_BLE_NOT_ENABLED       1601
#define BSA_ERROR_SRV_BLE_NO_RESOURCE       1602
#define BSA_ERROR_SRV_BLE_INVALID_PARAM     1603

#define BSA_ERROR_SRV_BLE_OFFSET            BSA_ERROR_SRV_BLE_NO_RESOURCE


/*
 * HD (Hid Device) Errors
 */
#define BSA_ERROR_SRV_HD_FAIL               1700 /* HID Device Fail */
#define BSA_ERROR_SRV_HD_NOT_ENABLED        1701
#define BSA_ERROR_SRV_HD_UNKNOWN_KEY_TYPE   1702
#define BSA_ERROR_SRV_HD_AREADY_CONNECTED   1703 /* HID device already connected */
#define BSA_ERROR_SRV_HD_WRONG_RPT_LEN      1704
/* Insert new HD errors here and don't forget to update the following line (offset) */
#define BSA_ERROR_SRV_HD_OFFSET             BSA_ERROR_SRV_HD_WRONG_RPT_LEN


/*
 * MAP Server Errors
 */
#define BSA_ERROR_SRV_MCS_OFFSET           1800
#define BSA_ERROR_SRV_MCS_FAIL            (BSA_ERROR_SRV_MCS_OFFSET +   BTA_MA_STATUS_FAIL)  /* any MCS error */

/*
 * MAP Client Errors
 */

#define BSA_ERROR_MCE_FIRST                 1850   /* Offset for BTA/BSA error conversion */
#define BSA_ERROR_MCE_FAIL                  (BTA_MA_STATUS_FAIL             + BSA_ERROR_MCE_FIRST) /* Generic Failure */
#define BSA_ERROR_MCE_STATUS_ABORTED        (BTA_MA_STATUS_ABORTED          + BSA_ERROR_MCE_FIRST)
#define BSA_ERROR_MCE_STATUS_NO_RESOURCE    (BTA_MA_STATUS_NO_RESOURCE      + BSA_ERROR_MCE_FIRST)
#define BSA_ERROR_MCE_STATUS_EACCES         (BTA_MA_STATUS_EACCES           + BSA_ERROR_MCE_FIRST)
#define BSA_ERROR_MCE_STATUS_ENOTEMPTY      (BTA_MA_STATUS_ENOTEMPTY        + BSA_ERROR_MCE_FIRST)
#define BSA_ERROR_MCE_STATUS_EOF            (BTA_MA_STATUS_EOF              + BSA_ERROR_MCE_FIRST)
#define BSA_ERROR_MCE_STATUS_EODIR          (BTA_MA_STATUS_EODIR            + BSA_ERROR_MCE_FIRST)
#define BSA_ERROR_MCE_STATUS_ENOSPACE       (BTA_MA_STATUS_ENOSPACE         + BSA_ERROR_MCE_FIRST) /* Returned in bta_fs_ci_open if no room */
#define BSA_ERROR_MCE_STATUS_DUPLICATE_ID   (BTA_MA_STATUS_DUPLICATE_ID     + BSA_ERROR_MCE_FIRST)
#define BSA_ERROR_MCE_STATUS_ID_NOT_FOUND   (BTA_MA_STATUS_ID_NOT_FOUND     + BSA_ERROR_MCE_FIRST)
#define BSA_ERROR_MCE_STATUS_FULL           (BTA_MA_STATUS_FULL             + BSA_ERROR_MCE_FIRST) /* reach the max packet size */
#define BSA_ERROR_MCE_STATUS_UNSUP_FEATURES (BTA_MA_STATUS_UNSUP_FEATURES   + BSA_ERROR_MCE_FIRST) /* feature is not suppored on the peer device */
#define BSA_ERROR_MCE_BUSY                  (BSA_ERROR_MCE_STATUS_UNSUP_FEATURES      + 1) /* Another operation ongoing */

/*
 * SIM Access Server Errors
 */

#define BSA_ERROR_SC_FIRST                 1900   /* Offset for BTA/BSA error conversion */
#define BSA_ERROR_SC_FAIL                  1901   /* Generic Failure */
#define BSA_ERROR_SC_BUSY                  1902   /* Another operation ongoing */

/*
 * SIM Access Client Errors
 */
#define BSA_ERROR_SAC_FAIL                  1950    /* Generic failure */
#define BSA_ERROR_SAC_BUSY                  1951    /* Another operation is ongoing */
#define BSA_ERROR_SAC_NO_RESOURCE           1952    /* Failed to create connection because a
                                                       connection already exist */
#define BSA_ERROR_SAC_NO_SERVER             1953    /* Failed to initiate connection to server */
#define BSA_ERROR_SAC_NO_SERVICE            1954    /* Remote device does not support SAP server
                                                       role */
#define BSA_ERROR_SAC_SERVER_ERROR          1955    /* Server does not accept SAP connection */
#define BSA_ERROR_SAC_SERVER_RESP           1956    /* Server sends error code in response
                                                       (check response code) */
#define BSA_ERROR_SAC_NO_CONN               1957    /* Disconnect request failed because there's
                                                       no active connection */

/*
 * PAN Errors
 */

#define BSA_ERROR_PAN_FIRST                 2000   /* Offset for BTA/BSA error conversion */
#define BSA_ERROR_PAN_FAIL                  2001
#define BSA_ERROR_PAN_ALREADY_CONNECTED     2002   /* peer device already connected */
#define BSA_ERROR_PAN_NO_MORE_CONNECTION    2003   /* no more connection */

#endif /* BSA_STATUS_H */
