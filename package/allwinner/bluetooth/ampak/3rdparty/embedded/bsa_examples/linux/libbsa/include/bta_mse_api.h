/*****************************************************************************
**
**  Name:           bta_mse_api.h
**
**  Description:    This is the public interface file for the Message Server Equipment
**                  (MSE) subsystem of BTA, Broadcom's
**                  Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2009-2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_MSE_API_H
#define BTA_MSE_API_H

#include "bta_api.h"
#include "bta_ma_def.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* Extra Debug Code */
#ifndef BTA_MSE_DEBUG
#define BTA_MSE_DEBUG           TRUE
#endif

/**************************
**  Common Definitions
***************************/

/* MSE callback function events */
enum
{
    BTA_MSE_ENABLE_EVT            = 0,   /* MAS server is enabled. */
    BTA_MSE_DISABLE_EVT,
    BTA_MSE_ACCESS_EVT,
    BTA_MSE_OPER_CMPL_EVT,             /* MSE operation complete */
    BTA_MSE_MA_OPEN_EVT,
    BTA_MSE_MA_CLOSE_EVT,
    BTA_MSE_SET_NOTIF_REG_EVT,
    BTA_MSE_NOTIF_REG_EVT,
    BTA_MSE_UPDATE_INBOX_EVT,
    BTA_MSE_GET_MSG_IN_PROG_EVT,
    BTA_MSE_PUSH_MSG_IN_PROG_EVT,
    BTA_MSE_SEND_NOTIF_EVT,
    BTA_MSE_MN_OPEN_EVT,
    BTA_MSE_MN_CLOSE_EVT,
    BTA_MSE_START_EVT,
    BTA_MSE_STOP_EVT
};
typedef UINT8 tBTA_MSE_EVT;


/* Structure associated with BTA_MSE_ENABLE_EVT
   BTA_MSE_DISABLE_EVT
*/
typedef struct
{
    tBTA_MA_STATUS  status;
    UINT8           app_id;
} tBTA_MSE_ENABLE_DISABLE;


/*******************************************
**  Message Access Server Definitions
********************************************/
/* Access event operation types */
#define BTA_MSE_OPER_NONE               0
#define BTA_MSE_OPER_SETPATH            1
#define BTA_MSE_OPER_GET_FOLDER_LIST    2
#define BTA_MSE_OPER_GET_MSG_LIST       3
#define BTA_MSE_OPER_GET_MSG            4
#define BTA_MSE_OPER_SET_MSG_STATUS     5
#define BTA_MSE_OPER_DEL_MSG            6
#define BTA_MSE_OPER_PUSH_MSG           7
#define BTA_MSE_OPER_NOTIF_REG          8
#define BTA_MSE_OPER_UPDATE_INBOX       9

typedef UINT8 tBTA_MSE_OPER;

/* Structure associated with BTA_MSE_ACCESS_EVT */
typedef struct
{
    tBTA_MA_SESS_HANDLE         mas_session_id; /* MAS Session ID */
    tBTA_MSE_OPER               oper;           /* operation */
    char                        *p_path;        /* full path name  */
    tBTM_BD_NAME                dev_name;       /* Name of device, "" if unknown */
    BD_ADDR                     bd_addr;        /* Address of device */
    tBTA_MA_MSG_HANDLE          handle;
    BOOLEAN                     delete_sts;
} tBTA_MSE_ACCESS;

/* Structure associated with BTA_MSE_OPER_CMPL_EVT */
typedef struct
{
    UINT32              obj_size;       /* Total size of object 0 if unknow*/
    tBTA_MA_SESS_HANDLE mas_session_id; /* MAS Session ID */
    tBTA_MSE_OPER       operation;
    tBTA_MA_STATUS      status;
} tBTA_MSE_OPER_CMPL;


/* Structure associated with BTA_MSE_MA_OPEN_EVT */
typedef struct
{
    tBTA_MA_INST_ID     mas_instance_id;    /* MAS instance ID: one MAS
                                               instance can support multiple
                                               sessions */
    tBTA_MA_SESS_HANDLE mas_session_id;      /* MAS Session ID, all session based
                                              operation will need to use this ID */
    tBTM_BD_NAME        dev_name;           /* Name of device, "" if unknown */
    BD_ADDR             bd_addr;            /* Address of device */

} tBTA_MSE_MA_OPEN;

