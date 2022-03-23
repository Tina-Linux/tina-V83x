/*****************************************************************************
**
**  Name:           bta_avk_api.h
**
**  Description:    This is the public interface file for the advanced
**                  audio/video streaming (AV) subsystem of BTA, Widcomm's
**                  Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2004-2015, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AVK_API_H
#define BTA_AVK_API_H

#include "avrc_api.h"
#include "avdt_api.h"
#include "a2d_api.h"
#include "vdp_api.h"
#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* AVK status values */
#define BTA_AVK_SUCCESS          0       /* successful operation */
#define BTA_AVK_FAIL             1       /* generic failure */
#define BTA_AVK_FAIL_SDP         2       /* service not found */
#define BTA_AVK_FAIL_STREAM      3       /* stream connection failed */
#define BTA_AVK_FAIL_RESOURCES   4       /* no resources */
#define BTA_AVK_FAIL_SIG         5       /* signaling connection failed */
#define BTA_AVK_FAIL_AVRCP_CTRL  6       /* AVRCP control channel failed */

typedef UINT8 tBTA_AVK_STATUS;

/* AVK features masks */
#define BTA_AVK_FEAT_PROTECT     0x0001  /* streaming media content protection */
#define BTA_AVK_FEAT_REPORT      0x0002  /* use reporting service for VDP */
#define BTA_AVK_FEAT_DELAY_RPT   0x0004  /* use delay reporting */

typedef UINT16 tBTA_AVK_FEAT;

/* AVK channel values */
#define BTA_AVK_CHNL_AUDIO       0  /* audio channel */
#define BTA_AVK_CHNL_VIDEO       1  /* video channel */
typedef UINT8 tBTA_AVK_CHNL;

/* offset of codec type in codec info byte array */
#define BTA_AVK_CODEC_TYPE_IDX      2   /* AVDT_CODEC_TYPE_INDEX */

#ifndef BTA_AVK_MAX_SEPS
#define BTA_AVK_MAX_SEPS         2
#endif

/* maximum number of streams created: Only 1 for audio  */
#ifndef BTA_AVK_NUM_STRS
#define BTA_AVK_NUM_STRS         3                                  /* TRYING 3AVK */
#endif

/* Maximun number of AVDTP signaling and AVRCP connections */
#ifndef BTA_AVK_MAX_NUM_CONN
#define BTA_AVK_MAX_NUM_CONN        1
#endif

/* the index for the audio stream */
#define BTA_AVK_AUDIO_STR_IDX    0


#define BTA_AVK_MAX_MTU          1008

#define BTA_AVK_CO_METADATA      AVRC_CO_METADATA

/* codec type */
#define BTA_AVK_CODEC_SBC        A2D_MEDIA_CT_SBC        /* SBC media codec type */
#define BTA_AVK_CODEC_M12        A2D_MEDIA_CT_M12        /* MPEG-1, 2 Audio media codec type */
#define BTA_AVK_CODEC_M24        A2D_MEDIA_CT_M24        /* MPEG-2, 4 AAC media codec type */
#define BTA_AVK_CODEC_ATRAC      A2D_MEDIA_CT_ATRAC      /* ATRAC family media codec type */
#define BTA_AVK_CODEC_H263_P0    VDP_MEDIA_CT_H263_P0    /* H.263 baseline (profile 0) */
#define BTA_AVK_CODEC_MPEG4      VDP_MEDIA_CT_MPEG4      /* MPEG-4 Visual Simple Profile */
#define BTA_AVK_CODEC_H263_P3    VDP_MEDIA_CT_H263_P3    /* H.263 profile 3 */
#define BTA_AVK_CODEC_H263_P8    VDP_MEDIA_CT_H263_P8    /* H.263 profile 8 */
#define BTA_AVK_CODEC_VEND       VDP_MEDIA_CT_VEND       /* Non-VDP */

typedef UINT8 tBTA_AVK_CODEC;

