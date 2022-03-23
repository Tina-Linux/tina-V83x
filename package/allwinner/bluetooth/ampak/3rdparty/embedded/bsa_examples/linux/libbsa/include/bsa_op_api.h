/*****************************************************************************
 **
 **  Name:           bsa_op_api.h
 **
 **  Description:    This is the public interface file for the object push
 **                  (OP) client and server subsystem of BSA, Widcomm's
 **                  Bluetooth application layer for mobile phones.
 **
 **  Copyright (c) 2003-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_OP_API_H
#define BSA_OP_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"
/* for tBSA_SEC_AUTH */
#include "bsa_sec_api.h"

#include "bta_op_api.h"

#define BSA_OP_OBJECT_FMT        0x0   /* format given by tBSA_OP_FMT */
#define BSA_OP_CUSTOM_FMT        0x1   /* interface in another custom format BSA will transalte to Object format*/

typedef UINT8 tBSA_OP_API_FMT;

#define BSA_OP_SERVICE_NAME_LEN_MAX 150
#define BSA_OP_OBJECT_NAME_LEN_MAX  255

/* Object format */
#define BSA_OP_VCARD21_FMT          BTA_OP_VCARD21_FMT       /* vCard 2.1 */
#define BSA_OP_VCARD30_FMT          BTA_OP_VCARD30_FMT       /* vCard 3.0 */
#define BSA_OP_VCAL_FMT             BTA_OP_VCAL_FMT       /* vCal 1.0 */
#define BSA_OP_ICAL_FMT             BTA_OP_ICAL_FMT       /* iCal 2.0 */
#define BSA_OP_VNOTE_FMT            BTA_OP_VNOTE_FMT       /* vNote */
#define BSA_OP_VMSG_FMT             BTA_OP_VMSG_FMT       /* vMessage */
#define BSA_OP_OTHER_FMT            BTA_OP_OTHER_FMT    /* other format */

typedef UINT8 tBSA_OP_FMT;

/* Object format mask */
#define BSA_OP_VCARD21_MASK         BTA_OP_VCARD21_MASK    /* vCard 2.1 */
#define BSA_OP_VCARD30_MASK         BTA_OP_VCARD30_MASK    /* vCard 3.0 */
#define BSA_OP_VCAL_MASK            BTA_OP_VCAL_MASK    /* vCal 1.0 */
#define BSA_OP_ICAL_MASK            BTA_OP_ICAL_MASK    /* iCal 2.0 */
#define BSA_OP_VNOTE_MASK           BTA_OP_VNOTE_MASK    /* vNote */
#define BSA_OP_VMSG_MASK            BTA_OP_VMSG_MASK    /* vMessage */
#define BSA_OP_ANY_MASK             BTA_OP_ANY_MASK    /* Any type of object. */

typedef UINT8 tBSA_OP_FMT_MASK;

/* Access response types */
#define BSA_OP_ACCESS_ALLOW     BTA_OP_ACCESS_ALLOW   /* Allow the requested operation */
#define BSA_OP_ACCESS_FORBID    BTA_OP_ACCESS_FORBID   /* Disallow the requested operation */
#define BSA_OP_ACCESS_NONSUP    BTA_OP_ACCESS_NONSUP   /* Requested operation is not supported */
#define BSA_OP_ACCESS_EN_AUTO_OVERWRITE   3   /*Enable automatic Access response with file overwrite */
#define BSA_OP_ACCESS_EN_AUTO   4   /* Enable automatic Access response, access event used to resolve overwrite */
#define BSA_OP_ACCESS_DIS_AUTO  5   /* Requested operation is not supported */

typedef UINT8 tBSA_OP_ACCESS;

#define BSA_OP_ACCESS_NO_RESP  0   /* Requested operation is not supported */
#define BSA_OP_ACCESS_NO_AUTO_ACCEPT  1
#define BSA_OP_ACCESS_FS_ERROR  2   /* Requested operation is not supported */

