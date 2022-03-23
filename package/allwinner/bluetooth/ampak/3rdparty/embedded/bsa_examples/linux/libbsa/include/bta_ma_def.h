/*****************************************************************************
**
**  Name:           bta_ma_def.h
**
**  Description:    This file contains the common definitions for the Message Access
**                  profile (MA) related SW modules
**
**  Copyright (c) 2009-2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_MA_DEF_H
#define BTA_MA_DEF_H

#include "obx_api.h"
#include "bta_api.h"
#include "btm_api.h"
#include "bta_sys.h"
#include "bta_fs_co.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/


#define BTA_MA_HANDLE_SIZE          8

typedef UINT8  tBTA_MA_MSG_HANDLE[BTA_MA_HANDLE_SIZE];

#define BTA_MA_DEFAULT_SUPPORTED_FEATURES   0x0000001F

typedef UINT32 tBTA_MA_SUPPORTED_FEATURES;


#define BTA_MA_DATETIME_SIZE         19   /* In the format of YEAR MONTH DATE T HOURS MINUTES SECONDS "<20120822T100000>" */
#define BTA_MA_SUBJECT_SIZE          255  /* 255 according to MAP 1.2*/
#define BTA_MA_PRIORITY_SIZE         7    /* Either "<Yes>" or "<No>"*/
#define BTA_MA_NAME_LEN              255  /* 255 according to MAP 1.2*/


typedef tOBX_HANDLE tBTA_MA_SESS_HANDLE;

#ifndef BTA_MA_INS_INFO_MAX_LEN
#define BTA_MA_INS_INFO_MAX_LEN    200  /* Instance info cannot be longer than 200 accoriding to spec (including NULL termination) */
#endif
#if (defined(BTA_MAP_1_2_SUPPORTED) && BTA_MAP_1_2_SUPPORTED == TRUE)
typedef char tBTA_MA_MAS_INS_INFO[BTA_MA_INS_INFO_MAX_LEN];
#endif

typedef UINT8 tBTA_MA_INST_ID;

#define BTA_MA_SUP_FEA_NOTIF_REG        0x00000001
#define BTA_MA_SUP_FEA_NOTIF            0x00000002
#define BTA_MA_SUP_FEA_BROWSING         0x00000004
#define BTA_MA_SUP_FEA_UPLOADING        0x00000008
#define BTA_MA_SUP_FEA_DELETE           0x00000010
#define BTA_MA_SUP_FEA_INST_INFO        0x00000020
#define BTA_MA_SUP_FEA_EXT_EVENT_REP    0x00000040

typedef UINT32 tBTA_MA_SUP_FEA_MASK;


#define BTA_MA_STATUS_OK                0
#define BTA_MA_STATUS_FAIL              1   /* Used to pass all other errors */
#define BTA_MA_STATUS_ABORTED           2
#define BTA_MA_STATUS_NO_RESOURCE       3
#define BTA_MA_STATUS_EACCES            4
#define BTA_MA_STATUS_ENOTEMPTY         5
#define BTA_MA_STATUS_EOF               6
#define BTA_MA_STATUS_EODIR             7
#define BTA_MA_STATUS_ENOSPACE          8   /* Returned in bta_fs_ci_open if no room */
#define BTA_MA_STATUS_DUPLICATE_ID      9
#define BTA_MA_STATUS_ID_NOT_FOUND      10
#define BTA_MA_STATUS_FULL              11  /* reach the max packet size */
#define BTA_MA_STATUS_UNSUP_FEATURES    12  /* feature is not suppored on the peer device */

typedef UINT8 tBTA_MA_STATUS;

#define BTA_MA_OPER_NONE                0
#define BTA_MA_OPER_GET_MSG             1
#define BTA_MA_OPER_PUSH_MSG            2

typedef UINT8 tBTA_MA_OPER;

/* mode field in tBTA_MSE_CO_FOLDER_ENTRY (OR'd together) */
#define BTA_MA_A_RDONLY         1
#define BTA_MA_A_DIR            2      /* Entry is a sub directory */


/* message status inficator */
#define BTA_MA_STS_INDTR_READ       0
#define BTA_MA_STS_INDTR_DELETE     1
typedef UINT8 tBTA_MA_STS_INDCTR;

/* message status value */
#define BTA_MA_STS_VALUE_NO         0
#define BTA_MA_STS_VALUE_YES        1
typedef UINT8 tBTA_MA_STS_VALUE;

/* notification status */
enum
{
    BTA_MA_NOTIF_OFF = 0,
    BTA_MA_NOTIF_ON,
    BTA_MA_NOTIF_MAX

};
typedef UINT8 tBTA_MA_NOTIF_STATUS;


