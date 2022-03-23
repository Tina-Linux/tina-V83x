/*****************************************************************************
**
**  Name:           bta_gps_api.h
**
**  Description:    This is the public interface file for the GPS shared UART
**                  (GPS) subsystem of BTA, Broadcom's Bluetooth application
**                  layer for mobile phones.
**
**  Copyright (c) 2009-2010, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_GPS_API_H
#define BTA_GPS_API_H

#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
#define BTA_GPS_ENABLE_EVT               0       /* Enable Event */
#define BTA_GPS_DISABLE_EVT              1       /* Disable Event */

typedef UINT8 tBTA_GPS_EVT;


/* data associated with BTA_GPS_ENABLE_EVT */
typedef struct
{
    tBTA_STATUS    status;
} tBTA_GPS_ENABLE;



/* Union of all GPS callback structures */
typedef union
{
    tBTA_GPS_ENABLE    enable;
} tBTA_GPS;


/* Security callback */
typedef void (tBTA_GPS_CBACK)(tBTA_GPS_EVT event, tBTA_GPS *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_GpsEnable
**
** Description
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern tBTA_STATUS BTA_GpsEnable(tBTA_GPS_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_GpsDisable
**
** Description
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_GpsDisable(void);

#ifdef __cplusplus
}
#endif

#endif /* BTA_GPS_API_H */
