/*****************************************************************************
**
**  Name:           bta_ss_api.h
**
**  Description:    This is the public interface file for the synchronization
**                  server (SS) subsystem of BTA, Widcomm's
**                  Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_SS_API_H
#define BTA_SS_API_H

#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* Extra Debug Code */
#ifndef BTA_SS_DEBUG
#define BTA_SS_DEBUG           FALSE
#endif

#define BTA_SS_MAX_TIMESTAMP_SIZE   17 /* YYYYMMDDTHHMMSSZ including NULL */
#define BTA_SS_EMPTY_TIMESTAMP      "19000101T000000Z"

typedef char *tBTA_SS_LUID;


/* Server callback function events */
#define BTA_SS_ENABLE_EVT      0   /* Synchronization server is enabled. */
#define BTA_SS_OPEN_EVT        1   /* Connection to peer is open. */
#define BTA_SS_CLOSE_EVT       2   /* Connection to peer closed. */
#define BTA_SS_AUTH_EVT        3   /* Request for Authentication key and realm */
#define BTA_SS_SYNC_CMD_EVT    4   /* Sync Command is sent to Sync client */

typedef UINT8 tBTA_SS_EVT;

#define BTA_SS_MAX_DATA_TYPES   5

/* Data types supported - used by API as bit mask */
#define BTA_SS_API_VCARD        0x01
#define BTA_SS_API_VCAL         0x02
#define BTA_SS_API_VNOTE        0x04
#define BTA_SS_API_VMSG         0x08
#define BTA_SS_API_VMSG_O       0x10
#define BTA_SS_API_VMSG_S       0x20
#define BTA_SS_API_RTC          0x40

typedef UINT8 tBTA_SS_DATA_TYPE_MASK;


/* Data types supported - used for call-in call-out */
enum
{
    BTA_SS_DT_NONE,
    BTA_SS_DT_VCARD,
    BTA_SS_DT_VCAL,
    BTA_SS_DT_VNOTE,
    BTA_SS_DT_VMSG_IN,
    BTA_SS_DT_VMSG_OUT,
    BTA_SS_DT_VMSG_SNT
};
#define BTA_SS_DT_MAX   BTA_SS_DT_VMSG_SNT

typedef UINT8 tBTA_SS_DATA_TYPE;


/**************************
**  Common Definitions
***************************/
/* Configuration structure */
typedef struct
{
    UINT8                   realm_charset;  /* charset for authentication realm */
    BOOLEAN                 userid_req;     /* TRUE if user id is required during obex authentication (Server only) */
    UINT16                  name_size;      /* hold the result of name, when parsing name header */

} tBTA_SS_CFG;

enum
{
    BTA_SS_SVR_CHL,
    BTA_SS_CMD_CHL
};
typedef UINT16   tBTA_SS_CHL;

/* Sync server ctatus code, currently used onlyy for sync command request */
enum
{
    BTA_SS_OK,
    BTA_SS_ERROR,
    BTA_SS_NO_SERVICE,
    BTA_SS_NO_RESOURCE,
    BTA_SS_SDP_FAIL
};
typedef UINT8 tBTA_SS_STATUS;

typedef struct
{
    tBTA_SS_CHL     channel;            /* in which channel(sync command client/sync server) authentication is reqested */
    UINT8           *p_userid;
    UINT8           userid_len;
    BOOLEAN         userid_required;    /* TRUE if user ID is required in response (rechallanged)  */
} tBTA_SS_AUTH;

/* sync server callback data associated with corresponding event */
typedef union
{
    tBTA_SS_AUTH        auth;             /* BTA_SS_AUTH_EVT */
    tBTA_SS_STATUS      status;           /* BTA_SS_SYNC_CMD_EVT status code */
} tBTA_SS;

/**************************
**  Server Definitions
***************************/

/* Server callback function */
typedef void tBTA_SS_CBACK (tBTA_SS_EVT event, tBTA_SS *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_SsEnable
**
** Description      Enable the synchronization server.  This function must be
**                  called before any other functions in the SS API are called.
**                  When the enable operation is complete the callback function
**                  will be called with an BTA_SS_ENABLE_EVT event.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsEnable(tBTA_SEC sec_mask,
                                 const char *p_service_name,
                                 const char *p_temp_name,
                                 tBTA_SS_DATA_TYPE_MASK type_mask,
                                 BOOLEAN enable_authen, UINT8 realm_len,
                                 UINT8 *p_realm, tBTA_SS_CBACK *p_cback,
                                 UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_SsDisable
**
** Description      Disable the synchronization server.  If the server is currently
**                  connected to a peer device the connection will be closed.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsDisable(void);

/*******************************************************************************
**
** Function         BTA_SsPassword
**
** Description      Verify the authentication response from the connected client.
**                  Send connect response.
**                  Called in response to an BTA_SS_AUTH_EVT event.
**                  Used when "enable_authen" is set to TRUE in BTA_SsEnable().
**
**                  Note: If the "userid_required" is TRUE in the BTA_SS_AUTH_EVT
**                        event, then p_userid is required, otherwise it is optional.
**
**                  p_password  must be less than BTA_SS_MAX_AUTH_KEY_SIZE (16 bytes)
**                  p_userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsPassword (char *p_password, char *p_userid, tBTA_SS_CHL chnl);

/*******************************************************************************
**
** Function         BTA_SsSyncCmd
**
** Description      Request the start of Sync Command.
**
**                  type_mask:  type of data want to synchronized with client.
**                  peer_bda:   BDA of target Sync client.
**                  sec_mask:   security level want to put on OBX channel where
**                              sync command to be sent.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsSyncCmd (BD_ADDR peer_bda, tBTA_SS_DATA_TYPE_MASK type_mask, tBTA_SEC  sec_mask);

#ifdef __cplusplus
}
#endif

#endif /* BTA_SS_API_H */
