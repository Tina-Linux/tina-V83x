/********************************************************************************
**                                                                              *
**  Name:          hcrp_api.h                                                   *
**                                                                              *
**                                                                              *
**  Description:    This file contains the Hardcopy Cable Replacement Profile   *
**                  Client API definitions.                                     *
**                                                                              *
**                                                                              *
**  Copyright (c) 2001-2004, WIDCOMM Inc., All Rights Reserved.                 *
**  WIDCOMM Bluetooth Core. Proprietary and confidential.                       *
********************************************************************************/
#ifndef HCRP_API_H
#define HCRP_API_H

#include "gap_api.h"
#include "l2c_api.h"

/*****************************************************************************
**  Constants
*****************************************************************************/

/* Security Requirements (used when device is in Service Level Security (Mode 2)
** Note:  If authorization or encryption is to be used then authentication MUST
**        be enabled
*/
#define HCRP_SEC_NONE        BTM_SEC_NONE               /* Nothing required */
#define HCRP_SEC_IN_AUTHOR   BTM_SEC_IN_AUTHORIZE       /* Inbound conn requires authorization */
#define HCRP_SEC_IN_AUTHEN   BTM_SEC_IN_AUTHENTICATE    /* Inbound conn requires authentication */
#define HCRP_SEC_IN_ENCRYPT  BTM_SEC_IN_ENCRYPT         /* Inbound conn requires encryption */
#define HCRP_SEC_OUT_AUTHOR  BTM_SEC_OUT_AUTHORIZE      /* Outbound conn requires authorization */
#define HCRP_SEC_OUT_AUTHEN  BTM_SEC_OUT_AUTHENTICATE   /* Outbound conn requires authentication */
#define HCRP_SEC_OUT_ENCRYPT BTM_SEC_OUT_ENCRYPT        /* Outbound conn requires encryption */

/* Options set in registration information (HCRP_RegisterServer and HCRP_RegisterClient) */
#define HCRP_REG_OPT_NOTIFICATION   0x01  /* Application supports notifications */
#define HCRP_REG_OPT_VENDOR_SPEC    0x02  /* Application supports vendor specific commands */
#define HCRP_REG_OPT_LIMITED_DISC   0x04  /* Server uses Limited Discoverable Mode when in Public Online Mode */

/* HCRP supported device function (see HCRP profile spec) */
#define HCRP_FUNC_PRINT         ((UINT8) 0x01)  /* print function supported */
#define HCRP_FUNC_SCAN          ((UINT8) 0x02)  /* scan  function supported */

/* Offset into the BT_HDR buffer where data is written (HCRP_SendData) */
#define HCRP_BT_HDR_DATA_OFFSET     L2CAP_MIN_OFFSET
#define HCRP_BT_HDR_DATA_SIZE       (sizeof(BT_HDR) + L2CAP_MIN_OFFSET)

/* Protocol return status definitions */
#define HCRP_PROTO_UNSUPPORTED_FEATURE  ((UINT16) 0x0000)   /* No valid return params with this status */
#define HCRP_PROTO_SUCCESS              ((UINT16) 0x0001)
#define HCRP_PROTO_CREDIT_SYNC_ERROR    ((UINT16) 0x0002)
#define HCRP_PROTO_GENERIC_FAILURE      ((UINT16) 0xFFFF)

/* Max string portion that can be sent in one PDU.
** This is based on the HCRP_CMD_POOL_SIZE (target.h)
** minus the PDU header size and 2 bytes for the status code.
*/
#define HCRP_MAX_1284_PDU_LEN   (HCRP_MAX_CTRL_PARAM_LEN)

/* Register for Notifications Command */
#define HCRP_DEREGISTER_FOR_NOTIFICATIONS   0
#define HCRP_REGISTER_FOR_NOTIFICATIONS     1

/* Notification event constants */
#define HCRP_MAX_NOTIF_PARAM_LEN    (4)

/* Notification PDU IDs (0x8000-0xffff are user definable) */
#define HCRP_NOTIF_PDU_NINTERRUPT   ((UINT16) 0x0001)

/* HCRP_RegUpdate - constants */
#define HCRP_UPDATE_IGNORE_SEC      ((UINT8) 0x80)  /* do not use this parameter */

