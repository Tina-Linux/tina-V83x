/*****************************************************************************
**
**  Name:           bta_mip_co.h
**
**  Description:    This is the interface file for BTA MIP call-out
**                  functions.
**
**  Copyright (c) 2011, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
*****************************************************************************/
#ifndef BTA_MIP_CO_H
#define BTA_MIP_CO_H

#include "bta_mip_api.h"

/*******************************************************************************
**
** Function         bta_mip_co_a2dp_open
**
** Description      This callout function is executed when the first MIP device
**                  is connected so that the application get ready for the audio
**                  codec.
**
** Returns          Void.
**
*******************************************************************************/
BTA_API extern void bta_mip_co_a2dp_open (void);

/*******************************************************************************
**
** Function         bta_mip_co_a2dp_start
**
** Description      This callout function is executed when the first MIP device
**                  moves into MIP mode. The application is supposed to start
**                  audio codec.
**
** Returns          Void.
**
*******************************************************************************/
BTA_API extern void bta_mip_co_a2dp_start (void);

/*******************************************************************************
**
** Function         bta_mip_co_a2dp_close
**
** Description      This callout function is executed when there is no more
**                  connected MIP devices. The application is supposed to stop
**                  the audio codec.
**
** Returns          Void.
**
*******************************************************************************/
BTA_API extern void bta_mip_co_a2dp_close (void);

/*******************************************************************************
**
** Function         bta_mip_co_a2dp_get_data
**
** Description      This callout function is executed to get audio packet from
**                  the audio codec.
**
** Returns          NULL if data is not ready.
**                  Otherwise, a GKI buffer (BT_HDR*) containing the audio data.
*******************************************************************************/
BTA_API extern void *bta_mip_co_a2dp_get_data (UINT32 *p_len);

/*******************************************************************************
**
** Function         bta_mip_co_3dg_begin
**
** Description      This callout function is executed when the first 3DG
**                  moves into MIP mode. The application is supposed to start
**                  suttering process by sending VSC to the controller.
**
** Returns          Void.
**
*******************************************************************************/
BTA_API extern void bta_mip_co_3dg_begin (void);

/*******************************************************************************
**
** Function         bta_mip_co_3dg_end
**
** Description      This callout function is executed when ther is no more
**                  connected MIP 3DG. The application is supposed to stop
**                  shuttering process by sending VSC to the controller.
**
** Returns          Void.
**
*******************************************************************************/
BTA_API extern void bta_mip_co_3dg_end (void);

#endif /* BTA_MIP_CO_H */
