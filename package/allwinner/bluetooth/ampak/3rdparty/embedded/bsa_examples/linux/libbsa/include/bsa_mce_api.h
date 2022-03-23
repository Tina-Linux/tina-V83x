/*****************************************************************************
**
**  Name:           bsa_mce_api.h
**
**  Description:    This is the public interface file for MAP client part of
**                  the Bluetooth simplified API
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BSA_MCE_API_H
#define BSA_MCE_API_H

#include "uipc.h"

/* for tBSA_STATUS */
#include "bsa_status.h"

/*****************************************************************************
**  Constants and Type Definitions
*****************************************************************************/
#define BSA_MCE_FILENAME_MAX                255
#define BSA_MCE_SERVICE_NAME_LEN_MAX        150
#define BSA_MCE_ROOT_PATH_LEN_MAX           255
#define BSA_MCE_MAX_VALUE_LEN               512
#define BSA_MCE_HANDLE_SIZE                 8
#define BSA_MCE_MN_MAX_MSG_EVT_OBECT_SIZE   960
#define BSA_MCE_MAX_FILTER_TEXT_SIZE        255
#define BSA_MCE_MAX_MAS_INSTANCE_NAME_LEN   21
#define BSA_MCE_MAX_MAS_INSTANCES           4

typedef UINT16  tBSA_MCE_SESS_HANDLE;
typedef UINT8   tBSA_MCE_INST_ID;
typedef UINT8   tBSA_MCE_MSG_HANDLE[BSA_MCE_HANDLE_SIZE];
typedef UINT32  tBSA_MCE_FEAT;

#ifndef BSA_MCE_INS_INFO_MAX_LEN
#define BSA_MCE_INS_INFO_MAX_LEN    200  /* Instance info cannot be longer than 200 according to spec (including NULL termination) */
#endif

typedef char tBSA_MCE_MAS_INS_INFO[BSA_MCE_INS_INFO_MAX_LEN];


/* ============================================================================ */
/* BSA MAP Client callback events */
typedef enum
{
    BSA_MCE_START_EVT,
    BSA_MCE_STOP_EVT,
    BSA_MCE_OPEN_EVT,
    BSA_MCE_CLOSE_EVT,
    BSA_MCE_MN_OPEN_EVT,
    BSA_MCE_MN_CLOSE_EVT,
    BSA_MCE_NOTIF_EVT,
    BSA_MCE_NOTIF_REG_EVT,
    BSA_MCE_SET_MSG_STATUS_EVT,
    BSA_MCE_UPDATE_INBOX_EVT,
    BSA_MCE_SET_FOLDER_EVT,
    BSA_MCE_FOLDER_LIST_EVT,
    BSA_MCE_MSG_LIST_EVT,
    BSA_MCE_GET_MSG_EVT,
    BSA_MCE_PUSH_MSG_EVT,
    BSA_MCE_PUSH_MSG_DATA_REQ_EVT,
    BSA_MCE_MSG_PROG_EVT,
    BSA_MCE_OBEX_PUT_RSP_EVT,
    BSA_MCE_OBEX_GET_RSP_EVT,
    BSA_MCE_DISABLE_EVT,
    BSA_MCE_ABORT_EVT,
    BSA_MCE_GET_MAS_INS_INFO_EVT,
    BSA_MCE_GET_MAS_INSTANCES_EVT,
    BSA_MCE_INVALID_EVT
} tBSA_MCE_EVT;

/* Message type see SDP supported message type */
#define BSA_MCE_MSG_TYPE_EMAIL                (1<<0)
#define BSA_MCE_MSG_TYPE_SMS_GSM              (1<<1)
#define BSA_MCE_MSG_TYPE_SMS_CDMA             (1<<2)
#define BSA_MCE_MSG_TYPE_MMS                  (1<<3)

typedef UINT8 tBSA_MCE_MSG_TYPE;

/* Message type mask for FilterMessageType in Application parameter */
#define BSA_MCE_MSG_TYPE_MASK_SMS_GSM         (1<<0)
#define BSA_MCE_MSG_TYPE_MASK_SMS_CDMA        (1<<1)
#define BSA_MCE_MSG_TYPE_MASK_EMAIL           (1<<2)
#define BSA_MCE_MSG_TYPE_MASK_MMS             (1<<3)

typedef UINT8 tBSA_MCE_MSG_TYPE_MASK;

