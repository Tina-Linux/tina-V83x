/*****************************************************************************
**
**  Name:           bta_ft_api.h
**
**  Description:    This is the public interface file for the file transfer
**                  (FT) server subsystem of BTA, Widcomm's
**                  Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2003-2009, Broadcom Corp., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_FT_API_H
#define BTA_FT_API_H

#include "bta_api.h"
#include "btm_api.h"
#include "bip_api.h"
#include "bta_sys.h"
#include "bta_fs_co.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
#define BTA_FT_VERSION_1_1          0x0101
#define BTA_FT_VERSION_1_2          0x0102
#define BTA_FT_VERSION_1_3          0x0103

#define BTA_FT_ENHANCED_VERSION     BTA_FT_VERSION_1_2

/**************************
**  Common Definitions
***************************/
/* Access response types */
#define BTA_FT_ACCESS_ALLOW     0   /* Allow the requested operation */
#define BTA_FT_ACCESS_FORBID    1   /* Disallow the requested operation */

typedef UINT8 tBTA_FT_ACCESS;

/* Access event operation types */
#define BTA_FT_OPER_DEFAULT     0   /* Default mode */
#define BTA_FT_OPER_PUT         1   /* Request is a PUT file */
#define BTA_FT_OPER_GET         2   /* Request is a GET file */
#define BTA_FT_OPER_DEL_FILE    3   /* Request is a DELETE file */
#define BTA_FT_OPER_DEL_DIR     4   /* Request is a DELETE folder */
#define BTA_FT_OPER_CHG_DIR     5   /* Request is a Change Folder */
#define BTA_FT_OPER_MK_DIR      6   /* Request is a Make Folder */
#define BTA_FT_OPER_COPY_ACT    7   /* Request is a Copy Action command */
#define BTA_FT_OPER_MOVE_ACT    8   /* Request is a Move Action command */
#define BTA_FT_OPER_SET_PERM    9   /* Request is a SetPermission Action command */

typedef UINT8 tBTA_FT_OPER;

#define BTA_FT_ACT_COPY         OBX_ACT_COPY        /* 0x00 Copy object */
#define BTA_FT_ACT_MOVE         OBX_ACT_MOVE        /* 0x01 Move/rename object */
#define BTA_FT_ACT_PERMISSION   OBX_ACT_PERMISSION  /* 0x02 Set object permission */
typedef tOBX_ACTION tBTA_FTC_ACT;

#define BTA_FTC_RESUME_NONCE	0xFFFFFFFF

/* permission flags */
#define BTA_FT_PERMISSION_READ   OBX_PERMISSION_READ   /* 0x01 */
#define BTA_FT_PERMISSION_WRITE  OBX_PERMISSION_WRITE  /* 0x02 */
#define BTA_FT_PERMISSION_DELETE OBX_PERMISSION_DELETE /* 0x04 */
#define BTA_FT_PERMISSION_MODIFY OBX_PERMISSION_MODIFY /* 0x80 */

/**************************
**  Server Definitions
***************************/
/* Extra Debug Code */
#ifndef BTA_FTS_DEBUG
#define BTA_FTS_DEBUG           FALSE
#endif

#define BTA_FTS_OK              0
#define BTA_FTS_FAIL            1
#define BTA_FTS_ON_BT           2   /* only used for BTA_FTS_MOVE_CH_EVT */
#define BTA_FTS_ON_AMP          3   /* only used for BTA_FTS_MOVE_CH_EVT */
typedef UINT8 tBTA_FTS_STATUS;

/* Server callback function events */
#define BTA_FTS_ENABLE_EVT      0   /* File transfer server is enabled. */
#define BTA_FTS_OPEN_EVT        1   /* Connection to peer is open. */
#define BTA_FTS_CLOSE_EVT       2   /* Connection to peer closed. */
#define BTA_FTS_AUTH_EVT        3   /* Request for Authentication key and realm */
#define BTA_FTS_ACCESS_EVT      4   /* Request for access to put a file */
#define BTA_FTS_PROGRESS_EVT    5   /* Number of bytes read or written so far */
#define BTA_FTS_PUT_CMPL_EVT    6   /* File Put complete */
#define BTA_FTS_GET_CMPL_EVT    7   /* File Get complete */
#define BTA_FTS_DEL_CMPL_EVT    8   /* Remove File or Folder complete */
#define BTA_FTS_DISABLE_EVT     9   /* Reply to a disable api request */
#define BTA_FTS_MOVE_CH_EVT    10   /* AMP move channel event */

