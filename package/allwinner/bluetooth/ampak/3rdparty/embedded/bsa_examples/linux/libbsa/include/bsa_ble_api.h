/*****************************************************************************
 **
 **  Name:          bsa_ble_api.h
 **
 **  Description:   This is the public interface file for BLE part of
 **                 the Bluetooth simplified API
 **
 **  Copyright (c) 2015, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_BLE_API_H
#define BSA_BLE_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"
#include "bta_gatt_api.h"
#include "bsa_dm_api.h"
#include "bta_gattc_ci.h"

/*****************************************************************************
 **  Constants and Type Definitions
 *****************************************************************************/
#ifndef BSA_BLE_DEBUG
#define BSA_BLE_DEBUG FALSE
#endif

typedef tGATT_IF                   tBSA_BLE_IF;
typedef tBTA_GATTC_CHAR_ID         tBSA_BLE_CL_CHAR_ID;
typedef tBTA_GATTC_CHAR_DESCR_ID   tBSA_BLE_CL_CHAR_DESCR_ID;
typedef tBTA_GATT_ID               tBSA_BLE_ID;
typedef tBTA_GATT_READ_VAL         tBSA_BLE_READ_VAL;
typedef tBTA_GATT_REASON           tBSA_BLE_REASON;
typedef tBTA_GATT_PERM             tBSA_BLE_PERM;
typedef tBTA_GATT_CHAR_PROP        tBSA_BLE_CHAR_PROP;
typedef tBTA_GATT_TRANSPORT        tBSA_BLE_TRANSPORT;
typedef tBTA_GATTC_WRITE_TYPE      tBSA_BLE_WRITE_TYPE;

/* Attribute permissions
*/
#define BSA_GATT_PERM_READ              BTA_GATT_PERM_READ              /* bit 0 -  0x0001 */
#define BSA_GATT_PERM_READ_ENCRYPTED    BTA_GATT_PERM_READ_ENCRYPTED    /* bit 1 -  0x0002 */
#define BSA_GATT_PERM_READ_ENC_MITM     BTA_GATT_PERM_READ_ENC_MITM     /* bit 2 -  0x0004 */
#define BSA_GATT_PERM_WRITE             BTA_GATT_PERM_WRITE             /* bit 4 -  0x0010 */
#define BSA_GATT_PERM_WRITE_ENCRYPTED   BTA_GATT_PERM_WRITE_ENCRYPTED   /* bit 5 -  0x0020 */
#define BSA_GATT_PERM_WRITE_ENC_MITM    BTA_GATT_PERM_WRITE_ENC_MITM    /* bit 6 -  0x0040 */
#define BSA_GATT_PERM_WRITE_SIGNED      BTA_GATT_PERM_WRITE_SIGNED      /* bit 7 -  0x0080 */
#define BSA_GATT_PERM_WRITE_SIGNED_MITM BTA_GATT_PERM_WRITE_SIGNED_MITM /* bit 8 -  0x0100 */

/* definition of characteristic properties */
#define BSA_GATT_CHAR_PROP_BIT_BROADCAST    BTA_GATT_CHAR_PROP_BIT_BROADCAST   /* 0x01 */
#define BSA_GATT_CHAR_PROP_BIT_READ         BTA_GATT_CHAR_PROP_BIT_READ        /* 0x02 */
#define BSA_GATT_CHAR_PROP_BIT_WRITE_NR     BTA_GATT_CHAR_PROP_BIT_WRITE_NR    /* 0x04 */
#define BSA_GATT_CHAR_PROP_BIT_WRITE        BTA_GATT_CHAR_PROP_BIT_WRITE       /* 0x08 */
#define BSA_GATT_CHAR_PROP_BIT_NOTIFY       BTA_GATT_CHAR_PROP_BIT_NOTIFY      /* 0x10 */
#define BSA_GATT_CHAR_PROP_BIT_INDICATE     BTA_GATT_CHAR_PROP_BIT_INDICATE    /* 0x20 */
#define BSA_GATT_CHAR_PROP_BIT_AUTH         BTA_GATT_CHAR_PROP_BIT_AUTH        /* 0x40 */
#define BSA_GATT_CHAR_PROP_BIT_EXT_PROP     BTA_GATT_CHAR_PROP_BIT_EXT_PROP    /* 0x80 */

#define BSA_BLE_MAX_ATTR_LEN  GATT_MAX_ATTR_LEN


#define BSA_GATTC_ATTR_TYPE_INCL_SRVC    BTA_GATTC_ATTR_TYPE_INCL_SRVC
#define BSA_GATTC_ATTR_TYPE_CHAR         BTA_GATTC_ATTR_TYPE_CHAR
#define BSA_GATTC_ATTR_TYPE_CHAR_DESCR   BTA_GATTC_ATTR_TYPE_CHAR_DESCR
#define BSA_GATTC_ATTR_TYPE_SRVC         BTA_GATTC_ATTR_TYPE_SRVC
typedef tBTA_GATTC_ATTR_TYPE             tBSA_GATTC_ATTR_TYPE;

/* max client application BSA BLE Client can support */
#ifndef BSA_BLE_CLIENT_MAX
#define BSA_BLE_CLIENT_MAX    3
#endif

/* max server application BSA BLE Server can support */
#define BSA_BLE_SERVER_MAX    4
#define BSA_BLE_ATTRIBUTE_MAX 50

#ifndef BSA_BLE_SERVER_SECURITY
#define BSA_BLE_SERVER_SECURITY BTA_DM_BLE_SEC_NONE
#endif

