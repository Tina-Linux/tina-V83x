/*****************************************************************************
**
**  Name:           bta_pbc_co.h
**
**  Description:    This is the interface file for the PBAP client call-out functions.
**
**  Copyright (c) 2003-2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_PBC_CO_H
#define BTA_PBC_CO_H

#include <time.h>

#include "bta_api.h"
#include "goep_fs.h"
#include "obx_api.h"

/*****************************************************************************
**  Constants and Data Types
*****************************************************************************/

#ifndef BTA_PBC_CO_MAX_SSN_ENTRIES
#define BTA_PBC_CO_MAX_SSN_ENTRIES   10
#endif

/* Maximum path length supported by FS_CO */
#ifndef BTA_PBC_CO_PATH_LEN
#define BTA_PBC_CO_PATH_LEN          294
#endif

#ifndef BTA_PBC_CO_TEST_ROOT
#define BTA_PBC_CO_TEST_ROOT         "test_files"
#endif

#define BTA_PBC_CO_TEST_TYPE_NONE    0
#define BTA_PBC_CO_TEST_TYPE_REJECT  1
#define BTA_PBC_CO_TEST_TYPE_SUSPEND 2

#ifndef BTA_PBC_CO_TEST_AB_END
#define BTA_PBC_CO_TEST_AB_END   BTA_PBC_CO_TEST_TYPE_NONE
#endif

/**************************
**  Common Definitions
***************************/

/* Status codes returned by call-out functions, or in call-in functions as status */
#define BTA_PBC_CO_OK            GOEP_OK
#define BTA_PBC_CO_FAIL          GOEP_FAIL   /* Used to pass all other errors */
#define BTA_PBC_CO_EACCES        GOEP_EACCES
#define BTA_PBC_CO_ENOTEMPTY     GOEP_ENOTEMPTY
#define BTA_PBC_CO_EOF           GOEP_EOF
#define BTA_PBC_CO_EODIR         GOEP_EODIR
#define BTA_PBC_CO_ENOSPACE      GOEP_ENOSPACE/* Returned in bta_pbc_ci_open if no room */
#define BTA_PBC_CO_EIS_DIR       GOEP_EIS_DIR
#define BTA_PBC_CO_RESUME        GOEP_RESUME /* used in ci_open, on resume */
#define BTA_PBC_CO_NONE          GOEP_NONE /* used in ci_open, on resume (no file to resume) */

typedef UINT16 tBTA_PBC_CO_STATUS;

/* the index to the permission flags */
#define BTA_PBC_PERM_USER    0
#define BTA_PBC_PERM_GROUP   1
#define BTA_PBC_PERM_OTHER   2
/* max number of the permission flags */
#define BTA_PBC_PERM_SIZE    3

/* Flags passed to the open function (bta_pbc_co_open)
**      Values are OR'd together. (First 3 are
**      mutually exclusive.
*/
#define BTA_PBC_O_RDONLY         GOEP_O_RDONLY
#define BTA_PBC_O_WRONLY         GOEP_O_WRONLY
#define BTA_PBC_O_RDWR           GOEP_O_RDWR

#define BTA_PBC_O_CREAT          GOEP_O_CREAT
#define BTA_PBC_O_EXCL           GOEP_O_EXCL
#define BTA_PBC_O_TRUNC          GOEP_O_TRUNC

#define BTA_PBC_O_MODE_MASK(x)      (((UINT16)(x)) & 0x0003)

/* Origin for the bta_pbc_co_seek function  */
#define BTA_PBC_SEEK_SET         GOEP_SEEK_SET
#define BTA_PBC_SEEK_CUR         GOEP_SEEK_CUR
#define BTA_PBC_SEEK_END         GOEP_SEEK_END

/* mode field in bta_pbc_co_access callout */
#define BTA_PBC_ACC_EXIST        GOEP_ACC_EXIST
#define BTA_PBC_ACC_READ         GOEP_ACC_READ
#define BTA_PBC_ACC_RDWR         GOEP_ACC_RDWR

#define BTA_PBC_LEN_UNKNOWN      GOEP_LEN_UNKNOWN
#define BTA_PBC_INVALID_FD       GOEP_INVALID_FD
#define BTA_PBC_INVALID_APP_ID   (0xFF)  /* this app_id is reserved */

