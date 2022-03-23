/*****************************************************************************
**
**  Name:           bta_ag_ci.h
**
**  Description:    This is the interface file for audio gateway call-in
**                  functions.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AG_CI_H
#define BTA_AG_CI_H

#include "bta_ag_api.h"

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         bta_ag_ci_rx_write
**
** Description      This function is called to send data to the AG when the AG
**                  is configured for AT command pass-through.  The function
**                  copies data to an event buffer and sends it.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_ag_ci_rx_write(UINT16 handle, char *p_data, UINT16 len);

/******************************************************************************
**
** Function         bta_ag_ci_slc_ready
**
** Description      This function is called to notify AG that SLC is up at
**                  the application. This funcion is only used when the app
**                  is running in pass-through mode.
**
** Returns          void
**
******************************************************************************/
BTA_API extern void bta_ag_ci_slc_ready(UINT16 handle);

#ifdef __cplusplus
}
#endif

#endif /* BTA_AG_CI_H */
