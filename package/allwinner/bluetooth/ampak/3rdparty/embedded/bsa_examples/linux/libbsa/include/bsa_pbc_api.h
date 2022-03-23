/*****************************************************************************
**
**  Name:           bsa_pbc_api.h
**
**  Description:    This is the public interface file for PBAP client part of
**                  the Bluetooth simplified API
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BSA_PBC_API_H
#define BSA_PBC_API_H

#include "uipc.h"

/* for tBSA_STATUS */
#include "bsa_status.h"

/*****************************************************************************
**  Constants and Type Definitions
*****************************************************************************/

#define BSA_PBC_FILENAME_MAX            255
#define BSA_PBC_SERVICE_NAME_LEN_MAX    150
#define BSA_PBC_ROOT_PATH_LEN_MAX       255
#define BSA_PBC_MAX_REALM_LEN           30
#define BSA_PBC_MAX_AUTH_KEY_SIZE       16
#define BSA_PBC_MAX_VALUE_LEN           512
#define BSA_PBC_LIST_DIR_LEN_MAX        512


#define BSA_PBC_CLOSE_CLOSED        1
#define BSA_PBC_CLOSE_CONN_LOSS     2


/* BSA PBAP Client callback events */
typedef enum
{
    BSA_PBC_OPEN_EVT,       /* Connection to peer is open. */
    BSA_PBC_CLOSE_EVT,      /* Connection to peer closed. */
    BSA_PBC_DISABLE_EVT,    /* Connection Disable */
    BSA_PBC_ABORT_EVT,      /* Connection Abort */
    BSA_PBC_AUTH_EVT,       /* Request for Authentication key and user id */
    BSA_PBC_GET_EVT,        /* Event contains a response to Get request */
    BSA_PBC_SET_EVT         /* Event contains a response to Get request */
} tBSA_PBC_EVT;

/* BSA PBC Access Response */
#define BSA_PBC_ACCESS_ALLOW    0  /* Allow access to operation */
#define BSA_PBC_ACCESS_FORBID   1  /* Deny access to operation */
typedef UINT8 tBSA_PBC_ACCESS_TYPE;

/* BSA PBC Access operation type */
typedef enum
{
    BSA_PBC_OPER_GET, /* Request is a GET file */
    BSA_PBC_OPER_CHG_DIR /* Request is a Change Folder */
} tBSA_PBC_OPER;

#define BSA_PBC_OK              0
#define BSA_PBC_FAIL            1
#define BSA_PBC_NO_PERMISSION   2
#define BSA_PBC_NOT_FOUND       3
#define BSA_PBC_FULL            4
#define BSA_PBC_BUSY            5
#define BSA_PBC_ABORTED         6

/* typedef UINT8 tBSA_STATUS; */

#define BSA_PBC_FLAG_NONE       0
#define BSA_PBC_FLAG_BACKUP     1
typedef UINT8 tBSA_PBC_FLAG;

#define BSA_PBC_FEA_DOWNLOADING                         0x00000001      /* Downloading */
#define BSA_PBC_FEA_BROWSING                            0x00000002      /* Browsing */
#define BSA_PBC_FEA_DATABASE_ID                         0x00000004      /* Database identifier */
#define BSA_PBC_FEA_FOLDER_VER_COUNTER                  0x00000008      /* Folder version counter */
#define BSA_PBC_FEA_VCARD_SELECTING                     0x00000010      /* Vcard selecting */
#define BSA_PBC_FEA_ENH_MISSED_CALLS                    0x00000020      /* Enhanced missed calls */
#define BSA_PBC_FEA_UCI_VCARD_FIELD                     0x00000040      /* UCI Vcard field */
#define BSA_PBC_FEA_UID_VCARD_FIELD                     0x00000080      /* UID Vcard field */
#define BSA_PBC_FEA_CONTACT_REF                         0x00000100      /* Contact Referencing */
#define BSA_PBC_FEA_DEF_CONTACT_IMAGE_FORMAT            0x00000200      /* Default contact image format */
typedef UINT32 tBSA_PBC_FEA_MASK;

