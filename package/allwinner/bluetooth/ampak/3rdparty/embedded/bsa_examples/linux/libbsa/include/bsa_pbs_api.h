/*****************************************************************************
 **
 **  Name:           bsa_pbs_api.h
 **
 **  Description:    This is the public interface file for PBAP server part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_PBS_API_H
#define BSA_PBS_API_H

#include "uipc.h"

/* for tBSA_STATUS */
#include "bsa_status.h"

/*****************************************************************************
 **  Constants and Type Definitions
 *****************************************************************************/
#ifndef BSA_PBS_DEBUG
#define BSA_PBS_DEBUG FALSE
#endif

#define BSA_PBS_FILENAME_MAX            128
#define BSA_PBS_SERVICE_NAME_LEN_MAX    150
#define BSA_PBS_ROOT_PATH_LEN_MAX       255
#define BSA_PBS_MAX_REALM_LEN           30
#define BSA_PBS_MAX_AUTH_KEY_SIZE       16
#define BSA_PBS_MAX_SEARCH_VALUE        64
#define BSA_PBS_UID_LEN                 32
#define BSA_PBS_MAX_HANDLE_LEN          64
#define BSA_PBS_MAX_CONTACT_NAME_LEN    128

#define BSA_UINT128_SIZE   16    /* 128 bit */
typedef UINT8 tBSA_UINT128[BSA_UINT128_SIZE];

/* Profile supported features */
#define BSA_PBS_FEA_DOWNLOADING                 0x00000001      /* Downloading */
#define BSA_PBS_FEA_BROWSING                    0x00000002      /* Browsing */
#define BSA_PBS_FEA_DATABASE_ID                 0x00000004      /* Database identifier */
#define BSA_PBS_FEA_FOLDER_VER_COUNTER          0x00000008      /* Folder version counter */
#define BSA_PBS_FEA_VCARD_SELECTING             0x00000010      /* Vcard selecting */
#define BSA_PBS_FEA_ENH_MISSED_CALLS            0x00000020      /* Enhanced missed calls */
#define BSA_PBS_FEA_UCI_VCARD_FIELD             0x00000040      /* UCI Vcard field */
#define BSA_PBS_FEA_UID_VCARD_FIELD             0x00000080      /* UID Vcard field */
#define BSA_PBS_FEA_CONTACT_REF                 0x00000100      /* Contact Referencing */
#define BSA_PBS_FEA_DEF_CONTACT_IMAGE_FORMAT    0x00000200      /* Default contact image format */

typedef UINT32 tBSA_PBS_FEA_MASK;

/* Profile supported repositories */
#define BSA_PBS_REPOSIT_LOCAL      0x01    /* Local PhoneBook */
#define BSA_PBS_REPOSIT_SIM        0x02    /* SIM card PhoneBook */
#define BSA_PBS_REPOSIT_SPEED_DIAL 0x04    /* Speed Dial */
#define BSA_PBS_REPOSIT_FAVORITES  0x08    /* Favorites */

typedef UINT8 tBSA_PBS_REPOSIT_MASK;

/* BSA PBAP Server callback events */
typedef enum
{
    BSA_PBS_OPEN_EVT, /* Connection Open*/
    BSA_PBS_CLOSE_EVT, /* Connection Closed */
    BSA_PBS_AUTH_EVT, /* Obex authentication request */
    BSA_PBS_ACCESS_EVT, /* Access to file or directory requested */
    BSA_PBS_PULL_EVT, /* Pull Phonebook/vCard Listing/vCard Entry request received */
    BSA_PBS_DATA_REQ_EVT, /* Request for Phonebook/vCard Listing/vCard Entry data */
    BSA_PBS_GET_PB_INFO_EVT, /* Get Phonebook info */
    BSA_PBS_RESET_NMC_EVT, /* Reset new missed calls */
    BSA_PBS_OPER_CMPL_EVT,/* Phonebook access operation completed */
} tBSA_PBS_EVT;

