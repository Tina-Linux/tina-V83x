/*****************************************************************************
**
**  Name:           bta_sac_api.h
**
**  Description:    This is the header file for the SIM
**                  Access Client(SAC) subsystem of BTA, Broadcom's Bluetooth
**                  application layer for mobile phones.
**
**  Copyright (c) 2003-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_SAC_API_H
#define BTA_SAC_API_H

#include "bta_api.h"
#include "sap_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* BTA_SAC status codes (for notification events) */
#define BTA_SAC_STATUS_OK                   0x00    /* OK */
#define BTA_SAC_STATUS_ERROR                0x01    /* General error */
typedef UINT8 tBTA_SAC_STATUS;

/* SAP client callback events */
#define BTA_SAC_ENABLE_EVT                  0x00    /* BTA_SacEnable */
#define BTA_SAC_DISABLE_EVT                 0x01    /* BTA_SacDisable */
#define BTA_SAC_CONNECT_EVT                 0x02    /* BTA_SacConnect */
#define BTA_SAC_DISCONNECT_EVT              0x03    /* BTA_SacDisconnect */
#define BTA_SAC_TRANSFER_APDU_EVT           0x04    /* BTA_SacTransferAPDU */
#define BTA_SAC_TRANSFER_ATR_EVT            0x05    /* BTA_SACTransferATR */
#define BTA_SAC_POWER_SIM_EVT               0x06    /* BTA_SACPowerSimOnOff */
#define BTA_SAC_RESET_SIM_EVT               0x07    /* BTA_SacResetSIM */
#define BTA_SAC_TRANSFER_READER_STATUS_EVT  0x08    /* BTA_SacTransferCardReaderStatus */
#define BTA_SAC_SET_TRANS_PROTOCOL_EVT      0x09    /* BTA_SacSetTransportProtocol */
#define BTA_SAC_DISCONNECT_IND_EVT          0x0A    /* DISCONNECT_IND from server */
#define BTA_SAC_STATUS_IND_EVT              0x0B    /* STATUS_IND from server */
#define BTA_SAC_ERROR_RESP_EVT              0x0C    /* ERROER_RESP from server */
typedef UINT8 tBTA_SAC_EVT;

/* Results codes (for responses from SAP server) */
#define BTA_SAC_RESULT_OK                   SAP_RESULT_OK                   /* Request processed correctly. */
#define BTA_SAC_RESULT_ERROR                SAP_RESULT_ERROR                /* Error - no reason specified. */
#define BTA_SAC_RESULT_CARD_NOT_ACCESSIBLE  SAP_RESULT_CARD_NOT_ACCESSIBLE  /* Error - card inserted but not accessible. */
#define BTA_SAC_RESULT_CARD_ALREADY_OFF     SAP_RESULT_CARD_ALREADY_OFF     /* Error - card is already powered off. */
#define BTA_SAC_RESULT_CARD_REMOVED         SAP_RESULT_CARD_REMOVED         /* Error - card is not inserted. */
#define BTA_SAC_RESULT_CARD_ALREADY_ON      SAP_RESULT_CARD_ALREADY_ON      /* Error - card is already powered on. */
#define BTA_SAC_RESULT_DATA_NOT_AVAILABLE   SAP_RESULT_DATA_NOT_AVAILABLE   /* Error - data not available. */
#define BTA_SAC_RESULT_NOT_SUPPORTED        SAP_RESULT_NOT_SUPPORTED        /* Error - not supported */
typedef UINT8 tBTA_SAC_RESULT_CODE;

/* CONNECT_RESP status codes from SAP server (BTA_SAC_CONNECT_EVT) */
#define BTA_SAC_CONN_STATUS_OK                      SAP_CONN_OK                     /* Server can fulfill requirements and open connection. */
#define BTA_SAC_CONN_STATUS_ERR_SAP_CONN_ERR        SAP_CONN_ERR                    /* Server unable to open connection. */
#define BTA_SAC_CONN_STATUS_ERR_MAX_SIZE            SAP_CONN_MSG_SIZE_UNSUPPORTED   /* Server doesn't support max message size requested by the client.
                                                                                       Alternate message size will be sent in CONNECT_RESP. */