typedef UINT8 tBTA_FTS_EVT;

/* Structure associated with BTA_FTS_xxx_CMPL_EVT */
typedef struct
{
    char               *p_name;        /* file or folder name. */
    tBTA_FTS_STATUS     status;
} tBTA_FTS_OBJECT;

typedef struct
{
    UINT32 file_size;   /* Total size of file (BTA_FS_LEN_UNKNOWN if unknown) */
    UINT16 bytes;       /* Number of bytes read or written since last progress event */
} tBTA_FTS_PROGRESS;

typedef struct
{

    UINT8  *p_userid;
    UINT8   userid_len;
    BOOLEAN userid_required;    /* TRUE if user ID is required in response (rechallanged)  */
} tBTA_FTS_AUTH;

typedef struct
{
    char          *p_name;      /* file name with fully qualified path */
    UINT32         size;        /* file size */
    tBTM_BD_NAME   dev_name;    /* Name of device, "" if unknown */
    tBTA_FT_OPER   oper;        /* operation (put) */
    BD_ADDR        bd_addr;     /* Address of device */
    char          *p_dest_name; /* destination file name with fully qualified path (BTA_FT_OPER_COPY_ACT & BTA_FT_OPER_MOVE_ACT) */
    UINT8          perms[BTA_FS_PERM_SIZE]; /* user/group/other permission (BTA_FT_OPER_SET_PERM) */
} tBTA_FTS_ACCESS;

typedef union
{
    tBTA_FTS_STATUS     status;
    tBTA_FTS_PROGRESS   prog;
    tBTA_FTS_AUTH       auth;
    tBTA_FTS_ACCESS     access;
    tBTA_FTS_OBJECT     obj;
    BD_ADDR             bd_addr;
} tBTA_FTS;

/* Server callback function */
typedef void tBTA_FTS_CBACK(tBTA_FTS_EVT event, tBTA_FTS *p_data);


/**************************
**  Client Definitions
***************************/
/* Extra Debug Code */
#ifndef BTA_FTC_DEBUG
#define BTA_FTC_DEBUG           FALSE
#endif

/* Additional paramters for BTA_FtcPutFile using BIP service */
typedef union
{
    tBIP_IMAGE_DESC     desc;       /* when connectied with BIP service  */
} tBTA_FTC_PARAM;

/* Client callback function events */
#define BTA_FTC_ENABLE_EVT      0   /* File transfer client is enabled. */
#define BTA_FTC_OPEN_EVT        1   /* Connection to peer is open. */
#define BTA_FTC_CLOSE_EVT       2   /* Connection to peer closed. */
#define BTA_FTC_AUTH_EVT        3   /* Request for Authentication key and user id */
#define BTA_FTC_LIST_EVT        4   /* Event contains a directory entry (tBTA_FTC_LIST) */
#define BTA_FTC_PROGRESS_EVT    5   /* Number of bytes read or written so far */
#define BTA_FTC_PUTFILE_EVT     6   /* File Put complete */
#define BTA_FTC_GETFILE_EVT     7   /* File Get complete */
#define BTA_FTC_BI_CAPS_EVT     8   /* BIP imaging capabilities */
#define BTA_FTC_THUMBNAIL_EVT   9   /* BIP responder requests for the thumbnail version */
#define BTA_FTC_CHDIR_EVT      10   /* Change Directory complete */
#define BTA_FTC_MKDIR_EVT      11   /* Make Directory complete */
#define BTA_FTC_REMOVE_EVT     12   /* Remove File/Directory complete */
#define BTA_FTC_PHONEBOOK_EVT  13   /* Report the Application Parameters for BTA_FtcGetPhoneBook response */
#define BTA_FTC_COPY_EVT       14   /* Copy File complete */
#define BTA_FTC_MOVE_EVT       15   /* Move File complete */
#define BTA_FTC_PERMISSION_EVT 16   /* Set File permission complete */
#define BTA_FTC_MOVE_CH_EVT    17   /* AMP move channel event */