#define BSA_BLE_INVALID_IF         0xff
#define BSA_BLE_INVALID_CONN       0xffff


#define BSA_BLE_UUID_SERVCLASS_GAP_SERVER                    UUID_SERVCLASS_GAP_SERVER
#define BSA_BLE_UUID_SERVCLASS_GATT_SERVER                   UUID_SERVCLASS_GATT_SERVER
#define BSA_BLE_UUID_SERVCLASS_IMMEDIATE_ALERT               UUID_SERVCLASS_IMMEDIATE_ALERT
#define BSA_BLE_UUID_SERVCLASS_LINKLOSS                      UUID_SERVCLASS_LINKLOSS
#define BSA_BLE_UUID_SERVCLASS_TX_POWER                      UUID_SERVCLASS_TX_POWER
#define BSA_BLE_UUID_SERVCLASS_CURRENT_TIME                  UUID_SERVCLASS_CURRENT_TIME
#define BSA_BLE_UUID_SERVCLASS_DST_CHG                       UUID_SERVCLASS_DST_CHG
#define BSA_BLE_UUID_SERVCLASS_REF_TIME_UPD                  UUID_SERVCLASS_REF_TIME_UPD
#define BSA_BLE_UUID_SERVCLASS_GLUCOSE                       UUID_SERVCLASS_GLUCOSE
#define BSA_BLE_UUID_SERVCLASS_HEALTH_THERMOMETER            UUID_SERVCLASS_THERMOMETER
#define BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION            UUID_SERVCLASS_DEVICE_INFO
#define BSA_BLE_UUID_SERVCLASS_NWA                           UUID_SERVCLASS_NWA
#define BSA_BLE_UUID_SERVCLASS_PHALERT                       UUID_SERVCLASS_PHALERT
#define BSA_BLE_UUID_SERVCLASS_HEART_RATE                    UUID_SERVCLASS_HEART_RATE
#define BSA_BLE_UUID_SERVCLASS_BATTERY_SERVICE               UUID_SERVCLASS_BATTERY
#define BSA_BLE_UUID_SERVCLASS_BLOOD_PRESSURE                UUID_SERVCLASS_BPM
#define BSA_BLE_UUID_SERVCLASS_ALERT_NOTIFICATION_SERVICE    UUID_SERVCLASS_ALERT_NOTIFICATION
#define BSA_BLE_UUID_SERVCLASS_HUMAN_INTERFACE_DEVICE        UUID_SERVCLASS_LE_HID
#define BSA_BLE_UUID_SERVCLASS_SCAN_PARAMETERS               UUID_SERVCLASS_SCAN_PARAM
#define BSA_BLE_UUID_SERVCLASS_RUNNING_SPEED_AND_CADENCE     UUID_SERVCLASS_RSC
#define BSA_BLE_UUID_SERVCLASS_CYCLING_SPEED_AND_CADENCE     UUID_SERVCLASS_CSC
#define BSA_BLE_UUID_SERVCLASS_TEST_SERVER                   UUID_SERVCLASS_TEST_SERVER

#define BSA_BLE_GATT_UUID_PRI_SERVICE                       GATT_UUID_PRI_SERVICE
#define BSA_BLE_GATT_UUID_SEC_SERVICE                       GATT_UUID_SEC_SERVICE
#define BSA_BLE_GATT_UUID_INCLUDE_SERVICE                   GATT_UUID_INCLUDE_SERVICE
#define BSA_BLE_GATT_UUID_CHAR_DECLARE                      GATT_UUID_CHAR_DECLARE            /*  Characteristic Declaration*/
#define BSA_BLE_GATT_UUID_CHAR_EXT_PROP                     GATT_UUID_CHAR_EXT_PROP           /*  Characteristic Extended Properties */
#define BSA_BLE_GATT_UUID_CHAR_DESCRIPTION                  GATT_UUID_CHAR_DESCRIPTION        /*  Characteristic User Description*/
#define BSA_BLE_GATT_UUID_CHAR_CLIENT_CONFIG                GATT_UUID_CHAR_CLIENT_CONFIG      /*  Client Characteristic Configuration */
#define BSA_BLE_GATT_UUID_CHAR_VALID_RANGE                  GATT_UUID_CHAR_VALID_RANGE        /*  Characteristic Valid Range */

#define BSA_BLE_GATT_UUID_CHAR_CLIENT_CONFIG_ENABLE_NOTI    0x01    /* Enable Notification of Client Characteristic Configuration, defined at bluetooth org */
#define BSA_BLE_GATT_UUID_CHAR_CLIENT_CONFIG_ENABLE_INDI    0x02    /* Enable Indication of Client Characteristic Configuration, defined at bluetooth org */

#define BSA_BLE_GATT_UUID_CHAR_SRVR_CONFIG                  GATT_UUID_CHAR_SRVR_CONFIG        /*  Server Characteristic Configuration */
#define BSA_BLE_GATT_UUID_CHAR_PRESENT_FORMAT               GATT_UUID_CHAR_PRESENT_FORMAT     /*  Characteristic Presentation Format*/
#define BSA_BLE_GATT_UUID_CHAR_AGG_FORMAT                   GATT_UUID_CHAR_AGG_FORMAT         /*  Characteristic Aggregate Format*/
#define BSA_BLE_GATT_UUID_CHAR_VALID_RANGE                  GATT_UUID_CHAR_VALID_RANGE        /*  Characteristic Valid Range */