/* Company ID in BT assigned numbers */
#define BTA_AVK_BT_VENDOR_ID     VDP_BT_VENDOR_ID        /* Broadcom Corporation */

/* vendor specific codec ID */
#define BTA_AVK_CODEC_ID_H264    VDP_CODEC_ID_H264       /* Non-VDP codec ID - H.264 */
#define BTA_AVK_CODEC_ID_IMG     VDP_CODEC_ID_IMG        /* Non-VDP codec ID - images/slideshow */

/* error codes for BTA_AvkProtectRsp */
#define BTA_AVK_ERR_NONE             A2D_SUCCESS         /* Success, no error */
#define BTA_AVK_ERR_BAD_STATE        AVDT_ERR_BAD_STATE  /* Message cannot be processed
                                                          * in this state */
#define BTA_AVK_ERR_RESOURCE         AVDT_ERR_RESOURCE   /* Insufficient resources */
#define BTA_AVK_ERR_BAD_CP_TYPE      A2D_BAD_CP_TYPE     /* The requested Content
                                                          * Protection Type is not
                                                          * supported */
#define BTA_AVK_ERR_BAD_CP_FORMAT    A2D_BAD_CP_FORMAT   /* The format of Content
                                                          * Protection Data is
                                                          * not correct */

typedef UINT8 tBTA_AVK_ERR;

/* AV callback events */
#define BTA_AVK_ENABLE_EVT       0       /* AV enabled */
#define BTA_AVK_DISABLE_EVT      1       /* AV disabled */
#define BTA_AVK_REGISTER_EVT     2       /* registered to AVDT */
#define BTA_AVK_DEREGISTER_EVT   3       /* deregistered to AVDT */
#define BTA_AVK_SIG_OPEN_EVT     4       /* A2DP signaling channel opened */
#define BTA_AVK_SIG_CLOSE_EVT    5       /* A2DP signaling channel closed */
#define BTA_AVK_STR_OPEN_EVT     6       /* A2DP streaming channel opened */
#define BTA_AVK_STR_CLOSE_EVT    7       /* A2DP streaming channel closed */
#define BTA_AVK_START_EVT        8       /* stream data transfer started */
#define BTA_AVK_STOP_EVT         9       /* stream data transfer stopped */
#define BTA_AVK_PROTECT_REQ_EVT  10       /* content protection request */
#define BTA_AVK_PROTECT_RSP_EVT  11       /* content protection response */
#define BTA_AVK_RECONFIG_EVT     12      /* reconfigure response */
#define BTA_AVK_SUSPEND_EVT      13      /* suspend response */
#define BTA_AVK_UPDATE_SEPS_EVT  14      /* update SEPS to available or unavailable */
#define BTA_AVK_RECONFIG_PENDING 15      /* reconfig pending event */
#define BTA_AVK_APP_TIMER_EVT    16      /* User timer event */ /* BSA_SPECIFC */
typedef UINT8 tBTA_AVK_EVT;


#define BTA_AVK_CLOSE_STR_CLOSED    1
#define BTA_AVK_CLOSE_CONN_LOSS     2

typedef UINT8   tBTA_AVK_CLOSE_REASON;

/* Event associated with BTA_AVK_REGISTER_EVT */
typedef struct
{
    UINT8           app_id;     /* ID associated with call to BTA_AvkRegister() */
    tBTA_AVK_STATUS  status;
} tBTA_AVK_REGISTER;

/* Event associated with BTA_AVK_UPDATE_SEPS_EVT */
typedef struct
{
    UINT16  status;
    BOOLEAN available;
} tBTA_AVK_UPDATE_SEPS;

/* data associated with BTA_AVK_SIG_OPEN_EVT */
typedef struct
{
    BD_ADDR             bd_addr;
    tBTA_AVK_STATUS     status;
    UINT8               ccb_handle;
} tBTA_AVK_SIG_OPEN;