/* Property filter */
#define BSA_PBS_FILTER_VERSION              ((UINT64)1<<0)  /* vCard Version */
#define BSA_PBS_FILTER_FN                   ((UINT64)1<<1)  /* Formatted Name */
#define BSA_PBS_FILTER_N                    ((UINT64)1<<2)  /* Structured Presentation of Name */
#define BSA_PBS_FILTER_PHOTO                ((UINT64)1<<3)  /* Associated Image or Photo */
#define BSA_PBS_FILTER_BDAY                 ((UINT64)1<<4)  /* Birthday */
#define BSA_PBS_FILTER_ADR                  ((UINT64)1<<5)  /* Delivery Address */
#define BSA_PBS_FILTER_LABEL                ((UINT64)1<<6)  /* Delivery */
#define BSA_PBS_FILTER_TEL                  ((UINT64)1<<7)  /* Telephone Number */
#define BSA_PBS_FILTER_EMAIL                ((UINT64)1<<8)  /* Electronic Mail Address */
#define BSA_PBS_FILTER_MAILER               ((UINT64)1<<9)  /* Electronic Mail */
#define BSA_PBS_FILTER_TZ                   ((UINT64)1<<10) /* Time Zone */
#define BSA_PBS_FILTER_GEO                  ((UINT64)1<<11) /* Geographic Position */
#define BSA_PBS_FILTER_TITLE                ((UINT64)1<<12) /* Job */
#define BSA_PBS_FILTER_ROLE                 ((UINT64)1<<13) /* Role within the Organization */
#define BSA_PBS_FILTER_LOGO                 ((UINT64)1<<14) /* Organization Logo */
#define BSA_PBS_FILTER_AGENT                ((UINT64)1<<15) /* vCard of Person Representing */
#define BSA_PBS_FILTER_ORG                  ((UINT64)1<<16) /* Name of Organization */
#define BSA_PBS_FILTER_NOTE                 ((UINT64)1<<17) /* Comments */
#define BSA_PBS_FILTER_REV                  ((UINT64)1<<18) /* Revision */
#define BSA_PBS_FILTER_SOUND                ((UINT64)1<<19) /* Pronunciation of Name */
#define BSA_PBS_FILTER_URL                  ((UINT64)1<<20) /* Uniform Resource Locator */
#define BSA_PBS_FILTER_UID                  ((UINT64)1<<21) /* Unique ID */
#define BSA_PBS_FILTER_KEY                  ((UINT64)1<<22) /* Public Encryption Key */
#define BSA_PBS_FILTER_NICKNAME             ((UINT64)1<<23) /* Nickname */
#define BSA_PBS_FILTER_CATEGORIES           ((UINT64)1<<24) /* Categories */
#define BSA_PBS_FILTER_PROID                ((UINT64)1<<25) /* Product ID */
#define BSA_PBS_FILTER_CLASS                ((UINT64)1<<26) /* Class Information */
#define BSA_PBS_FILTER_SORT_STRING          ((UINT64)1<<27) /* String used for sorting operations */
#define BSA_PBS_FILTER_CALL_DATETIME        ((UINT64)1<<28) /* Time stamp */
#define BSA_PBS_FILTER_X_BT_SPEEDDIALKEY    ((UINT64)1<<29) /* Speed-dial shortcut */
#define BSA_PBS_FILTER_X_BT_UCI             ((UINT64)1<<30) /* Uniform Caller Identifier field */
#define BSA_PBS_FILTER_X_BT_UID             ((UINT64)1<<31) /* Bluetooth Contact Unique Identifier */
#define BSA_PBS_FILTER_PROPRIETARY          ((UINT64)1<<39) /* Indicates the usage of a proprietary filter */
#define BSA_PBS_FILTER_ALL      (0)

typedef UINT64 tBSA_PBS_FILTER_MASK;

/* vCard format */
enum
{
    BSA_PBS_VCF_FMT_21 = 0,    /* vcard format 2.1                          */
    BSA_PBS_VCF_FMT_30         /* vcard format 3.0                          */
};
typedef UINT8 tBSA_PBS_VCF_FMT;

/* vCard selector operator */
enum
{
    BSA_PBS_OP_OR = 0,      /* OR */
    BSA_PBS_OP_AND          /* AND */
};
typedef UINT8 tBSA_PBS_OP;