typedef UINT8 tBSA_OP_ACCESS_RESPONSE;

/* Access event operation types */
#define BSA_OP_OPER_NONE        0
#define BSA_OP_OPER_PUSH        BTA_OP_OPER_PUSH
#define BSA_OP_OPER_PULL        BTA_OP_OPER_PULL

typedef UINT8 tBSA_OP_OPER;


/* Server callback function event */
#define BSA_OPS_OPEN_EVT            1   /* Connection to peer is open. */
#define BSA_OPS_PROGRESS_EVT        2   /* Object being sent or received. */
#define BSA_OPS_OBJECT_EVT          3   /* Object has been received. */
#define BSA_OPS_CLOSE_EVT           4   /* Connection to peer closed. */
#define BSA_OPS_ACCESS_EVT          5   /* Request for access to push or pull object */

typedef UINT8 tBSA_OPS_EVT;

/* Structure associated with BSA_OPS_OBJECT_EVT */
typedef struct {
    char p_name[BSA_OP_OBJECT_NAME_LEN_MAX];
    tBSA_OP_FMT format; /* Object format. */
} tBSA_OPS_OBJECT_EVT;

typedef struct {
    UINT32 obj_size; /* Total size of object (BSA_FS_LEN_UNKNOWN if unknown) */
    UINT16 bytes; /* Number of bytes read or written since last progress event */
    tBSA_OP_OPER operation; /* Is progress for Push or Pull */
} tBSA_OPS_PROGRESS_EVT;

typedef struct {
    char p_name[BSA_OP_OBJECT_NAME_LEN_MAX];
    char *p_type; /* Object type (NULL if not specified) */
    UINT32 size; /* Object size */
    BD_NAME dev_name; /* Name of device, "" if unknown */
    BD_ADDR bd_addr; /* Address of device */
    tBSA_OP_OPER oper; /* Operation (push or pull) */
    tBSA_OP_FMT format; /* Object format */
    tBSA_OP_ACCESS_RESPONSE responce_required; /* if this is set to BSA_OP_ACCESS_RESP_NAME_OVERWRITE then the application must respond with a BSA_OpsAccess command resolving the object name contention.*/
} tBSA_OPS_ACCESS_EVT;

/* Union of all server callback structures */
typedef union {
    tBSA_OPS_OBJECT_EVT object;
    tBSA_OPS_PROGRESS_EVT prog;
    tBSA_OPS_ACCESS_EVT access;
    BD_ADDR bd_addr;
} tBSA_OPS_MSG;

/* Server callback function */
typedef void ( tBSA_OPS_CBACK)(tBSA_OPS_EVT event, tBSA_OPS_MSG *p_data);

typedef struct {
    tBSA_SEC_AUTH sec_mask;
    tBSA_OP_API_FMT api_fmt;
    tBSA_OP_FMT_MASK formats;
    char service_name[BSA_OP_SERVICE_NAME_LEN_MAX];
    tBSA_OPS_CBACK *p_cback;
} tBSA_OPS_ENABLE;

typedef struct {
    tBSA_OP_OPER oper;
    tBSA_OP_ACCESS access;
    char object_name[BSA_OP_OBJECT_NAME_LEN_MAX];
} tBSA_OPS_ACCESS;

typedef struct {
    UINT8 dummy;
} tBSA_OPS_DISABLE;

typedef struct {
    UINT8 dummy;
} tBSA_OPS_CLOSE;

