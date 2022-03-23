/*****************************************************************************
**
**  Name:           bta_ss_ci.h
**
**  Description:    This is the interface file for synchronization server
**                  call-in functions.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_SS_CI_H
#define BTA_SS_CI_H

#include "data_types.h"
#include "bta_ss_api.h"
#include "bta_ss_co.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* Change Log Open event */
typedef struct
{
    BT_HDR          hdr;
    UINT32          cur_cc;
    UINT16          obj_count;
    char            *p_did;
} tBTA_SS_CI_CL_OPEN_EVT;

/* Put Ready Event */
typedef struct
{
    BT_HDR          hdr;
    UINT8           obx_rsp;
    tBTA_SS_LUID    luid;
    UINT32          cc;
} tBTA_SS_CI_PUT_EVT;

/* Get Ready Event */
typedef struct
{
    BT_HDR  hdr;
    int     fd;
    UINT32  file_size;
} tBTA_SS_CI_GET_EVT;

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function     bta_ss_ci_cl_open
**
** Description  This function sends an event to SS to indicate the total number
**              of objects, the current change counter and the database id,
**              If cl_open fails, return NULL as p_did, .
**
** Parameters   obj_count - the total number of objects
**              cur_cc    - the current change counter
**              p_did     - the database id.
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_ss_ci_cl_open(UINT16 obj_count, UINT32 cur_cc, char * p_did );

/*******************************************************************************
**
** Function     bta_ss_ci_put_obj
**
** Description  This function sends an event to SS to indicate the put object
**              operation is finished
**
** Parameters   obx_rsp_code- the OBEX response code to send back to the client
**              luid        - the result LUID
**              cc          - the result change counter.
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_ss_ci_put_obj(UINT8 obx_rsp_code, tBTA_SS_LUID luid, UINT32 cc);

/*******************************************************************************
**
** Function     bta_ss_ci_get_obj
**
** Description  This function sends an event to SS to indicate the get object
**              operation is finished
**
** Parameters   fd          - the file handle that contains the requested object
**                            0, if the requested object is not found.
**              file_size   - the total number of bytes of the requested object
**
** Returns      void
**
*******************************************************************************/
BTA_API extern void bta_ss_ci_get_obj(int fd, UINT32  file_size);


#ifdef __cplusplus
}
#endif

#endif /* BTA_SS_CI_H */
