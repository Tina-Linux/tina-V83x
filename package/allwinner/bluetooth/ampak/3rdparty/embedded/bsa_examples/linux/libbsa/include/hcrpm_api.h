/********************************************************************************
**                                                                              *
**  Name:          hcrpm_api.h                                                  *
**                                                                              *
**                                                                              *
**  Description:    This file contains the Hardcopy Cable Replacement Profile   *
**                  Multi-Client Server API definitions.                        *
**                                                                              *
**                                                                              *
**  Copyright (c) 2002-2004, WIDCOMM Inc., All Rights Reserved.                 *
**  WIDCOMM Bluetooth Core. Proprietary and confidential.                       *
********************************************************************************/
#ifndef HCRPM_API_H
#define HCRPM_API_H

#include "gap_api.h"
#include "l2c_api.h"

/******************************************************************************
**
**                              HCRPM API Definitions
**
******************************************************************************/

/* Security Requirements (used when device is in Service Level Security (Mode 2)
** Note:  If authorization or encryption is to be used then authentication MUST
**        be enabled
*/
#define HCRPM_SEC_NONE          BTM_SEC_NONE            /* Nothing required */
#define HCRPM_SEC_IN_AUTHOR     BTM_SEC_IN_AUTHORIZE    /* Inbound conn requires authorization */
#define HCRPM_SEC_IN_AUTHEN     BTM_SEC_IN_AUTHENTICATE /* Inbound conn requires authentication */
#define HCRPM_SEC_IN_ENCRYPT    BTM_SEC_IN_ENCRYPT      /* Inbound conn requires encryption */

/* Options set in registration information (HCRPM_RegisterServer) */
#define HCRPM_REG_OPT_NOTIFICATION  0x01  /* Application supports notifications */
#define HCRPM_REG_OPT_VENDOR_SPEC   0x02  /* Application supports vendor specific commands */
#define HCRPM_REG_OPT_LIMITED_DISC  0x04  /* Server uses Limited Discoverable Mode when in Public Online Mode */

/* HCRPM supported device function (see HCRP profile spec) */
#define HCRPM_FUNC_PRINT            ((UINT16) 0x01)  /* print function supported */
#define HCRPM_FUNC_SCAN             ((UINT16) 0x02)  /* scan  function supported */

/* Offset into the BT_HDR buffer where data is written (HCRPM_SendData) */
#define HCRPM_BT_HDR_DATA_OFFSET    L2CAP_MIN_OFFSET
#define HCRPM_BT_HDR_DATA_SIZE      (sizeof(BT_HDR) + L2CAP_MIN_OFFSET)

/* Protocol return status definitions */
#define HCRPM_PROTO_UNSUPPORTED_FEATURE ((UINT16) 0x0000)   /* No valid return params with this status */
#define HCRPM_PROTO_SUCCESS             ((UINT16) 0x0001)
#define HCRPM_PROTO_CREDIT_SYNC_ERROR   ((UINT16) 0x0002)
#define HCRPM_PROTO_GENERIC_FAILURE     ((UINT16) 0xFFFF)

/* Max string portion that can be sent in one PDU.
** This is based on the HCRP_CMD_POOL_SIZE (target.h)
** minus the PDU header size and 2 bytes for the status code.
*/
#define HCRPM_MAX_1284_PDU_LEN      HCRPM_MAX_CTRL_PARAM_LEN

/* Notification event constants */
#define HCRPM_MAX_NOTIF_PARAM_LEN   4

/* Register for Notifications Command */
#define HCRPM_DEREGISTER_FOR_NOTIFICATIONS  0
#define HCRPM_REGISTER_FOR_NOTIFICATIONS    1

/* Notification PDU IDs (0x8000-0xffff are user definable) */
#define HCRPM_NOTIF_PDU_NINTERRUPT  ((UINT16) 0x0001)

/* HCRPM_RegUpdate - constants */
#define HCRPM_UPDATE_IGNORE_SEC     ((UINT8) 0x80)  /* do not use this parameter */

#define HCRPM_IGNORE_HANDLE         ((UINT8) 0xFF)  /* Not used if set to 0xff */

/*****************************************************************************
**  Type Definitions
*****************************************************************************/

/* Server Modes */
enum
{
    HCRPM_OFFLINE        = 0,
    HCRPM_PRIVATE_ONLINE = 1,
    HCRPM_PUBLIC_ONLINE  = 2
};
typedef UINT8 tHCRPM_SERVER_MODE;