/* Access response types */
enum
{
    BTA_MA_ACCESS_TYPE_ALLOW = 0,    /* Allow the requested operation */
    BTA_MA_ACCESS_TYPE_FORBID        /* Disallow the requested operation */
};

typedef UINT8 tBTA_MA_ACCESS_TYPE;

/* Structure for in progress related event*/
typedef struct
{
    UINT32              obj_size;   /* Total size of object 0 if unknow*/
    tBTA_MA_SESS_HANDLE mas_session_id;
    UINT16              bytes;      /* Number of bytes read or written since last progress event */
} tBTA_MA_IN_PROG;


/* Message type see SDP supported message type */
#define BTA_MA_MSG_TYPE_EMAIL                (1<<0)
#define BTA_MA_MSG_TYPE_SMS_GSM              (1<<1)
#define BTA_MA_MSG_TYPE_SMS_CDMA             (1<<2)
#define BTA_MA_MSG_TYPE_MMS                  (1<<3)
typedef UINT8 tBTA_MA_MSG_TYPE;

#define BTA_MA_MAX_FILTER_TEXT_SIZE 255

/* Message type mask for FilterMessageType in Application parameter */
#define BTA_MA_MSG_TYPE_MASK_SMS_GSM         (1<<0)
#define BTA_MA_MSG_TYPE_MASK_SMS_CDMA        (1<<1)
#define BTA_MA_MSG_TYPE_MASK_EMAIL           (1<<2)
#define BTA_MA_MSG_TYPE_MASK_MMS             (1<<3)


typedef UINT8 tBTA_MA_MSG_TYPE_MASK;



/* Parameter Mask for Messages-Listing   */
#define BTA_MA_ML_MASK_SUBJECT               (1<<0)
#define BTA_MA_ML_MASK_DATETIME              (1<<1)
#define BTA_MA_ML_MASK_SENDER_NAME           (1<<2)
#define BTA_MA_ML_MASK_SENDER_ADDRESSING     (1<<3)
#define BTA_MA_ML_MASK_RECIPIENT_NAME        (1<<4)
#define BTA_MA_ML_MASK_RECIPIENT_ADDRESSING  (1<<5)
#define BTA_MA_ML_MASK_TYPE                  (1<<6)
#define BTA_MA_ML_MASK_SIZE                  (1<<7)
#define BTA_MA_ML_MASK_RECEPTION_STATUS      (1<<8)
#define BTA_MA_ML_MASK_TEXT                  (1<<9)
#define BTA_MA_ML_MASK_ATTACHMENT_SIZE       (1<<10)
#define BTA_MA_ML_MASK_PRIORITY              (1<<11)
#define BTA_MA_ML_MASK_READ                  (1<<12)
#define BTA_MA_ML_MASK_SENT                  (1<<13)
#define BTA_MA_ML_MASK_PROTECTED             (1<<14)
#define BTA_MA_ML_MASK_REPLYTO_ADDRESSING    (1<<15)

typedef UINT32 tBTA_MA_ML_MASK;

/* Read status used for  message list */
enum
{
    BTA_MA_READ_STATUS_NO_FILTERING = 0,
    BTA_MA_READ_STATUS_UNREAD       = 1,
    BTA_MA_READ_STATUS_READ         = 2
};
typedef UINT8 tBTA_MA_READ_STATUS;

/* Priority status used for filtering message list */
enum
{
    BTA_MA_PRI_STATUS_NO_FILTERING = 0,
    BTA_MA_PRI_STATUS_HIGH         = 1,
    BTA_MA_PRI_STATUS_NON_HIGH     = 2
};
typedef UINT8 tBTA_MA_PRI_STATUS;

#define BTA_MA_LTIME_LEN 15
typedef struct
{
    tBTA_MA_ML_MASK       parameter_mask;
    UINT16                max_list_cnt;
    UINT16                list_start_offset;
    UINT8                 subject_length;  /* valid range 1...255 */
    tBTA_MA_MSG_TYPE_MASK msg_mask;
    char                  period_begin[BTA_MA_LTIME_LEN+1]; /* "yyyymmddTHHMMSS", or "" if none */
    char                  period_end[BTA_MA_LTIME_LEN+1]; /* "yyyymmddTHHMMSS", or "" if none */
    tBTA_MA_READ_STATUS   read_status;
    char                  recipient[BTA_MA_MAX_FILTER_TEXT_SIZE+1]; /* "" if none */
    char                  originator[BTA_MA_MAX_FILTER_TEXT_SIZE+1];/* "" if none */
    tBTA_MA_PRI_STATUS    pri_status;
} tBTA_MA_MSG_LIST_FILTER_PARAM;