#define BSA_BLE_GATT_UUID_GAP_DEVICE_NAME                   GATT_UUID_GAP_DEVICE_NAME
#define BSA_BLE_GATT_UUID_GAP_ICON                          GATT_UUID_GAP_ICON
#define BSA_BLE_GATT_UUID_GAP_PRIVACY_FLAG                  GATT_UUID_GAP_PRIVACY_FLAG
#define BSA_BLE_GATT_UUID_GAP_RECONN_ADDR                   GATT_UUID_GAP_RECONN_ADDR
#define BSA_BLE_GATT_UUID_GAP_PREF_CONN_PARAM               GATT_UUID_GAP_PREF_CONN_PARAM

#define BSA_BLE_GATT_UUID_SENSOR_LOCATION                   GATT_UUID_SENSOR_LOCATION

/* Battery Service */
#define BSA_BLE_GATT_UUID_BATTERY_LEVEL                     GATT_UUID_BATTERY_LEVEL

/* device infor characteristic */
#define BSA_BLE_GATT_UUID_SYSTEM_ID                         GATT_UUID_SYSTEM_ID
#define BSA_BLE_GATT_UUID_MODEL_NUMBER_STR                  GATT_UUID_MODEL_NUMBER_STR
#define BSA_BLE_GATT_UUID_SERIAL_NUMBER_STR                 GATT_UUID_SERIAL_NUMBER_STR
#define BSA_BLE_GATT_UUID_FW_VERSION_STR                    GATT_UUID_FW_VERSION_STR
#define BSA_BLE_GATT_UUID_HW_VERSION_STR                    GATT_UUID_HW_VERSION_STR
#define BSA_BLE_GATT_UUID_SW_VERSION_STR                    GATT_UUID_SW_VERSION_STR
#define BSA_BLE_GATT_UUID_MANU_NAME                         GATT_UUID_MANU_NAME
#define BSA_BLE_GATT_UUID_IEEE_DATA                         GATT_UUID_IEEE_DATA
#define BSA_BLE_GATT_UUID_PNP_ID                            GATT_UUID_PNP_ID

/* Link Loss Service */
#define BSA_BLE_GATT_UUID_ALERT_LEVEL                       GATT_UUID_ALERT_LEVEL      /* Alert Level */
#define BSA_BLE_GATT_UUID_TX_POWER_LEVEL                    GATT_UUID_TX_POWER_LEVEL      /* TX power level */

/* Heart Rate Service */
#define BSA_BLE_GATT_UUID_HEART_RATE_MEASUREMENT            0x2A37
#define BSA_BLE_GATT_UUID_BODY_SENSOR_LOCATION              0x2A38
#define BSA_BLE_GATT_UUID_BODY_SENSOR_CONTROL_POINT         0x2A39

/* BLOOD PRESSURE SERVICE */
#define BSA_BLE_GATT_UUID_BLOOD_PRESSURE_FEATURE            0x2A49
#define BSA_BLE_GATT_UUID_BLOOD_PRESSURE_MEASUREMENT        0x2A35
#define BSA_BLE_GATT_UUID_INTERMEDIATE_CUFF_PRESSURE        0x2A36
#define BSA_BLE_GATT_LENGTH_OF_BLOOD_PRESSURE_MEASUREMENT   13

/*HEALTH THERMOMETER SERVICE*/
#define BSA_BLE_GATT_UUID_TEMPERATURE_TYPE                  0x2A1D
#define BSA_BLE_GATT_UUID_TEMPERATURE_MEASUREMENT           0X2A1C
#define BSA_BLE_GATT_UUID_INTERMEDIATE_TEMPERATURE          0x2A1E
#define BSA_BLE_GATT_UUID_TEMPERATURE_MEASUREMENT_INTERVAL  0x2A21

/* CYCLING SPEED AND CADENCE SERVICE      */
#define BSA_BLE_GATT_UUID_CSC_MEASUREMENT                   GATT_UUID_CSC_MEASUREMENT
#define BSA_BLE_GATT_UUID_CSC_FEATURE                       GATT_UUID_CSC_FEATURE
#define BSA_BLE_GATT_LENGTH_OF_CSC_MEASUREMENT              11

/* RUNNERS SPEED AND CADENCE SERVICE      */
#define BSA_BLE_GATT_UUID_RSC_MEASUREMENT                   GATT_UUID_RSC_MEASUREMENT
#define BSA_BLE_GATT_UUID_RSC_FEATURE                       GATT_UUID_RSC_FEATURE
#define BSA_BLE_GATT_LENGTH_OF_RSC_MEASUREMENT              10

#define BSA_BLE_GATT_UUID_SC_CONTROL_POINT                  GATT_UUID_SC_CONTROL_POINT

/* HID characteristics */
#define BSA_BLE_GATT_UUID_HID_INFORMATION                   GATT_UUID_HID_INFORMATION
#define BSA_BLE_GATT_UUID_HID_REPORT_MAP                    GATT_UUID_HID_REPORT_MAP
#define BSA_BLE_GATT_UUID_HID_CONTROL_POINT                 GATT_UUID_HID_CONTROL_POINT
#define BSA_BLE_GATT_UUID_HID_REPORT                        GATT_UUID_HID_REPORT
#define BSA_BLE_GATT_UUID_HID_PROTO_MODE                    GATT_UUID_HID_PROTO_MODE
#define BSA_BLE_GATT_UUID_HID_BT_KB_INPUT                   GATT_UUID_HID_BT_KB_INPUT
#define BSA_BLE_GATT_UUID_HID_BT_KB_OUTPUT                  GATT_UUID_HID_BT_KB_OUTPUT
#define BSA_BLE_GATT_UUID_HID_BT_MOUSE_INPUT                GATT_UUID_HID_BT_MOUSE_INPUT


