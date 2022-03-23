/*****************************************************************************
**
**  Name:           bta_lecoc_api.h
**
**  Description:    This is the public interface file the LE COC I/F
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_LECOC_API_H
#define BTA_LECOC_API_H

#include "bt_target.h"
#include "bt_types.h"
#include "bta_api.h"
#include "bta_sys.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* status values */
#define BTA_LECOC_SUCCESS             0            /* Successful operation. */
#define BTA_LECOC_FAILURE             1            /* Generic failure. */
#define BTA_LECOC_BUSY                2            /* Temporarily can not handle this request. */
#define BTA_LECOC_NO_RESOURCE         3            /* No more set pm control block */

typedef UINT8 tBTA_LECOC_STATUS;

#define BTA_LECOC_MAX_CONN     MAX_L2CAP_BLE_CHANNELS
#define BTA_LECOC_MAX_RCB      MAX_L2CAP_BLE_CLIENTS

/* events received by tBTA_LECOC_CBACK */
#define BTA_LECOC_OPEN_EVT          1 /* open status of L2CAP connection */
#define BTA_LECOC_CLOSE_EVT         2 /* L2CAP connection closed */
#define BTA_LECOC_START_EVT         3 /* L2CAP server started */
#define BTA_LECOC_STOP_EVT          4 /* L2CAP server started */
#define BTA_LECOC_CL_INIT_EVT       5 /* L2CAP client initiated a connection */
#define BTA_LECOC_CONG_EVT          6 /* L2CAP connection congestion status changed */
#define BTA_LECOC_WRITE_EVT         7 /* the result for BTA_LecocWrite*/
#define BTA_LECOC_FLOW_CONTROL_EVT  8 /* result of flow control operation status */

#define BTA_LECOC_MAX_EVT              9 /* max number of LECOC events */

typedef UINT16 tBTA_LECOC_EVT;

/* data associated with BTA_LECOC_OPEN_EVT */
typedef struct
{
    tBTA_LECOC_STATUS   status;     /* Whether the operation succeeded or failed. */
    UINT32              handle;     /* The connection handle */
    BD_ADDR             rem_bda;    /* The peer address */
    INT32               tx_mtu;     /* The transmit MTU */
}tBTA_LECOC_OPEN;

/* data associated with BTA_LECOC_CLOSE_EVT */
typedef struct
{
    tBTA_LECOC_STATUS   status;     /* Whether the operation succeeded or failed. */
    UINT32              handle;     /* The connection handle */
}tBTA_LECOC_CLOSE;

/* data associated with BTA_LECOC_START_EVT */
typedef struct
{
    tBTA_LECOC_STATUS   status;     /* Whether the server started succeeded or failed. */
}tBTA_LECOC_START;

/* data associated with BTA_LECOC_CL_INIT_EVT */
typedef struct
{
    tBTA_LECOC_STATUS   status;     /* Whether the operation succeeded or failed. */
    UINT32              handle;     /* The connection handle */
}tBTA_LECOC_CL_INIT;

/* data associated with BTA_LECOC_CONG_EVT */
typedef struct
{
    tBTA_LECOC_STATUS   status;     /* Whether the operation succeeded or failed. */
    UINT32              handle;     /* The connection handle */
    BOOLEAN             tx_cong;       /* TRUE, congested. FALSE, uncongested */
}tBTA_LECOC_CONG;

/* data associated with BTA_LECOC_WRITE_EVT */
typedef struct
{
    tBTA_LECOC_STATUS   status;     /* Whether the operation succeeded or failed. */
    UINT32              req_id;
    UINT32              handle;     /* The connection handle */
    UINT32              len;        /* The length of the data written. */
    BOOLEAN             tx_cong;       /* congestion status */
}tBTA_LECOC_WRITE;

/* data associated with BTA_LECOC_FLOW_CONTROL_EVT */
typedef struct
{
    tBTA_LECOC_STATUS   status;     /* Whether the operation succeeded or failed. */
    UINT32              handle;     /* The connection handle */
    BOOLEAN             rx_cong;       /* RX congestion status, TRUE indicate congested; FALSE indicate un-congested */
}tBTA_LECOC_FLOW_CONTROL;

typedef union
{
    tBTA_LECOC_STATUS           status;
   tBTA_LECOC_OPEN              lecoc_open;       /* BTA_LECOC_OPEN_EVT */
   tBTA_LECOC_CLOSE             lecoc_close;      /* BTA_LECOC_CLOSE_EVT */
   tBTA_LECOC_CL_INIT           lecoc_cl_init;    /* BTA_LECOC_CL_INIT_EVT */
   tBTA_LECOC_CONG              lecoc_cong;       /* BTA_LECOC_CONG_EVT */
   tBTA_LECOC_WRITE             lecoc_write;      /* BTA_LECOC_WRITE_EVT */
   tBTA_LECOC_FLOW_CONTROL      flow_control; /* BTA_LECOC_FLOW_CONTROL_EVT */
}tBTA_LECOC;

/* LECOC Enable callback */
typedef void (tBTA_LECOC_ENB_CBACK)(tBTA_LECOC_STATUS status);