/*****************************************************************************
**  Type Definitions
*****************************************************************************/

/* HCRP application callback event codes */
enum
{
/* Common Events */
    HCRP_EVT_REGISTER_CMPL = 0, /* Results of the HCRP_RegisterServer or HCRP_RegisterClient calls */
    HCRP_EVT_DEREGISTER_CMPL,   /* Results of the HCRP_Deregister API call,
                                   or when a hard reset command is received
                                   from the peer device */
    HCRP_EVT_CHAN_OPEN,         /* Control and Data Channels are open */
    HCRP_EVT_CHAN_CLOSED,       /* Control and Data Channels are closed */
    HCRP_EVT_RCV_READY_CMPL,    /* Returns status and number of credits
                                   currently available to the peer */
    HCRP_EVT_SEND_DATA_CMPL,    /* Data sent to lower layer; ready to accept
                                   next buffer */
    HCRP_EVT_RCV_CREDITS_LOW,   /* Warning that receive credits are below watermark registered */
    HCRP_EVT_HCRP_STATUS,       /* Report of HCRP internal processing error
                                   code (see below) */
    HCRP_EVT_DEREG_NOTIF,       /* Notification have been turned off */
    HCRP_EVT_SDP_CMPL,          /* Service discovery complete event */
    HCRP_EVT_PROTO_RSP,         /* Response to a protocol message */
    HCRP_EVT_NOTIFICATION       /* Report an asynchronous notification event */
};
typedef UINT8 tHCRP_APP_EVENT;



/* HCRP Status Codes */
enum
{
    HCRP_SUCCESS = BT_PASS,      /* Success */
    HCRP_BUSY = HCRP_ERR_GRP,    /* [0x300] Unspecified failure. */
    HCRP_NO_RESOURCES,           /* [0x301] Unspecified failure to allocate resource */
    HCRP_ERR_ALREADY_REGISTERED, /* [0x302] Attempt to register HCRP failed: Already registered */
    HCRP_ERR_NOT_REGISTERED,     /* [0x303] Attempt to call API function failed: Not registered */
    HCRP_ERR_NO_BDADDR,          /* [0x304] Attempt to establish connection failed: No BD Address specified */
    HCRP_ERR_UNSUPPORTED_CMD,    /* [0x305] Unrecognized protocol command specified */
    HCRP_ERR_ILLEGAL_PARAMETER,  /* [0x306] A parameter was out of range */
    HCRP_ERR_CHAN_OPEN  ,        /* [0x307] The channel is already open */
    HCRP_ERR_CHAN_NOT_OPEN,      /* [0x308] Channel not open */
    HCRP_ERR_DISCOVERY,          /* [0x309] Attempt to perform Discovery failed. */
    HCRP_ERR_SDP_REG,            /* [0x30a] Attempt to register SDP for Discovery failed. */
    HCRP_ERR_SDP_PROTO,          /* [0x30b] Attempt to add Protocol List to SDP failed. */
    HCRP_ERR_SDP_CLASSID,        /* [0x30c] Attempt to add Class ID to SDP failed */
    HCRP_ERR_SDP_PROFILE,        /* [0x30d] Attempt to add Profile Descriptor list to SDP failed */
    HCRP_ERR_SDP_ATTR,           /* [0x30e] Attempt to add Attribute to SDP failed */
    HCRP_ERR_OPENING_GAP_CHAN,   /* [0x30f] Error opening one of the GAP channels */
    HCRP_SND_DATA_ABORTED,       /* [0x310] The send data command was aborted before finishing */
    HCRP_PROTO_RSP_TOUT,         /* [0x311] A timeout occurred waiting for a response from the server */
    HCRP_REG_NO_CMD_CBACK,       /* [0x312] No command callback registered */
    HCRP_REG_NO_DATA_CBACK,      /* [0x313] No data channel callback registered */
    HCRP_ERR_MAX
};
typedef UINT16 tHCRP_STATUS;

