/*****************************************************************************
**
**  Name:           bta_lecoc_ci.h
**
**  Description:    This is the interface file for the LE COC subsystem call-in functions.
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_LECOC_CI_H
#define BTA_LECOC_CI_H

#include "bta_api.h"
#include "bta_lecoc_api.h"

/*******************************************************************************
**
** Function         bta_lecoc_ci_tx_ready
**
** Description      This function is called to send an event to LECOC indicating the
**                  TX data is ready.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_lecoc_ci_tx_ready (UINT16 handle, UINT16 len, tBTA_LECOC_STATUS status);

#endif
