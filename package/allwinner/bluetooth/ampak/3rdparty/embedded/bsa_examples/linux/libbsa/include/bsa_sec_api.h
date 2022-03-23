/*****************************************************************************
 **
 **  Name:           bsa_sec_api.h
 **
 **  Description:    This is the public interface file for Security part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2015, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_SEC_API_H
#define BSA_SEC_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"
#include "bta_api.h"
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
#include "btm_api.h"
#include "smp_api.h"
#endif


#define BSA_OOB_INCLUDED                  BTM_OOB_INCLUDED

/* Device Management Callback Events */
typedef enum
{
    BSA_SEC_LINK_UP_EVT,        /* A device is physically connected (for info) */
    BSA_SEC_LINK_DOWN_EVT,      /* A device is physically disconnected (for info)*/
    BSA_SEC_PIN_REQ_EVT,        /* PIN code Request */
    BSA_SEC_AUTH_CMPL_EVT,      /* pairing/authentication complete */
    BSA_SEC_BOND_CANCEL_CMPL_EVT,         /* Bond cancel complete indication */
    BSA_SEC_AUTHORIZE_EVT,      /* Authorization request */
    BSA_SEC_SP_CFM_REQ_EVT,     /* Simple Pairing confirm request */
    BSA_SEC_SP_KEY_NOTIF_EVT,   /* Simple Pairing Passkey Notification */
    BSA_SEC_SP_KEY_REQ_EVT,     /* Simple Pairing Passkey request Notification */
    BSA_SEC_SP_RMT_OOB_EVT,     /* Simple Pairing Remote OOB Data request */
    BSA_SEC_LOCAL_OOB_DATA_EVT,   /* Response to read local OOB info request */
    BSA_SEC_SP_KEYPRESS_EVT,    /* Simple Pairing Key press notification event */
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    BSA_SEC_BLE_KEY_EVT,          /* BLE Key information event */
    BSA_SEC_BLE_PASSKEY_REQ_EVT,  /* BLE Passkey Request */
#endif
    BSA_SEC_SUSPENDED_EVT,      /* An ACL Link is in Suspended mode */
    BSA_SEC_RESUMED_EVT,        /* An ACL Link is in Resumed mode */
    BSA_SEC_RSSI_EVT,           /* RSSI report received */

} tBSA_SEC_EVT;

#define BSA_SEC_NOT_AUTH        BTA_DM_NOT_AUTH     /* to refuse authorization */
#define BSA_SEC_AUTH_TEMP       BTA_DM_AUTH_TEMP    /* to grant access temporarily (only this session/connection)*/
#define BSA_SEC_AUTH_PERM       BTA_DM_AUTH_PERM    /* to grant permanent access */
typedef tBTA_AUTH_RESP tBSA_SEC_AUTH_RESP;

#define BSA_SEC_NONE            BTA_SEC_NONE        /* No security/authentication needed (insecure) */
#define BSA_SEC_AUTHENTICATION  BTA_SEC_AUTHENTICATE/* Authentication/pairing needed*/
#define BSA_SEC_AUTHORIZATION   BTA_SEC_AUTHORIZE   /* Authorization needed to access services */
#define BSA_SEC_ENCRYPTION      BTA_SEC_ENCRYPT     /* Encryption needed */
typedef tBTA_SEC tBSA_SEC_AUTH;

/* Input/Output capabilities used for Simple Pairing */
#define BSA_SEC_IO_CAP_OUT      BTA_IO_CAP_OUT          /* 0 DisplayOnly */
#define BSA_SEC_IO_CAP_IN       BTA_IO_CAP_IN           /* 1 KeyboardOnly */
#define BSA_SEC_IO_CAP_IO       BTA_IO_CAP_IO           /* 2 DisplayYesNo */
#define BSA_SEC_IO_CAP_NONE     BTA_IO_CAP_NONE         /* 3 NoInputNoOutput */
#if BLE_INCLUDED == TRUE && SMP_INCLUDED == TRUE
#define BSA_SEC_IO_CAP_KBDISP   BTA_IO_CAP_KBDISP       /* 4 Keyboard display */
#endif


