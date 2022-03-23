/*****************************************************************************
**
**  Name:           bta_pbs_api.h
**
**  Description:    This is the public interface file for the phone book access
**                  (PB) server subsystem of BTA, Widcomm's
**                  Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2003-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_PBS_API_H
#define BTA_PBS_API_H

#include "bta_api.h"
#include "btm_api.h"
#include "bta_sys.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
#define BTA_PBS_VERSION_1_1         0x0101 /* PBAP 1.1 */
#define BTA_PBS_VERSION_1_2         0x0102 /* PBAP 1.2 */

#if (defined(BTA_PBAP_1_2_SUPPORTED) && BTA_PBAP_1_2_SUPPORTED == TRUE)
#define BTA_PBS_DEFAULT_VERSION     BTA_PBS_VERSION_1_2
#else
#define BTA_PBS_DEFAULT_VERSION     BTA_PBS_VERSION_1_1
#endif

#define BTA_PBS_DEFAULT_SUPPORTED_FEATURES  0x00000003  /* Default peer supported features */

/**************************
**  Common Definitions
***************************/

/* Profile supported features */
#define BTA_PBS_SUP_FEA_DOWNLOADING                         0x00000001      /* Downloading */
#define BTA_PBS_SUP_FEA_BROWSING                            0x00000002      /* Browsing */
#define BTA_PBS_SUP_FEA_DATABASE_ID                         0x00000004      /* Database identifier */
#define BTA_PBS_SUP_FEA_FOLDER_VER_COUNTER                  0x00000008      /* Folder version counter */
#define BTA_PBS_SUP_FEA_VCARD_SELECTING                     0x00000010      /* Vcard selecting */
#define BTA_PBS_SUP_FEA_ENH_MISSED_CALLS                    0x00000020      /* Enhanced missed calls */
#define BTA_PBS_SUP_FEA_UCI_VCARD_FIELD                     0x00000040      /* UCI Vcard field */
#define BTA_PBS_SUP_FEA_UID_VCARD_FIELD                     0x00000080      /* UID Vcard field */
#define BTA_PBS_SUP_FEA_CONTACT_REF                         0x00000100      /* Contact Referencing */
#define BTA_PBS_SUP_FEA_DEF_CONTACT_IMAGE_FORMAT            0x00000200      /* Default contact image format */

typedef UINT32 tBTA_PBS_SUP_FEA_MASK;

/* Profile supported repositories */
#define BTA_PBS_REPOSIT_LOCAL      0x01    /* Local PhoneBook */
#define BTA_PBS_REPOSIT_SIM        0x02    /* SIM card PhoneBook */
#define BTA_PBS_REPOSIT_SPEED_DIAL 0x04    /* Speed Dial */
#define BTA_PBS_REPOSIT_FAVORITES  0x08    /* Favorites */

typedef UINT8 tBTA_PBS_SUP_REPOSIT_MASK;

enum
{
    BTA_PBS_VCF_FMT_21 = 0,    /* vcard format 2.1                          */
    BTA_PBS_VCF_FMT_30         /* vcard format 3.0                          */
};
typedef UINT8 tBTA_PBS_VCF_FMT;

enum
{
    BTA_PBS_ORDER_INDEX = 0,   /* Default. vCard handle */
    BTA_PBS_ORDER_ALPHA_NUM,   /* UTF8 name attribute, LastName then FirstName then MiddleName */
    BTA_PBS_ORDER_PHONETIC     /* UTF8 representation of the sound attribute */
};
typedef UINT8 tBTA_PBS_ORDER;

enum
{
    BTA_PBS_ATTR_NAME = 0,      /* name */
    BTA_PBS_ATTR_NUMBER,        /* number */
    BTA_PBS_ATTR_SOUND,         /* sound */
    BTA_PBS_ATTR_MAX
};
typedef UINT8 tBTA_PBS_ATTR;