/*****************************************************************************
 **  External Function Declarations
 *****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function         BSA_OpsEnable
 **
 ** Description      Enable the object push server.  This function must be
 **                  called before any other functions in the OPS API are called.
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpsEnable(tBSA_OPS_ENABLE *ops_enable_req);

/*******************************************************************************
 **
 ** Function         BSA_OpsEnableInit
 **
 ** Description      Initialize the tBSA_OPS_ENABLE_CMD structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpsEnableInit(tBSA_OPS_ENABLE *ops_enable_req);

/*******************************************************************************
 **
 ** Function         BSA_OpsDisable
 **
 ** Description      Disable the object push server.  If the server is currently
 **                  connected to a peer device the connection will be closed.
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpsDisable(tBSA_OPS_DISABLE *ops_disable_req);

/*******************************************************************************
 **
 ** Function         BSA_OpsDisableInit
 **
 ** Description      Initialize the tBSA_OPS_DISABLE_CMD structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpsDisableInit(tBSA_OPS_DISABLE *ops_disable_req);

/*******************************************************************************
 **
 ** Function         BSA_OpsClose
 **
 ** Description      Close the current connection.  This function is called if
 **                  the phone wishes to close the connection before the object
 **                  push is completed.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpsClose(tBSA_OPS_CLOSE *ops_close_req);\

/*******************************************************************************
 **
 ** Function         BSA_OpsCloseInit
 **
 ** Description      Initialize the tBSA_OPS_CLOSE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpsCloseInit(tBSA_OPS_CLOSE *ops_close_req);

/*******************************************************************************
 **
 ** Function         BSA_OpsAccess
 **
 ** Description      Sends a reply to an access request event (BSA_OPS_ACCESS_EVT).
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpsAccess(tBSA_OPS_ACCESS *ops_access);

/*******************************************************************************
 **
 ** Function         BSA_OpsAccessInit
 **
 ** Description      Initialize the tBSA_OPS_ACCESS structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpsAccessInit(tBSA_OPS_ACCESS *ops_access);


/* Client callback function event */
#define BSA_OPC_OPEN_EVT            1   /* Connection to peer is open. */
#define BSA_OPC_PROGRESS_EVT        2   /* push/pull in progres */
#define BSA_OPC_OBJECT_EVT          3   /* Object Pulled */
#define BSA_OPC_OBJECT_PSHD_EVT     4   /* Object pushed */
#define BSA_OPC_CLOSE_EVT           5   /* Connection to peer closed. */

typedef UINT8 tBSA_OPC_EVT;

/* Structure associated with BSA_OPC_OBJECT_EVT */
typedef struct {
    char name[BSA_OP_OBJECT_NAME_LEN_MAX]; /* Object name. */
    tBSA_STATUS status;
} tBSA_OPC_OBJECT_MSG;

typedef struct {
    UINT32 obj_size; /* Total size of object (BSA_FS_LEN_UNKNOWN if unknown) */
    UINT16 bytes; /* Number of bytes read or written since last progress event */
    tBSA_OP_OPER operation; /* Is progress for Push or Pull */
} tBSA_OPC_PROGRESS_MSG;

/* Union of all client callback structures */
typedef union {
    tBSA_OPC_OBJECT_MSG object;
    tBSA_OPC_PROGRESS_MSG prog;
    tBSA_STATUS status;
} tBSA_OPC_MSG;

/* Client callback function */
typedef void (tBSA_OPC_CBACK)(tBSA_OPC_EVT event, tBSA_OPC_MSG *p_data);

/* Structure associated with BSA_OpcEnable */
typedef struct {
    tBSA_SEC_AUTH sec_mask;
    tBSA_OPC_CBACK *p_cback;
    BOOLEAN single_op;
} tBSA_OPC_ENABLE;

/* Structure associated with BSA_OpcDisable */
typedef struct {
    UINT8 dummy;
} tBSA_OPC_DISABLE;

/* Structure associated with BSA_OpcClose */
typedef struct {
    UINT8 dummy;
} tBSA_OPC_CLOSE;

/* Structure associated with BSA_OpcPush */
typedef struct {
    BD_ADDR bd_addr;
    tBSA_OP_FMT format;
    char send_path[BSA_OP_OBJECT_NAME_LEN_MAX];
} tBSA_OPC_PUSH;

/* Structure associated with BSA_OpcPullCard */
typedef struct {
    BD_ADDR bd_addr;
    tBSA_OP_FMT format;
    char recv_path[BSA_OP_OBJECT_NAME_LEN_MAX];
} tBSA_OPC_PULL_CARD;

