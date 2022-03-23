/*****************************************************************************
**
**  Name:           bta_avk_ci.h
**
**  Description:    This is the interface file for advanced audio/video call-in
**                  functions.
**
**  Copyright (c) 2004-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AVK_CI_H
#define BTA_AVK_CI_H

#include "bta_avk_api.h"

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         bta_avk_ci_setconfig
**
** Description      This function must be called in response to function
**                  bta_avk_co_audio_setconfig() or bta_avk_co_video_setconfig.
**                  Parameter err_code is set to an AVDTP status value;
**                  AVDT_SUCCESS if the codec configuration is ok,
**                  otherwise error.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_avk_ci_setconfig(tBTA_AVK_CHNL chnl, UINT8 err_code, UINT8 category, BD_ADDR bd_addr);

/*******************************************************************************
**
** Function         bta_avk_ci_cp_scms
**
** Description      This function is called to set the SCMS Content Protection
**                  for an AVK connection
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_avk_ci_cp_scms(tBTA_AVK_CHNL chnl, BOOLEAN enable, UINT8 scms_hdr);

/*******************************************************************************
**
** Function         bta_avk_ci_audio_btc_start
**
** Description      This function is called in response to function
**                  bta_avk_co_audio_btc_start
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_avk_ci_audio_btc_start(tBTA_AVK_CHNL chnl);

#ifdef __cplusplus
}
#endif

#endif /* BTA_AVK_CI_H */
