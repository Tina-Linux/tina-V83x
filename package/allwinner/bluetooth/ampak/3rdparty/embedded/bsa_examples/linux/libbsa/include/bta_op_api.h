/*****************************************************************************
**
**  Name:           bta_op_api.h
**
**  Description:    This is the public interface file for the object push
**                  (OP) client and server subsystem of BTA, Widcomm's
**                  Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2003-2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_OP_API_H
#define BTA_OP_API_H

#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* Extra Debug Code */
#ifndef BTA_OPS_DEBUG
#define BTA_OPS_DEBUG           FALSE
#endif

#ifndef BTA_OPC_DEBUG
#define BTA_OPC_DEBUG           FALSE
#endif


/* Object format */
#define BTA_OP_VCARD21_FMT          1       /* vCard 2.1 */
#define BTA_OP_VCARD30_FMT          2       /* vCard 3.0 */
#define BTA_OP_VCAL_FMT             3       /* vCal 1.0 */
#define BTA_OP_ICAL_FMT             4       /* iCal 2.0 */
#define BTA_OP_VNOTE_FMT            5       /* vNote */
#define BTA_OP_VMSG_FMT             6       /* vMessage */
#define BTA_OP_OTHER_FMT            0xFF    /* other format */

typedef UINT8 tBTA_OP_FMT;

/* Object format mask */
#define BTA_OP_VCARD21_MASK         0x01    /* vCard 2.1 */
#define BTA_OP_VCARD30_MASK         0x02    /* vCard 3.0 */
#define BTA_OP_VCAL_MASK            0x04    /* vCal 1.0 */
#define BTA_OP_ICAL_MASK            0x08    /* iCal 2.0 */
#define BTA_OP_VNOTE_MASK           0x10    /* vNote */
#define BTA_OP_VMSG_MASK            0x20    /* vMessage */
#define BTA_OP_ANY_MASK             0x40    /* Any type of object. */

typedef UINT8 tBTA_OP_FMT_MASK;

/* Status */
#define BTA_OP_OK                   0       /* Operation successful. */
#define BTA_OP_FAIL                 1       /* Operation failed. */
#define BTA_OP_MEM                  2       /* Not enough memory to complete operation. */

typedef UINT8 tBTA_OP_STATUS;

/* vCal Object Type */
#define BTA_OP_VCAL_EVENT           0       /* Object is an "Event" object. */
#define BTA_OP_VCAL_TODO            1       /* Object is a "ToDo" object. */

typedef UINT8 tBTA_OP_VCAL;

/* vCard Property Names */
#define BTA_OP_VCARD_ADR            1       /* Address. */
#define BTA_OP_VCARD_EMAIL          2       /* Email address. */
#define BTA_OP_VCARD_FN             3       /* Formatted name. */
#define BTA_OP_VCARD_NOTE           4       /* Note. */
#define BTA_OP_VCARD_NICKNAME       5       /* Nickname. */
#define BTA_OP_VCARD_N              6       /* Name. */
#define BTA_OP_VCARD_ORG            7       /* Organization. */
#define BTA_OP_VCARD_TEL            8       /* Telephone number. */
#define BTA_OP_VCARD_TITLE          9       /* Title. */
#define BTA_OP_VCARD_URL            10      /* URL. */
#define BTA_OP_VCARD_UID            11      /* Globally Unique Identifier. */
#define BTA_OP_VCARD_BDAY           12      /* Birthday. */
#define BTA_OP_VCARD_PHOTO          13      /* Photo. */
#define BTA_OP_VCARD_SOUND          14      /* Sound. */
#define BTA_OP_VCARD_CALL           15      /* Call date-time */
#define BTA_OP_VCARD_CATEGORIES     16      /* Categories */
#define BTA_OP_VCARD_PROID          17      /* Product ID */
#define BTA_OP_VCARD_CLASS          18      /* Class information */
#define BTA_OP_VCARD_SORT_STRING    19      /* String used for sorting operation */
#define BTA_OP_VCARD_SPD            20      /* Speed-dial shortcut */
#define BTA_OP_VCARD_UCI            21      /* Uniform Caller Identifier filed */
#define BTA_OP_VCARD_BT_UID         22      /* Bluetooth Contact Unique Identifier */
#define BTA_OP_VCARD_VERSION        23      /* vCard Version */

