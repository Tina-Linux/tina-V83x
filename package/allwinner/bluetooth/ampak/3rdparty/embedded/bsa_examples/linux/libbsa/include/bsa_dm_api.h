/*****************************************************************************
 **
 **  Name:           bsa_dm_api.h
 **
 **  Description:    This is the public interface file for Device Management part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2015, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_DM_API_H
#define BSA_DM_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"

/* for BTA 3DS */
#include "bta_3ds_api.h"


#define BSA_DM_CONFIG_NONE                 0x0000
#define BSA_DM_CONFIG_VISIBILITY_MASK      0x0001
#define BSA_DM_CONFIG_BDADDR_MASK          0x0002
#define BSA_DM_CONFIG_NAME_MASK            0x0004
#define BSA_DM_CONFIG_DEV_CLASS_MASK       0x0008
#define BSA_DM_CONFIG_CHANNEL_MASK         0x0010
#define BSA_DM_CONFIG_BRCM_MASK            0x0020
#define BSA_DM_CONFIG_PAGE_SCAN_PARAM_MASK 0x0040
#define BSA_DM_CONFIG_INQ_SCAN_PARAM_MASK  0x0080
#define BSA_DM_CONFIG_TX_POWER_MASK        0x0100
#define BSA_DM_CONFIG_EIR_MASK             0x0200
#define BSA_DM_CONFIG_DUAL_STACK_MODE_MASK 0x0400
#define BSA_DM_CONFIG_BLE_BGCONN_TYPE_MASK 0x0800
#define BSA_DM_CONFIG_BLE_SCAN_PARAM_MASK  0x1000
#define BSA_DM_CONFIG_BLE_VISIBILITY_MASK  0x2000
#define BSA_DM_CONFIG_BLE_CONN_PARAM_MASK  0x4000
#define BSA_DM_CONFIG_BLE_ADV_CONFIG_MASK  0x8000
#define BSA_DM_CONFIG_BLE_ADV_PARAM_MASK  0x10000
#define BSA_DM_CONFIG_BLE_PRIVACY_MASK    0x20000
#define BSA_DM_CONFIG_LINK_POLICY_MASK    0x40000
#define BSA_DM_CONFIG_MONITOR_RSSI        0x80000

#define BSA_DM_CONFIG_DEFAULT_MASK         0x001F
typedef UINT32 tBSA_DM_CONFIG_MASK;
typedef UINT32 tBSA_DM_BLE_AD_MASK;

#define BSA_SERVICE_NAME_LEN BTA_SERVICE_NAME_LEN

/* Maximum size of the EIR buffer provided by the application */
#ifndef BSA_DM_EIR_SIZE_MAX
#define BSA_DM_EIR_SIZE_MAX             200
#endif

/* Alternate Frequency Hopping */
#define BSA_DM_LAST_AFH_CHANNEL         79

/* Stack Switch definitions */
#define BSA_DM_DUAL_STACK_MODE_BSA      0       /* BSA in charge of audio (default) */
#define BSA_DM_DUAL_STACK_MODE_MM       1       /* Switch to MultiMedia (Kernel) */
#define BSA_DM_DUAL_STACK_MODE_BTC      2       /* Switch to Bluetooth Controller (aka SBC Offloading) */
typedef UINT8 tBSA_DM_DUAL_STACK_MODE;
/*
 * Broadcom Specific definitions
*/
/* Definitions for 3D (used if BSA_DM_CONFIG_BRCM_MASK is set in config_mask field) */
/* This field is only interpreted if BSA_3DS_INCLUDED or BSA_3DTV_INCLUDED is TRUE */
#define BSA_DM_3DTV_MASTER_MASK         0x0001  /* Master 3D */
#define BSA_DM_3DTV_SLAVE_MASK          0x0002  /* Slave 3D */
#define BSA_DM_RC_ASSO_MASK             0x0004  /* RC Associated */
#define BSA_DM_SHOW_ROOM_MASK           0x0008  /* Show Room */
#define BSA_DM_RC_PAIRABLE_MASK         0x0010  /* RC Pairable */
#define BSA_DM_3D_CAP_MASK              0x0020  /* Controls 3D_Capable bit in EIR and 3DS EIR field */
/* The following three values are only interpreted if BSA_3DS_INCLUDED is TRUE */
#define BSA_DM_3D_FACTORY_TEST_MASK     0x0040  /* Factory Test */
/* Then either BSA_DM_3D_TX_DATA_MASK or BSA_DM_3D_TX_DATA_MASK bit is set then */
/* BSA will use the 3D data located in tx_data_3d field for the 3D Beacon */
#define BSA_DM_3D_TX_DATA_MASK          0x0080  /* 3D Tx data (offset, etc) */
/* If the following bit is used to send 3D Tx without changing anything to other 3D parameters */
/* If this bit is set, BSA will ignore every other bits of brcm_mask */
#define BSA_DM_3D_TX_DATA_ONLY_MASK     0x0100  /* 3D Tx data (offset, etc) */
typedef UINT16 tBSA_DM_MASK;

