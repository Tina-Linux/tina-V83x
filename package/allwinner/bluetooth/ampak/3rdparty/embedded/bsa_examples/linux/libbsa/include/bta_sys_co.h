/*****************************************************************************
**
**  Name:           bta_sys_co.h
**
**  Description:    This is the interface file for system callout
**                  functions.
**
**  Copyright (c) 2010, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_SYS_CO_H
#define BTA_SYS_CO_H

#include "bta_sys.h"



/*****************************************************************************
**  Function Declarations
*****************************************************************************/


/*******************************************************************************
**
** Function         bta_sys_hw_co_enable
**
** Description      This function is called by the stack to power up the HW
**
** Returns          void
**
*******************************************************************************/
BTA_API void bta_sys_hw_co_enable( tBTA_SYS_HW_MODULE module );

/*******************************************************************************
**
** Function         bta_sys_hw_co_disable
**
** Description     This function is called by the stack to power down the HW
**
** Returns          void
**
*******************************************************************************/
BTA_API void bta_sys_hw_co_disable( tBTA_SYS_HW_MODULE module );


#endif
