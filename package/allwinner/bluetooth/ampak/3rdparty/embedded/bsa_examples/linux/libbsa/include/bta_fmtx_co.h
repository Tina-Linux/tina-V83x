/*****************************************************************************
**
**  Name:           bta_fmtx_co.h
**
**  Description:    This is the interface file for FMTX call-out
**                  functions.
**
**  Copyright (c) 2006, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_FMTX_CO_H
#define BTA_FMTX_CO_H

#include "bta_fmtx_api.h"

/*******************************************************************************
**
** Function         bta_fmtx_co_enable
**
** Description      This callout function is executed by FMTX when it is
**                  started by calling BTA_FmtxEnable.  This function can be
**                  used by the phone to initialize audio data source.
**
**
** Returns         tBTA_FMTX_STATUS: operation status.
**
*******************************************************************************/
BTA_API extern tBTA_FMTX_STATUS bta_fmtx_co_enable(tBTA_FMTX_AUD_PATH aud_path);

/*******************************************************************************
**
** Function         bta_fmtx_co_enable
**
** Description      This callout function is executed by FMTX when it is
**                  started by calling BTA_FmtxEnable.  This function can be
**                  used by the phone to initialize audio data.
**
**
** Returns          tBTA_FMTX_STATUS: operation status.
**
*******************************************************************************/
BTA_API extern tBTA_FMTX_STATUS bta_fmtx_co_disable(void);

/*******************************************************************************
**
** Function         bta_fmtx_co_aud_path
**
** Description      This callout function is executed by FMTX when audio path is
**                  reconfigured by BTA_FmtxConfig().  This function can be
**                  used by the phone to initialize audio data souce.
**
**
** Returns          tBTA_FMTX_STATUS: operation status.
**
*******************************************************************************/
BTA_API extern tBTA_FMTX_STATUS bta_fmtx_co_aud_path(tBTA_FMTX_AUD_PATH aud_path);

/*******************************************************************************
**
** Function         bta_fmtx_co_rds_init
**
** Description      This callout function is executed by FMTX when it is
**                  started by calling BTA_FmtxRdsInit() to turn on/off RDS mode.
**                  This function can be used by the phone to init/reset RDS
**                  encoder if RDS is turn on/off to/from RDS manual mode.
**
** Returns          None.
**
*******************************************************************************/
BTA_API extern void bta_fmtx_co_rds_init(tBTA_FMTX_RDS_MODE rds_mode);


#endif /* BTA_FMTX_CO_H */
