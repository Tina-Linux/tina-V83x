/*****************************************************************************
**
**  Name:           bta_pbc_api.h
**
**  Description:    This is the public interface file for the Phone Book Access Client
**                  subsystem of BTA, Widcomm's
**                  Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2003-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_PBC_API_H
#define BTA_PBC_API_H

#include "bta_api.h"
#include "btm_api.h"
#include "bta_sys.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/**************************
**  Client Definitions
***************************/
/* Extra Debug Code */
#ifndef BTA_PBC_DEBUG
#define BTA_PBC_DEBUG           FALSE
#endif

#define BTA_PBC_FLAG_NONE       0
#define BTA_PBC_FLAG_BACKUP     1

typedef UINT8 tBTA_PBC_FLAG;

/* Client supported feature bits */
#define BTA_PBC_SUP_FEA_DOWNLOADING                         0x00000001      /* Downloading */
#define BTA_PBC_SUP_FEA_BROWSING                            0x00000002      /* Browsing */
#define BTA_PBC_SUP_FEA_DATABASE_ID                         0x00000004      /* Database identifier */
#define BTA_PBC_SUP_FEA_FOLDER_VER_COUNTER                  0x00000008      /* Folder version counter */
#define BTA_PBC_SUP_FEA_VCARD_SELECTING                     0x00000010      /* Vcard selecting */
#define BTA_PBC_SUP_FEA_ENH_MISSED_CALLS                    0x00000020      /* Enhanced missed calls */
#define BTA_PBC_SUP_FEA_UCI_VCARD_FIELD                     0x00000040      /* UCI Vcard field */
#define BTA_PBC_SUP_FEA_UID_VCARD_FIELD                     0x00000080      /* UID Vcard field */
#define BTA_PBC_SUP_FEA_CONTACT_REF                         0x00000100      /* Contact Referencing */
#define BTA_PBC_SUP_FEA_DEF_CONTACT_IMAGE_FORMAT            0x00000200      /* Default contact image format */

typedef UINT32 tBTA_PBC_SUP_FEA_MASK;

#define BTA_PBC_FILTER_VERSION              ((UINT64)1<<0)  /* Version */
#define BTA_PBC_FILTER_FN                   ((UINT64)1<<1)  /* Formatted Name */
#define BTA_PBC_FILTER_N                    ((UINT64)1<<2)  /* Structured Presentation of Name */
#define BTA_PBC_FILTER_PHOTO                ((UINT64)1<<3)  /* Associated Image or Photo */
#define BTA_PBC_FILTER_BDAY                 ((UINT64)1<<4)  /* Birthday */
#define BTA_PBC_FILTER_ADR                  ((UINT64)1<<5)  /* Delivery Address */
#define BTA_PBC_FILTER_LABEL                ((UINT64)1<<6)  /* Delivery */
#define BTA_PBC_FILTER_TEL                  ((UINT64)1<<7)  /* Telephone Number */
#define BTA_PBC_FILTER_EMAIL                ((UINT64)1<<8)  /* Electronic Mail Address */
#define BTA_PBC_FILTER_MAILER               ((UINT64)1<<9)  /* Electronic Mail */
#define BTA_PBC_FILTER_TZ                   ((UINT64)1<<10)  /* Time Zone */
#define BTA_PBC_FILTER_GEO                  ((UINT64)1<<11) /* Geographic Position */
#define BTA_PBC_FILTER_TITLE                ((UINT64)1<<12) /* Job */
#define BTA_PBC_FILTER_ROLE                 ((UINT64)1<<13) /* Role within the Organization */
#define BTA_PBC_FILTER_LOGO                 ((UINT64)1<<14) /* Organization Logo */
#define BTA_PBC_FILTER_AGENT                ((UINT64)1<<15) /* vCard of Person Representing */
#define BTA_PBC_FILTER_ORG                  ((UINT64)1<<16) /* Name of Organization */
#define BTA_PBC_FILTER_NOTE                 ((UINT64)1<<17) /* Comments */
#define BTA_PBC_FILTER_REV                  ((UINT64)1<<18) /* Revision */
#define BTA_PBC_FILTER_SOUND                ((UINT64)1<<19) /* Pronunciation of Name */
#define BTA_PBC_FILTER_URL                  ((UINT64)1<<20) /* Uniform Resource Locator */
#define BTA_PBC_FILTER_UID                  ((UINT64)1<<21) /* Unique ID */
#define BTA_PBC_FILTER_KEY                  ((UINT64)1<<22) /* Public Encryption Key */
#define BTA_PBC_FILTER_NICKNAME             ((UINT64)1<<23) /* Nickname */
#define BTA_PBC_FILTER_CATEGORIES           ((UINT64)1<<24) /* Categories */
#define BTA_PBC_FILTER_PROID                ((UINT64)1<<25) /* Product ID */
#define BTA_PBC_FILTER_CLASS                ((UINT64)1<<26) /* Class information */
#define BTA_PBC_FILTER_SORT_STRING          ((UINT64)1<<27) /* String used for sorting operation */
#define BTA_PBC_FILTER_CALL_DATETIME        ((UINT64)1<<28) /* Time stamp */
#define BTA_PBC_FILTER_X_BT_SPEEDDIALKEY    ((UINT64)1<<29) /* Speed-dial shortcut */
#define BTA_PBC_FILTER_X_BT_UCI             ((UINT64)1<<30) /* Uniform Caller Identifier field */
#define BTA_PBC_FILTER_X_BT_UID             ((UINT64)1<<31) /* Bluetooth Contact Unique Identifier */
#define BTA_PBC_FILTER_ALL      (0)
typedef UINT64 tBTA_PBC_FILTER_MASK;