/* enum for charset used in GetMEssage */
enum
{
    BTA_MA_CHARSET_NATIVE = 0,
    BTA_MA_CHARSET_UTF_8  = 1,
    BTA_MA_CHARSET_UNKNOWN,
    BTA_MA_CHARSET_MAX
};
typedef UINT8 tBTA_MA_CHARSET;

/* enum for fraction request used in GetMEssage */
enum
{
    BTA_MA_FRAC_REQ_FIRST = 0,
    BTA_MA_FRAC_REQ_NEXT  = 1,
    BTA_MA_FRAC_REQ_NO,        /* this is not a fraction request */
    BTA_MA_FRAC_REQ_MAX
};
typedef UINT8 tBTA_MA_FRAC_REQ;

/* enum for fraction delivery used in GetMEssage */
enum
{
    BTA_MA_FRAC_DELIVER_MORE  = 0,
    BTA_MA_FRAC_DELIVER_LAST  = 1,
    BTA_MA_FRAC_DELIVER_NO,          /* this is not a fraction deliver*/
    BTA_MA_FRAC_DELIVER_MAX
};
typedef UINT8 tBTA_MA_FRAC_DELIVER;


typedef struct
{
    BOOLEAN            attachment;
    tBTA_MA_MSG_HANDLE handle;
    tBTA_MA_CHARSET    charset;
    tBTA_MA_FRAC_REQ   fraction_request;
} tBTA_MA_GET_MSG_PARAM;

#define BTA_MA_RETRY_OFF        0
#define BTA_MA_RETRY_ON         1
#define BTA_MA_RETRY_UNKNOWN    0xff
typedef UINT8   tBTA_MA_RETRY_TYPE;


#define BTA_MA_TRANSP_OFF        0
#define BTA_MA_TRANSP_ON         1
#define BTA_MA_TRANSP_UNKNOWN    0xff
typedef UINT8   tBTA_MA_TRANSP_TYPE;

typedef struct
{
    char                  *p_folder;  /* current or child folder
                                         for current folder set
                                         *p_folder = ""
                                      */
    char                  *p_msg_name; /* for MCE use only*/
    tBTA_MA_TRANSP_TYPE   transparent;
    tBTA_MA_RETRY_TYPE    retry;
    tBTA_MA_CHARSET       charset;

} tBTA_MA_PUSH_MSG_PARAM;

/* get or push message multi-packet status */
enum
{
    BTA_MA_MPKT_STATUS_MORE = 0,
    BTA_MA_MPKT_STATUS_LAST,
    BTA_MA_MPKT_MAX
};

typedef UINT8 tBTA_MA_MPKT_STATUS;

/* definitions for directory navigation */
#define BTA_MA_DIR_NAV_ROOT_OR_DOWN_ONE_LVL     2
#define BTA_MA_DIR_NAV_UP_ONE_LVL               3

typedef UINT8 tBTA_MA_DIR_NAV;

enum
{
    BTA_MA_ATTACH_OFF = 0,
    BTA_MA_ATTACH_ON
};
typedef UINT8 tBTA_MA_ATTACH_TYPE;