typedef tBTA_IO_CAP tBSA_SEC_IO_CAP;

#ifndef BSA_SEC_AUTH_REQ_SETTING
#define BSA_SEC_AUTH_REQ_SETTING  BTM_AUTH_SPGB_NO //BTM_AUTH_AP_YES
#endif

#ifndef BSA_BLE_SEC_AUTH_REQ_SETTING
#define BSA_BLE_SEC_AUTH_REQ_SETTING  BTM_AUTH_SPGB_NO //BTM_AUTH_AP_YES
#endif

/*
 * Service Mask
 * Service Mask is used when several services Id are needed (e.g. Services search)
 */
#define BSA_RES_SERVICE_MASK        BTA_RES_SERVICE_MASK         /* Reserved */
#define BSA_SPP_SERVICE_MASK        BTA_SPP_SERVICE_MASK         /* Serial port profile. */
#define BSA_DUN_SERVICE_MASK        BTA_DUN_SERVICE_MASK         /* Dial-up networking profile. */
#define BSA_A2DP_SRC_SERVICE_MASK   BTA_A2DP_SRC_SERVICE_MASK    /* A2DP Source */
#define BSA_AVRCP_TG_SERVICE_MASK   BTA_AVRCP_TG_SERVICE_MASK    /* AVRCP Target */
#define BSA_HSP_SERVICE_MASK        BTA_HSP_SERVICE_MASK         /* Headset profile. */
#define BSA_HFP_SERVICE_MASK        BTA_HFP_SERVICE_MASK         /* Hands-free profile. */
#define BSA_OPP_SERVICE_MASK        BTA_OPP_SERVICE_MASK         /* Object push  */
#define BSA_FTP_SERVICE_MASK        BTA_FTP_SERVICE_MASK         /* File transfer */
#define BSA_CTP_SERVICE_MASK        BTA_CTP_SERVICE_MASK         /* Cordless Terminal */
#define BSA_ICP_SERVICE_MASK        BTA_ICP_SERVICE_MASK         /* Intercom Terminal */
#define BSA_SYNC_SERVICE_MASK       BTA_SYNC_SERVICE_MASK        /* Synchronization */
#define BSA_BPP_SERVICE_MASK        BTA_BPP_SERVICE_MASK         /* Print server */
#define BSA_BIP_SERVICE_MASK        BTA_BIP_SERVICE_MASK         /* Basic Imaging */
#define BSA_PANU_SERVICE_MASK       BTA_PANU_SERVICE_MASK        /* PAN User */
#define BSA_NAP_SERVICE_MASK        BTA_NAP_SERVICE_MASK         /* PAN Network access point */
#define BSA_GN_SERVICE_MASK         BTA_GN_SERVICE_MASK          /* PAN Group Ad-hoc networks */
#define BSA_SAP_SERVICE_MASK        BTA_SAP_SERVICE_MASK         /* PAN Group Ad-hoc networks */
#define BSA_A2DP_SERVICE_MASK       BTA_A2DP_SERVICE_MASK        /* Advanced audio distribution */
#define BSA_AVRCP_SERVICE_MASK      BTA_AVRCP_SERVICE_MASK       /* A/V remote control */
#define BSA_HID_SERVICE_MASK        BTA_HID_SERVICE_MASK         /* HID */
#define BSA_HID_DEVICE_SERVICE_MASK BTA_HID_DEVICE_SERVICE_MASK  /* HID Device */
#define BSA_VDP_SERVICE_MASK        BTA_VDP_SERVICE_MASK         /* Video distribution */
#define BSA_PBAP_SERVICE_MASK       BTA_PBAP_SERVICE_MASK        /* Phone Book */
#define BSA_HSP_HS_SERVICE_MASK     BTA_HSP_HS_SERVICE_MASK      /* HFP HS role */
#define BSA_HFP_HS_SERVICE_MASK     BTA_HFP_HS_SERVICE_MASK      /* HSP HS role */
#define BSA_MAS_SERVICE_MASK        BTA_MAS_SERVICE_MASK         /* Message Access Profile */
#define BSA_MN_SERVICE_MASK         BTA_MN_SERVICE_MASK          /* Message Notification Profile */
#define BSA_HL_SERVICE_MASK         BTA_HL_SERVICE_MASK          /* Health Profile Profile */
#define BSA_BLE_SERVICE_MASK        BTA_BLE_SERVICE_MASK         /* GATT based service */