/* vCal Property Names */
#define BTA_OP_VCAL_CATEGORIES      1       /* Categories of event. */
#define BTA_OP_VCAL_COMPLETED       2       /* Time event is completed. */
#define BTA_OP_VCAL_DESCRIPTION     3       /* Description of event. */
#define BTA_OP_VCAL_DTEND           4       /* End date and time of event. */
#define BTA_OP_VCAL_DTSTART         5       /* Start date and time of event. */
#define BTA_OP_VCAL_DUE             6       /* Due date and time of event. */
#define BTA_OP_VCAL_LOCATION        7       /* Location of event. */
#define BTA_OP_VCAL_PRIORITY        8       /* Priority of event. */
#define BTA_OP_VCAL_STATUS          9       /* Status of event. */
#define BTA_OP_VCAL_SUMMARY         10      /* Summary of event. */
#define BTA_OP_VCAL_LUID            11      /* Locally Unique Identifier. */

/* vNote Property Names */
#define BTA_OP_VNOTE_BODY           1       /* Message body text. */
#define BTA_OP_VNOTE_LUID           2       /* Locally Unique Identifier. */

/* Structure of the 32-bit parameters mask:
**
**                  + property-specific
** +reserved        |        + character set
** |                |        |     + encoding
** |                |        |     |
** 0000000000000000 00000000 00000 000
*/

/* Encoding Parameter */
#define BTA_OP_ENC_QUOTED_PRINTABLE (1<<0)  /* Quoted-Printable encoding. */
#define BTA_OP_ENC_8BIT             (2<<0)  /* 8-bit encoding */
#define BTA_OP_ENC_BASE64           (3<<0)  /* Base64 encoding */
#define BTA_OP_ENC_BINARY           (4<<0)  /* Binary encoding */

/* Character Set Parameter */
#define BTA_OP_CHAR_BIG5            (1<<3)  /* Big5 character set. */
#define BTA_OP_CHAR_EUC_JP          (2<<3)  /* EUC-JP character set. */
#define BTA_OP_CHAR_EUC_KR          (3<<3)  /* EUC-KR character set. */
#define BTA_OP_CHAR_GB2312          (4<<3)  /* GB2312 character set. */
#define BTA_OP_CHAR_ISO_2022_JP     (5<<3)  /* ISO-2022-JP character set. */
#define BTA_OP_CHAR_ISO_8859_1      (6<<3)  /* ISO-8859-1 character set. */
#define BTA_OP_CHAR_ISO_8859_2      (7<<3)  /* ISO-8859-2 character set. */
#define BTA_OP_CHAR_ISO_8859_3      (8<<3)  /* ISO-8859-3 character set. */
#define BTA_OP_CHAR_ISO_8859_4      (9<<3)  /* ISO-8859-4 character set. */
#define BTA_OP_CHAR_ISO_8859_5      (10<<3) /* ISO-8859-5 character set. */
#define BTA_OP_CHAR_ISO_8859_6      (11<<3) /* ISO-8859-6 character set. */
#define BTA_OP_CHAR_ISO_8859_7      (12<<3) /* ISO-8859-7 character set. */
#define BTA_OP_CHAR_ISO_8859_8      (13<<3) /* ISO-8859-8 character set. */
#define BTA_OP_CHAR_KOI8_R          (14<<3) /* KOI8-R character set. */
#define BTA_OP_CHAR_SHIFT_JIS       (15<<3) /* Shift_JIS character set. */
#define BTA_OP_CHAR_UTF_8           (16<<3) /* UTF-8 character set. */

/* Address Type Parameter */
#define BTA_OP_ADR_DOM              (1<<8)  /* Domestic address. */
#define BTA_OP_ADR_INTL             (1<<9)  /* International address. */
#define BTA_OP_ADR_POSTAL           (1<<10) /* Postal address. */
#define BTA_OP_ADR_PARCEL           (1<<11) /* Parcel post address. */
#define BTA_OP_ADR_HOME             (1<<12) /* Home address. */
#define BTA_OP_ADR_WORK             (1<<13) /* Work address. */

