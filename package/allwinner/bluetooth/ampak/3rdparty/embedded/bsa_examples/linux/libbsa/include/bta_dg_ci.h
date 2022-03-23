/*****************************************************************************
**
**  Name:           bta_dg_ci.h
**
**  Description:    This is the interface file for data gateway call-in
**                  functions.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_DG_CI_H
#define BTA_DG_CI_H

#include "bta_dg_api.h"

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         bta_dg_ci_tx_ready
**
** Description      This function sends an event to DG indicating the phone is
**                  ready for more data and DG should call bta_dg_co_tx_path().
**                  This function is used when the TX data path is configured
**                  to use a pull interface.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_ci_tx_ready(UINT16 handle);

/*******************************************************************************
**
** Function         bta_dg_ci_rx_ready
**
** Description      This function sends an event to DG indicating the phone
**                  has data available to send to DG and DG should call
**                  bta_dg_co_rx_path().  This function is used when the RX
**                  data path is configured to use a pull interface.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_ci_rx_ready(UINT16 handle);

/*******************************************************************************
**
** Function         bta_dg_ci_tx_flow
**
** Description      This function is called to enable or disable data flow on
**                  the TX path.  The phone should call this function to
**                  disable data flow when it is congested and cannot handle
**                  any more data sent by bta_dg_co_tx_write() or
**                  bta_dg_co_tx_writebuf().  This function is used when the
**                  TX data path is configured to use a push interface.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_ci_tx_flow(UINT16 handle, BOOLEAN enable);

/*******************************************************************************
**
** Function         bta_dg_ci_rx_writebuf
**
** Description      This function is called to send data to the phone when
**                  the RX path is configured to use a push interface with
**                  zero copy.  The function sends an event to DG containing
**                  the data buffer.  The buffer must be allocated using
**                  functions GKI_getbuf() or GKI_getpoolbuf().  The buffer
**                  will be freed by BTA; the phone must not free the buffer.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_ci_rx_writebuf(UINT16 handle, BT_HDR *p_buf);

/*******************************************************************************
**
** Function         bta_dg_ci_control
**
** Description      This function is called to send RS-232 signal information
**                  to DG to be propagated over RFCOMM.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_ci_control(UINT16 handle, UINT8 signals, UINT8 values);

#ifdef __cplusplus
}
#endif

#endif /* BTA_DG_CI_H */
