/*****************************************************************************
**
**  Name:           bta_ag_co.h
**
**  Description:    This is the interface file for audio gateway call-out
**                  functions.
**
**  Copyright (c) 2003-2006, Broadcom Corp., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AG_CO_H
#define BTA_AG_CO_H

#include "bta_ag_api.h"

/* Definitions for audio state callout function "state" parameter */
#define BTA_AG_CO_AUD_STATE_OFF         0
#define BTA_AG_CO_AUD_STATE_OFF_XFER    1   /* Closed pending transfer of audio */
#define BTA_AG_CO_AUD_STATE_ON          2
#define BTA_AG_CO_AUD_STATE_SETUP       3

/*******************************************************************************
**
** Function         bta_ag_co_init
**
** Description      This callout function is executed by AG when it is
**                  started by calling BTA_AgEnable().  This function can be
**                  used by the phone to initialize audio paths or for other
**                  initialization purposes.
**
**
** Returns          Void.
**
*******************************************************************************/
BTA_API extern void bta_ag_co_init(void);

/*******************************************************************************
**
** Function         bta_ag_co_audio_state
**
** Description      This function is called by the AG before the audio connection
**                  is brought up, after it comes up, and after it goes down.
**
** Parameters       handle - handle of the AG instance
**                  state - Audio state
**                      BTA_AG_CO_AUD_STATE_OFF      - Audio has been turned off
**                      BTA_AG_CO_AUD_STATE_OFF_XFER - Audio is closed pending transfer
**                      BTA_AG_CO_AUD_STATE_ON       - Audio has been turned on
**                      BTA_AG_CO_AUD_STATE_SETUP    - Audio is about to be turned on
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_ag_co_audio_state(UINT16 handle, UINT8 app_id, UINT8 state);

/*******************************************************************************
**
** Function         bta_ag_co_data_open
**
** Description      This function is executed by AG when a service level connection
**                  is opened.  The phone can use this function to set
**                  up data paths or perform any required initialization or
**                  set up particular to the connected service.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_ag_co_data_open(UINT16 handle, tBTA_SERVICE_ID service);

/*******************************************************************************
**
** Function         bta_ag_co_data_close
**
** Description      This function is called by AG when a service level
**                  connection is closed
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_ag_co_data_close(UINT16 handle);

/*******************************************************************************
**
** Function         bta_ag_co_tx_write
**
** Description      This function is called by the AG to send data to the
**                  phone when the AG is configured for AT command pass-through.
**                  The implementation of this function must copy the data to
**                  the phone’s memory.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_ag_co_tx_write(UINT16 handle, UINT8 *p_data, UINT16 len);

#endif /* BTA_AG_CO_H */