/* EMAIL Type Parameter */
#define BTA_OP_EMAIL_PREF           (1<<8)  /* Preferred email. */
#define BTA_OP_EMAIL_INTERNET       (1<<9)  /* Internet email. */
#define BTA_OP_EMAIL_X400           (1<<10) /* x400 emaill */

/* Telephone Number Type Parameter */
#define BTA_OP_TEL_PREF             (1<<8)  /* Preferred number. */
#define BTA_OP_TEL_WORK             (1<<9)  /* Work number. */
#define BTA_OP_TEL_HOME             (1<<10) /* Home number. */
#define BTA_OP_TEL_VOICE            (1<<11) /* Voice number. */
#define BTA_OP_TEL_FAX              (1<<12) /* Fax number. */
#define BTA_OP_TEL_MSG              (1<<13) /* Message number. */
#define BTA_OP_TEL_CELL             (1<<14) /* Cell phone number. */
#define BTA_OP_TEL_PAGER            (1<<15) /* Pager number. */

/* Photo Parameter */
#define BTA_OP_PHOTO_VALUE_URI      (1<<8)  /* URI value */
#define BTA_OP_PHOTO_VALUE_URL      (1<<9)  /* URL value */
#define BTA_OP_PHOTO_TYPE_JPEG      (1<<10) /* JPEG photo */
#define BTA_OP_PHOTO_TYPE_GIF       (1<<11) /* GIF photo */

/* Sound Parameter */
#define BTA_OP_SOUND_VALUE_URI      (1<<8)  /* URI value */
#define BTA_OP_SOUND_VALUE_URL      (1<<9)  /* URL value */
#define BTA_OP_SOUND_TYPE_BASIC     (1<<10) /* BASIC sound */
#define BTA_OP_SOUND_TYPE_WAVE      (1<<11) /* WAVE sound */

/* vCard filter mask */
#define BTA_OP_FILTER_VERSION  (1<<0)  /* vCard Version */
#define BTA_OP_FILTER_FN       (1<<1)  /* Formatted Name */
#define BTA_OP_FILTER_N        (1<<2)  /* Structured Presentation of Name */
#define BTA_OP_FILTER_PHOTO    (1<<3)  /* Associated Image or Photo */
#define BTA_OP_FILTER_BDAY     (1<<4)  /* Birthday */
#define BTA_OP_FILTER_ADR      (1<<5)  /* Delivery Address */
#define BTA_OP_FILTER_LABEL    (1<<6)  /* Delivery */
#define BTA_OP_FILTER_TEL      (1<<7)  /* Telephone Number */
#define BTA_OP_FILTER_EMAIL    (1<<8)  /* Electronic Mail Address */
#define BTA_OP_FILTER_MAILER   (1<<9)  /* Electronic Mail */
#define BTA_OP_FILTER_TZ       (1<<10)  /* Time Zone */
#define BTA_OP_FILTER_GEO      (1<<11) /* Geographic Position */
#define BTA_OP_FILTER_TITLE    (1<<12) /* Job */
#define BTA_OP_FILTER_ROLE     (1<<13) /* Role within the Organization */
#define BTA_OP_FILTER_LOGO     (1<<14) /* Organization Logo */
#define BTA_OP_FILTER_AGENT    (1<<15) /* vCard of Person Representing */
#define BTA_OP_FILTER_ORG      (1<<16) /* Name of Organization */
#define BTA_OP_FILTER_NOTE     (1<<17) /* Comments */
#define BTA_OP_FILTER_REV      (1<<18) /* Revision */
#define BTA_OP_FILTER_SOUND    (1<<19) /* Pronunciation of Name */
#define BTA_OP_FILTER_URL      (1<<20) /* Uniform Resource Locator */
#define BTA_OP_FILTER_UID      (1<<21) /* Unique ID */
#define BTA_OP_FILTER_KEY      (1<<22) /* Public Encryption Key */
#define BTA_OP_FILTER_NICKNAME (1<<23) /* Nickname */
#define BTA_OP_FILTER_CATEGORIES (1<<24) /* Categories */
#define BTA_OP_FILTER_PROID    (1<<25) /* Product ID */
#define BTA_OP_FILTER_CLASS    (1<<26) /* Class Information */
#define BTA_OP_FILTER_SORT_STRING (1<<27) /* String used for sorting operations */
#define BTA_OP_FILTER_TIME_STAMP (1<<28) /* Time Stamp */
#define BTA_OP_FILTER_X_BT_SPEEDDIALKEY     (1<<29) /* Speed-dial shortcut */
#define BTA_OP_FILTER_X_BT_UCI              (1<<30) /* Uniform Caller Identifier field */
#define BTA_OP_FILTER_X_BT_UID              (1<<31) /* Bluetooth Contact Unique Identifier */
#define BTA_OP_FILTER_ALL      (0)