/* HCRPM application callback event codes */
enum
{
/* Common Events */
    HCRPM_EVT_REGISTER_CMPL = 0, /* Results of the HCRPM_RegisterServer calls */
    HCRPM_EVT_DEREGISTER_CMPL,   /* Results of the HCRPM_Deregister API call,
                                    or when a hard reset command is received
                                    from the peer device */
    HCRPM_EVT_CHAN_OPEN,         /* Control and Data Channels are open */
    HCRPM_EVT_CHAN_CLOSED,       /* Control and Data Channels are closed */
    HCRPM_EVT_RCV_READY_CMPL,    /* Returns status and number of credits
                                    currently available to the peer */
    HCRPM_EVT_SEND_DATA_CMPL,    /* Data sent to lower layer; ready to accept
                                    next buffer */
    HCRPM_EVT_RCV_CREDITS_LOW,   /* Warning that receive credits are below watermark registered */
    HCRPM_EVT_HCRP_STATUS,       /* Report of HCRPM internal processing error
                                    code (see below) */
    HCRPM_EVT_DEREG_NOTIF,       /* Notification have been turned off */
    HCRPM_EVT_MODE_CMPL,         /* Mode change request complete event */
    HCRPM_EVT_PROTO_CMD,         /* Request received from client; need a reply */
    HCRPM_EVT_NOTIF_CMPL,        /* Notification has completed */
    HCRPM_EVT_UPDATE_CMPL        /* Registration update has completed */
};
typedef UINT8 tHCRPM_APP_EVENT;

/* HCRPM Status Codes */
enum
{
    HCRPM_SUCCESS = BT_PASS,      /* Success */
    HCRPM_FAILURE = HCRPM_ERR_GRP,/* [0x400] Unspecified failure. */
    HCRPM_NO_RESOURCES,           /* [0x401] Unspecified failure to allocate resource */
    HCRPM_BUSY,                   /* [0x402] A command is already in progress */
    HCRPM_ERR_ALREADY_REGISTERED, /* [0x403] Attempt to register HCRPM failed: Already registered */
    HCRPM_ERR_NOT_REGISTERED,     /* [0x404] Attempt to call API function failed: Not registered */
    HCRPM_ERR_NO_BDADDR,          /* [0x405] Attempt to establish connection failed: No BD Address specified */
    HCRPM_ERR_UNSUPPORTED_CMD,    /* [0x406] Unrecognized protocol command specified */
    HCRPM_ERR_ILLEGAL_PARAMETER,  /* [0x407] A parameter was out of range */
    HCRPM_ERR_CHAN_OPEN  ,        /* [0x408] The channel is already open */
    HCRPM_ERR_CHAN_NOT_OPEN,      /* [0x409] Channel not open */
    HCRPM_ERR_DISCOVERY,          /* [0x40a] Attempt to perform Discovery failed. */
    HCRPM_ERR_SDP_REG,            /* [0x40b] Attempt to register SDP for Discovery failed. */
    HCRPM_ERR_SDP_PROTO,          /* [0x40c] Attempt to add Protocol List to SDP failed. */
    HCRPM_ERR_SDP_CLASSID,        /* [0x40d] Attempt to add Class ID to SDP failed */
    HCRPM_ERR_SDP_PROFILE,        /* [0x40e] Attempt to add Profile Descriptor list to SDP failed */
    HCRPM_ERR_SDP_ATTR,           /* [0x40f] Attempt to add Attribute to SDP failed */
    HCRPM_ERR_OPENING_GAP_CHAN,   /* [0x410] Error opening one of the GAP channels */
    HCRPM_SND_DATA_ABORTED,       /* [0x411] The send data command was aborted before finishing */
    HCRPM_REG_NO_CMD_CBACK,       /* [0x412] No command callback registered */
    HCRPM_REG_NO_DATA_CBACK,      /* [0x413] No data channel callback registered */
    HCRPM_PROTO_RSP_NOT_PENDING,  /* [0x414] Received a reply to a command that was not pending */
    HCRPM_NOTIF_FAILED,           /* [0x415] Notification failed (No service or not connectable */
    HCRPM_BAD_HANDLE,             /* [0x416] Bad Client Handle Received */
    HCRPM_ERR_MAX
};
typedef UINT16 tHCRPM_STATUS;