#define BTA_PBS_FILTER_VERSION              ((UINT64)1<<0)  /* vCard Version */
#define BTA_PBS_FILTER_FN                   ((UINT64)1<<1)  /* Formatted Name */
#define BTA_PBS_FILTER_N                    ((UINT64)1<<2)  /* Structured Presentation of Name */
#define BTA_PBS_FILTER_PHOTO                ((UINT64)1<<3)  /* Associated Image or Photo */
#define BTA_PBS_FILTER_BDAY                 ((UINT64)1<<4)  /* Birthday */
#define BTA_PBS_FILTER_ADR                  ((UINT64)1<<5)  /* Delivery Address */
#define BTA_PBS_FILTER_LABEL                ((UINT64)1<<6)  /* Delivery */
#define BTA_PBS_FILTER_TEL                  ((UINT64)1<<7)  /* Telephone Number */
#define BTA_PBS_FILTER_EMAIL                ((UINT64)1<<8)  /* Electronic Mail Address */
#define BTA_PBS_FILTER_MAILER               ((UINT64)1<<9)  /* Electronic Mail */
#define BTA_PBS_FILTER_TZ                   ((UINT64)1<<10)  /* Time Zone */
#define BTA_PBS_FILTER_GEO                  ((UINT64)1<<11) /* Geographic Position */
#define BTA_PBS_FILTER_TITLE                ((UINT64)1<<12) /* Job */
#define BTA_PBS_FILTER_ROLE                 ((UINT64)1<<13) /* Role within the Organization */
#define BTA_PBS_FILTER_LOGO                 ((UINT64)1<<14) /* Organization Logo */
#define BTA_PBS_FILTER_AGENT                ((UINT64)1<<15) /* vCard of Person Representing */
#define BTA_PBS_FILTER_ORG                  ((UINT64)1<<16) /* Name of Organization */
#define BTA_PBS_FILTER_NOTE                 ((UINT64)1<<17) /* Comments */
#define BTA_PBS_FILTER_REV                  ((UINT64)1<<18) /* Revision */
#define BTA_PBS_FILTER_SOUND                ((UINT64)1<<19) /* Pronunciation of Name */
#define BTA_PBS_FILTER_URL                  ((UINT64)1<<20) /* Uniform Resource Locator */
#define BTA_PBS_FILTER_UID                  ((UINT64)1<<21) /* Unique ID */
#define BTA_PBS_FILTER_KEY                  ((UINT64)1<<22) /* Public Encryption Key */
#define BTA_PBS_FILTER_NICKNAME             ((UINT64)1<<23) /* Nickname */
#define BTA_PBS_FILTER_CATEGORIES           ((UINT64)1<<24) /* Categories */
#define BTA_PBS_FILTER_PROID                ((UINT64)1<<25) /* Product ID */
#define BTA_PBS_FILTER_CLASS                ((UINT64)1<<26) /* Class Information */
#define BTA_PBS_FILTER_SORT_STRING          ((UINT64)1<<27) /* String used for sorting operations */
#define BTA_PBS_FILTER_CALL_DATETIME        ((UINT64)1<<28) /* Time stamp */
#define BTA_PBS_FILTER_X_BT_SPEEDDIALKEY    ((UINT64)1<<29) /* Speed-dial shortcut */
#define BTA_PBS_FILTER_X_BT_UCI             ((UINT64)1<<30) /* Uniform Caller Identifier field */
#define BTA_PBS_FILTER_X_BT_UID             ((UINT64)1<<31) /* Bluetooth Contact Unique Identifier */
#define BTA_PBS_FILTER_PROPRIETARY          ((UINT64)1<<39) /* Indicates the usage of a proprietary filter */
#define BTA_PBS_FILTER_ALL      (0)

typedef UINT64 tBTA_PBS_FILTER_MASK;

