/*****************************************************************************
**
**  Name:           bsa_sac_api.h
**
**  Description:    This is the implementation of the API for the SIM
**                  Access Profile (SAP) client subsystem of BSA
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BSA_SAC_API_H
#define BSA_SAC_API_H

#include "bsa_api.h"
#include "sap_api.h"

/* #include "uipc.h" */

/* for tBSA_STATUS */
#include "bsa_status.h"


/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* Connection handle used by BSA SAP client */
typedef UINT16 tBSA_SAC_HANDLE;
#define BSA_SAC_INVALID_HANDLE              0xFFFF

/* Connection status in CONNECT_RESP message from server */
/* OK, Server can fulfill requirements */
#define BSA_SAC_CONN_OK                     SAP_CONN_OK
/* Error, Server unable to establish connection */
#define BSA_SAC_CONN_ERR                    SAP_CONN_ERR
/* Error, Server does not support maximum message size */
#define BSA_SAC_CONN_MSG_SIZE_UNSUPPORTED   SAP_CONN_MSG_SIZE_UNSUPPORTED
/* Error, maximum message size by Client is too small */
#define BSA_SAC_CONN_MSG_SIZE_TOO_SMALL     SAP_CONN_MSG_SIZE_TOO_SMALL
/* OK, ongoing call */
#define BSA_SAC_CONN_OK_ONGOING_CALL        SAP_CONN_OK_ONGOING_CALL
typedef UINT8 tBSA_SAC_CONN_STATUS;

/* Response result code from SAP server */
/* OK, request processed correctly */
#define BSA_SAC_RESULT_OK                   SAP_RESULT_OK
/* Error, no reason specified */
#define BSA_SAC_RESULT_ERROR                SAP_RESULT_ERROR
/* Error, card inserted but not accessible */
#define BSA_SAC_RESULT_CARD_NOT_ACCESSIBLE  SAP_RESULT_CARD_NOT_ACCESSIBLE
/* Error, card already powered off */
#define BSA_SAC_RESULT_CARD_ALREADY_OFF     SAP_RESULT_CARD_ALREADY_OFF
/* Error, card not inserted */
#define BSA_SAC_RESULT_CARD_REMOVED         SAP_RESULT_CARD_REMOVED
/* Error, card already powered on */
#define BSA_SAC_RESULT_CARD_ALREADY_ON       SAP_RESULT_CARD_ALREADY_ON
/* Error, data not available */
#define BSA_SAC_RESULT_DATA_NOT_AVAILABLE   SAP_RESULT_DATA_NOT_AVAILABLE
/* Error, not supported */
#define BSA_SAC_RESULT_NOT_SUPPORTED        SAP_RESULT_NOT_SUPPORTED
typedef UINT8 tBSA_SAC_RESULT_CODE;

/* Disconnection type in DISCONNECT_IND message from server */
#define BSA_SAC_DISC_GRACEFUL               SAP_DISCONNECT_GRACEFUL
#define BSA_SAC_DISC_IMMEDIATE              SAP_DISCONNECT_IMMEDIATE
typedef UINT8 tBSA_SAC_DISC_TYPE;

/* Status change in STATUS_IND message from server */
/* Unknown problem with SIM card */
#define BSA_SAC_STATUS_UNKNOWN_ERROR        SAP_CARD_UNKNOWN_ERROR
/* SIM inserted and powered on prior to SIM Access Profile connection */
#define BSA_SAC_STATUS_CARD_RESET           SAP_CARD_RESET
/* SIM inserted, but not accessible */
#define BSA_SAC_STATUS_CARD_NOT_ACCESSIBLE  SAP_CARD_NOT_ACCESSIBLE
/* SIM not inserted, or has been removed */
#define BSA_SAC_STATUS_CARD_REMOVED         SAP_CARD_REMOVED
/* SIM inserted, but not powered on. Client needs to power on the SIM before using it */
#define BSA_SAC_STATUS_CARD_INSERTED        SAP_CARD_INSERTED
/* Previously not accessible card has been made accessible again, and is powered on by server */
#define BSA_SAC_STATUS_CARD_RECOVERED       SAP_CARD_RECOVERED
typedef UINT8 tBSA_SAC_SERVER_STATUS;

