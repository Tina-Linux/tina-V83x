/*****************************************************************************
 **
 **  Name:           bsa_hl_api.h
 **
 **  Description:    This is the public interface file for Health part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2010-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_HL_API_H
#define BSA_HL_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"

#include "bta_hl_api.h"

/*****************************************************************************
 **  Constants and Type Definitions
 *****************************************************************************/
#ifndef BSA_HL_DEBUG
#define BSA_HL_DEBUG    FALSE
#endif

/* Defined Data Type (from IEEE 11073 Personal Health Devices Working Group) */
typedef UINT16 tBSA_HL_DATA_TYPE;
typedef UINT16 tBSA_HL_HNDL;

/* Maximum number of HL application/service */
#define BSA_HL_APP_MAX BTA_HL_NUM_APPS

/* Application Handle (got when an application is registered) */
#define BSA_HL_BAD_APP_HANDLE   BTA_HL_INVALID_APP_HANDLE
typedef tBTA_HL_APP_HANDLE tBSA_HL_APP_HANDLE;

/* Data Link Handle */
#define BSA_HL_BAD_DATA_HANDLE   0xFF
typedef tBTA_HL_MDL_HANDLE tBSA_HL_DATA_HANDLE;

/* Control Link Handle */
#define BSA_HL_BAD_MCL_HANDLE BTA_HL_INVALID_MCL_HANDLE
typedef tBTA_HL_MCL_HANDLE tBSA_HL_MCL_HANDLE;

/* MDEP Identifier */
#define BSA_HL_MDEP_ID_BAD BTA_HL_INVALID_MDEP_ID
typedef tBTA_HL_MDEP_ID tBSA_HL_MDEP_ID;

/* MDL Identifier */
#define BSA_HL_DELETE_ALL_MDL_IDS BTA_HL_DELETE_ALL_MDL_IDS
typedef tBTA_HL_MDL_ID tBSA_HL_MDL_ID;

/* Maximum number of MDL configuration saved (per application/service) */
#define BSA_HL_NUM_MDL_CFGS  BTA_HL_NUM_MDL_CFGS

/* Maximum Length of Service Name, Service Description and Provider Name */
#define BSA_HL_SERVICE_NAME_LEN_MAX 35
#define BSA_HL_SERVICE_DESC_LEN_MAX 75
#define BSA_HL_PROVIDER_NAME_LEN_MAX 35

/* Maximum length of the description of an MDEP */
#define BSA_HL_MDEP_DESC_LEN_MAX BTA_HL_MDEP_DESP_LEN

/* Maximum number of MDEP per application */
#define BSA_HL_NUM_MDEPS_MAX BTA_HL_NUM_MDEPS

/* Maximum number of data types can be supported per MDEP */
#define BSA_HL_NUM_DATA_TYPES_MAX BTA_HL_NUM_DATA_TYPES

/* Maximum number of SDP Records (applications/services) received */
#define BSA_HL_NUM_SDP_RECS_MAX BTA_HL_NUM_SDP_RECS

/* MDEP can be either Source or Sink */
#define BSA_HL_MDEP_ROLE_SOURCE BTA_HL_MDEP_ROLE_SOURCE
#define BSA_HL_MDEP_ROLE_SINK BTA_HL_MDEP_ROLE_SINK
#define BSA_HL_MDEP_ROLE_BAD 0xFF
typedef tBTA_HL_MDEP_ROLE tBSA_HL_MDEP_ROLE;

/* Data Channel Mode (Reliable or Streaming) */
#define BSA_HL_DCH_MODE_RELIABLE BTA_HL_DCH_MODE_RELIABLE
#define BSA_HL_DCH_MODE_STREAMING BTA_HL_DCH_MODE_STREAMING
#define BSA_HL_DCH_MODE_UNKNOWN 0xFF
typedef UINT8 tBSA_HL_DCH_MODE;

