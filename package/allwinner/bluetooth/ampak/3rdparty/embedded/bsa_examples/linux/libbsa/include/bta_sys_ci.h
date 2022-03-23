/*****************************************************************************
**
**  Name:           bta_sys_ci.h
**
**  Description:    This is the interface file for system call-in
**                  functions.
**
**  Copyright (c) 2010, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_SYS_CI_H
#define BTA_SYS_CI_H

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
** Function         bta_sys_hw_ci_enabled
**
** Description      This function must be called in response to function
**                  bta_sys_hw_co_enable(), when HW is indeed enabled
**
**
** Returns          void
**
*******************************************************************************/
BTA_API  void bta_sys_hw_ci_enabled(tBTA_SYS_HW_MODULE module );


/*******************************************************************************
**
** Function         bta_sys_hw_ci_disabled
**
** Description      This function must be called in response to function
**                  bta_sys_hw_co_disable() when HW is really OFF
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void bta_sys_hw_ci_disabled( tBTA_SYS_HW_MODULE module  );

#ifdef __cplusplus
}
#endif

#endif