#define BSA_PBC_FILTER_VERSION              ((UINT64)1<<0)  /* Version */
#define BSA_PBC_FILTER_FN                   ((UINT64)1<<1)  /* Formatted Name */
#define BSA_PBC_FILTER_N                    ((UINT64)1<<2)  /* Structured Presentation of Name */
#define BSA_PBC_FILTER_PHOTO                ((UINT64)1<<3)  /* Associated Image or Photo */
#define BSA_PBC_FILTER_BDAY                 ((UINT64)1<<4)  /* Birthday */
#define BSA_PBC_FILTER_ADR                  ((UINT64)1<<5)  /* Delivery Address */
#define BSA_PBC_FILTER_LABEL                ((UINT64)1<<6)  /* Delivery */
#define BSA_PBC_FILTER_TEL                  ((UINT64)1<<7)  /* Telephone Number */
#define BSA_PBC_FILTER_EMAIL                ((UINT64)1<<8)  /* Electronic Mail Address */
#define BSA_PBC_FILTER_MAILER               ((UINT64)1<<9)  /* Electronic Mail */
#define BSA_PBC_FILTER_TZ                   ((UINT64)1<<10)  /* Time Zone */
#define BSA_PBC_FILTER_GEO                  ((UINT64)1<<11) /* Geographic Position */
#define BSA_PBC_FILTER_TITLE                ((UINT64)1<<12) /* Job */
#define BSA_PBC_FILTER_ROLE                 ((UINT64)1<<13) /* Role within the Organization */
#define BSA_PBC_FILTER_LOGO                 ((UINT64)1<<14) /* Organization Logo */
#define BSA_PBC_FILTER_AGENT                ((UINT64)1<<15) /* vCard of Person Representing */
#define BSA_PBC_FILTER_ORG                  ((UINT64)1<<16) /* Name of Organization */
#define BSA_PBC_FILTER_NOTE                 ((UINT64)1<<17) /* Comments */
#define BSA_PBC_FILTER_REV                  ((UINT64)1<<18) /* Revision */
#define BSA_PBC_FILTER_SOUND                ((UINT64)1<<19) /* Pronunciation of Name */
#define BSA_PBC_FILTER_URL                  ((UINT64)1<<20) /* Uniform Resource Locator */
#define BSA_PBC_FILTER_UID                  ((UINT64)1<<21) /* Unique ID */
#define BSA_PBC_FILTER_KEY                  ((UINT64)1<<22) /* Public Encryption Key */
#define BSA_PBC_FILTER_NICKNAME             ((UINT64)1<<23) /* Nickname */
#define BSA_PBC_FILTER_CATEGORIES           ((UINT64)1<<24) /* Categories */
#define BSA_PBC_FILTER_PROID                ((UINT64)1<<25) /* Product ID */
#define BSA_PBC_FILTER_CLASS                ((UINT64)1<<26) /* Class information */
#define BSA_PBC_FILTER_SORT_STRING          ((UINT64)1<<27) /* String used for sorting operation */
#define BSA_PBC_FILTER_CALL_DATETIME        ((UINT64)1<<28) /* Time stamp */
#define BSA_PBC_FILTER_X_BT_SPEEDDIALKEY    ((UINT64)1<<29) /* Speed-dial shortcut */
#define BSA_PBC_FILTER_X_BT_UCI             ((UINT64)1<<30) /* Uniform Caller Identifier field */
#define BSA_PBC_FILTER_X_BT_UID             ((UINT64)1<<31) /* Bluetooth Contact Unique Identifier */
#define BSA_PBC_FILTER_ALL      (0)
typedef UINT64 tBSA_PBC_FILTER_MASK;

enum
{
    BSA_PBC_FORMAT_CARD_21, /* vCard format 2.1 */
    BSA_PBC_FORMAT_CARD_30, /* vCard format 3.0 */
    BSA_PBC_FORMAT_MAX
};
typedef UINT8 tBSA_PBC_FORMAT;

typedef struct
{
    UINT16          phone_book_size;
    BOOLEAN         pbs_exist;          /* phone_book_size is present in the response */
    UINT8           new_missed_calls;
    BOOLEAN         nmc_exist;          /* new_missed_calls is present in the response */
} tBSA_PBC_PB_PARAM_MSG;

typedef struct
{
    UINT8            data[BSA_PBC_LIST_DIR_LEN_MAX];
    UINT16           len;
    UINT16           num_entry; /* number of entries listed */
    BOOLEAN          final;     /* If TRUE, entry is last of the series */
    BOOLEAN          is_xml;
    tBSA_PBC_PB_PARAM_MSG param;
} tBSA_PBC_LIST_MSG;