/* This structure describes an object property, or individual item, inside an object. */
typedef struct
{
    UINT8       *p_data;            /* Pointer to property data. */
    UINT32      parameters;         /* Property parameters. */
    UINT16      name;               /* Property name. */
    UINT16      len;                /* Length of data. */
    UINT8       *p_param;           /* Pointer to the Parameters */
    UINT16      param_len;          /* Param Len */
} tBTA_OP_PROP;


/* Access response types */
#define BTA_OP_ACCESS_ALLOW     0   /* Allow the requested operation */
#define BTA_OP_ACCESS_FORBID    1   /* Disallow the requested operation */
#define BTA_OP_ACCESS_NONSUP    2   /* Requested operation is not supported */

typedef UINT8 tBTA_OP_ACCESS;

/* Access event operation types */
#define BTA_OP_OPER_PUSH        1
#define BTA_OP_OPER_PULL        2

typedef UINT8 tBTA_OP_OPER;


/* Client callback function event */
#define BTA_OPC_ENABLE_EVT          0   /* Object push client is enabled. */
#define BTA_OPC_OPEN_EVT            1   /* Connection to peer is open. */
#define BTA_OPC_PROGRESS_EVT        2   /* push/pull in progres */
#define BTA_OPC_OBJECT_EVT          3   /* Object Pulled */
#define BTA_OPC_OBJECT_PSHD_EVT     4   /* Object pushed */
#define BTA_OPC_CLOSE_EVT           5   /* Connection to peer closed. */
#define BTA_OPC_MOVE_CH_EVT         6   /* AMP move channel event */

typedef UINT8 tBTA_OPC_EVT;

/* Client callback function result */
#define BTA_OPC_OK             0   /* Object push succeeded. */
#define BTA_OPC_FAIL           1   /* Object push failed. */
#define BTA_OPC_NOT_FOUND      2   /* Object not found. */
#define BTA_OPC_NO_PERMISSION  3   /* Operation not authorized. */
#define BTA_OPC_SRV_UNAVAIL    4   /* Service unavaliable */
#define BTA_OPC_ON_BT          5   /* only used for BTA_OPC_MOVE_CH_EVT */
#define BTA_OPC_ON_AMP         6   /* only used for BTA_OPC_MOVE_CH_EVT */
#define BTA_OPC_RSP_FORBIDDEN  7   /* Operation forbidden */
#define BTA_OPC_RSP_NOT_ACCEPTABLE  8  /* Operation not acceptable */

typedef UINT8 tBTA_OPC_STATUS;

/* Structure associated with BTA_OPC_OBJECT_EVT */
typedef struct
{
    char                *p_name;        /* Object name. */
    tBTA_OPC_STATUS    status;
} tBTA_OPC_OBJECT;

typedef struct
{
    UINT32          obj_size;   /* Total size of object (BTA_FS_LEN_UNKNOWN if unknown) */
    UINT16          bytes;      /* Number of bytes read or written since last progress event */
    tBTA_OP_OPER    operation;  /* Is progress for Push or Pull */
} tBTA_OPC_PROGRESS;

/* Union of all client callback structures */
typedef union
{
    tBTA_OPC_OBJECT   object;
    tBTA_OPC_PROGRESS prog;
    tBTA_OPC_STATUS   status;
} tBTA_OPC;

/* Client callback function */
typedef void (tBTA_OPC_CBACK)(tBTA_OPC_EVT event, tBTA_OPC *p_data);