/***************************************************************
******** Protocol Command/Response Structure Definitions *******
****************************************************************/
enum
{
    HCRP_PROTO_VENDOR_SPEC_CMD  = 0,
    HCRP_PROTO_GET_LPT_CMD      = 5,
    HCRP_PROTO_GET_1284ID_CMD   = 6,
    HCRP_PROTO_SOFT_RESET_CMD   = 7,
    HCRP_PROTO_HARD_RESET_CMD   = 8,
    HCRP_PROTO_REG_NOTIF_CMD    = 9,
    HCRP_PROTO_NCONN_ALIVE_CMD  = 10,
    HCRP_PROTO_PARAMETER_UNUSED = 0xffff    /* Used in non-proto callback events */
};
typedef UINT16 tHCRP_PROTO_CMD_ID;

/* Type of HCRP application command callback function */
typedef void (tHCRP_CMD_CBACK) (tHCRP_APP_EVENT event, tHCRP_STATUS status,
                                tHCRP_PROTO_CMD_ID cmd, void *p_data,
                                UINT16 data_len);


/* Type of HCRP application receive data callback function
** Note: Buffer is freed after completion of callback so it needs
**       to be copied before exiting the callback
*/
typedef void (tHCRP_RCV_CBACK) (UINT8 *p_data, UINT16 data_len);

/***************************************************************
******** Registration Structure and Constant Definitions *******
****************************************************************/

/* Structure used to pass information for HCRP_RegisterClient */
typedef struct
{
    tHCRP_CMD_CBACK *cmd_cback;     /* Callback notifies application of command responses/results */
    tHCRP_RCV_CBACK *rcv_cback;     /* Callback notifies application of received data */
    UINT32           snd_low_wmark; /* credit watermark used to signal requesting more credits */
    UINT32           rcv_low_wmark; /* credit watermark used to signal granting more credits */
    UINT16           function;      /* Supported functions UUID (print or scan) [see sdpdefs.h] */
    UINT8            options;       /* HCRP_REG_OPT_NOTIFICATION |  HCRP_REG_OPT_VENDOR_SPEC  |
                                       HCRP_REG_OPT_LIMITED_DISC */
    UINT8            security;      /* Security requirements for HCRP (see options above) */

#if HCRP_NOTIFICATION_INCLUDED == TRUE
    UINT16           notif_psm;  /* PSM of the notification channel SDP record */
    UINT8            service_name_len;
    UINT8            service_name[HCRP_MAX_SERVICE_NAME_LEN];
#else
    /* Define minimal space because it isn't used. RPC requires its definition
       This else conditional can be removed if not using RPC */
    UINT16           notif_psm;  /* PSM of the notification channel SDP record */
    UINT8            service_name_len;
    UINT8            service_name[1];
#endif
} tHCRP_CLIENT_REG_INFO;


/*******************************************************************
*** Command  Definitions (HCRP_EVT_PROTO_CMD, HCRP_SendProtoCmd) ***
********************************************************************/

/* Get 1284 ID String Command API (client) */
typedef struct
{
    tHCRP_PROTO_CMD_ID  cmd;
    UINT16              start_byte;     /* Starting byte of the string to return (0 - based) */
    UINT16              num_bytes;      /* Number of bytes to return of 1284 ID string in one
                                           message. This value should not be larger than
                                           HCRP_MAX_1284_PDU_LEN.
                                        */
} tHCRP_1284_ID_CMD;

typedef struct
{
    tHCRP_PROTO_CMD_ID  cmd;
    UINT32              context_id;     /* given to server to be used for notification ID */
    UINT32              cback_timeout;  /* requested amount of time to keep registration */
    UINT8               reg_cmd;        /* Register or deregister for notifications */
} tHCRP_REG_NOTIF_CMD;

/* Vendor Specific Command API (server) */
typedef struct
{
    tHCRP_PROTO_CMD_ID  cmd;
    UINT16              vs_cmd;      /* Protocol command (value 0x8000-0xffff) */
    UINT16              param_len;   /* Cannot exceed HCRP_MAX_VEND_SPEC_LEN (target.h) */
    UINT8               param[HCRP_MAX_VEND_SPEC_LEN];
} tHCRP_VEND_SPEC_CMD;


/************************************************************
**    Protocol Response Definitions
** These structures are passed to the application in
** response to a protocol request through the
** HCRP_EVT_PROTO_RSP event.
*************************************************************/
/* General response structure */
typedef struct
{
    tHCRP_PROTO_CMD_ID  cmd;            /* Command this is a response to */
    UINT16              status_code;    /* Protocol return code */
} tHCRP_RSP_HDR;

