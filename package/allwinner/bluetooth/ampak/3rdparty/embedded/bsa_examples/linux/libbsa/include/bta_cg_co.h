/*****************************************************************************
**
**  Name:           bta_cg_co.h
**
**  Description:    This is the interface file for cordless gateway call-out
**                  functions.
**
**  Copyright (c) 2005, Broadcom, All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_CG_CO_H
#define BTA_CG_CO_H

#include "bta_cg_api.h"


/*******************************************************************************
**
** Function         bta_cg_co_init
**
** Description      This callout function is executed by CG when it is
**                  started by calling BTA_CGEnable().  This function can be
**                  used by the phone to initialize audio paths or for other
**                  initialization purposes.
**
**
** Returns          CG bearer configuration
**
*******************************************************************************/
BTA_API extern void bta_cg_co_init(UINT8 * p_cg_bearer_config);

/*******************************************************************************
**
** Function         bta_cg_co_audio_state
**
** Description      This function is called by the CG when the audio connection
**                  goes up or down.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_cg_co_audio_state(BOOLEAN on);


#endif /* BTA_CG_CO_H */