/* Server callback function event */
#define BTA_OPS_ENABLE_EVT          0   /* Object push server is enabled. */
#define BTA_OPS_OPEN_EVT            1   /* Connection to peer is open. */
#define BTA_OPS_PROGRESS_EVT        2   /* Object being sent or received. */
#define BTA_OPS_OBJECT_EVT          3   /* Object has been received. */
#define BTA_OPS_CLOSE_EVT           4   /* Connection to peer closed. */
#define BTA_OPS_ACCESS_EVT          5   /* Request for access to push or pull object */
#define BTA_OPS_MOVE_CH_EVT         6   /* AMP move channel event */
#define BTA_OPS_DISABLE_EVT         7   /* Object push server is disabled.*/

typedef UINT8 tBTA_OPS_EVT;

/* Server callback function result */
#define BTA_OPS_ON_BT               1   /* only used for BTA_OPS_MOVE_CH_EVT */
#define BTA_OPS_ON_AMP              2   /* only used for BTA_OPS_MOVE_CH_EVT */

typedef UINT8 tBTA_OPS_STATUS;

/* Structure associated with BTA_OPS_OBJECT_EVT */
typedef struct
{
    char                *p_name;        /* Object name. */
    tBTA_OP_FMT         format;         /* Object format. */
} tBTA_OPS_OBJECT;

typedef struct
{
    UINT32              obj_size;       /* Total size of object (BTA_FS_LEN_UNKNOWN if unknown) */
    UINT16              bytes;          /* Number of bytes read or written since last progress event */
    tBTA_OP_OPER        operation;      /* Is progress for Push or Pull */
} tBTA_OPS_PROGRESS;

typedef struct
{
    char                *p_name;        /* Object name */
    char                *p_type;        /* Object type (NULL if not specified) */
    UINT32              size;           /* Object size */
    tBTM_BD_NAME        dev_name;       /* Name of device, "" if unknown */
    BD_ADDR             bd_addr;        /* Address of device */
    tBTA_OP_OPER        oper;           /* Operation (push or pull) */
    tBTA_OP_FMT         format;         /* Object format */
} tBTA_OPS_ACCESS;

/* Union of all server callback structures */
typedef union
{
    tBTA_OPS_STATUS     status;
    tBTA_OPS_OBJECT     object;
    tBTA_OPS_PROGRESS   prog;
    tBTA_OPS_ACCESS     access;
    BD_ADDR             bd_addr;
} tBTA_OPS;

