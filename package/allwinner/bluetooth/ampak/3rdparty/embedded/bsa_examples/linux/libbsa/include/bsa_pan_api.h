/*****************************************************************************
 **
 **  Name:           bsa_pan_api.h
 **
 **  Description:    This is the public interface file for the Personal Area
 **                  Networking Profile (PAN) subsystem of BSA, Broadcom's
 **                  Bluetooth simple api layer.
 **
 **  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_PAN_API_H
#define BSA_PAN_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"
#include "bta_pan_api.h"


/* PAN connection handle */
typedef UINT16 tBSA_PAN_HNDL;

/* PAN role */
typedef tBTA_PAN_ROLE tBSA_PAN_ROLE;
#define BSA_PAN_ROLE_PANU               BTA_PAN_ROLE_PANU
#define BSA_PAN_ROLE_NAP                BTA_PAN_ROLE_NAP
#define BSA_PAN_ROLE_GN                 BTA_PAN_ROLE_GN

/* Maximum number of protocol/multicast filters */
#define BSA_PAN_MAX_PROT_FILTERS        BNEP_MAX_PROT_FILTERS
#define BSA_PAN_MAX_MULTI_FILTERS       BNEP_MAX_MULTI_FILTERS

/* PAN Callback events */
typedef enum {
    BSA_PAN_OPEN_EVT,                   /* Connection has been opened. */
    BSA_PAN_CLOSE_EVT,                  /* Connection has been closed. */
    BSA_PAN_PFILT_EVT,                  /* Protocol filter has beed received. */
    BSA_PAN_MFILT_EVT,                  /* Multicast filter has beed received.*/
} tBSA_PAN_EVT;

/* Structure associated with BSA_PAN_OPEN_EVT */
typedef struct {
    tBSA_STATUS status;
    BD_ADDR bd_addr;
    tBSA_PAN_ROLE local_role;
    tBSA_PAN_ROLE peer_role;
    tBSA_PAN_HNDL handle;
    tUIPC_CH_ID uipc_channel;
} tBSA_PAN_OPEN_MSG;

/* Structure associated with BSA_PAN_CLOSE_EVT */
typedef struct {
    tBSA_PAN_HNDL handle;
} tBSA_PAN_CLOSE_MSG;

/* Structure associated with BSA_PAN_PFILT_EVT */
typedef struct {
    tBSA_PAN_HNDL handle;
    BOOLEAN indication;
    tBSA_STATUS status;
    UINT16 len;                         /* bytes of data[] */
    UINT16 data[BSA_PAN_MAX_PROT_FILTERS * 2];
} tBSA_PAN_PFILT_MSG;

/* Structure associated with BSA_PAN_MFILT_EVT */
typedef struct {
    tBSA_PAN_HNDL handle;
    BOOLEAN indication;
    tBSA_STATUS status;
    UINT16 len;                         /* bytes of data[] */
    BD_ADDR data[BSA_PAN_MAX_MULTI_FILTERS * 2];
} tBSA_PAN_MFILT_MSG;


/* Union of all server callback structures */
typedef union {
    tBSA_PAN_OPEN_MSG open;
    tBSA_PAN_CLOSE_MSG close;
    tBSA_PAN_PFILT_MSG pfilt;
    tBSA_PAN_MFILT_MSG mfilt;
} tBSA_PAN_MSG;


/* Server callback function */
typedef void (tBSA_PAN_CBACK)(tBSA_PAN_EVT event, tBSA_PAN_MSG *p_data);


/* PAN API struectures */
typedef struct {
    tBSA_PAN_CBACK *p_cback;
} tBSA_PAN_ENABLE;

typedef struct {
    UINT8 dummy;
} tBSA_PAN_DISABLE;

typedef struct {
    char srv_name[BSA_SERVICE_NAME_LEN];
    UINT8 app_id;
    tBSA_SEC_AUTH sec_mask;
} tBSA_PAN_ROLE_INFO;

typedef struct {
    tBSA_PAN_ROLE role;
    tBSA_PAN_ROLE_INFO user_info;
    tBSA_PAN_ROLE_INFO gn_info;
    tBSA_PAN_ROLE_INFO nap_info;
} tBSA_PAN_SET_ROLE;

typedef struct {
    BD_ADDR bd_addr;
    tBSA_PAN_ROLE local_role;
    tBSA_PAN_ROLE peer_role;
} tBSA_PAN_OPEN;

typedef struct {
    tBSA_PAN_HNDL handle;
} tBSA_PAN_CLOSE;

typedef struct {
    tBSA_PAN_HNDL handle;
    UINT16 num_filter;
    UINT16 data[BSA_PAN_MAX_PROT_FILTERS * 2];
} tBSA_PAN_PFILT;

typedef struct {
    tBSA_PAN_HNDL handle;
    UINT16 num_filter;
    BD_ADDR data[BSA_PAN_MAX_MULTI_FILTERS * 2];
} tBSA_PAN_MFILT;