#define BSA_BLE_GATT_TRANSPORT_LE          GATT_TRANSPORT_LE
#define BSA_BLE_GATT_TRANSPORT_BR_EDR      GATT_TRANSPORT_BR_EDR
#define BSA_BLE_GATT_TRANSPORT_LE_BR_EDR   GATT_TRANSPORT_LE_BR_EDR


typedef enum
{
    /* BLE Client events */
    BSA_BLE_CL_DEREGISTER_EVT,    /* BLE client is registered. */
    BSA_BLE_CL_OPEN_EVT,          /* BLE open request status  event */
    BSA_BLE_CL_READ_EVT,          /* BLE read characteristic/descriptor event */
    BSA_BLE_CL_WRITE_EVT,         /* BLE write characteristic/descriptor event */
    BSA_BLE_CL_CLOSE_EVT,         /* GATTC  close request status event */
    BSA_BLE_CL_SEARCH_CMPL_EVT,   /* GATT discovery complete event */
    BSA_BLE_CL_SEARCH_RES_EVT,    /* GATT discovery result event */
    BSA_BLE_CL_NOTIF_EVT,         /* GATT attribute notification event */
    BSA_BLE_CL_CONGEST_EVT,       /* GATT congestion/uncongestion event */
    BSA_BLE_CL_CACHE_SAVE_EVT,
    BSA_BLE_CL_CACHE_LOAD_EVT,

    /* BLE Server events */
    BSA_BLE_SE_DEREGISTER_EVT,    /* BLE Server is deregistered */
    BSA_BLE_SE_CREATE_EVT,        /* Service is created */
    BSA_BLE_SE_ADDCHAR_EVT,       /* char data is added */
    BSA_BLE_SE_START_EVT,         /* Service is started */
    BSA_BLE_SE_WRITE_EVT,         /* Write request from client */
    BSA_BLE_SE_READ_EVT,          /* Read request from client */
    BSA_BLE_SE_EXEC_WRITE_EVT,    /* Exec write request from client */
    BSA_BLE_SE_OPEN_EVT,          /* Connect request from client */
    BSA_BLE_SE_CLOSE_EVT,         /* Disconnect request from client */
    BSA_BLE_SE_CONF_EVT,
    BSA_BLE_SE_CONGEST_EVT,
    BSA_BLE_SE_MTU_EVT,
} tBSA_BLE_EVT;

/* BSA BLE Client Host callback events */
/* Client callback function events */

/* callback event data for BSA_BLE_CL_OPEN_EVT event */
typedef struct
{
    tBSA_STATUS     status; /* operation status */
    UINT16          conn_id;
    tBSA_BLE_IF     client_if; /* Client interface ID */
    BD_ADDR         bd_addr;
} tBSA_BLE_CL_OPEN_MSG;

/* callback event data for BSA_BLE_CL_CLOSE_EVT event */
typedef struct
{
    tBSA_STATUS         status;
    UINT16              conn_id;
    tBSA_BLE_IF         client_if;
    BD_ADDR             remote_bda;
    tBSA_BLE_REASON     reason;         /* disconnect reason code, not useful when connect event is reported */
} tBSA_BLE_CL_CLOSE_MSG;

/* callback event data for BSA_BLE_CL_DEREGISTER_EVT event */
typedef struct
{
    tBSA_STATUS    status; /* operation status */
    tBSA_BLE_IF    client_if; /* Client interface ID */
} tBSA_BLE_CL_DEREGISTER_MSG;

/* callback event data for BSA_BLE_CL_SEARCH_RES_EVT event */
typedef struct
{
    UINT16       conn_id;
    tBTA_GATT_SRVC_ID  service_uuid;
} tBSA_BLE_CL_SEARCH_RES_MSG;

/* callback event data for BSA_BLE_CL_SEARCH_CMPL_EVT event */
typedef struct
{
    tBSA_STATUS    status; /* operation status */
    UINT16         conn_id;
} tBSA_BLE_CL_SEARCH_CMPL_MSG;

#define MAX_READ_LEN 100
/* callback event data for BSA_BLE_CL_READ_EVT event */
typedef struct
{
    tBSA_STATUS         status;
    UINT16              conn_id;
    tBTA_GATT_SRVC_ID         srvc_id;
    tBSA_BLE_ID         char_id;
    tBTA_GATT_ID            descr_type;
    UINT16              len;
    UINT8               value[MAX_READ_LEN];
} tBSA_BLE_CL_READ_MSG;

/* callback event data for BSA_BLE_CL_WRITE_EVT event */
typedef struct
{
    UINT16              conn_id;
    tBSA_STATUS         status;
    tBTA_GATT_SRVC_ID         srvc_id;
    tBSA_BLE_ID         char_id;
    tBTA_GATT_ID            descr_type;
} tBSA_BLE_CL_WRITE_MSG;

/* callback event data for BSA_BLE_CL_NOTIF_EVT event */
typedef struct
{
    UINT16              conn_id;
    BD_ADDR             bda;
    tBTA_GATTC_CHAR_ID  char_id;
    tBTA_GATT_ID            descr_type;
    UINT16              len;
    UINT8               value[BSA_BLE_MAX_ATTR_LEN];
    BOOLEAN             is_notify;
} tBSA_BLE_CL_NOTIF_MSG;