/* mode field in tBTA_PBC_DIRENTRY (OR'd together) */
#define BTA_PBC_A_RDONLY         GOEP_A_RDONLY
#define BTA_PBC_A_DIR            GOEP_A_DIR      /* Entry is a sub directory */

#define BTA_PBC_CTIME_LEN        GOEP_CTIME_LEN  /* Creation time "yyyymmddTHHMMSSZ" */

/*****************************************************************************
**  Function Declarations
*****************************************************************************/


/*******************************************************************************
**
** Function         bta_pbc_co_open
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
**
** Returns          void
**
**                  Note: Upon completion of the request, a file descriptor (int),
**                        if successful, and an error code (tBTA_PBC_CO_STATUS)
**                        are returned in the call-in function, bta_pbc_ci_open().
**
*******************************************************************************/
// BTA_API extern void bta_pbc_co_open(const char *p_path, int oflags, UINT32 size,
                           // UINT16 evt, UINT8 app_id);
void bta_pbc_co_open(UINT16 evt, UINT8 app_id);

/*******************************************************************************
**
** Function         bta_pbc_co_close
**
** Description      This function is called by BTA when a connection to a
**                  client is closed.
**
** Parameters       fd      - file descriptor of file to close.
**                  app_id  - application ID specified in the enable functions.
**                            It can be used to identify which profile is the caller
**                            of the call-out function.
**
** Returns          (tBTA_PBC_CO_STATUS) status of the call.
**                      [BTA_PBC_CO_OK if successful],
**                      [BTA_PBC_CO_FAIL if failed  ]
**
*******************************************************************************/
BTA_API extern tBTA_PBC_CO_STATUS bta_pbc_co_close(int fd, UINT8 app_id);

/*******************************************************************************
**
** Function         bta_pbc_co_read
**
** Description      This function is called by BTA to read in data from the
**                  previously opened file on the phone.
**
** Parameters       fd      - file descriptor of file to read from.
**                  p_buf   - buffer to read the data into.
**                  nbytes  - number of bytes to read into the buffer.
**                  evt     - event that must be passed into the call-in function.
**                  ssn     - session sequence number. Ignored, if bta_pbc_co_open
**                            was not called with BTA_PBC_CO_RELIABLE.
**                  app_id  - application ID specified in the enable functions.
**                            It can be used to identify which profile is the caller
**                            of the call-out function.
**
** Returns          void
**
**                  Note: Upon completion of the request, bta_pbc_ci_read() is
**                        called with the buffer of data, along with the number
**                        of bytes read into the buffer, and a status.  The
**                        call-in function should only be called when ALL requested
**                        bytes have been read, the end of file has been detected,
**                        or an error has occurred.
**
*******************************************************************************/
BTA_API extern void bta_pbc_co_read(int fd, UINT8 *p_buf, UINT16 nbytes, UINT16 evt,
                           UINT8 ssn, UINT8 app_id);

/*******************************************************************************
**
** Function         bta_pbc_co_write
**
** Description      This function is called by io to send file data to the
**                  phone.
**
** Parameters       fd      - file descriptor of file to write to.
**                  p_buf   - buffer to read the data from.
**                  nbytes  - number of bytes to write out to the file.
**                  evt     - event that must be passed into the call-in function.
**                  ssn     - session sequence number. Ignored, if bta_pbc_co_open
**                            was not called with BTA_PBC_CO_RELIABLE.
**                  app_id  - application ID specified in the enable functions.
**                            It can be used to identify which profile is the caller
**                            of the call-out function.
**
** Returns          void
**
**                  Note: Upon completion of the request, bta_pbc_ci_write() is
**                        called with the file descriptor and the status.  The
**                        call-in function should only be called when ALL requested
**                        bytes have been written, or an error has been detected,
**
*******************************************************************************/
BTA_API extern void bta_pbc_co_write(int fd, const UINT8 *p_buf, UINT16 nbytes, UINT16 evt,
                            UINT8 ssn, UINT8 app_id);

#endif /* BTA_PBC_CO_H */
