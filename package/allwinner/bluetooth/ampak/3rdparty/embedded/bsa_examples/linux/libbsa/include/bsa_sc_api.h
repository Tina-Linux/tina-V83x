/*****************************************************************************
**
**  Name:           bsa_sc_api.h
**
**  Description:    This is the implementation of the API for the SIM
**                  Card (SC) server subsystem of BSA
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BSA_SC_API_H
#define BSA_SC_API_H

#include "bsa_api.h"
#include "sap_api.h"

/* #include "uipc.h" */

/* for tBSA_STATUS */
#include "bsa_status.h"


/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* Enumeration of sap states */
enum {
     BSA_SAP_STATE_IDLE,           /* Not connected to client */
     BSA_SAP_STATE_OPEN,           /* Connected to client, but not active yet (waiting for SIM to be reset for client) */
     BSA_SAP_STATE_RESET_PENDING,  /* Reset is pending (there was a on-going call on the phone..waiting for call to be released) */
     BSA_SAP_STATE_READY           /* SIM card has been reset...client can start sending commands */
};

/* Status of SIM card (for BSA_ScCardStatus) */
#define BSA_SC_CARD_UNKNOWN_ERROR  SAP_CARD_UNKNOWN_ERROR   /* Unknown problem with SIM card. */
#define BSA_SC_CARD_RESET          SAP_CARD_RESET           /* SIM inserted and powered on prior to SIM Access Profile connection. */
#define BSA_SC_CARD_NOT_ACCESSIBLE SAP_CARD_NOT_ACCESSIBLE  /* SIM inserted, but not accessible. */
#define BSA_SC_CARD_REMOVED        SAP_CARD_REMOVED         /* SIM not inserted, or has been removed. */
#define BSA_SC_CARD_INSERTED       SAP_CARD_INSERTED        /* SIM inserted, but not powered on. Client needs to power on the SIM before using it. */
#define BSA_SC_CARD_RECOVERED      SAP_CARD_RECOVERED       /* Previously not accessible card has been made accessible again, and is powered on by server. */

#define BSA_SC_CARD_READY          BSA_SC_CARD_RECOVERED + 1       /* SIM is inserted and powered on */

typedef UINT8 tBSA_SC_CARD_STATUS;

/* Flags describing Card Reader options (for reader_flags parameter of BSA_ScEnable) */
#define BSA_SC_READER_FL_REMOVABLE  0x0001
typedef UINT16 tBSA_SC_READER_FLAGS;

/* Status of card reader (for BSA_ScReaderStatus) */
#define BSA_SC_READER_STATUS_ATTACHED   0
#define BSA_SC_READER_STATUS_REMOVED    1
typedef UINT8 tBSA_SC_READER_STATUS;

/* Disconnection type (for BSA_Disconnect) */
#define BSA_SC_DISC_GRACEFUL    SAP_DISCONNECT_GRACEFUL
#define BSA_SC_DISC_IMMEDIATE   SAP_DISCONNECT_IMMEDIATE
typedef UINT8 tBSA_SC_DISCONNECT_TYPE;

/* Results codes for use with call-in functions */
#define BSA_SC_RESULT_OK                    SAP_RESULT_OK                   /* Request processed correctly. */
#define BSA_SC_RESULT_ERROR                 SAP_RESULT_ERROR                /* Error - no reason specified. */
#define BSA_SC_RESULT_CARD_NOT_ACCESSIBLE   SAP_RESULT_CARD_NOT_ACCESSIBLE  /* Error - card inserted but not accessible. */
#define BSA_SC_RESULT_CARD_POWERED_OFF      SAP_RESULT_CARD_POWERED_OFF     /* Error - card is powered off. */
#define BSA_SC_RESULT_CARD_REMOVED          SAP_RESULT_CARD_REMOVED         /* Error - card is not inserted. */
#define BSA_SC_RESULT_CARD_ALREADY_ON       SAP_RESULT_CARD_ALREADY_ON      /* Error - card already turned on. */
#define BSA_SC_RESULT_DATA_NOT_AVAILABLE    SAP_RESULT_DATA_NOT_AVAILABLE   /* Error - data not available. */
/* Request codes for use with call-in functions */
#define BSA_SC_REQUEST_APDU                 (BSA_SC_RESULT_OK + 10)
typedef UINT8 tBSA_SC_RESULT;


/* Results codes for sim reset */
#define BSA_SC_RESET_RESULT_OK              0                               /* Reset successful */
#define BSA_SC_RESET_RESULT_ERROR           1                               /* Reset failed */
#define BSA_SC_RESET_RESULT_OK_ONGOING_CALL 2                               /* Ongoing call (will reset after call is released) */
typedef UINT8 tBSA_SC_RESET_RESULT;

/**************************
**  Structure definitions
***************************/

/*****************************************************************************
**  Constants and Type Definitions
*****************************************************************************/

#define BSA_SC_FILENAME_MAX            255
#define BSA_SC_SERVICE_NAME_LEN_MAX    150
#define BSA_SC_ROOT_PATH_LEN_MAX       255
#define BSA_SC_MAX_REALM_LEN           30
#define BSA_SC_MAX_AUTH_KEY_SIZE       16
#define BSA_SC_MAX_VALUE_LEN           512
#define BSA_SC_LIST_DIR_LEN_MAX        512

