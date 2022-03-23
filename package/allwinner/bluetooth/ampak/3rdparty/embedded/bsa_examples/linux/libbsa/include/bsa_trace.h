/******************************************************************************
**
** File:         bsa_trace.h
**
** Description:  Contains API for BTE Test Tool trace related functions.
**
**
**  Copyright (c) 2001-2004, WIDCOMM Inc., All Rights Reserved.
**  WIDCOMM Bluetooth Core. Proprietary and confidential.
**
******************************************************************************/

#ifndef BSA_TRACE_H_
#define BSA_TRACE_H_
#ifdef __cplusplus
extern "C" {
#endif
/* Self sufficiency requirement */
/* for integer types */
#include "data_types.h"


/* Globally disable the traces */
extern BOOLEAN global_trace_disable;

/* Application interface trace level */
extern UINT8 appl_trace_level;


/********************************************************************************
 **
 **    Function Name:   scru_dump_hex
 **
 **    Purpose:
 **
 **    Input Parameters:
 **
 **    Returns:
 **
 *********************************************************************************/
UINT8 *scru_dump_hex (UINT8 *p_data, char *p_title, UINT16 len, UINT32 trace_layer, UINT32 trace_type);

#ifdef __cplusplus
}
#endif
#endif /* BSA_TRACE_H_ */