/* vCard listing sort order */
enum
{
    BSA_PBS_ORDER_INDEX = 0,   /* Default. vCard handle */
    BSA_PBS_ORDER_ALPHA_NUM,   /* UTF8 name attribute, LastName then FirstName then MiddleName */
    BSA_PBS_ORDER_PHONETIC     /* UTF8 representation of the sound attribute */
};
typedef UINT8 tBSA_PBS_ORDER;

/* vCard listing search attribute */
enum
{
    BSA_PBS_ATTR_NAME = 0,      /* name */
    BSA_PBS_ATTR_NUMBER,        /* number */
    BSA_PBS_ATTR_SOUND          /* sound */
};
typedef UINT8 tBSA_PBS_ATTR;

/* BSA PBS Access Response */
enum
{
    BSA_PBS_ACCESS_ALLOW = 0,    /* Allow the requested operation */
    BSA_PBS_ACCESS_FORBID,       /* Disallow the requested operation */
    BSA_PBS_ACCESS_PRECONDITION  /* Get vCard Entry operation only (object handle invalid) */
};
typedef UINT8 tBSA_PBS_ACCESS_TYPE;

/* BSA PBS operation type */
enum
{
    BSA_PBS_OPER_NONE,
    BSA_PBS_OPER_PULL_PB,           /* Pull the whole phonebook */
    BSA_PBS_OPER_SET_PB,            /* Set phonebook directory */
    BSA_PBS_OPER_PULL_VCARD_LIST,   /* Pull Vcard list */
    BSA_PBS_OPER_PULL_VCARD_ENTRY,  /* Pull Vcard entry */
};
typedef UINT8 tBSA_PBS_OPER;

/* BSA_PBS_AUTH_EVT callback event data */
typedef struct
{
    char userid[BSA_PBS_MAX_REALM_LEN];
    UINT8 userid_len;
    BOOLEAN userid_required;
} tBSA_PBS_AUTH_MSG;

/* BSA_PBS_ACCESS_EVT callback event data */
typedef struct
{
    char name[BSA_PBS_FILENAME_MAX]; /* ascii name of file or directory */
    BD_NAME dev_name; /* Name of device requesting access */
    tBSA_PBS_OPER oper; /* Operation attempting to gain access */
} tBSA_PBS_ACCESS_MSG;

/* BSA_PBS_OPER_CMPL_EVT callback event data */
typedef struct
{
    char name[BSA_PBS_FILENAME_MAX]; /* ascii name of file or directory */
    tBSA_STATUS status; /* success or failure */
} tBSA_PBS_OBJECT_MSG;

/* BSA_PBS_OPEN_EVT callback event data */
typedef struct
{
    BD_ADDR bd_addr;
    tBSA_PBS_FEA_MASK peer_features; /* peer supported features */
} tBSA_PBS_OPEN_MSG;

/* BSA_PBS_CLOSE_EVT callback event data */
typedef struct
{
    tBSA_STATUS status;
} tBSA_PBS_CLOSE_MSG;

/* BSA_PBS_PULL_EVT callback event data */
typedef struct
{
    tBSA_PBS_OPER oper; /* operation type */
    char name[BSA_PBS_FILENAME_MAX]; /* path name */
    tBSA_PBS_FILTER_MASK filter; /* property filter */
    tBSA_PBS_VCF_FMT format; /* vCard format */
    UINT16 max_count; /* max list count */
    UINT16 start_offset; /* list start offset */
    tBSA_PBS_FILTER_MASK selector; /* vCard selector */
    tBSA_PBS_OP selector_op; /* vCard selector operator */
    tBSA_PBS_ORDER order; /* vCard listing sort order */
    tBSA_PBS_ATTR search_attr; /* vCard listing search attribute */
    char search_value[BSA_PBS_MAX_SEARCH_VALUE]; /* vCard listing search value */
} tBSA_PBS_PULL_MSG;

/* BSA_PBS_DATA_REQ_EVT callback event data */
typedef struct
{
    tBSA_PBS_OPER oper; /* operation type */
    UINT16 data_size; /* requested data size (bytes for phonebook/vCard,
                         or number of entries for vCard listing) */
} tBSA_PBS_DATA_REQ_MSG;