#define BTA_SAC_CONN_STATUS_ERR_MIN_SIZE            SAP_CONN_MSG_SIZE_TOO_SMALL     /* Message size requested by the client is too small. Connection closed */
#define BTA_SAC_CONN_STATUS_OK_ONGOING_CALL         SAP_CONN_OK_ONGOING_CALL        /* OK, ongoing call */
typedef UINT8 tBTA_SAC_CONN_STATUS;

/* DISCONNECT_IND data from SAP server (BTA_SAC_DISCONNECT_IND_EVT) */
#define BTA_SAC_DISC_TYPE_GRACEFUL                  SAP_DISCONNECT_GRACEFUL     /* Close SAP gracefully, send DISCONNECT_REQ then close RFCOMM */
#define BTA_SAC_DISC_TYPE_IMMEDIATE                 SAP_DISCONNECT_IMMEDIATE    /* Close SAP immediately, close RFCOMM directly */
typedef tSAP_DISCONNECT_TYPE tBTA_SAC_DISC_TYPE;

/* STATUS_IND Data from SAP server (BTA_SAC_STATUS_IND_EVT) */
#define BTA_SAC_STATUS_UNKNOWN_ERROR                SAP_CARD_UNKNOWN_ERROR  /* Unknown problem with SIM card. */
#define BTA_SAC_STATUS_CARD_RESET                   SAP_CARD_RESET          /* SIM inserted and powered on prior to SIM Access Profile connection. */
#define BTA_SAC_STATUS_CARD_NOT_ACCESSIBLE          SAP_CARD_NOT_ACCESSIBLE /* SIM inserted, but not accessible. */
#define BTA_SAC_STATUS_CARD_REMOVED                 SAP_CARD_REMOVED        /* SIM not inserted, or has been removed. */
#define BTA_SAC_STATUS_CARD_INSERTED                SAP_CARD_INSERTED       /* SIM inserted, but not powered on. Client needs to power on the SIM before using it. */
#define BTA_SAC_STATUS_CARD_RECOVERED               SAP_CARD_RECOVERED      /* Previously not accessible card has been made accessible again, and is powered on by server. */
typedef tSAP_CARD_STATUS tBTA_SAC_STATUS_CHANGE;

/* TRANSPORT_PROTOCOL Data from SAP server (BTA_SAC_SET_TRANS_PROTOCOL_EVT) */
#define BTA_SAC_SAP_TRANS_PROCOTOL_T_0  SAP_TRANS_PROCOTOL_T_0  /* Transport protocol T=0 */
#define BTA_SAC_SAP_TRANS_PROCOTOL_T_1  SAP_TRANS_PROCOTOL_T_1  /* Transport protocol T=1 */
typedef tSAP_TRANS_PROCOTOL tBTA_SAC_SAP_TRANS_PROCOTOL;

/* Reasons for fail to open connection (BTA_SAC_CONNECT_EVT) */
#define BTA_SAC_FAIL_REASON_SDP             0x0     /* Connection fails to open due to fail to start SDP */
#define BTA_SAC_FAIL_REASON_NO_RESOURCE     0x1     /* Connection fails to open due to no resource, already an active connection */
#define BTA_SAC_FAIL_REASON_SDP_NOT_FOUND   0x2     /* Connection fails to open due to fail to found matched SDP record */
#define BTA_SAC_FAIL_REASON_RFCOMM          0x3     /* Connection fails to open due to fail to open RFCOMM */
#define BTA_SAC_FAIL_REASON_NO_ACTIVE_CONN  0x4     /* Fail to disconnect due to no active SAP connection */

#define BTA_SAC_FAIL_REASON_IGNORE          0xFF    /* There is no failure, ignored */
typedef UINT8 tBTA_SAC_FAIL_REASON;

#define BTA_SAC_INVALID_HANDLE              0xFFFF  /* Invalid connection handle */

/**************************
**  Structure definitions
***************************/
/* CONNECT_RESP data from SAP server (BTA_SAC_CONNECT_EVT) */
typedef struct
{
    tBTA_SAC_CONN_STATUS    conn_status;
    tBTA_SAC_FAIL_REASON    fail_reason;
    UINT16                  max_msg_size;
    BD_ADDR                 bd_addr;
} tBTA_SAC_CONNECT;