/* Parameter Mask for Messages-Listing */
#define BSA_MCE_ML_MASK_SUBJECT               (1<<0)
#define BSA_MCE_ML_MASK_DATETIME              (1<<1)
#define BSA_MCE_ML_MASK_SENDER_NAME           (1<<2)
#define BSA_MCE_ML_MASK_SENDER_ADDRESSING     (1<<3)
#define BSA_MCE_ML_MASK_RECIPIENT_NAME        (1<<4)
#define BSA_MCE_ML_MASK_RECIPIENT_ADDRESSING  (1<<5)
#define BSA_MCE_ML_MASK_TYPE                  (1<<6)
#define BSA_MCE_ML_MASK_SIZE                  (1<<7)
#define BSA_MCE_ML_MASK_RECEPTION_STATUS      (1<<8)
#define BSA_MCE_ML_MASK_TEXT                  (1<<9)
#define BSA_MCE_ML_MASK_ATTACHMENT_SIZE       (1<<10)
#define BSA_MCE_ML_MASK_PRIORITY              (1<<11)
#define BSA_MCE_ML_MASK_READ                  (1<<12)
#define BSA_MCE_ML_MASK_SENT                  (1<<13)
#define BSA_MCE_ML_MASK_PROTECTED             (1<<14)
#define BSA_MCE_ML_MASK_REPLYTO_ADDRESSING    (1<<15)

typedef UINT32 tBSA_MCE_ML_MASK;

/* Read status used for  message list */
enum
{
    BSA_MCE_READ_STATUS_NO_FILTERING = 0,
    BSA_MCE_READ_STATUS_UNREAD       = 1,
    BSA_MCE_READ_STATUS_READ         = 2
};
typedef UINT8 tBSA_MCE_READ_STATUS;

/* Priority status used for filtering message list */
enum
{
    BSA_MCE_PRI_STATUS_NO_FILTERING = 0,
    BSA_MCE_PRI_STATUS_HIGH         = 1,
    BSA_MCE_PRI_STATUS_NON_HIGH     = 2
};
typedef UINT8 tBSA_MCE_PRI_STATUS;

#define BSA_MCE_LTIME_LEN 15
typedef struct
{
    tBSA_MCE_ML_MASK        parameter_mask;
    UINT16                  max_list_cnt;
    UINT16                  list_start_offset;
    UINT8                   subject_length; /* valid range 1...255 */
    tBSA_MCE_MSG_TYPE_MASK  msg_mask;
    char                    period_begin[BSA_MCE_LTIME_LEN+1]; /* "yyyymmddTHHMMSS", or "" if none */
    char                    period_end[BSA_MCE_LTIME_LEN+1]; /* "yyyymmddTHHMMSS", or "" if none */
    tBSA_MCE_READ_STATUS    read_status;
    char                    recipient[BSA_MCE_MAX_FILTER_TEXT_SIZE+1]; /* "" if none */
    char                    originator[BSA_MCE_MAX_FILTER_TEXT_SIZE+1];/* "" if none */
    tBSA_MCE_PRI_STATUS     pri_status;
} tBSA_MCE_MSG_LIST_FILTER_PARAM;

/* enum for charset used in GetMessage */
enum
{
    BSA_MCE_CHARSET_NATIVE = 0,
    BSA_MCE_CHARSET_UTF_8  = 1,
    BSA_MCE_CHARSET_UNKNOWN,
    BSA_MCE_CHARSET_MAX
};
typedef UINT8 tBSA_MCE_CHARSET;

/* enum for fraction request used in GetMEssage */
enum
{
    BSA_MCE_FRAC_REQ_FIRST = 0,
    BSA_MCE_FRAC_REQ_NEXT  = 1,
    BSA_MCE_FRAC_REQ_NO,/* this is not a fraction request */
    BSA_MCE_FRAC_REQ_MAX
};
typedef UINT8 tBSA_MCE_FRAC_REQ;

/* enum for fraction delivery used in GetMEssage */
enum
{
    BSA_MCE_FRAC_DELIVER_MORE  = 0,
    BSA_MCE_FRAC_DELIVER_LAST  = 1,
    BSA_MCE_FRAC_DELIVER_NO,    /* this is not a fraction deliver*/
    BSA_MCE_FRAC_DELIVER_MAX
};
typedef UINT8 tBSA_MCE_FRAC_DELIVER;