typedef void (tBTA_LECOC_CBACK)(tBTA_LECOC_EVT event, void *p_ref, tBTA_LECOC *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_LecocEnable
**
** Description      Enable the LE COC I/F service. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_LECOC_ENABLE_EVT. This function must
**                  be called before other function in the LECOC API are
**                  called.
**
** Parameter        p_cback: enable completion callback.
**
** Returns          BTA_LECOC_SUCCESS if successful.
**                  BTA_LECOC_FAIL if internal failure.
**
*******************************************************************************/
BTA_API extern tBTA_LECOC_STATUS BTA_LecocEnable(tBTA_LECOC_ENB_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_LecocDisable
**
** Description      Disable the LE COC I/F
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_LecocDisable(void);

/*******************************************************************************
**
** Function         BTA_LecocConnect
**
** Description      Higher layers call this function to create an L2CAP connection
**                  for LE_PSM.
**                  Note that the connection is not established at this time, but
**                  connection establishment gets started. The callback function
**                  will be invoked when connection establishes or fails.
**
** Parameter        sec_mask: security requirement of the connection request.
**                  le_psm: to which psm that the connection is going to request on.
**                  peer_bd_addr: remote device address.
**                  rx_mtu: rx path MTU size.
**                  rx_sdu_pool_id: RX SDU pool ID. If not assigned, default will be used.
**                  sec_key_size: security key size requirement for the servie.
**                  p_cback: callback function for this connection.
**                  p_ref:   reference data for the caller of this function.
**
** Returns          BTA_LECOC_SUCCESS, if the request is being processed.
**                  BTA_LECOC_FAILURE, otherwise.
**
*******************************************************************************/
BTA_API extern tBTA_LECOC_STATUS BTA_LecocConnect (tBTA_SEC sec_mask,
                                            UINT16 le_psm, BD_ADDR peer_bd_addr,
                                           UINT16 rx_mtu, UINT8 rx_sdu_pool_id,
                                           UINT8 sec_key_size, tBTA_LECOC_CBACK *p_cback,
                                           void *p_ref);

/*******************************************************************************
**
** Function         BTA_LecocDataWrite
**
** Description      Higher layers call this function to write data.  TX data will be pull
**                  from application through callout function bta_lecoc_co_tx_data(),
**                  when TX data is ready callin function bta_lecoc_ci_tx_ready() needs
**                  to be called to indicate data ready. tBTA_LECOC_L2CAP_CBACK is called
**                  with BTA_LECOC_L2CAP_LE_WRITE_EVT when the write request is complete,.
**
** Parameter        handle: connection handle.
**                  req_id: write requtest ID.
**
** Returns          BTA_LECOC_SUCCESS, if the request is being processed.
**                  BTA_LECOC_FAILURE, otherwise.
**
*******************************************************************************/
BTA_API extern tBTA_LECOC_STATUS BTA_LecocDataWrite (UINT16 handle, UINT32 req_id);

/*******************************************************************************
**
** Function         BTA_LecocClose
**
** Description      Higher layers call this function to disconnect a LE COC client
**                  connection.
**
** Parameter        handle: connection handle.
**
** Returns          BTA_LECOC_SUCCESS, if the request is being processed.
**                  BTA_LECOC_FAILURE, otherwise.
**
*******************************************************************************/
BTA_API extern tBTA_LECOC_STATUS BTA_LecocClose (UINT16 handle);

/*******************************************************************************
**
** Function         BTA_LecocStartServer
**
** Description      This function starts an L2CAP server and listens for an LE L2CAP
**                  connection oriented connection request from a remote Bluetooth device.
**                  When the server is started successfully, tBTA_LECOC_L2CAP_CBACK is called with
**                  BTA_LECOC_L2CAP_LE_START_EVT.  When the connection is established,
**                  tBTA_LECOC_L2CAP_CBACK is called with BTA_LECOC_L2CAP_LE_COC_OPEN_EVT.
**
** Parameter        sec_mask: security requirement of the service.
**                  sec_key_size: security key size requirement for the servie.
**                  local_psm: psm that the service is based on.
**                  rx_mtu: rx path MTU size.
**                  p_cback: callback function for this service.
**                  p_ref:   reference data for the caller of this function.
**
** Returns          BTA_LECOC_SUCCESS, if the request is being processed.
**                  BTA_LECOC_FAILURE, otherwise.
**
*******************************************************************************/
BTA_API extern tBTA_LECOC_STATUS BTA_LecocStartServer(tBTA_SEC sec_mask, UINT8 sec_key_size,
                                        UINT16 local_psm, UINT16 rx_mtu, tBTA_LECOC_CBACK *p_cback,
                                        void *p_ref);

/*******************************************************************************
**
** Function         BTA_LecocStopServer
**
** Description      This function stops the LE L2CAP CoC server. If the server has an
**                  active connection, it would be closed.
**
** Parameter        local_psm: psm that the service is based on.
**
** Returns          BTA_LECOC_SUCCESS, if the request is being processed.
**                  BTA_LECOC_FAILURE, otherwise.
**
*******************************************************************************/
BTA_API extern tBTA_LECOC_STATUS BTA_LecocStopServer(UINT16 local_psm);

/*******************************************************************************
**
** Function         BTA_LecocFlowControl
**
** Description      This function is called to enable or disable flow control on
**                  the data receiving path.  The layer above should call this function to
**                  disable data flow when it is congested and cannot handle
**                  any more data sent by bta_lecoc_co_data().
**
** Parameter        handle: connection handle.
**                  flow_on: TRUE to indicate congestion on RX path; FALSE indcate uncongested.

** Returns          BTA_LECOC_SUCCESS, if the request is being processed.
**                  BTA_LECOC_FAILURE, otherwise.
**
*******************************************************************************/
BTA_API extern tBTA_LECOC_STATUS BTA_LecocFlowControl (UINT16 handle, BOOLEAN flow_on);

#ifdef __cplusplus
}
#endif

#endif /* BTA_LECOC_API_H */