/* Access response types */
enum
{
    BTA_PBS_ACCESS_TYPE_ALLOW = 0,    /* Allow the requested operation */
    BTA_PBS_ACCESS_TYPE_FORBID,       /* Disallow the requested operation */
    BTA_PBS_ACCESS_TYPE_PRECONDITION  /* Get vCard Entry operation only (object handle invalid) */
};

typedef UINT8 tBTA_PBS_ACCESS_TYPE;

#define BTA_PBS_MAX_FILE_LEN         64
#define BTA_PBS_MAX_CONTACT_NAME_LEN  128

/* Obex application params passed to open callout */
typedef struct
{
    tBTA_PBS_FILTER_MASK filter;
    tBTA_PBS_VCF_FMT format;
    UINT16 max_count;
    UINT16 start_offset;
    BOOLEAN                 reset_new_missed_call;
    tBTA_PBS_FILTER_MASK    selector;
    UINT8                   selector_op;

} tBTA_PBS_PULLPB_APP_PARAMS;

/* Obex application params passed to getvlist callout */
typedef struct
{
    tBTA_PBS_ORDER order;
    tBTA_PBS_ATTR attribute;
    UINT8 p_value[64];
    UINT16 value_len;
    UINT16 max_count;
    UINT16 start_offset;
    BOOLEAN                 reset_new_missed_call;
    tBTA_PBS_FILTER_MASK    selector;
    UINT8                   selector_op;
} tBTA_PBS_VCARDLIST_APP_PARAMS;

/* VCard Listing structuture Returned by application*/
typedef struct
{
    char    handle[BTA_PBS_MAX_FILE_LEN];          /* Contains the vcf name */
    char    name[BTA_PBS_MAX_CONTACT_NAME_LEN];    /* Contains the contacted name of the vlist */
} tBTA_PBS_VCARDLIST;

/* Access event operation types */
#define BTA_PBS_OPER_NONE               0
#define BTA_PBS_OPER_PULL_PB            1
#define BTA_PBS_OPER_SET_PB             2
#define BTA_PBS_OPER_PULL_VCARD_LIST    3
#define BTA_PBS_OPER_PULL_VCARD_ENTRY   4

typedef UINT8 tBTA_PBS_OPER;

/* Object type */
#define BTA_PBS_NONE_OBJ                0
#define BTA_PBS_PB_OBJ                  1
#define BTA_PBS_ICH_OBJ                 2
#define BTA_PBS_OCH_OBJ                 3
#define BTA_PBS_MCH_OBJ                 4
#define BTA_PBS_CCH_OBJ                 5
#if (defined(BTA_PBAP_1_2_SUPPORTED) && BTA_PBAP_1_2_SUPPORTED == TRUE)
#define BTA_PBS_SPD_OBJ                 6
#define BTA_PBS_FAV_OBJ                 7
#endif

typedef UINT8 tBTA_PBS_OBJ_TYPE;


/**************************
**  Server Definitions
***************************/
/* Extra Debug Code */
#ifndef BTA_PBS_DEBUG
#define BTA_PBS_DEBUG           FALSE
#endif

#define BTA_PBS_OK              0
#define BTA_PBS_FAIL            1
typedef UINT8 tBTA_PBS_STATUS;

/* Server callback function events */
#define BTA_PBS_ENABLE_EVT      0   /* PB transfer server is enabled. */
#define BTA_PBS_OPEN_EVT        1   /* Connection to peer is open. */
#define BTA_PBS_CLOSE_EVT       2   /* Connection to peer closed. */
#define BTA_PBS_AUTH_EVT        3   /* Request for Authentication key and realm */
#define BTA_PBS_ACCESS_EVT      4   /* Request for access to put a file */
#define BTA_PBS_OPER_CMPL_EVT   5   /* PB operation complete */

typedef UINT8 tBTA_PBS_EVT;