typedef struct
{
    BOOLEAN                 attachment;
    tBSA_MCE_MSG_HANDLE     handle;
    tBSA_MCE_CHARSET        charset;
    tBSA_MCE_FRAC_REQ       fraction_request;
} tBSA_MCE_GET_MSG_PARAM;

/* message status inficator */
#define BSA_MCE_STS_INDTR_READ       0
#define BSA_MCE_STS_INDTR_DELETE     1
typedef UINT8 tBSA_MCE_STS_INDCTR;

/* message status value */
#define BSA_MCE_STS_VALUE_NO         0
#define BSA_MCE_STS_VALUE_YES        1
typedef UINT8 tBSA_MCE_STS_VALUE;

#define BSA_MCE_RETRY_OFF        0
#define BSA_MCE_RETRY_ON         1
#define BSA_MCE_RETRY_UNKNOWN    0xff
typedef UINT8   tBSA_MCE_RETRY_TYPE;

#define BSA_MCE_TRANSP_OFF        0
#define BSA_MCE_TRANSP_ON         1
#define BSA_MCE_TRANSP_UNKNOWN    0xff
typedef UINT8   tBSA_MCE_TRANSP_TYPE;

typedef struct
{
    char                    folder[BSA_MCE_ROOT_PATH_LEN_MAX];  /* current or child folder
                                                            for current folder set */
    tBSA_MCE_TRANSP_TYPE    transparent;
    tBSA_MCE_RETRY_TYPE     retry;
    tBSA_MCE_CHARSET        charset;

} tBSA_MCE_PUSH_MSG_PARAM;

/* BSA MCE Access Response */
#define BSA_MCE_ACCESS_ALLOW    0  /* Allow access to operation */
#define BSA_MCE_ACCESS_FORBID   1  /* Deny access to operation */
typedef UINT8 tBSA_MCE_ACCESS_TYPE;

/* BSA MCE Access operation type */
typedef enum
{
    BSA_MCE_OPER_NONE,
    BSA_MCE_OPER_GET_MSG,
    BSA_MCE_OPER_PUSH_MSG
} tBSA_MCE_OPER;

/* Types for Get operation */
enum
{
    BSA_MCE_GET_MSG = 0,            /* Get Message */
    BSA_MCE_GET_FOLDER_LIST,        /* Get Folder List*/
    BSA_MCE_GET_MSG_LIST,           /* Get Message List */
    BSA_MCE_GET_MAS_INST_INFO,      /* Get MAS Instance Info */
    BSA_MCE_GET_MAS_INSTANCES       /* Get MAS Instances */
    /* ADD NEW GET TYPES HERE */
};
typedef UINT8 tBSA_MCE_GET_TYPE;

/* Types for Set operation */
enum
{
    BSA_MCE_SET_MSG_STATUS = 0,     /* Set Message Status*/
    BSA_MCE_SET_FOLDER,             /* Set Folder */
    /* ADD NEW GET TYPES HERE */
};
typedef UINT8 tBSA_MCE_SET_TYPE;

/* BSA_MCE_DISABLE_EVT callback event data */
typedef struct
{
    tBSA_STATUS     status;
} tBSA_MCE_DISABLE_MSG;

/* BSA_MCE_ABORT_EVT callback event data */
typedef struct
{
    tBSA_STATUS     status;
} tBSA_MCE_ABORT_MSG;

/* Structure associated with BSA_MCE_OPEN_EVT or BSA_MCE_CLOSE_EVT*/
typedef struct
{
    tBSA_STATUS             status;
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_MCE_INST_ID        instance_id;
    BD_ADDR                 bd_addr;            /* Address of device */
    BOOLEAN                 initiator;          /* connection initiator, local TRUE, peer FALSE */
    tUIPC_CH_ID             uipc_mce_rx_channel;       /* in/out: uipc MCE Receive channel */
    tUIPC_CH_ID             uipc_mce_tx_channel;       /* in/out: uipc MCE Transmit channel */
} tBSA_MCE_OPEN_CLOSE_MSG;

/* Structure associated with BSA_MCE_START_EVT or BSA_MCE_STOP_EVT*/
typedef struct
{
    tBSA_STATUS             status;
    tBSA_MCE_SESS_HANDLE    session_handle;
} tBSA_MCE_MN_START_STOP_MSG;