/* Structure associated with BTA_MSE_MA_CLOSE_EVT */
typedef struct
{
    tBTA_MA_STATUS      status;
    tBTA_MA_SESS_HANDLE mas_session_id;
    tBTA_MA_INST_ID     mas_instance_id;
} tBTA_MSE_MA_CLOSE;

/* Structure associated with BTA_MSE_MN_OPEN_EVT or
   BTA_MSE_MN_CLOSE_EVT
*/
typedef struct
{
    tBTM_BD_NAME    dev_name;       /* Name of device, "" if unknown */
    BD_ADDR         bd_addr;        /* Address of device */
    tBTA_MA_INST_ID first_mas_instance_id; /* MN connection can be used for more
                                              than one MasInstanceIDs.
                                              first_mas_instance_id is the MasInstanceId of a
                                              MAS service MCE first registers to receive the
                                              message notification */
} tBTA_MSE_MN_OPEN;

typedef struct
{
    tBTM_BD_NAME    dev_name;       /* Name of device, "" if unknown */
    BD_ADDR         bd_addr;        /* Address of device */
} tBTA_MSE_MN_CLOSE;

/* Update Inbox response types */
enum
{
    BTA_MSE_UPDATE_INBOX_ALLOW = 0,    /* Allow the update inbox */
    BTA_MSE_UPDATE_INBOX_FORBID        /* Disallow the update inbox */
};
typedef UINT8 tBTA_MSE_UPDATE_INBOX_TYPE;


/* Set Notification Registration response types */
enum
{
    BTA_MSE_SET_NOTIF_REG_ALLOW = 0,    /* Allow the notification registration request*/
    BTA_MSE_SET_NOTIF_REG_FORBID        /* Disallow the notification registration request */
};
typedef UINT8 tBTA_MSE_SET_NOTIF_REG_TYPE;

/* Structure associated with BTA_MSE_SEND_NOTIF_EVT */
typedef struct
{
    tBTA_MA_STATUS          status;
    tBTA_MA_INST_ID         mas_instance_id;
    BD_ADDR                 bd_addr;  /* remote MCE's BD address*/
} tBTA_MSE_SEND_NOTIF;


/* Structure associated with BTA_MSE_START_EVT or
   BTA_MSE_STOP_EVT                               */
typedef struct
{
    tBTA_MA_STATUS      status;
    UINT8               mas_instance_id;
} tBTA_MSE_START_STOP;

typedef struct
{
    tBTA_MA_SESS_HANDLE     mas_session_id;
    tBTA_MA_INST_ID         mas_instance_id;
    tBTA_MA_NOTIF_STATUS    notif_status;
    BD_ADDR                 bd_addr;  /* remote MCE's BD address*/
}tBTA_MSE_SET_NOTIF_REG;

typedef struct
{
    tBTA_MA_STATUS          status;
    tBTA_MA_SESS_HANDLE     mas_session_id;
    tBTA_MA_INST_ID         mas_instance_id;
    tBTA_MA_NOTIF_STATUS    notif_status;
    BD_ADDR                 bd_addr;  /* remote MCE's BD address*/
}tBTA_MSE_NOTIF_REG;




/* Data associated with call back events */
typedef union
{
    tBTA_MA_SESS_HANDLE     mas_session_id; /* BTA_MSE_UPDATE_INBOX_EVT*/
    tBTA_MSE_ENABLE_DISABLE enable;
    tBTA_MSE_ENABLE_DISABLE disable;
    tBTA_MSE_ACCESS         access;
    tBTA_MSE_SET_NOTIF_REG  set_notif_reg;
    tBTA_MSE_NOTIF_REG      notif_reg;
    tBTA_MSE_MN_OPEN        mn_open;
    tBTA_MSE_MN_CLOSE       mn_close;
    tBTA_MSE_MA_OPEN        ma_open;
    tBTA_MSE_MA_CLOSE       ma_close;
    tBTA_MSE_SEND_NOTIF     send_notif;
    tBTA_MA_IN_PROG         get_msg_in_prog;
    tBTA_MA_IN_PROG         push_msg_in_prog;
    tBTA_MSE_OPER_CMPL      oper_cmpl;
    tBTA_MSE_START_STOP     start;
    tBTA_MSE_START_STOP     stop;
} tBTA_MSE;

/* MSE callback function */
typedef void tBTA_MSE_CBACK(tBTA_MSE_EVT event, tBTA_MSE *p_data);


/********************************************
**  Message Notification Client Definitions
*********************************************/