/* Structure associated with BSA_OpcPullCard */
typedef struct {
    BD_ADDR bd_addr;
    tBSA_OP_FMT format;
    char send_path[BSA_OP_OBJECT_NAME_LEN_MAX];
    char recv_path[BSA_OP_OBJECT_NAME_LEN_MAX];
} tBSA_OPC_EXCH_CARD;

/*******************************************************************************
 **
 ** Function         BSA_OpcEnableInit
 **
 ** Description      Initialize the tBSA_OPC_ENABLE structure to default values.
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcEnableInit(tBSA_OPC_ENABLE *p_enable_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcEnable
 **
 ** Description      Enable the object push client.  This function must be
 **                  called before any other functions in the OP API are called.
 **                  When the enable operation is complete the callback function
 **                  will be called with a BSA_OPC_ENABLE_EVT.
 **
 **                  If single_op is FALSE, the connection stays open after
 **                  the operation finishes (until BSA_OpcClose is called).
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcEnable(tBSA_OPC_ENABLE *p_enable_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcDisable
 **
 ** Description      Initialize the tBSA_OPC_DISABLE structure to default values..
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcDisableInit(tBSA_OPC_DISABLE *p_disable_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcDisable
 **
 ** Description      Disable the object push client.  If the client is currently
 **                  connected to a peer device the connection will be closed.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcDisable(tBSA_OPC_DISABLE *p_disable_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcPush
 **
 ** Description      Initialize the tBSA_OPC_PUSH structure to default values.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcPushInit(tBSA_OPC_PUSH *p_push_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcPush
 **
 ** Description      Push an object to a peer device.  p_name must point to
 **                  a fully qualified path and file name.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcPush(tBSA_OPC_PUSH *p_push_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcPullCard
 **
 ** Description      Pull default card from peer. p_path must point to a fully
 **                  qualified path specifying where to store the pulled card.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcPullCardInit(tBSA_OPC_PULL_CARD *p_pull_card_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcPullCard
 **
 ** Description      Pull default card from peer. p_path must point to a fully
 **                  qualified path specifying where to store the pulled card.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcPullCard(tBSA_OPC_PULL_CARD *p_pull_card_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcPullCard
 **
 ** Description      Pull default card from peer. p_path must point to a fully
 **                  qualified path specifying where to store the pulled card.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcPullCard(tBSA_OPC_PULL_CARD *p_pull_card_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcExchCard
 **
 ** Description      Exchange business cards with a peer device. p_send points to
 **                  a fully qualified path and file name of vcard to push.
 **                  p_recv_path points to a fully qualified path specifying
 **                  where to store the pulled card.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcExchCardInit(tBSA_OPC_EXCH_CARD *p_pull_card_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcExchCard
 **
 ** Description      Exchange business cards with a peer device. p_send points to
 **                  a fully qualified path and file name of vcard to push.
 **                  p_recv_path points to a fully qualified path specifying
 **                  where to store the pulled card.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcExchCard(tBSA_OPC_EXCH_CARD *p_pull_card_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcClose
 **
 ** Description      Close the current connection.  This function is called if
 **                  the phone wishes to close the connection before the object
 **                  push is completed.  In a typical connection this function
 **                  does not need to be called; the connection will be closed
 **                  automatically when the object push is complete.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcCloseInit(tBSA_OPC_CLOSE *p_close_req);

/*******************************************************************************
 **
 ** Function         BSA_OpcClose
 **
 ** Description      Close the current connection.  This function is called if
 **                  the phone wishes to close the connection before the object
 **                  push is completed.  In a typical connection this function
 **                  does not need to be called; the connection will be closed
 **                  automatically when the object push is complete.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_OpcClose(tBSA_OPC_CLOSE *p_close_req);


#ifdef __cplusplus
}
#endif

#endif /* BSA_OP_API_H */
