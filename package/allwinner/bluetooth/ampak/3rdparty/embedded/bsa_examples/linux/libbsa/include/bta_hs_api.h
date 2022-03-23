/*****************************************************************************
**
**  Name:           bta_hs_api.h
**
**  Description:    This is the public interface file for the headset
**                  (HS) subsystem of BTA, Broadcom's Bluetooth application
**                  layer for audio devices.
**
**  Copyright (c) 2000-2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_HS_API_H
#define BTA_HS_API_H

#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* HS feature masks */
#define BTA_HS_FEAT_ECNR        0x0001   /* Echo cancellation and/or noise reduction */
#define BTA_HS_FEAT_3WAY        0x0002   /* Call waiting and three-way calling */
#define BTA_HS_FEAT_CLIP        0x0004   /* Caller ID presentation capability  */
#define BTA_HS_FEAT_VREC        0x0008   /* Voice recoginition activation capability  */
#define BTA_HS_FEAT_RVOL        0x0010   /* Remote volume control capability  */
#define BTA_HS_FEAT_ECS         0x0020   /* Enhanced Call Status  */
#define BTA_HS_FEAT_ECC         0x0040   /* Enhanced Call Control  */
#define BTA_HS_FEAT_CODEC       0x0080   /* Codec negotiation */
#define BTA_HS_FEAT_VOIP        0x0100   /* VoIP call */
#define BTA_HS_FEAT_UNAT        0x1000   /* Pass unknown AT command responses to application */

typedef UINT16 tBTA_HS_FEAT;

/* HS settings */
typedef struct
{
    UINT8          spk_vol;
    UINT8          mic_vol;
    BOOLEAN        ecnr_enabled;
} tBTA_HS_SETTINGS;


/* HS open status */
#define BTA_HS_SUCCESS          0 /* Connection successfully opened */
#define BTA_HS_FAIL_SDP         1 /* Open failed due to SDP */
#define BTA_HS_FAIL_RFCOMM      2 /* Open failed due to RFCOMM */
#define BTA_HS_FAIL_CONN_TOUT   3 /* Link loss occured due to connection timeout */

typedef UINT8 tBTA_HS_STATUS;

/* special service value used with APIs */
#define BTA_HS_SERVICE_UNKNOWN  0xFF

/* special handle values used with APIs */
#define BTA_HS_HANDLE_NONE      0
#define BTA_HS_HANDLE_ALL       0xFFFF


/* Audio status */
#define BTA_HS_AUDIO_STATE_OPEN           0 /* audio opened */
#define BTA_HS_AUDIO_STATE_CLOSE          1 /* audio closed */
#define BTA_HS_AUDIO_STATE_CONNECTING     2 /* audio connection will be opened */
#define BTA_HS_AUDIO_STATE_SETUP          3 /* Setting up audio connection */

typedef UINT8 tBTA_HS_AUDIO_STATE;

/* peer feature (AG) mask values */
#define BTA_HS_PEER_FEAT_3WAY    0x0001     /* Three-way calling */
#define BTA_HS_PEER_FEAT_ECNR    0x0002     /* Echo cancellation and/or noise reduction */
#define BTA_HS_PEER_FEAT_VREC    0x0004     /* Voice recognition */
#define BTA_HS_PEER_FEAT_INBAND  0x0008     /* In-band ring tone */
#define BTA_HS_PEER_FEAT_VTAG    0x0010     /* Attach a phone number to a voice tag */
#define BTA_HS_PEER_FEAT_REJECT  0x0020     /* Ability to reject incoming call */
#define BTA_HS_PEER_FEAT_ECS     0x0040     /* Enhanced call status */
#define BTA_HS_PEER_FEAT_ECC     0x0080     /* Enhanced call control */
#define BTA_HS_PEER_FEAT_EERC    0x0100     /* Extended error result codes */
#define BTA_HS_PEER_FEAT_CODEC   0x0200     /* Codec Negotiation */
#define BTA_HS_PEER_FEAT_VOIP    0x0400     /* VoIP call */

typedef UINT16 tBTA_HS_PEER_FEAT;

