/*****************************************************************************
**
**  Name:           bta_mce_api.h
**
**  Description:    This is the public interface file for the Message Client Equipment
**                  (MCE) subsystem of BTA, Broadcom's
**                  Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2009-2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_MCE_API_H
#define BTA_MCE_API_H

#include "bta_api.h"
#include "btm_api.h"
#include "obx_api.h"
#include "bta_sys.h"
#include "bta_ma_def.h"
#include "bta_mse_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* Extra Debug Code */
#ifndef BTA_MCE_DEBUG
#define BTA_MCE_DEBUG           TRUE
#endif

/* Client callback function events */

enum
{
    BTA_MCE_ENABLE_EVT            =0,
    BTA_MCE_START_EVT,
    BTA_MCE_STOP_EVT,
    BTA_MCE_DISCOVER_EVT,
    BTA_MCE_MA_OPEN_EVT,
    BTA_MCE_MA_CLOSE_EVT,
    BTA_MCE_MN_OPEN_EVT,
    BTA_MCE_MN_CLOSE_EVT,
    BTA_MCE_NOTIF_EVT,
    BTA_MCE_NOTIF_REG_EVT,
    BTA_MCE_SET_MSG_STATUS_EVT,
    BTA_MCE_UPDATE_INBOX_EVT,
    BTA_MCE_SET_FOLDER_EVT,
    BTA_MCE_FOLDER_LIST_EVT,
    BTA_MCE_MSG_LIST_EVT,
    BTA_MCE_GET_MSG_EVT,
#if (defined(BTA_MAP_1_2_SUPPORTED) && BTA_MAP_1_2_SUPPORTED == TRUE)
    BTA_MCE_GET_MAS_INS_INFO,
#endif
    BTA_MCE_PUSH_MSG_EVT,
    BTA_MCE_MSG_PROG_EVT,
    BTA_MCE_OBEX_PUT_RSP_EVT,
    BTA_MCE_OBEX_GET_RSP_EVT,
    BTA_MCE_DISABLE_EVT,
    BTA_MCE_INVALID_EVT
};
typedef UINT8 tBTA_MCE_EVT;

#define BTA_MCE_OPER_NONE               0
#define BTA_MCE_OPER_GET_MSG            1
#define BTA_MCE_OPER_PUSH_MSG           2

typedef UINT8 tBTA_MCE_OPER;

#ifndef BTA_MCE_NUM_SESS
#define BTA_MCE_NUM_SESS    4       /* Number of MCE sessions */
#endif

#define BTA_MCE_MAX_NAME_LEN_PER_LINE    17

typedef struct
{
    char            name[BTA_MCE_MAX_NAME_LEN_PER_LINE];
    tBTA_MA_INST_ID mas_inst_id;
    UINT8           supported_msg_type;
    UINT8           scn;
    UINT16          version;
    UINT16          psm;
    UINT32          peer_features;

} tBTA_MCE_MAS_REC;

/* Structure associated with BTA_MCE_DISCOVER_EVT */
typedef struct
{
    UINT16              status;
    BD_ADDR             bd_addr;
    UINT8               num_mas_srv;
    tBTA_MCE_MAS_REC    rec[BTA_MCE_NUM_SESS];
} tBTA_MCE_DISCOVER;

/* Structure associated with BTA_MCE_MA_OPEN_EVT or
   BTA_MCE_MA_CLOSE_EVT                            */
typedef struct
{
    tBTA_MA_STATUS      status;
    tBTA_MA_SESS_HANDLE session_id;
    tBTA_MA_INST_ID     mas_inst_id;
    tBTM_BD_NAME        dev_name;    /* Name of device, "" if unknown */
    BD_ADDR             bd_addr;     /* Address of device */
} tBTA_MCE_MA_OPEN_CLOSE;


/* Structure associated with BTA_MCE_MN_OPEN_EVT or
   BTA_MCE_MN_START_EVT                            */
