/*****************************************************************************
**
**  Name:           bta_sc_ci.h
**
**  Description:    This is the interface file for SIM Access server
**                  call-in functions.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_SC_CI_H
#define BTA_SC_CI_H

#include "bta_sc_api.h"

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function     bta_sc_ci_apdu
**
** Description  This function sends an event to SC to indicate the APDU request
**              has been completed or requested
**
** Parameters
**      result  result of operation or request of operation
**      p_apdu   response buffer for APDU command (if result=BTA_SC_RESULT_OK)
**               or request buffer for APDU command (if result=BTA_SC_REQUEST_APDU)
**      apdulen  length of response or request buffer
**      is_apdu_7816 TRUE if format of apdu data is APDU7816
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_ci_apdu(tBTA_SC_RESULT result, UINT8 *p_apdu, UINT16 apdulen, BOOLEAN is_apdu_7816);

/*******************************************************************************
**
** Function     bta_sc_ci_atr
**
** Description  This function sends an event to SC to indicate the ATR request
**              has been completed
**
** Parameters
**      result  result of operation
**      p_atr   response buffer for ATR request (if result=BTA_SC_RESULT_OK)
**      atrlen  length of response buffer
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_ci_atr(tBTA_SC_RESULT result, UINT8 *p_atr, UINT16 atrlen);

/*******************************************************************************
**
** Function     bta_sc_ci_sim_on
**
** Description  This function sends an event to SC to indicate the SIM_ON request
**              has been completed
**
** Parameters
**      result  result of operation
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_ci_sim_on(tBTA_SC_RESULT result);

/*******************************************************************************
**
** Function     bta_sc_ci_sim_off
**
** Description  This function sends an event to SC to indicate the SIM_OFF request
**              has been completed
**
** Parameters
**      result  result of operation
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_ci_sim_off(tBTA_SC_RESULT result);

/*******************************************************************************
**
** Function     bta_sc_ci_sim_reset
**
** Description  This function sends an event to SC to indicate the SIM_RESET request
**              has been completed
**
** Parameters
**      result  result of operation
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_ci_sim_reset(tBTA_SC_RESET_RESULT result);

#ifdef __cplusplus
}
#endif

#endif /* BTA_SC_CI_H */
