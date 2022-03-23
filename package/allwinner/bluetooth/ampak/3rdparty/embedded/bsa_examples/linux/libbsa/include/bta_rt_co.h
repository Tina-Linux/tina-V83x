/*****************************************************************************
**
**  Name:           bta_rt_co.h
**
**  Description:    This is the interface file for RT(audio routing) callout
**                  functions.
**
**  Copyright (c) 2010, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_RT_CO_H
#define BTA_RT_CO_H

#include "bta_sys.h"


/*****************************************************************************
**  Function Declarations
*****************************************************************************/

/*******************************************************************************
**
** Function         bta_rt_co_tx_burst_data
**
** Description      This function is called to send audio burst data over HCI.
**
** Returns          TRUE if this is the last frame
**
*******************************************************************************/
BTA_API extern BOOLEAN bta_rt_co_tx_burst_data(void *p_data, UINT16 length);

/*******************************************************************************
**
** Function         bta_rt_co_rx_burst_data
**
** Description      This function is called to send incoming audio burst data to application.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_rt_co_rx_burst_data(BT_HDR *p_data);
#endif