/* Set SIM operation type */
#define BSA_SAC_SIM_POWER_ON                0       /* Power on SIM */
#define BSA_SAC_SIM_POWER_OFF               1       /* Power off SIM */
#define BSA_SAC_SIM_RESET                   2       /* Reset SIM */
typedef UINT8 tBSA_SAC_SET_SIM_TYPE;

/* Transport protocol used between client and server */
#define BSA_SAC_TRANS_PROCOTOL_T_0          SAP_TRANS_PROCOTOL_T_0   /* Type 0 */
#define BSA_SAC_TRANS_PROCOTOL_T_1          SAP_TRANS_PROCOTOL_T_1   /* Type 1 */
typedef UINT8 tBSA_SAC_TRANS_PROTOCOL;

/**************************
**  Structure definitions
***************************/

/*****************************************************************************
**  Constants and Type Definitions
*****************************************************************************/

/* BSA SAP client callback events */
typedef enum
{
    BSA_SAC_ENABLE_EVT,             /* SAP client enable result */
    BSA_SAC_DISABLE_EVT,            /* SAP client disable result */
    BSA_SAC_CONNECT_EVT,            /* Connect result */
    BSA_SAC_DISCONNECT_EVT,         /* Disconnect result */
    BSA_SAC_TRANS_APDU_EVT,         /* Transfer APDU result */
    BSA_SAC_GET_ATR_EVT,            /* Get ATR result */
    BSA_SAC_SET_SIM_EVT,            /* Set SIM result */
    BSA_SAC_CARD_READER_STATUS_EVT, /* Get card reader status result */
    BSA_SAC_SET_PROTOCOL_EVT,       /* Set transport protocol result */
    BSA_SAC_DISCONNECT_IND_EVT,     /* Disconnect request from server */
    BSA_SAC_STATUS_IND_EVT,         /* Server status update */
    BSA_SAC_ERROR_RESP_EVT,          /* Error response from server */
} tBSA_SAC_EVT;


/* BSA_SAC_ENABLE_EVT callback event data */
typedef struct
{
    tBSA_STATUS             status;
} tBSA_SAC_ENABLE_MSG;

/* BSA_SAC_DISABLE_EVT callback event data */
typedef struct
{
    tBSA_STATUS             status;
} tBSA_SAC_DISABLE_MSG;

/* BSA_SAC_CONNECT_EVT callback event data */
typedef struct
{
    tBSA_STATUS             status;
    tBSA_SAC_CONN_STATUS    conn_status;        /* CONNECT_RESP status from SAP server */
    tBSA_SAC_HANDLE         handle;             /* Connection handle */
    UINT16                  max_msg_size;       /* Maximum message size */
} tBSA_SAC_CONNECT_MSG;

/* BSA_SAC_DISCONNECT_EVT callback event data */
typedef struct
{
    tBSA_STATUS             status;
} tBSA_SAC_DISCONNECT_MSG;

/* BSA_SAC_TRANS_APDU_EVT callback event data */
typedef struct
{
    tBSA_STATUS             status;
    tBSA_SAC_RESULT_CODE    result_code;        /* Response result code from SAP server */
    UINT8*                  p_apdu;             /* Pointer to APDU payload */
    UINT16                  apdu_len;           /* Length of APDU payload */
} tBSA_SAC_TRANS_APDU_MSG;

/* BSA_SAC_GET_ATR_EVT callback event data */
typedef struct
{
    tBSA_STATUS             status;
    tBSA_SAC_RESULT_CODE    result_code;        /* Response result code from SAP server */
    UINT8*                  p_atr;              /* Pointer to ATR payload */
    UINT16                  atr_len;            /* Length of ATR data */
} tBSA_SAC_GET_ATR_MSG;

/* BSA_SAC_SET_SIM_EVT callback event data */
typedef struct
{
    tBSA_STATUS             status;
    tBSA_SAC_RESULT_CODE    result_code;        /* Response result code from SAP server */
    tBSA_SAC_SET_SIM_TYPE   set_sim;            /* On/Off/Reset */
} tBSA_SAC_SET_SIM_MSG;

