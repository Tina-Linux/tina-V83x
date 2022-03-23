/*****************************************************************************
**
**  Name:           bta_bav_api.h
**
**  Description:    This is the public interface file for the Broadcast
**                  audio/video streaming (AV) subsystem of BTA.
**
**  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_BAV_API_H
#define BTA_BAV_API_H

#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* BAV Status Values */
#define BTA_BAV_SUCCESS             0   /* Successful operation */
#define BTA_BAV_FAIL                1   /* Generic failure */
#define BTA_BAV_FAIL_BAD_PARAM      2   /* Invalid parameter */
#define BTA_BAV_FAIL_STREAM         3   /* Stream failed */
#define BTA_BAV_FAIL_RESOURCES      4   /* No resources */
#define BTA_BAV_FAIL_LT_ADDR        5   /* Bad LtAddr (already used) */
#define BTA_BAV_FAIL_UNSUPPORTED    6   /* CSB unsupported by local device */
typedef UINT8 tBTA_BAV_STATUS;

/* Maximum number of streams created: 1 by default */
#ifndef BTA_BAV_STREAM_MAX
#define BTA_BAV_STREAM_MAX      1
#endif

/* BSA Handle */
#define BTA_BAV_STREAM_BAD     BTA_BAV_STREAM_MAX
typedef UINT8 tBTA_BAV_STREAM;

/* AV callback events */
#define BTA_BAV_ENABLE_EVT      0       /* BAV enabled */
#define BTA_BAV_DISABLE_EVT     1       /* BAV Disabled */
#define BTA_BAV_REGISTER_EVT    2       /* BAV Registered */
#define BTA_BAV_DEREGISTER_EVT  3       /* BAV Deregistered */
#define BTA_BAV_START_EVT       4       /* Stream Started */
#define BTA_BAV_STOP_EVT        5       /* Stream Stopped */
typedef UINT8 tBTA_BAV_EVT;

/* Data associated with BTA_BAV_ENABLE_EVT */
typedef struct
{
    tBTA_BAV_STATUS     status;
} tBTA_BAV_ENABLE;

/* Data associated with BTA_BAV_REGISTER_EVT */
typedef struct
{
    tBTA_BAV_STATUS     status;
    UINT8               lt_addr;
    tBTA_BAV_STREAM     stream;       /* Handle associated with the stream. */
    UINT8               app_id;      /* ID associated with call to BTA_BavRegister() */
} tBTA_BAV_REGISTER;

/* Data associated with BTA_BAV_DEREGISTER_EVT */
typedef struct
{
    tBTA_BAV_STATUS     status;
    tBTA_BAV_STREAM     stream;       /* Handle associated with the stream. */
} tBTA_BAV_DEREGISTER;

/* Data associated with BTA_BAV_START_EVT */
typedef struct
{
    tBTA_BAV_STATUS     status;
    tBTA_BAV_STREAM     stream;
} tBTA_BAV_START;

/* Data associated with BTA_BAV_STOP_EVT */
typedef struct
{
    tBTA_BAV_STATUS     status;
    tBTA_BAV_STREAM     stream;
} tBTA_BAV_STOP;



/* union of data associated with BAV callback */
typedef union
{
    tBTA_BAV_ENABLE      enable;
    tBTA_BAV_REGISTER    registr;
    tBTA_BAV_DEREGISTER  deregister;
    tBTA_BAV_START       start;
    tBTA_BAV_STOP        stop;
} tBTA_BAV;


/* BAV callback */
typedef void (tBTA_BAV_CBACK)(tBTA_BAV_EVT event, tBTA_BAV *p_data);

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/

/*******************************************************************************
**
** Function         BTA_BavEnable
**
** Description      Enable the Broadcast audio/video service. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_BAV_ENABLE_EVT. This function must
**                  be called before other function in the BAV API are
**                  called.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BavEnable(tBTA_BAV_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_BavDisable
**
** Description      Disable the Broadcast audio/video service.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BavDisable(void);

/*******************************************************************************
**
** Function         BTA_BavRegister
**
** Description      Register the audio or video service to stack. When the
**                  operation is complete the callback function will be
**                  called with a BTA_BAV_REGISTER_EVT. This function must
**                  be called before BAV stream is open.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BavRegister(UINT8 lt_addr, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_BavDeregister
**
** Description      Deregister the audio or video service
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BavDeregister(tBTA_BAV_STREAM stream);

/*******************************************************************************
**
** Function         BTA_BavStart
**
** Description      Start a Broadcast audio/video stream
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BavStart(tBTA_BAV_STREAM stream);

/*******************************************************************************
**
** Function         BTA_BavStop
**
** Description      Stop a Broadcast audio/video stream
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BavStop(tBTA_BAV_STREAM stream);

#ifdef __cplusplus
}
#endif

#endif /* BTA_BAV_API_H */