typedef struct
{
    tBTA_MA_STATUS      status;
    tBTA_MA_SESS_HANDLE session_id;
} tBTA_MCE_MN_START_STOP;


/* Structure associated with BTA_MCE_MN_OPEN_EVT or
   BTA_MCE_MN_CLOSE_EVT                            */
typedef struct
{
    tBTA_MA_STATUS      status;
    tBTM_BD_NAME        dev_name;    /* Name of device, "" if unknown */
    BD_ADDR             bd_addr;     /* Address of device */
    tBTA_MA_SESS_HANDLE session_id;
    tOBX_RSP_CODE       obx_rsp_code;/* obex response code */

} tBTA_MCE_MN_OPEN_CLOSE;

/* Structure associated with BTA_MCE_NOTIF_REG_EVT  */
typedef struct
{
    tBTA_MA_SESS_HANDLE     handle;
    tBTA_MA_STATUS          status;
} tBTA_MCE_SET_NOTIF_REG;

/* Structure associated with BTA_MCE_NOTIF_EVT  */
typedef struct
{
    UINT8               *p_object;          /* event report data */
    tBTA_MA_SESS_HANDLE session_id;         /* MCE connection handle */
    UINT16              len;                /* length of the event report */
    BOOLEAN             final;
    tBTA_MA_STATUS      status;
    UINT8               inst_id;            /* MAS instance ID */
    tOBX_RSP_CODE       obx_rsp_code;       /* obex response code */
} tBTA_MCE_NOTIF;

typedef union
{
    UINT16              fld_list_size;
    struct
    {
        UINT16          msg_list_size;
        UINT8           new_msg;
    }                   msg_list_param;
}tBTA_MCE_LIST_APP_PARAM;

/* Structure associated with BTA_MCE_GET_FOLDER_LIST_EVT  */
typedef struct
{
    UINT8                   *p_data;
    tBTA_MCE_LIST_APP_PARAM *p_param;
    tBTA_MA_SESS_HANDLE     session_id;
    UINT16                  len; /* 0 if object not included */
    BOOLEAN                 is_final; /* TRUE - no more pkt to come */
    tBTA_MA_STATUS          status;
    tOBX_RSP_CODE           obx_rsp_code;       /* obex response code */
} tBTA_MCE_LIST_DATA;

/* Structure associated with BTA_MCE_GET_MSG_EVT  */
typedef struct
{
    tBTA_MA_STATUS          status;
    tBTA_MA_SESS_HANDLE     session_id;
    tBTA_MA_FRAC_DELIVER    frac_deliver;
    tOBX_RSP_CODE           obx_rsp_code;       /* obex response code */
} tBTA_MCE_GET_MSG;

#if (defined(BTA_MAP_1_2_SUPPORTED) && BTA_MAP_1_2_SUPPORTED == TRUE)
/* Structure associated with BTA_MCE_GET_MAS_INS_INFO */
typedef struct
{
    tBTA_MA_STATUS          status;
    tBTA_MA_SESS_HANDLE     session_id;
    tBTA_MA_INST_ID         mas_instance_id;
    tBTA_MA_MAS_INS_INFO    mas_ins_info;
    tOBX_RSP_CODE           obx_rsp_code;       /* obex response code */
} tBTA_MCE_GET_MAS_INS_INFO;
#endif

/* Structure associated with BTA_MCE_MSG_PROG_EVT  */
typedef struct
{
    UINT32                  read_size;
    UINT32                  obj_size;
    tBTA_MA_SESS_HANDLE     handle;
    tBTA_MCE_OPER           operation;
} tBTA_MCE_MSG_PROG;

/* Structure associated with BTA_MCE_PUSH_MSG_EVT  */
typedef struct
{
    tBTA_MA_SESS_HANDLE     session_id;
    tBTA_MA_STATUS          status;
    tBTA_MA_MSG_HANDLE      msg_handle;
    tOBX_RSP_CODE           obx_rsp_code;       /* obex response code */
} tBTA_MCE_PUSH_MSG;