typedef struct
{
    UINT16              conn_id;
    BOOLEAN             congested; /* congestion indicator */
}tBSA_BLE_CL_CONGEST_MSG;

#define BSA_BLE_CL_NV_LOAD_MAX   BTA_GATTC_NV_LOAD_MAX
/* callback event data for BSA_BLE_CL_CACHE_SAVE_EVT event */
/* attributes in one service */
typedef struct
{
    UINT16               evt;
    UINT16               num_attr;
    UINT16               attr_index;
    UINT16               conn_id;
    BD_ADDR              bd_addr;
    tBTA_GATTC_NV_ATTR   attr[BSA_BLE_CL_NV_LOAD_MAX];
} tBSA_BLE_CL_CACHE_SAVE_MSG;

/* callback event data for BSA_BLE_CL_CACHE_LOAD_EVT event */
typedef struct
{
    UINT16         conn_id;
    tBSA_STATUS    status;
    BD_ADDR        bd_addr;
} tBSA_BLE_CL_CACHE_LOAD_MSG;


/* BSA BLE Server Host callback events */
/* Server callback function events */

/* callback event data for BSA_BLE_SE_DEREGISTER_EVT event */
typedef struct
{
    tBSA_STATUS    status; /* operation status */
    tBSA_BLE_IF    server_if; /* Server interface ID */
} tBSA_BLE_SE_DEREGISTER_MSG;

/* callback event data for BSA_BLE_SE_CREATE_EVT event */
typedef struct
{
    tBSA_BLE_IF    server_if;
    UINT16         service_id;
    tBSA_STATUS    status;
} tBSA_BLE_SE_CREATE_MSG;

/* callback event data for BSA_BLE_SE_ADDCHAR_EVT event */
typedef struct
{
    tBSA_BLE_IF    server_if;
    UINT16         service_id;
    UINT16         attr_id;
    tBSA_STATUS    status;
    BOOLEAN        is_discr;
} tBSA_BLE_SE_ADDCHAR_MSG;

/* callback event data for BSA_BLE_SE_START_EVT event */
typedef struct
{
    tBSA_BLE_IF    server_if;
    UINT16         service_id;
    tBSA_STATUS    status;
} tBSA_BLE_SE_START_MSG;

typedef struct
{
    BD_ADDR       remote_bda;
    UINT32        trans_id;
    UINT16        conn_id;
    UINT16        handle;
    UINT16        offset;
    BOOLEAN       is_long;
    tBSA_STATUS   status;
} tBSA_BLE_SE_READ_MSG;

typedef struct
{
    BD_ADDR       remote_bda;
    UINT32        trans_id;
    UINT16        conn_id;
    UINT16        handle;     /* attribute handle */
    UINT16        offset;     /* attribute value offset, if no offset is needed for the command, ignore it */
    UINT16        len;        /* length of attribute value */
    UINT8         value[BSA_BLE_MAX_ATTR_LEN];  /* the actual attribute value */
    BOOLEAN       need_rsp;   /* need write response */
    BOOLEAN       is_prep;    /* is prepare write */
    tBSA_STATUS   status;
} tBSA_BLE_SE_WRITE_MSG;

typedef struct
{
    BD_ADDR     remote_bda;
    UINT32      trans_id;
    UINT16      conn_id;
    tGATT_EXEC_FLAG     exec_write;
    tBSA_STATUS     status;
} tBSA_BLE_SE_EXEC_WRITE_MSG;

typedef struct
{
    tBSA_BLE_IF        server_if;
    BD_ADDR            remote_bda;
    UINT16             conn_id;
    tBSA_BLE_REASON    reason;
} tBSA_BLE_SE_OPEN_MSG;

typedef tBSA_BLE_SE_OPEN_MSG tBSA_BLE_SE_CLOSE_MSG;

typedef struct
{
    UINT16              conn_id;    /* connection ID */
    tBTA_GATT_STATUS    status;     /* notification/indication status */
} tBSA_BLE_SE_CONF_MSG;

typedef struct
{
    UINT16              conn_id;
    BOOLEAN             congested; /* congestion indicator */
}tBSA_BLE_SE_CONGEST_MSG;

typedef struct
{
    UINT16              conn_id;
    UINT16              mtu;
}tBSA_BLE_SE_MTU_MSG;

