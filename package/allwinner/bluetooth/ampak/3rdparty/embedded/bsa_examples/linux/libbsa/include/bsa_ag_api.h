/*****************************************************************************
 **
 **  Name:                  bsa_ag_api.h
 **
 **  Description:        This is the public interface file for the Headset part of
 **                      the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_AG_API_H
#define BSA_AG_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"
#include "bta_ag_api.h"

/*****************************************************************************
**  Constants and Type Definitions
*****************************************************************************/
#ifndef BSA_AG_DEBUG
#define BSA_AG_DEBUG    FALSE
#endif

/* HS feature masks */
#define BSA_AG_FEAT_3WAY    BTA_AG_FEAT_3WAY     /* Three-way calling */
#define BSA_AG_FEAT_ECNR    BTA_AG_FEAT_ECNR     /* Echo cancellation and/or noise reduction */
#define BSA_AG_FEAT_VREC    BTA_AG_FEAT_VREC     /* Voice recognition */
#define BSA_AG_FEAT_INBAND  BTA_AG_FEAT_INBAND   /* In-band ring tone */
#define BSA_AG_FEAT_VTAG    BTA_AG_FEAT_VTAG     /* Attach a phone number to a voice tag */
#define BSA_AG_FEAT_REJECT  BTA_AG_FEAT_REJECT   /* Ability to reject incoming call */
#define BSA_AG_FEAT_ECS     BTA_AG_FEAT_ECS      /* Enhanced Call Status */
#define BSA_AG_FEAT_ECC     BTA_AG_FEAT_ECC      /* Enhanced Call Control */
#define BSA_AG_FEAT_EXTERR  BTA_AG_FEAT_EXTERR   /* Extended error codes */
#define BSA_AG_FEAT_CODEC   BTA_AG_FEAT_CODEC    /* Codec Negotiation */
#define BSA_AG_FEAT_VOIP    BTA_AG_FEAT_VOIP     /* VoIP call */

/* Proprietary features: using 31 ~ 16 bits */
#define BSA_AG_FEAT_UNAT    BTA_AG_FEAT_UNAT   /* Pass unknown AT commands to application */
#define BSA_AG_FEAT_NOSCO   BTA_AG_FEAT_NOSCO  /* No SCO control performed by BTA AG */

typedef UINT16  tBSA_AG_FEAT;

/* HFP peer features */
#define BSA_AG_PEER_FEAT_ECNR       BTA_AG_PEER_FEAT_ECNR    /* Echo cancellation and/or noise reduction */
#define BSA_AG_PEER_FEAT_3WAY       BTA_AG_PEER_FEAT_3WAY    /* Call waiting and three-way calling */
#define BSA_AG_PEER_FEAT_CLI        BTA_AG_PEER_FEAT_CLI     /* Caller ID presentation capability */
#define BSA_AG_PEER_FEAT_VREC       BTA_AG_PEER_FEAT_VREC    /* Voice recognition activation */
#define BSA_AG_PEER_FEAT_VOL        BTA_AG_PEER_FEAT_VOL     /* Remote volume control */
#define BSA_AG_PEER_FEAT_ECS        BTA_AG_PEER_FEAT_ECS     /* Enhanced Call Status */
#define BSA_AG_PEER_FEAT_ECC        BTA_AG_PEER_FEAT_ECC     /* Enhanced Call Control */
#define BSA_AG_PEER_FEAT_CODEC      BTA_AG_PEER_FEAT_CODEC   /* Codec Negotiation */
#define BSA_AG_PEER_FEAT_VOIP       BTA_AG_PEER_FEAT_VOIP    /* VoIP call */

typedef UINT16 tBSA_AG_PEER_FEAT;

/* HFP peer supported codec mask */
typedef tBTA_AG_PEER_CODEC tBSA_AG_PEER_CODEC;

/* ASCII charcter string of arguments to the AT command or result */
#define BSA_AG_AT_MAX_LEN           BTA_AG_AT_MAX_LEN

#define BSA_AG_SERVICE_NAME_LEN_MAX     150
#define BSA_AG_NUM_PROFILES             2
#define BSA_AG_APP_ID                   3  /* BTUI_DM_SCO_4_AG_APP_ID */

#define BSA_SCO_ROUTE_PCM               0   /* HCI_BRCM_SCO_ROUTE_PCM */
#define BSA_SCO_ROUTE_HCI               1   /* HCI_BRCM_SCO_ROUTE_HCI */

/* Reserved handle to report bad handle values to client */
#define BSA_AG_BAD_HANDLE               0xFF

/* data type for BTA_AgCommand() */
typedef union
{
    char            str[BSA_AG_AT_MAX_LEN+1];
    UINT16          num;
} tBSA_AG_CMD_DATA;


