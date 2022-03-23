/*****************************************************************************
**
**  Name:           bta_av_ci.h
**
**  Description:    This is the interface file for advanced audio/video call-in
**                  functions.
**
**  Copyright (c) 2005 - 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AV_CI_H
#define BTA_AV_CI_H

#include "bta_av_api.h"

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         bta_av_ci_src_data_ready
**
** Description      This function sends an event to the AV indicating that
**                  the phone has audio stream data ready to send and AV
**                  should call bta_av_co_audio_src_data_path() or
**                  bta_av_co_video_src_data_path().
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_ci_src_data_ready(tBTA_AV_CHNL chnl);

/*******************************************************************************
**
** Function         bta_av_ci_setconfig
**
** Description      This function must be called in response to function
**                  bta_av_co_audio_setconfig() or bta_av_co_video_setconfig.
**                  Parameter err_code is set to an AVDTP status value;
**                  AVDT_SUCCESS if the codec configuration is ok,
**                  otherwise error.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_ci_setconfig(tBTA_AV_HNDL hndl, UINT8 err_code,
                                        UINT8 category, UINT8 num_seid, UINT8 *p_seid,
                                        BOOLEAN recfg_needed);

#if (BTU_DUAL_STACK_MM_INCLUDED == TRUE)
/*******************************************************************************
**
** Function         bta_av_ci_startok
**
** Description      This function is called to send the START_OK_EVT to
**                  BTA-AV after processing the starting-callout from BTA-AV
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_ci_startok(tBTA_AV_HNDL hndl);

#endif

/*******************************************************************************
**
** Function         bta_av_ci_cp_scms
**
** Description      This function is called to set the SCMS Content Protection
**                  for an AV connection
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_av_ci_cp_scms(tBTA_AV_HNDL hndl, BOOLEAN enable, UINT8 scms_hdr);

#ifdef __cplusplus
}
#endif

#endif /* BTA_AV_CI_H */