#define BSA_ALL_SERVICE_MASK        (BTA_ALL_SERVICE_MASK & ~BTA_RES_SERVICE_MASK)  /* All services supported by BSA. */

typedef tBTA_SERVICE_MASK tBSA_SERVICE_MASK;

/*
 * Service ID
 * Service ID is used when only one service Id is needed (e.g. Service Authorization)
 */
#define BSA_RES_SERVICE_ID          BTA_RES_SERVICE_ID         /* Reserved */
#define BSA_SPP_SERVICE_ID          BTA_SPP_SERVICE_ID         /* Serial port profile. */
#define BSA_DUN_SERVICE_ID          BTA_DUN_SERVICE_ID         /* Dial-up networking profile. */
#define BSA_A2DP_SRC_SERVICE_ID     BTA_A2DP_SRC_SERVICE_ID    /* A2DP Source */
#define BSA_AVRCP_TG_SERVICE_ID     BTA_AVRCP_TG_SERVICE_ID    /* AVRCP Target */
#define BSA_HSP_SERVICE_ID          BTA_HSP_SERVICE_ID         /* Headset profile. */
#define BSA_HFP_SERVICE_ID          BTA_HFP_SERVICE_ID         /* Hands-free profile. */
#define BSA_OPP_SERVICE_ID          BTA_OPP_SERVICE_ID         /* Object push  */
#define BSA_FTP_SERVICE_ID          BTA_FTP_SERVICE_ID         /* File transfer */
#define BSA_CTP_SERVICE_ID          BTA_CTP_SERVICE_ID         /* Cordless Terminal */
#define BSA_ICP_SERVICE_ID          BTA_ICP_SERVICE_ID         /* Intercom Terminal */
#define BSA_SYNC_SERVICE_ID         BTA_SYNC_SERVICE_ID        /* Synchronization */
#define BSA_BPP_SERVICE_ID          BTA_BPP_SERVICE_ID         /* Basic printing profile */
#define BSA_BIP_SERVICE_ID          BTA_BIP_SERVICE_ID         /* Basic Imaging profile */
#define BSA_PANU_SERVICE_ID         BTA_PANU_SERVICE_ID        /* PAN User */
#define BSA_NAP_SERVICE_ID          BTA_NAP_SERVICE_ID         /* PAN Network access point */
#define BSA_GN_SERVICE_ID           BTA_GN_SERVICE_ID          /* PAN Group Ad-hoc networks */
#define BSA_SAP_SERVICE_ID          BTA_SAP_SERVICE_ID         /* SIM Access profile */
#define BSA_A2DP_SERVICE_ID         BTA_A2DP_SERVICE_ID        /* Advanced audio distribution */
#define BSA_AVRCP_SERVICE_ID        BTA_AVRCP_SERVICE_ID       /* A/V remote control */
#define BSA_HID_SERVICE_ID          BTA_HID_SERVICE_ID         /* HID */
#define BSA_HID_DEVICE_SERVICE_ID   BTA_HID_DEVICE_SERVICE_ID  /* HID Device */
#define BSA_VDP_SERVICE_ID          BTA_VDP_SERVICE_ID         /* Video distribution */
#define BSA_PBAP_SERVICE_ID         BTA_PBAP_SERVICE_ID        /* PhoneBook Access */
#define BSA_HFP_HS_SERVICE_ID       BTA_HFP_HS_SERVICE_ID      /* HFP HS role */
#define BSA_HSP_HS_SERVICE_ID       BTA_HSP_HS_SERVICE_ID      /* HSP HS role */
#define BSA_MAP_SERVICE_ID          BTA_MAP_SERVICE_ID         /* Message Access Profile */
#define BSA_HL_SERVICE_ID           BTA_HDP_SERVICE_ID         /* Health profile */
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
#define BSA_BLE_SERVICE_ID          BTA_BLE_SERVICE_ID         /* BLE service */
#endif
#define BSA_MAX_SERVICE_ID          BTA_MAX_SERVICE_ID

