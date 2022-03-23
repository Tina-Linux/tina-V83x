/*****************************************************************************
**
**  Name:           bta_mce_co.h
**
**  Description:    This is the interface file for the Message Client Equipment
**                  (MCE) subsystem call-out functions.
**
**  Copyright (c) 2009-2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_MCE_CO_H
#define BTA_MCE_CO_H

#include "bta_api.h"
#include "bta_ma_def.h"
#include "bta_mse_api.h"
#include "bta_mce_api.h"
#include "bta_fs_co.h"

/*****************************************************************************
**  Constants and Data Types
*****************************************************************************/
/**************************
**  Common Definitions
***************************/
/* BSA_SPECIFIC */
/* Status codes returned by call-out functions, or in call-in functions as status */
#define BTA_MCE_CO_OK            GOEP_OK
#define BTA_MCE_CO_FAIL          GOEP_FAIL   /* Used to pass all other errors */
#define BTA_MCE_CO_EACCES        GOEP_EACCES
#define BTA_MCE_CO_ENOTEMPTY     GOEP_ENOTEMPTY
#define BTA_MCE_CO_EOF           GOEP_EOF
#define BTA_MCE_CO_EODIR         GOEP_EODIR
#define BTA_MCE_CO_ENOSPACE      GOEP_ENOSPACE/* Returned in bta_fs_ci_open if no room */
#define BTA_MCE_CO_EIS_DIR       GOEP_EIS_DIR
#define BTA_MCE_CO_RESUME        GOEP_RESUME /* used in ci_open, on resume */
#define BTA_MCE_CO_NONE          GOEP_NONE /* used in ci_open, on resume (no file to resume) */

typedef UINT16 tBTA_MCE_CO_STATUS;


#define BTA_MCE_LEN_UNKNOWN      GOEP_LEN_UNKNOWN

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
/**************************
**  Common Functions
***************************/


/*******************************************************************************
**
** Function         bta_mce_co_write_msg
**
** Description      This function is called to write rcvd message to application
**
**
** Parameters       mas_session_id - MAS session ID
**                  msg_size - size of message to be written
**                  p_msg - points to the message to be written to the application
**                  is_last_packet - FASLE - need to do more write TRUE- done
**                  evt - event that to be passed into the call-in function
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mce_co_write_msg( int fd, tBTA_MA_SESS_HANDLE session_id,
                                          UINT32 msg_size,
                                          tBTA_MA_FRAC_DELIVER frac_delv,
                                          UINT8 *p_msg,
                                          BOOLEAN is_last_packet,
                                          UINT16 evt );

/*******************************************************************************
**
** Function         bta_mce_co_read_upload_msg
**
** Description      This function is called to write rcvd message to application
**
**
** Parameters       mas_session_id - MAS session ID
**                  buffer_size - size of buffer
**                  p_buffer - pointer to the address of the buffer
**                  evt - event that to be passed into the call-in function
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mce_co_read_upload_msg( int fd,
                                                UINT32 buffer_size,
                                                UINT8 *p_buffer,
                                                UINT16 evt,
                                                tBTA_MA_SESS_HANDLE session_id);


/*******************************************************************************
**
** Function         bta_mce_co_open
**
** Description      This function is executed by BTA when a file is opened.
**                  The phone uses this function to open
**                  a file for reading or writing.
**
** Parameters       p_path  - Fully qualified path and file name.
**                  oflags  - permissions and mode (see constants above)
**                  size    - size of file to put (0 if unavailable or not applicable)
**                  evt     - event that must be passed into the call-in function.
**                  app_id  - application ID specified in the enable functions.
**                            It can be used to identify which profile is the caller
**                            of the call-out function.
**                  inst_id - MA instance ID
**
** Returns          void
**
**                  Note: Upon completion of the request, a file descriptor (int),
**                        if successful, and an error code (tBTA_FS_CO_STATUS)
**                        are returned in the call-in function, bta_fs_ci_open().
**
*******************************************************************************/
BTA_API extern void bta_mce_co_open(const char *p_path, int oflags, UINT32 size, UINT16 evt,
                    UINT8 app_id, tBTA_MA_INST_ID inst_id);

/*******************************************************************************
**
** Function         bta_mce_co_get_msg_path
**
** Description      This function is used to get the path for the specified
**                  message handle
**
** Parameters       handle_str - message handle string
**                  p_path  - (output) Fully qualified path and file name to store
**                            the message
**
** Returns          BOOELAN TRUE - operation is successful
**
**
*******************************************************************************/
BTA_API extern BOOLEAN bta_mce_co_get_msg_path(char *handle_str, char *p_path);

#endif /* BTA_MCE_CO_H */