/* Send Notification function arguments definition */
enum
{
    BTA_MSE_NOTIF_TYPE_NEW_MSG = 0,
    BTA_MSE_NOTIF_TYPE_DELIVERY_SUCCESS,
    BTA_MSE_NOTIF_TYPE_SENDING_SUCCESS,
    BTA_MSE_NOTIF_TYPE_DELIVERY_FAILURE,
    BTA_MSE_NOTIF_TYPE_SENDING_FAILURE,
    BTA_MSE_NOTIF_TYPE_MEMORY_FULL,
    BTA_MSE_NOTIF_TYPE_MEMORY_AVAILABLE,
    BTA_MSE_NOTIF_TYPE_MESSAGE_DELETED,
    BTA_MSE_NOTIF_TYPE_MESSAGE_SHIFT,
    BTA_MSE_NOTIF_TYPE_READ_STATUS_CHANGED,
    BTA_MSE_NOTIF_TYPE_MAX
};

typedef UINT8 tBTA_MSE_NOTIF_TYPE;

/* configuration related constants */
/* MSE configuration data */
#define BTA_MSE_NUM_INST    4
#define BTA_MSE_NUM_SESS    4
#define BTA_MSE_NUM_MN      7  /* i.e. up to 7 MCEs can be connected to the MSE */


typedef struct
{
    INT32   obx_rsp_tout;           /* maximum amount of time to wait for obx rsp */
    UINT16  max_name_len;           /* maximum folder name length */
} tBTA_MSE_CFG;


typedef struct
{
    tBTA_MA_INST_ID		    mas_instance_id;
    tBTA_MSE_NOTIF_TYPE	    notif_type;
    tBTA_MA_MSG_HANDLE	    handle;
    char			        folder[BTA_MA_NAME_LEN+1];
    char			        old_folder[BTA_MA_NAME_LEN+1];
    tBTA_MA_MSG_TYPE        msg_type;
    BD_ADDR				    except_bd_addr;
    char                    datetime[BTA_MA_DATETIME_SIZE+1]; /* In the format of YEAR MONTH DATA T HOURS MINUTES SECONDS "20120822T100000" */
    char                    subject[BTA_MA_SUBJECT_SIZE+1];
    char                    sender_name[BTA_MA_NAME_LEN+1];
    char                    priority[BTA_MA_PRIORITY_SIZE+1];

}tBTA_MSE_SEND_ENH_MSG_NOTIF;


extern tBTA_MSE_CFG *p_bta_mse_cfg;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/**************************
**  API Functions
***************************/

