/*****************************************************************************
**
**  Name:           bta_mse_co.h
**
**  Description:    This is the interface file for the Message Server Equipment
**                  (MSE) subsystem call-out functions.
**
**  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_MSE_CO_H
#define BTA_MSE_CO_H

#include "bta_api.h"
#include "bta_ma_def.h"
#include "bta_mse_api.h"

/*****************************************************************************
**  Constants and Data Types
*****************************************************************************/
/**************************
**  Common Definitions
***************************/


/* Return structure type for a folder entry i.e. only folder level
   information under the specified path and no file name */
typedef struct
{
    void    *refdata;           /* holder for OS specific data used to get next entry */
    UINT32  filesize;
    char    *p_name;            /* Contains the addr of memory to copy name into */
    char    crtime[BTA_FS_CTIME_LEN]; /* "yyyymmddTHHMMSSZ", or "" if none */
    UINT8   mode;               /* BTA_MSE_A_RDONLY and/or BTA_MSE_A_DIR */
} tBTA_MSE_CO_FOLDER_ENTRY;


#define BTA_MSE_CO_MAX_ADDR_LEN  257 /* See MAP spec 3.1.6, and one byte for string terminator */
#define BTA_MSE_CO_MAX_DATE_TIME 21  /* "YYYYMMDDTHHMMSS+hhmm" or "YYYYMMDDTHHMMSS" */

enum
{
    BTA_MSE_CO_RCV_STATUS_COMPLETE = 0,
    BTA_MSE_CO_RCV_STATUS_FRACTIONED,
    BTA_MSE_CO_RCV_STATUS_NOTIFICATION,
    BTA_MSE_CO_RCV_STATUS_MAX
};

typedef UINT8 tBTA_MSE_CO_RCV_STATUS;

typedef struct
{
    UINT16                   msg_list_size;
    UINT8                    mse_time_len;
    char                     mse_time[BTA_MSE_CO_MAX_DATE_TIME];
    BOOLEAN                  new_msg;
} tBTA_MSE_CO_MSG_LIST_INFO;

typedef struct
{
    void                     *refdata;            /* holder for OS specific data used to get next msg entry */
    tBTA_MA_ML_MASK          parameter_mask;
    UINT32                   org_msg_size;
    UINT32                   attachment_size;
    BOOLEAN                  text;
    BOOLEAN                  high_priority;
    BOOLEAN                  read;
    BOOLEAN                  sent;
    BOOLEAN                  is_protected;
    tBTA_MA_MSG_HANDLE       msg_handle;
    tBTA_MA_MSG_TYPE         type;
    tBTA_MSE_CO_RCV_STATUS   reception_status;
    char                     subject[BTA_MSE_CO_MAX_ADDR_LEN];
    char                     date_time[BTA_MSE_CO_MAX_DATE_TIME];  /* "YYYYMMDDTHHMMSS+hhmm", "YYYYMMDDTHHMMSS", or "" if none */
    char                     sender_name[BTA_MSE_CO_MAX_ADDR_LEN];
    char                     sender_addressing[BTA_MSE_CO_MAX_ADDR_LEN];
    char                     recipient_name[BTA_MSE_CO_MAX_ADDR_LEN];
    char                     recipient_addressing[BTA_MSE_CO_MAX_ADDR_LEN];
    char                     replyto_addressing[BTA_MSE_CO_MAX_ADDR_LEN];
} tBTA_MSE_CO_MSG_LIST_ENTRY;

