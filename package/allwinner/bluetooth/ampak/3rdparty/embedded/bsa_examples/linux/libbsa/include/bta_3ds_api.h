/*****************************************************************************
**
**  Name:           bta_3ds_api.h
**
**  Description:    This is the implementation of the API for the 3D Synchronization
**                  (3DS) subsystem of BTA
**
**  Copyright (c) 2012-2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_3DS_API_H
#define BTA_3DS_API_H

#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
#ifndef BTA_3DS_DEBUG
#define BTA_3DS_DEBUG    FALSE
#endif

/* 3DS status values */
#define BTA_3DS_SUCCESS         0       /* successful operation */
#define BTA_3DS_FAIL            1       /* Generic 3D failure */
#define BTA_3DS_FAIL_RESOURCES  2       /* no resources */
#define BTA_3DS_UNSUPPORTED     3       /* Feature unsupported */
typedef UINT8 tBTA_3DS_STATUS;

/* 3D Connection Announcement Type */
#define BTA_3DS_ANNOUNC_TYPE_ASSOS_NOTIF_MSK 0x01    /* Association Notification received */
#define BTA_3DS_ANNOUNC_TYPE_BAT_LVL_RPT_MSK 0x02    /* Battery Level Report received */
typedef UINT8 tBTA_3DS_ANNOUNC_TYPE;


/* 3D Sync Features masks (to populate EIR) */
#define BTA_3DS_FEAT_ASSOS_NOTIF    0x01    /* Association Notification Support */
#define BTA_3DS_FEAT_BAT_LVL_RPT    0x02    /* Battery Level Report Support */

/* The following bit indicate if the peer device must send either an Association
 * Notification or a  Battery Level Report on Start-up Synchronization */
#define BTA_3DS_FEAT_ASSO_BAT       0x04    /* Association Notification */
#define BTA_3DS_FACTORY_TEST        0x80    /* Factory Test */

/* Define default 3D feature mask */
#ifndef BTA_3DS_FEAT_DEF_MASK
/* By default, Association Notification and Battery Level supported */
/* Furthermore, application will receive Battery Level Report on Start-up Synchronization */
#define BTA_3DS_FEAT_DEF_MASK (BTA_3DS_FEAT_ASSOS_NOTIF | \
                               BTA_3DS_FEAT_BAT_LVL_RPT | \
                               BTA_3DS_FEAT_ASSO_BAT)
#endif /* BTA_3DS_FEAT_MASK */

/* Define default 3D Path Loss Threshold */
#ifndef BTA_3DS_PATH_LOSS_THRESHOLD
#define BTA_3DS_PATH_LOSS_THRESHOLD (60)    /* Around one meter (3 feets) */
#endif /* BTA_3DS_PATH_LOSS_THRESHOLD */

/* 3DS callback events */
#define BTA_3DS_ENABLE_EVT      0   /* 3D Sync enabled */
#define BTA_3DS_DISABLE_EVT     1   /* 3D Sync disabled */
#define BTA_3DS_TX_START_EVT    2   /* 3D Sync Tx Started event */
#define BTA_3DS_RX_START_EVT    3   /* 3D Sync Rx Started event */
#define BTA_3DS_STOP_EVT        4   /* 3D Sync Tx Stopped event */
#define BTA_3DS_RX_TIMEOUT_EVT  5   /* 3D Sync Lost */
#define BTA_3DS_ANNOUNC_EVT     6   /* 3D Connection Announcement received */
typedef UINT8 tBTA_3DS_EVT;

/* Event associated with BTA_AV_ENABLE_EVT */
typedef struct
{
    tBTA_3DS_STATUS  status;
} tBTA_3DS_ENABLE;

/* Event associated with BTA_AV_DISABLE_EVT */
typedef struct
{
    tBTA_3DS_STATUS  status;
} tBTA_3DS_DISABLE;


/* Event associated with BTA_3DS_TX_START_EVT */
typedef struct
{
    tBTA_3DS_STATUS  status;
} tBTA_3DS_TX_START;

/* Event associated with BTA_3DS_RX_START_EVT */
typedef struct
{
    tBTA_3DS_STATUS  status;
} tBTA_3DS_RX_START;

/* Event associated with BTA_3DS_STOP_EVT */
typedef struct
{
    tBTA_3DS_STATUS  status;
} tBTA_3DS_STOP;

/* Event associated with BTA_3DS_ANNOUNC_EVT */
typedef struct
{
    BD_ADDR bd_addr;
    tBTA_3DS_ANNOUNC_TYPE type;     /* Announcement type */
    UINT8 battery_level;            /* battery level */
    UINT16 additional_info_len;     /* Length of the Additional Information (if any) */
    UINT8 *p_additional_info;       /* Pointer on Additional Information (NULL if not data) */
} tBTA_3DS_ANNOUNC;

/* Union of data associated with 3DS callback */
typedef union
{
    tBTA_3DS_ENABLE         enable;
    tBTA_3DS_DISABLE        disable;
    tBTA_3DS_TX_START       tx_start;
    tBTA_3DS_RX_START       rx_start;
    tBTA_3DS_STOP           stop;
    tBTA_3DS_ANNOUNC        announcement;
} tBTA_3DS;

/* 3DS callback */
typedef void (tBTA_3DS_CBACK)(tBTA_3DS_EVT event, tBTA_3DS *p_data);

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/

/*******************************************************************************
**
** Function         BTA_3dsEnable
**
** Description      Enable the 3D sync service. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_3DS_ENABLE_EVT. This function must
**                  be called before other API functions.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_3dsEnable(tBTA_3DS_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_3dsDisable
**
** Description      Disable the 3D sync service. When the disable
**                  operation is complete the callback function will be
**                  called with a BTA_3DS_DISABLE_EVT.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_3dsDisable(void);

/*******************************************************************************
**
** Function         BTA_3dsTxStart
**
** Description      Start the 3D Sync Tx service. When the TxStart
**                  operation is complete the callback function will be
**                  called with a BTA_3DS_TX_START_EVT.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_3dsTxStart(void);

/*******************************************************************************
**
** Function         BTA_3dsTxData
**
** Description      Send 3D Data (shutter offsets/delay, etc) parameters
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_3dsTxData(UINT8 *p_data, UINT8 length);

/*******************************************************************************
**
** Function         BTA_3dsRxStart
**
** Description      Start the 3D Sync Rx service. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_3DS_RX_START_EVT.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_3dsRxStart(BD_ADDR bd_addr);

/*******************************************************************************
**
** Function         BTA_3dsStop
**
** Description      Stop the 3D Sync Tx/Rx service. When the stop
**                  operation is complete the callback function will be
**                  called with a BTA_3DS_STOP_EVT.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_3dsStop(void);


#ifdef __cplusplus
}
#endif

#endif /* BTA_3DS_API_H */
