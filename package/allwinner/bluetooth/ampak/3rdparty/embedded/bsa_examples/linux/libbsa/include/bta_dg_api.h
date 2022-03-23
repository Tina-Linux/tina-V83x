/*****************************************************************************
**
**  Name:           bta_dg_api.h
**
**  Description:    This is the public interface file for the data gateway
**                  (DG) subsystem of BTA, Widcomm's Bluetooth application
**                  layer for mobile phones.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_DG_API_H
#define BTA_DG_API_H

#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* DG Callback events */
#define BTA_DG_ENABLE_EVT       0       /* DG service is enabled. */
#define BTA_DG_LISTEN_EVT       1       /* Server listen is started. */
#define BTA_DG_OPENING_EVT      2       /* Client connection opening. */
#define BTA_DG_OPEN_EVT         3       /* Connection has been opened. */
#define BTA_DG_CLOSE_EVT        4       /* Connection has been closed. */

typedef UINT8 tBTA_DG_EVT;

/* Event associated with BTA_DG_LISTEN_EVT */
typedef struct
{
    UINT16          handle;     /* Handle associated with this server. */
    UINT8           app_id;     /* ID associated with call to BTA_DgListen(). */
} tBTA_DG_LISTEN;

/* Event associated with BTA_DG_OPENING_EVT */
typedef struct
{
    UINT16          handle;     /* Handle associated with this server. */
    UINT8           app_id;     /* ID associated with call to BTA_DgListen(). */
} tBTA_DG_OPENING;


/* Event associated with BTA_DG_OPEN_EVT */
typedef struct
{
    BD_ADDR         bd_addr;    /* BD address of peer device. */
    UINT16          handle;     /* Handle associated with this server. */
    tBTA_SERVICE_ID service;    /* Service ID of opened service. */
    UINT8           app_id;     /* ID associated with call to BTA_DgListen(). */
} tBTA_DG_OPEN;

/* Event associated with BTA_DG_CLOSE_EVT */
typedef struct
{
    UINT16          handle;     /* Handle associated with this server. */
    UINT8           app_id;     /* ID associated with call to BTA_DgListen(). */
} tBTA_DG_CLOSE;

/* Union of all DG callback structures */
typedef union
{
    tBTA_DG_LISTEN  listen;     /* Server listen is started. */
    tBTA_DG_OPENING opening;    /* Client connection opening. */
    tBTA_DG_OPEN    open;       /* Connection has been opened. */
    tBTA_DG_CLOSE   close;      /* Connection has been closed. */
} tBTA_DG;

/* DG callback */
typedef void (tBTA_DG_CBACK)(tBTA_DG_EVT event, tBTA_DG *p_data);

/* configuration structure */
typedef struct
{
    UINT16      mtu[4];                     /* MTU for SPP, DUN, FAX, LAP */

} tBTA_DG_CFG;

/* Number of DG servers */
#ifndef BTA_DG_NUM_CONN
#define BTA_DG_NUM_CONN         20 /* BSA_SPECIFIC */
#endif

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_DgEnable
**
** Description      Enable the data gateway service.  This function must be
**                  called before any other functions in the DG API are called.
**                  When the enable operation is complete the callback function
**                  will be called with a BTA_DG_ENABLE_EVT.  After the DG
**                  service is enabled a server can be started by calling
**                  BTA_DgListen().
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_DgEnable(tBTA_DG_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_DgDisable
**
** Description      Disable the data gateway service.  Before calling this
**                  function all DG servers must be shut down by calling
**                  BTA_DgShutdown().
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_DgDisable(void);

/* BSA_SPECIFIC */
/*******************************************************************************
**
** Function         BTA_DgListen
**
** Description      Create a DG server for DUN or SPP.  After creating a
**                  server peer devices can open an RFCOMM connection to the
**                  server.  When the listen operation has started the callback
**                  function will be called with a BTA_DG_LISTEN_EVT providing
**                  the handle associated with this server.  The handle
**                  identifies server when calling other DG functions such as
**                  BTA_DgClose() or BTA_DgShutdown().
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_DgListen(tBTA_SERVICE_ID service, tBTA_SEC sec_mask,
                                 char *p_service_name, UINT8 app_id, tBT_UUID uuid);

/* BSA_SPECIFIC */
/*******************************************************************************
**
** Function         BTA_DgOpen
**
** Description      Open a DG client connection to a peer device.  BTA first
**                  searches for the requested service on the peer device.  If
**                  the service name is specified it will also match the
**                  service name.  Then BTA initiates an RFCOMM connection to
**                  the peer device.  The handle associated with the connection
**                  is returned with the BTA_DG_OPEN_EVT.  If the connection
**                  fails or closes at any time the callback function will be
**                  called with a BTA_DG_CLOSE_EVT.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_DgOpen(BD_ADDR bd_addr, tBTA_SERVICE_ID service,
        tBTA_SEC sec_mask, char *p_service_name, UINT8 app_id, tBT_UUID uuid);

/*******************************************************************************
**
** Function         BTA_DgClose
**
** Description      Close a DG connection to a peer device.  BTA will
**                  close the RFCOMM connection to the peer device.  Servers
**                  will still be listening for subsequent connections.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_DgClose(UINT16 handle);

/*******************************************************************************
**
** Function         BTA_DgShutdown
**
** Description      Shutdown a DG server previously started by calling
**                  BTA_DgListen().  The server will no longer be available
**                  to peer devices.  If there is currently a connection open
**                  to the server it will be closed.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_DgShutdown(UINT16 handle);

#ifdef __cplusplus
}
#endif

#endif /* BTA_DG_API_H */