/* Definitions for Tx Power */
#define BSA_DM_TX_POWER_CLASS_2         0x00    /* BSA and HCI Value */
#define BSA_DM_TX_POWER_CLASS_1_5       0x01    /* BSA and HCI Value */
#define BSA_DM_TX_POWER_UNKNOWN         0xFF

/* 3D Connection Announcement Type (Association Notification or Battery Level Report) */
#define BSA_3DS_ANNOUNC_TYPE_ASSOS_NOTIF_MASK   BTA_3DS_ANNOUNC_TYPE_ASSOS_NOTIF_MSK
#define BSA_3DS_ANNOUNC_TYPE_BAT_LVL_RPT_MASK   BTA_3DS_ANNOUNC_TYPE_BAT_LVL_RPT_MSK
typedef UINT8 tBSA_3DS_ANNOUNC_TYPE;

#ifndef BSA_DM_3D_ADD_DATA_MAX
#define BSA_DM_3D_ADD_DATA_MAX          1       /* Maximum additional data */
#endif

/* BSA DM callback events */
typedef enum
{
    BSA_DM_3D_ANNOUCEMENT_EVT,      /* 3DG Connection Announcement Event */
} tBSA_DM_EVT;

/* Device Type for BSA_DM_3DTV_CONNECT_EVT (Proximity Pairing) event */
#define BSA_DM_3DTV_CONNECT_TYPE_3DSG       0x00    /* 3D Glasses Connect */
#define BSA_DM_3DTV_CONNECT_TYPE_RC         0x01    /* Remote Control Connect */

/* Device SubType for 3DSG Type for BSA_DM_3DTV_CONNECT_EVT (Proximity Pairing) event */
#define BSA_DM_CONNECT_SUB_TYPE_3DSG_MCAST  0x00    /* Multicast Connection */
#define BSA_DM_CONNECT_SUB_TYPE_3DSG_MIP    0x01    /* MIP Connection */

/* Device SubType for RC Type for BSA_DM_3DTV_CONNECT_EVT (Proximity Pairing) event */
#define BSA_DM_CONNECT_SUB_TYPE_RC          0x00    /* Remote Control */
#define BSA_DM_CONNECT_SUB_TYPE_RC_VOICE    0x01    /* Remote Control with Voice support */

#define BSA_DM_BLE_AD_DATA_LEN              31   /*BLE Advertisement data size limit, stack takes 31bytes of data */
#define BSA_DM_BLE_AD_UUID_MAX              6   /*Max number of Service UUID the device can advertise*/

/* ADV data flag bit definition used for BTM_BLE_AD_TYPE_FLAG */
#define BSA_DM_BLE_LIMIT_DISC_FLAG         BTM_BLE_LIMIT_DISC_FLAG
#define BSA_DM_BLE_GEN_DISC_FLAG           BTM_BLE_GEN_DISC_FLAG
#define BSA_DM_BLE_BREDR_NOT_SPT           BTM_BLE_BREDR_NOT_SPT
#define BSA_DM_BLE_NON_LIMIT_DISC_FLAG     BTM_BLE_NON_LIMIT_DISC_FLAG         /* lowest bit unset */
#define BSA_DM_BLE_ADV_FLAG_MASK           BTM_BLE_ADV_FLAG_MASK
#define BSA_DM_BLE_LIMIT_DISC_MASK         BTM_BLE_LIMIT_DISC_MASK