/* PAN UIPC header */
typedef struct {
    UINT16 evt;
    UINT16 len;
} tBSA_PAN_UIPC_HDR;

/* PAN UIPC events */
typedef enum {
    BSA_PAN_UIPC_DATA_EVT,              /* Data indication. */
} tBSA_PAN_UIPC_EVT;

/* Structure associated with BSA_PAN_UIPC_DATA_EVT */
typedef struct {
    tBSA_PAN_HNDL handle;
    UINT8 app_id;
    BD_ADDR src;
    BD_ADDR dst;
    UINT16 protocol;
    BOOLEAN ext;
    BOOLEAN forward;
    UINT8 data;
} tBSA_PAN_UIPC_DATA_MSG;

/* Union of all UIPC callback structures */
typedef union {
    tBSA_PAN_UIPC_DATA_MSG data;
} tBSA_PAN_UIPC_MSG_BDY;

typedef struct {
    tBSA_PAN_UIPC_HDR hdr;
    tBSA_PAN_UIPC_MSG_BDY bdy;
} tBSA_PAN_UIPC_MSG;


/* PAN UIPC requests */
typedef enum {
    BSA_PAN_UIPC_DATA_CMD,              /* Data request. */
} tBSA_PAN_UIPC_CMD;

/* Structure associated with BSA_PAN_UIPC_DATA_CMD */
typedef struct {
    tBSA_PAN_HNDL handle;
    BD_ADDR src;
    BD_ADDR dst;
    UINT16 protocol;
    BOOLEAN ext;
    UINT8 data;
} tBSA_PAN_UIPC_DATA_REQ;

/* Union of all UIPC request structures */
typedef union {
    tBSA_PAN_UIPC_DATA_REQ data;
} tBSA_PAN_UIPC_REQ_BDY;

typedef struct {
    tBSA_PAN_UIPC_HDR hdr;
    tBSA_PAN_UIPC_REQ_BDY bdy;
} tBSA_PAN_UIPC_REQ;


/*****************************************************************************
 **  External Function Declarations
 *****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 **
 ** Function         BSA_PanEnable
 **
 ** Description      Enable the PAN service.  This function must be called
 **                  before any other functions in the PAN API are called.
 **                  When the enable operation is complete this function
 **                  will return BSA_SUCCESS.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanEnable(tBSA_PAN_ENABLE *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanEnableInit
 **
 ** Description      Initialize the tBSA_PAN_ENABLE structure to default values.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanEnableInit(tBSA_PAN_ENABLE *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanDisable
 **
 ** Description      Disable the PAN service.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanDisable(tBSA_PAN_DISABLE *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanDisableInit
 **
 ** Description      Initialize the tBSA_PAN_DISABLE structure to default
 **                  values.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanDisableInit(tBSA_PAN_DISABLE *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanSetRole
 **
 ** Description      Set the supported Personal Area Network roles.
 **                  When the enable operation is complete this function
 **                  will return BSA_SUCCESS and role is set to p_req->role.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanSetRole(tBSA_PAN_SET_ROLE *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanSetRoleInit
 **
 ** Description      Initialize the tBSA_PAN_SET_ROLE structure to default
 **                  values.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanSetRoleInit(tBSA_PAN_SET_ROLE *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanOpen
 **
 ** Description      Open a PAN connection to a remote device.  After the
 **                  connection is open, the callback function is called
 **                  with a BSA_PAN_OPEN_EVT.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanOpen(tBSA_PAN_OPEN *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanOpenInit
 **
 ** Description      Initialize the tBSA_PAN_OPEN structure to default values.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanOpenInit(tBSA_PAN_OPEN *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanClose
 **
 ** Description      Close an active PAN connection.  After the connection is
 **                  closed, the callback function is called with a
 **                  BSA_PAN_CLOSE_EVT.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanClose(tBSA_PAN_CLOSE *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanCloseInit
 **
 ** Description      Initialize the tBSA_PAN_CLOSE structure to default values.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanCloseInit(tBSA_PAN_CLOSE *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanSetPFilter
 **
 ** Description      Set protocol filters.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanSetPFilter(tBSA_PAN_PFILT *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanSetPFilterInit
 **
 ** Description      Initialize the tBSA_PAN_PFILT structure to default values.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanSetPFilterInit(tBSA_PAN_PFILT *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanSetMFilter
 **
 ** Description      Set multicast filters.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanSetMFilter(tBSA_PAN_MFILT *p_req);

/******************************************************************************
 **
 ** Function         BSA_PanSetPMilterInit
 **
 ** Description      Initialize the tBSA_PAN_MFILT structure to default values.
 **
 ** Returns          Status
 **
 ******************************************************************************/
extern tBSA_STATUS BSA_PanSetMFilterInit(tBSA_PAN_MFILT *p_req);

#ifdef __cplusplus
}
#endif

#endif /* BSA_PAN_API_H */
