/*****************************************************************************
**
**  Name:           bta_dm_ci.h
**
**  Description:    This is the interface file for device mananger call-in
**                  functions.
**
**  Copyright (c) 2006, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_DM_CI_H
#define BTA_DM_CI_H

#include "bta_api.h"

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         bta_dm_ci_io_req
**
** Description      This function must be called in response to function
**                  bta_dm_co_io_req(), if *p_oob_data is set to BTA_OOB_UNKNOWN
**                  by bta_dm_co_io_req().
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dm_ci_io_req(BD_ADDR bd_addr, tBTA_IO_CAP io_cap,
                                     tBTA_OOB_DATA oob_data, tBTA_AUTH_REQ auth_req);

/*******************************************************************************
**
** Function         bta_dm_ci_rmt_oob
**
** Description      This function must be called in response to function
**                  bta_dm_co_rmt_oob() ??? to provide the OOB data associated
**                  with the remote device.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dm_ci_rmt_oob(BOOLEAN accept, BD_ADDR bd_addr,
                                      BT_OCTET16 c, BT_OCTET16 r);

#if (BTM_BR_SC_INCLUDED == TRUE)
/*******************************************************************************
**
** Function         bta_dm_ci_rmt_oob_ext
**
** Description      This function must be called in response to function
**                  bta_dm_co_rmt_oob() ??? to provide the OOB extended data associated
**                  with the remote device.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dm_ci_rmt_oob_ext(BOOLEAN accept, BD_ADDR bd_addr,
                                      BT_OCTET16 c_192, BT_OCTET16 r_192,
                                      BT_OCTET16 c_256, BT_OCTET16 r_256);
#endif

/*******************************************************************************
**
** Function         bta_dm_sco_ci_data_ready
**
** Description      This function sends an event to indicating that the phone
**                  has SCO data ready..
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dm_sco_ci_data_ready(UINT16 event, UINT16 sco_handle);

#ifdef __cplusplus
}
#endif

#endif
