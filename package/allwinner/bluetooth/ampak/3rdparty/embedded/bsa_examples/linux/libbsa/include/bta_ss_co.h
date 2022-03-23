/*****************************************************************************
**
**  Name:           bta_ss_co.h
**
**  Description:    This is the interface file for the synchronization
**                  server call-out functions.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_SS_CO_H
#define BTA_SS_CO_H

#include "bta_ss_api.h"

/*****************************************************************************
**  Constants and Data Types
*****************************************************************************/
enum
{
    BTA_SS_OP_NONE,         /* nothing */
    BTA_SS_OP_OBJ,          /* object */
    BTA_SS_OP_DEV_INFO,     /* device info */
    BTA_SS_OP_INFO_LOG,     /* store info log */
    BTA_SS_OP_CC,           /* change counter */
    BTA_SS_OP_CHG_LOG,      /* change log */
    BTA_SS_OP_GET_FIRST,    /* get the first PIM object */
    BTA_SS_OP_GET_NEXT,     /* get the next PIM object */
    BTA_SS_OP_GET_LUID,     /* get the PIM object by the given LUID */
    BTA_SS_OP_RTC,          /* real time clock */
    BTA_SS_OP_PUT_MODIFY,   /* modify or add a PIM object */
    BTA_SS_OP_PUT_DELETE,   /* delete a PIM object */
    BTA_SS_OP_PUT_RESTORE   /* the Restore operation */
};
typedef UINT8   tBTA_SS_OP;

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
/*******************************************************************************
**
** Function     bta_ss_co_open
**
** Description  Set up access to PIM
**
** Parameters   void
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_ss_co_open(void);

/*******************************************************************************
**
** Function     bta_ss_co_cl_open
**
** Description  open the change log for the requested data type
**              When the operation is done, call call-in function with
**              the total number of objects
**              the current/latest change counter
**              and the Database ID
**              If cl_open fails, call bta_ss_ci_cl_open with NULL Database ID, .
**
** Parameters   data_type: the requested data type
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_ss_co_cl_open(tBTA_SS_DATA_TYPE data_type);

/*******************************************************************************
**
** Function     bta_ss_co_put_obj
**
** Description  Add/Modify/Delete an PIM item or set the device real time clock
**              When the operation is done, call call-in function with
**              the OBX response code
**              the LUID
**              and the change counter
**
** Parameters   action: the requested action
**              p_file: file name (including path) that contains the received
**                      object
**              luid:   The LUID of the object
**              cur_cc: The current change counter.
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_ss_co_put_obj(tBTA_SS_OP action, char *p_file, tBTA_SS_LUID luid,
                              UINT32 cur_cc);

/*******************************************************************************
**
** Function     bta_ss_co_get_obj
**
** Description  put the requested object in a file and open the file.
**              When the operation is done, call call-in function with
**              the file id
**              and the size of the file
**
** Parameters   item:   the requested PIM item
**              luid:   The LUID of the object
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_ss_co_get_obj(tBTA_SS_OP item, tBTA_SS_LUID luid);

/*******************************************************************************
**
** Function     bta_ss_co_cl_close
**
** Description  close the file - change.log. If the file has too many entries
**              remove some entries.
**              Put database ID, last LUID, current timestamp and
**              the LUIDs of current objects in object.log
**
** Parameters   data_type: the requested data type
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_ss_co_cl_close(tBTA_SS_DATA_TYPE data_type);

/*******************************************************************************
**
** Function     bta_ss_co_close
**
** Description  detach from PIM
**
** Parameters   void
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_ss_co_close(void);

#endif /* BTA_SS_CO_H */
