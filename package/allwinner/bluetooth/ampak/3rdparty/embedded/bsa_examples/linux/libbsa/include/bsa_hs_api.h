/*****************************************************************************
 **
 **  Name:           bsa_hs_api.h
 **
 **  Description:    This is the public interface file for the Headset part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_HS_API_H
#define BSA_HS_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"

#include "bta_hs_api.h"

/*****************************************************************************
**  Constants and Type Definitions
*****************************************************************************/
#ifndef BSA_HS_DEBUG
#define BSA_HS_DEBUG    FALSE
#endif

/* HS feature masks */
#define BSA_HS_FEAT_ECNR  BTA_HS_FEAT_ECNR   /* Echo cancellation and/or noise reduction */
#define BSA_HS_FEAT_3WAY  BTA_HS_FEAT_3WAY   /* Call waiting and three-way calling */
#define BSA_HS_FEAT_CLIP  BTA_HS_FEAT_CLIP   /* Caller ID presentation capability  */
#define BSA_HS_FEAT_VREC  BTA_HS_FEAT_VREC   /* Voice recoginition activation capability  */
#define BSA_HS_FEAT_RVOL  BTA_HS_FEAT_RVOL   /* Remote volume control capability  */
#define BSA_HS_FEAT_ECS   BTA_HS_FEAT_ECS    /* Enhanced Call Status  */
#define BSA_HS_FEAT_ECC   BTA_HS_FEAT_ECC    /* Enhanced Call Control  */
#define BSA_HS_FEAT_CODEC BTA_HS_FEAT_CODEC  /* Codec negotiation */
#define BSA_HS_FEAT_VOIP  BTA_HS_FEAT_VOIP   /* VoIP call */
#define BSA_HS_FEAT_UNAT  BTA_HS_FEAT_UNAT   /* Pass unknown AT command responses to application */

typedef UINT16  tBSA_HS_FEAT;

/* peer feature (AG) mask values */
#define BSA_HS_PEER_FEAT_3WAY   BTA_HS_PEER_FEAT_3WAY    /* Three-way calling */
#define BSA_HS_PEER_FEAT_ECNR   BTA_HS_PEER_FEAT_ECNR    /* Echo cancellation and/or noise reduction */
#define BSA_HS_PEER_FEAT_VREC   BTA_HS_PEER_FEAT_VREC    /* Voice recognition */
#define BSA_HS_PEER_FEAT_INBAND BTA_HS_PEER_FEAT_INBAND  /* In-band ring tone */
#define BSA_HS_PEER_FEAT_VTAG   BTA_HS_PEER_FEAT_VTAG    /* Attach a phone number to a voice tag */
#define BSA_HS_PEER_FEAT_REJECT BTA_HS_PEER_FEAT_REJECT  /* Ability to reject incoming call */
#define BSA_HS_PEER_FEAT_ECS    BTA_HS_PEER_FEAT_ECS     /* Enhanced call status */
#define BSA_HS_PEER_FEAT_ECC    BTA_HS_PEER_FEAT_ECC     /* Enhanced call control */
#define BSA_HS_PEER_FEAT_EERC   BTA_HS_PEER_FEAT_EERC    /* Extended error result codes */
#define BSA_HS_PEER_FEAT_CODEC  BTA_HS_PEER_FEAT_CODEC   /* Codec Negotiation */
#define BSA_HS_PEER_FEAT_VOIP   BTA_HS_PEER_FEAT_VOIP    /* VoIP call */

typedef UINT16 tBSA_HS_PEER_FEAT;

/* HS settings */
typedef struct
{
    UINT8          spk_vol;
    UINT8          mic_vol;
    BOOLEAN        ecnr_enabled;
} tBSA_HS_SETTINGS;

/* ASCII charcter string of arguments to the AT command or result */
#define BSA_HS_AT_MAX_LEN           BTA_HS_AT_MAX_LEN

#define BSA_HS_SERVICE_NAME_LEN_MAX     150
#define BSA_HS_NUM_PROFILES             2
#define BSA_HS_APP_ID                   3  /* BTUI_DM_SCO_4_HS_APP_ID */

/* data type for BTA_HsCommand() */
typedef tBTA_HS_CMD_DATA tBSA_HS_CMD_DATA;