/* HS commands used in BTA_HsCommand() */
#define BTA_HS_SPK_CMD              0   /* Update speaker volume */
#define BTA_HS_MIC_CMD              1   /* Update microphone volume */
#define BTA_HS_CKPD_CMD             2   /* Key stroke command */
#define BTA_HS_A_CMD                3   /* Answer incoming call */
#define BTA_HS_BINP_CMD             4   /* Retrieve number from voice tag */
#define BTA_HS_BVRA_CMD             5   /* Enable/Disable voice recoginition */
#define BTA_HS_BLDN_CMD             6   /* Last Number redial */
#define BTA_HS_CHLD_CMD             7   /* Call hold command */
#define BTA_HS_CHUP_CMD             8   /* Call hang up command */
#define BTA_HS_CIND_CMD             9   /* Read Indicator Status */
#define BTA_HS_CNUM_CMD             10  /* Retrieve Subscriber number */
#define BTA_HS_D_CMD                11  /* Place a call using a number or memory dial */
#define BTA_HS_NREC_CMD             12  /* Disable Noise reduction and echo cancelling in AG */
#define BTA_HS_VTS_CMD              13  /* Transmit DTMF tone */
#define BTA_HS_BTRH_CMD             14  /* CCAP incoming call hold */
#define BTA_HS_COPS_CMD             15  /* Query operator selection */
#define BTA_HS_CMEE_CMD             16  /* Enable/disable extended AG result codes */
#define BTA_HS_CLCC_CMD             17  /* Query list of current calls in AG */
#define BTA_HS_BCC_CMD              18  /* Bluetooth Codec Connection  */
#define BTA_HS_BCS_CMD              19  /* Bluetooth Codec Selection  */
#define BTA_HS_BAC_CMD              20  /* Bluetooth Available Codecs */
#define BTA_HS_BIA_CMD              21  /* Activate/Deactivate indicators */
#define BTA_HS_UNAT_CMD             22  /* Transmit AT command not in the spec  */
#define BTA_HS_MAX_CMD              23  /* For command validataion */

typedef UINT8 tBTA_HS_CMD;


/* ASCII charcter string of arguments to the AT command or result */
#define BTA_HS_AT_MAX_LEN           255


/* data type for BTA_HsCommand() */
typedef union
{
    char            str[BTA_HS_AT_MAX_LEN+1];
    UINT16          num;
} tBTA_HS_CMD_DATA;

/* HS callback events */
#define BTA_HS_ENABLE_EVT           0  /* HS enabled */
#define BTA_HS_DISABLE_EVT          1  /* HS Disabled */
#define BTA_HS_REGISTER_EVT         2  /* HS Registered */
#define BTA_HS_DEREGISTER_EVT       3  /* HS Registered */
#define BTA_HS_OPEN_EVT             4 /* HS connection open or connection attempt failed  */
#define BTA_HS_CLOSE_EVT            5 /* HS connection closed */
#define BTA_HS_CONN_EVT             6 /* HS Service Level Connection is UP */
#define BTA_HS_CONN_LOSS_EVT        7 /* Link loss of connection to audio gateway happened */
#define BTA_HS_AUDIO_OPEN_REQ_EVT   8 /* Audio open request*/
#define BTA_HS_AUDIO_OPEN_EVT       9 /* Audio connection open */
#define BTA_HS_AUDIO_CLOSE_EVT      10/* Audio connection closed */
#define BTA_HS_CIND_EVT             11/* Indicator string from AG */
#define BTA_HS_CIEV_EVT             12/* Indicator status from AG */
#define BTA_HS_RING_EVT             13/* RING alert from AG */
#define BTA_HS_CLIP_EVT             14/* Calling subscriber information from AG */
#define BTA_HS_BSIR_EVT             15/* In band ring tone setting */
#define BTA_HS_BVRA_EVT             16/* Voice recognition activation/deactivation */
#define BTA_HS_CCWA_EVT             17/* Call waiting notification */
#define BTA_HS_CHLD_EVT             18/* Call hold and multi party service in AG */
#define BTA_HS_VGM_EVT              19/* MIC volume setting */
#define BTA_HS_VGS_EVT              20/* Speaker volume setting */
#define BTA_HS_BINP_EVT             21/* Input data response from AG */
#define BTA_HS_BTRH_EVT             22/* CCAP incoming call hold */
#define BTA_HS_CNUM_EVT             23/* Subscriber number */
#define BTA_HS_COPS_EVT		        24/* Operator selection info from AG */
#define BTA_HS_CMEE_EVT		        25/* Enhanced error result from AG */
#define BTA_HS_CLCC_EVT             26/* Current active call list info */
#define BTA_HS_UNAT_EVT             27/* AT command response fro AG which is not specified in HFP or HSP */
#define BTA_HS_OK_EVT               28 /* OK response */
#define BTA_HS_ERROR_EVT            29 /* ERROR response */
#define BTA_HS_BCS_EVT              30 /* Codec selection from AG */
typedef UINT8 tBTA_HS_EVT;