/* BSA_PBS_GET_PB_INFO_EVT callback event data */
typedef struct
{
    tBSA_PBS_OPER oper; /* operation type */
    char name[BSA_PBS_FILENAME_MAX]; /* path name */
    BOOLEAN get_pb_size; /* if phonebook size is requested */
    tBSA_PBS_FILTER_MASK selector; /* vCard selector (when get_pb_size is TRUE) */
    tBSA_PBS_OP selector_op; /* vCard selector operator (when get_pb_size is TRUE) */
    BOOLEAN get_new_missed_call; /* if new missed calls is requested */
    BOOLEAN get_pri_ver; /* if primary folder version is requested */
    BOOLEAN get_sec_ver; /* if secondary folder version is requested */
    BOOLEAN get_db_id; /* if database identification is requested */
} tBSA_PBS_GET_PB_INFO_MSG;

/* BSA_PBS_RESET_NMC_EVT callback event data */
typedef struct
{
    int dummy;
} tBSA_PBS_RESET_NMC_MSG;

/* union of data associated with HD callback */
typedef union
{
    tBSA_PBS_OPEN_MSG open; /* BSA_PBS_OPEN_EVT */
    tBSA_PBS_CLOSE_MSG close; /* BSA_PBS_CLOSE_EVT */
    tBSA_PBS_AUTH_MSG auth; /* BSA_PBS_AUTH_EVT */
    tBSA_PBS_ACCESS_MSG access_req; /* BSA_PBS_ACCESS_EVT */
    tBSA_PBS_PULL_MSG pull; /* BSA_PBS_PULL_EVT */
    tBSA_PBS_DATA_REQ_MSG data_req; /* BSA_PBS_DATA_REQ_EVT */
    tBSA_PBS_GET_PB_INFO_MSG get_info; /* BSA_PBS_GET_PB_INFO_EVT */
    tBSA_PBS_RESET_NMC_MSG reset_nmc; /* BSA_PBS_RESET_NMC_EVT */
    tBSA_PBS_OBJECT_MSG oper_complete; /* BSA_PBS_OPER_CMPL_EVT */
} tBSA_PBS_MSG;

/* BSA PBS callback function */
typedef void ( tBSA_PBS_CBACK)(tBSA_PBS_EVT event, tBSA_PBS_MSG *p_data);

/*
 * Structures used to pass parameters to BSA API functions
 */

typedef struct
{
    tBSA_SEC_AUTH sec_mask;
    BOOLEAN enable_authen;
    char service_name[BSA_PBS_SERVICE_NAME_LEN_MAX];
    char root_path[BSA_PBS_ROOT_PATH_LEN_MAX];
    char realm[BSA_PBS_MAX_REALM_LEN];
    tBSA_PBS_FEA_MASK local_features;
    tBSA_PBS_REPOSIT_MASK local_repositories;
    tBSA_PBS_CBACK *p_cback;
} tBSA_PBS_ENABLE;

typedef struct
{
    int dummy;
} tBSA_PBS_DISABLE;

typedef struct
{
    char password[BSA_PBS_MAX_AUTH_KEY_SIZE];
    char userid[BSA_PBS_MAX_REALM_LEN];
} tBSA_PBS_AUTHRSP;

typedef struct
{
    tBSA_PBS_OPER oper;
    tBSA_PBS_ACCESS_TYPE access;
    char name[BSA_PBS_FILENAME_MAX];
} tBSA_PBS_ACCESSRSP;

typedef struct
{
    char handle[BSA_PBS_MAX_HANDLE_LEN]; /* Contains the .vcf file name */
    char name[BSA_PBS_MAX_CONTACT_NAME_LEN]; /* Contains the contact name */
} tBSA_PBS_VLIST_ENTRY;

typedef struct
{
    tBSA_PBS_OPER oper; /* operation type */
    tBSA_STATUS status; /* if get data succeeded */
    UINT8 *p_data; /* pointer to phonebook/vCard data */
    tBSA_PBS_VLIST_ENTRY *p_vlist; /* pointer to the first vCard listing entry */
    UINT16 data_size; /* bytes for phonebook/vCard, or number of entries for vCard listing */
    BOOLEAN final; /* end of data */
} tBSA_PBS_DATA_RSP;