/* HS commands used in BSA_HsCommand() */
#define BSA_HS_SPK_CMD   BTA_HS_SPK_CMD   /* Update speaker volume */
#define BSA_HS_MIC_CMD   BTA_HS_MIC_CMD   /* Update microphone volume */
#define BSA_HS_CKPD_CMD  BTA_HS_CKPD_CMD  /* Key stroke command */
#define BSA_HS_A_CMD     BTA_HS_A_CMD     /* Answer incoming call */
#define BSA_HS_BINP_CMD  BTA_HS_BINP_CMD  /* Retrieve number from voice tag */
#define BSA_HS_BVRA_CMD  BTA_HS_BVRA_CMD  /* Enable/Disable voice recoginition */
#define BSA_HS_BLDN_CMD  BTA_HS_BLDN_CMD  /* Last Number redial */
#define BSA_HS_CHLD_CMD  BTA_HS_CHLD_CMD  /* Call hold command */
#define BSA_HS_CHUP_CMD  BTA_HS_CHUP_CMD  /* Call hang up command */
#define BSA_HS_CIND_CMD  BTA_HS_CIND_CMD  /* Read Indicator Status */
#define BSA_HS_CNUM_CMD  BTA_HS_CNUM_CMD  /* Retrieve Subscriber number */
#define BSA_HS_D_CMD     BTA_HS_D_CMD     /* Place a call using a number or memory dial */
#define BSA_HS_NREC_CMD  BTA_HS_NREC_CMD  /* Disable Noise reduction and echo cancelling in AG */
#define BSA_HS_VTS_CMD   BTA_HS_VTS_CMD   /* Transmit DTMF tone */
#define BSA_HS_BTRH_CMD  BTA_HS_BTRH_CMD  /* CCAP incoming call hold */
#define BSA_HS_COPS_CMD  BTA_HS_COPS_CMD  /* Query operator selection */
#define BSA_HS_CMEE_CMD  BTA_HS_CMEE_CMD  /* Enable/disable extended AG result codes */
#define BSA_HS_CLCC_CMD  BTA_HS_CLCC_CMD  /* Query list of current calls in AG */
#define BSA_HS_BCC_CMD   BTA_HS_BCC_CMD   /* Bluetooth Codec Connection  */
#define BSA_HS_BCS_CMD   BTA_HS_BCS_CMD   /* Bluetooth Codec Selection  */
#define BSA_HS_BAC_CMD   BTA_HS_BAC_CMD   /* Bluetooth Available Codecs */
#define BSA_HS_UNAT_CMD  BTA_HS_UNAT_CMD  /* Transmit AT command not in the spec  */
#define BSA_HS_MAX_CMD   BTA_HS_MAX_CMD   /* For command validataion */

typedef UINT8 tBSA_HS_CMD_CODE;

/* BSA HS callback events */
typedef enum
{
    BSA_HS_OPEN_EVT,                    /* HS connection open or connection attempt failed  */
    BSA_HS_CLOSE_EVT,                   /* HS connection closed */
    BSA_HS_CONN_EVT,                    /* HS Service Level Connection is UP */
    BSA_HS_CONN_LOSS_EVT,               /* Link loss of connection to audio gateway happened */
    BSA_HS_AUDIO_OPEN_REQ_EVT,          /* Audio open request*/
    BSA_HS_AUDIO_OPEN_EVT,              /* Audio connection open */
    BSA_HS_AUDIO_CLOSE_EVT,             /* Audio connection closed */

    BSA_HS_CIND_EVT  = BTA_HS_CIND_EVT, /* Indicator string from AG */
    BSA_HS_CIEV_EVT  = BTA_HS_CIEV_EVT, /* Indicator status from AG */
    BSA_HS_RING_EVT  = BTA_HS_RING_EVT, /* RING alert from AG */
    BSA_HS_CLIP_EVT  = BTA_HS_CLIP_EVT, /* Calling subscriber information from AG */
    BSA_HS_BSIR_EVT  = BTA_HS_BSIR_EVT, /* In band ring tone setting */
    BSA_HS_BVRA_EVT  = BTA_HS_BVRA_EVT, /* Voice recognition activation/deactivation */
    BSA_HS_CCWA_EVT  = BTA_HS_CCWA_EVT, /* Call waiting notification */
    BSA_HS_CHLD_EVT  = BTA_HS_CHLD_EVT, /* Call hold and multi party service in AG */
    BSA_HS_VGM_EVT   = BTA_HS_VGM_EVT,  /* MIC volume setting */
    BSA_HS_VGS_EVT   = BTA_HS_VGS_EVT,  /* Speaker volume setting */
    BSA_HS_BINP_EVT  = BTA_HS_BINP_EVT, /* Input data response from AG */
    BSA_HS_BTRH_EVT  = BTA_HS_BTRH_EVT, /* CCAP incoming call hold */
    BSA_HS_CNUM_EVT  = BTA_HS_CNUM_EVT, /* Subscriber number */
    BSA_HS_COPS_EVT  = BTA_HS_COPS_EVT, /* Operator selection info from AG */
    BSA_HS_CMEE_EVT  = BTA_HS_CMEE_EVT, /* Enhanced error result from AG */
    BSA_HS_CLCC_EVT  = BTA_HS_CLCC_EVT, /* Current active call list info */
    BSA_HS_UNAT_EVT  = BTA_HS_UNAT_EVT, /* AT command response fro AG which is not specified in HFP or HSP */
    BSA_HS_OK_EVT    = BTA_HS_OK_EVT,   /* OK response */
    BSA_HS_ERROR_EVT = BTA_HS_ERROR_EVT,/* ERROR response */
    BSA_HS_BCS_EVT   = BTA_HS_BCS_EVT,  /* Codec selection from AG */
} tBSA_HS_EVT;