/* MAS tag ID in application parameters header definition */
#define BTA_MA_APH_MAX_LIST_COUNT   0x01    /* MaxListCount     2 bytes     0x0000 to 0xFFFF */
#define BTA_MA_APH_START_STOFF      0x02    /* StartOffset      2 bytes     0x0000 to 0xFFFF */
#define BTA_MA_APH_FILTER_MSG_TYPE  0x03    /* SearchAttribute  1 byte      1,2,4 */
#define BTA_MA_APH_FILTER_PRD_BEGIN 0x04    /* Filter Period Begin  variable */
#define BTA_MA_APH_FILTER_PRD_END   0x05    /* Filter Period End    variable */
#define BTA_MA_APH_FILTER_READ_STS  0x06    /* Filter read status 1 byte      0, 1, 2 */
#define BTA_MA_APH_FILTER_RECEIP    0x07    /* FilterRecipient variable */
#define BTA_MA_APH_FILTER_ORIGIN    0x08    /* FilterOriginator variable */
#define BTA_MA_APH_FILTER_PRIORITY  0x09    /* FilterPriority 1 byte */
#define BTA_MA_APH_ATTACH           0x0a    /* Attachment 1 byte */
#define BTA_MA_APH_TRANSPARENT      0x0b    /* transparent 1 byte */
#define BTA_MA_APH_RETRY            0x0c    /* retry 1 byte */
#define BTA_MA_APH_NEW_MSG          0x0d    /* NewMessage 1 byte */
#define BTA_MA_APH_NOTIF_STATUS     0x0e    /* Notification Status 1 byte */
#define BTA_MA_APH_MAS_INST_ID      0x0f    /* MAS instance ID 1 byte  0 ... 255 */
#define BTA_MA_APH_PARAM_MASK       0x10    /* Parameter mask    2 bytes  */
#define BTA_MA_APH_FOLDER_LST_SIZE  0x11    /* Folder Listing Size    2 bytes  */
#define BTA_MA_APH_MSG_LST_SIZE     0x12    /* Message Listing Size    2 bytes  */
#define BTA_MA_APH_SUBJ_LEN         0x13    /* Subject Length    1 byte  */
#define BTA_MA_APH_CHARSET          0x14    /* Character Set    1 byte :0, 1 */
#define BTA_MA_APH_FRAC_REQ         0x15    /* Fraction request    1 byte :0, 1 */
#define BTA_MA_APH_FRAC_DELVR       0x16    /* Fraction delivery    1 byte :0, 1 */
#define BTA_MA_APH_STS_INDCTR       0x17    /* Status Indicator    1 byte  */
#define BTA_MA_APH_STS_VALUE        0x18    /* Status Value    1 byte: 0, 1  */
#define BTA_MA_APH_MSE_TIME         0x19    /* MSETime variable  */
#define BTA_MA_BODY_FILLER_BYTE     0x30    /* Used for SetMessageStatus */

/* MAS type header */
#define BTA_MA_HDR_TYPE_NOTIF_REG       "x-bt/MAP-NotificationRegistration"
#define BTA_MA_HDR_TYPE_MSG_UPDATE      "x-bt/MAP-messageUpdate"
#define BTA_MA_HDR_TYPE_EVENT_RPT       "x-bt/MAP-event-report"
#define BTA_MA_HDR_TYPE_MSG_LIST        "x-bt/MAP-msg-listing"
#define BTA_MA_HDR_TYPE_MSG             "x-bt/message"
#define BTA_MA_HDR_TYPE_MSG_STATUS      "x-bt/messageStatus"
#define BTA_MA_HDR_TYPE_FOLDER_LIST     "x-obex/folder-listing"
#if (defined(BTA_MAP_1_2_SUPPORTED) && BTA_MAP_1_2_SUPPORTED == TRUE)
#define BTA_MA_HDR_TYPE_MAS_INS_INFO    "x-bt/MASInstanceInformation"
#endif

#define BTA_MAS_MESSAGE_ACCESS_TARGET_UUID       "\xBB\x58\x2B\x40\x42\x0C\x11\xDB\xB0\xDE\x08\x00\x20\x0C\x9A\x66"
#define BTA_MAS_MESSAGE_NOTIFICATION_TARGET_UUID "\xBB\x58\x2B\x41\x42\x0C\x11\xDB\xB0\xDE\x08\x00\x20\x0C\x9A\x66"
#define BTA_MAS_UUID_LENGTH                 16

#define BTA_MA_VERSION_1_0          0x0100      /* MAP 1.0 */
#define BTA_MA_VERSION_1_1          0x0101      /* MAP 1.1 */
#define BTA_MA_VERSION_1_2          0x0102      /* MAP 1.2 */


#define BTA_MA_NOTIF_STS_TAG_ID     0x0E
#define BTA_MA_NOTIF_STS_LEN        0x01
#define BTA_MA_NOTIF_STS_ON         0x01
#define BTA_MA_NOTIF_STS_OFF        0x00

#define BTA_MA_NAS_INST_ID_TAG_ID     0x0F
#define BTA_MA_NAS_INST_ID_LEN        0x01

#define BTA_MA_DEFAULT_MAX_LIST_CNT   1024


#define BTA_MA_64BIT_HEX_STR_SIZE               (16+1)
#define BTA_MA_32BIT_HEX_STR_SIZE               (8+1)



/*******************************************************************************
**
** bMessage types
**
** Description      The following types are designed to hold data in memory.
**                  The internal structure of these types are implementation
**                  specific.
**
*******************************************************************************/

enum
{
    BTA_MA_BMSG_ENC_8BIT = 0,      /* Used for Email/MMS */

    BTA_MA_BMSG_ENC_G7BIT,         /* Used for GSM-SMS */
    BTA_MA_BMSG_ENC_G7BITEXT,
    BTA_MA_BMSG_ENC_GUCS2,
    BTA_MA_BMSG_ENC_G8BIT,

