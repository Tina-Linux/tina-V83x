/*****************************************************************************
**
**  Name:           bta_busapp_api.h
**
**  Description:    This is the public interface file for the BusApp service
**                  of BTA, Broadcom's Bluetooth application layer
**                  for mobile phones.
**
**  Copyright (c) 2010-2011, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_BUSAPP_API_H
#define BTA_BUSAPP_API_H

#include "bta_api.h"
#include "bta_op_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* Extra Debug Code */
#ifndef BTA_BUSAPP_DEBUG
#define BTA_BUSAPP_DEBUG             FALSE
#endif

/* Callback function event */
#define BTA_BUSAPP_LISTENING_EVT        0   /* Server has been started, no callback data */
#define BTA_BUSAPP_SEARCH_START_EVT     1   /* Search has been started, no callback data */
#define BTA_BUSAPP_FOUND_EVT            2   /* Found a device, tBTA_BUSAPP_DEVICE_INFO */
#define BTA_BUSAPP_SEARCH_COMP_EVT      3   /* Search complete/timed-out/cancelled successfully, no callback data */
#define BTA_BUSAPP_CONNECTING_EVT       4   /* Connecting a device, tBTA_BUSAPP_DEVICE_INFO */
#define BTA_BUSAPP_CONNECTED_EVT        5   /* Connected a device,  tBTA_BUSAPP_DEVICE_INFO */
#define BTA_BUSAPP_DISCONNECTED_EVT     6   /* Disconnect a device, tBTA_BUSAPP_DEVICE_INFO */
#define BTA_BUSAPP_TX_PROGRESS_EVT      7   /* Transmitting status, tBTA_BUSAPP_PROGRESS */
#define BTA_BUSAPP_RX_PROGRESS_EVT      8   /* Receiving status,    tBTA_BUSAPP_PROGRESS */
#define BTA_BUSAPP_TX_MOVE_CH_EVT       9   /* Move channel event,  tBTA_BUSAPP_STATUS (BTA_OPC_ON_BT/AMP) */
#define BTA_BUSAPP_RX_MOVE_CH_EVT       10  /* Move channel event,  tBTA_BUSAPP_STATUS (BTA_OPS_ON_BT/AMP)*/
#define BTA_BUSAPP_TX_COMPLETE_EVT      11  /* Transmitting complete, tBTA_BUSAPP_OBJECT*/
#define BTA_BUSAPP_RX_COMPLETE_EVT      12  /* Receiving complete,  tBTA_BUSAPP_OBJECT*/
#define BTA_BUSAPP_ERROR_EVT            13  /* Error report,        tBTA_BUSAPP_STATUS */
#define BTA_BUSAPP_APP_INFO_EVT         14  /* Received App info from peer, tBTA_BUSAPP_APP_INFO */

typedef UINT8 tBTA_BUSAPP_EVT;

/* API and callback function result */
#define BTA_BUSAPP_OK                0   /* succeeded */
#define BTA_BUSAPP_FAIL              1   /* failed without specific reason */
#define BTA_BUSAPP_OUT_OF_MEMEORY    2   /* failed to allocate memory */
#define BTA_BUSAPP_INTERNAL_ERROR    3   /* failed because of internal error */
#define BTA_BUSAPP_NO_MATCH_HASH     4   /* failed to find matching hash with peer */

typedef UINT8 tBTA_BUSAPP_STATUS;

#define BTA_BUSAPP_DEVICE_NAME_LENGTH   32  /* max length of device name */
#define BTA_BUSAPP_OBJECT_NAME_LENGTH   32  /* max length of object name sent or received */
#define BTA_BUSAPP_OBJ_PATH_NAME_LENGTH 128 /* max length of path and name to send */

#define BTA_BUSAPP_APP_NAME_LENGTH      64  /* max length of application name */
#define BTA_BUSAPP_APP_DATA_LENGTH      64  /* max length of application data */

/* data type for BTA_BUSAPP_FOUND_EVT, BTA_BUSAPP_CONNECTING_EVT, BTA_BUSAPP_CONNECTED_EVT */
typedef struct
{
    char                    name[BTA_BUSAPP_DEVICE_NAME_LENGTH+1];
    BD_ADDR                 bd_addr;
    int                     rssi;
} tBTA_BUSAPP_DEVICE_INFO;

/* data type for BTA_BUSAPP_APP_INFO_EVT */
typedef struct
{
    UINT16                   app_data_len;
    UINT8                   *p_app_data;
} tBTA_BUSAPP_APP_INFO;