/* Data Channel Configuration*/
#define BSA_HL_DCH_CFG_NO_PREF BTA_HL_DCH_CFG_NO_PREF
#define BSA_HL_DCH_CFG_RELIABLE BTA_HL_DCH_CFG_RELIABLE
#define BSA_HL_DCH_CFG_STREAMING BTA_HL_DCH_CFG_STREAMING
typedef UINT8 tBSA_HL_DCH_CFG;

#define BSA_HL_OPEN_RSP_SUCCESS     0   /* Accept to open HL Data link */
#define BSA_HL_OPEN_RSP_CFG_REJ     1   /* Refuse to open HL data link */
typedef UINT8 tBSA_HL_OPEN_RSP_CODE;

/* MCAP Feature definitions */
#define BSA_HL_FEATURE_RECONNECT_INIT   0x02    /* Reconnect Initiator Feature */
#define BSA_HL_FEATURE_RECONNECT_ACPT   0x04    /* Reconnect Acceptor Feature */
typedef UINT8 tBSA_HL_FEATURE;


/* BSA HL callback events */
typedef enum
{
    BSA_HL_OPEN_EVT,            /* Data Link Opened Event */
    BSA_HL_RECONNECT_EVT,       /* Data Link Reconnected Event */
    BSA_HL_CLOSE_EVT,           /* Data Link Closed Event */
    BSA_HL_SEND_DATA_CFM_EVT,   /* Send Data Confirm Event */
    BSA_HL_OPEN_REQ_EVT,        /* Data Link Open Request Event (peer initiated)*/
    BSA_HL_SDP_QUERY_EVT,       /* SDP Query Response Event */
    BSA_HL_SAVE_MDL_EVT,        /* Save MDL Event */
    BSA_HL_DELETE_MDL_EVT,      /* Delete MDL Event */
} tBSA_HL_EVT;

typedef struct
{
    UINT8 time; /* last save timestamp */
    UINT16 mtu; /* MTU negotiated */
    tBSA_HL_MDL_ID mdl_id; /* MDL Identification */
    tBSA_HL_MDEP_ID local_mdep_id; /* Local MDEP ID */
    tBSA_HL_MDEP_ROLE local_mdep_role; /* Role of this MDEP */
    BOOLEAN active; /* true if this item is in use */
    tBSA_HL_DCH_MODE dch_mode; /* DataConnection mode */
    UINT8 fcs; /* fcs */
    BD_ADDR peer_bd_addr; /* BdAddr of peer device */
} tBSA_HL_MDL_CFG;

/* This structure contains the configuration of one Data type supported by */
/* a specific MDEP to use for an application registration */
typedef struct
{
    tBSA_HL_DATA_TYPE data_type; /* IEEE Dev Specialization (Oximeter, Thermometer, etc.) */
    UINT16 max_rx_apdu_size; /* local rcv MTU */
    UINT16 max_tx_apdu_size; /* maximum TX APDU size*/
    char desc[BSA_HL_MDEP_DESC_LEN_MAX + 1]; /* Description */
} tBSA_HL_MDEP_DATA_TYPE_CFG;

/* This structure contains the configuration on the MDEPs to use for an application registration */
typedef struct
{
    tBSA_HL_MDEP_ROLE mdep_role; /* Source or Sink */
    UINT8 num_of_mdep_data_types; /* Number of Data types supported */
    tBSA_HL_MDEP_ID mdep_id; /* OUT Parameter: MDEP ID allocated */
    tBSA_HL_MDEP_DATA_TYPE_CFG data_cfg[BSA_HL_NUM_DATA_TYPES_MAX]; /* Data Types supported */
} tBSA_HL_MDEP_CFG_REG;


typedef struct
{
    UINT16 ctrl_psm; /* L2CAP Control PSM */
    tBSA_HL_FEATURE mcap_sup_features; /* MCAP Features supported */
    UINT8 num_mdeps; /* number of mdep elements from SDP */
    char service_name[BSA_HL_SERVICE_NAME_LEN_MAX + 1];
    char service_desc[BSA_HL_SERVICE_DESC_LEN_MAX + 1];
    char provider_name[BSA_HL_PROVIDER_NAME_LEN_MAX + 1];
    tBSA_HL_MDEP_CFG_REG mdep[BSA_HL_NUM_MDEPS_MAX]; /* End Points table */
} tBSA_HL_SDP_REC;

