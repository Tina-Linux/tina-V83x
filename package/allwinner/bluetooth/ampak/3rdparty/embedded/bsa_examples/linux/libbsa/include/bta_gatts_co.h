/*****************************************************************************
**
**  Name:           bta_gatts_co.h
**
**  Description:    This is the interface file for BTA GATT server call-out
**                  functions.
**
**  Copyright (c) 2010, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_GATTS_CO_H
#define BTA_GATTS_CO_H

#include "bta_gatt_api.h"

/*******************************************************************************
**
** Function         bta_gatts_co_update_handle_range
**
** Description      This callout function is executed by GATTS when a GATT server
**                  handle range ios to be added or removed.
**
** Parameter        is_add: true is to add a handle range; otherwise is to delete.
**                  p_hndl_range: handle range.
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_gatts_co_update_handle_range(BOOLEAN is_add, tBTA_GATTS_HNDL_RANGE *p_hndl_range);

/*******************************************************************************
**
** Function         bta_gatts_co_srv_chg
**
** Description      This call-out is to read/write/remove service change related
**                  informaiton. The request consists of the cmd and p_req and the
**                  response is returned in p_rsp
**
** Parameter        cmd - request command
**                  p_req - request paramters
**                  p_rsp - response data for the request
**
** Returns          TRUE - if the request is processed successfully and
**                         the response is returned in p_rsp.
**                  FASLE - if the request can not be processed
**
*******************************************************************************/
BTA_API extern BOOLEAN bta_gatts_co_srv_chg(tBTA_GATTS_SRV_CHG_CMD cmd,
                                            tBTA_GATTS_SRV_CHG_REQ *p_req,
                                            tBTA_GATTS_SRV_CHG_RSP *p_rsp);

/*******************************************************************************
**
** Function         bta_gatts_co_load_handle_range
**
** Description      This callout function is executed by GATTS when a GATT server
**                  handle range is requested to be loaded from NV.
**
** Parameter
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern  BOOLEAN bta_gatts_co_load_handle_range(UINT8 index,
                                                       tBTA_GATTS_HNDL_RANGE *p_handle);


#endif /* BTA_GATTS_CO_H */
