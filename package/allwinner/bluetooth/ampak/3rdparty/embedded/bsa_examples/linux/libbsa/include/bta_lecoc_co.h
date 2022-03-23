/*****************************************************************************
**
**  Name:           bta_lecoc_co.h
**
**  Description:    This is the interface file for LE COC call-out
**                  functions.
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_LECOC_CO_H
#define BTA_LECOC_CO_H

#include "bta_lecoc_api.h"

/*******************************************************************************
**
** Function         bta_lecoc_co_data
**
** Description      This callout function is executed by LE COC to send a callout
**                  data to application.
**
**
** Returns          Void.
**
*******************************************************************************/
BTA_API extern void bta_lecoc_co_data(UINT16 handle, void *p_ref, UINT16 len, UINT8 *p_data);

/*******************************************************************************
**
** Function         bta_lecoc_co_tx_data
**
** Description      This function is called by LeL2C COC to fetch the data
**                  to be sent in the LE COC channel. Application fill in the
**                  data buffer referred by the data pointer in the required length.
**                  This function will be called when BTA_LecocWrite() is started
**                  sucessfully, and when tx data fetching is done, callin function
**                  bta_lecoc_ci_tx_ready() should be called in response.
**
** Returns          TRUE if sucessfully started; FALSE if failed.
**
*******************************************************************************/
BTA_API extern BOOLEAN bta_lecoc_co_tx_data(UINT16 handle, void *p_ref, UINT32 req_id,
                                            UINT8 *p_data, UINT16 len);

/*******************************************************************************
**
** Function         bta_lecoc_co_tx_data_size
**
** Description      This function is called by LeL2C COC to get the size of data
**                  needs to be sent when data write requested was initailly started.
**
** Returns          TRUE if sucessful, FALSE if failed. return p_len as 0 if no
**                  data needs to be sent.
**
*******************************************************************************/
BTA_API extern BOOLEAN bta_lecoc_co_tx_data_size(UINT16 handle, void *p_ref,
                                                UINT32 req_id, UINT32 *p_len);

#endif /* BTA_LECOC_CO_H */