/* Structure associated with BSA_MCE_MN_OPEN_EVT or BSA_MCE_MN_CLOSE_EVT*/
typedef struct
{
    tBSA_STATUS             status;
    BD_ADDR                 bd_addr;     /* Address of device */
    tBSA_MCE_SESS_HANDLE    session_handle;
} tBSA_MCE_MN_OPEN_CLOSE_MSG;

/* Structure associated with BSA_MCE_NOTIF_EVT  */
typedef struct
{
    UINT8                   data[BSA_MCE_MN_MAX_MSG_EVT_OBECT_SIZE]; /* event report data */
    tBSA_MCE_SESS_HANDLE    session_handle;     /* MCE connection handle */
    UINT16                  len;                /* length of the event report */
    BOOLEAN                 final;
    tBSA_STATUS             status;
    UINT8                   instance_id;            /* MAS instance ID */
} tBSA_MCE_NOTIF_MSG;

typedef union
{
    UINT16              fld_list_size;
    struct
    {
        UINT16          msg_list_size;
        UINT8           new_msg;
    } msg_list_param;
}tBSA_MCE_LIST_APP_PARAM_MSG;

/* Structure associated with BSA_MCE_GET_FOLDER_LIST_EVT  */
typedef struct
{
    UINT8                       data[BSA_MCE_MAX_VALUE_LEN];
    tBSA_MCE_LIST_APP_PARAM_MSG param;
    tBSA_MCE_SESS_HANDLE        session_handle;
    UINT16                      len;        /* 0 if object not included */
    BOOLEAN                     is_final;   /* TRUE - no more pkt to come */
    tBSA_STATUS                 status;

    UINT16                      num_entry; /* number of entries listed */
    BOOLEAN                     is_xml;
} tBSA_MCE_LIST_DATA_MSG;

/* Structure associated with BSA_MCE_GET_MSG_EVT  */
typedef struct
{
    tBSA_STATUS                 status;
    tBSA_MCE_SESS_HANDLE        session_handle;
    tBSA_MCE_FRAC_DELIVER       frac_deliver;
    char                        msg_name[BSA_MCE_FILENAME_MAX];
} tBSA_MCE_GETMSG_MSG;

/* Structure associated with BSA_MCE_GET_MAS_INS_INFO */
typedef struct
{
    tBSA_STATUS                 status;
    tBSA_MCE_SESS_HANDLE        session_handle;
    tBSA_MCE_INST_ID            instance_id;
    tBSA_MCE_MAS_INS_INFO       mas_ins_info;
} tBSA_MCE_GET_MAS_INS_INFO_MSG;

/* Structure associated with BSA_MCE_MSG_PROG_EVT  */
typedef struct
{
    UINT32                  read_size;
    UINT32                  obj_size;
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_MCE_OPER           operation;
} tBSA_MCE_MSG_PROG_MSG;

/* Structure associated with BSA_MCE_PUSH_MSG_DATA_REQ_EVT */
typedef struct{
    UINT32                  bytes_req;
    tBSA_MCE_SESS_HANDLE    session_handle;
}tBSA_MCE_PUSH_MSG_DATA_REQ_MSG;

/* Structure associated with BSA_MCE_PUSH_MSG_EVT */
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_STATUS             status;
    tBSA_MCE_MSG_HANDLE     msg_handle;
} tBSA_MCE_PUSHMSG_MSG;

/* Structure associated with BSA_MCE_UPDATE_INBOX_EVT  */
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_STATUS             status;
} tBSA_MCE_UPDATEINBOX_MSG;

/* Structure associated with BSA_MCE_SET_MSG_STATUS_EVT  */
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_STATUS             status;
} tBSA_MCE_SET_MSG_STATUS_MSG;

/* Structure associated with BSA_MCE_SET_FOLDER_EVT  */
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_STATUS             status;
} tBSA_MCE_SET_FOLDER_MSG;

/* Structure associated with BSA_MCE_NOTIF_REG_EVT */
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_STATUS             status;
} tBSA_MCE_NOTIFREG_MSG;

/* Structure associated with BSA_MCE_OBEX_PUT_RSP_EVT
and BSA_MCE_OBEX_GET_RSP_EVT                        */
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_MCE_INST_ID        instance_id;
    UINT8                   rsp_code;
} tBSA_MCE_OBEX_RSP_MSG;

typedef struct
{
    tBSA_MCE_INST_ID            instance_id;
    UINT8                       instance_name[BSA_MCE_MAX_MAS_INSTANCE_NAME_LEN + 1];
    tBSA_MCE_MSG_TYPE_MASK      msg_type;
} tBSA_MCE_MAS_INSTANCE;