#define BSA_DM_BLE_AD_BIT_DEV_NAME        BTM_BLE_AD_BIT_DEV_NAME
#define BSA_DM_BLE_AD_BIT_FLAGS           BTM_BLE_AD_BIT_FLAGS
#define BSA_DM_BLE_AD_BIT_MANU            BTM_BLE_AD_BIT_MANU
#define BSA_DM_BLE_AD_BIT_TX_PWR          BTM_BLE_AD_BIT_TX_PWR
#define BSA_DM_BLE_AD_BIT_INT_RANGE       BTM_BLE_AD_BIT_INT_RANGE
#define BSA_DM_BLE_AD_BIT_SERVICE         BTM_BLE_AD_BIT_SERVICE
#define BSA_DM_BLE_AD_BIT_SERVICE_SOL     BTM_BLE_AD_BIT_SERVICE_SOL
#define BSA_DM_BLE_AD_BIT_SERVICE_DATA    BTM_BLE_AD_BIT_SERVICE_DATA
#define BSA_DM_BLE_AD_BIT_SIGN_DATA       BTM_BLE_AD_BIT_SIGN_DATA
#define BSA_DM_BLE_AD_BIT_SERVICE_128     BTM_BLE_AD_BIT_SERVICE_128
#define BSA_DM_BLE_AD_BIT_SERVICE_128SOL  BTM_BLE_AD_BIT_SERVICE_128SOL
#define BSA_DM_BLE_AD_BIT_APPEARANCE      BTM_BLE_AD_BIT_APPEARANCE
#define BSA_DM_BLE_AD_BIT_PUBLIC_ADDR     BTM_BLE_AD_BIT_PUBLIC_ADDR
#define BSA_DM_BLE_AD_BIT_RANDOM_ADDR     BTM_BLE_AD_BIT_RANDOM_ADDR

#define BSA_DM_BLE_AD_BIT_PROPRIETARY     BTM_BLE_AD_BIT_PROPRIETARY

/* 3D Tx data */
typedef struct
{
    UINT16 left_open_offset;
    UINT16 left_close_offset;
    UINT16 right_open_offset;
    UINT16 right_close_offset;
    UINT16 delay;
    UINT8 dual_view;
} tBSA_DM_3D_TX_DATA;

/* Callback data for BSA_DM_3D_ANNOUCEMENT_EVT event */
typedef struct
{
    BD_ADDR bd_addr;    /* BdAddr of the peer device */
    tBSA_3DS_ANNOUNC_TYPE type; /* Announcement type */
    UINT8 battery_level; /* Battery Level */
    UINT16 additional_data_len;
    UINT8 additional_data[BSA_DM_3D_ADD_DATA_MAX];
} tBSA_DM_3D_ANNOUCEMENT_MSG;

/* union of data associated with tBSA_DM_EVT */
typedef union
{
    tBSA_DM_3D_ANNOUCEMENT_MSG announcement; /* BSA_DM_3D_ANNOUCEMENT_EVT*/
} tBSA_DM_MSG;

/* BSA DM callback function */
typedef void (tBSA_DM_CBACK)(tBSA_DM_EVT event, tBSA_DM_MSG *p_data);

/* Definitions of Chip Id returned by the FW */
#define BSA_DM_CHIP_ID_UNKNOWN       0x00    /* This Chip Id does not exist */
#define BSA_DM_CHIP_ID_2046B1        0x0C    /* 2046B1 */
#define BSA_DM_CHIP_ID_20702A1       0x3F    /* 20702A1 */
#define BSA_DM_CHIP_ID_20702B0       0x49    /* 20702B0 */
#define BSA_DM_CHIP_ID_43242A1       0x55    /* 43242A1 */
#define BSA_DM_CHIP_ID_43569A2       0x6C    /* 43569A2 */
typedef UINT8 tBSA_DM_CHIP_ID;

/* BLE Preferred connection parameter */
typedef struct
{
    BD_ADDR bd_addr;            /* BdAddr of the peer device */
    UINT16 min_conn_int;        /* Preferred minimum connection interval */
    UINT16 max_conn_int;        /* Preferred maximum connection interval */
    UINT16 slave_latency;       /* Preferred slave latency */
    UINT16 supervision_tout;    /* Preferred supervision timeout value */
    BOOLEAN is_immediate_updating;  /*If TRUE,send udpate conn param request(slave) or update param to controller(master).*/
                                    /*Otherwise, set preferred value for master*/
} tBSA_DM_BLE_CONN_PARAM;

typedef struct
{
    UINT16  low;
    UINT16  hi;
}tBSA_DM_BLE_INT_RANGE;

typedef struct
{
    UINT8       adv_type;
    UINT8       len;        /* number of len byte*/
    UINT8       *p_val;
}tBSA_DM_BLE_PROP_ELEM;