typedef UINT8 tBTA_FTC_EVT;


#define BTA_FTC_OK              0
#define BTA_FTC_FAIL            1
#define BTA_FTC_NO_PERMISSION   2
#define BTA_FTC_NOT_FOUND       3
#define BTA_FTC_FULL            4
#define BTA_FTC_BUSY            5
#define BTA_FTC_ABORTED         6
#define BTA_FTC_SERVICE_UNAVL   7
#define BTA_FTC_SDP_ERR         8
#define BTA_FTC_OBX_ERR         9
#define BTA_FTC_OBX_TOUT        10
#define BTA_FTC_ON_BT           11  /* only used for BTA_FTS_MOVE_CH_EVT */
#define BTA_FTC_ON_AMP          12  /* only used for BTA_FTS_MOVE_CH_EVT */

typedef UINT8 tBTA_FTC_STATUS;

#define BTA_FTC_FLAG_NONE       0
#define BTA_FTC_FLAG_BACKUP     1

typedef UINT8 tBTA_FTC_FLAG;

typedef struct
{
    tBTA_SERVICE_ID service;    /* Connection is open with OPP, BIP, PBAP or FTP service */
    UINT16          version;
} tBTA_FTC_OPEN;

#define BTA_FTC_FILTER_VERSION  (1<<0)  /* Version */
#define BTA_FTC_FILTER_FN       (1<<1)  /* Formatted Name */
#define BTA_FTC_FILTER_N        (1<<2)  /* Structured Presentation of Name */
#define BTA_FTC_FILTER_PHOTO    (1<<3)  /* Associated Image or Photo */
#define BTA_FTC_FILTER_BDAY     (1<<4)  /* Birthday */
#define BTA_FTC_FILTER_ADR      (1<<5)  /* Delivery Address */
#define BTA_FTC_FILTER_LABEL    (1<<6)  /* Delivery */
#define BTA_FTC_FILTER_TEL      (1<<7)  /* Telephone Number */
#define BTA_FTC_FILTER_EMAIL    (1<<8)  /* Electronic Mail Address */
#define BTA_FTC_FILTER_MAILER   (1<<9)  /* Electronic Mail */
#define BTA_FTC_FILTER_TZ       (1<<10)  /* Time Zone */
#define BTA_FTC_FILTER_GEO      (1<<11) /* Geographic Position */
#define BTA_FTC_FILTER_TITLE    (1<<12) /* Job */
#define BTA_FTC_FILTER_ROLE     (1<<13) /* Role within the Organization */
#define BTA_FTC_FILTER_LOGO     (1<<14) /* Organization Logo */
#define BTA_FTC_FILTER_AGENT    (1<<15) /* vCard of Person Representing */
#define BTA_FTC_FILTER_ORG      (1<<16) /* Name of Organization */
#define BTA_FTC_FILTER_NOTE     (1<<17) /* Comments */
#define BTA_FTC_FILTER_REV      (1<<18) /* Revision */
#define BTA_FTC_FILTER_SOUND    (1<<19) /* Pronunciation of Name */
#define BTA_FTC_FILTER_URL      (1<<20) /* Uniform Resource Locator */
#define BTA_FTC_FILTER_UID      (1<<21) /* Unique ID */
#define BTA_FTC_FILTER_KEY      (1<<22) /* Public Encryption Key */
#define BTA_FTC_FILTER_ALL      (0)
typedef UINT32 tBTA_FTC_FILTER_MASK;

enum
{
    BTA_FTC_FORMAT_CARD_21, /* vCard format 2.1 */
    BTA_FTC_FORMAT_CARD_30, /* vCard format 3.0 */
    BTA_FTC_FORMAT_MAX
};
typedef UINT8 tBTA_FTC_FORMAT;