/*******************************************************************************
**
** Function        bta_mse_co_update_inbox
**
** Description     Update the inbox
**
** Parameters      mas_session_id - MAS session ID
**                 app_id         - application ID specified in the enable functions.
**                                  It can be used to identify which application
**                                  is the caller of the call-out function.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mse_co_update_inbox(tBTA_MA_SESS_HANDLE mas_session_id, UINT8 app_id);

/*******************************************************************************
**
** Function        bta_mse_co_set_folder
**
** Description     Set the current foldeer to the specified path
**
** Parameters      mas_session_id - MAS session ID
**                 p_path - points to the current folder path
**                 app_id - application ID specified in the enable functions.
**                          It can be used to identify which application
**                          is the caller of the call-out function.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mse_co_set_folder(tBTA_MA_SESS_HANDLE  mas_session_id,
                           const char *p_path,
                           UINT8 app_id);

/*******************************************************************************
**
** Function         bta_mse_co_get_folder_entry
**
** Description      This function is called to get a folder entry for the
**                  specified path.  The folder name should be filled
**                  into the location specified by p_entry.
**
** Parameters       mas_session_id - MAS session ID
**                  p_path - points to the folder path to get the subfolder entry
**                           (fully qualified path)
**                  first_item - TRUE if first search, FALSE if next search
**                  p_entry (input/output) - Points to the current entry data
**                  evt - event that must be passed into the call-in function.
**                  app_id - application ID specified in the enable functions.
**                          It can be used to identify which application
**                          is the caller of the call-out function.
**
** Returns          void
**
**                  Note: Upon completion of the request, the status is passed
**                        in the bta_mse_ci_get_folder_entry() call-in function.
**                        BTA_MA_STATUS_OK is returned when p_entry is valid,
**                        BTA_MA_STATUS_EODIR is returned when no more entries [finished]
**                        BTA_MA_STATUS_FAIL is returned if an error occurred
**
*******************************************************************************/
BTA_API extern void bta_mse_co_get_folder_entry(tBTA_MA_SESS_HANDLE  mas_session_id, const char *p_path,
                                 BOOLEAN first_item, tBTA_MSE_CO_FOLDER_ENTRY *p_entry,
                                 UINT16 evt, UINT8 app_id);

/*******************************************************************************
**
** Function         bta_mse_co_get_msg_list_info
**
** Description      This function is called to get a message list information for
**                  the specified folder
**
** Parameters       mas_session_id  - MAS session ID
**                  p_name - points to the current or child folder for getting the
**                           message list information
**                          (if *p_name == "" it means current folder)
**                  filter_para - filter parameters for getting the message
**                                list information
**                  p_info(input/output) - Points to the message listing information
**                  evt - event that be passed into the call-in function.
**                  app_id - application ID specified in the enable functions.
**                          It can be used to identify which application
**                          is the caller of the call-out function.
**
** Returns          void
**
**                  Note: Upon completion of the request, the status is passed
**                        in the bta_mse_ci_get_msg_list_info() call-in function.
**                        BTA_MA_STATUS_OK is returned when p_entry is valid
**                        BTA_MA_STATUS_FAIL is returned if an error occurred
**
*******************************************************************************/
BTA_API extern void bta_mse_co_get_msg_list_info(tBTA_MA_SESS_HANDLE  mas_session_id, const char *p_name,
                                  tBTA_MA_MSG_LIST_FILTER_PARAM * p_filter_param,
                                  tBTA_MSE_CO_MSG_LIST_INFO * p_info,
                                  UINT16 evt, UINT8 app_id);
/*******************************************************************************
**
** Function         bta_mse_co_get_msg_list_entry
**
** Description      This function is called to retrieve a message list entry for
**                  the specified folder.  The msg information should be filled by
**                  application into the location specified by p_entry.
**
** Parameters       mas_session_id - MAS session ID
**                  p_name - points to the current or child folder for getting the
**                           message list entry
**                           (if *p_name == "" it means current folder)
**                  filter_para - filter parameters for getting message list
**                  first_item - TRUE if first get, FALSE if next msg
**                  p_entry(input/output)  - Points to current entry data
**                  evt - event that must be passed into the call-in function.
**                  app_id - application ID specified in the enable functions.
**                          It can be used to identify which application
**                          is the caller of the call-out function.
**
** Returns          void
**
**                  Note: Upon completion of the request, the status is passed
**                        in the bta_mse_ci_get_msg_list_entry() call-in function.
**                        BTA_MA_STATUS_OK is returned when p_entry is valid,
**                        BTA_MA_STATUS_EODIR is returned when no more entries [finished]
**                        BTA_MA_STATUS_FAIL is returned if an error occurred
*******************************************************************************/
BTA_API extern void bta_mse_co_get_msg_list_entry(tBTA_MA_SESS_HANDLE  mas_session_id, const char *p_name,
                                   tBTA_MA_MSG_LIST_FILTER_PARAM    *p_filter_param,
                                   BOOLEAN first_item, tBTA_MSE_CO_MSG_LIST_ENTRY *p_entry,
                                   UINT16 evt, UINT8 app_id);