/* DISCCONNECT_RESP data from SAP server (BTA_SAC_DISCONNECT_EVT) */
typedef struct
{
    tBTA_SAC_FAIL_REASON    fail_reason;
} tBTA_SAC_DISCONNECT;

/* DISCONN_IND data from SAP server (BTA_SAC_DISCONNECT_IND_EVT) */
typedef struct
{
    tBTA_SAC_DISC_TYPE      disc_type;
} tBTA_SAC_DISCONN_IND;

/* TRANSFER_APDU_RESP data from SAP server (BTA_SAC_TRANSFER_APDU_EVT) */
typedef struct
{
    tBTA_SAC_RESULT_CODE    result_code;
    void                    *p_apdu_resp;
    UINT16                  apdu_len;
} tBTA_SAC_TRANSFER_APDU;

/* TRANSFER_ATR_RESP data from SAP server (BTA_SAC_TRANSFER_ATR_EVT) */
typedef struct
{
    tBTA_SAC_RESULT_CODE    result_code;
    void                    *p_atr_resp;
    UINT16                  atr_len;
} tBTA_SAC_TRANSFER_ATR;

/* POWER_SIM_RESP data from SAP server (BTA_SAC_POWER_SIM_EVT) */
typedef struct
{
    BOOLEAN                 is_on;
    tBTA_SAC_RESULT_CODE    result_code;
} tBTA_SAC_POWER_SIM;

/* RESET_SIM_RESP data from SAP server (BTA_SAC_RESET_SIM_EVT) */
typedef struct
{
    tBTA_SAC_RESULT_CODE    result_code;
} tBTA_SAC_RESET_SIM;

/* STATUS_IND data from SAP server (BTA_SAC_STATUS_IND_EVT) */
typedef struct
{
    tBTA_SAC_STATUS_CHANGE    status;
} tBTA_SAC_STATUS_IND;

/* TRANSFER_CARD_READER_STATUS_RESP data from SAP server
   (BTA_SAC_TRANSFER_READER_STATUS_EVT) */
typedef struct
{
    tBTA_SAC_RESULT_CODE        result_code;
    UINT8                       reader_status;
} tBTA_SAC_TRANSFERP_CARD_READER_STATUS;

/* SET_TRANSPORT_PROTOCOL_RESP data from SAP server
   (BTA_SAC_SET_TRANS_PROTOCOL_EVT) */
typedef struct
{
    tBTA_SAC_RESULT_CODE        result_code;
} tBTA_SAC_SET_TRANS_PROTOCOL;

/* Response data from SAP server */
typedef union
{
    tBTA_SAC_CONNECT                        connect;            /* BTA_SAC_CONNECT_EVT                      */
    tBTA_SAC_DISCONNECT                     disconn;            /* BTA_SAC_DISCONNECT_EVT                   */
    tBTA_SAC_DISCONN_IND                    disconn_ind;        /* BTA_SAC_DISCONNECT_IND_EVT               */
    tBTA_SAC_TRANSFER_APDU                  apdu_resp;          /* BTA_SAC_TRANSFER_APDU_EVT                */
    tBTA_SAC_TRANSFER_ATR                   atr_resp;           /* BTA_SAC_TRANSFER_ATR_EVT                 */
    tBTA_SAC_POWER_SIM                      power_sim_resp;     /* BTA_SAC_POWER_SIM_EVT                    */
    tBTA_SAC_RESET_SIM                      reset_sim_resp;     /* BTA_SAC_RESET_SIM_EVT                    */
    tBTA_SAC_STATUS_IND                     status_ind;         /* BTA_SAC_STATUS_IND_EVT                   */
    tBTA_SAC_TRANSFERP_CARD_READER_STATUS   reader_status_resp; /* BTA_SAC_TRANSFER_READER_STATUS_EVT       */
    tBTA_SAC_SET_TRANS_PROTOCOL             set_protocol_resp;  /* BTA_SAC_SET_TRANS_PROTOCOL_EVT        */
} tBTA_SAC_RSP;

/* Event data for tBTA_SAC_CBACK server callback */
typedef struct
{
    tBTA_SAC_STATUS         status;             /* status of the callback event */
    UINT16                  handle;             /* Handle of connection (not used for event = BTA_SAC_ENABLE_EVT / BTA_SAC_DISABLE_EVT) */
    tBTA_SAC_RSP            rsp;                /* Response data from SAP server */
} tBTA_SAC;