/* data associated with BTA_AVK_SIG_CLOSE_EVT */
typedef struct
{
    BD_ADDR                 bd_addr;
    tBTA_AVK_STATUS         status;
    tBTA_AVK_CLOSE_REASON   reason;
    UINT8                   ccb_handle;
} tBTA_AVK_SIG_CLOSE;

/* data associated with BTA_AVK_STR_OPEN_EVT */
typedef struct
{
    BD_ADDR         bd_addr;
    tBTA_AVK_STATUS status;
    BOOLEAN         starting;
    UINT8           ccb_handle;
} tBTA_AVK_STR_OPEN;

/* data associated with BTA_AVK_STR_CLOSE_EVT */
typedef struct
{
    BD_ADDR         bd_addr;
    UINT8           ccb_handle;
} tBTA_AVK_STR_CLOSE;

/* data associated with BTA_AVK_START_EVT */
typedef struct
{
    tBTA_AVK_STATUS  status;
    BD_ADDR         bd_addr;
} tBTA_AVK_START;

/* data associated with BTA_AVK_SUSPEND_EVT */
typedef struct
{
    BD_ADDR          bd_addr;
    tBTA_AVK_STATUS  status;
} tBTA_AVK_SUSPEND;

/* data associated with BTA_AVK_RECONFIG_EVT */
typedef struct
{
    tBTA_AVK_STATUS  status;
} tBTA_AVK_RECONFIG;

/* data associated with BTA_AVK_PROTECT_REQ_EVT */
typedef struct
{
    UINT8           *p_data;
    UINT16          len;
} tBTA_AVK_PROTECT_REQ;

/* data associated with BTA_AVK_PROTECT_RSP_EVT */
typedef struct
{
    UINT8           *p_data;
    UINT16          len;
    tBTA_AVK_ERR     err_code;
} tBTA_AVK_PROTECT_RSP;

/* data associated with BTA_AVK_APP_TIMER_EVT */ /* BSA_SPECIFC */
typedef struct
{
    UINT32          app_data;
} tBTA_AVK_APP_TIMER_RSP;

/* union of data associated with AV callback */
typedef union
{
    tBTA_AVK_CHNL        chnl;
    tBTA_AVK_REGISTER    registr;
    tBTA_AVK_UPDATE_SEPS update_seps;
    tBTA_AVK_SIG_OPEN    sig_open;
    tBTA_AVK_SIG_CLOSE   sig_close;
    tBTA_AVK_STR_OPEN    str_open;
    tBTA_AVK_STR_CLOSE   str_close;
    tBTA_AVK_START       start;
    tBTA_AVK_PROTECT_REQ protect_req;
    tBTA_AVK_PROTECT_RSP protect_rsp;
    tBTA_AVK_RECONFIG    reconfig;
    tBTA_AVK_SUSPEND     suspend;
    tBTA_AVK_APP_TIMER_RSP   app_timer_rsp; /* BSA_SPECIFC */
} tBTA_AVK;

/* AVK callback */
typedef void (tBTA_AVK_CBACK)(tBTA_AVK_EVT event, tBTA_AVK *p_data);

/* type for stream state machine action functions */
typedef void (*tBTA_AVK_ACT)(void *p_cb, void *p_data);

/* type for registering VDP */
typedef void (tBTA_AVK_REG) (tAVDT_CS *p_cs, char *p_service_name, void *p_data);

/* AV configuration structure */
typedef struct
{
    UINT16  sig_mtu;            /* AVDTP signaling channel MTU at L2CAP */

    /* Audio channel configuration */
    UINT16  audio_mtu;          /* AVDTP audio transport channel MTU at L2CAP */
    UINT16  audio_flush_to;     /* AVDTP audio transport channel flush timeout */
    UINT16  audio_mqs;          /* AVDTP audio channel max data queue size */
    const tBTA_AVK_ACT *p_audio_act_tbl;/* the action function table for audio stream */
    tBTA_AVK_REG       *p_audio_reg;    /* action function to register audio */

} tBTA_AVK_CFG;

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/