typedef struct
{
    tBSA_STATUS status; /* Status of the SDP Query */
    BD_ADDR bd_addr; /* BdAddr of Peer Device */
    UINT8 num_records; /* Number of records (Services/application) received */
    tBSA_HL_SDP_REC sdp_records[BSA_HL_NUM_SDP_RECS_MAX];
} tBSA_HL_SDP_QUERY_MSG;

typedef struct
{
    tBSA_STATUS status; /* Status of the Open event */
    BD_ADDR bd_addr; /* BdAddr of Peer Device */
    tBSA_HL_APP_HANDLE app_handle; /* Application Handle */
    tBSA_HL_DATA_HANDLE data_handle; /* Data Handle to control this Data Link */
    tBSA_HL_MDEP_ID local_mdep_id; /* Local MDEP ID connected */
    tBSA_HL_DCH_MODE channel_mode; /* Reliable or Streaming */
    tUIPC_CH_ID uipc_channel; /* UIPC channel to carry HL data */
    UINT16 mtu; /* Peer receiver MTU (Maximum Transmit Unit) size */
    tBSA_HL_MCL_HANDLE mcl_handle; /* Control Handle */
    tBSA_HL_MDL_ID mdl_id; /* MCAP data link ID for this Data connection */
} tBSA_HL_OPEN_MSG;

typedef struct
{
    BD_ADDR bd_addr; /* BdAddr of Peer Device */
    tBSA_HL_APP_HANDLE app_handle; /* Application Handle */
    tBSA_HL_MDEP_ID local_mdep_id; /* Local MDEP ID requested to be connected */
    tBSA_HL_DCH_CFG channel_cfg; /* No Pref, Reliable or Streaming */
    tBSA_HL_MCL_HANDLE mcl_handle; /* mcl_handle (to be used in BSA_HlOpenRsp API) */
    tBSA_HL_MDL_ID mdl_id; /* mdl_id (to be used in BSA_HlOpenRsp API) */
} tBSA_HL_OPEN_REQ_MSG;

typedef struct
{
    tBSA_STATUS status; /* Status of the Close event */
    BD_ADDR bd_addr; /* BdAddr of Peer Device */
    tBSA_HL_DATA_HANDLE data_handle; /* Data Handle of the closed Data Link */
    tUIPC_CH_ID uipc_channel; /* UIPC channel to carry HL data */
    BOOLEAN intentional; /* TRUE if the close is intentional (requested)
                            FALSE if the close is unintentional (link error) */
} tBSA_HL_CLOSE_MSG;

typedef tBSA_HL_OPEN_MSG tBSA_HL_RECONNECT_MSG;

typedef struct
{
    tBSA_STATUS status; /* Status of the Send event */
    tBSA_HL_APP_HANDLE app_handle; /* Application Handle */
    tBSA_HL_DATA_HANDLE data_handle; /* Data Handle of the sended Data Link */
    tBSA_HL_MCL_HANDLE mcl_handle; /* mcl_handle */
    tUIPC_CH_ID uipc_channel; /* UIPC channel to carry HL data */
} tBSA_HL_SEND_DATA_CFM_MSG;

typedef struct
{
    tBSA_HL_APP_HANDLE app_handle; /* Application Handle */
    UINT8 mdl_index; /* Index of this MDL */
    tBSA_HL_MDL_CFG mdl_cfg; /* MDL configuration to save */
} tBSA_HL_SAVE_MDL_MSG;

typedef struct
{
    tBSA_STATUS status; /* Status */
    tBSA_HL_APP_HANDLE app_handle; /* Application Handle of the Deleted MDL */
    tBSA_HL_MCL_HANDLE mcl_handle; /* Control Handle of the Deleted MDL */
    tBSA_HL_MDL_ID mdl_id; /* MDL_ID of the Deleted MDL */
} tBSA_HL_DELETE_MDL_MSG;