typedef UINT8 tBSA_SERVICE_ID;

/* Simple Pairing: Passkey model */
#define BSA_SP_KEY_STARTED          BTA_SP_KEY_STARTED      /* passkey entry started */
#define BSA_SP_KEY_ENTERED          BTA_SP_KEY_ENTERED      /* passkey digit entered */
#define BSA_SP_KEY_ERASED           BTA_SP_KEY_ERASED       /* passkey digit erased */
#define BSA_SP_KEY_CLEARED          BTA_SP_KEY_CLEARED      /* passkey cleared */
#define BSA_SP_KEY_COMPLETE         BTA_SP_KEY_COMPLT       /* passkey entry completed */

typedef UINT8 tBSA_SEC_SP_KEY_TYPE;


#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
#define BSA_LE_KEY_PENC      BTM_LE_KEY_PENC       /* encryption information of peer device */
#define BSA_LE_KEY_PID       BTM_LE_KEY_PID        /* identity key of the peer device */
#define BSA_LE_KEY_PCSRK     BTM_LE_KEY_PCSRK      /* peer SRK */
#define BSA_LE_KEY_LENC      BTM_LE_KEY_LENC       /* master role security information:div */
#define BSA_LE_KEY_LID       BTM_LE_KEY_LID        /* master device ID key */
#define BSA_LE_KEY_LCSRK     BTM_LE_KEY_LCSRK      /* local CSRK has been deliver to peer */
typedef UINT8 tBSA_LE_KEY_TYPE;
#endif

#define BSA_TRANSPORT_UNKNOWN   0
#define BSA_TRANSPORT_BR_EDR    BT_TRANSPORT_BR_EDR
#define BSA_TRANSPORT_LE        BT_TRANSPORT_LE
typedef tBT_TRANSPORT tBSA_TRANSPORT;

/*
 * Security messages
 */

/* Structure associated with BSA_SEC_PIN_REQ_EVT */
/* A PIN code is needed for Pairing */
typedef struct
{
    BD_ADDR bd_addr; /* BD address peer device. */
    BD_NAME bd_name; /* Name of peer device. */
    DEV_CLASS class_of_device; /* Class of Device */
} tBSA_SEC_PIN_REQ;

/* Structure associated with BSA_SEC_AUTH_CMPL_EVT */
/* Pairing complete */
typedef struct
{
    BD_ADDR bd_addr; /* BD address peer device. */
    BD_NAME bd_name; /* Name of peer device. */
    BOOLEAN key_present; /* Valid link key value in key element */
    LINK_KEY key; /* Link key associated with peer device. */
    UINT8 key_type; /* The type of Link Key */
    BOOLEAN success; /* TRUE of authentication succeeded, FALSE if failed. */
    UINT8 fail_reason; /* failure reason code */
} tBSA_SEC_AUTH_CMPL;

/* Structure associated with BSA_SEC_BOND_CANCEL_CMPL_EVT */
typedef struct
{
    tBSA_STATUS status; /* Result of cancel */
} tBSA_SEC_BOND_CANCEL_CMPL;

/* Structure associated with BSA_SEC_AUTHORIZE_EVT */
/* Remote device requests access to a service */
typedef struct
{
    BD_ADDR bd_addr; /* BD address peer device. */
    BD_NAME bd_name; /* Name of peer device. */
    tBSA_SERVICE_ID service; /* Service ID to authorize. */
} tBSA_SEC_AUTHORIZE;