/****************************************************************************
**
**                         HCRPM API Structure Definitions
**
*****************************************************************************/
/****************************************************************************
******** Protocol Command/Response Structure and Constant Definitions *******
*****************************************************************************/
enum
{
    HCRPM_PROTO_VENDOR_SPEC_CMD  = 0,
    HCRPM_PROTO_GET_LPT_CMD      = 5,
    HCRPM_PROTO_GET_1284ID_CMD   = 6,
    HCRPM_PROTO_SOFT_RESET_CMD   = 7,
    HCRPM_PROTO_HARD_RESET_CMD   = 8,
    HCRPM_PROTO_REG_NOTIF_CMD    = 9,
    HCRPM_PROTO_NCONN_ALIVE_CMD  = 10,
    HCRPM_PROTO_PARAMETER_UNUSED = 0xffff    /* Used in non-proto callback events */
};
typedef UINT16 tHCRPM_PROTO_CMD_ID;

/* Type of HCRPM application command callback function */
typedef void (tHCRPM_CMD_CBACK) (UINT8 handle, tHCRPM_APP_EVENT event,
                                 tHCRPM_STATUS status, tHCRPM_PROTO_CMD_ID cmd,
                                 void *p_data, UINT16 data_len);


/* Type of HCRPM application receive data callback function
** Note: Buffer is freed after completion of callback so it needs
**       to be copied before exiting the callback
*/
typedef void (tHCRPM_RCV_CBACK) (UINT8 handle, UINT8 *p_data, UINT16 data_len);

/***************************************************************
******** Registration Structure and Constant Definitions *******
****************************************************************/

/* Structure used to pass information for HCRPM_RegisterServer */
typedef struct
{
    tHCRPM_CMD_CBACK *cmd_cback;     /* Callback notifies application of command responses/results */
    tHCRPM_RCV_CBACK *rcv_cback;     /* Callback notifies application of received data */
    UINT32            rcv_low_wmark; /* credit watermark used to signal application to grant more credits */
    UINT16            function;      /* Supported functions UUID (print or scan) [see sdpdefs.h] */
    UINT16            base_psm;      /* Used in determining the control and data PSMs for the server */
    UINT16            id_1284_len;                               /* Length of UTF-8 1284 ID string */
    UINT8             max_clients;                               /* Maximum clients simultaneously connected */
    UINT8             options;       /* HCRPM_REG_OPT_NOTIFICATION |  HCRPM_REG_OPT_VENDOR_SPEC  |
                                        HCRPM_REG_OPT_LIMITED_DISC */
    UINT8             security;      /* Security requirements for HCRPM (see options above) */
    UINT8             service_name_len;                          /* Length of UTF-8 service name */
    UINT8             device_name_len;                           /* Length of UTF-8 device name */
    UINT8             friendly_name_len;                         /* Length of UTF-8 friendly name */
    UINT8             device_location_len;                       /* Length of UTF-8 device location */
    UINT8             service_name[HCRP_MAX_SERVICE_NAME_LEN];   /* Service name to present during Service Discovery */
    UINT8             device_name[HCRP_MAX_DEVICE_NAME_LEN];     /* Device name to present during Service Discovery */
    UINT8             friendly_name[HCRP_MAX_FRIENDLY_NAME_LEN]; /* Friendly name to present during Service Discovery */
    UINT8             id_1284[HCRP_MAX_SDP_1284_ID_LEN];         /* Maximum length of the 1284 ID string */
    UINT8             device_location[HCRP_MAX_DEVICE_LOC_LEN];  /* Maximum length of the device location string */
} tHCRPM_SERVER_REG_INFO;

/* Structure used to pass information for HCRPM_RegUpdate */
typedef struct
{
    UINT8            security;          /* Same as register params (HCRPM_UPDATE_IGNORE_SEC ignores) */
    UINT8            device_name_len;                           /* Length of UTF-8 device name */
    UINT8            device_location_len;                       /* Length of UTF-8 device location */
    UINT8            device_name[HCRP_MAX_DEVICE_NAME_LEN];     /* Device name to present during Service Discovery */
    UINT8            device_location[HCRP_MAX_DEVICE_LOC_LEN];  /* Maximum length of the device location string */
} tHCRPM_REG_UPDATE_INFO;


/*******************************************************************
*** Command  Definitions (HCRPM_EVT_PROTO_CMD, HCRPM_SendProtoCmd) ***
********************************************************************/

/* Get 1284 ID String Command API */
typedef struct
{
    tHCRPM_PROTO_CMD_ID cmd;
    UINT16              start_byte;     /* Starting byte of the string to return (0 - based) */
    UINT16              num_bytes;      /* Number of bytes to return of 1284 ID string in one
                                           message. This value should not be larger than
                                           HCRP_MAX_1284_PDU_LEN.
                                        */
} tHCRPM_1284_ID_CMD;