/* Union of all HL messages passed as parameter of Application HL Callback*/
typedef union
{
    tBSA_HL_OPEN_MSG open; /* tBSA_HL_OPEN_EVT */
    tBSA_HL_CLOSE_MSG close; /* tBSA_HL_CLOSE_EVT */
    tBSA_HL_OPEN_REQ_MSG open_req; /* tBSA_HL_OPEN_REQ_EVT */
    tBSA_HL_RECONNECT_MSG reconnect; /* tBSA_HL_RECONNECT_EVT */
    tBSA_HL_SDP_QUERY_MSG sdp_query; /* tBSA_HL_SDP_QUERY_EVT */
    tBSA_HL_SEND_DATA_CFM_MSG send_data_cfm; /* tBSA_HL_SEND_DATA_CFM_EVT */
    tBSA_HL_SAVE_MDL_MSG save_mdl; /* BSA_HL_SAVE_MDL_EVT */
    tBSA_HL_DELETE_MDL_MSG delete_mdl; /* BSA_HL_DELETE_MDL_EVT */
} tBSA_HL_MSG;

typedef void tBSA_HL_CBACK(tBSA_HL_EVT event, tBSA_HL_MSG *p_data);

/*
 * Structures use to pass parameters to BSA API functions
 */
typedef struct
{
    tBSA_HL_CBACK *p_cback; /* Health Application callback */
    tBSA_SEC_AUTH sec_mask; /* security mask for peer initiated connections */
} tBSA_HL_ENABLE;

typedef struct
{
    UINT8 dummy; /* No parameter needed, but some compiler may need this field */
} tBSA_HL_DISABLE;

/* This structure contains the parameters used to register an application */
typedef struct
{
    tBSA_HL_APP_HANDLE app_handle; /* OUT Parameter */
    tBSA_SEC_AUTH sec_mask; /* security mask */
    char service_name[BSA_HL_SERVICE_NAME_LEN_MAX];
    char service_desc[BSA_HL_SERVICE_DESC_LEN_MAX];
    char provider_name[BSA_HL_PROVIDER_NAME_LEN_MAX];
    UINT8 num_of_mdeps; /* Number of MDEPs (default:TRUE)*/
    BOOLEAN advertize_source; /* Indicates if Peer sink can see our source MDEPs */
    tBSA_HL_MDEP_CFG_REG mdep[BSA_HL_NUM_MDEPS_MAX]; /* End Points table */
    tBSA_HL_MDL_CFG saved_mdl[BSA_HL_NUM_MDL_CFGS]; /* Saved MDL */
} tBSA_HL_REGISTER;

/* This structure contains the parameters used to Deregister an application */
typedef struct
{
    tBSA_HL_APP_HANDLE app_handle; /* app_handle to Deregister */
} tBSA_HL_DEREGISTER;

/* This structure contains the parameters used to Query a peer device */
typedef struct
{
    BD_ADDR bd_addr; /* BdAddr of Peer Device */
} tBSA_HL_SDP_QUERY;

/* This structure contains the parameters used to Open a Data Link to a peer device */
typedef struct
{
    tBSA_HL_APP_HANDLE app_handle; /* Which local Registered, Application to use */
    BD_ADDR bd_addr; /* BdAddr of Peer Device */
    UINT16 ctrl_psm; /* Control PSM (retrieved during SDP Request) */
    tBSA_SEC_AUTH sec_mask; /* security mask for initiating connection*/
    tBSA_HL_MDEP_ID peer_mdep_id; /* mdep id to connect to */
    tBSA_HL_MDEP_ID local_mdep_id; /* mdep id to connect to */
    tBSA_HL_DCH_CFG channel_cfg;
} tBSA_HL_OPEN;