/* Get LPT Status Response API (server) */
typedef struct
{
    tHCRP_RSP_HDR   hdr;         /* Command and status code to return to client */
    UINT8           lpt_status;  /* returned LPT status */
} tHCRP_LPT_STATUS_RSP;


/* Get 1284 ID String Response API (server) */
typedef struct
{
    tHCRP_RSP_HDR   hdr;          /* Command and status code to return to client */
    UINT16          len;          /* Length of this portion of the 1284 ID string */
    UINT8          *p_id_portion; /* returned string (or portion of) [Not NULL terminated] */
} tHCRP_1284_ID_RSP;


/* Vendor Specific Command/Response API (server) */
typedef struct
{
    tHCRP_RSP_HDR   hdr;         /* Command and status code to return to client */
    UINT16          vs_cmd;      /* Protocol command (value 0x8000-0xffff) */
    UINT16          param_len;   /* Cannot exceed HCRP_MAX_VEND_SPEC_LEN (target.h) */
    UINT8           param[HCRP_MAX_VEND_SPEC_LEN];
} tHCRP_VEND_SPEC_RSP;


/* Register for Notifications Response */
typedef struct
{
    tHCRP_RSP_HDR   hdr;            /* Command and status code to return to client */
    UINT32          timeout;        /* milliseconds to remain registered */
    UINT32          cback_timeout;  /* Time that server will keep the notifictaion channel open */
} tHCRP_REG_NOTIF_RSP;

/* Notification Connection Alive Response */
typedef struct
{
    tHCRP_RSP_HDR   hdr;                /* Command and status code to return to client */
    UINT32          timeout_increment;  /* sent to client with increased connection timer (if any) */
} tHCRP_NOTIF_CONN_ALIVE_RSP;


/******************************************************************
******** Channel Open Event Definition (HCRP_EVT_CHAN_OPEN) *******
*******************************************************************/
typedef struct
{
    UINT16  peer_ctrl_mtu;  /* Maximum size of the peer's control channel */
    UINT16  peer_data_mtu;  /* Maximum size of the peer's data channel */
    BD_ADDR peer_bd_addr;   /* Peer device's bd address */
} tHCRP_CHAN_OPEN_INFO;


/********************************************************************
******** Send Data Event Definition (HCRP_EVT_SEND_DATA_CMPL) *******
**  This event a pointer to the number of bytes written out        **
*********************************************************************/

/********************************************************************
******** Find Services Results Structures (HCRP_EVT_SDP_CMPL) *******
*********************************************************************/

/* Results of a service search */
typedef struct
{
    UINT16  ctrl_psm;
    UINT16  data_psm;
    UINT16  service_id;
    UINT16  version;
    UINT16  id_1284_len;                                /* Length of UTF-8 1284 ID string */
    UINT8   function;       /* HCRP_FUNC_PRINT, HCRP_FUNC_SCAN */
    UINT8   service_name_len;                           /* Length of UTF-8 service name */
    UINT8   device_name_len;                            /* Length of UTF-8 device name */
    UINT8   friendly_name_len;                          /* Length of UTF-8 friendly name */
    UINT8   device_location_len;                        /* Length of UTF-8 device location */
    UINT8   device_name[HCRP_MAX_DEVICE_NAME_LEN];      /* Model Specific Display Name */
    UINT8   service_name[HCRP_MAX_SERVICE_NAME_LEN];    /* Service Name */
    UINT8   friendly_name[HCRP_MAX_FRIENDLY_NAME_LEN];  /* Friendly name */
    UINT8   id_1284[HCRP_MAX_SDP_1284_ID_LEN];          /* 1284 ID string */
    UINT8   device_location[HCRP_MAX_DEVICE_LOC_LEN];   /* Maximum length of the device location string */
} tHCRP_RESULT_RECORD;

typedef struct
{
    tHCRP_RESULT_RECORD  result[HCRP_MAX_SEARCH_RESULTS];
    UINT16               sdp_status;    /* Contains detailed error if status is HCRP_ERR_DISCOVERY */
    UINT8                num_results;   /* number of results found on remote device */
} tHCRP_SEARCH_RESULTS;