/* Structure associated with BSA_SEC_LINK_UP_EVT */
/* For info only => indicate that a physical link is established */
typedef struct
{
    BD_ADDR bd_addr; /* BD address peer device. */
    DEV_CLASS class_of_device;
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    tBSA_TRANSPORT  link_type;          /* link type */
#endif
} tBSA_SEC_LINK_UP;

/* Structure associated with BSA_SEC_LINK_DOWN_EVT */
/* For info only => indicate that a physical link is released */
typedef struct
{
    BD_ADDR bd_addr; /* BD address peer device. */
    UINT8 status; /* connection open/closed */
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    tBSA_TRANSPORT  link_type;          /* link type */
#endif
} tBSA_SEC_LINK_DOWN;

/*
 * Simple Pairing Input/Output Capability
 */

/* Structure associated with BSA_SEC_SP_CFM_REQ_EVT */
/* Simple Pairing request user accept/refuse */
typedef struct
{
    BD_ADDR bd_addr; /* peer address */
    DEV_CLASS class_of_device; /* peer CoD */
    BD_NAME bd_name; /* peer device name */
    int num_val; /* the numeric value for comparison. If just_works, do not show this number to UI */
    BOOLEAN just_works; /* TRUE, if "Just Works" association model */
} tBSA_SEC_SP_CFM_REQ;

/* Structure associated with BSA_SEC_SP_KEYPRESS_EVT */
typedef struct
{
    BD_ADDR bd_addr; /* peer address */
    tBSA_SEC_SP_KEY_TYPE notif_type;
} tBSA_SEC_SP_KEY_PRESS;

/* Structure associated with BSA_SEC_SP_KEY_NOTIF_EVT */
typedef struct
{
    BD_ADDR bd_addr; /* peer address */
    DEV_CLASS class_of_device; /* peer CoD */
    BD_NAME bd_name; /* peer device name */
    int passkey; /* the numeric value for comparison. If just_works, do not show this number to UI */
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    tBSA_TRANSPORT  link_type;          /* link type */
#endif
} tBSA_SEC_SP_KEY_NOTIF;

/* Callback data for BSA_SEC_LOCAL_OOB_DATA_EVT event */
typedef struct
{
    BOOLEAN valid;
    BT_OCTET16 c;
    BT_OCTET16 r;
} tBSA_SEC_LOCAL_OOB_MSG;

/* Structure associated with BSA_SEC_SP_RMT_OOB_EVT */
typedef struct
{
    BD_ADDR bd_addr; /* peer address */
    DEV_CLASS class_of_device; /* peer CoD */
    BD_NAME bd_name; /* peer device name */
} tBSA_SEC_SP_RMT_OOB;

/* Structure associated with BSA_SEC_SUSPENDED_EVT */
typedef struct
{
    BD_ADDR bd_addr; /* BD address peer device. */
} tBSA_SEC_SUSPENDED_MSG;

/* Structure associated with BSA_SEC_RESUMED_EVT */
typedef struct
{
    BD_ADDR bd_addr; /* BD address peer device. */
} tBSA_SEC_RESUMED_MSG;

#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)

/* BLE encryption keys */
#define tBSA_LE_PENC_KEYS    tBTM_LE_PENC_KEYS
#define tBSA_LE_PCSRK_KEYS   tBTM_LE_PCSRK_KEYS
#define tBSA_LE_LENC_KEYS    tBTM_LE_LENC_KEYS
#define tBSA_LE_LCSRK_KEYS   tBTM_LE_LCSRK_KEYS
#define tBSA_LE_PID_KEYS     tBTM_LE_PID_KEYS
#define tBSA_LE_KEY_VALUE    tBTM_LE_KEY_VALUE
#define tBSA_LE_KEY_TYPE     tBTM_LE_KEY_TYPE