typedef struct
{
    tHCRPM_PROTO_CMD_ID cmd;
    UINT32              context_id;     /* given to server to be used for notification ID */
    UINT32              cback_timeout;  /* requested amount of time to keep registration */
    BD_ADDR             rba;            /* remote device address */
    UINT8               reg_cmd;        /* Register or deregister for notifications */
} tHCRPM_REG_NOTIF_CMD;

/* Vendor Specific Command API */
typedef struct
{
    tHCRPM_PROTO_CMD_ID cmd;
    UINT16              vs_cmd;      /* Protocol command (value 0x8000-0xffff) */
    UINT16              param_len;   /* Cannot exceed HCRP_MAX_VEND_SPEC_LEN (target.h) */
    UINT8               param[HCRP_MAX_VEND_SPEC_LEN];
} tHCRPM_VEND_SPEC_CMD;


/************************************************************
**    Protocol Response Definitions
** (HCRPM_EVT_PROTO_RSP, HCRPM_SendProtoRsp)
** These structures are passed by the application to
** respond to a protocol request (server), and are passed
** by the stack to the peer client.
*************************************************************/
/* General response structure */
typedef struct
{
    tHCRPM_PROTO_CMD_ID cmd;            /* Command this is a response to */
    UINT16              status_code;    /* Protocol return code */
} tHCRPM_RSP_HDR;

/* Get LPT Status Response API (server) */
typedef struct
{
    tHCRPM_RSP_HDR  hdr;         /* Command and status code to return to client */
    UINT8           lpt_status;  /* returned LPT status */
} tHCRPM_LPT_STATUS_RSP;


/* Get 1284 ID String Response API (server) */
typedef struct
{
    tHCRPM_RSP_HDR  hdr;          /* Command and status code to return to client */
    UINT16          len;          /* Length of this portion of the 1284 ID string */
    UINT8          *p_id_portion; /* returned string (or portion of) [Not NULL terminated] */
} tHCRPM_1284_ID_RSP;


/* Vendor Specific Command/Response API (server) */
typedef struct
{
    tHCRPM_RSP_HDR  hdr;         /* Command and status code to return to client */
    UINT16          vs_cmd;      /* Protocol command (value 0x8000-0xffff) */
    UINT16          param_len;   /* Cannot exceed HCRP_MAX_VEND_SPEC_LEN (target.h) */
    UINT8           param[HCRP_MAX_VEND_SPEC_LEN];
} tHCRPM_VEND_SPEC_RSP;


/* Register for Notifications Response */
typedef struct
{
    tHCRPM_RSP_HDR  hdr;            /* Command and status code to return to client */
    BD_ADDR         rba;            /* Passed in with the registration event */
    UINT32          context_id;     /* Context_id passed in with event */
    UINT32          timeout;        /* milliseconds to remain registered */
    UINT32          cback_timeout;  /* Time that server will keep the notifictaion channel open */
} tHCRPM_REG_NOTIF_RSP;

/* Notification Connection Alive Response */
typedef struct
{
    tHCRPM_RSP_HDR  hdr;                /* Command and status code to return to client */
    UINT32          timeout_increment;  /* sent to client with increased connection timer (if any) */
} tHCRPM_NOTIF_CONN_ALIVE_RSP;


/******************************************************************
********* Registration Complete (HCRPM_EVT_REGISTER_CMPL) *********
*******************************************************************/
typedef struct
{
    UINT32  sdp_handle;                 /* Service record handle */
    UINT8   chandle[HCRPM_MAX_CLIENTS]; /* Handle for each client */
} tHCRPM_REG_CMPL_INFO;


/******************************************************************
******** Channel Open Event Definition (HCRPM_EVT_CHAN_OPEN) *******
*******************************************************************/
typedef struct
{
    UINT16  peer_ctrl_mtu;  /* Maximum size of the peer's control channel */
    UINT16  peer_data_mtu;  /* Maximum size of the peer's data channel */
    BD_ADDR peer_bd_addr;   /* Peer device's bd address */
} tHCRPM_CHAN_OPEN_INFO;


/********************************************************************
******* Send Data Event Definition (HCRPM_EVT_SEND_DATA_CMPL) *******
**  This event a pointer to the number of bytes written out        **
*********************************************************************/


/****************************************************************************
* Client Notification Deregistered Event Definition (HCRPM_EVT_DEREG_NOTIF) *
*****************************************************************************/
typedef struct
{
    UINT32  context_id;     /* Context ID of client */
    BD_ADDR peer_bd_addr;   /* Peer device's bd address */
} tHCRPM_NOTIF_REG_INFO;


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/****************** Multi-Client Server API Functions *********************/