/* Structure associated with BSA_MCE_GET_MAS_INSTANCES */
typedef struct
{
    int                         count;
    tBSA_STATUS                 status;
    tBSA_MCE_MAS_INSTANCE       mas_ins[BSA_MCE_MAX_MAS_INSTANCES];
} tBSA_MCE_GET_MAS_INSTANCES_MSG;

typedef union
{
    tBSA_MCE_OPEN_CLOSE_MSG         open;
    tBSA_MCE_OPEN_CLOSE_MSG         close;
    tBSA_MCE_MN_START_STOP_MSG      mn_start;
    tBSA_MCE_MN_START_STOP_MSG      mn_stop;
    tBSA_MCE_MN_OPEN_CLOSE_MSG      mn_open;
    tBSA_MCE_MN_OPEN_CLOSE_MSG      mn_close;
    tBSA_MCE_NOTIF_MSG              notif;
    tBSA_STATUS                     status;         /* ENABLE and DISABLE event */
    tBSA_MCE_UPDATEINBOX_MSG        upd_ibx;
    tBSA_MCE_SET_MSG_STATUS_MSG     set_msg_sts;
    tBSA_MCE_SET_FOLDER_MSG         set_folder;
    tBSA_MCE_NOTIFREG_MSG           notif_reg;
    tBSA_MCE_LIST_DATA_MSG          list_data;      /*  BSA_MCE_FOLDER_LIST_EVT,
                                                        BSA_MCE_MSG_LIST_EVT */
    tBSA_MCE_GETMSG_MSG             getmsg_msg;     /*  BSA_MCE_GET_MSG_EVT  */
    tBSA_MCE_PUSHMSG_MSG            pushmsg_msg;    /*  BSA_MCE_PUSH_MSG_EVT */
    tBSA_MCE_PUSH_MSG_DATA_REQ_MSG  push_data_req_msg;   /* BSA_MCE_PUSH_MSG_DATA_REQ_EVT */
    tBSA_MCE_MSG_PROG_MSG           prog;           /*  BSA_MCE_MSG_PROG_EVT */
    tBSA_MCE_OBEX_RSP_MSG           ma_put_rsp;
    tBSA_MCE_OBEX_RSP_MSG           ma_get_rsp;
    tBSA_MCE_GET_MAS_INS_INFO_MSG   get_mas_ins_info; /* BSA_MCE_GET_MAS_INS_INFO_EVT */
    tBSA_MCE_DISABLE_MSG            disable;
    tBSA_MCE_ABORT_MSG              abort;
    tBSA_MCE_GET_MAS_INSTANCES_MSG  mas_instances;

} tBSA_MCE_MSG;

/* BSA MCE callback function */
typedef void tBSA_MCE_CBACK(tBSA_MCE_EVT event, tBSA_MCE_MSG *p_data);

/*
* Structures used to pass parameters to BSA API functions
*/
typedef struct
{
    tBSA_MCE_CBACK  *p_cback;
} tBSA_MCE_ENABLE;

typedef struct
{
    int dummy;
} tBSA_MCE_DISABLE;

typedef struct
{
    tBSA_MCE_INST_ID    instance_id;
} tBSA_MCE_CANCEL;

/*
**          sec_mask - The security setting for the message access server.
**          p_service_name - The name of the Message Notification service, in SDP.
**          Maximum length is 35 bytes.
*/
typedef struct
{
    char                    service_name[BSA_MCE_SERVICE_NAME_LEN_MAX];
    tBSA_SEC_AUTH           sec_mask;
    tBSA_MCE_FEAT           features;
} tBSA_MCE_MN_START;

typedef struct
{
    int     dummy;
} tBSA_MCE_MN_STOP;

/*
**      instance_id - MAS instance ID on server device.
**      bd_addr: MAS server bd address.
**      sec_mask: security mask used for this connection.
*/
typedef struct
{
    BD_ADDR             bd_addr;
    tBSA_MCE_INST_ID    instance_id;
    tBSA_SEC_AUTH       sec_mask;
} tBSA_MCE_OPEN;

/*
**  session_handle - MAS session ID
*/
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
} tBSA_MCE_CLOSE;