/* Structure associated with BTA_MCE_UPDATE_INBOX_EVT  */
typedef struct
{
    tBTA_MA_SESS_HANDLE     session_id;
    tBTA_MA_STATUS          status;
    tOBX_RSP_CODE           obx_rsp_code;       /* obex response code */
} tBTA_MCE_UPDATE_INBOX;

/* Structure associated with BTA_MCE_SET_MSG_STATUS_EVT  */
typedef struct
{
    tBTA_MA_SESS_HANDLE     session_id;
    tBTA_MA_STATUS          status;
    tOBX_RSP_CODE           obx_rsp_code;       /* obex response code */
} tBTA_MCE_SET_MSG_STATUS;

/* Structure associated with BTA_MCE_SET_MSG_FOLDER_EVT  */
typedef struct
{
    tBTA_MA_SESS_HANDLE     session_id;
    tBTA_MA_STATUS          status;
    tOBX_RSP_CODE           obx_rsp_code;       /* obex response code */
} tBTA_MCE_SET_FOLDER_STATUS;

/* Structure associated with BTA_MCE_NOTIF_REG_EVT  */
typedef struct
{
    tBTA_MA_SESS_HANDLE     session_id;
    tBTA_MA_STATUS          status;
    tOBX_RSP_CODE           obx_rsp_code;       /* obex response code */
} tBTA_MCE_NOTIF_REG;

/* Structure associated with BTA_MCE_OBEX_PUT_RSP_EVT
   and BTA_MCE_OBEX_GET_RSP_EVT                        */
typedef struct
{
    tBTA_MA_SESS_HANDLE     session_id;
    tBTA_MA_INST_ID         mas_instance_id;
    UINT8                   rsp_code;
} tBTA_MCE_OBEX_RSP;

typedef union
{
    tBTA_MCE_DISCOVER           discover;
    tBTA_MCE_MA_OPEN_CLOSE      ma_open;
    tBTA_MCE_MA_OPEN_CLOSE      ma_close;
    tBTA_MCE_MN_START_STOP      mn_start;
    tBTA_MCE_MN_START_STOP      mn_stop;
    tBTA_MCE_MN_OPEN_CLOSE      mn_open;
    tBTA_MCE_MN_OPEN_CLOSE      mn_close;
    tBTA_MCE_NOTIF              notif;
    tBTA_MA_STATUS              status;         /* ENABLE and DISABLE event */
    tBTA_MCE_UPDATE_INBOX       upd_ibx;
    tBTA_MCE_SET_MSG_STATUS     set_msg_sts;
    tBTA_MCE_SET_FOLDER_STATUS  set_folder_sts;
    tBTA_MCE_NOTIF_REG          notif_reg;

    tBTA_MCE_LIST_DATA          list_data;      /* BTA_MCE_FOLDER_LIST_EVT,
                                                   BTA_MCE_MSG_LIST_EVT */
    tBTA_MCE_GET_MSG            get_msg;        /* BTA_MCE_GET_MSG_EVT  */
#if (defined(BTA_MAP_1_2_SUPPORTED) && BTA_MAP_1_2_SUPPORTED == TRUE)
    tBTA_MCE_GET_MAS_INS_INFO   get_mas_ins_info; /* BTA_MCE_GET_MAS_INS_INFO */
#endif
    tBTA_MCE_PUSH_MSG           push_msg;       /* BTA_MCE_PUSH_MSG_EVT */
    tBTA_MCE_MSG_PROG           prog;           /* BTA_MCE_MSG_PROG_EVT */
    tBTA_MCE_OBEX_RSP           ma_put_rsp;
    tBTA_MCE_OBEX_RSP           ma_get_rsp;
} tBTA_MCE;

