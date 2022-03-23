/*****************************************************************************
**
**  Name:           bta_hh_co.h
**
**  Description:    This is the interface file for hid host call-out
**                  functions.
**
**  Copyright (c) 2005, Broadcom Corp., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_HH_CO_H
#define BTA_HH_CO_H

#include "bta_hh_api.h"


/*******************************************************************************
**
** Function         bta_hh_co_data
**
** Description      This callout function is executed by HH when data is received
**                  in interupt channel.
**
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_hh_co_data(UINT8 dev_handle, UINT8 *p_rpt, UINT16 len,
                                   tBTA_HH_PROTO_MODE  mode, UINT8 sub_class,
                                   UINT8 ctry_code, BD_ADDR peer_addr, UINT8 app_id);

/*******************************************************************************
**
** Function         bta_hh_co_open
**
** Description      This callout function is executed by HH when connection is
**                  opened, and application may do some device specific
**                  initialization.
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_hh_co_open(UINT8 dev_handle, UINT8 sub_class,
                                   UINT16 attr_mask, UINT8 app_id);

/*******************************************************************************
**
** Function         bta_hh_co_close
**
** Description      This callout function is executed by HH when connection is
**                  closed, and device specific finalizatio nmay be needed.
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_hh_co_close(UINT8 dev_handle, UINT8 app_id);

#endif /* BTA_HH_CO_H */