/*******************************************************************************
**
** Function         BTA_MseEnable
**
** Description      Enable the MSE subsystems.  This function must be
**                  called before any other functions in the MSE API are called.
**                  When the enable operation is completed the callback function
**                  will be called with an BTA_MSE_ENABLE_EVT event.
**
** Parameters       p_cback - MSE event call back function
**                  app_id  - Application ID
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseEnable(tBTA_MSE_CBACK *p_cback, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_MseDisable
**
** Description     Disable the MSE subsystem.
**
**  Parameters
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseDisable(void);

/*******************************************************************************
**
** Function         BTA_MseStart
**
** Description      Start a MA server on the MSE
**
**
** Parameters       mas_inst_id     - MAS instance ID
**                  sec_mask        - Security Setting Mask
**                                     MSE always enables
**                                    (BTA_SEC_AUTHENTICATE | BTA_SEC_ENCRYPT)
**                  p_service_name  - Pointer to service name
**                  p_root_path     - Pointer to root path
**                                    (one level above telecom)
**                  sup_msg_type    - supported message type(s)
**                  p_mas_inst_info - MAS instance information
**                  features        - Local supported featues
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseStart(tBTA_MA_INST_ID  mas_inst_id, tBTA_SEC sec_mask,
                                 const char *p_service_name, const char *p_root_path,
                                 tBTA_MA_MSG_TYPE sup_msg_type,
                                 const char *p_mas_inst_info, tBTA_MA_SUPPORTED_FEATURES features);

/*******************************************************************************
**
** Function         BTA_MseStop
**
** Description      Stop a MAS service on the MSE
**
** Parameters       mas_instance_id - MAS Instance ID
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseStop(tBTA_MA_INST_ID mas_instance_id);

/*******************************************************************************
**
** Function         BTA_MseClose
**
** Description      Close all MAS sessions on the specified MAS Instance ID
**
** Parameters       mas_instance_id - MAS Inatance ID
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseClose(tBTA_MA_INST_ID mas_instance_id);

/*******************************************************************************
**
** Function         BTA_MseMaClose
**
** Description      Close a MAS sessions on the specified BD address
**
** Parameters       bd_addr         - remote BD's address
**                  mas_instance_id - MAS Inatance ID
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseMaClose(BD_ADDR bd_addr, tBTA_MA_INST_ID mas_instance_id);

/*******************************************************************************
**
** Function         BTA_MseMnClose
**
** Description       Close a MN session
**
** Parameters        bd_addr - remote BT's address
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseMnClose(BD_ADDR bd_addr);

/*******************************************************************************
**
** Function         BTA_MseAccessRsp
**
** Description      Send a response for the access request
**
** Parameters       mas_session_id  - MAS session ID
**                  oper            - MAS operation type
**                  access          - Access is allowed or not
**                  p_path          - pointer to a path if if the operation
**                                    involves accessing a folder
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseAccessRsp(tBTA_MA_SESS_HANDLE mas_session_id, tBTA_MSE_OPER oper,
                                     tBTA_MA_ACCESS_TYPE access, char *p_name);

/*******************************************************************************
**
** Function         BTA_MseUpdateInboxRsp
**
** Description      Send a response for the update inbox request
**
**
** Parameters       mas_session_id  - MAS session ID
**                  update_rsp      - update inbox is allowed or not
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseUpdateInboxRsp(tBTA_MA_SESS_HANDLE mas_session_id,
                                          tBTA_MSE_UPDATE_INBOX_TYPE update_rsp);

/*******************************************************************************
**
** Function         BTA_MseSetNotifRegRsp
**
** Description      Send a response for the set notification registration
**
**
** Parameters       mas_session_id  - MAS session ID
**                  set_notif_reg_rsp   - indicate whether the set notification
**                                        registration is allowed or not
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseSetNotifRegRsp(tBTA_MA_SESS_HANDLE mas_session_id,
                                          tBTA_MSE_SET_NOTIF_REG_TYPE set_notif_reg_rsp);

/*******************************************************************************
**
** Function         BTA_MseSendEnhNotif
**
** Description      Send a Message notification report to all MCEs registered with
**                  the specified MAS instance ID
**
** Parameters       The following parameters are in the tBTA_MSE_SEND_ENH_MSG_NOTIF
**                  mas_instance_id - MAS Instance ID
**                  notif_type      - message notification type
**                  handle          - message handle
**                  folder          - pointer to current folder
**                  old_folder      - pointer to older folder
**                  msg_type        - message type (E_MAIL, SMS_GSM, SMS_CDMA, MMS)
**                  except_bd_addr  - except to the MCE on this BD Address.
**                                    (Note: notification will be not sent to
**                                     this BD Addreess)
**                  datetime        - date and time
**                  subject         - message subject
**                  sender_name     - sender name
**                  priority        - whether a priority message, yes or no
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseSendEnhNotif(tBTA_MSE_SEND_ENH_MSG_NOTIF *p_enh_msg_notif_param);


/*******************************************************************************
**
** Function         BTA_MseSendNotif
**
** Description      Send a Message notification report to all MCEs registered with
**                  the specified MAS instance ID
**
** Parameters
                    mas_instance_id - MAS Instance ID
**                  notif_type      - message notification type
**                  handle          - message handle
**                  p_folder        - pointer to current folder
**                  p_old_folder    - pointer to older folder
**                  msg_type        - message type (E_MAIL, SMS_GSM, SMS_CDMA, MMS)
**                  except_bd_addr  - except to the MCE on this BD Address.
**                                    (Note: notification will be not sent to
**                                     this BD Addreess)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseSendNotif(tBTA_MA_INST_ID mas_instance_id,
                                     tBTA_MSE_NOTIF_TYPE notif_type,
                                     tBTA_MA_MSG_HANDLE handle,
                                     char * p_folder, char *p_old_folder,
                                     tBTA_MA_MSG_TYPE msg_type,
                                     BD_ADDR except_bd_addr);

/*******************************************************************************
**
** Function         BTA_MseMnAbort
**
** Description      Abort the current OBEX multi-packt operation
**
** Parameters       mas_instance_id - MAS Instance ID
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MseMnAbort(tBTA_MA_INST_ID mas_instance_id);

#ifdef __cplusplus
}
#endif

#endif /* BTA_MSE_API_H */
