/*****************************************************************************
**
**  Name:           bta_av_co.h
**
**  Description:    This is the interface file for advanced audio/video call-out
**                  functions.
**
**  Copyright (c) 2005 - 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AV_CO_H
#define BTA_AV_CO_H

#include "l2c_api.h"
#include "bta_av_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* BSA_SPECIFIC */
/* TRUE to use SCMS-T content protection */
#ifndef BTA_AV_CO_CP_SCMS_T
#define BTA_AV_CO_CP_SCMS_T     FALSE
#endif

/* the content protection IDs assigned by BT SIG */
#define BTA_AV_CP_SCMS_T_ID     0x0002
#define BTA_AV_CP_DTCP_ID       0x0001

#define BTA_AV_CP_LOSC                  2
#define BTA_AV_CP_INFO_LEN              3


#define BTA_AV_CP_SCMS_COPY_ALLOWED     0x00
#define BTA_AV_CP_SCMS_COPY_ONCE        0x10
#define BTA_AV_CP_SCMS_COPY_PROHIBITED  0x11

#define BTA_AV_CP_SCMS_COPY_FREE        0x12
#define BTA_AV_CP_SCMS_COPY_NEVER       0x13

#if (BTU_MULTI_AV_INCLUDED == TRUE)
#define BTA_AV_CO_DEFAULT_AUDIO_OFFSET      (L2CAP_MULTI_AV_HCI_HDR_LEN_WITH_PADDING + \
                            (L2CAP_MULTI_AV_L2C_HDR_LEN * BTA_AV_NUM_STRS) + AVDT_MEDIA_HDR_SIZE)
#else
#define BTA_AV_CO_DEFAULT_AUDIO_OFFSET      AVDT_MEDIA_OFFSET
#endif

enum
{
    BTA_AV_CO_ST_INIT,
    BTA_AV_CO_ST_IN,
    BTA_AV_CO_ST_OUT,
    BTA_AV_CO_ST_OPEN,
    BTA_AV_CO_ST_STREAM
};

/* data type for the Video Codec Information Element*/
typedef struct
{
    UINT8   codec_type;     /* Codec type */
    UINT8   levels;         /* level mask */
} tBTA_AV_VIDEO_CFG;

/* data type for the Audio Codec Information*/
typedef struct
{
    UINT16  bit_rate;       /* SBC encoder bit rate in kbps */
    UINT16  bit_rate_busy;  /* SBC encoder bit rate in kbps */
    UINT16  bit_rate_swampd;/* SBC encoder bit rate in kbps */
    UINT8   busy_level;     /* Busy level indicating the bit-rate to be used */
    UINT8   codec_info[AVDT_CODEC_SIZE];
    UINT8   codec_type;     /* Codec type */
} tBTA_AV_AUDIO_CODEC_INFO;

/*******************************************************************************
**
** Function         bta_av_co_audio_init
**
** Description      This callout function is executed by AV when it is
**                  started by calling BTA_AvEnable().  This function can be
**                  used by the phone to initialize audio paths or for other
**                  initialization purposes.
**
**
** Returns          Stream codec and content protection capabilities info.
**
*******************************************************************************/
BTA_API extern BOOLEAN bta_av_co_audio_init(UINT8 *p_codec_type, UINT8 *p_codec_info,
                                   UINT8 *p_num_protect, UINT8 *p_protect_info, UINT8 index);

/*******************************************************************************
**
** Function         bta_av_co_video_init
**
** Description      This callout function is executed by AV when it is
**                  started by calling BTA_AvEnable().  This function can be
**                  used by the phone to initialize video paths or for other
**                  initialization purposes.
**
**
** Returns          Stream codec and content protection capabilities info.
**
*******************************************************************************/
BTA_API extern BOOLEAN bta_av_co_video_init(UINT8 *p_codec_type, UINT8 *p_codec_info,
                                   UINT8 *p_num_protect, UINT8 *p_protect_info, UINT8 index);