    BTA_MA_BMSG_ENC_C8BIT,         /* Used for CDMA-SMS */
    BTA_MA_BMSG_ENC_CEPM,
    BTA_MA_BMSG_ENC_C7ASCII,
    BTA_MA_BMSG_ENC_CIA5,
    BTA_MA_BMSG_ENC_CUNICODE,
    BTA_MA_BMSG_ENC_CSJIS,
    BTA_MA_BMSG_ENC_CKOREAN,
    BTA_MA_BMSG_ENC_CLATINHEB,
    BTA_MA_BMSG_ENC_CLATIN,

    BTA_MA_BMSG_ENC_UNKNOWN
};
typedef UINT8 tBTA_MA_BMSG_ENCODING;

enum
{
    BTA_MA_BMSG_LANG_UNSPECIFIED = 0,

    BTA_MA_BMSG_LANG_UNKNOWN,
    BTA_MA_BMSG_LANG_SPANISH,      /* GSM-SMS or CDMA-SMS */

    BTA_MA_BMSG_LANG_TURKISH,      /* GSM-SMS only */
    BTA_MA_BMSG_LANG_PORTUGUESE,

    BTA_MA_BMSG_LANG_ENGLISH,      /* CDMA-SMS only */
    BTA_MA_BMSG_LANG_FRENCH,
    BTA_MA_BMSG_LANG_JAPANESE,
    BTA_MA_BMSG_LANG_KOREAN,
    BTA_MA_BMSG_LANG_CHINESE,
    BTA_MA_BMSG_LANG_HEBREW
};
typedef UINT8 tBTA_MA_BMSG_LANGUAGE;


enum
{
    BTA_MA_VCARD_VERSION_21=0,
    BTA_MA_VCARD_VERSION_30
};
typedef UINT8 tBTA_MA_VCARD_VERSION;

enum
{
    BTA_MA_VCARD_PROP_N,
    BTA_MA_VCARD_PROP_FN,
    BTA_MA_VCARD_PROP_TEL,
    BTA_MA_VCARD_PROP_EMAIL,

    BTA_MA_VCARD_PROP_MAX
};
typedef UINT8 tBTA_MA_VCARD_PROP;

typedef struct
{
    char *                      p_param;
    char *                      p_value;

    /* link to the next property (if any) */
    void *                      p_next;

} tBTA_MA_VCARD_PROPERTY;

typedef struct
{
    tBTA_MA_VCARD_VERSION         version;

    tBTA_MA_VCARD_PROPERTY *           p_prop[BTA_MA_VCARD_PROP_MAX];

    /* link to the next vCard (if any) */
    void *                      p_next;

} tBTA_MA_BMSG_VCARD;

typedef struct BMSG_MESSAGE_struct
{
    char *                      p_text;

    /* link to the next chunk of message text (if any) */
    void *                      p_next;

} tBTA_MA_BMSG_MESSAGE;

typedef struct BMSG_CONTENT_struct
{
    /* this is the first bit of message text */
    tBTA_MA_BMSG_MESSAGE *             p_message;

    /* this points to the last entered text -or-
    ** it is the last that we returned back to
    **
    */
    tBTA_MA_BMSG_MESSAGE *             p_last;

    /* link to the next chunk of content (if any) */
    void *                      p_next;

} tBTA_MA_BMSG_CONTENT;


typedef struct
{
    /* Part ID */
    UINT16                      part_id;
    BOOLEAN                     is_multipart;

    /* Properties */
    tBTA_MA_BMSG_ENCODING              encoding;
    tBTA_MA_BMSG_LANGUAGE              language;       /* optional */
    tBTA_MA_CHARSET                    charset;
    /* One or more body content */
    tBTA_MA_BMSG_CONTENT *             p_content;

} tBTA_MA_BMSG_BODY;

typedef struct BMSG_ENVELOPE_struct
{
    /* One or more Recipient (vCards) */
    tBTA_MA_BMSG_VCARD *               p_recip;

    /* There will be either another envelope or the body */
    void *                      p_next;
    tBTA_MA_BMSG_BODY *                p_body;

} tBTA_MA_BMSG_ENVELOPE;

typedef struct
{
    /* Property values */
    BOOLEAN                     read_sts;
    tBTA_MA_MSG_TYPE            msg_type;
    char *                      p_folder;

    /* One or more Originator (vCards) */
    tBTA_MA_BMSG_VCARD *               p_orig;

    /* Envelope */
    tBTA_MA_BMSG_ENVELOPE *            p_envelope;

} tBTA_MA_BMSG;


#endif
