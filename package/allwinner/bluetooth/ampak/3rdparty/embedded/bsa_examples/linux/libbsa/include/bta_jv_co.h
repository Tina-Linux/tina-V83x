/*****************************************************************************
**
**  Name:           bta_jv_co.h
**
**  Description:    This is the interface file for java interface call-out
**                  functions.
**
**  Copyright (c) 2007, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_JV_CO_H
#define BTA_JV_CO_H

#include "bta_jv_api.h"

/*****************************************************************************
**  Function Declarations
*****************************************************************************/


/*******************************************************************************
**
** Function         bta_jv_co_rfc_data
**
** Description      This function is called by JV to send data to the java glue
**                  code when the RX data path is configured to use a call-out
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_jv_co_rfc_data(UINT32 handle, UINT8 *p_data, UINT16 len);


#endif /* BTA_DG_CO_H */