typedef struct
{
    UINT16          phone_book_size;
    BOOLEAN         pbs_exist;          /* phone_book_size is present in the response */
    UINT8           new_missed_calls;
    BOOLEAN         nmc_exist;          /* new_missed_calls is present in the response */
} tBTA_FTC_PB_PARAM;

typedef struct
{
    tBTA_FTC_PB_PARAM *p_param;
    UINT8           *data;
    UINT16           len;
    BOOLEAN          final;     /* If TRUE, entry is last of the series */
    tBTA_FTC_STATUS  status;    /* Fields are valid when status is BTA_FTC_OK */
} tBTA_FTC_LIST;

enum
{
    BTA_FTC_ORDER_INDEXED = 0,  /* indexed */
    BTA_FTC_ORDER_ALPHANUM,     /* alphanumeric */
    BTA_FTC_ORDER_PHONETIC,      /* phonetic */
    BTA_FTC_ORDER_MAX
};
typedef UINT8 tBTA_FTC_ORDER;
enum
{
    BTA_FTC_ATTR_NAME = 0,      /* name */
    BTA_FTC_ATTR_NUMBER,        /* number */
    BTA_FTC_ATTR_SOUND,         /* sound */
    BTA_FTC_ATTR_MAX
};
typedef UINT8 tBTA_FTC_ATTR;

typedef struct
{
    UINT32 file_size;   /* Total size of file (BTA_FS_LEN_UNKNOWN if unknown) */
    UINT16 bytes;       /* Number of bytes read or written since last progress event */
} tBTA_FTC_PROGRESS;

typedef struct
{
    UINT8  *p_realm;
    UINT8   realm_len;
    UINT8   realm_charset;
    BOOLEAN userid_required;    /* If TRUE, a user ID must be sent */
} tBTA_FTC_AUTH;

typedef struct
{
    tBIP_IMAGING_CAPS   *p_bi_caps; /* BIP imaging capabilities */
} tBTA_FTC_CAPS;

typedef struct
{
    UINT8               *p_name;    /* the image file name */
    tBIP_IMG_HDL_STR    handle;     /* The image’s handle assigned by BIP responder. */
} tBTA_FTC_THUMB;

typedef union
{
    tBTA_FTC_STATUS     status;
    tBTA_FTC_OPEN       open;
    tBTA_FTC_LIST       list;
    tBTA_FTC_PROGRESS   prog;
    tBTA_FTC_AUTH       auth;
    tBTA_FTC_CAPS       bi_caps;
    tBTA_FTC_THUMB      thumb;
    tBTA_FTC_PB_PARAM   pb;
} tBTA_FTC;

/* Client callback function */
typedef void tBTA_FTC_CBACK(tBTA_FTC_EVT event, tBTA_FTC *p_data);

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/**************************
**  Server Functions
***************************/

/*******************************************************************************
**
** Function         BTA_FtsEnable
**
** Description      Enable the file transfer server.  This function must be
**                  called before any other functions in the FTS API are called.
**                  When the enable operation is complete the callback function
**                  will be called with an BTA_FTS_ENABLE_EVT event.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtsEnable(tBTA_SEC sec_mask, const char *p_service_name,
                                  const char *p_root_path, BOOLEAN enable_authen,
                                  UINT8 realm_len, UINT8 *p_realm,
                                  tBTA_FTS_CBACK *p_cback, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_FtsDisable
**
** Description      Disable the file transfer server.  If the server is currently
**                  connected to a peer device the connection will be closed.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtsDisable(void);

/*******************************************************************************
**
** Function         BTA_FtsClose
**
** Description      Close the current connection.  This function is called if
**                  the phone wishes to close the connection before the FT
**                  client disconnects.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtsClose(void);

/*******************************************************************************
**
** Function         BTA_FtsUnauthRsp
**
** Description      Sends an OBEX authentication challenge to the connected
**                  OBEX client. Called in response to an BTA_FTS_AUTH_EVT event.
**                  Used when "enable_authen" is set to TRUE in BTA_FtsEnable().
**
**                  Note: If the "userid_required" is TRUE in the BTA_FTS_AUTH_EVT
**                        event, then p_userid is required, otherwise it is optional.
**
**                  p_password  must be less than BTA_FTS_MAX_AUTH_KEY_SIZE (16 bytes)
**                  p_userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtsAuthRsp (char *p_password, char *p_userid);

/*******************************************************************************
**
** Function         BTA_FtsAccessRsp
**
** Description      Sends a reply to an access request event (BTA_FTS_ACCESS_EVT).
**                  This call MUST be made whenever the event occurs.
**
** Parameters       oper    - operation being accessed.
**                  access  - BTA_FT_ACCESS_ALLOW or BTA_FT_ACCESS_FORBID
**                  p_name  - Full path of file to pulled or pushed.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtsAccessRsp(tBTA_FT_OPER oper, tBTA_FT_ACCESS access,
                                     char *p_name);


/**************************
**  Client Functions
***************************/

