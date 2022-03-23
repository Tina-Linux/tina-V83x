/*****************************************************************************
**
**  Name:           bta_ct_co.h
**
**  Description:    This is the interface file for cordless terminal call-out
**                  functions.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_CT_CO_H
#define BTA_CT_CO_H

#include "bta_ct_api.h"


/*******************************************************************************
**
** Function         bta_ct_co_init
**
** Description      This callout function is executed by CT when it is
**                  started by calling BTA_CtEnable().  This function can be
**                  used by the phone to initialize audio paths or for other
**                  initialization purposes.
**
**
** Returns          CT and IC bearer configuration
**
*******************************************************************************/
BTA_API extern void bta_ct_co_init(UINT8 * p_ct_bearer_config, UINT8 * p_ic_bearer_config);

/*******************************************************************************
**
** Function         bta_ct_co_audio_state
**
** Description      This function is called by the CT when the audio connection
**                  goes up or down.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_ct_co_audio_state(BOOLEAN on);


#endif /* BTA_CT_CO_H */