/* Structure associated with BTA_PBS_OPER_CMPL_EVT */
typedef struct
{
    char               *p_name;        /* file or folder name. */
    tBTA_PBS_OPER      operation;
    tBTA_PBS_STATUS    status;
} tBTA_PBS_OBJECT;

typedef struct
{
    UINT8  *p_userid;
    UINT8   userid_len;
    BOOLEAN userid_required;    /* TRUE if user ID is required in response (rechallanged)  */
} tBTA_PBS_AUTH;

typedef struct
{
    char          *p_name;      /* file or directory name with fully qualified path */
    tBTM_BD_NAME   dev_name;    /* Name of device, "" if unknown */
    tBTA_PBS_OPER  oper;        /* operation */
    BD_ADDR        bd_addr;     /* Address of device */
} tBTA_PBS_ACCESS;

typedef struct
{
    tBTM_BD_NAME   dev_name;    /* Name of device, "" if unknown */
    BD_ADDR        bd_addr;     /* Address of device */
    tBTA_PBS_SUP_FEA_MASK   peer_features;              /* peer supported features */ /* BSA_SPECIFIC */
} tBTA_PBS_OPEN;

typedef union
{
    tBTA_PBS_AUTH       auth;
    tBTA_PBS_ACCESS     access;
    tBTA_PBS_OBJECT     obj;
    tBTA_PBS_OPEN       open;
} tBTA_PBS;

/* Server callback function */
typedef void tBTA_PBS_CBACK(tBTA_PBS_EVT event, tBTA_PBS *p_data);

/**************************
**  Server Functions
***************************/

/*******************************************************************************
**
** Function         BTA_PbsEnable
**
** Description      Enable the phone book access server.  This function must be
**                  called before any other functions in the PB Server API are called.
**                  When the enable operation is complete the callback function
**                  will be called with an BTA_PBS_ENABLE_EVT event.
**                  Note: Pbs always enable (BTA_SEC_AUTHENTICATE | BTA_SEC_ENCRYPT)
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbsEnable(tBTA_SEC sec_mask, const char *p_service_name,
                                  const char *p_root_path, BOOLEAN enable_authen,
                                  UINT8 realm_len, UINT8 *p_realm,
                                  tBTA_PBS_CBACK *p_cback, UINT8 app_id, tBTA_PBS_SUP_FEA_MASK local_features,
                                  tBTA_PBS_SUP_REPOSIT_MASK local_repositories);

/*******************************************************************************
**
** Function         BTA_PbsDisable
**
** Description      Disable the Phone book access server.  If the server is currently
**                  connected to a peer device the connection will be closed.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbsDisable(void);

/*******************************************************************************
**
** Function         BTA_PbsAuthRsp
**
** Description      Respond to obex client authenticate repond by sending back password to
**                  BTA. Called in response to an BTA_PBS_AUTH_EVT event.
**                  Used when "enable_authen" is set to TRUE in BTA_PbapsEnable().
**
**                  Note: If the "userid_required" is TRUE in the BTA_PBS_AUTH_EVT
**                        event, then p_userid is required, otherwise it is optional.
**
**                  p_password  must be less than BTA_PBS_MAX_AUTH_KEY_SIZE (16 bytes)
**                  p_userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbsAuthRsp (char *p_password, char *p_userid);

/*******************************************************************************
**
** Function         BTA_PbsAccessRsp
**
** Description      Sends a reply to an access request event (BTA_PBS_ACCESS_EVT).
**                  This call MUST be made whenever the event occurs.
**
** Parameters       oper    - operation being accessed.
**                  access  - BTA_PBS_ACCESS_ALLOW or BTA_PBS_ACCESS_FORBID
**                  p_name  - path of file or directory to be accessed.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbsAccessRsp(tBTA_PBS_OPER oper, tBTA_PBS_ACCESS_TYPE access,
                                     char *p_name);

/*******************************************************************************
**
** Function         BTA_PbsClose
**
** Description      Close the current connection.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PbsClose(void);
#endif