/*******************************************************************************
**
** Function         bta_av_co_audio_disc_res
**
** Description      This callout function is executed by AV to report the
**                  number of stream end points (SEP) were found during the
**                  AVDT stream discovery process.
**
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_disc_res(tBTA_AV_HNDL hndl, UINT8 num_seps,
                                             UINT8 num_snk, BD_ADDR addr);

/*******************************************************************************
**
** Function         bta_av_co_video_disc_res
**
** Description      This callout function is executed by AV to report the
**                  number of stream end points (SEP) were found during the
**                  AVDT stream discovery process.
**
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_av_co_video_disc_res(tBTA_AV_HNDL hndl, UINT8 num_seps,
                                             UINT8 num_snk, BD_ADDR addr);

/*******************************************************************************
**
** Function         bta_av_co_audio_getconfig
**
** Description      This callout function is executed by AV to retrieve the
**                  desired codec and content protection configuration for the
**                  audio stream.
**
**
** Returns          Stream codec and content protection configuration info.
**
*******************************************************************************/
BTA_API extern UINT8 bta_av_co_audio_getconfig(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
                                         UINT8 *p_codec_info, UINT8 *p_sep_info_idx, UINT8 seid,
                                         UINT8 *p_num_protect, UINT8 *p_protect_info);

/*******************************************************************************
**
** Function         bta_av_co_video_getconfig
**
** Description      This callout function is executed by AV to retrieve the
**                  desired codec and content protection configuration for the
**                  video stream.
**
**
** Returns          Stream codec and content protection configuration info.
**
*******************************************************************************/
BTA_API extern UINT8 bta_av_co_video_getconfig(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
                                         UINT8 *p_codec_info, UINT8 *p_sep_info_idx, UINT8 seid,
                                         UINT8 *p_num_protect, UINT8 *p_protect_info);

/*******************************************************************************
**
** Function         bta_av_co_audio_setconfig
**
** Description      This callout function is executed by AV to set the
**                  codec and content protection configuration of the audio stream.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_setconfig(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
                                        UINT8 *p_codec_info, UINT8 seid, BD_ADDR addr,
                                        UINT8 num_protect, UINT8 *p_protect_info);

/*******************************************************************************
**
** Function         bta_av_co_video_setconfig
**
** Description      This callout function is executed by AV to set the
**                  codec and content protection configuration of the video stream.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_video_setconfig(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
                                        UINT8 *p_codec_info, UINT8 seid, BD_ADDR addr,
                                        UINT8 num_protect, UINT8 *p_protect_info);

/*******************************************************************************
**
** Function         bta_av_co_audio_open
**
** Description      This function is called by AV when the audio stream connection
**                  is opened.
**                  BTA-AV maintains the MTU of A2DP streams.
**                  If this is the 2nd audio stream, mtu is the smaller of the 2
**                  streams.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_open(tBTA_AV_HNDL hndl,
                                         tBTA_AV_CODEC codec_type, UINT8 *p_codec_info,
                                         UINT16 mtu);

/*******************************************************************************
**
** Function         bta_av_co_video_open
**
** Description      This function is called by AV when the video stream connection
**                  is opened.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_video_open(tBTA_AV_HNDL hndl,
                                         tBTA_AV_CODEC codec_type, UINT8 *p_codec_info,
                                         UINT16 mtu);

/*******************************************************************************
**
** Function         bta_av_co_audio_close
**
** Description      This function is called by AV when the audio stream connection
**                  is closed.
**                  BTA-AV maintains the MTU of A2DP streams.
**                  When one stream is closed and no other audio stream is open,
**                  mtu is reported as 0.
**                  Otherwise, the MTU remains open is reported.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_close(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
                                          UINT16 mtu);

/*******************************************************************************
**
** Function         bta_av_co_video_close
**
** Description      This function is called by AV when the video stream connection
**                  is closed.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_video_close(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
                                          UINT16 mtu);

/*******************************************************************************
**
** Function         bta_av_co_audio_start
**
** Description      This function is called by AV when the audio streaming data
**                  transfer is started.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_start(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
        UINT8 *p_codec_info, BOOLEAN *p_no_rtp_hdr);

/*******************************************************************************
**
** Function         bta_av_co_video_start
**
** Description      This function is called by AV when the video streaming data
**                  transfer is started.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_video_start(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
        UINT8 *p_codec_info, BOOLEAN *p_no_rtp_hdr);

/*******************************************************************************
**
** Function         bta_av_co_audio_stop
**
** Description      This function is called by AV when the audio streaming data
**                  transfer is stopped.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_stop(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type);

/*******************************************************************************
**
** Function         bta_av_co_video_stop
**
** Description      This function is called by AV when the video streaming data
**                  transfer is stopped.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_video_stop(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type);

/*******************************************************************************
**
** Function         bta_av_co_audio_src_data_path
**
** Description      This function is called to get the next data buffer from
**                  the audio codec
**
** Returns          NULL if data is not ready.
**                  Otherwise, a GKI buffer (BT_HDR*) containing the audio data.
**
*******************************************************************************/
BTA_API extern void * bta_av_co_audio_src_data_path(tBTA_AV_CODEC codec_type,
                                                    UINT32 *p_len, UINT32 *p_timestamp);

