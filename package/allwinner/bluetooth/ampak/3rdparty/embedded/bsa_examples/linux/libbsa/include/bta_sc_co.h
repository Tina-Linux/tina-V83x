/*****************************************************************************
**
**  Name:           bta_sc_co.h
**
**  Description:    This is the interface file for the SIM access
**                  server call-out functions.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_SC_CO_H
#define BTA_SC_CO_H

#include "bta_sc_api.h"

/*****************************************************************************
**  Constants and Data Types
*****************************************************************************/

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function     bta_sc_co_sim_opem
**
** Description  Called when client connection is opened, in case any special
**              handling or intialization of the SIM is required.
**
** Parameters
**          None.
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_co_sim_open(void);

/*******************************************************************************
**
** Function     bta_sc_co_sim_close
**
** Description  Called when client connection is closed.
**
** Parameters
**          None.
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_co_sim_close(void);

/*******************************************************************************
**
** Function     bta_sc_co_sim_reset
**
** Description  This function is called by BTA to reset the SIM card
**
**              Once SIM has been turned reset, call bta_sc_ci_sim_reset().
**
** Parameters
**          None.
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_co_sim_reset(void);

/*******************************************************************************
**
** Function     bta_sc_co_sim_off
**
** Description  This function is called by BTA to turn the SIM card off
**
**              Once SIM has been turned on, call bta_sc_ci_sim_on().
**
** Parameters
**          None.
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_co_sim_off(void);

/*******************************************************************************
**
** Function     bta_sc_co_sim_on
**
** Description  This function is called by BTA to turn the SIM card on
**
**              Once SIM has been turned on, call bta_sc_ci_sim_on().
**
** Parameters
**          None.
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_co_sim_on(void);

/*******************************************************************************
**
** Function     bta_sc_co_atr
**
** Description  This function is called by BTA to retrieve ATR information
**              (operational requirements of the SIM, as described in
**              section 5.8 of GSM 11.11)
**
**              Once the ATR information is retrieved, call bta_sc_ci_atr().
**
** Parameters
**          None.
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_co_atr(void);

/*******************************************************************************
**
** Function     bta_sc_co_apdu
**
** Description  This function is called by BTA to to transfer APDU command
**              messages to the SIM. Generally used for selecting, storing
**              and retrieving data from the SIM.
**
**              Once the command has been completed, call bta_sc_ci_apdu()
**              with the result.
**
** Parameters
**      p_apdu_req  Pointer to the APDU message from the client. Format
**                  is described in section 9 of GSM 11.11
**
**      req_len     Length of APDU message.
**
**      rsp_maxlen  Maximum length of response message allowed by client
**                  (negotiated during CONNECT_REQ)
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_sc_co_apdu(UINT8 *p_apdu_req, UINT16 req_len, UINT16 rsp_maxlen, BOOLEAN is_apdu_7816);

#ifdef __cplusplus
}
#endif

#endif /* BTA_SC_CO_H */
