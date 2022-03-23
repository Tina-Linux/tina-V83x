/*****************************************************************************
 **
 **  Name:           bsa_dg_api.h
 **
 **  Description:    This is the public interface file for the data gateway
 **                  (DG) subsystem of BSA, Broadcom's Bluetooth simple api
 **                  layer.
 **
 **  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_DG_API_H
#define BSA_DG_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"
#include "bta_dg_api.h"

/* DG Callback events */
#define BSA_DG_OPEN_EVT         0       /* Connection has been opened. */
#define BSA_DG_CLOSE_EVT        1       /* Connection has been closed. */
#define BSA_DG_FIND_SERVICE_EVT 2       /* Find Service */

typedef UINT8 tBSA_DG_EVT;

typedef UINT16 tBSA_DG_HNDL;


/* Structure associated with BSA_DG_OPEN_EVT */
typedef struct {
    tBSA_STATUS status;
    BD_ADDR bd_addr;
    tBSA_SERVICE_ID service;
    tBSA_DG_HNDL handle;
    tUIPC_CH_ID uipc_channel;
} tBSA_DG_OPEN_MSG;

/* Structure associated with BSA_DG_CLOSE_EVT */
typedef struct {
    tBSA_DG_HNDL handle;
} tBSA_DG_CLOSE_MSG;

/* Structure associated with BSA_DG_FIND_SERVICE_EVT */
typedef struct {
    tBSA_STATUS status; /* BSA_SUCCESS if service lookup succeeded */
    BD_ADDR bd_addr;
    tBT_UUID uuid;
    BOOLEAN found;  /* TRUE if service found */
    UINT8     scn;
    char      name[BSA_SERVICE_NAME_LEN];
} tBSA_DG_FIND_SERVICE_MSG;

/* Union of all server callback structures */
typedef union {
    tBSA_DG_OPEN_MSG open;
    tBSA_DG_CLOSE_MSG close;
    tBSA_DG_FIND_SERVICE_MSG find_service;
} tBSA_DG_MSG;

/* Server callback function */
typedef void ( tBSA_DG_CBACK)(tBSA_DG_EVT event, tBSA_DG_MSG *p_data);

typedef struct {
    tBSA_DG_CBACK *p_cback;
} tBSA_DG_ENABLE;

typedef struct {
    UINT8 dummy;
} tBSA_DG_DISABLE;

typedef struct {
    tBSA_SERVICE_ID service;
    tBSA_SEC_AUTH sec_mask;
    char service_name[BSA_SERVICE_NAME_LEN];
    tBSA_DG_HNDL handle;
    tBT_UUID uuid;
} tBSA_DG_LISTEN;

typedef struct {
    BD_ADDR bd_addr;
    tBSA_SERVICE_ID service;
    tBSA_SEC_AUTH sec_mask;
    char service_name[BSA_SERVICE_NAME_LEN];
    tBT_UUID uuid;
} tBSA_DG_OPEN;

typedef struct {
    tBSA_DG_HNDL handle;
} tBSA_DG_CLOSE;

typedef struct {
    tBSA_DG_HNDL handle;
} tBSA_DG_SHUTDOWN;

typedef struct {
    BD_ADDR bd_addr;
    tBT_UUID uuid;
} tBSA_DG_FIND_SERVICE;

/*****************************************************************************
 **  External Function Declarations
 *****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function         BSA_DgEnable
 **
 ** Description      Enable the data gateway service.  This function must be
 **                  called before any other functions in the DG API are called.
 **                  When the enable operation is complete the callback function
 **                  will return BSA_SUCCESS.  After the DG
 **                  service is enabled a server can be started by calling
 **                  BSA_DgListen().
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgEnable(tBSA_DG_ENABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_DgEnableInit
 **
 ** Description      Initialize the tBSA_DG_ENABLE_CMD structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgEnableInit(tBSA_DG_ENABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_DgDisable
 **
 ** Description      Disable the data gateway service.  Before calling this
 **                  function all DG servers must be shut down by calling
 **                  BSA_DgShutdown().
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgDisable(tBSA_DG_DISABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_DgDisableInit
 **
 ** Description      Initialize the tBSA_DG_DISABLE_CMD structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgDisableInit(tBSA_DG_DISABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_DgListen
 **
 ** Description      Create a DG server for DUN or SPP.  After creating a
 **                  server peer devices can open an RFCOMM connection to the
 **                  server.  When the listen operation has started the function will return BSA_SUCCESS
 **                  the handle associated with this server.  The handle
 **                  identifies server when calling other DG functions such as
 **                  BSA_DgClose() or BSA_DgShutdown().
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgListen(tBSA_DG_LISTEN *p_req);

/*******************************************************************************
 **
 ** Function         BSA_DgListenInit
 **
 ** Description      Initialize the tBSA_DG_LISTEN structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgListenInit(tBSA_DG_LISTEN *p_req);

/*******************************************************************************
 **
 ** Function         BSA_DgOpen
 **
 ** Description      Open a DG client connection to a peer device.  BSA first
 **                  searches for the requested service on the peer device.  If
 **                  the service name is specified it will also match the
 **                  service name.  Then BSA initiates an RFCOMM connection to
 **                  the peer device.  The handle associated with the connection
 **                  is returned with the BSA_DG_OPEN_EVT.  If the connection
 **                  fails or closes at any time the callback function will be
 **                  called with a BSA_DG_CLOSE_EVT.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgOpen(tBSA_DG_OPEN *p_req);\

/*******************************************************************************
 **
 ** Function         BSA_DgOpenInit
 **
 ** Description      Initialize the tBSA_DG_OPEN structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgOpenInit(tBSA_DG_OPEN *p_req);

/*******************************************************************************
 **
 ** Function         BSA_DgClose
 **
 ** Description      Close a DG server connection to a peer device.  BSA will
 **                  close the RFCOMM connection to the peer device.  The server
 **                  will still be listening for subsequent connections.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgClose(tBSA_DG_CLOSE *p_req);\

/*******************************************************************************
 **
 ** Function         BSA_DgCloseInit
 **
 ** Description      Initialize the tBSA_DG_CLOSE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgCloseInit(tBSA_DG_CLOSE *p_req);

/*******************************************************************************
**
** Function         BSA_DgShutdown
**
** Description      Shutdown a DG server previously started by calling
**                  BSA_DgListen().  The server will no longer be available
**                  to peer devices.  If there is currently a connection open
**                  to the server it will be closed.
**
**
** Returns          void
**
*******************************************************************************/
extern tBSA_STATUS BSA_DgShutdown(tBSA_DG_SHUTDOWN *p_req);

/*******************************************************************************
 **
 ** Function         BSA_DgShutdownInit
 **
 ** Description      Initialize the tBSA_DG_SHUTDOWN structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgShutdownInit(tBSA_DG_SHUTDOWN *p_req);

/*******************************************************************************
 **
 ** Function         BSA_DgFindService
 **
 ** Description      Performs SDP on specified device to check if the device supports
 **                  the specified serial service. Response is received in the
 **                  BSA_DG_FIND_SERVICE_EVT indicating if the service is present
 **                  or not.
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgFindService(tBSA_DG_FIND_SERVICE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_DgFindServiceInit
 **
 ** Description      Initialize the tBSA_DG_FIND_SERVICE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_DgFindServiceInit(tBSA_DG_FIND_SERVICE *p_req);

#ifdef __cplusplus
}
#endif

#endif /* BSA_DG_API_H */