enum
{
    BSA_PBC_ORDER_INDEXED = 0,      /* indexed */
    BSA_PBC_ORDER_ALPHANUM,         /* alphanumeric */
    BSA_PBC_ORDER_PHONETIC,         /* phonetic */
    BSA_PBC_ORDER_MAX
};
typedef UINT8 tBSA_PBC_ORDER;

enum
{
    BSA_PBC_ATTR_NAME = 0,      /* name */
    BSA_PBC_ATTR_NUMBER,        /* number */
    BSA_PBC_ATTR_SOUND,         /* sound */
    BSA_PBC_ATTR_MAX
};
typedef UINT8 tBSA_PBC_ATTR;

enum
{
    BSA_PBC_OP_OR = 0,      /* OR */
    BSA_PBC_OP_AND,        /* AND */
    BSA_PBC_OP_MAX
};
typedef UINT8 tBSA_PBC_OP;

enum
{
    BSA_PBC_GET_CARD = 0,       /* Get Card */
    BSA_PBC_GET_LIST_CARDS,     /* Get List Cards*/
    BSA_PBC_GET_PHONEBOOK       /* Get Phonebook */
    /* ADD NEW GET TYPES HERE */
};
typedef UINT8 tBSA_PBC_GET_TYPE;

enum
{
    BSA_PBC_GET_PARAM_STATUS = 0,       /* Status only*/
    BSA_PBC_GET_PARAM_LIST,             /* List message */
    BSA_PBC_GET_PARAM_PROGRESS,         /* Progress Message*/
    BSA_PBC_GET_PARAM_PHONEBOOK,        /* Phonebook param*/
    BSA_PBC_GET_PARAM_FILE_TRANSFER_STATUS  /* File Transfer Status*/
};
typedef UINT8 tBSA_PBC_GET_PARAM_TYPE;

enum
{
    BSA_PBC_SET_CHDIR = 0       /* Set ChDir */
    /* ADD NEW SET TYPES HERE */
};
typedef UINT8 tBSA_PBC_SET_TYPE;

enum
{
    BSA_PBC_SET_PARAM_CHDIR = 0       /* Set Param for ChDir */
    /* ADD NEW SET TYPES HERE */
};
typedef UINT8 tBSA_PBC_SET_PARAM_TYPE;

typedef struct
{
    UINT32 file_size;   /* Total size of file (BSA_FS_LEN_UNKNOWN if unknown) */
    UINT16 num_bytes;       /* Number of bytes read or written since last progress event */
} tBSA_PBC_PROGRESS_MSG;

/* Get operation is used for GetList, GetCard, GetPhoneBook */
typedef struct
{
/*     This defines what the data in this structure corresponds to.
    GetList, GetCard, GetPhoneBook or ChDir */
    tBSA_PBC_GET_PARAM_TYPE type;
    tBSA_STATUS             status;

    union {
        /* get operations */
        tBSA_PBC_LIST_MSG       list;
        tBSA_PBC_PROGRESS_MSG   prog;
        tBSA_PBC_PB_PARAM_MSG   pb;
    } param;

} tBSA_PBC_GET_MSG;

/* Set operation is used for ChDir */
typedef struct
{
    /* This defines what the data in this structure corresponds to ChDir */
    tBSA_PBC_SET_PARAM_TYPE type;
    tBSA_STATUS             status;
    union {
        /* Reserved. Not used. - Set operations event data*/
        int dummy;
    }param;

} tBSA_PBC_SET_MSG;

/* Request for authorization */
typedef struct
{
    UINT8   realm[BSA_PBC_MAX_REALM_LEN] ;
    UINT8   realm_len;
    UINT8   realm_charset;
    BOOLEAN userid_required;    /* If TRUE, a user ID must be sent */
} tBSA_PBC_AUTH_MSG;

/* BSA_PBC_OPEN_EVT callback event data */
typedef struct
{
    tBSA_SERVICE_ID     service;            /* Connection is open with PBAP */
    tBSA_STATUS         status;
    BOOLEAN             initiator;          /* connection initiator, local TRUE, peer FALSE */
    tUIPC_CH_ID         uipc_channel;       /* out: uipc channel */
    tBSA_PBC_FEA_MASK   peer_features;      /* Peer supported features */
} tBSA_PBC_OPEN_MSG;