/* BSA HS callback events */
typedef enum
{
    /* AG callback events */
    BSA_AG_OPEN_EVT           , /* AG connection open */
    BSA_AG_CLOSE_EVT          , /* AG connection closed */
    BSA_AG_CONN_EVT           , /* Service level connection opened */
    BSA_AG_AUDIO_OPEN_EVT     , /* Audio connection open */
    BSA_AG_AUDIO_CLOSE_EVT    , /* Audio connection closed */
    BSA_AG_WBS_EVT            , /* WBS */
    BSA_AG_SPK_EVT            , /* Speaker volume changed */
    BSA_AG_MIC_EVT            , /* Microphone volume changed */
    BSA_AG_AT_CKPD_EVT        , /* CKPD from the HS */

    /* Values below are for HFP only (order must match BTA_AG_AT_*_EVT) */
    BSA_AG_AT_A_EVT          , /* Answer a call */
    BSA_AG_AT_D_EVT          , /* Place a call using number or memory dial */
    BSA_AG_AT_CHLD_EVT       , /* Call hold */
    BSA_AG_AT_CHUP_EVT       , /* Hang up a call */
    BSA_AG_AT_CIND_EVT       , /* Read indicator settings */
    BSA_AG_AT_VTS_EVT        , /* Transmit DTMF tone */
    BSA_AG_AT_BINP_EVT       , /* Retrieve number from voice tag */
    BSA_AG_AT_BLDN_EVT       , /* Place call to last dialed number */
    BSA_AG_AT_BVRA_EVT       , /* Enable/disable voice recognition */
    BSA_AG_AT_NREC_EVT       , /* Disable echo canceling */
    BSA_AG_AT_CNUM_EVT       , /* Retrieve subscriber number */
    BSA_AG_AT_BTRH_EVT       , /* CCAP-style incoming call hold */
    BSA_AG_AT_CLCC_EVT       , /* Query list of current calls */
    BSA_AG_AT_COPS_EVT       , /* Query list of current calls */
    BSA_AG_AT_UNAT_EVT       , /* Unknown AT command */
    BSA_AG_AT_CBC_EVT        , /* Battery Level report from HF */
    BSA_AG_AT_BAC_EVT        , /* Codec select */
    BSA_AG_AT_BCS_EVT          /* Codec select */
} tBSA_AG_EVT;

/* callback event data */
typedef struct
{
    UINT16                          hndl;
    tBSA_STATUS                     status;
} tBSA_AG_HDR_MSG;

/* callback event data */
typedef struct
{
    UINT16                          hndl;
    char                            str[BSA_AG_AT_MAX_LEN+1];   /* AT command response string argument */
    UINT16                          num;                        /* Name of device requesting access */
    UINT8                           idx;                        /* call number used by CLCC and CHLD */
} tBSA_AG_VAL_MSG;

/* callback event data */
typedef struct
{
    UINT16                          hndl;
    BD_ADDR                         bd_addr;
    tBSA_STATUS                     status;     /* status of connection */
    tBSA_SERVICE_ID                 service_id; /* profile used for connection */
} tBSA_AG_OPEN_MSG;

/* callback event data */
typedef struct
{
    UINT16                          hndl;
    BD_ADDR                         bd_addr;
    tBSA_STATUS                     status;     /* status of connection */
    tBSA_SERVICE_MASK               service;    /* profile used for connection */
    tBSA_AG_PEER_FEAT               peer_features;
} tBSA_AG_CONN_MSG;

/* callback event data */
typedef struct
{
    UINT16                          hndl;
    tBSA_STATUS                     status;     /* status of connection */
} tBSA_AG_CLOSE_MSG;

/* union of data associated with HS callback */
typedef union
{
    tBSA_AG_HDR_MSG         hdr;
    tBSA_AG_OPEN_MSG        open;
    tBSA_AG_VAL_MSG         val;
    tBSA_AG_CONN_MSG        conn;
    tBSA_AG_CLOSE_MSG       close;
} tBSA_AG_MSG;

/* BSA HS callback function */
typedef void (tBSA_AG_CBACK) (tBSA_AG_EVT event, tBSA_AG_MSG *p_data);


#ifndef BSA_AG_MEDIA_MAX_BUF_LEN
#define BSA_AG_MEDIA_MAX_BUF_LEN  240
#endif

/* data associated with UIPC media data */
typedef struct
{
    BT_HDR                          hdr;
    UINT8                           multimedia[BSA_AG_MEDIA_MAX_BUF_LEN];
} tBSA_AG_MEDIA;

/*
* Structures used to pass parameters to BSA API functions
*/

typedef struct
{
    tBSA_AG_CBACK *p_cback;
} tBSA_AG_ENABLE;