/* This structure contains the parameters used to Response to an Open Data Link from a peer device */
typedef struct
{
    tBSA_HL_APP_HANDLE app_handle; /* Application Handle (received in OpenReq message) */
    tBSA_HL_MCL_HANDLE mcl_handle; /* mcl_handle (received in OpenReq message) */
    BD_ADDR bd_addr; /* BdAddr of Peer Device (received in OpenReq message)*/
    tBSA_HL_MDEP_ID local_mdep_id; /* mdep id (retrieved from OpenReq message) */
    tBSA_HL_DCH_CFG channel_cfg; /* Channel configuration */
    tBSA_HL_MDL_ID mdl_id; /* mdl_id (received in OpenReq message) */
    tBSA_HL_OPEN_RSP_CODE response_code; /* Accept/Refuse */
} tBSA_HL_OPEN_RSP;

/* This structure contains the parameters used to close a Data Link */
typedef struct
{
    tBSA_HL_DATA_HANDLE data_handle; /* Data Handle of the Data Link to close */
} tBSA_HL_CLOSE;

/* This structure contains the parameters used to Reconnect a Data Link to a peer device */
typedef struct
{
    tBSA_HL_APP_HANDLE app_handle; /* Which local Registered, Application to use */
    BD_ADDR bd_addr; /* BdAddr of Peer Device */
    UINT16 ctrl_psm; /* Control PSM (retrieved during SDP Request) */
    tBSA_SEC_AUTH sec_mask; /* security mask for initiating connection*/
    tBSA_HL_MDL_ID mdl_id; /* The MDL Id to reconnect */
} tBSA_HL_RECONNECT;

/* This structure contains the parameters used to Delete a MCL */
typedef struct
{
    tBSA_HL_MCL_HANDLE mcl_handle; /* Control handle */
    tBSA_HL_MDL_ID mdl_id; /* The MDL Id to Delete */
} tBSA_HL_DELETE_MDL;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function         BSA_HlEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlEnableInit(tBSA_HL_ENABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlEnable
 **
 ** Description      This function enable Health profile
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlEnable(tBSA_HL_ENABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlDisableInit(tBSA_HL_DISABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlDisable
 **
 ** Description      This function Disable Health profile
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlDisable(tBSA_HL_DISABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlRegisterInit(tBSA_HL_REGISTER *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlRegister
 **
 ** Description      This function is used to Register an Health application
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlRegister(tBSA_HL_REGISTER *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlDeregisterInit(tBSA_HL_DEREGISTER *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlDeregister
 **
 ** Description      This function is used to Deregister an Health application
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlDeregister(tBSA_HL_DEREGISTER *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlSdpQueryInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlSdpQueryInit(tBSA_HL_SDP_QUERY *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlSdpQuery
 **
 ** Description      This function is used to Query (SDP) an Health peer device
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlSdpQuery(tBSA_HL_SDP_QUERY *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlOpenInit(tBSA_HL_OPEN *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlOpen
 **
 ** Description      This function is used to Open a Data Link to a peer Health device
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlOpen(tBSA_HL_OPEN *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlOpenRspInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlOpenRspInit(tBSA_HL_OPEN_RSP *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlOpen
 **
 ** Description      This function is used to Respond to a Data Link Open Request from
 **                  a peer Health device
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlOpenRsp(tBSA_HL_OPEN_RSP *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlCloseInit(tBSA_HL_CLOSE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlOpen
 **
 ** Description      This function is used to close a Data Link
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlClose(tBSA_HL_CLOSE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlReconnectInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlReconnectInit(tBSA_HL_RECONNECT *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlReconnect
 **
 ** Description      This function is used to Reconnect a Data Link
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlReconnect(tBSA_HL_RECONNECT *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlDeleteMdlInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlDeleteMdlInit(tBSA_HL_DELETE_MDL *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HlDeleteMdl
 **
 ** Description      This function is used to Delete a Data Link
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlDeleteMdl(tBSA_HL_DELETE_MDL *p_req);

#ifdef __cplusplus
}
#endif

#endif