/* Union of data associated with HD callback */
typedef union
{
    tBSA_BLE_CL_OPEN_MSG         cli_open;          /* BSA_BLE_CL_OPEN_EVT */
    tBSA_BLE_CL_SEARCH_RES_MSG   cli_search_res;    /* BSA_BLE_CL_SEARCH_RES_EVT */
    tBSA_BLE_CL_SEARCH_CMPL_MSG  cli_search_cmpl;   /* BSA_BLE_CL_SEARCH_CMPL_EVT */
    tBSA_BLE_CL_READ_MSG         cli_read;          /* BSA_BLE_CL_READ_EVT */
    tBSA_BLE_CL_WRITE_MSG        cli_write;         /* BSA_BLE_CL_WRITE_EVT */
    tBSA_BLE_CL_NOTIF_MSG        cli_notif;         /* BSA_BLE_CL_NOTIF_EVT */
    tBSA_BLE_CL_CONGEST_MSG      cli_congest;       /* BSA_BLE_CL_CONGEST_EVT */
    tBSA_BLE_CL_CLOSE_MSG        cli_close;         /* BSA_BLE_CL_CLOSE_EVT */
    tBSA_BLE_CL_DEREGISTER_MSG   cli_deregister;    /* BSA_BLE_CL_DEREGISTER_EVT */
    tBSA_BLE_CL_CACHE_SAVE_MSG   cli_cache_save;    /* BSA_BLE_SE_CACHE_SAVE_EVT */
    tBSA_BLE_CL_CACHE_LOAD_MSG   cli_cache_load;    /* BSA_BLE_SE_CACHE_LOAD_EVT */

    tBSA_BLE_SE_DEREGISTER_MSG   ser_deregister;    /* BSA_BLE_SE_DEREGISTER_EVT */
    tBSA_BLE_SE_CREATE_MSG       ser_create;        /* BSA_BLE_SE_CREATE_EVT */
    tBSA_BLE_SE_ADDCHAR_MSG      ser_addchar;       /* BSA_BLE_SE_ADDCHAR_EVT */
    tBSA_BLE_SE_START_MSG        ser_start;         /* BSA_BLE_SE_START_EVT */
    tBSA_BLE_SE_READ_MSG         ser_read;          /* BSA_BLE_SE_READ_EVT */
    tBSA_BLE_SE_WRITE_MSG        ser_write;         /* BSA_BLE_SE_WRITE_EVT */
    tBSA_BLE_SE_OPEN_MSG         ser_open;          /* BSA_BLE_SE_OPEN_EVT */
    tBSA_BLE_SE_EXEC_WRITE_MSG   ser_exec_write;    /* BSA_BLE_SE_EXEC_WRITE_EVT */
    tBSA_BLE_SE_CLOSE_MSG        ser_close;         /* BSA_BLE_SE_CLOSE_EVT */
    tBSA_BLE_SE_CONF_MSG         ser_conf;         /* BSA_BLE_SE_CONF_EVT */
    tBSA_BLE_SE_CONGEST_MSG      ser_congest;       /* BSA_BLE_SE_CONGEST_EVT */
    tBSA_BLE_SE_MTU_MSG          ser_mtu;
} tBSA_BLE_MSG;

/* BSA BLE Client callback function */
typedef void (tBSA_BLE_CBACK)(tBSA_BLE_EVT event, tBSA_BLE_MSG *p_data);

/*
 * Structures use to pass parameters to BSA API functions
 */
typedef struct
{
    int dummy; /* May be needed for some compilers */
} tBSA_BLE_ENABLE;

typedef struct
{
    int dummy; /* May be needed for some compilers */
} tBSA_BLE_DISABLE;

/*
 * Structures use to pass parameters to BSA BLE Clinet API functions
 */
typedef struct
{
    tBT_UUID        uuid;
    tBSA_BLE_IF     client_if; /* Client interface ID given by BSA*/
    tBSA_BLE_CBACK  *p_cback;

    tBTA_DM_BLE_SEC_ACT sec_act;
} tBSA_BLE_CL_REGISTER;

typedef struct
{
    tBSA_BLE_IF    client_if;
} tBSA_BLE_CL_DEREGISTER;

typedef struct
{
    tBSA_BLE_IF    client_if;
    BD_ADDR        bd_addr;
    BOOLEAN        is_direct;
} tBSA_BLE_CL_OPEN;

typedef struct
{
    UINT16  conn_id;
} tBSA_BLE_CL_CLOSE;

typedef struct
{
    UINT16      conn_id;
    tBT_UUID    uuid;
} tBSA_BLE_CL_SEARCH;

typedef struct
{
    UINT16                      conn_id;
    tBSA_BLE_CL_CHAR_ID          char_id;
    tBSA_BLE_CL_CHAR_DESCR_ID    descr_id;
    UINT8                       auth_req;
    BOOLEAN                     descr;
} tBSA_BLE_CL_READ;

#define BSA_BLE_CL_WRITE_MAX  100

typedef struct
{
    UINT16                      conn_id;
    tBSA_BLE_CL_CHAR_ID          char_id;
    tBSA_BLE_WRITE_TYPE         write_type;
    UINT16                      len;
    UINT8                       value[BSA_BLE_CL_WRITE_MAX];
    UINT8                       auth_req;
    tBSA_BLE_CL_CHAR_DESCR_ID    descr_id;
    BOOLEAN                     descr;
} tBSA_BLE_CL_WRITE;

typedef struct
{
    tBSA_BLE_IF         client_if;
    tBSA_BLE_CL_CHAR_ID  notification_id;
    BD_ADDR             bd_addr;
} tBSA_BLE_CL_NOTIFREG;

typedef tBSA_BLE_CL_NOTIFREG tBSA_BLE_CL_NOTIFDEREG;

typedef struct
{
    UINT16               more;
    UINT16               num_attr;
    UINT16               attr_index;
    UINT16               conn_id;
    BD_ADDR              bd_addr;
    tBTA_GATTC_NV_ATTR   attr[BSA_BLE_CL_NV_LOAD_MAX];
} tBSA_BLE_CL_CACHE_LOAD;

typedef struct
{
    UINT16                      conn_id;
    tBSA_BLE_CL_CHAR_ID          char_id;
} tBSA_BLE_CL_INDCONF;


/*
 * Structures use to pass parameters to BSA BLE Server API functions
 */
#define BSA_BLE_SE_WRITE_MAX  BTA_GATT_MAX_ATTR_LEN

typedef struct
{
    tBT_UUID        uuid;
    tBSA_BLE_IF     server_if; /* Server interface ID set by BSA */
    tBSA_BLE_CBACK  *p_cback;

    tBTA_DM_BLE_SEC_ACT sec_act;
} tBSA_BLE_SE_REGISTER;