/* Notification Event Structure Definitions */
typedef struct
{
    UINT32              context_id; /* given to server to be used for notification ID */
    UINT16              param_len;  /* Number of parameter bytes to follow */
    UINT16              pdu_id;     /* Register or deregister for notifications */
#if HCRP_MAX_NOTIF_PARAM_LEN > 0
    UINT8               params[HCRP_MAX_NOTIF_PARAM_LEN];
#endif
} tHCRP_NOTIF_CMD;


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/******************    Client API Functions *********************/
/******************************************************************************
**
** Function     HCRP_Deregister
**
** Description  Deregister a client or server.
**              If the return status is HCRP_SUCCESS, the HCRP_EVT_DEREGISTER_CMPL
**              event will signal the completion of registration (with status).
**
** Returns      tHCRP_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRP_STATUS HCRP_Deregister(void);


/******************************************************************************
**
** Function     HCRP_SendData
**
** Description  Send raw data to the peer device.
**
** Returns      The HCRP_EVT_SEND_DATA_CMPL event returns the status of the request.
**
** Return Value tHCRP_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRP_STATUS HCRP_SendData (BT_HDR *p_buf);


/******************************************************************************
**
** Function     HCRP_RcvReady
**
** Description  API call used to flow control the peer.  It passes the number
**                  of (additional) bytes the application can handle.  The peer
**                  device cannot send more bytes than it has credits granted.
**
** Returns      The HCRP_EVT_RCV_READY_CMPL event returns the status of the request,
**              along with the total number of unused credits.
**
** Return Value tHCRP_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRP_STATUS HCRP_RcvReady(UINT32 credits);


/*******************************************************************************
**
** Function         HCRP_SetTraceLevel
**
** Description      This function sets the trace level for HCRP. If called with
**                  a value of 0xFF, it simply reads the current trace level.
**
** Returns          the new (current) trace level
**
*******************************************************************************/
HCRP_API extern UINT8 HCRP_SetTraceLevel (UINT8 new_level);


/*******************************************************************************
**
** Function         HCRP_Init
**
** Description      Initializes internal data structures at stack startup or
**                  startup of HCRP application (Once before HCRP_Register is called.
**
** Return Value     void
**
*******************************************************************************/
HCRP_API extern void HCRP_Init (void);


/******************************************************************************
**
** Function     HCRP_RegisterClient
**
** Description  This function is called to register a client.
**              It must be called before any other HCRP
**              API function is called.  If the return status is HCRP_SUCCESS,
**              the HCRP_EVT_REGISTER_CMPL event will signal the completion of
**              registration (with status).
**
** Return Value tHCRP_STATUS   (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRP_STATUS HCRP_RegisterClient(tHCRP_CLIENT_REG_INFO *p_reginfo);


/******************************************************************************
**
** Function     HCRP_EstablishConnection
**
** Description  API call used to establish a connection to an HCRP server.
**
** Return Value tHCRP_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRP_STATUS HCRP_EstablishConnection(BD_ADDR bd_addr,
                                                      UINT16 ctrl_psm,
                                                      UINT16 data_psm);


/******************************************************************************
**
** Function     HCRP_ReleaseConnection
**
** Description  API call used to disconnect from a peer server.
**
** Return Value tHCRP_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRP_STATUS HCRP_ReleaseConnection(void);


/******************************************************************************
**
** Function     HCRP_FindServices
**
** Description  API call used to search for a specified service on a remote device.
**
** Return Value tHCRP_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRP_STATUS HCRP_FindServices(BD_ADDR bd_addr,
                                               tSDP_DISCOVERY_DB *p_db,
                                               UINT32 db_len, UINT8 function);


/******************************************************************************
**
** Function     HCRP_SendProtoCmd
**
** Description  API call used to send a protocol request to the server.
**
** Return Value tHCRP_STATUS    (See descriptions in hcrp_api.h)
**
******************************************************************************/
HCRP_API extern tHCRP_STATUS HCRP_SendProtoCmd(tHCRP_PROTO_CMD_ID cmd,
                                               void *p_msg);

#ifdef __cplusplus
}
#endif

#endif /* HCRP_API_H */