#define BSA_HS_CLOSE_CLOSED        1
#define BSA_HS_CLOSE_CONN_LOSS     2

#define BSA_SCO_CODEC_NONE      BTM_SCO_CODEC_NONE
#define BSA_SCO_CODEC_CVSD      BTM_SCO_CODEC_CVSD
#define BSA_SCO_CODEC_MSBC      BTM_SCO_CODEC_MSBC

/* callback event data */
typedef struct
{
        UINT16                          handle;
        tBSA_STATUS                     status;
} tBSA_HS_HDR_MSG;

/* callback event data */
typedef struct
{
        UINT16                          handle;
        char                            str[BSA_HS_AT_MAX_LEN+1]; /* AT command response string argument */
        UINT16                          num;    /* Name of device requesting access */
} tBSA_HS_VAL_MSG;

/* callback event data */
typedef struct
{
    UINT16                          handle;
    BD_ADDR                         bd_addr;
    tBSA_STATUS                     status;     /* status of connection */
    tBSA_SERVICE_MASK               service;    /* profile used for connection */
    BOOLEAN                         initiator;  /* connection initiator,local TRUE peer FALSE*/
} tBSA_HS_OPEN_MSG;

/* callback event data */
typedef struct
{
        UINT16                          handle;
        BD_ADDR                         bd_addr;
        tBSA_STATUS                     status;     /* status of connection */
        tBSA_SERVICE_MASK               service;    /* profile used for connection */
        tBSA_HS_PEER_FEAT               peer_features;
} tBSA_HS_CONN_MSG;

/* union of data associated with HS callback */
typedef union
{
    tBSA_HS_HDR_MSG         hdr;
    tBSA_HS_OPEN_MSG        open;
    tBSA_HS_VAL_MSG         val;
    tBSA_HS_CONN_MSG        conn;
} tBSA_HS_MSG;

/* BSA HS callback function */
typedef void (tBSA_HS_CBACK) (tBSA_HS_EVT event, tBSA_HS_MSG *p_data);


#ifndef BSA_HS_MEDIA_MAX_BUF_LEN
#define BSA_HS_MEDIA_MAX_BUF_LEN  240
#endif

/* data associated with UIPC media data */
typedef struct
{
        BT_HDR                          hdr;
        UINT8                           multimedia[BSA_HS_MEDIA_MAX_BUF_LEN];
} tBSA_HS_MEDIA;

/*
* Structures used to pass parameters to BSA API functions
*/

typedef struct
{
        tBSA_HS_CBACK *p_cback;
} tBSA_HS_ENABLE;

typedef struct
{
        int dummy;
} tBSA_HS_DISABLE;

typedef struct
{
        tBSA_SERVICE_MASK  services;
        tBSA_SEC_AUTH      sec_mask;
        tBSA_HS_FEAT       features;
        tBSA_HS_SETTINGS   settings;
        char               service_name[BSA_HS_NUM_PROFILES][BSA_HS_SERVICE_NAME_LEN_MAX];
        UINT8              sco_route;
        UINT16             hndl;
        tUIPC_CH_ID        uipc_channel;
} tBSA_HS_REGISTER;

