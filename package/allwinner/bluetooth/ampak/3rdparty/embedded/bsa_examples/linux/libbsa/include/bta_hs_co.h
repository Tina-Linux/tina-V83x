/*****************************************************************************
**
**  Name:           bta_hs_co.h
**
**  Description:    This is the interface file for audio gateway call-out
**                  functions.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_HS_CO_H
#define BTA_HS_CO_H

#include "bta_hs_api.h"


/*******************************************************************************
**
** Function         bta_hs_co_init
**
** Description      This callout function is executed by HS when it is
**                  started by calling BTA_HsEnable().  This function can be
**                  used by the phone to initialize audio paths or for other
**                  initialization purposes.
**
**
** Returns          Void.
**
*******************************************************************************/
BTA_API extern void bta_hs_co_init(void);

/*******************************************************************************
**
** Function         bta_hs_co_audio_state
**
** Description      Thius function is called by the HS when the audio connection
**                  goes up or down.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_hs_co_audio_state(UINT16 handle, UINT8 app_id, tBTA_HS_AUDIO_STATE status);


#endif /* BTA_AG_CO_H */