/* BSA SAP server callback events */
typedef enum
{
    BSA_SC_ENABLE_EVT,  /* SIM Access server is enabled. */
    BSA_SC_OPEN_EVT,       /* Connection to peer is open. */
    BSA_SC_CLOSE_EVT,      /* Connection to peer closed. */
    BSA_SC_DISABLE_EVT,    /* Connection Disable */
    BSA_SC_SET_EVT         /* Event contains a response to Get request */
} tBSA_SC_EVT;


enum
{
    BSA_SC_SET_CARD_STATUS = 0,       /* Notify client of change in SIM card status */
    BSA_SC_SET_READER_STATUS,         /* Notify client of change in SIM card reader status */
    /* ADD NEW SET TYPES HERE */
};
typedef UINT8 tBSA_SC_SET_TYPE;


/* Set operation is used for ChDir */
typedef struct
{
    /* This defines what the data in this structure corresponds to ChDir */
    tBSA_SC_SET_TYPE    type;
    tBSA_STATUS         status;
} tBSA_SC_SET_MSG;

/* BSA_SC_OPEN_EVT callback event data */
typedef struct
{
    BD_ADDR         bd_addr;                /* Client bd address */
    tBSA_STATUS         status;
} tBSA_SC_OPEN_MSG;

/* BSA_SC_CLOSE_EVT callback event data */
typedef struct
{
    tBSA_STATUS         status;
} tBSA_SC_CLOSE_MSG;

/* BSA_SC_DISABLE_EVT callback event data */
typedef struct
{
    tBSA_STATUS         status;
} tBSA_SC_DISABLE_MSG;

/* BSA_SC_ENABLE_EVT callback event data */
typedef struct
{
    tBSA_STATUS         status;
} tBSA_SC_ENABLE_MSG;


/* Data for all SC events */
typedef union
{
    tBSA_SC_OPEN_MSG       open;
    tBSA_SC_CLOSE_MSG      close;
    tBSA_SC_DISABLE_MSG    disable;
    tBSA_SC_ENABLE_MSG     enable;
    tBSA_SC_SET_MSG        set;
} tBSA_SC_MSG;

/* BSA SC callback function */
typedef void (tBSA_SC_CBACK)(tBSA_SC_EVT event, tBSA_SC_MSG *p_data);

/* Data for BSA SC Enable Command */
typedef struct
{
    tBSA_SEC_AUTH           sec_mask;           /* Security options */
    char                    service_name[BSA_SC_SERVICE_NAME_LEN_MAX]; /* Service name for SDP record */
    UINT8                   reader_id;          /* SIM Card Reader ID (for TRANSFER_CARD_READER_STATUS requests) */
    tBSA_SC_READER_FLAGS    reader_flags;       /* Flags describing card reader */
    UINT16                  msg_size_min;       /* Min message size for SIM APDU commands */
    UINT16                  msg_size_max;       /* Max message size for SIM APDU commands */
    tBSA_SC_CBACK           *p_cback;           /* Callback for BSA_SC event notification */
} tBSA_SC_ENABLE;


typedef struct
{
    int dummy;
} tBSA_SC_DISABLE;

typedef struct
{
    tBSA_SC_DISCONNECT_TYPE type; /*Type of disconnection desired (BSA_SC_DISC_GRACEFUL or BSA_SC_DISC_IMMEDIATE */
} tBSA_SC_CLOSE;

/* Set operation is used to set card / Reader status */
typedef struct
{
    /* This defines what the data in this structure corresponds to. */
    tBSA_SC_SET_TYPE type;
    tBSA_SC_CARD_STATUS     card_status;
    tBSA_SC_READER_STATUS   reader_status;

} tBSA_SC_SET;


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function            BSA_ScEnableInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_ScEnableInit(tBSA_SC_ENABLE *p_enable);

/*******************************************************************************
**
** Function         BSA_ScEnable
**
** Description      Enable the SIM Card server.  This function must be
**                  called before any other functions in the SC API are called.
**                  When the enable operation is complete the callback function
**                  will be called with a BSA_SC_ENABLE_EVT.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_ScEnable(tBSA_SC_ENABLE *p_enable);

/*******************************************************************************
**
** Function            BSA_ScDisableInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_ScDisableInit(tBSA_SC_DISABLE *p_disable);

/*******************************************************************************
**
** Function            BSA_ScDisable
**
** Description         Disable the SIM Card server.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_ScDisable(tBSA_SC_DISABLE *p_disable);

/*******************************************************************************
**
** Function            BSA_ScCloseInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_ScCloseInit(tBSA_SC_CLOSE *p_close);

/*******************************************************************************
**
** Function            BSA_ScClose
**
** Description         Close the current connection to the server. Aborts any
**                     active SAP transfer.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_ScClose(tBSA_SC_CLOSE *p_close);

/*******************************************************************************
**
** Function            BSA_ScSetInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_ScSetInit(tBSA_SC_SET *p_set);

/*******************************************************************************
**
** Function            BSA_ScSet
**
** Description         Phone Book Set Operation
**                     - Change PB path on the peer device.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_ScSet(tBSA_SC_SET *p_set);

#ifdef __cplusplus
}
#endif

#endif