typedef struct
{
    int dummy;
} tBSA_AG_DISABLE;

typedef struct
{
    tBSA_SERVICE_MASK  services;
    tBSA_SEC_AUTH      sec_mask;
    tBSA_AG_FEAT       features;
    char               service_name[BSA_AG_NUM_PROFILES][BSA_AG_SERVICE_NAME_LEN_MAX];
    UINT8              sco_route; /* BSA_SCO_ROUTE_HCI or BSA_SCO_ROUTE_PCM */
    tUIPC_CH_ID        uipc_channel;
    UINT16             hndl;
} tBSA_AG_REGISTER;

typedef struct
{
    UINT16 hndl;
} tBSA_AG_DEREGISTER;

typedef struct
{
    BD_ADDR            bd_addr;
    UINT16             hndl;
    tBSA_SEC_AUTH      sec_mask;
    tBSA_SERVICE_MASK  services;
} tBSA_AG_OPEN;

typedef struct
{
    UINT16 hndl;
} tBSA_AG_CLOSE;

typedef struct
{
    UINT16 hndl;
    UINT8  sco_route;
} tBSA_AG_AUDIO_OPEN;

typedef struct
{
    UINT16 hndl;
} tBSA_AG_AUDIO_CLOSE;

typedef struct
{
    UINT16              hndl;
    tBTA_AG_RES         result;
    tBTA_AG_RES_DATA    data;
} tBSA_AG_RES;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function         BSA_AgEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgEnableInit(tBSA_AG_ENABLE *p_req);

/*******************************************************************************
**
** Function         BSA_AgEnable
**
** Description      Enable the audio gateway service. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_AG_ENABLE_EVT. This function must
**                  be called before other function in the AG API are
**                  called.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_AgEnable (tBSA_AG_ENABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_AgDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgDisableInit(tBSA_AG_DISABLE *p_req);

/*******************************************************************************
**
** Function         BSA_AgDisable
**
** Description      Disable the audio gateway service
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_AgDisable(tBSA_AG_DISABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_AgRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgRegisterInit(tBSA_AG_REGISTER *p_req);

/*******************************************************************************
**
** Function         BSA_AgRegister
**
** Description      Register an Audio Gateway service.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_AgRegister (tBSA_AG_REGISTER *p_req);

/*******************************************************************************
 **
 ** Function         BSA_AgDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgDeregisterInit(tBSA_AG_DEREGISTER *p_req);

/*******************************************************************************
**
** Function         BSA_AgDeregister
**
** Description      Deregister an Audio Gateway service.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_AgDeregister(tBSA_AG_DEREGISTER *p_req);

/*******************************************************************************
 **
 ** Function         BSA_AgOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgOpenInit(tBSA_AG_OPEN *p_req);

/*******************************************************************************
**
** Function         BSA_AgOpen
**
** Description      Opens a connection to a headset or hands-free device.
**                  When connection is open callback function is called
**                  with a BSA_AG_OPEN_EVT. Only the data connection is
**                  opened. The audio connection is not opened.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_AgOpen (tBSA_AG_OPEN    *p_req);

/*******************************************************************************
 **
 ** Function         BSA_AgCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgCloseInit(tBSA_AG_CLOSE *p_req);

/*******************************************************************************
**
** Function         BSA_AgClose
**
** Description      Close the current connection to a headset or a handsfree
**                  Any current audio connection will also be closed
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_AgClose (tBSA_AG_CLOSE  *p_req);

/*******************************************************************************
 **
 ** Function         BSA_AgAudioOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgAudioOpenInit(tBSA_AG_AUDIO_OPEN *p_req);

/*******************************************************************************
**
** Function         BSA_AgAudioOpen
**
** Description      Opens an audio connection to the currently connected
**                  headset or handsfree
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_AgAudioOpen (tBSA_AG_AUDIO_OPEN *p_req);

/*******************************************************************************
 **
 ** Function         BSA_AgAudioCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgAudioCloseInit(tBSA_AG_AUDIO_CLOSE *p_req);

/*******************************************************************************
**
** Function         BSA_AgAudioClose
**
** Description      Close the currently active audio connection to a headset
**                  or hnadsfree. The data connection remains open
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_AgAudioClose (tBSA_AG_AUDIO_CLOSE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_AgResultInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgResultInit(tBSA_AG_RES *p_req);

/*******************************************************************************
**
** Function         BSA_AgResult
**
** Description      Send an AT result code to a headset or hands-free device.
**                  This function is only used when the AG parse mode is set
**                  to BTA_AG_PARSE.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_AgResult (tBSA_AG_RES      *p_req);

#ifdef __cplusplus
}
#endif

#endif