/* Server callback function */
typedef void (tBTA_OPS_CBACK)(tBTA_OPS_EVT event, tBTA_OPS *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_OpcEnable
**
** Description      Enable the object push client.  This function must be
**                  called before any other functions in the OP API are called.
**                  When the enable operation is complete the callback function
**                  will be called with a BTA_OPC_ENABLE_EVT.
**
**                  If single_op is FALSE, the connection stays open after
**                  the operation finishes (until BTA_OpcClose is called).
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_OpcEnable(tBTA_SEC sec_mask, tBTA_OPC_CBACK *p_cback,
                                  BOOLEAN single_op, BOOLEAN srm, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_OpcDisable
**
** Description      Disable the object push client.  If the client is currently
**                  connected to a peer device the connection will be closed.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_OpcDisable(void);

/*******************************************************************************
**
** Function         BTA_OpcPush
**
** Description      Push an object to a peer device.  p_name must point to
**                  a fully qualified path and file name.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_OpcPush(BD_ADDR bd_addr, tBTA_OP_FMT format, char *p_name);

/*******************************************************************************
**
** Function         BTA_OpcPullCard
**
** Description      Pull default card from peer. p_path must point to a fully
**                  qualified path specifying where to store the pulled card.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_OpcPullCard(BD_ADDR bd_addr, char *p_path);


/*******************************************************************************
**
** Function         BTA_OpcExchCard
**
** Description      Exchange business cards with a peer device. p_send points to
**                  a fully qualified path and file name of vcard to push.
**                  p_recv_path points to a fully qualified path specifying
**                  where to store the pulled card.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_OpcExchCard(BD_ADDR bd_addr, char *p_send,
                                    char *p_recv_path);


/*******************************************************************************
**
** Function         BTA_OpcClose
**
** Description      Close the current connection.  This function is called if
**                  the phone wishes to close the connection before the object
**                  push is completed.  In a typical connection this function
**                  does not need to be called; the connection will be closed
**                  automatically when the object push is complete.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_OpcClose(void);

#define BTA_OPC_SWITCH_MODE_AUTO    0
#define BTA_OPC_SWITCH_MODE_AMP     1
#define BTA_OPC_SWITCH_MODE_BT      2

typedef UINT8 tBTA_OPC_SWITCH_MODE;

/*******************************************************************************
**
** Function         BTA_OpcSetSwitchMode
**
** Description      Set AMP switching mode
**                  BTA_OPC_SWITCH_MODE_AUTO: use auto switching algorithm
**                  BTA_OPC_SWITCH_MODE_AMP : use AMP if possible
**                  BTA_OPC_SWITCH_MODE_BT  : use BT only
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_OpcSetSwitchMode(tBTA_OPC_SWITCH_MODE mode);

/*******************************************************************************
**
** Function         BTA_OpsEnable
**
** Description      Enable the object push server.  This function must be
**                  called before any other functions in the OPS API are called.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_OpsEnable(tBTA_SEC sec_mask, tBTA_OP_FMT_MASK formats,
                                  char *p_service_name, tBTA_OPS_CBACK *p_cback,
                                  BOOLEAN srm, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_OpsDisable
**
** Description      Disable the object push server.  If the server is currently
**                  connected to a peer device the connection will be closed.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_OpsDisable(void);

/*******************************************************************************
**
** Function         BTA_OpsClose
**
** Description      Close the current connection.  This function is called if
**                  the phone wishes to close the connection before the object
**                  push is completed.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_OpsClose(void);

/*******************************************************************************
**
** Function         BTA_OpsAccessRsp
**
** Description      Sends a reply to an access request event (BTA_OPS_ACCESS_EVT).
**                  This call MUST be made whenever the event occurs.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_OpsAccessRsp(tBTA_OP_OPER oper, tBTA_OP_ACCESS access,
                                     char *p_name);

/*******************************************************************************
**
** Function         BTA_OpBuildCard
**
** Description      Build a vCard object. The input to this function is
**                  requested format(2.1/3.0), an array of vCard properties
**                  and a pointer to memory to store the card.
**                  The output is a formatted vCard.
**
**
** Returns          BTA_OP_OK if operation successful.
**                  BTA_OP_FAIL if invalid property data.
**                  BTA_OP_MEM if not enough memory to complete build.
**
*******************************************************************************/
BTA_API extern tBTA_OP_STATUS BTA_OpBuildCard(UINT8 *p_card, UINT16 *p_len,
                                              tBTA_OP_FMT fmt,
                                              tBTA_OP_PROP *p_prop,
                                              UINT8 num_prop);

/*******************************************************************************
**
** Function         BTA_OpBuildNote
**
** Description      Build a vNote object.  The input to this function is an
**                  array of vNote properties and a pointer to memory to store
**                  the card.  The output is a formatted vNote.
**
**
** Returns          BTA_OP_OK if operation successful.
**                  BTA_OP_FAIL if invalid property data.
**                  BTA_OP_MEM if not enough memory to complete build.
**
*******************************************************************************/
BTA_API extern tBTA_OP_STATUS BTA_OpBuildNote(UINT8 *p_note, UINT16 *p_len,
                                              tBTA_OP_PROP *p_prop,
                                              UINT8 num_prop);

/*******************************************************************************
**
** Function         BTA_OpBuildCal
**
** Description      Build a vCal 1.0 object.  The input to this function is an
**                  array of vCaalproperties and a pointer to memory to store
**                  the card.  The output is a formatted vCal.
**
**
** Returns          BTA_OP_OK if operation successful.
**                  BTA_OP_FAIL if invalid property data.
**                  BTA_OP_MEM if not enough memory to complete build.
**
*******************************************************************************/
BTA_API extern tBTA_OP_STATUS BTA_OpBuildCal(UINT8 *p_cal, UINT16 *p_len,
                                             tBTA_OP_PROP *p_prop,
                                             UINT8 num_prop,
                                             tBTA_OP_VCAL vcal_type);

/*******************************************************************************
**
** Function         BTA_OpParseCard
**
** Description      Parse a vCard 2.1 object.  The input to this function is
**                  a pointer to vCard data.  The output is an array of parsed
**                  vCard properties.
**
**
** Returns          BTA_OP_OK if operation successful.
**                  BTA_OP_FAIL if invalid property data.
**                  BTA_OP_MEM if not enough memory to complete parsing.
**
*******************************************************************************/
BTA_API extern tBTA_OP_STATUS BTA_OpParseCard(tBTA_OP_PROP *p_prop,
                                              UINT8 *p_num_prop, UINT8 *p_card,
                                              UINT32 len);

/*******************************************************************************
**
** Function         BTA_OpCheckCard
**
** Description      Go through the properties of the vcard and check whether it
**                  contains the properties meets the requirement of vCardSelector and
**                  vCardSelectorOperator
**
**
** Returns          BTA_OP_OK   if it does meets the requirements
**                  BTA_OP_FAIL otherwise
**
*******************************************************************************/
BTA_API extern tBTA_OP_STATUS BTA_OpCheckCard(tBTA_OP_PROP *p_prop, UINT8 num_prop);

/*******************************************************************************
**
** Function         BTA_OpGetCardProperty
**
** Description      Get Card property value by name.  The input to this function is
**                  property name.  The output is property value and len
**
**
** Returns          BTA_OP_OK if operation successful.
**                  BTA_OP_FAIL if invalid property data.
**
*******************************************************************************/
BTA_API extern tBTA_OP_STATUS BTA_OpGetCardProperty(UINT8 *p_value, UINT16 *p_len, tBTA_OP_PROP *p_prop,
                                     UINT8 num_prop, UINT8 *p_name);

/*******************************************************************************
**
** Function         BTA_OpParseNote
**
** Description      Parse a vNote object.  The input to this function is a
**                  pointer to vNote data.  The output is an array of parsed
**                  vNote properties.
**
**
** Returns          BTA_OP_OK if operation successful.
**                  BTA_OP_FAIL if invalid property data.
**                  BTA_OP_MEM if not enough memory to complete parsing.
**
*******************************************************************************/
BTA_API extern tBTA_OP_STATUS BTA_OpParseNote(tBTA_OP_PROP *p_prop,
                                              UINT8 *p_num_prop,
                                              UINT8 *p_note, UINT32 len);

/*******************************************************************************
**
** Function         BTA_OpParseCal
**
** Description      Parse a vCal object.  The input to this function is a
**                  pointer to vCal data.  The output is an array of parsed
**                  vCal properties.
**
**
** Returns          BTA_OP_OK if operation successful.
**                  BTA_OP_FAIL if invalid property data.
**                  BTA_OP_MEM if not enough memory to complete parsing.
**
*******************************************************************************/
BTA_API extern tBTA_OP_STATUS BTA_OpParseCal(tBTA_OP_PROP *p_prop,
                                             UINT8 *p_num_prop, UINT8 *p_cal,
                                             UINT32 len, tBTA_OP_VCAL *p_vcal_type);

/*******************************************************************************
**
** Function         BTA_OpSetCardSelectorOperator
**
** Description      Set vCardSelector and SelectorOperator
**
**
** Returns
**
*******************************************************************************/
BTA_API extern void BTA_OpSetCardSelectorOperator(UINT64 selector, UINT8 selector_op);

/*******************************************************************************
**
** Function         BTA_OpSetCardPropFilterMask
**
** Description      Set Property Filter Mask
**
**
** Returns
**
*******************************************************************************/
BTA_API extern void BTA_OpSetCardPropFilterMask(UINT64 mask);

/*******************************************************************************
**
** Function         BTA_OpGetPbSize
**
** Description      Get the phonebook size.
**
**
** Returns          Pointer to end of match or NULL if end of data reached.
**
*******************************************************************************/
BTA_API extern void BTA_OpGetPbSize(UINT8 *p_start, UINT8 *p_end, UINT16 *pb_size);

#ifdef __cplusplus
}
#endif

#endif /* BTA_OP_API_H */