/* notification status */
enum
{
    BSA_MCE_NOTIF_OFF = 0,
    BSA_MCE_NOTIF_ON,
    BSA_MCE_NOTIF_MAX

};
typedef UINT8 tBSA_MCE_NOTIF_STATUS;

/*
**          status - BSA_MCE_NOTIF_ON if notification required
**                   BSA_MCE_NOTIF_OFF if no notification
**          session_handle - MAS session ID
*/
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_MCE_NOTIF_STATUS   status;
} tBSA_MCE_NOTIFYREG;

typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
} tBSA_MCE_UPDATEINBOX;

/* definitions for directory navigation */
#define BSA_MCE_DIR_NAV_ROOT_OR_DOWN_ONE_LVL     2
#define BSA_MCE_DIR_NAV_UP_ONE_LVL               3

typedef UINT8 tBSA_MCE_DIR_NAV;

/*
** Description      This SET operation is used to navigate the folders of the MSE for
**                  the specified MAS instance
**
** Parameter        a combination of direction_flag and p_folder specify how to nagivate the
**                  folders on the MSE
**                  case 1 direction_flag = 2 folder = empty - reset to the default directory "telecom"
**                  case 2 direction_flag = 2 folder = name of child folder - go down 1 level into
**                  this directory name
**                  case 3 direction_flag = 3 folder = name of child folder - go up 1 level into
**                  this directory name (same as cd ../name)
**                  case 4 direction_flag = 3 folder = empty - go up 1 level to the parent directory
**                  (same as cd ..)
*/
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_MCE_DIR_NAV        direction_flag;
    char                    folder[BSA_MCE_ROOT_PATH_LEN_MAX];
} tBSA_MCE_SETFOLDER;

/*
** Description      This GET operation structure is used to retrieve the folder list object from
**                  the current folder
**
** Parameter        session_handle - MAS session ID
**                  max_list_count - maximum number of foldr-list objects allowed
**                            The maximum allowed value for this filed is 1024
**                  start_offset - offset of the from the first entry of the folder-list
**                                 object
*/
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
    UINT16                  max_list_count;
    UINT16                  start_offset;
} tBSA_MCE_GETFOLDERLIST;

/*
** Description      This GET operation structure is used to retrieve list of messages
**                  in the specified folder
**
** Parameter        session_handle -  session handle
**                  p_folder        - folder name
**                  p_filter_param - message listing filter parameters
*/
typedef struct
{
    tBSA_MCE_SESS_HANDLE            session_handle;
    char                            folder[BSA_MCE_ROOT_PATH_LEN_MAX];
    tBSA_MCE_MSG_LIST_FILTER_PARAM  filter_param;
} tBSA_MCE_GETMSGLIST;

/*
**          This GET operation is used to get bMessage or bBody of the
**          specified message handle fromMSE
**          session_handle - session ID
**          p_param - get message parameters, it shall not be NULL.
*/
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_MCE_GET_MSG_PARAM  msg_param;
} tBSA_MCE_GETMSG;

/*
**          This GET operation is used to get bMessage or bBody of the
**          specified message handle from MSE
**          session_handle - session ID
**          p_param - get message parameters, it shall not be NULL.
*/
typedef struct
{
    tBSA_MCE_SESS_HANDLE    session_handle;
    tBSA_MCE_INST_ID        instance_id;
} tBSA_MCE_GET_MAS_INFO;

/*
**          This GET operation is used to get MAS instances available on specified peer device
**          bd_addr: MAS server bd address.
*/
typedef struct
{
    BD_ADDR             bd_addr;
} tBSA_MCE_GET_MAS_INSTANCES;

/*
** Description      This SET operation is used to set the message status of the
**                  specified message handle
**
** Parameter        session_handle - MAS session ID
**                  status_indicator : read/delete message
**                  status_value : on/off
*/
typedef struct
{
    tBSA_MCE_SESS_HANDLE        session_handle;
    tBSA_MCE_MSG_HANDLE         msg_handle;
    tBSA_MCE_STS_INDCTR         status_indicator;
    tBSA_MCE_STS_VALUE          status_value;
} tBSA_MCE_SETMSGSTATUS;

/*
**      session_handle - MAS session ID
**      p_param - push message parameters, it shall not be NULL.
*/
typedef struct
{
    tBSA_MCE_SESS_HANDLE        session_handle;
    tBSA_MCE_PUSH_MSG_PARAM     msg_param;
} tBSA_MCE_PUSHMSG;