/* data associated with most non-AT events */
typedef struct
{
    UINT16              handle;
    UINT8               app_id;
} tBTA_HS_HDR;

/* data associated with BTA_HS_OPEN_EVT */
typedef struct
{
    tBTA_HS_HDR         hdr;
    BD_ADDR             bd_addr;
    tBTA_SERVICE_ID     service;
    tBTA_HS_STATUS      status;
} tBTA_HS_OPEN;

/* data associated with AT command response event */
typedef struct
{
    tBTA_HS_HDR         hdr;
    char                str[BTA_HS_AT_MAX_LEN+1];
    UINT16              num;
} tBTA_HS_VAL;

/* data associated with BTA_HS_OPEN_EVT */
typedef struct
{
     tBTA_HS_HDR         hdr;
     tBTA_HS_PEER_FEAT   peer_features;
} tBTA_HS_CONN;

/* union of data associated with HS callback */
typedef union
{
    tBTA_HS_HDR         hdr;
    tBTA_HS_OPEN        open;
    tBTA_HS_VAL         val;
    tBTA_HS_CONN        conn;
} tBTA_HS;

/* HS callback */
typedef void (tBTA_HS_CBACK)(tBTA_HS_EVT event, tBTA_HS *p_data);



/* HS configuration structure */
typedef struct
{
    UINT16       sco_pkt_types;
} tBTA_HS_CFG;

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/

/*******************************************************************************
**
** Function         BTA_HsEnable
**
** Description      Enable the headset service. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_HS_ENABLE_EVT. This function must
**                  be called before other function in the HS API are
**                  called.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HsEnable(tBTA_HS_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_HsDisable
**
** Description      Disable the headset service
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HsDisable(void);



/*******************************************************************************
**
** Function         BTA_HsRegister
**
** Description      Register an Headset Device service.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HsRegister(tBTA_SERVICE_MASK services, tBTA_SEC sec_mask,
                 tBTA_HS_FEAT features, tBTA_HS_SETTINGS * p_settings, char *p_service_names[], UINT8 app_id);


/*******************************************************************************
**
** Function         BTA_HsDeregister
**
** Description      Deregister an headset service.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HsDeregister(UINT16 handle);


/*******************************************************************************
**
** Function         BTA_HsOpen
**
** Description      Opens a connection to a audio gateway.
**                  When connection is open callback function is called
**                  with a BTA_HS_OPEN_EVT. Only the data connection is
**                  opened. The audio connection is not opened.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HsOpen(UINT16 handle, BD_ADDR bd_addr, tBTA_SEC sec_mask, tBTA_SERVICE_MASK services);

/*******************************************************************************
**
** Function         BTA_HsClose
**
** Description      Close the current connection to a headset or a handsfree
**                  Any current audio connection will also be closed
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HsClose(UINT16 handle);

/*******************************************************************************
**
** Function         BTA_HsAudioOpen
**
** Description      Opens an audio connection to the currently connected
**                  audio gateway specified by the handle
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HsAudioOpen(UINT16 handle);

/*******************************************************************************
**
** Function         BTA_HsAudioClose
**
** Description      Close the currently active audio connection to a audio
**                  gateway specified by the handle. The data connection remains open
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HsAudioClose(UINT16 handle);

/*******************************************************************************
**
** Function         BTA_HsAudioOpenResp
**
** Description      Response to an audio connection request from peer
**
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HsAudioOpenResp(UINT16 handle, BOOLEAN accept);

/*******************************************************************************
**
** Function         BTA_HsSetAudioConnPriority
**
** Description      Sets the audio connection corresponding to the handle as the
**                  priority connection. This will result in the audio from the
**                  connection getting played.
**
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HsSetAudioConnPriority(UINT16 handle);


/*******************************************************************************
**
** Function         BTA_HsCommand
**
** Description      Send an AT command code to the audio gateway specified by the handle
**
**
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HsCommand(UINT16 handle, tBTA_HS_CMD command, tBTA_HS_CMD_DATA *p_data);

#ifdef __cplusplus
}
#endif

#endif /* BTA_HS_API_H */