typedef struct
{
    tBSA_PBS_OPER oper; /* operation type */
    tBSA_STATUS status; /* if get phonebook info succeeded */
    UINT16 pb_size; /* phonebook size */
    UINT16 new_missed_call; /* new missed calls */
    tBSA_UINT128 pri_ver; /* primary folder version */
    tBSA_UINT128 sec_ver; /* secondary folder version */
    tBSA_UINT128 db_id; /* database identification */
} tBSA_PBS_PB_INFO;

/*****************************************************************************
 **  External Function Declarations
 *****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function            BSA_PbsEnableInit
 **
 ** Description         Initialize structure containing API parameters with default values
 **
 ** Parameters          Pointer to structure containing API parameters
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsEnableInit(tBSA_PBS_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function            BSA_PbsEnable
 **
 ** Description         This function enables PBAP Server and registers it with
 **                     the lower layers.
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsEnable(tBSA_PBS_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function            BSA_PbsDisableInit
 **
 ** Description         Initialize structure containing API parameters with default values
 **
 ** Parameters          Pointer to structure containing API parameters
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsDisableInit(tBSA_PBS_DISABLE *p_disable);

/*******************************************************************************
 **
 ** Function            BSA_PbsDisable
 **
 ** Description         This function is called when the host is about power down.
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsDisable(tBSA_PBS_DISABLE *p_disable);

/*******************************************************************************
 **
 ** Function            BSA_PbsAuthRspInit
 **
 ** Description         Initialize structure containing API parameters with default values
 **
 ** Parameters          Pointer to structure containing API parameters
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAuthRspInit(tBSA_PBS_AUTHRSP *p_authrsp);

/*******************************************************************************
 **
 ** Function            BSA_PbsAuthRsp
 **
 ** Description         This function is called to send an OBEX authentication challenge
 **                     to a connected OBEX client.
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAuthRsp(tBSA_PBS_AUTHRSP *p_authrsp);

/*******************************************************************************
 **
 ** Function            BSA_PbsAccessRspInit
 **
 ** Description         Initialize structure containing API parameters with default values
 **
 ** Parameters          Pointer to structure containing API parameters
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAccessRspInit(tBSA_PBS_ACCESSRSP *p_accessrsp);

/*******************************************************************************
 **
 ** Function            BSA_PbsAccessRsp
 **
 ** Description         This function sends a reply to an access request event.
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAccessRsp(tBSA_PBS_ACCESSRSP *p_accessrsp);

/*******************************************************************************
 **
 ** Function           BSA_PbsDataRspInit
 **
 ** Description         Initialize structure containing API parameters with default values
 **
 ** Parameters          Pointer to structure containing API parameters
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsDataRspInit(tBSA_PBS_DATA_RSP *p_datarsp);

/*******************************************************************************
 **
 ** Function            BSA_PbsDataRsp
 **
 ** Description         This function sends phonebook/vCard/vList entry data
 **                     This is a response to BSA_PBS_DATA_REQ_EVT event.
 **                     For phonebook/vCard data, send a block of data.  For
 **                     vCard listing, send listing entries.  The size of data
 **                     requested is specified in tBSA_PBS_DATA_REQ_MSG.
 **                     Set final to TRUE if there is no more data.
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsDataRsp(tBSA_PBS_DATA_RSP *p_datarsp);

/*******************************************************************************
 **
 ** Function            BSA_PbsPbInfoInit
 **
 ** Description         Initialize structure containing API parameters with default values
 **
 ** Parameters          Pointer to structure containing API parameters
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsPbInfoInit(tBSA_PBS_PB_INFO *p_pbinfo);

/*******************************************************************************
 **
 ** Function            BSA_PbsPbInfo
 **
 ** Description         This function sends phonebook information
 **                     This is a response to BSA_PBS_GET_PB_INFO_EVT event.
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsPbInfo(tBSA_PBS_PB_INFO *p_pbinfo);

#ifdef __cplusplus
}
#endif

#endif