/* data type for BTA_BUSAPP_TX_PROGRESS_EVT, BTA_BUSAPP_RX_PROGRESS_EVT */
typedef struct
{
    UINT32                  obj_size;   /* Total size of object (BTA_FS_LEN_UNKNOWN if unknown) */
    UINT16                  bytes;      /* Number of bytes read or written since last progress event */
    tBTA_OP_OPER            operation;  /* Is progress for Push or Pull */
} tBTA_BUSAPP_PROGRESS;

/* data type for BTA_BUSAPP_TX_COMPLETE_EVT, BTA_BUSAPP_RX_COMPLETE_EVT */
typedef struct
{
    char                    name[BTA_BUSAPP_OBJECT_NAME_LENGTH+1];
    tBTA_OPC_STATUS         status;     /* if BTA_BUSAPP_TX_COMPLETE_EVT */
} tBTA_BUSAPP_OBJECT;

typedef union
{
    tBTA_BUSAPP_STATUS       status;
    tBTA_BUSAPP_DEVICE_INFO  device_info;
    tBTA_BUSAPP_APP_INFO     app_info;
    tBTA_BUSAPP_PROGRESS     prog;
    tBTA_BUSAPP_OBJECT       object;
} tBTA_BUSAPP_CBACK_DATA;

/* BusApp callback function */
typedef void (tBTA_BUSAPP_CBACK)(tBTA_BUSAPP_EVT event, tBTA_BUSAPP_CBACK_DATA *p_data);

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_BusAppRegister
**
** Description      Register BUS-Application per app_id.
**
** Returns          BTA_BUSAPP_OK if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_BUSAPP_STATUS BTA_BusAppRegister (UINT8              app_id,
                                                      tBTA_BUSAPP_CBACK *p_cback,
                                                      char              *app_name,
                                                      char              *p_rx_path,
                                                      tBTA_SEC           sec_mask);

/*******************************************************************************
**
** Function         BTA_BusAppSetAppInfo
**
** Description      Set data sent to peer application through BUS.
**
** Returns          BTA_BUSAPP_OK if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_BUSAPP_STATUS BTA_BusAppSetAppInfo (UINT8  app_id,
                                                        UINT8 *app_data,
                                                        UINT16 app_data_len);

/*******************************************************************************
**
** Function         BTA_BusAppDeregister
**
** Description      Deregister BUS-application per app_id
**
** Returns          BTA_BUSAPP_OK if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_BUSAPP_STATUS BTA_BusAppDeregister (UINT8  app_id);


/*******************************************************************************
**
** Function         BTA_BusAppActiveConnect
**
** Description      Advertizes the bus-specific EIR info
**                  Performs inquiry and connect to the nearest device
**                  running the BUS-App specified by app-id
**
**                  duration : inquiry duration in sec
**
** Returns          BTA_BUSAPP_OK if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_BUSAPP_STATUS BTA_BusAppActiveConnect (UINT8  app_id,
                                                           UINT16 duration);

/*******************************************************************************
**
** Function         BTA_BusAppDisconnect
**
** Description      Terminate ActiveConnect session as well as a connected
**                  device on a specific app-id
**
** Returns          BTA_BUSAPP_OK if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_BUSAPP_STATUS BTA_BusAppDisconnect (UINT8 app_id);


/*******************************************************************************
**
** Function         BTA_BusAppSendObject
**
** Description      Send an object to peer.
**
** Returns          BTA_BUSAPP_OK if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_BUSAPP_STATUS BTA_BusAppSendObject (UINT8       app_id,
                                                        tBTA_OP_FMT format,
                                                        char       *p_name);

/*******************************************************************************
**
** Function         BTA_BusAppEnableServer
**
** Description      Starts to advertize the BUS-specific EIR info and listen BUS
**
** Returns          BTA_BUSAPP_OK if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_BUSAPP_STATUS BTA_BusAppEnableServer (UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_BusAppDisableServer
**
** Description      Removes the BUS-specific EIR info and stops listening BUS.
**
** Returns          BTA_BUSAPP_OK if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_BUSAPP_STATUS BTA_BusAppDisableServer (UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_BusAppFindDevices
**
** Description      Performs inquiry and finds nearby devices running the
**                  specified BUS-App
**
**                  duration : inquiry duration in sec
**
** Returns          BTA_BUSAPP_OK if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_BUSAPP_STATUS BTA_BusAppFindDevices (UINT8  app_id,
                                                         UINT16 duration);

/*******************************************************************************
**
** Function         BTA_BusAppConnect
**
** Description      Connects to the specific device running the specified BUS-App
**
** Returns          BTA_BUSAPP_OK if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_BUSAPP_STATUS BTA_BusAppConnect (UINT8   app_id,
                                                     BD_ADDR bda);

#ifdef __cplusplus
}
#endif

#endif /* BTA_BUSAPP_API_H */
