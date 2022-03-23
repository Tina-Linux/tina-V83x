/*****************************************************************************
**
**  Name:           bta_avk_co.h
**
**  Description:    This is the interface file for advanced audio/video call-out
**                  functions.
**
**  Copyright (c) 2003-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AVK_CO_H
#define BTA_AVK_CO_H

#include "bta_avk_api.h"

enum
{
    BTA_AVK_CO_ST_INIT,
    BTA_AVK_CO_ST_IN,
    BTA_AVK_CO_ST_OUT,
    BTA_AVK_CO_ST_OPEN,
    BTA_AVK_CO_ST_STREAM
};

/* TRUE to use SCMS-T content protection */
#ifndef BTA_AVK_CO_CP_SCMS_T
#define BTA_AVK_CO_CP_SCMS_T     FALSE
#endif

/* the content protection IDs assigned by BT SIG */
#define BTA_AVK_CP_SCMS_T_ID     0x0002
#define BTA_AVK_CP_DTCP_ID       0x0001
#define BTA_AVK_CP_NONE_ID       0x0000

#define BTA_AVK_CP_LOSC                     2
#define BTA_AVK_CP_INFO_LEN                 3

#define BTA_AVK_CP_SCMS_COPY_ALLOWED     0x00
#define BTA_AVK_CP_SCMS_COPY_ONCE        0x10
#define BTA_AVK_CP_SCMS_COPY_PROHIBITED  0x11


/*******************************************************************************
**
** Function         bta_avk_co_audio_init
**
** Description      This callout function is executed by AV when it is
**                  started by calling BTA_AvkEnable().  This function can be
**                  used by the phone to initialize audio paths or for other
**                  initialization purposes.
**
**
** Returns          Stream codec and content protection capabilities info.
**
*******************************************************************************/
BTA_API extern BOOLEAN bta_avk_co_audio_init(UINT8 *p_codec_type, UINT8 *p_codec_info,
                                   UINT8 *p_num_protect, UINT8 *p_protect_info, UINT8 index);


/*******************************************************************************
**
** Function         bta_avk_co_audio_disc_res
**
** Description      This callout function is executed by AV to report the
**                  number of stream end points (SEP) were found during the
**                  AVDT stream discovery process.
**
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_avk_co_audio_disc_res(UINT8 num_seps, UINT8 num_snk, BD_ADDR addr);

/*******************************************************************************
**
** Function         bta_avk_co_audio_getconfig
**
** Description      This callout function is executed by AV to retrieve the
**                  desired codec and content protection configuration for the
**                  audio stream.
**
**
** Returns          Stream codec and content protection configuration info.
**
*******************************************************************************/
BTA_API extern UINT8 bta_avk_co_audio_getconfig(tBTA_AVK_CODEC codec_type,
                                         UINT8 *p_codec_info, UINT8 *p_sep_info_idx, UINT8 seid,
                                         UINT8 *p_num_protect, UINT8 *p_protect_info);

/*******************************************************************************
**
** Function         bta_avk_co_audio_setconfig
**
** Description      This callout function is executed by AV to set the
**                  codec and content protection configuration of the audio stream.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_avk_co_audio_setconfig(tBTA_AVK_CODEC codec_type, UINT8 *p_codec_info,
                                        UINT8 seid, BD_ADDR addr,
                                        UINT8 num_protect, UINT8 *p_protect_info);

/*******************************************************************************
**
** Function         bta_avk_co_audio_open
**
** Description      This function is called by AV when the audio stream connection
**                  is opened.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_avk_co_audio_open(tBTA_AVK_CODEC codec_type, UINT8 *p_codec_info,
                                         UINT16 mtu);

/*******************************************************************************
**
** Function         bta_avk_co_audio_close
**
** Description      This function is called by AV when the audio stream connection
**                  is closed.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_avk_co_audio_close(void);

/*******************************************************************************
**
** Function         bta_avk_co_audio_delay
**
** Description      This function is called by AV when the audio stream connection
**                  needs to send the initial delay report to the connected SRC.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_avk_co_audio_delay(UINT8 err);

/*******************************************************************************
**
** Function         bta_avk_co_audio_start
**
** Description      This function is called by AV when the audio streaming data
**                  transfer is started.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_avk_co_audio_start(tBTA_AVK_CODEC codec_type);


/*******************************************************************************
**
** Function         bta_avk_co_audio_btc_start
**
** Description      This function is called by AV when the audio streaming data
**                  transfer is started.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_avk_co_audio_btc_start(void);

/*******************************************************************************
**
** Function         bta_avk_co_audio_stop
**
** Description      This function is called by AV when the audio streaming data
**                  transfer is stopped.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_avk_co_audio_stop(void);


/*******************************************************************************
**
** Function         bta_avk_co_audio_src_data_path
**
** Description      This function is called to get the next data buffer from
**                  the audio codec
**
** Returns          NULL if data is not ready.
**                  Otherwise, a GKI buffer (BT_HDR*) containing the audio data.
**
*******************************************************************************/
BTA_API extern void bta_avk_co_audio_data(BD_ADDR bd_addr, tBTA_AVK_CODEC codec_type, BT_HDR *p_pkt,
                                                    UINT32 timestamp, UINT16 seq_num, UINT8 m_pt);


#if (defined(BTA_AVK_AV_AUDIO_RELAY) && (BTA_AVK_AV_AUDIO_RELAY==TRUE))
/*******************************************************************************
**
** Function         bta_avk_co_should_relay_audio
**
** Description      This function is called by AV when the video stream connection
**                  needs to send the initial delay report to the connected SRC.
**
**
** Returns         TRUE to relay, FALSE otherwise
**
*******************************************************************************/
BTA_API extern BOOLEAN  bta_avk_co_should_relay_audio();

/*******************************************************************************
 **
 ** Function         bta_avk_co_relay_audio_ready
 **
 ** Description     Determines if audio should be relayed to AV
 **
 ** Returns         TRUE to relay, FALSE otherwise
 **
 *******************************************************************************/
BTA_API extern BOOLEAN  bta_avk_co_relay_audio_ready();

/*******************************************************************************
 **
 ** Function        bta_avk_co_media_aa_sbc_buf_cb
 **
 ** Description     Enqueue a buffer for SBC transmission
 **
 ** Returns         none
 **
 *******************************************************************************/
BTA_API extern void bta_avk_co_media_aa_sbc_buf_cb(BT_HDR *p_buf);

#endif

#endif /* BTA_AVK_CO_H */
