/*****************************************************************************
**
**  Name:           bta_bav_ci.h
**
**  Description:    This is the public interface file for the Broadcast
**                  audio/video streaming (BAV) Call-In subsystem of BTA.
**
**  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef BTA_BAV_CI_H
#define BTA_BAV_CI_H

#include "bta_bav_int.h"

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         bta_bav_ci_src_data_ready
**
** Description      This function sends an event to the BAV indicating that
**                  the application has Broadcast audio stream data ready to
**                  send and BAV should call bta_bav_co_audio_src_data_path()
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_bav_ci_src_data_ready(tBTA_BAV_STREAM stream);

#ifdef __cplusplus
}
#endif

#endif /* BTA_BAV_CI_H */