/*******************************************************************************
**
** Function         bta_av_co_video_src_data_path
**
** Description      This function is called to get the next data buffer from
**                  the video codec.
**
** Returns          NULL if data is not ready.
**                  Otherwise, a video data buffer (UINT8*).
**
*******************************************************************************/
BTA_API extern void * bta_av_co_video_src_data_path(tBTA_AV_CODEC codec_type,
                                                    UINT32 *p_len, UINT32 *p_timestamp);

/*******************************************************************************
**
** Function         bta_av_co_audio_drop
**
** Description      An Audio packet is dropped. .
**                  It's very likely that the connected headset with this handle
**                  is moved far away. The implementation may want to reduce
**                  the encoder bit rate setting to reduce the packet size.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_drop(tBTA_AV_HNDL hndl);

/*******************************************************************************
**
** Function         bta_av_co_video_report_conn
**
** Description      This function is called by AV when the reporting channel is
**                  opened (open=TRUE) or closed (open=FALSE).
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_video_report_conn (BOOLEAN open, UINT8 avdt_handle);

/*******************************************************************************
**
** Function         bta_av_co_video_report_rr
**
** Description      This function is called by AV when a Receiver Report is
**                  received
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_video_report_rr (UINT32 packet_lost);

/*******************************************************************************
**
** Function         bta_av_co_audio_delay
**
** Description      This function is called by AV when the audio stream connection
**                  needs to send the initial delay report to the connected SRC.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_delay(tBTA_AV_HNDL hndl, UINT16 delay);

/*******************************************************************************
**
** Function         bta_av_co_video_delay
**
** Description      This function is called by AV when the video stream connection
**                  needs to send the initial delay report to the connected SRC.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_video_delay(tBTA_AV_HNDL hndl, UINT16 delay);

/*******************************************************************************
**
** Function         bta_av_co_audio_cp
**
** Description      This function is called by AV when the audio stream connection
**                  needs to check the content protection capability.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern UINT8 bta_av_co_audio_cp(tBTA_AV_HNDL hndl, UINT8 *p_num_protect, UINT8 *p_protect_info);

/*******************************************************************************
**
** Function         bta_av_co_video_cp
**
** Description      This function is called by AV when the video stream connection
**                  needs to check the content protection capability.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern UINT8 bta_av_co_video_cp(tBTA_AV_HNDL hndl, UINT8 *p_num_protect, UINT8 *p_protect_info);


#if (BTU_DUAL_STACK_MM_INCLUDED == TRUE)
/*******************************************************************************
**
** Function         bta_av_co_audio_getcodec
**
** Description
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_getcodec (tBTA_AV_HNDL hndl,
                                              tBTA_AV_AUDIO_CODEC_INFO *p_cfg);

/*******************************************************************************
**
** Function         bta_av_co_audio_prestart
**
** Description      This function is called from BTA when it recieves the
**                  avdtp start-indication from the peer. This callout is
**                  meant for the app to do switching if needed. Once the
**                  app has completed the processing, it should send a
**                  start-ok event to BTA to actually start the audio
**                  streaming.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_starting (tBTA_AV_HNDL hndl);

/*******************************************************************************
**
** Function         bta_av_co_update_ai_state
**
** Description
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_update_ai_state(UINT8 from_state, UINT8 to_state);

#endif

#if (BTU_STACK_LITE_ENABLED == TRUE)
/*******************************************************************************
**
** Function         bta_av_co_audio_setcodec
**
** Description
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_setcodec (tBTA_AV_HNDL hndl,
                                              tBTA_AV_AUDIO_CODEC_INFO p_cfg);

/*******************************************************************************
**
** Function         bta_av_co_audio_setmtu
**
** Description
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_co_audio_setmtu (UINT16 mtu);
#endif

#endif /* BTA_AV_CO_H */
