/*****************************************************************************
**
**  Name:           bta_pbs_co.h
**
**  Description:    This is the interface file for the phone book access server
**                  call-out functions.
**
**  Copyright (c) 2003-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_PBS_CO_H
#define BTA_PBS_CO_H

#include "bta_api.h"
#include "goep_fs.h"
#include "bta_pbs_api.h"


/*****************************************************************************
**  Constants and Data Types
*****************************************************************************/
/**************************
**  Common Definitions
***************************/

/* Status codes returned by call-out functions, or in call-in functions as status */
#define BTA_PBS_CO_OK            GOEP_OK
#define BTA_PBS_CO_FAIL          GOEP_FAIL   /* Used to pass all other errors */
#define BTA_PBS_CO_EACCES        GOEP_EACCES
#define BTA_PBS_CO_EOF           GOEP_EOF
#define BTA_PBS_CO_EODIR         GOEP_EODIR

typedef UINT16 tBTA_PBS_CO_STATUS;

#define BTA_PBS_LEN_UNKNOWN      GOEP_LEN_UNKNOWN
#define BTA_PBS_INVALID_FD       GOEP_INVALID_FD
#define BTA_PBS_INVALID_APP_ID   (0xFF)  /* this app_id is reserved */

/* BSA_SPECIFIC */
/* Bit mask for get phonebook info */
#define BTA_PBS_CO_GET_PB_SIZE      0x0001      /* get phonebook size */
#define BTA_PBS_CO_GET_MISSED_CALL  0x0002      /* get new missed call */
#define BTA_PBS_CO_GET_PRI_VER      0x0004      /* get folder primary version */
#define BTA_PBS_CO_GET_SEC_VER      0x0008      /* get folder secondary version */
#define BTA_PBS_CO_GET_DB_ID        0x0010      /* get database ID */

typedef UINT16 tBTA_PBS_CO_PB_INFO_MASK;

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
/**************************
**  Common Functions
***************************/
/*******************************************************************************
**
** Function         bta_pbs_co_open
**
** Description      This function is executed by BTA when a pb file is requested to be opened.
**                  The phone book access profile server uses this function to open
**                  a file for reading on two phone book access operations
**                  (pull pb or pull pb entry)
**
** Parameters       p_path  - path and file name.
**                  operation - BTA_PB_OPER_PULL_PB or BTA_PB_OPER_PULL_VCARD_ENTRY
**                  p_app_params - obex application params
**
**
** Returns          void
**
**                  Note: Upon completion of the request
**                        if successful, and an error code (tBTA_PBS_CO_STATUS)
**                        are returned in the call-in function, bta_pbs_ci_open().
**
*******************************************************************************/
BTA_API extern void bta_pbs_co_open(const char *p_path, tBTA_PBS_OPER operation, tBTA_PBS_PULLPB_APP_PARAMS *p_app_params);

/*******************************************************************************
**
** Function         bta_pbs_co_close
**
** Description      This function is called by BTA when a connection to a
**                  client is closed.
**
** Parameters       fd      - file descriptor of file to close.
**
**
** Returns          (tBTA_PBS_CO_STATUS) status of the call.
**                      [BTA_PBS_CO_OK if successful],
**                      [BTA_PBS_CO_FAIL if failed  ]
**
*******************************************************************************/
BTA_API extern tBTA_PBS_CO_STATUS bta_pbs_co_close(int fd);

/* BSA_SPECIFIC */
/*******************************************************************************
**
** Function         bta_pbs_co_get_pbinfo
**
** Description      This function is called by BTA to inquire about pb size and new missed calls.
**
** Parameters       p_name: which type of phone book object Eg. telecom/pb.vcf, telecom/ich.vcf
**                  operation: phone book operation type Eg. BTA_PBS_OPER_PULL_PB
**                  obj_type: phone book repository type Eg. BTA_PBS_MCH_OBJ
**
** Returns          pb_size - phone book size
**                  new_missed_call - new missed calls
*                       (tBTA_PBS_CO_STATUS) status of the call.
**                      [BTA_PBS_CO_OK if successful],
**                      [BTA_PBS_CO_FAIL if failed  ]
**
*******************************************************************************/
BTA_API extern tBTA_PBS_CO_STATUS bta_pbs_co_getpbinfo(tBTA_PBS_OPER operation,
                                tBTA_PBS_OBJ_TYPE obj_type, BOOLEAN is_selector,
                                const char *p_path, tBTA_PBS_VCARDLIST_APP_PARAMS *p_app_params,
                                tBTA_PBS_CO_PB_INFO_MASK mask);

/*******************************************************************************
**
** Function         bta_pbs_co_read
**
** Description      This function is called by BTA to read in data from the
**                  previously opened pb file on the phone.
**                  the application callin should fill in the PB object needed to be
**                  send to the client
**
** Parameters       fd      - file descriptor of file to read from.
**                  operation - BTA_PBS_OPER_PULL_PB or BTA_PBS_OPER_PULL_VCARD_ENTRY
**                  p_buf   - buffer to read the data into.
**                  nbytes  - number of bytes to read into the buffer.
**
**
** Returns          void
**
**                  Note: Upon completion of the request, bta_pbs_ci_read() is
**                        called with the buffer of data, along with the number
**                        of bytes read into the buffer, and a status.
**
*******************************************************************************/
BTA_API extern void bta_pbs_co_read(int fd, tBTA_PBS_OPER operation, UINT8 *p_buf, UINT16 nbytes);

/*******************************************************************************
**
** Function         bta_pbs_co_reset_newmissedcalls
**
** Description      This function is called by BTA to reset newmisscalls.
**
** Parameters       None
**
** Returns          Status: whether reset newmissedcalls is successful or not
**
*******************************************************************************/
BTA_API extern tBTA_PBS_CO_STATUS bta_pbs_co_reset_newmissedcalls(void);

/* BSA_SPECIFIC */

/*******************************************************************************
**
** Function         bta_pbs_co_getvlist
**
** Description      This function is called to retrieve a vcard list entry for the
**                  specified path.
**                  The first/next directory should be filled by application
**                  into the location specified by p_entry.
**
** Parameters       p_path      - directory to search
**                  p_app_params - Obex application params, NULL if first_item is FALSE
**                  first_item  - TRUE if first get, FALSE if next getvlist
**                                      (p_entry contains previous)
**                  p_entry(input/output)  - Points to the dynamically allocated memory,
**                                           the callout application need to fill in with
**                                           the listing entries
**                  size - size of the dynamically allocated memory
**
**
** Returns          void
**
**                  Note: Upon completion of the request, bta_pbs_ci_getvlist() is
**                        called with the a status and final flag.
**
**
**
*******************************************************************************/
BTA_API extern void bta_pbs_co_getvlist(const char *p_path, tBTA_PBS_VCARDLIST_APP_PARAMS *p_app_params,
                                        BOOLEAN first_item, tBTA_PBS_VCARDLIST *p_entry, UINT16 size);


#endif /* BTA_PBS_CO_H */