/*******************************************************************************
**
** Function         HCRPM_Init
**
** Description      Initializes internal data structures, along with tracing
**                  default value.
**
** Return Value     None.
**
*******************************************************************************/
HCRP_API extern void HCRPM_Init(void);


/******************************************************************************
**
** Function     HCRPM_SendData
**
** Description  Send raw data to the peer device.
**
** Returns      The HCRPM_EVT_SEND_DATA_CMPL event returns the status of the request.
**
** Return Value tHCRPM_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRPM_STATUS HCRPM_SendData (UINT8 handle, BT_HDR *p_buf);


/******************************************************************************
**
** Function     HCRPM_RcvReady
**
** Description  API call used to flow control the peer.  It passes the number
**                  of (additional) bytes the application can handle.  The peer
**                  device cannot send more bytes than it has credits granted.
**
** Returns      The HCRPM_EVT_RCV_READY_CMPL event returns the status of the request,
**              along with the total number of unused credits.
**
** Return Value tHCRPM_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRPM_STATUS HCRPM_RcvReady (UINT8 handle, UINT32 credits);


/******************************************************************************
**
** Function     HCRPM_RegisterServer
**
** Description  This function is called to register a server.
**              It must be called before any other HCRP
**              API function is called.  If the return status is HCRPM_SUCCESS,
**              the HCRPM_EVT_REGISTER_CMPL event will signal the completion of
**              registration (with status).
**
** Return Value tHCRPM_STATUS   (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRPM_STATUS HCRPM_RegisterServer(tHCRPM_SERVER_REG_INFO *p_reginfo);


/******************************************************************************
**
** Function      HCRPM_RegUpdate
**
** Description   This function is called to change the security requirements,
**               along with the device name and location in the Server's SDP record.
**
**               If the return status is HCRP_SUCCESS,
**               the HCRPM_EVT_UPDATE_CMPL event will signal the completion of
**               registration (with status).
**
** Parameters    p_reginfo      Pointer to registration update information
**                              server type tHCRPM_REG_UPDATE_INFO
**                              (see hcrp_api.h)
**                              device location set to NULL is ignored
**                              device name set to NULL is ignored
**                              security set to HCRP_UPDATE_IGNORE_SEC is ignored
**
** Return Value  tHCRP_STATUS   (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRPM_STATUS HCRPM_RegUpdate (tHCRPM_REG_UPDATE_INFO *p_reginfo);


/******************************************************************************
**
** Function     HCRPM_SendProtoRsp
**
** Description  API call used to send a protocol request to the server.
**
** Return Value tHCRPM_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRPM_STATUS HCRPM_SendProtoRsp(UINT8 handle,
                                                 tHCRPM_PROTO_CMD_ID cmd,
                                                 void *p_rsp);


/******************************************************************************
**
** Function     HCRPM_Deregister
**
** Description  Deregister a client or server.
**              If the return status is HCRPM_SUCCESS, the HCRPM_EVT_DEREGISTER_CMPL
**              event will signal the completion of registration (with status).
**
** Returns      tHCRPM_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRPM_STATUS HCRPM_Deregister(void);


/*******************************************************************************
**
** Function         HCRPM_SetTraceLevel
**
** Description      This function sets the trace level for HCRP. If called with
**                  a value of 0xFF, it simply reads the current trace level.
**
** Returns          the new (current) trace level
**
*******************************************************************************/
HCRP_API extern UINT8 HCRPM_SetTraceLevel (UINT8 new_level);


/******************************************************************************
**
** Function     HCRPM_SetMode
**
** Description  API call used to place a server into a particular mode
**
** Return Value tHCRPM_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRPM_STATUS HCRPM_SetMode(tHCRPM_SERVER_MODE mode);


/******************************************************************************
**
** Function     HCRPM_GetMode
**
** Description  API call used to read the current mode of the server.
**
** Returns      HCRPM_OFFLINE, HCRPM_PRIVATE_ONLINE, or HCRPM_PUBLIC_ONLINE
**
******************************************************************************/
HCRP_API extern tHCRPM_SERVER_MODE HCRPM_GetMode(void);


/******************************************************************************
**
** Function      HCRPM_SendNotification
**
** Description   API call used to send a notification to a client
**
** Return Value  tHCRPM_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRPM_STATUS HCRPM_SendNotification(UINT16  pdu_id,
                                                     BD_ADDR rba,
                                                     UINT32  context_id,
                                                     UINT16  param_len,
                                                     UINT8  *params);

#ifdef __cplusplus
}
#endif

#endif /* HCRPM_API_H */
