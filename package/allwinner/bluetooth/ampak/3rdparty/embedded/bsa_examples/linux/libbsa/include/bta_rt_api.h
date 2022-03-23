/*****************************************************************************
**
**  Name:           bta_rt_api.h
**
**  Description:    This is the public interface file for the audio routing
**                  subsystem of BTA, Broadcom's Bluetooth application layer
**                  for mobile phones.
**
**  Copyright (c) 2006, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_RT_API_H
#define BTA_RT_API_H

#include "bta_api.h"



/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* RT callback events */
#define BTA_RT_REGISTER_EVT             0       /* registered to RT module */
#define BTA_RT_SET_AUDIO_CODEC_RESP     1
#define BTA_RT_SET_AUDIO_ROUTE_RESP     2
#define BTA_RT_SET_AUDIO_MIX_RESP       3
#define BTA_RT_AUDIO_BURST_FRAMES_IND   4
#define BTA_RT_AUDIO_BURST_END_IND      5
#define BTA_RT_SET_SUB_ROUTE_RESP       6

typedef UINT8 tBTA_RT_EVT;

/* Event associated with BTA_RT_REGISTER_EVT */
typedef struct
{
    tBTA_STATUS         status;
    UINT16              rt_handle;
} tBTA_RT_REGISTER;

/* Event associated with generic response callback */
typedef struct
{
    tAUDIO_CONFIG_STATUS    status;
    UINT32                  param;
} tBTA_RT_RESP;

/* union of data associated with RT callback */
typedef union
{
    tBTA_RT_REGISTER    registr;
    tBTA_RT_RESP        resp;
} tBTA_RT;

/* RT callback */
typedef void (tBTA_RT_CBACK)(tBTA_RT_EVT event, tBTA_RT *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#if (BTU_DUAL_STACK_BTC_INCLUDED == TRUE)
/*******************************************************************************
**
** Function         BTA_RtRegister
**
** Description      Register an application to BTA audio routing module.
**
** Parameters
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtRegister (tBTA_RT_CBACK *p_callback);

/*******************************************************************************
**
** Function         BTA_RtDeregister
**
** Description      Register an application to BTA audio routing module.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_RtDeregister (UINT16 rt_handle);

/*******************************************************************************
**
** Function         BTA_RtSetAudioCodec
**
** Description      Specify the audio codec type and configuration to be used
**                  by the controller.
**
** Parameters
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtSetAudioCodec (UINT16 rt_handle, tAUDIO_CODEC_TYPE codec_type,
                          tCODEC_INFO codec_info);

/*******************************************************************************
**
** Function         BTA_RtSetCodecBitRate
**
** Description      Change the bit rate of the enabled controller codec.
**
** Parameters
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtSetCodecBitRate (UINT16 rt_handle, tAUDIO_CODEC_TYPE codec_type, UINT32 param);

/*******************************************************************************
**
** Function         BTA_RtSetAudioRoute
**
** Description      Start or stop audio routing from the specified audio source
**                  to the audio output path.
**
** Parameters
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtSetAudioRoute (UINT16 rt_handle,
                            tAUDIO_ROUTE_SRC   src,
                            tAUDIO_ROUTE_SF    src_sf,
                            tAUDIO_ROUTE_OUT   out,
                            tAUDIO_ROUTE_SF    codec_sf,
                            tAUDIO_ROUTE_SF    i2s_sf,
                            tAUDIO_ROUTE_EQ    eq_mode);

/*******************************************************************************
**
** Function         BTA_RtSetSubRoute
**
** Description      Used to set up I2S->DAC for FM TIVO.
**
** Parameters
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtSetSubRoute (UINT16 rt_handle,
                            tAUDIO_ROUTE_SRC   src,
                            tAUDIO_ROUTE_SF    src_sf,
                            tAUDIO_ROUTE_OUT   out,
                            tAUDIO_ROUTE_SF    codec_sf,
                            tAUDIO_ROUTE_SF    i2s_sf,
                            tAUDIO_ROUTE_EQ    eq_mode);

/*******************************************************************************
**
** Function         BTA_RtSetAudioMixing
**
** Description      Start or stop audio mixing.
**                  Audio mixing is valid only after the primary audio path is
**                  established by BTA_RtSetAudioRoute().
**
** Parameters
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtSetAudioMixing (UINT16 rt_handle,
                            tAUDIO_ROUTE_MIX  mix_src,
                            tAUDIO_ROUTE_SF   mix_src_sf,
                            tMIX_SCALE_CONFIG mix_scale,
                            tCHIRP_CONFIG    *p_chirp_config);

/*******************************************************************************
**
** Function         BTA_RtSendBurstData
**
** Description      Send burst packets to ARIP-supporting controller.
**
** Parameters
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtSendBurstData(UINT16 length);

/*******************************************************************************
**
** Function         BTA_RtFlushBurstBuffer
**
** Description      Request the ARIP-supporting controller to clear the burst
**                  buffer. This function is used for fast-forward or rewind
**                  operation to clear buffered controller data.
**
** Parameters
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtFlushBurstBuffer (void);

/*******************************************************************************
**
** Function         BTA_RtSetEQMode
**
** Description      Set the equalizer mode. The equalizer mode is set when audio
**                  routing is started by BTA_RtSetAudioRoute(). This function is
**                  used to change the equalizer mode on the existing audio route
**                  without stopping it.
**
** Parameters
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtSetEQMode (UINT16 rt_handle, tAUDIO_ROUTE_EQ eq_mode);

/*******************************************************************************
**
** Function         BTA_RtSetEQGainConfig
**
** Description      Set equalizer gain configuration.
**
** Parameters
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtSetEQGainConfig (UINT16 rt_handle, tEQ_GAIN_CONFIG *p_eq_gain);

/*******************************************************************************
**
** Function         BTA_RtSetDigitalVolume
**
** Description      Set digital volume.
**
** Parameters       rt_handle: audio routing handle
**                  digital_volume: from 0 to 15. 0 means max volume and 15 means mute.
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtSetDigitalVolume (UINT16 rt_handle, UINT16 digital_volume);

/*******************************************************************************
**
** Function         BTA_RtSetAudioScale
**
** Description      Set the audio mixing ratio(scale) between the primary and mixing
**                  audio source. The audio scale can be specified by
**                  BTA_RtSetAudioMixing(), this function is used to change the
**                  audio scale whether audio mixing is in progress or not.
**
** Parameters
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_RtSetAudioScale (UINT16 rt_handle, tMIX_SCALE_CONFIG mix_scale);

#endif /* (BTU_DUAL_STACK_BTC_INCLUDED == TRUE) */

#ifdef __cplusplus
}
#endif

#endif /* BTA_RT_API_H */