/* BSA_PBC_CLOSE_EVT callback event data */
typedef struct
{
    tBSA_STATUS         status;
} tBSA_PBC_CLOSE_MSG;

/* BSA_PBC_DISABLE_EVT callback event data */
typedef struct
{
    tBSA_STATUS         status;
} tBSA_PBC_DISABLE_MSG;

/* BSA_PBC_ABORT_EVT callback event data */
typedef struct
{
    tBSA_STATUS         status;
} tBSA_PBC_ABORT_MSG;

/* Data for all PBC events */
typedef union
{
    tBSA_PBC_OPEN_MSG       open;
    tBSA_PBC_CLOSE_MSG      close;
    tBSA_PBC_DISABLE_MSG    disable;
    tBSA_PBC_ABORT_MSG      abort;
    tBSA_PBC_AUTH_MSG       auth;
    tBSA_PBC_GET_MSG        get;
    tBSA_PBC_SET_MSG        set;
} tBSA_PBC_MSG;

/* BSA PBC callback function */
typedef void (tBSA_PBC_CBACK)(tBSA_PBC_EVT event, tBSA_PBC_MSG *p_data);

/*
* Structures used to pass parameters to BSA API functions
*/

typedef struct
{
    tBSA_PBC_CBACK  *p_cback;
    tBSA_PBC_FEA_MASK local_features;
} tBSA_PBC_ENABLE;

typedef struct
{
    int dummy;
} tBSA_PBC_DISABLE;

typedef struct
{
    int dummy;
} tBSA_PBC_CLOSE;

typedef struct
{
    int dummy;
} tBSA_PBC_ABORT;

typedef struct
{
    UINT16 dummy;
} tBSA_PBC_CANCEL;


typedef struct
{
    BD_ADDR         bd_addr;
    tBSA_SEC_AUTH   sec_mask;
} tBSA_PBC_OPEN;

typedef struct
{
    char                    remote_name[BSA_PBC_MAX_REALM_LEN];
    tBSA_PBC_FILTER_MASK    filter;
    tBSA_PBC_FORMAT         format;
    UINT16                  max_list_count;
    UINT16                  list_start_offset;
    BOOLEAN                 is_reset_miss_calls;
    tBSA_PBC_FILTER_MASK    selector;
    tBSA_PBC_OP             selector_op;
}tBSA_PBC_GETPHONEBOOK;

typedef struct
{
    char                    remote_name[BSA_PBC_FILENAME_MAX];
    tBSA_PBC_FILTER_MASK    filter;
    tBSA_PBC_FORMAT         format;
}tBSA_PBC_GETCARD;

typedef struct
{
    char            dir[BSA_PBC_ROOT_PATH_LEN_MAX];
    tBSA_PBC_ORDER  order;
    char            value[BSA_PBC_MAX_VALUE_LEN];
    tBSA_PBC_ATTR   attribute;
    UINT16          max_list_count;
    UINT16          list_start_offset;
    BOOLEAN                 is_reset_miss_calls;
    tBSA_PBC_FILTER_MASK    selector;
    tBSA_PBC_OP             selector_op;
} tBSA_PBC_LISTCARDS;


/*  Get operation is used for GetList, GetCard, GetPhoneBook */
typedef struct
{
/*  This defines what the data in this structure corresponds to.
    GetList, GetCard, GetPhoneBook or ChDir*/
    tBSA_PBC_GET_TYPE type;
    BOOLEAN     is_xml;

    union {
        /* get operations */
        tBSA_PBC_LISTCARDS      list_cards;
        tBSA_PBC_GETCARD        get_card;
        tBSA_PBC_GETPHONEBOOK   get_phonebook;
    } param;

} tBSA_PBC_GET;

typedef struct
{
    char            dir[BSA_PBC_ROOT_PATH_LEN_MAX];
    tBSA_PBC_FLAG   flag;
} tBSA_PBC_CHDIR;

/* Set operation is used for ChDir */
typedef struct
{
    /* This defines what the data in this structure corresponds to. */
    tBSA_PBC_SET_TYPE type;

    union {
        /*For CHDIR*/
        tBSA_PBC_CHDIR  ch_dir;
        /* Add other set type structs here. */
    }param;
} tBSA_PBC_SET;