/*
**      bd_addr: MAS server bd address.
**      instance_id - MAS instance ID on server device.
*/
typedef struct
{
    BD_ADDR             bd_addr;
    tBSA_MCE_INST_ID    instance_id;
} tBSA_MCE_ABORT;

/* Get operation is used for GetFolderList, GetMsgList, GetMsg */
typedef struct
{
    /* This defines what the data in this structure corresponds to GetFolderList, GetMsgList, GetMsg */

    tBSA_MCE_GET_TYPE type;

    union {
        tBSA_MCE_GETFOLDERLIST      folderlist;
        tBSA_MCE_GETMSGLIST         msglist;
        tBSA_MCE_GETMSG             msg;
        tBSA_MCE_GET_MAS_INFO       mas_info;
        tBSA_MCE_GET_MAS_INSTANCES  mas_instances;
    }param;

} tBSA_MCE_GET;

/*  Set operation is used for ChDir */
typedef struct
{
    /*  This defines what the data in this structure corresponds to. */
    tBSA_MCE_SET_TYPE type;
    union{
        tBSA_MCE_SETMSGSTATUS   msg_status;
        tBSA_MCE_SETFOLDER      folder;
        /* Add new set types here... */
    } param;

} tBSA_MCE_SET;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/**************************
**  Server Functions
***************************/

/*******************************************************************************
**
** Function            BSA_MceEnableInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceEnableInit(tBSA_MCE_ENABLE* p_enable);

/*******************************************************************************
**
** Function         BSA_MceEnable
**
** Description      Enable the MCE subsystem.  This function must be
**                  called before any other functions in the MCE API are called.
**                  When the enable operation is complete the callback function
**                  will be called with an BSA_MCE_ENABLE_EVT event.
**
** Parameter        p_enable: Pointer to structure containing API parameters
**
** Returns         tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceEnable(tBSA_MCE_ENABLE* p_enable);

/*******************************************************************************
**
** Function            BSA_MceDisableInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceDisableInit(tBSA_MCE_DISABLE* p_disable);

/*******************************************************************************
**
** Function         BSA_MceDisable
**
** Description      Disable the MCE subssytem.  If the client is currently
**                  connected to a peer device the connection will be closed.
**
** Parameter        p_disable: Pointer to structure containing API parameters
**
** Returns         tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceDisable(tBSA_MCE_DISABLE* p_disable);

/*******************************************************************************
**
** Function            BSA_MceMnStartInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          p_mn_start - Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceMnStartInit(tBSA_MCE_MN_START* p_mn_start);

/*******************************************************************************
**
** Function         BSA_MceMnStart
**
** Description      Start the Message Notification service server.
**                  When the Start operation is complete the callback function
**                  will be called with an BSA_MCE_START_EVT event.
**                  Note: Mas always enable (BSA_SEC_AUTHENTICATE | BSA_SEC_ENCRYPT)
**
**  Parameters     p_mn_start - Pointer to structure containing API parameters
**
** Returns         tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceMnStart(tBSA_MCE_MN_START* p_mn_start);

/*******************************************************************************
**
** Function            BSA_MceMnStopInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceMnStopInit(tBSA_MCE_MN_STOP* p_mn_stop);

/*******************************************************************************
**
** Function         BSA_MceStop
**
** Description      Stop the Message Access service server.  If the server is currently
**                  connected to a peer device the connection will be closed.
**
** Parameter        p_mn_stop: Pointer to structure containing API parameters
**
** Returns         tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceMnStop(tBSA_MCE_MN_STOP* p_mn_stop);

/**************************
**  Client Functions
***************************/