/* Structure associated with BSA_SEC_MSGID_BLE_KEY_EVT */
typedef struct
{
    BD_ADDR                 bd_addr;        /* peer address */
    tBSA_LE_KEY_TYPE        key_type;
    tBSA_LE_KEY_VALUE       key_value;
}tBSA_SEC_BLE_KEY;


/* Structure associated with BTA_DM_BLE_PASSKEY_REQ_EVT */
typedef struct
{
    BD_ADDR bd_addr; /* BD address peer device. */
} tBSA_SEC_BLE_PASSKEY_REQ;

#endif

typedef INT8 tBSA_SEC_RSSI_VALUE;
typedef INT8 tBSA_SIG_STRENGTH_MASK;
typedef INT8 tBSA_SEC_LINK_QUALITY_VALUE;

/* Structure associated with BTA_DM_SIG_STRENGTH_EVT */
typedef struct
{
    BD_ADDR         bd_addr;            /* BD address peer device. */
    tBSA_SIG_STRENGTH_MASK mask;        /* mask for the values that are valid */
    tBSA_SEC_RSSI_VALUE  rssi_value;
    tBSA_SEC_LINK_QUALITY_VALUE link_quality_value;

} tBSA_SEC_SIG_STRENGTH;

/* Union of all security callback structures */
typedef union
{
    tBSA_SEC_PIN_REQ pin_req; /* Remote device request PIN code */
    tBSA_SEC_AUTH_CMPL auth_cmpl; /* Authentication complete indication */
    tBSA_SEC_BOND_CANCEL_CMPL bond_cancel; /* Cancel complete indication */
    tBSA_SEC_AUTHORIZE authorize; /* Remote device requests Authorization */
    tBSA_SEC_LINK_UP link_up; /* An ACL connection has been established */
    tBSA_SEC_LINK_DOWN link_down; /* An ACL connection has been released*/
    tBSA_SEC_SP_CFM_REQ cfm_req; /* Simple Pairing: user confirm request */
    tBSA_SEC_SP_KEY_NOTIF key_notif; /* Simple Pairing: passkey notification */
    tBSA_SEC_SP_RMT_OOB rmt_oob; /* Simple Pairing: remote oob */
    tBSA_SEC_LOCAL_OOB_MSG  local_oob;  /* local oob information */
    tBSA_SEC_SP_KEY_PRESS key_press; /* Simple Pairing: key press notification event */
    tBSA_SEC_SUSPENDED_MSG suspended; /* Connection is Suspended */
    tBSA_SEC_RESUMED_MSG resumed; /* Connection is Resumed */
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    tBSA_SEC_BLE_KEY ble_key;
    tBSA_SEC_BLE_PASSKEY_REQ ble_passkey_req;
#endif
    tBSA_SEC_SIG_STRENGTH sig_strength;	/* rssi and link quality value */

} tBSA_SEC_MSG;

/* Security Callback */
typedef void ( tBSA_SEC_CBACK)(tBSA_SEC_EVT event, tBSA_SEC_MSG *p_data);

/*
 * Structures use to pass parameters to BSA API functions
 */
typedef struct
{
    tBSA_SEC_IO_CAP simple_pairing_io_cap; /* Simple Pairing IO capability */
    tBSA_SEC_CBACK *sec_cback; /* security callcack */
    BOOLEAN ssp_debug;
} tBSA_SEC_SET_SECURITY;

typedef struct
{
    BD_ADDR bd_addr; /* BdAddr of remote device */
    PIN_CODE pin_code; /* PIN Code */
    UINT8 pin_len; /* PIN Length */
    UINT32 ble_passkey; /* LE passkey, must be 6 digit */
    BOOLEAN is_ble;  /* TRUE if BLE device */
    BOOLEAN ble_accept; /* accept ble device */
} tBSA_SEC_PIN_CODE_REPLY;

typedef struct
{
    BD_ADDR bd_addr; /* BdAddr of remote device */
    UINT32 passkey; /* passkey, must be 6 digit */
} tBSA_SEC_PASSKEY_REPLY;