/* vendor proprietary adv type */
typedef struct
{
    UINT8                   num_elem;
    tBTA_BLE_PROP_ELEM      *p_elem;
}tBSA_DM_BLE_PROPRIETARY;

typedef struct
{
   BOOLEAN list_cmpl;
   UINT8 uuid128[MAX_UUID_SIZE];
}tBSA_DM_BLE_128SERVICE;

/* BLE Advertisement configuration parameters */
typedef struct
{
    UINT8                     len; /* Number of bytes of data to be advertised */
    UINT8                     flag; /* AD flag value to be set */
    UINT8                     p_val[BSA_DM_BLE_AD_DATA_LEN];/* Data to be advertised */
    tBSA_DM_BLE_AD_MASK       adv_data_mask; /* Data Mask: Eg: BTM_BLE_AD_BIT_FLAGS, BTM_BLE_AD_BIT_MANU */
    UINT16                    appearance_data;     /* Device appearance data */
    UINT8                     num_service; /* number of services */
    UINT16                    uuid_val[BSA_DM_BLE_AD_UUID_MAX];
    /* for DataType Service Data - 0x16 */
    UINT8                     service_data_len; /* length = AD type + service data uuid + data) */
    tBT_UUID                  service_data_uuid; /* service data uuid */
    UINT8                     service_data_val[BSA_DM_BLE_AD_DATA_LEN];
    BOOLEAN                   is_scan_rsp;  /* is the data scan response or adv data */
    UINT8                     tx_power;
    tBSA_DM_BLE_INT_RANGE     int_range;
    tBSA_DM_BLE_128SERVICE    services_128b;
    tBSA_DM_BLE_128SERVICE    sol_service_128b;
    tBSA_DM_BLE_PROPRIETARY   elem;
}tBSA_DM_BLE_ADV_CONFIG;

/* BLE Packet Content Filter parameters */
#define BSA_BLE_PF_COND_MAX_LEN 20
typedef struct
{
    tBLE_BD_ADDR                target_addr;   /* target address, if NULL, generic UUID filter */
    tBT_UUID                    uuid;          /* UUID condition */
    tBTA_DM_BLE_PF_LOGIC_TYPE   cond_logic;    /* AND/OR */
}tBSA_DM_BLE_PF_UUID_COND;

typedef struct
{
    UINT8                   data_len;       /* <= 20 bytes */
    UINT8                   data[BSA_BLE_PF_COND_MAX_LEN];
}tBSA_DM_BLE_PF_LOCAL_NAME_COND;

typedef struct
{
    UINT16                  company_id;      /* company ID */
    UINT8                   data_len;        /* <= 20 bytes */
    UINT8                   pattern[BSA_BLE_PF_COND_MAX_LEN];
    UINT16                  company_id_mask; /* UUID value mask */
    UINT8                   pattern_mask[BSA_BLE_PF_COND_MAX_LEN];
}tBSA_DM_BLE_PF_MANU_COND;

typedef union
{
    tBLE_BD_ADDR                               target_addr;
    tBSA_DM_BLE_PF_LOCAL_NAME_COND             local_name; /* lcoal name filtering */
    tBSA_DM_BLE_PF_MANU_COND                   manu_data;  /* manufactuer data filtering */
    tBSA_DM_BLE_PF_UUID_COND                   srvc_uuid;  /* service UUID filtering */
    tBSA_DM_BLE_PF_UUID_COND                   solicitate_uuid;   /* solicitated service UUID filtering */
}tBSA_DM_BLE_PF_COND_PARAM;

/*****************************************************************************
**                          Low Energy definitions
**
** Address types
*/
#define BSA_DM_BLE_ADDR_PUBLIC         0x00
#define BSA_DM_BLE_ADDR_RANDOM         0x01
#define BSA_DM_BLE_ADDR_TYPE_MASK      (BSA_DM_BLE_ADDR_RANDOM | BSA_DM_BLE_ADDR_PUBLIC)
typedef UINT8 tBSA_DM_LE_ADDR_TYPE;

typedef struct
{
    tBSA_DM_LE_ADDR_TYPE   type;
    BD_ADDR             bd_addr;
} tBSA_DM_BLE_BD_ADDR;


/* set adv parameter for BLE advertising */
typedef struct
{
    UINT16 adv_int_min;             /* Minimum Advertisement interval */
    UINT16 adv_int_max;             /* Maximum Advertisement interval */
    tBSA_DM_BLE_BD_ADDR dir_bda;       /* Directed Adv BLE addr type and addr */

}tBSA_DM_BLE_ADV_PARAM;