/* Client callback function */
typedef void tBTA_MCE_CBACK(tBTA_MCE_EVT event, tBTA_MCE *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/

/**************************
**  Server Functions
***************************/

/*******************************************************************************
**
** Function         BTA_MceEnable
**
** Description      Enable the MCE subsystem.  This function must be
**                  called before any other functions in the MCE API are called.
**                  When the enable operation is complete the callback function
**                  will be called with an BTA_MCE_ENABLE_EVT event.
**
** Parameter        p_cback: call function registered to receive call back events.
**                  app_id: application ID.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceEnable(tBTA_MCE_CBACK *p_cback, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_MceDisable
**
** Description      Disable the MCE subssytem.  If the client is currently
**                  connected to a peer device the connection will be closed.
**
** Parameter        None
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceDisable(void);

/*******************************************************************************
**
** Function         BTA_MceMnStart
**
** Description      Start the Message Notification service server.
**                  When the Start operation is complete the callback function
**                  will be called with an BTA_MCE_START_EVT event.
**                  Note: Mas always enable (BTA_SEC_AUTHENTICATE | BTA_SEC_ENCRYPT)
**
**  Parameters      sec_mask - The security setting for the message access server.
**                  p_service_name - The name of the Message Notification service, in SDP.
**                                   Maximum length is 35 bytes.
**                  features - Local supported features
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceMnStart(tBTA_SEC sec_mask, const char *p_service_name,
                                   tBTA_MA_SUPPORTED_FEATURES features);

/*******************************************************************************
**
** Function         BTA_MceStop
**
** Description      Stop the Message Access service server.  If the server is currently
**                  connected to a peer device the connection will be closed.
**
** Parameter        None
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceMnStop(void);



/**************************
**  Client Functions
***************************/

/*******************************************************************************
**
** Function         BTA_MceDiscover
**
** Description      Start service discover of MAP on the peer device
**
**                  When SDP is finished, the callback function will be called
**                  with BTA_MCE_DISCOVER_EVT with status.
**
** Parameter        bd_addr: MAS server bd address.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceDiscover(BD_ADDR bd_addr);

/*******************************************************************************
**
** Function         BTA_MceOpen
**
** Description      Open a connection to an Message Access service server
**                  based on specified mas_instance_id
**
**                  When the connection is open the callback function
**                  will be called with a BTA_MCE_MA_OPEN_EVT.  If the connection
**                  fails or otherwise is closed the callback function will be
**                  called with a BTA_MCE_MA_CLOSE_EVT.
**
**                  Note: MAS always enable (BTA_SEC_AUTHENTICATE | BTA_SEC_ENCRYPT)
**
** Parameter        mas_instance_id - MAS instance ID on server device.
**                  bd_addr: MAS server bd address.
**                  sec_mask: security mask used for this connection.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceOpen(BD_ADDR bd_addr, tBTA_MA_INST_ID mas_instance_id,
                                tBTA_SEC sec_mask);

/*******************************************************************************
**
** Function         BTA_MceClose
**
** Description      Close the specified MAS session to the server.
**
** Parameter        session_id - MAS session ID
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceClose(tBTA_MA_SESS_HANDLE session_id);


/*******************************************************************************
**
** Function         BTA_MceNotifReg
**
** Description      Set the Message Notification status to On or OFF on the MSE.
**                  When notification is registered, message notification service
**                  must be enabled by calling API BTA_MceMnStart().
**
** Parameter        status - BTA_MA_NOTIF_ON if notification required
**                           BTA_MA_NOTIF_OFF if no notification
**                  session_id - MAS session ID
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceNotifReg(tBTA_MA_SESS_HANDLE session_id,
                                    tBTA_MA_NOTIF_STATUS status);

/*******************************************************************************
**
** Function         BTA_MceUpdateInbox
**
** Description      This function is used to update the inbox for the
**                  specified MAS session.
**
** Parameter        session_id - MAS session ID
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceUpdateInbox(tBTA_MA_SESS_HANDLE session_id);

/*******************************************************************************
**
** Function         BTA_MceSetFolder
**
** Description      This function is used to navigate the folders of the MSE for
**                  the specified MAS instance
**
** Parameter        a combination of flag and p_folder specify how to nagivate the
**                  folders on the MSE
**                  case 1 flag = 2 folder = empty - reset to the default directory "telecom"
**                  case 2 flag = 2 folder = name of child folder - go down 1 level into
**                  this directory name
**                  case 3 flag = 3 folder = name of child folder - go up 1 level into
**                  this directory name (same as cd ../name)
**                  case 4 flag = 3 folder = empty - go up 1 level to the parent directory
**                  (same as cd ..)
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceSetFolder(tBTA_MA_SESS_HANDLE session_id,
                                     tBTA_MA_DIR_NAV flag, char *p_folder);


/*******************************************************************************
**
** Function         BTA_MceGetFolderList
**
** Description      This function is used to retrieve the folder list object from
**                  the current folder
**
** Parameter        session_id - MAS session ID
**                  max_list_count - maximum number of foldr-list objects allowed
**                            The maximum allowed value for this filed is 1024
**                  start_offset - offset of the from the first entry of the folder-list
**                                   object
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceGetFolderList(tBTA_MA_SESS_HANDLE session_id,
                                         UINT16 max_list_count, UINT16 start_offset);



/*******************************************************************************
**
** Function         BTA_MceGetMsgList
**
** Description      This function is used to retrieve the folder list object from
**                  the current folder of the MSE
**
** Parameter        session_id -  session handle
**                  p_folder        - folder name
**                  p_filter_param - message listing filter parameters
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceGetMsgList(tBTA_MA_SESS_HANDLE session_id,
                                      const char *p_folder,
                                      tBTA_MA_MSG_LIST_FILTER_PARAM *p_filter_param);



/*******************************************************************************
**
** Function         BTA_MceGetMsg
**
** Description      This function is used to get bMessage or bBody of the
**                  specified message handle from MSE
**
** Parameter        session_id - session ID
**                  p_param - get message parameters, it shall not be NULL.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceGetMsg(tBTA_MA_SESS_HANDLE session_id,
                                  tBTA_MA_GET_MSG_PARAM *p_param);


/*******************************************************************************
**
** Function         BTA_MceGetMASInstanceInfo
**
** Description      This function enables the MCE to get the MAS instance information
**                  from the MSE
**
** Parameters       session_id - session ID
**                  mas_instance_id - MAS Instance ID
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceGetMASInstanceInfo(tBTA_MA_SESS_HANDLE session_id, tBTA_MA_INST_ID mas_instance_id);
/*******************************************************************************
**
** Function         BTA_MceSetMsgStatus
**
** Description      This function is used to set the message status of the
**                  specified message handle
**
** Parameter        session_id - MAS session ID
**                  status_indicator : read/delete message
**                  status_value : on/off
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceSetMsgStatus(tBTA_MA_SESS_HANDLE session_id,
                                        tBTA_MA_MSG_HANDLE msg_handle,
                                        tBTA_MA_STS_INDCTR status_indicator,
                                        tBTA_MA_STS_VALUE status_value);

/*******************************************************************************
**
** Function         BTA_McePushMsg
**
** Description      This function is used to upload a message
**                  to the specified folder in MSE
**
** Parameter        session_id - MAS session ID
**                  p_param - push message parameters, it shall not be NULL.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_McePushMsg(tBTA_MA_SESS_HANDLE session_id,
                                   tBTA_MA_PUSH_MSG_PARAM *p_param);

/*******************************************************************************
**
** Function         BTA_MceAbort
**
** Description      This function is used to abort the current OBEX multi-packt
**                  operation
**
** Parameter        bd_addr: MAS server bd address.
**                  mas_instance_id - MAS instance ID on server device.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_MceAbort(BD_ADDR bd_addr, tBTA_MA_INST_ID mas_instance_id);

#endif