typedef struct
{
    BD_ADDR bd_addr; /* BdAddr of remote device */
    BOOLEAN accept; /* TRUE to accept SP; otherwise false */
} tBSA_SEC_SP_CFM_REPLY;

typedef struct
{
    BD_ADDR bd_addr; /* BdAddr of remote device */
} tBSA_SEC_BOND;

typedef struct
{
    BD_ADDR bd_addr; /* BdAddr of remote device */
} tBSA_SEC_BOND_CANCEL;

typedef struct
{
    BD_ADDR bd_addr; /* BdAddr of remote device */
    tBSA_SERVICE_ID trusted_service; /* Services trusted/untrusted */
    tBSA_SEC_AUTH_RESP auth; /* auth or no auth */
} tBSA_SEC_AUTH_REPLY;

typedef struct
{
    BD_ADDR bd_addr; /* BdAddr of remote device */
} tBSA_SEC_REMOVE_DEV;

typedef struct
{
    BD_ADDR bd_addr; /* BdAddr of remote device */
    DEV_CLASS class_of_device;
    LINK_KEY link_key;
    BOOLEAN link_key_present;
    tBSA_SERVICE_MASK trusted_services;
    BOOLEAN is_trusted;
    UINT8 key_type;
    tBSA_SEC_IO_CAP io_cap;
    BD_NAME bd_name;
    BD_FEATURES features;
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    UINT8 ble_addr_type;
    tBT_DEVICE_TYPE device_type;
    UINT8 inq_result_type;
    BOOLEAN ble_link_key_present;
    tBSA_LE_PENC_KEYS le_penc_key;
    tBSA_LE_PCSRK_KEYS le_pcsrk_key;
    tBSA_LE_LCSRK_KEYS le_lcsrk_key;
    tBSA_LE_PID_KEYS le_pid_key;
    tBSA_LE_LENC_KEYS le_lenc_key;
#endif

} tBSA_SEC_ADD_DEV;

/* Structure to respond to remote OOB data request from stack */
typedef struct
{
    BD_ADDR bd_addr;    /* BdAddr of the peer device */
    BT_OCTET16 p_c;
    BT_OCTET16 p_r;
    BOOLEAN result;
} tBSA_SEC_SET_REMOTE_OOB;

/* Structure to respond to remote OOB data request from stack */
typedef struct
{
    int dummy;
} tBSA_SEC_READ_OOB;

#define BSA_DEV_PLATFORM_UNKNOWN    0 /* unknown platform */
#define BSA_DEV_PLATFORM_IOS        1 /* Apple iOS */
typedef struct
{
    BD_ADDR bd_addr; /* BdAddr of remote device */
    UINT8 platform; /* device platform */
} tBSA_SEC_ADD_SI_DEV;


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function         BSA_SecSetSecurityInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSetSecurityInit(tBSA_SEC_SET_SECURITY *p_set_sec);

