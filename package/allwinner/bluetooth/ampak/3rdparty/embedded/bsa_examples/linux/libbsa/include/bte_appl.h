/*****************************************************************************
**                                                                           *
**  Name:          bte_appl.h                                                *
**                                                                           *
**  Description:   This is the interface file for the bte application task   *
**                                                                           *
**  Copyright (c) 2002-2004 WIDCOMM Inc., All Rights Reserved.               *
**  WIDCOMM Bluetooth Core. Proprietary and confidential.                    *
******************************************************************************/

#ifndef BTE_APPL_H
#define BTE_APPL_H

/* Exports the application task */
extern void BTE_appl_task(UINT32 params);

extern UINT8 appl_trace_level;
#endif  /* BTE_APPL_H */