/*******************************************************************************
**
** Function         bta_mse_co_get_msg
**
** Description      This function is called to retrieve a msessage for the
**                  specified message handle. The message will be filled by
**                  application in bMessage format into the location
**                  specified by p_buffer. The status and size of the filled
**                  buffer are returned by bta_mse_ci_get_msg() call-in function
**
**
** Parameters       mas_session_id - MAS session ID
**                  p_param  - points to the parameters for the get message operation
**                  first_get - TRUE first get FALSE subsequent get
**                  buffer_size - size of the buffer pointed by p_buffer
**                  p_buffer - points to the bMessage object storage location
**                  evt - event that be passed into the call-in function.
**                  app_id - application ID specified in the enable functions.
**                          It can be used to identify which application
**                          is the caller of the call-out function.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mse_co_get_msg(tBTA_MA_SESS_HANDLE  mas_session_id,
                        tBTA_MA_GET_MSG_PARAM *p_param,
                        BOOLEAN first_get,
                        UINT16 buffer_size,
                        UINT8 *p_buffer,
                        UINT16 evt,
                        UINT8 app_id);

/*******************************************************************************
**
** Function         bta_mse_co_set_msg_delete_status
**
** Description      This function is called to set a message delete status
**
** Parameters       mas_session_id - MAS session ID
**                  handle - message handle
**                  status value - 1- yes, 0 = No
**                  evt_id - event that be passed into the call-in function.
**                  app_id - application ID specified in the enable functions.
**                          It can be used to identify which application
**                          is the caller of the call-out function.
**
** Returns          void
**
**                  Note: Upon completion of the request, the status is passed
**                        in the bta_mse_ci_set_msg_delete_status() call-in function.
**                        BTA_MA_STATUS_OK is returned when the delete operation is successful
**                        BTA_MA_STATUS_FAIL is returned if an error occurred
*******************************************************************************/
BTA_API extern void bta_mse_co_set_msg_delete_status(tBTA_MA_SESS_HANDLE  mas_session_id,
                                      tBTA_MA_MSG_HANDLE handle,
                                      UINT8 status_value,
                                      UINT16 evt_id,
                                      UINT8 app_id);

/*******************************************************************************
**
** Function         bta_mse_co_set_msg_read_status
**
** Description      This function is called to set a message read status
**
** Parameters       mas_session_id - MAS session ID
**                  handle - message handle
**                  status value - 1- yes, 0 = No
**                  app_id - application ID specified in the enable functions.
**                          It can be used to identify which application
**                          is the caller of the call-out function.
**
** Returns          BTA_MA_STATUS_OK - read status change is successful
**                  BTA_MA_STATUS_FAIL
*******************************************************************************/
BTA_API extern tBTA_MA_STATUS bta_mse_co_set_msg_read_status(tBTA_MA_SESS_HANDLE  mas_session_id,
                                              tBTA_MA_MSG_HANDLE handle,
                                              UINT8 status_value,
                                              UINT8 app_id);

/*******************************************************************************
**
** Function         bta_mse_co_push_msg
**
** Description      This function is called to upload a message to the
**                  specified folder
**
** Parameters       mas_session_id - MAS session ID
**                  p_param - points to parameters for message upload
**                  msg_len - length of the message to be uploaded
**                  p_msg - points to the message to be uploaded
**                  first_pkt - TRUE first push message packet
**                  multi_pkt_status -
**                       BTA_MA_MPKT_STATUS_MORE - need to get more packets
**                       BTA_MA_MPKT_STATUS_LAST - last packet of
**                                                 the bMessage to be uploaded
**                  evt - event that be passed into the call-in function.
**                  app_id - application ID specified in the enable functions.
**                          It can be used to identify which application
**                          is the caller of the call-out function.
**
** Returns          void
**
**                  Note1: Upon completion of the request, the status is passed
**                        in the bta_mse_ci_push_msg() call-in function.
**                        BTA_MA_STATUS_OK is returned if the request is successful,
**                        BTA_MA_STATUS_FAIL is returned if an error occurred
**
*******************************************************************************/
BTA_API extern void bta_mse_co_push_msg(tBTA_MA_SESS_HANDLE  mas_session_id,
                         tBTA_MA_PUSH_MSG_PARAM *p_param,
                         UINT16 msg_len,
                         UINT8 *p_msg,
                         BOOLEAN first_pkt,
                         tBTA_MA_MPKT_STATUS mpkt_status,
                         UINT16 evt,
                         UINT8 app_id);

/*******************************************************************************
**
** Function         bta_mse_co_init_msg_cb
**
** Description      This function is called to initialize the msg control block
**
** Parameters
**
** Returns
*******************************************************************************/
BTA_API extern void bta_mse_co_init_msg_cb(void);


#endif /* BTA_MSE_CO_H */