enum
{
    BTA_PBC_FORMAT_CARD_21, /* vCard format 2.1 */
    BTA_PBC_FORMAT_CARD_30, /* vCard format 3.0 */
    BTA_PBC_FORMAT_MAX
};
typedef UINT8 tBTA_PBC_FORMAT;

enum
{
    BTA_PBC_ORDER_INDEXED = 0,  /* indexed */
    BTA_PBC_ORDER_ALPHANUM,     /* alphanumeric */
    BTA_PBC_ORDER_PHONETIC,      /* phonetic */
    BTA_PBC_ORDER_MAX
};
typedef UINT8 tBTA_PBC_ORDER;

enum
{
    BTA_PBC_ATTR_NAME = 0,      /* name */
    BTA_PBC_ATTR_NUMBER,        /* number */
    BTA_PBC_ATTR_SOUND,         /* sound */
    BTA_PBC_ATTR_MAX
};
typedef UINT8 tBTA_PBC_ATTR;

/* Client callback function events */
#define BTA_PBC_ENABLE_EVT      0   /* Phone Book Access client is enabled. */
#define BTA_PBC_OPEN_EVT        1   /* Connection to peer is open. */
#define BTA_PBC_CLOSE_EVT       2   /* Connection to peer closed. */
#define BTA_PBC_AUTH_EVT        3   /* Request for Authentication key and user id */
#define BTA_PBC_LIST_EVT        4   /* Event contains a directory entry (tBTA_PBC_LIST) */
#define BTA_PBC_PROGRESS_EVT    5   /* Number of bytes read or written so far */
#define BTA_PBC_GETFILE_EVT     6   /* Get complete */
#define BTA_PBC_CHDIR_EVT       7   /* Change Directory complete */
#define BTA_PBC_PHONEBOOK_EVT   8   /* Report the Application Parameters for BTA_PbcGetPhoneBook response */
#define BTA_PBC_DISABLE_EVT     9   /* Phone Book Access client is disabled. */

typedef UINT8 tBTA_PBC_EVT;

/* Client callback function event data */

#define BTA_PBC_OK              0
#define BTA_PBC_FAIL            1
#define BTA_PBC_NO_PERMISSION   2
#define BTA_PBC_NOT_FOUND       3
#define BTA_PBC_FULL            4
#define BTA_PBC_BUSY            5
#define BTA_PBC_ABORTED         6
#define BTA_PBC_PRECONDITION_FAILED     7

typedef UINT8 tBTA_PBC_STATUS;

typedef struct
{
    tBTA_SERVICE_ID service;    /* Connection is open with PBAP */
    tBTA_PBC_SUP_FEA_MASK   peer_features;      /* Peer supported features */ /* BSA_SPECIFIC */
} tBTA_PBC_OPEN;

typedef struct
{
    UINT16          phone_book_size;
    BOOLEAN         pbs_exist;          /* phone_book_size is present in the response */
    UINT8           new_missed_calls;
    BOOLEAN         nmc_exist;          /* new_missed_calls is present in the response */
} tBTA_PBC_PB_PARAM;

typedef struct
{
    tBTA_PBC_PB_PARAM *p_param;
    UINT8           *data;
    UINT16           len;
    BOOLEAN          final;     /* If TRUE, entry is last of the series */
    tBTA_PBC_STATUS  status;    /* Fields are valid when status is BTA_PBC_OK */
} tBTA_PBC_LIST;

typedef struct
{
    UINT32 file_size;   /* Total size of file (BTA_FS_LEN_UNKNOWN if unknown) */
    UINT16 bytes;       /* Number of bytes read or written since last progress event */
} tBTA_PBC_PROGRESS;

typedef struct
{
    UINT8  *p_realm;
    UINT8   realm_len;
    UINT8   realm_charset;
    BOOLEAN userid_required;    /* If TRUE, a user ID must be sent */
} tBTA_PBC_AUTH;


typedef union
{
    tBTA_PBC_STATUS     status;
    tBTA_PBC_OPEN       open;
    tBTA_PBC_LIST       list;
    tBTA_PBC_PROGRESS   prog;
    tBTA_PBC_AUTH       auth;
    tBTA_PBC_PB_PARAM   pb;
} tBTA_PBC;