/* BSA_SAC_CARD_READER_STATUS_EVT callback event data */
typedef struct
{
    tBSA_STATUS             status;
    tBSA_SAC_RESULT_CODE    result_code;        /* Response result code from SAP server */
    UINT8                   reader_status;      /* Card reader status */
} tBSA_SAC_CARD_READER_STATUS_MSG;

/* BSA_SAC_SET_PROTOCOL_EVT callback event data */
typedef struct
{
    tBSA_STATUS             status;
    tBSA_SAC_RESULT_CODE    result_code;        /* Response result code from SAP server */
} tBSA_SAC_SET_PROTOCOL_MSG;

/* BSA_SAC_DISCONNECT_IND_EVT callback event data */
typedef struct
{
    tBSA_SAC_DISC_TYPE      disc_type;          /* Disconnect type */
} tBSA_SAC_DISCONNECT_IND_MSG;

/* BSA_SAC_STATUS_IND_EVT callback event data */
typedef struct
{
    tBSA_SAC_SERVER_STATUS  server_status;      /* Server status */
} tBSA_SAC_STATUS_IND_MSG;

/* BSA_SAC_ERROR_RESP_EVT callback event data */
typedef struct
{
    int                     dummy;              /* Dummy place holder */
} tBSA_SAC_ERROR_RESP_MSG;

/* Data for all SAC events */
typedef union
{
    tBSA_SAC_ENABLE_MSG         enable;
    tBSA_SAC_DISABLE_MSG        disable;
    tBSA_SAC_CONNECT_MSG        connect;
    tBSA_SAC_DISCONNECT_MSG     disconnect;
    tBSA_SAC_TRANS_APDU_MSG     trans_apdu;
    tBSA_SAC_GET_ATR_MSG        get_atr;
    tBSA_SAC_SET_SIM_MSG        set_sim;
    tBSA_SAC_CARD_READER_STATUS_MSG reader_status;
    tBSA_SAC_SET_PROTOCOL_MSG   set_protocol;
    tBSA_SAC_DISCONNECT_IND_MSG disconnect_ind;
    tBSA_SAC_STATUS_IND_MSG     status_ind;
    tBSA_SAC_ERROR_RESP_MSG     error_resp;
} tBSA_SAC_MSG;

/* BSA SAC callback function */
typedef void (tBSA_SAC_CBACK)(tBSA_SAC_EVT event, tBSA_SAC_MSG *p_data);


/* Data for BSA SAC Enable Command */
typedef struct
{
    tBSA_SAC_CBACK          *p_cback;           /* Callback for BSA_SAC event notification */
} tBSA_SAC_ENABLE;

/* Data for BSA SAC Disable Command */
typedef struct
{
    int                     dummy;              /* Dummy place holder */
} tBSA_SAC_DISABLE;

/* Data for BSA SAC Connect Command */
typedef struct
{
    BD_ADDR                 bd_addr;            /* Remote SAP server BD address */
    tBSA_SEC_AUTH           sec_mask;           /* Security settings for SAP client */
    UINT16                  msg_size_max;       /* Max message size for SIM APDU commands */
} tBSA_SAC_CONNECT;

/* Data for BSA SAC Disconnect Command */
typedef struct
{
    tBSA_SAC_HANDLE         handle;             /* Connection handle */
} tBSA_SAC_DISCONNECT;

/* Data for BSA SAC Transfer APDU Command */
typedef struct
{
    tBSA_SAC_HANDLE         handle;             /* Connection handle */
    UINT8*                  p_apdu;             /* Pointer to APDU payload */
    UINT16                  apdu_len;           /* Length of APDU payload */
} tBSA_SAC_TRANS_APDU;

/* Data for BSA SAC Get ATR Command */
typedef struct
{
    tBSA_SAC_HANDLE         handle;             /* Connection handle */
} tBSA_SAC_GET_ATR;

