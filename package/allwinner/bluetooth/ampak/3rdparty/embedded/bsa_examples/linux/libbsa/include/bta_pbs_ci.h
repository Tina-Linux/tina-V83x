/*****************************************************************************
**
**  Name:           bta_pbs_ci.h
**
**  Description:    This is the interface file for phone book access server
**                  call-in functions.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_PBS_CI_H
#define BTA_PBS_CI_H

#include "bta_pbs_co.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/


/*****************************************************************************
**  Function Declarations
*****************************************************************************/


/*******************************************************************************
**
** Function         bta_pbs_ci_read
**
** Description      This function sends an event to BTA indicating the phone has
**                  read in the requested amount of data specified in the
**                  bta_pbs_co_read() call-out function.
**
** Parameters       fd - file descriptor passed to the stack in the
**                       bta_pbs_ci_open call-in function.
**                  num_bytes_read - number of bytes read into the buffer
**                      specified in the read callout-function.
**                  status - BTA_PBS_CO_OK if get buffer of data,
**                           BTA_PBS_CO_FAIL if an error has occurred.
**                  final - indicate whether it is the final data
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_pbs_ci_read(int fd, UINT16 num_bytes_read,
                                   tBTA_PBS_CO_STATUS status, BOOLEAN final);

/*******************************************************************************
**
** Function         bta_pbs_ci_open
**
** Description      This function sends an event to BTA indicating the phone has
**                  finished opening a pb for reading.
**
** Parameters       fd - file descriptor passed to the stack in the
**                       bta_pbs_ci_open call-in function.
**                  status - BTA_PBS_CO_OK if file was opened in mode specified
**                                          in the call-out function.
**                           BTA_PBS_CO_EACCES if the file exists, but contains
**                                          the wrong access permissions.
**                           BTA_PBS_CO_FAIL if any other error has occurred.
**                  file_size - The total size of the file
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_pbs_ci_open(int fd, tBTA_PBS_CO_STATUS status,
                                    UINT32 file_size);

/*******************************************************************************
**
** Function         bta_pbs_ci_getvlist
**
** Description      This function sends an event to BTA indicating the phone has
**                  finished reading a VCard list entry.
**
** Parameters
**                  status - BTA_PBS_CO_OK if reading Vcard list entry
**                           BTA_PBS_CO_FAIL if any other error has occurred.
**                  final - whether it is the last entry
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_pbs_ci_getvlist(tBTA_PBS_CO_STATUS status, BOOLEAN final);

/* BSA_SPECIFIC */
/*******************************************************************************
**
** Function         bta_pbs_ci_getpbinfo
**
** Description      This function sends an event to BTA indicating the phone has
**                  retrieved phonebook info
**
** Parameters
**                  status - BTA_PBS_CO_OK if get phonebook info successfully
**                           BTA_PBS_CO_FAIL if any other error has occurred.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_pbs_ci_getpbinfo(tBTA_PBS_OPER operation, tBTA_PBS_CO_STATUS status,
                                        UINT16 pb_size, UINT16 new_missed_call,
                                        tBTA_UINT128 *p_pri_ver, tBTA_UINT128 *p_sec_ver,
                                        tBTA_UINT128 *p_db_id);


#endif