/*******************************************************************************
**
** Function         BTA_AvkEnable
**
** Description      Enable the advanced audio/video service. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_AVK_ENABLE_EVT. This function must
**                  be called before other function in the AV API are
**                  called.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkEnable(tBTA_SEC sec_mask, tBTA_AVK_FEAT features,
                          tBTA_AVK_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_AvkDisable
**
** Description      Disable the advanced audio/video service.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkDisable(void);

/*******************************************************************************
**
** Function         BTA_AvkRegister
**
** Description      Register the audio or video service to stack. When the
**                  operation is complete the callback function will be
**                  called with a BTA_AVK_REGISTER_EVT. This function must
**                  be called before AVDT stream is open.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkRegister(const char *p_service_name,
                             UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_AvkDeregister
**
** Description      Deregister the audio or video service
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkDeregister();

/*******************************************************************************
**
** Function         BTA_AvkOpenSig
**
** Description      Opens an advanced audio signaling connection to a peer device.
**                  When connection is open callback function is called
**                  with a BTA_AVK_SIG_OPEN_EVT for each channel.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkOpenSig(BD_ADDR bd_addr, tBTA_SEC sec_mask);

/*******************************************************************************
**
** Function         BTA_AvkCloseSig
**
** Description      Close the current A2DP signaling channel. Close the A2DP streaming
**                  channel as well if there is any
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkCloseSig(UINT8 ccb_handle);

/*******************************************************************************
**
** Function         BTA_AvkCloseStr
**
** Description      Close the current A2DP streaming channel only
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkCloseStr();

/*******************************************************************************
**
** Function         BTA_AvkStart
**
** Description      Start audio/video stream data transfer.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkStart(BD_ADDR bd_addr);

/*******************************************************************************
**
** Function         BTA_AvkStop
**
** Description      Stop audio/video stream data transfer.
**                  If suspend is TRUE, this function sends AVDT suspend signal
**                  to the connected peer(s).
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkStop(BOOLEAN suspend);

/*******************************************************************************
**
** Function         BTA_AvkProtectReq
**
** Description      Send a content protection request.  This function can only
**                  be used if AV is enabled with feature BTA_AVK_FEAT_PROTECT.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkProtectReq(UINT8 *p_data, UINT16 len);

/*******************************************************************************
**
** Function         BTA_AvkProtectRsp
**
** Description      Send a content protection response.  This function must
**                  be called if a BTA_AVK_PROTECT_REQ_EVT is received.
**                  This function can only be used if AV is enabled with
**                  feature BTA_AVK_FEAT_PROTECT.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkProtectRsp(UINT8 error_code, UINT8 *p_data,
                               UINT16 len);

/*******************************************************************************
**
** Function         BTA_AvkUpdateStreamEndPoints
**
** Description      Change all the sink SEPs to available or unavailable
**                  When the update operation is complete the callback function
**                  will be called with a BTA_AVK_UPDATE_SEPS_EVT
**
** Parameter        available: True Set all SEPs to available
**                             FALSE Set all SEPs to unavailable
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkUpdateStreamEndPoints(BOOLEAN available);

/*******************************************************************************
**
** Function         BTA_AvkDelayReport
**
** Description      Send a delay report.  This function can only
**                  be used if AV is enabled with feature BTA_AVK_FEAT_DELAY_RPT.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkDelayReport(UINT16 delay);



/* BSA_SPECIFIC */
#if (defined(BTA_AVK_AV_AUDIO_RELAY) && (BTA_AVK_AV_AUDIO_RELAY==TRUE))
/*******************************************************************************
**
** Function         BTA_AvkRelayAudio
**
** Description      Relay AVK audio to AV
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkRelayAudio(BOOLEAN relay);

#endif

/* BSA_SPECIFIC */
/*******************************************************************************
**
** Function         BTA_AvkAppTimer
**
** Description      Set application timer
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_AvkAppTimer(UINT32 app_data, INT32 timout);


#ifdef __cplusplus
}
#endif

#endif /* BTA_AVK_API_H */
