/*****************************************************************************
**
**  Name:           bta_gattc_co.h
**
**  Description:    This is the interface file for BTA GATT client call-out
**                  functions.
**
**  Copyright (c) 2010, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_GATTC_CO_H
#define BTA_GATTC_CO_H

#include "bta_gatt_api.h"

/*******************************************************************************
**
** Function         bta_gattc_co_cache_open
**
** Description      This callout function is executed by GATTC when a GATT server
**                  cache is ready to be sent.
**
** Parameter        server_bda: server bd address of this cache belongs to
**                  evt: call in event to be passed in when cache open is done.
**                  conn_id: connection ID of this cache operation attach to.
**                  to_save: open cache to save or to load.
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_gattc_co_cache_open(BD_ADDR server_bda, UINT16 evt,
                                            UINT16 conn_id, BOOLEAN to_save);

/*******************************************************************************
**
** Function         bta_gattc_co_cache_close
**
** Description      This callout function is executed by GATTC when a GATT server
**                  cache is written completely.
**
** Parameter        server_bda: server bd address of this cache belongs to
**                  conn_id: connection ID of this cache operation attach to.
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_gattc_co_cache_close(BD_ADDR server_bda, UINT16 conn_id);

/*******************************************************************************
**
** Function         bta_gattc_co_cache_save
**
** Description      This callout function is executed by GATT when a server cache
**                  is available to save.
**
** Parameter        server_bda: server bd address of this cache belongs to
**                  evt: call in event to be passed in when cache save is done.
**                  num_attr: number of attribute to be save.
**                  p_attr: pointer to the list of attributes to save.
**                  attr_index: starting attribute index of the save operation.
**                  conn_id: connection ID of this cache operation attach to.
** Returns
**
*******************************************************************************/
BTA_API extern void bta_gattc_co_cache_save(BD_ADDR server_bda, UINT16 evt,
                                          UINT16 num_attr, tBTA_GATTC_NV_ATTR *p_attr,
                                          UINT16 attr_index, UINT16 conn_id);

/*******************************************************************************
**
** Function         bta_gattc_co_cache_load
**
** Description      This callout function is executed by GATT when server cache
**                  is required to load.
**
** Parameter        server_bda: server bd address of this cache belongs to
**                  evt: call in event to be passed in when cache save is done.
**                  num_attr: number of attribute to be save.
**                  attr_index: starting attribute index of the save operation.
**                  conn_id: connection ID of this cache operation attach to.
** Returns
**
*******************************************************************************/
BTA_API extern void bta_gattc_co_cache_load(BD_ADDR server_bda, UINT16 evt,
                                            UINT16 start_index, UINT16 conn_id);

/*******************************************************************************
**
** Function         bta_gattc_co_cache_reset
**
** Description      This callout function is executed by GATTC to reset cache in
**                  application
**
** Parameter        server_bda: server bd address of this cache belongs to
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_gattc_co_cache_reset(BD_ADDR server_bda);

#endif /* BTA_GATT_CO_H */