/* Client callback function */
typedef void tBTA_PBC_CBACK(tBTA_PBC_EVT event, tBTA_PBC *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/**************************
**  Client Functions
***************************/

/*******************************************************************************
**
** Function         BTA_PbcEnable
**
** Description      Enable the phone book access client.  This function must be
**                  called before any other functions in the PBC API are called.
**                  When the enable operation is complete the callback function
**                  will be called with an BTA_PBC_ENABLE_EVT event.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbcEnable(tBTA_PBC_CBACK *p_cback, UINT8 app_id, tBTA_PBC_SUP_FEA_MASK local_features);

/*******************************************************************************
**
** Function         BTA_PbcDisable
**
** Description      Disable the phone book access client.  If the client is currently
**                  connected to a peer device the connection will be closed.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbcDisable(void);

/*******************************************************************************
**
** Function         BTA_PbcOpen
**
** Description      Open a connection to an PBAP server.
**
**                  When the connection is open the callback function
**                  will be called with a BTA_PBC_OPEN_EVT.  If the connection
**                  fails or otherwise is closed the callback function will be
**                  called with a BTA_PBC_CLOSE_EVT.
**
**                  Note: Pbc always enable (BTA_SEC_AUTHENTICATE | BTA_SEC_ENCRYPT)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbcOpen(BD_ADDR bd_addr, tBTA_SEC sec_mask);

/*******************************************************************************
**
** Function         BTA_PbcClose
**
** Description      Close the current connection to the server. Aborts any
**                  active PBAP transfer.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbcClose(void);


/*******************************************************************************
**
** Function         BTA_PbcGetPhoneBook
**
** Description      Retrieve a PhoneBook from the peer device and copy it to the
**                  local file system.
**
**                  This function can only be used when the client is connected
**                  in PBAP mode.
**
** Note:            local file name is specified with a fully qualified path.
**                  Remote file name is absolute path in UTF-8 format
**                  (telecom/pb.vcf or SIM1/telecom/pb.vcf).
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbcGetPhoneBook(char *p_local_name, char *p_remote_name,
                         tBTA_PBC_FILTER_MASK filter, tBTA_PBC_FORMAT format,
                         UINT16 max_list_count, UINT16 list_start_offset,
                         BOOLEAN is_reset_miss_calls, tBTA_PBC_FILTER_MASK selector,
                         UINT8 selector_op);

/*******************************************************************************
**
** Function         BTA_PbcGetCard
**
** Description      Retrieve a vCard from the peer device and copy it to the
**                  local file system.
**
**                  This function can only be used when the client is connected
**                  in PBAP mode.
**
** Note:            local file name is specified with a fully qualified path.
**                  Remote file name is relative path in UTF-8 format.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbcGetCard(char *p_local_name, char *p_remote_name,
                    tBTA_PBC_FILTER_MASK filter, tBTA_PBC_FORMAT format);


/*******************************************************************************
**
** Function         BTA_PbcChDir
**
** Description      Change PB path on the peer device.
**
**                  This function can only be used when the client is connected
**                  in PBAP mode.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbcChDir(char *p_dir, tBTA_PBC_FLAG flag);

/*******************************************************************************
**
** Function         BTA_PbcAuthRsp
**
** Description      Sends a response to an OBEX authentication challenge to the
**                  connected OBEX server. Called in response to an BTA_PBC_AUTH_EVT
**                  event.
**
** Note:            If the "userid_required" is TRUE in the BTA_PBC_AUTH_EVT event,
**                  then p_userid is required, otherwise it is optional.
**
**                  p_password  must be less than BTA_PBC_MAX_AUTH_KEY_SIZE (16 bytes)
**                  p_userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbcAuthRsp (char *p_password, char *p_userid);

/*******************************************************************************
**
** Function         BTA_PbcListCards
**
** Description      Retrieve a directory listing from the peer device.
**                  When the operation is complete the callback function will
**                  be called with one or more BTA_PBC_LIST_EVT events
**                  containing directory list information formatted as described
**                  in the PBAP Spec, Version 0.9, section 3.1.6.
**                  This function can only be used when the client is connected
**                  to a peer device.
**
**                  This function can only be used when the client is connected
**                  in PBAP mode.
**
** Parameters       p_dir - Name of directory to retrieve listing of.
**
** Returns          void
**
*******************************************************************************/

BTA_API extern void BTA_PbcListCards(char *p_dir, tBTA_PBC_ORDER order, char *p_value,
                      tBTA_PBC_ATTR attribute, UINT16 max_list_count,
                      UINT16 list_start_offset, BOOLEAN is_reset_miss_calls,
                      tBTA_PBC_FILTER_MASK selector, UINT8 selector_op);


/*******************************************************************************
**
** Function         BTA_PbcAbort
**
** Description      Aborts any active PBC operation.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbcAbort(void);


#ifdef __cplusplus
}
#endif

#endif /* BTA_PBC_API_H */
