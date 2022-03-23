/*****************************************************************************
**
**  Name:           bta_mip_ci.h
**
**  Description:    This is the interface file for BTA MIP call-in
**                  functions.
**
**  Copyright (c) 2011, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
*****************************************************************************/
#ifndef BTA_MIP_CI_H
#define BTA_MIP_CI_H

// #include "bta_av_api.h"

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         bta_mip_ci_data_ready
**
** Description      This function sends an event to the BTA MIP indicating that
**                  audio data is ready to send and BTA MIP should call-out
**                  to get the data from the application.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mip_ci_data_ready (void);

#ifdef __cplusplus
}
#endif

#endif /* BTA_MIP_CI_H */
