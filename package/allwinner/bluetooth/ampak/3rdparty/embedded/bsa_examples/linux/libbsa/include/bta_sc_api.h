/*****************************************************************************
**
**  Name:           bta_sc_api.c
**
**  Description:    This is the implementation of the API for the SIM
**                  Card (SC) server subsystem of BTA, Widcomm's Bluetooth
**                  application layer for mobile phones.
**
**  Copyright (c) 2003-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_SC_API_H
#define BTA_SC_API_H

#include "bta_api.h"
#include "sap_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* Status of SIM card (for BTA_ScCardStatus) */
#define BTA_SC_CARD_UNKNOWN_ERROR  SAP_CARD_UNKNOWN_ERROR   /* Unknown problem with SIM card. */
#define BTA_SC_CARD_RESET          SAP_CARD_RESET           /* SIM inserted and powered on prior to SIM Access Profile connection. */
#define BTA_SC_CARD_NOT_ACCESSIBLE SAP_CARD_NOT_ACCESSIBLE	/* SIM inserted, but not accessible. */
#define BTA_SC_CARD_REMOVED        SAP_CARD_REMOVED         /* SIM not inserted, or has been removed. */
#define BTA_SC_CARD_INSERTED       SAP_CARD_INSERTED        /* SIM inserted, but not powered on. Client needs to power on the SIM before using it. */
#define BTA_SC_CARD_RECOVERED      SAP_CARD_RECOVERED       /* Previously not accessible card has been made accessible again, and is powered on by server. */
typedef UINT8 tBTA_SC_CARD_STATUS;

/* Flags describing Card Reader options (for reader_flags parameter of BTA_ScEnable) */
#define BTA_SC_READER_FL_REMOVABLE  0x0001
typedef UINT16 tBTA_SC_READER_FLAGS;

/* Status of card reader (for BTA_ScReaderStatus) */
#define BTA_SC_READER_STATUS_ATTACHED   0
#define BTA_SC_READER_STATUS_REMOVED    1
typedef UINT8 tBTA_SC_READER_STATUS;


/* Disconnection type (for BTA_Disconnect) */
#define BTA_SC_DISC_GRACEFUL    SAP_DISCONNECT_GRACEFUL
#define BTA_SC_DISC_IMMEDIATE   SAP_DISCONNECT_IMMEDIATE
typedef UINT8 tBTA_SC_DISCONNECT_TYPE;

/* Server callback function events */
#define BTA_SC_ENABLE_EVT      0   /* SIM Access server is enabled. */
#define BTA_SC_OPEN_EVT        1   /* Connection to peer is open. */
#define BTA_SC_CLOSE_EVT       2   /* Connection to peer closed. */
typedef UINT8 tBTA_SC_EVT;


/* Results codes for use with call-in functions */
#define BTA_SC_RESULT_OK                    SAP_RESULT_OK                   /* Request processed correctly. */
#define BTA_SC_RESULT_ERROR                 SAP_RESULT_ERROR                /* Error - no reason specified. */
#define BTA_SC_RESULT_CARD_NOT_ACCESSIBLE   SAP_RESULT_CARD_NOT_ACCESSIBLE  /* Error - card inserted but not accessible. */
#define BTA_SC_RESULT_CARD_ALREADY_OFF      SAP_RESULT_CARD_ALREADY_OFF     /* Error - card is already powered off. */ /* BSA_SPECIFIC */
#define BTA_SC_RESULT_CARD_REMOVED          SAP_RESULT_CARD_REMOVED         /* Error - card is not inserted. */
#define BTA_SC_RESULT_CARD_ALREADY_ON       SAP_RESULT_CARD_ALREADY_ON      /* Error - card is already powered on. */ /* BSA_SPECIFIC */
#define BTA_SC_RESULT_DATA_NOT_AVAILABLE    SAP_RESULT_DATA_NOT_AVAILABLE   /* Error - data not available. */
/* Request codes for use with call-in functions */
#define BTA_SC_REQUEST_APDU                 (BTA_SC_RESULT_OK + 10)
typedef UINT8 tBTA_SC_RESULT;


/* Results codes for sim reset */
#define BTA_SC_RESET_RESULT_OK              0                               /* Reset successful */
#define BTA_SC_RESET_RESULT_ERROR           1                               /* Reset failed */
#define BTA_SC_RESET_RESULT_OK_ONGOING_CALL 2                               /* Ongoing call (will reset after call is released) */
typedef UINT8 tBTA_SC_RESET_RESULT;

/**************************
**  Structure definitions
***************************/
/* Event data for BTA_SC_OPEN_EVT */
typedef struct
{
    BD_ADDR bd_addr;                /* Client bd address */
} tBTA_SC_OPEN;


/* Union of data types for BTA_SC callback function */
typedef union
{
    tBTA_SC_OPEN open;
} tBTA_SC;

/* Server callback function */
typedef void tBTA_SC_CBACK (tBTA_SC_EVT event, tBTA_SC *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


/*******************************************************************************
**
** Function         BTA_ScEnable
**
** Description      Enable the SIM Card server.  This function must be
**                  called before any other functions in the SC API are called.
**                  When the enable operation is complete the callback function
**                  will be called with a BTA_SC_ENABLE_EVT.
**
** Parameters
**      sec_mask        Security options
**      p_service_name  Service name for SDP record
**      reader_id       SIM Card Reader ID (for TRANSFER_CARD_READER_STATUS requests)
**      reader_flags    Flags describing card reader
**      msg_size_min    Min message size for SIM APDU commands
**      msg_size_max    Max message size for SIM APDU commands
**      p_cback         Callback for BTA_SC event notification
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_ScEnable(tBTA_SEC sec_mask, const char *p_service_name,
                  UINT8 reader_id, tBTA_SC_READER_FLAGS reader_flags,
                  UINT16 msg_size_min, UINT16 msg_size_max,
                  tBTA_SC_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_ScDisable
**
** Description      Disable the SIM Card server.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_ScDisable(void);

/*******************************************************************************
**
** Function         BTA_ScClose
**
** Description      Close client connection.
**
** Parameters
**      type        Type of disconnection desired (BTA_SC_DISC_GRACEFUL or
**                  BTA_SC_DISC_IMMEDIATE)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_ScClose(tBTA_SC_DISCONNECT_TYPE type);


/*******************************************************************************
**
** Function         BTA_ScCardStatus
**
** Description      Notify client of change in SIM card status
**
** Parameters
**      status      New status
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_ScCardStatus(tBTA_SC_CARD_STATUS status);


/*******************************************************************************
**
** Function         BTA_ScReaderStatus
**
** Description      Notify client of change in SIM card reader status
**
** Parameters
**      status      New status
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_ScReaderStatus(tBTA_SC_READER_STATUS status);

#ifdef __cplusplus
}
#endif

#endif /* BTA_SC_API_H */