typedef struct
{
    tBSA_BLE_IF    server_if;
} tBSA_BLE_SE_DEREGISTER;

typedef struct
{
    tBSA_BLE_IF    server_if;
    BD_ADDR        bd_addr;
    BOOLEAN        is_direct;
} tBSA_BLE_SE_OPEN;

typedef struct
{
    UINT16  conn_id;
} tBSA_BLE_SE_CLOSE;

typedef struct
{
    tBSA_BLE_IF    server_if;
    tBT_UUID       service_uuid;
    UINT8          inst;
    UINT16         num_handle;
    BOOLEAN        is_primary;
} tBSA_BLE_SE_CREATE;

typedef struct
{
    BOOLEAN                  is_descr;
    UINT16                   service_id;
    tBT_UUID                 char_uuid;
    tBSA_BLE_PERM            perm;
    tBSA_BLE_CHAR_PROP       property;
} tBSA_BLE_SE_ADDCHAR;

typedef struct
{
    UINT16       conn_id;
    UINT16       attr_id;
    UINT16       data_len;
    UINT8        value[BSA_BLE_SE_WRITE_MAX];
    BOOLEAN      need_confirm;
} tBSA_BLE_SE_SENDIND;

typedef struct
{
    UINT16           conn_id;
    UINT32           trans_id;
    UINT8            status;    /*The status of the request to be sent to the remote devices */
    UINT16           handle;     /* attribute handle */
    UINT16           offset;     /* attribute value offset, if no offfset is needed for the command, ignore it */
    UINT16           len;        /* length of attribute value */
    tGATT_AUTH_REQ   auth_req;   /*  authentication request */
    UINT8            value[BSA_BLE_MAX_ATTR_LEN];  /* the actual attribute value */
}tBSA_BLE_SE_SENDRSP;

typedef struct
{
    UINT16                   service_id;
    tBSA_BLE_TRANSPORT       sup_transport;
} tBSA_BLE_SE_START;

typedef struct
{
    UINT32 cond_type;
    tBSA_DM_BLE_PF_COND_PARAM cond;
    tBSA_STATUS status;
} tBSA_BLE_WAKE_CFG;

typedef struct
{
    BOOLEAN enable;
    BD_ADDR tgt_bdaddr;
} tBSA_BLE_WAKE_ENABLE;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif



/*******************************************************************************
 **
 ** Function         BSA_BleEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleEnableInit(tBSA_BLE_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function         BSA_BleEnable
 **
 ** Description      This function enable BLE
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleEnable(tBSA_BLE_ENABLE *p_enable);


/*******************************************************************************
 **
 ** Function         BSA_BleDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleDisableInit(tBSA_BLE_DISABLE *p_disable);

/*******************************************************************************
 **
 ** Function         BSA_BleDisable
 **
 ** Description      This function disable BLE
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleDisable(tBSA_BLE_DISABLE *p_disable);

/*******************************************************************************
 **
 ** Function         BSA_BleClAppRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClAppRegisterInit(tBSA_BLE_CL_REGISTER *p_reg);

/*******************************************************************************
 **
 ** Function         BSA_BleClAppRegister
 **
 ** Description      This function is called to register client application
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClAppRegister(tBSA_BLE_CL_REGISTER *p_reg);

/*******************************************************************************
 **
 ** Function         BSA_BleClConnectInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClConnectInit(tBSA_BLE_CL_OPEN *p_open);

/*******************************************************************************
 **
 ** Function         BSA_BleClConnect
 **
 ** Description      This function is called to open an BLE connection  to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClConnect(tBSA_BLE_CL_OPEN *p_open);

/*******************************************************************************
 **
 ** Function         BSA_BleClSearchInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClSearchInit(tBSA_BLE_CL_SEARCH *p_search);

/*******************************************************************************
 **
 ** Function         BSA_BleClServiceSearch
 **
 ** Description      This function is called to search service list to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClSearch(tBSA_BLE_CL_SEARCH *p_search);

/*******************************************************************************
 **
 ** Function         BSA_BleClReadInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClReadInit(tBSA_BLE_CL_READ *p_read);

/*******************************************************************************
 **
 ** Function         BSA_BleClRead
 **
 ** Description      This function is called to read a data to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClRead(tBSA_BLE_CL_READ *p_read);

/*******************************************************************************
 **
 ** Function         BSA_BleClWriteInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClWriteInit(tBSA_BLE_CL_WRITE *p_write);

/*******************************************************************************
 **
 ** Function         BSA_BleClWrite
 **
 ** Description      This function is called to read a data to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClWrite(tBSA_BLE_CL_WRITE *p_write);

/*******************************************************************************
 **
 ** Function         BSA_BleClNotifRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClNotifRegisterInit(tBSA_BLE_CL_NOTIFREG *p_reg);

/*******************************************************************************
 **
 ** Function         BSA_BleClNotifRegister
 **
 ** Description      This function is called to register a notification
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClNotifRegister(tBSA_BLE_CL_NOTIFREG *p_reg);

/*******************************************************************************
 **
 ** Function         BSA_BleClCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClCloseInit(tBSA_BLE_CL_CLOSE *p_close);

/*******************************************************************************
 **
 ** Function         BSA_BleClClose
 **
 ** Description      This function is called to close an BLE connection  to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClClose(tBSA_BLE_CL_CLOSE *p_close);



/*******************************************************************************
 **
 ** Function         BSA_BleClAppDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClAppDeregisterInit(tBSA_BLE_CL_DEREGISTER *p_dereg);

/*******************************************************************************
 **
 ** Function         BSA_BleClAppDeregister
 **
 ** Description      This function is called to deregister app
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClAppDeregister(tBSA_BLE_CL_DEREGISTER *p_dereg);

/*******************************************************************************
 **
 ** Function         BSA_BleClNotifDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClNotifDeregisterInit(tBSA_BLE_CL_NOTIFDEREG *p_dereg);

/*******************************************************************************
 **
 ** Function         BSA_BleClNotifDeregister
 **
 ** Description      This function is called to deregister app
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClNotifDeregister(tBSA_BLE_CL_NOTIFDEREG *p_dereg);


/*******************************************************************************
 **
 ** Function         BSA_BleClCacheLoadInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClCacheLoadInit(tBSA_BLE_CL_CACHE_LOAD *p_load);

/*******************************************************************************
 **
 ** Function         BSA_BleClCacheLoad
 **
 ** Description      This function is called to load attribute to bsa_server
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClCacheLoad(tBSA_BLE_CL_CACHE_LOAD *p_load);

/*******************************************************************************
 **
 ** Function         BSA_BleClIndConfInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClIndConfInit(tBSA_BLE_CL_INDCONF *p_indc);

/*******************************************************************************
 **
 ** Function         BSA_BleClIndConf
 **
 ** Description      This function is called to send a confirmation to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClIndConf(tBSA_BLE_CL_INDCONF *p_indc);

/*******************************************************************************
 **
 ** Function         BSA_BleSeAppRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAppRegisterInit(tBSA_BLE_SE_REGISTER *p_reg);

/*******************************************************************************
 **
 ** Function         BSA_BleSeAppRegister
 **
 ** Description      This function is called to register server application
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAppRegister(tBSA_BLE_SE_REGISTER *p_reg);

/*******************************************************************************
 **
 ** Function         BSA_BleSeAppDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAppDeregisterInit(tBSA_BLE_SE_DEREGISTER *p_dereg);

/*******************************************************************************
 **
 ** Function         BSA_BleSeAppDeregister
 **
 ** Description      This function is called to register server application
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAppDeregister(tBSA_BLE_SE_DEREGISTER *p_dereg);


/*******************************************************************************
 **
 ** Function         BSA_BleSeCreateServiceInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeCreateServiceInit(tBSA_BLE_SE_CREATE *p_create);

/*******************************************************************************
 **
 ** Function         BSA_BleSeCreateService
 **
 ** Description      Create service
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeCreateService(tBSA_BLE_SE_CREATE *p_create);

/*******************************************************************************
 **
 ** Function         BSA_BleSeAddCharInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAddCharInit(tBSA_BLE_SE_ADDCHAR *p_add);

/*******************************************************************************
 **
 ** Function         BSA_BleSeAddChar
 **
 ** Description      Create service
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAddChar(tBSA_BLE_SE_ADDCHAR *p_add);

/*******************************************************************************
 **
 ** Function         BSA_BleSeStartServiceInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeStartServiceInit(tBSA_BLE_SE_START *p_start);

/*******************************************************************************
 **
 ** Function         BSA_BleSeStartService
 **
 ** Description      Start service
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeStartService(tBSA_BLE_SE_START *p_start);

/*******************************************************************************
 **
 ** Function         BSA_BleSeConnectInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeConnectInit(tBSA_BLE_SE_OPEN *p_open);

/*******************************************************************************
 **
 ** Function         BSA_BleSeConnect
 **
 ** Description      This function is called to open an BLE connection  to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeConnect(tBSA_BLE_SE_OPEN *p_open);

/*******************************************************************************
 **
 ** Function         BSA_BleSeCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeCloseInit(tBSA_BLE_SE_CLOSE *p_close);

/*******************************************************************************
 **
 ** Function         BSA_BleSeClose
 **
 ** Description      This function is called to close an BLE connection to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeClose(tBSA_BLE_SE_CLOSE *p_close);

/*******************************************************************************
 **
 ** Function         BSA_BleSeSendIndInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeSendIndInit(tBSA_BLE_SE_SENDIND *p_sendind);

/*******************************************************************************
 **
 ** Function         BSA_BleSeSendInd
 **
 ** Description      Send Indication
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeSendInd(tBSA_BLE_SE_SENDIND *p_sendind);

/*******************************************************************************
 **
 ** Function         BSA_BleSeSendRspInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeSendRspInit(tBSA_BLE_SE_SENDRSP *p_sendrsp);

/*******************************************************************************
 **
 ** Function         BSA_BleSeSendRsp
 **
 ** Description      Send Response to client for write/read request received
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeSendRsp(tBSA_BLE_SE_SENDRSP *p_sendrsp);

/*******************************************************************************
 **
 ** Function         BSA_BleWakeCfgInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleWakeCfgInit(tBSA_BLE_WAKE_CFG *p_req);

/*******************************************************************************
 **
 ** Function         BSA_BleWakeCfg
 **
 ** Description      Function to set controller for Wake on BLE
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleWakeCfg(tBSA_BLE_WAKE_CFG *p_req);

/*******************************************************************************
 **
 ** Function         BSA_BleWakeEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleWakeEnableInit(tBSA_BLE_WAKE_ENABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_BleWakeEnable
 **
 ** Description      Function to enable  Wake on BLE
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleWakeEnable(tBSA_BLE_WAKE_ENABLE *p_req);

#ifdef __cplusplus
}
#endif

#endif
