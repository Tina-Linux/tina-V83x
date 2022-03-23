/*****************************************************************************
**
**  Name:           bta_bav_co.h
**
**  Description:    This is the interface file for Broadcast audio/video call-out
**                  functions.
**
**  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_BAV_CO_H
#define BTA_BAV_CO_H

#include "bta_bav_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/*******************************************************************************
**
** Function         bta_bav_co_init
**
** Description      This callout function is executed by BAV when it is
**                  started by calling BTA_BavEnable().  This function can be
**                  used by the platform to initialize audio/video paths or for
**                  other initialization purposes.
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_bav_co_init(void);

/*******************************************************************************
**
** Function         bta_bav_co_open
**
** Description      This callout function is executed by BAV when the
**                  BTA_BavRegister is called
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_bav_co_open(tBTA_BAV_STREAM stream, UINT16 mtu);

/*******************************************************************************
**
** Function         bta_bav_co_close
**
** Description      This callout function is executed by BAV when the
**                  BTA_BavDeregister is called
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_bav_co_close(tBTA_BAV_STREAM stream);

/*******************************************************************************
**
** Function         bta_bav_co_start
**
** Description      This callout function is executed by BAV when the
**                  BTA_BavStart is called
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_bav_co_start(tBTA_BAV_STREAM stream);

/*******************************************************************************
**
** Function         bta_bav_co_stop
**
** Description      This callout function is executed by BAV when the
**                  BTA_BavStop is called
**
** Returns          void.
**
*******************************************************************************/
BTA_API extern void bta_bav_co_stop(tBTA_BAV_STREAM stream);

/*******************************************************************************
**
** Function         bta_bav_co_src_tx_ready
**
** Description      This callout function is executed by BAV when an AV
**                  Broadcast packet has been send. This is used to indicate
**                  that application can send a new packet
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_bav_co_src_tx_ready(tBTA_BAV_STREAM stream);

/*******************************************************************************
**
** Function         bta_bav_co_src_tx_data_path
**
** Description      This callout function is executed by BAV when to get the next
**                  Broadcast packet data
**
** Returns          NULL if no buffer, a GKI buffer otherwise.
**
*******************************************************************************/
BTA_API extern BT_HDR *bta_bav_co_src_tx_data_path(tBTA_BAV_STREAM stream);

#endif /* BTA_BAV_CO_H */