/* Data for BSA SAC Set SIM  Command */
typedef struct
{
    tBSA_SAC_HANDLE         handle;             /* Connection handle */
    tBSA_SAC_SET_SIM_TYPE   set_sim;            /* On/Off/Reset */
} tBSA_SAC_SET_SIM;

/* Data for BSA SAC Get Card Reader Status Command */
typedef struct
{
    tBSA_SAC_HANDLE         handle;             /* Connection handle */
} tBSA_SAC_GET_CARD_READER_STATUS;

/* Data for BSA SAC Set Transport Protocol Command */
typedef struct
{
    tBSA_SAC_HANDLE         handle;             /* Connection handle */
    tBSA_SAC_TRANS_PROTOCOL protocol;           /* Transport protocol */
} tBSA_SAC_SET_PROTOCOL;


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function            BSA_SacEnableInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacEnableInit(tBSA_SAC_ENABLE *p_enable);

/*******************************************************************************
**
** Function         BSA_SacEnable
**
** Description      Enable the SAP client.  This function must be
**                  called before any other functions in the SAC API are called.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacEnable(tBSA_SAC_ENABLE *p_enable);

/*******************************************************************************
**
** Function            BSA_SacDisableInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacDisableInit(tBSA_SAC_DISABLE *p_disable);

/*******************************************************************************
**
** Function            BSA_SacDisable
**
** Description         Disable the SAP client
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacDisable(tBSA_SAC_DISABLE *p_disable);

/*******************************************************************************
**
** Function            BSA_SacConnectInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacConnectInit(tBSA_SAC_CONNECT *p_connect);

/*******************************************************************************
**
** Function            BSA_SacConnect
**
** Description         Connect to a remote SAP server
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacConnect(tBSA_SAC_CONNECT *p_connect);

/*******************************************************************************
**
** Function            BSA_SacDisconnectInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacDisconnectInit(tBSA_SAC_DISCONNECT *p_disconnect);

/*******************************************************************************
**
** Function            BSA_SacDisconnect
**
** Description         Disconnect from SAP server
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacDisconnect(tBSA_SAC_DISCONNECT *p_disconnect);

/*******************************************************************************
**
** Function            BSA_SacTransferAPDUInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacTransferAPDUInit(tBSA_SAC_TRANS_APDU *p_trans_apdu);

/*******************************************************************************
**
** Function            BSA_SacTransferAPDU
**
** Description         Send an APDU to SAP server
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacTransferAPDU(tBSA_SAC_TRANS_APDU *p_trans_apdu);

/*******************************************************************************
**
** Function            BSA_SacGetATRInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacGetATRInit(tBSA_SAC_GET_ATR *p_get_atr);

/*******************************************************************************
**
** Function            BSA_SacGetATR
**
** Description         Send transfer ATR request to SAP server
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacGetATR(tBSA_SAC_GET_ATR *p_get_atr);

/*******************************************************************************
**
** Function            BSA_SacSetSimInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacSetSimInit(tBSA_SAC_SET_SIM *p_set_sim);

/*******************************************************************************
**
** Function            BSA_SacSetSim
**
** Description         Power on/off or reset SIM on SAP server
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacSetSim(tBSA_SAC_SET_SIM *p_set_sim);

/*******************************************************************************
**
** Function            BSA_SacGetCardReaderStatusInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacGetCardReaderStatusInit(tBSA_SAC_GET_CARD_READER_STATUS *p_get_status);

/*******************************************************************************
**
** Function            BSA_SacGetCardReaderStatus
**
** Description         Send transfer card reader status request to SAP server
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacGetCardReaderStatus(tBSA_SAC_GET_CARD_READER_STATUS *p_get_status);

/*******************************************************************************
**
** Function            BSA_SacSetTransportProtocolInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacSetTransportProtocolInit(tBSA_SAC_SET_PROTOCOL *p_set);

/*******************************************************************************
**
** Function            BSA_SacSetTransportProtocol
**
** Description         Set transport protocol
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_SacSetTransportProtocol(tBSA_SAC_SET_PROTOCOL *p_set);

#ifdef __cplusplus
}
#endif

#endif