/*******************************************************************************
**
** Function            BSA_MceOpenInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceOpenInit(tBSA_MCE_OPEN* p_open);

/*******************************************************************************
**
** Function         BSA_MceOpen
**
** Description      Open a connection to an Message Access service server
**                  based on specified instance_id
**
**                  When the connection is open the callback function
**                  will be called with a BSA_MCE_OPEN_EVT.  If the connection
**                  fails or otherwise is closed the callback function will be
**                  called with a BSA_MCE_CLOSE_EVT.
**
**                  Note: MAS always enable (BSA_SEC_AUTHENTICATE | BSA_SEC_ENCRYPT)
**
** Parameter        p_open: Pointer to structure containing API parameters
**
** Returns         tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceOpen(tBSA_MCE_OPEN* p_open);

/*******************************************************************************
**
** Function            BSA_MceCloseInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceCloseInit(tBSA_MCE_CLOSE* p_close);

/*******************************************************************************
**
** Function         BSA_MceClose
**
** Description      Close the specified MAS session to the server.
**
** Parameter        p_close: Pointer to structure containing API parameters
**
** Returns         tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceClose(tBSA_MCE_CLOSE* p_close);

/*******************************************************************************
**
** Function            BSA_MceNotifRegInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceNotifRegInit(tBSA_MCE_NOTIFYREG* p_notifyreg);

/*******************************************************************************
**
** Function         BSA_MceNotifReg
**
** Description      Set the Message Notification status to On or OFF on the MSE.
**                  When notification is registered, message notification service
**                  must be enabled by calling API BSA_MceMnStart().
**
** Parameter        p_notifyreg - A pointer to the structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceNotifReg(tBSA_MCE_NOTIFYREG* p_notifyreg);

/*******************************************************************************
**
** Function            BSA_MceUpdateInboxInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceUpdateInboxInit(tBSA_MCE_UPDATEINBOX* p_update_inbox);

/*******************************************************************************
**
** Function         BSA_MceUpdateInbox
**
** Description      This function is used to update the inbox for the
**                  specified MAS session.
**
** Parameter        p_notifyreg - A pointer to the structure containing API parameters
**
** Returns         tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceUpdateInbox(tBSA_MCE_UPDATEINBOX* p_update_inbox);

/*******************************************************************************
**
** Function            BSA_MceGetInit
**
** Description         Initialize structure containing API parameters with default values
**                     Following GET operations are supported
**                     1) Get Message 2) Get Message List 3) Get Folder List
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceGetInit(tBSA_MCE_GET* p_get);

/*******************************************************************************
**
** Function         BSA_MceGet
**
** Description      Performs a Get Operation based on the specified get type
**                  and parameters in the tBSA_MCE_GET structure.
**                      Following GET operations are supported
**                      1) Get Message 2) Get Message List 3) Get Folder List
**
** Parameter        p_get - A pointer to the structure containing API parameters
**
** Returns         tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceGet(tBSA_MCE_GET* p_get);

/*******************************************************************************
**
** Function            BSA_MceSet
**
** Description         Initialize structure containing API parameters with default values
**                     Performs a Set Operation based on the specified set type and parameters in the tBSA_MCE_SET structure.
**                     Following set operations are supported: 1) Set message status 2) Set folder
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceSetInit(tBSA_MCE_SET* p_set);

/*******************************************************************************
**
** Function         BSA_MceSet
**
** Description      This function is used to set the message status of the
**                  specified message handle
**
** Parameter        p_set - A pointer to the structure containing API parameters
**
** Returns         tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceSet(tBSA_MCE_SET* p_set);

/*******************************************************************************
**
** Function            BSA_McePushMsgInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_McePushMsgInit(tBSA_MCE_PUSHMSG* p_pushmsg);

/*******************************************************************************
**
** Function         BSA_McePushMsg
**
** Description      This function is used to upload a message
**                  to the specified folder in MSE
**
** Parameter        p_pushmsg - A pointer to the structure containing API parameters
**
** Returns         tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_McePushMsg(tBSA_MCE_PUSHMSG* p_pushmsg);

/*******************************************************************************
**
** Function            BSA_MceAbortInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceAbortInit(tBSA_MCE_ABORT* p_abort);

/*******************************************************************************
**
** Function         BSA_MceAbort
**
** Description      This function is used to abort the current OBEX multi-packet
**                  operation
**
** Parameter         p_abort - A pointer to the structure containing API parameters
**
** Returns         tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceAbort(tBSA_MCE_ABORT* p_abort);

/*******************************************************************************
**
** Function         BSA_MceCancelInit
**
** Description      Init a structure tBSA_MCE_CANCEL to be used with BSA_MceCancel
**
** Returns          tBSA_STATUS
**
*******************************************************************************/

tBSA_STATUS BSA_MceCancelInit(tBSA_MCE_CANCEL *pCancel);

/*******************************************************************************
**
** Function         BSA_MceCancel
**
** Description      Send a command to cancel connection.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceCancel(tBSA_MCE_CANCEL *pCancel);

#ifdef __cplusplus
}
#endif

#endif