/* Server callback function */
typedef void tBTA_SAC_CBACK (tBTA_SAC_EVT event, tBTA_SAC *p_data);
/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_SacEnable
**
** Description      Enable the SIM Card client.  This function must be
**                  called before any other API functions.
**
**                  When the enable operation is complete the callback function
**                  will be called with a BTA_SAC_ENABLE_EVT.
**
** Parameters
**                  p_cback         Callback for BTA_SC event notification
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SacEnable(tBTA_SAC_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_SacDisable
**
** Description      Disable the SIM Card client;
**                  If there is an existing SAP connection, it closes the SAP
**                  connection first.
**
** Parameters       void
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SacDisable(void);

/*******************************************************************************
**
** Function         BTA_SacConnect
**
** Description      Initiate connection to SAP server on requested BD_ADDR
**                  Open a RFCOMM first and send CONNECT_REQ
**
**                  Status of operation is reported by BTA_SAC_CONNECT_EVT.
**                  If successful, a handle will be provided for use with
**                  subsequent operations to this server.
**
** Parameters
**                  bd_addr         Address of SAP server to connect to.
**                  sec_mask        Security options. Recommended:
**                                  (BTA_SEC_AUTHENTICATE | BTA_SEC_ENCRYPT)
**                  msg_size_max    Max message size for commands
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SacConnect(BD_ADDR bd_addr, tBTA_SEC sec_mask,
                                   UINT16 msg_size_max);

/*******************************************************************************
**
** Function         BTA_SacDisconnect
**
** Description      This function is called to close a SAP connection.
**
**                  Note: If client initiates SAP disconnection, SAP will be closed
**                  gracefully. If server sends client a disconnection indication
**                  with disconnection type immediately, the SAP will be closed
**                  immediately, otherwise it will be closed gradefully.
**
** Parameters
**                  handle      Identifier of connection
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SacDisconnect(UINT16 handle);

/*******************************************************************************
**
** Function         BTA_SacTransferAPDU
**
** Description      Send TRANSFER_APDU_REQ to SAP server
**
**
** Parameters
**                  handle          Identifier of connection
**                  p_apdu          Pointer to APDU payload
**                  apdu_len        Length of APDU payload
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SacTransferAPDU(UINT16 handle, UINT8 *p_apdu,
                                        UINT16 apdu_len);

/*******************************************************************************
**
** Function         BTA_SacTransferATR
**
** Description      Send TRANSFER_ATR_REQ to SAP server
**
**
** Parameters
**                  handle      Identifier of connection
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SacTransferATR(UINT16 handle);

/*******************************************************************************
**
** Function         BTA_SacPowerSimOnOff
**
** Description      Send POWER_SIM_ON_REQ / POWER_SIM_OFF_REQ to SAP server
**
**
** Parameters
**                  handle      Identifier of connection
**                  is_on       TRUE for SIM_ON_REQ, FALSE for SIM_OFF_REQ
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SacPowerSimOnOff(UINT16 handle, BOOLEAN is_on);

/*******************************************************************************
**
** Function         BTA_SacResetSIM
**
** Description      Send RESET_SIM_REQ to SAP server
**
**
** Parameters
**                  handle      Identifier of connection
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SacResetSIM(UINT16 handle);

/*******************************************************************************
**
** Function         BTA_SacTransferCardReaderStatus
**
** Description      Send TRANSFER_CARD_READER_STATUS_REQ to SAP server
**
**
** Parameters
**      handle      Identifier of connection
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SacTransferCardReaderStatus(UINT16 handle);

/*******************************************************************************
**
** Function         BTA_SacSetTransportProtocol
**
** Description      Send SET_TRANSPORT_PROTOCOL_REQ to SAP server
**
**
** Parameters
**                  handle              Identifier of connection
**                  protocol            Transport protocol
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SacSetTransportProtocol(UINT16 handle,
                                                tBTA_SAC_SAP_TRANS_PROCOTOL protocol);

#ifdef __cplusplus
}
#endif

#endif /* BTA_SAC_API_H */
