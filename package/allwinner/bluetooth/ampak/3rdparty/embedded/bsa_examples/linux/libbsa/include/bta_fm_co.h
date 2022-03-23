/*****************************************************************************
**
**  Name:           bta_fm_co.h
**
**  Description:    This is the interface file for FM call-out
**                  functions.
**
**  Copyright (c) 2006, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_FM_CO_H
#define BTA_FM_CO_H

#include "bta_fm_api.h"
#include "bta_rds_api.h"

/*******************************************************************************
**
** Function         bta_fm_co_init
**
** Description      This callout function is executed by FM when it is
**                  started by calling BTA_FmSetRDSMode() and RDS mode is turned
**                  on.  This function can be used by the phone to initialize
**                  RDS decoder or other platform dependent and RDS related purposes.
**
**
** Returns
**
*******************************************************************************/
BTA_API extern tBTA_FM_STATUS bta_fm_co_init(tBTA_FM_RDS_B rds_mode);

/*******************************************************************************
**
** Function         bta_fm_co_rds_data
**
** Description      This function is called by FM when RDS data is ready.
**
** Parameter        p_data: RDS data in three bytes array, which includes 2 bytes
**                  RDS block data, and one byte control and correction indicator.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern tBTA_FM_STATUS bta_fm_co_rds_data(UINT8 * p_data, UINT16 len);

/*******************************************************************************
**
** Function         bta_fm_co_close
**
** Description      This callout function is executed by FM when it is
**                  started by calling BTA_FmSetRDSMode() and RDS mode is turned
**                  off.  This function can be used by the phone to reset RDS
**                  decoder.
**
**
** Returns
**
*******************************************************************************/
BTA_API extern void bta_fm_co_close(void);

#endif /* BTA_FM_CO_H */
