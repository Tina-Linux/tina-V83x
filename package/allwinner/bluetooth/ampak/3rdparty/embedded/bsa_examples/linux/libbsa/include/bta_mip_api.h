/*****************************************************************************
**
**  Name:           bta_mip_api.h
**
**  Description:    This is the public interface file for the BTA MIP module.
**
**  Copyright (c) 2011, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_MIP_API_H
#define BTA_MIP_API_H

#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* Extra Debug Code */
#ifndef BTA_MIP_DEBUG
#define BTA_MIP_DEBUG       FALSE
#endif

#define BTA_MIP_PWR_OFF        0
#define BTA_MIP_PWR_ON         1
typedef UINT8 tBTA_MIP_PWR;

#define BTA_MIP_DEV_A2DP        0
#define BTA_MIP_DEV_3DG         1
#define BTA_MIP_DEV_ANY         0xFF
typedef UINT8 tBTA_MIP_DEV_TYPE;

#define BTA_MIP_NUM_DEV_TYPES   2

/* Data type for BTA_MIPClose() */
#define BTA_MIP_ALL_A2DP_DEV    (0xF0 | BTA_MIP_DEV_A2DP)
#define BTA_MIP_ALL_3DG_DEV     (0xF0 | BTA_MIP_DEV_3DG)
#define BTA_MIP_ONE_DEV         0
typedef UINT8 tBTA_MIP_CLOSE_TYPE;

/* Callback function event */
#define BTA_MIP_DEV_READY_EVT   0   /* MIP device has opened ACL connection, ready for MIP open */
#define BTA_MIP_OPEN_EVT        1   /* MIP connection is opened for the device */
#define BTA_MIP_CLOSE_EVT       2   /* MIP connection was closed for the device */
#define BTA_MIP_RXDATA_EVT      3   /* RX data was received from remote MIP device */

#define BTA_MIP_LAST_EVT        BTA_MIP_RXDATA_EVT

typedef UINT8 tBTA_MIP_EVT;


typedef struct
{
    tBTA_STATUS             status;
    BD_ADDR                 bd_addr;
    tBTA_MIP_DEV_TYPE       dev_type;
} tBTA_MIP_DEVICE_INFO;

typedef struct
{
    tBTA_STATUS             status;
    BD_ADDR                 bd_addr;
} tBTA_MIP_OPEN, tBTA_MIP_CLOSE;

typedef struct
{
    tBTA_STATUS             status;
    BD_ADDR                 bd_addr;
    UINT8                   rx_len;
    UINT8                   rx_data[BTM_MIP_MAX_RX_LEN];
} tBTA_MIP_RXDATA;

typedef union
{
    tBTA_MIP_OPEN           open;
    tBTA_MIP_CLOSE          close;
    tBTA_MIP_DEVICE_INFO    dev_info;
    tBTA_MIP_RXDATA         rxdata;
} tBTA_MIP_CBACK_DATA;

/* MIP callback function */
typedef void (tBTA_MIP_CBACK) (tBTA_MIP_EVT event, tBTA_MIP_CBACK_DATA *p_data);

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_MIPRegister
**
** Description      Register MIP application to BTA
**
** Returns          BTA_SUCCESS if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_STATUS BTA_MIPRegister (tBTA_MIP_DEV_TYPE dev_type, tBTA_MIP_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_MIPDeregister
**
** Description      Deregister MIP application
**
** Returns          BTA_SUCCESS if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_STATUS BTA_MIPDeregister (tBTA_MIP_DEV_TYPE dev_type);

/*******************************************************************************
**
** Function         BTA_MIPOpen
**
** Description      Opens MIP connection with the specified device.
**
** Returns          BTA_SUCCESS if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_STATUS BTA_MIPOpen (BD_ADDR bd_addr);

/*******************************************************************************
**
** Function         BTA_MIPClose
**
** Description      Closes MIP connection with the specified device.
**                  If close_type is all devices, bd_addr param will be ignored.
**
** Returns          BTA_SUCCESS if successful, error code otherwise
**
*******************************************************************************/
BTA_API extern tBTA_STATUS BTA_MIPClose (tBTA_MIP_CLOSE_TYPE close_type, BD_ADDR bd_addr);

/*******************************************************************************
**
** Function         BTA_MipSetPower
**
** Description      Turns on or off MIP mode in BTA
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MipSetPower (tBTA_MIP_PWR mip_pwr, UINT8 *aud_cfg);

/*******************************************************************************
**
** Function         BTA_MipAddDevice
**
** Description      Add the device in BTA MIP database
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MipAddDevice (BD_ADDR bd_addr, tBTA_MIP_DEV_TYPE dev_type, LINK_KEY link_key);

/*******************************************************************************
**
** Function         BTA_MipDeleteDevice
**
** Description      Delete the device in BTA MIP database
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MipDeleteDevice (BD_ADDR bd_addr);

#ifdef __cplusplus
}
#endif

#endif /* BTA_MIP_API_H */