/*******************************************************************************
**
** Function         BTA_FtcEnable
**
** Description      Enable the file transfer client.  This function must be
**                  called before any other functions in the FTC API are called.
**                  When the enable operation is complete the callback function
**                  will be called with an BTA_FTC_ENABLE_EVT event.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcEnable(tBTA_FTC_CBACK *p_cback, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_FtcDisable
**
** Description      Disable the file transfer client.  If the client is currently
**                  connected to a peer device the connection will be closed.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcDisable(void);

/*******************************************************************************
**
** Function         BTA_FtcOpen
**
** Description      Open a connection to an FTP, PBAP, OPP or BIP server.
**                  If parameter services is set to use both all services,
**                  the client will attempt to connect to the device using
**                  FTP first and then PBAP, OPP, BIP.
**                  When the connection is open the callback function
**                  will be called with a BTA_FTC_OPEN_EVT.  If the connection
**                  fails or otherwise is closed the callback function will be
**                  called with a BTA_FTC_CLOSE_EVT.
**
**                  If the connection is opened with FTP profile and
**                  bta_ft_cfg.auto_file_list is TRUE , the callback
**                  function will be called with one or more BTA_FTC_LIST_EVT
**                  containing directory list information formatted in XML as
**                  described in the IrOBEX Spec, Version 1.2, section 9.1.2.3.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcOpen(BD_ADDR bd_addr, tBTA_SEC sec_mask,
                                tBTA_SERVICE_MASK services, BOOLEAN srm, UINT32 nonce);

/*******************************************************************************
**
** Function         BTA_FtcSuspend
**
** Description      Suspend the current connection to the server.
**                  This is allowed only for the sessions created by
**                  BTA_FtcConnect with nonce!=0
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcSuspend(void);

/*******************************************************************************
**
** Function         BTA_FtcClose
**
** Description      Close the current connection to the server. Aborts any
**                  active file transfer.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcClose(void);

/*******************************************************************************
**
** Function         BTA_FtcCopyFile
**
** Description      Invoke a Copy action on the server.
**                  Create a copy of p_src and name it as p_dest
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcCopyFile(const char *p_src, const char *p_dest);

/*******************************************************************************
**
** Function         BTA_FtcMoveFile
**
** Description      Invoke a Move action on the server.
**                  Move/rename p_src to p_dest
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcMoveFile(const char *p_src, const char *p_dest);

/*******************************************************************************
**
** Function         BTA_FtcSetPermission
**
** Description      Invoke a SetPermission action on the server.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcSetPermission(const char *p_src, UINT8 user, UINT8 group, UINT8 other);

/*******************************************************************************
**
** Function         BTA_FtcPutFile
**
** Description      Send a file to the connected server.
**
**                  This function can only be used when the client is connected
**                  in FTP, OPP and BIP mode.
**
** Note:            File name is specified with a fully qualified path.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcPutFile(const char *p_name, tBTA_FTC_PARAM *p_param);

/*******************************************************************************
**
** Function         BTA_FtcGetPhoneBook
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
BTA_API extern void BTA_FtcGetPhoneBook(char *p_local_name, char *p_remote_name,
                         tBTA_FTC_FILTER_MASK filter, tBTA_FTC_FORMAT format,
                         UINT16 max_list_count, UINT16 list_start_offset);

/*******************************************************************************
**
** Function         BTA_FtcGetCard
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
BTA_API extern void BTA_FtcGetCard(char *p_local_name, char *p_remote_name,
                    tBTA_FTC_FILTER_MASK filter, tBTA_FTC_FORMAT format);

/*******************************************************************************
**
** Function         BTA_FtcGetFile
**
** Description      Retrieve a file from the peer device and copy it to the
**                  local file system.
**
**                  This function can only be used when the client is connected
**                  in FTP mode.
**
** Note:            local file name is specified with a fully qualified path.
**                  Remote file name is specified in UTF-8 format.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcGetFile(char *p_local_name, char *p_remote_name);

/*******************************************************************************
**
** Function         BTA_FtcChDir
**
** Description      Change directory on the peer device.
**
**                  This function can only be used when the client is connected
**                  in FTP and PBAP mode.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcChDir(char *p_dir, tBTA_FTC_FLAG flag);

/*******************************************************************************
**
** Function         BTA_FtcAuthRsp
**
** Description      Sends a response to an OBEX authentication challenge to the
**                  connected OBEX server. Called in response to an BTA_FTC_AUTH_EVT
**                  event.
**
** Note:            If the "userid_required" is TRUE in the BTA_FTC_AUTH_EVT event,
**                  then p_userid is required, otherwise it is optional.
**
**                  p_password  must be less than BTA_FTC_MAX_AUTH_KEY_SIZE (16 bytes)
**                  p_userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcAuthRsp (char *p_password, char *p_userid);

/*******************************************************************************
**
** Function         BTA_FtcListCards
**
** Description      Retrieve a directory listing from the peer device.
**                  When the operation is complete the callback function will
**                  be called with one or more BTA_FTC_LIST_EVT events
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

BTA_API extern void BTA_FtcListCards(char *p_dir, tBTA_FTC_ORDER order, char *p_value,
                      tBTA_FTC_ATTR attribute, UINT16 max_list_count,
                      UINT16 list_start_offset);

/*******************************************************************************
**
** Function         BTA_FtcListDir
**
** Description      Retrieve a directory listing from the peer device.
**                  When the operation is complete the callback function will
**                  be called with one or more BTA_FTC_LIST_EVT events
**                  containing directory list information formatted as described
**                  in the IrOBEX Spec, Version 1.2, section 9.1.2.3.
**
**                  This function can only be used when the client is connected
**                  in FTP mode.
**
** Parameters       p_dir - Name of directory to retrieve listing of.  If NULL,
**                          the current working directory is retrieved.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcListDir(char *p_dir);

/*******************************************************************************
**
** Function         BTA_FtcAbort
**
** Description      Aborts any active Put or Get file operation.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcAbort(void);

/*******************************************************************************
**
** Function         BTA_FtcRemove
**
** Description      Remove a file or directory on the peer device.  When the
**                  operation is complete the status is returned with the
**                  BTA_FTC_REMOVE_EVT event.
**
**                  This function can only be used when the client is connected
**                  in FTP mode.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcRemove(char *p_name);

/*******************************************************************************
**
** Function         BTA_FtcMkDir
**
** Description      Create a directory on the peer device. When the operation is
**                  complete the status is returned with the BTA_FTC_MKDIR_EVT
**                  event.
**
**                  This function can only be used when the client is connected
**                  in FTP mode.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcMkDir(char *p_dir);

#define BTA_FTC_SWITCH_MODE_AUTO    0
#define BTA_FTC_SWITCH_MODE_AMP     1
#define BTA_FTC_SWITCH_MODE_BT      2

typedef UINT8 tBTA_FTC_SWITCH_MODE;

/*******************************************************************************
**
** Function         BTA_FtcSetSwitchMode
**
** Description      Set AMP switching mode
**                  BTA_FTC_SWITCH_MODE_AUTO: use auto switching algorithm
**                  BTA_FTC_SWITCH_MODE_AMP : use AMP if possible
**                  BTA_FTC_SWITCH_MODE_BT  : use BT only
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FtcSetSwitchMode(tBTA_FTC_SWITCH_MODE mode);


#ifdef __cplusplus
}
#endif

#endif /* BTA_FT_API_H */