/*******************************************************************************
 **
 ** Function         BSA_SecSetSecurity
 **
 ** Description      Set the Simple Pairing IO capability and security callcack
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSetSecurity(tBSA_SEC_SET_SECURITY *p_set_sec);

/*******************************************************************************
 **
 ** Function         BSA_SecPinCodeReplyInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecPinCodeReplyInit(tBSA_SEC_PIN_CODE_REPLY *p_pin_code_reply);

/*******************************************************************************
 **
 ** Function         BSA_SecPinCodeReply
 **
 ** Description      Send back a pin code
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecPinCodeReply(tBSA_SEC_PIN_CODE_REPLY *p_pin_code_reply);

/*******************************************************************************
 **
 ** Function         BSA_SecSpPasskeyReply
 **
 ** Description      Send back a pin code for simple pairing
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSpPasskeyReply(tBSA_SEC_PASSKEY_REPLY *p_passkey_reply);

/*******************************************************************************
 **
 ** Function         BSA_SecSpCfmReplyInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSpCfmReplyInit(tBSA_SEC_SP_CFM_REPLY *p_sp_cfm_reply);

/*******************************************************************************
 **
 ** Function         BSA_SecSpCfmReply
 **
 ** Description      Accept or refuse simple pairing request
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSpCfmReply(tBSA_SEC_SP_CFM_REPLY *p_sp_cfm_reply);

/*******************************************************************************
 **
 ** Function         BSA_SecBondInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecBondInit(tBSA_SEC_BOND *p_sec_bond);

/*******************************************************************************
 **
 ** Function         BSA_SecBond
 **
 ** Description      Send a pin code in order to bond with a remote device
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecBond(tBSA_SEC_BOND *p_sec_bond);

/*******************************************************************************
 **
 ** Function         BSA_SecBondCancelInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecBondCancelInit(tBSA_SEC_BOND_CANCEL *p_sec_bond_cancel);

/*******************************************************************************
 **
 ** Function         BSA_SecBondCancel
 **
 ** Description      Cancel bonding procedure
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecBondCancel(tBSA_SEC_BOND_CANCEL *p_sec_bond_cancel);


/*******************************************************************************
 **
 ** Function         BSA_SecAuthorizeReplyInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAuthorizeReplyInit(tBSA_SEC_AUTH_REPLY *p_sec_auth_reply);

/*******************************************************************************
 **
 ** Function         BSA_SecAuthorizeReply
 **
 ** Description      Reply to an authorization request for a service
 **
 ** Parameters       Pointer to structure containing API parameters
 **                    BSA_NOT_AUTH to refuse,
 **                    BSA_AUTH_TEMP to grant access temporarily,
 **                    BSA_AUTH_PERM to grant permanent access
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAuthorizeReply(tBSA_SEC_AUTH_REPLY *p_sec_auth_reply);

/*******************************************************************************
 **
 ** Function         BSA_SecRemoveDeviceInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecRemoveDeviceInit(tBSA_SEC_REMOVE_DEV *p_sec_del_dev);

/*******************************************************************************
 **
 ** Function         BSA_SecRemoveDevice
 **
 ** Description      Remove a device from the local database
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecRemoveDevice(tBSA_SEC_REMOVE_DEV *p_sec_del_dev);

/*******************************************************************************
 **
 ** Function         BSA_SecAddDeviceInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAddDeviceInit(tBSA_SEC_ADD_DEV *p_sec_add_dev);

/*******************************************************************************
 **
 ** Function         BSA_SecAddDevice
 **
 ** Description      Add a device to the local database
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAddDevice(tBSA_SEC_ADD_DEV *p_sec_add_dev);

/*******************************************************************************
 **
 ** Function         BSA_SecReadOOBInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecReadOOBInit(tBSA_SEC_READ_OOB *p_sec_read_oob);

/*******************************************************************************
 **
 ** Function         BSA_SecReadOOB
 **
 ** Description      Reads OOB information from local controller
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecReadOOB(tBSA_SEC_READ_OOB *p_sec_read_oob);

/*******************************************************************************
 **
 ** Function         BSA_SecSetRemoteOOBInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSetRemoteOOBInit(tBSA_SEC_SET_REMOTE_OOB *p_sec_set_remote_oob);

/*******************************************************************************
 **
 ** Function         BSA_SecSetRemoteOOB
 **
 ** Description      Respond to remote OOB information request from the stack
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSetRemoteOOB(tBSA_SEC_SET_REMOTE_OOB *p_sec_set_remote_oob);

/*******************************************************************************
 **
 ** Function         BSA_SecAddSiDevInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAddSiDevInit(tBSA_SEC_ADD_SI_DEV*p_sec_add_si_dev);

/*******************************************************************************
 **
 ** Function         BSA_SecAddSiDev
 **
 ** Description      Add a special interest device to the local database
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAddSiDev(tBSA_SEC_ADD_SI_DEV*p_sec_add_si_dev);


#ifdef __cplusplus
}
#endif

#endif