typedef struct
{
    char password[BSA_PBC_MAX_AUTH_KEY_SIZE];
    char userid[BSA_PBC_MAX_REALM_LEN];
} tBSA_PBC_AUTHRSP;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function            BSA_PbcEnableInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcEnableInit(tBSA_PBC_ENABLE *p_enable);

/*******************************************************************************
**
** Function            BSA_PbcEnable
**
** Description         Enable the phone book access client.  This function must be
**                     called before any other functions in the PBC API are called.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcEnable(tBSA_PBC_ENABLE *p_enable);

/*******************************************************************************
**
** Function            BSA_PbcDisableInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcDisableInit(tBSA_PBC_DISABLE *p_disable);

/*******************************************************************************
**
** Function            BSA_PbcDisable
**
** Description         Disable the phone book access client.  If the client is currently
**                     connected to a peer device the connection will be closed.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcDisable(tBSA_PBC_DISABLE *p_disable);

/*******************************************************************************
**
** Function            BSA_PbcOpenInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcOpenInit(tBSA_PBC_OPEN *p_open);

/*******************************************************************************
**
** Function            BSA_PbcOpen
**
** Description         Open a connection to an PBAP server.
**
**                     When the connection is open the callback function
**                     will be called with a BSA_PBC_OPEN_EVT.  If the connection
**                     fails or otherwise is closed the callback function will be
**                     called with a BSA_PBC_CLOSE_EVT.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcOpen(tBSA_PBC_OPEN *p_open);

/*******************************************************************************
**
** Function            BSA_PbcCloseInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcCloseInit(tBSA_PBC_CLOSE *p_close);

/*******************************************************************************
**
** Function            BSA_PbcClose
**
** Description         Close the current connection to the server. Aborts any
**                     active PBAP transfer.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcClose(tBSA_PBC_CLOSE *p_close);

/*******************************************************************************
**
** Function            BSA_PbcGetInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcGetInit(tBSA_PBC_GET *p_get);

/*******************************************************************************
**
** Function            BSA_PbcGet
**
** Description         Retrieve a PhoneBook from the peer device and copy it to the
**                     local file system.
**
**                     This function can only be used when the client is connected
**                     in PBAP mode.
**                     (Covers GetPhoneBook, GetCard, ListCards)
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcGet(tBSA_PBC_GET *p_get);

/*******************************************************************************
**
** Function            BSA_PbcSetInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcSetInit(tBSA_PBC_SET *p_set);

/*******************************************************************************
**
** Function            BSA_PbcSet
**
** Description         Phone Book Set Operation
**                     - Change PB path on the peer device.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcSet(tBSA_PBC_SET *p_set);

/*******************************************************************************
**
** Function            BSA_PbcAuthRspInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcAuthRspInit (tBSA_PBC_AUTHRSP *p_authrsp);

/*******************************************************************************
**
** Function            BSA_PbcAuthRsp
**
** Description         Sends a response to an OBEX authentication challenge to the
**                     connected OBEX server. Called in response to an BSA_PBC_AUTH_EVT
**                     event.
**
** Note:               If the "userid_required" is TRUE in the BSA_PBC_AUTH_EVT event,
**                     then userid is required, otherwise it is optional.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcAuthRsp (tBSA_PBC_AUTHRSP *p_authrsp);

/*******************************************************************************
**
** Function            BSA_PbcAbortInit
**
** Description         Initialize structure containing API parameters with default values
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcAbortInit(tBSA_PBC_ABORT *p_abort);

/*******************************************************************************
**
** Function            BSA_PbcAbort
**
** Description         Aborts any active PBC operation.
**
** Parameters          Pointer to structure containing API parameters
**
** Returns             tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcAbort(tBSA_PBC_ABORT *p_abort);

/*******************************************************************************
**
** Function         BSA_PbcCancelInit
**
** Description      Init a structure tBSA_PBC_CANCEL to be used with BSA_PbcCancel
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcCancelInit(tBSA_PBC_CANCEL *pCancel);

/*******************************************************************************
**
** Function         BSA_PbcCancel
**
** Description      Send a command to cancel connection.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcCancel(tBSA_PBC_CANCEL *pCancel);

#ifdef __cplusplus
}
#endif

#endif