typedef struct
{
        UINT16 hndl;
} tBSA_HS_DEREGISTER;

typedef struct
{
        BD_ADDR            bd_addr;
        UINT16             hndl;
        tBSA_SEC_AUTH      sec_mask;
        tBSA_SERVICE_MASK  services;
} tBSA_HS_OPEN;

typedef struct
{
        UINT16 hndl;
} tBSA_HS_CLOSE;

typedef struct
{
        UINT16 hndl;
        UINT8  sco_route;
} tBSA_HS_AUDIO_OPEN;

typedef struct
{
        UINT16 hndl;
} tBSA_HS_AUDIO_CLOSE;

typedef struct
{
        UINT16                     hndl;
        tBSA_HS_CMD_CODE           command;
        tBSA_HS_CMD_DATA           data;
} tBSA_HS_COMMAND;

typedef struct
{
        UINT16 dummy;
} tBSA_HS_CANCEL;


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


/*******************************************************************************
 **
 ** Function         BSA_HsEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsEnableInit(tBSA_HS_ENABLE *p_enable);

/*******************************************************************************
**
** Function         BSA_HsEnable
**
** Description      This function enables Headset profile.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsEnable (tBSA_HS_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function         BSA_HsDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsDisableInit(tBSA_HS_DISABLE *p_disable);

/*******************************************************************************
**
** Function         BSA_HsDisable
**
** Description      This function is called when the host is about power down.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsDisable(tBSA_HS_DISABLE *p_disable);

/*******************************************************************************
 **
 ** Function         BSA_HsRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsRegisterInit(tBSA_HS_REGISTER *p_register);

/*******************************************************************************
**
** Function         BSA_HsRegister
**
** Description      This function registers the Headset service(s) with
**                  the lower layers.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsRegister (tBSA_HS_REGISTER *p_register);

/*******************************************************************************
 **
 ** Function         BSA_HsDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsDeregisterInit(tBSA_HS_DEREGISTER *p_register);

/*******************************************************************************
**
** Function         BSA_HsDeregister
**
** Description      This function is called to deregister the headset service.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsDeregister(tBSA_HS_DEREGISTER *p_register);

/*******************************************************************************
 **
 ** Function         BSA_HsOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsOpenInit(tBSA_HS_OPEN *p_open);

/*******************************************************************************
**
** Function         BSA_HsOpen
**
** Description      This function is called to open a service level
**                  connection to an audio gateway.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsOpen (tBSA_HS_OPEN    *p_open);

/*******************************************************************************
 **
 ** Function         BSA_HsCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsCloseInit(tBSA_HS_CLOSE *p_close);

/*******************************************************************************
**
** Function         BSA_HsClose
**
** Description      This function is called to close a service level
**                  connection to an audio gateway.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsClose (tBSA_HS_CLOSE  *p_close);

/*******************************************************************************
 **
 ** Function         BSA_HsAudioOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsAudioOpenInit(tBSA_HS_AUDIO_OPEN *p_audio_open);

/*******************************************************************************
**
** Function         BSA_HsAudioOpen
**
** Description      This function is called to open audio on a service level
**                  connection to an audio gateway.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsAudioOpen (tBSA_HS_AUDIO_OPEN *p_audio_open);

/*******************************************************************************
 **
 ** Function         BSA_HsAudioCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsAudioCloseInit(tBSA_HS_AUDIO_CLOSE *p_audio_close);

/*******************************************************************************
**
** Function         BSA_HsAudioClose
**
** Description      This function is called to close audio on a service level
**                  connection to an audio gateway.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsAudioClose (tBSA_HS_AUDIO_CLOSE *p_audio_close);

/*******************************************************************************
 **
 ** Function         BSA_HsCommandInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsCommandInit(tBSA_HS_COMMAND *p_command);

/*******************************************************************************
**
** Function         BSA_HsCommand
**
** Description      This function is called to send an AT command to
**                  an audio gateway.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsCommand (tBSA_HS_COMMAND      *p_command);

/*******************************************************************************
 **
 ** Function         BSA_HsCancelInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsCancelInit(tBSA_HS_CANCEL *pCancel);

/*******************************************************************************
**
** Function         BSA_HsCancel
**
** Description      This function is called to cancel connection
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_HsCancel (tBSA_HS_CANCEL    *pCancel);


#ifdef __cplusplus
}
#endif

#endif