/* link policy masks  */
#define BSA_DM_LP_SWITCH    BTA_DM_LP_SWITCH
#define BSA_DM_LP_HOLD      BTA_DM_LP_HOLD
#define BSA_DM_LP_SNIFF     BTA_DM_LP_SNIFF
#define BSA_DM_LP_PARK      BTA_DM_LP_PARK
#define tBSA_DM_LP_MASK     tBTA_DM_LP_MASK

/* set link policy */
typedef struct
{
    BD_ADDR link_bd_addr; /* link BdAddr */
    tBSA_DM_LP_MASK policy_mask; /* link policy mask*/
    BOOLEAN set; /* set or clear policy */
}tBSA_DM_LINK_POLICY_PARAM;

/* enable rssi monitoring */
typedef struct
{
    BOOLEAN enable; /* Monitor RSSI enable */
    UINT16  period; /* Period in seconds */

}tBSA_DM_MONITOR_RSSI_PARAM;

/* tBSA_DM_GET_CONFIG structure */
typedef struct
{
    tBSA_DM_CONFIG_MASK config_mask; /* Indicates which fields to configure (only relevant when set) */
    BOOLEAN enable; /* Is Bluetooth Enable.disabled */
    BOOLEAN discoverable; /* Is bluetooth device discoverable (InquiryScan) */
    BOOLEAN connectable; /* Is bluetooth device Connectable (PageScan) */
    BD_ADDR bd_addr; /* Local BdAddr */
    BD_NAME name; /* Local Name */
    DEV_CLASS class_of_device;/* Class Of Device */
    UINT8 first_disabled_channel; /* First host disabled channel */
    UINT8 last_disabled_channel; /* Last host disabled channel */
    BD_ADDR master_3d_bd_addr;
    UINT8 path_loss_threshold;
    UINT16 page_scan_interval;
    UINT16 page_scan_window;
    UINT16 inquiry_scan_interval;
    UINT16 inquiry_scan_window;
    tBSA_DM_CBACK *callback;
    UINT8 tx_power; /* Tx Power (Point to Point) */
    tBSA_DM_MASK brcm_mask; /* Broadcom Specific Mask */
    UINT8 eir_length; /* Length of the EIR field provided by the application */
    UINT8 eir_data[BSA_DM_EIR_SIZE_MAX]; /* EIR field provided by the application */
    tBSA_DM_DUAL_STACK_MODE dual_stack_mode;
    tBSA_DM_3D_TX_DATA tx_data_3d;  /* 3D Tx Data */
    UINT8 ble_bg_conn_type;   /* BLE Background connection type, 0 = None, 1 = auto connection */
    UINT16 ble_scan_interval;
    UINT16 ble_scan_window;
    BOOLEAN ble_discoverable; /* Is bluetooth device ble discoverable */
    BOOLEAN ble_connectable; /* Is bluetooth device ble Connectable */
    BOOLEAN privacy_enable; /* BLE Local privacy  */
    tBSA_DM_BLE_CONN_PARAM ble_conn_param;
    tBSA_DM_BLE_ADV_CONFIG adv_config;
    tBSA_DM_BLE_ADV_PARAM ble_adv_param;
    tBSA_DM_CHIP_ID chip_id; /* bluetooth module chip id */
    tBSA_DM_LINK_POLICY_PARAM policy_param; /* link policy param*/
    tBSA_DM_MONITOR_RSSI_PARAM monitor_rssi_param; /* Monitor RSSI */
} tBSA_DM_GET_CONFIG;

typedef tBSA_DM_GET_CONFIG tBSA_DM_SET_CONFIG;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function         BSA_DmGetConfigInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DmGetConfigInit(tBSA_DM_GET_CONFIG *p_config);

/*******************************************************************************
 **
 ** Function         BSA_DmGetConfig
 **
 ** Description      This function retrieve the current configuration of the
 **                  local Bluetooth device
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DmGetConfig(tBSA_DM_GET_CONFIG *p_config);

/*******************************************************************************
 **
 ** Function         BSA_DmSetConfigInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DmSetConfigInit(tBSA_DM_SET_CONFIG *p_config);

/*******************************************************************************
 **
 ** Function         BSA_DmSetConfig
 **
 ** Description      This function change the current configuration of the
 **                  local Bluetooth device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DmSetConfig(tBSA_DM_SET_CONFIG *p_config);

#ifdef __cplusplus
}
#endif

#endif
