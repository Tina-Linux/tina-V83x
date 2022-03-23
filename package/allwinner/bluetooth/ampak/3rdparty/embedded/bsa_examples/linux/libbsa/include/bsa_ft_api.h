/*****************************************************************************
 **
 **  Name:           bsa_ft_api.h
 **
 **  Description:    This is the public interface file for File Transfer server
 **                  part of the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_FT_API_H
#define BSA_FT_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"

/*****************************************************************************
 **  Constants and data types
 *****************************************************************************/

/**************************
 **  Common Definitions
 ***************************/
/* Access response types */
#define BSA_FT_ACCESS_ALLOW     0   /* Allow the requested operation */
#define BSA_FT_ACCESS_FORBID    1   /* Disallow the requested operation */

typedef UINT8 tBSA_FT_ACCESS;

#define BSA_FTS_NO_RESP_REQ 0
#define BSA_FTS_RESP_REQ    1
typedef UINT8 tBSA_FT_RESPONSE;

/* Auto accept feature mask type*/
#define BSA_FT_ENABLE_AUTO_ACCEPT_PUT        0x01   /*Enable auto-accept of Put file requests*/
#define BSA_FT_ENABLE_AUTO_ACCEPT_GET        0x02   /*Enable auto-accept of Get file requests*/
#define BSA_FT_ENABLE_AUTO_ACCEPT_DEL_FILE   0x04   /*Enable auto-accept of Delete file requests*/
#define BSA_FT_ENABLE_AUTO_ACCEPT_DEL_DIR    0x08   /*Enable auto-accept of Delete folder requests*/
#define BSA_FT_ENABLE_AUTO_ACCEPT_CHG_DIR    0x10   /*Enable auto-accept of Change folder requests*/
#define BSA_FT_ENABLE_AUTO_ACCEPT_MK_DIR     0x20   /*Enable auto-accept of Make folder requests*/
#define BSA_FT_ENABLE_AUTO_ACCEPT            0x80   /*Set to make no chnages to the auto accept settings*/

typedef UINT8 tBSA_FT_ACCESS_MASK;

/* Access event operation types */
#define BSA_FT_OPER_DEFAULT     0   /* Default mode */
#define BSA_FT_OPER_PUT         1   /* Request is a PUT file */
#define BSA_FT_OPER_GET         2   /* Request is a GET file */
#define BSA_FT_OPER_DEL_FILE    3   /* Request is a DELETE file */
#define BSA_FT_OPER_DEL_DIR     4   /* Request is a DELETE folder */
#define BSA_FT_OPER_CHG_DIR     5   /* Request is a Change Folder */
#define BSA_FT_OPER_MK_DIR      6   /* Request is a Make Folder */

typedef UINT8 tBSA_FT_OPER;

/*
 * FILE TRANSFER SERVER DEFINITIONS
 */

/* BSA FTS callback events */
typedef enum {
    BSA_FTS_OPEN_EVT, /* Connection Open*/
    BSA_FTS_CLOSE_EVT, /* Connection Closed */
    BSA_FTS_AUTH_EVT, /* Authorization Event */
    BSA_FTS_ACCESS_EVT, /* Access requested Event */
    BSA_FTS_PROGRESS_EVT, /* Progress Event */
    BSA_FTS_PUT_CMPL_EVT, /* Put complete Event */
    BSA_FTS_GET_CMPL_EVT, /* Get complete Event */
    BSA_FTS_DEL_CMPL_EVT,
/* Delete complete Event */
} tBSA_FTS_EVT;

/**************************
 **  Server Definitions
 ***************************/
/* Extra Debug Code */
#ifndef BSA_FTS_DEBUG
#define BSA_FTS_DEBUG           FALSE
#endif

#define BSA_FTS_OK              0
#define BSA_FTS_FAIL            1
typedef UINT8 tBSA_FTS_STATUS;

#define BSA_FT_FILENAME_MAX         256
#define BSA_FT_PASSWORD_LEN_MAX     20
#define BSA_FT_USER_ID_LEN_MAX      20
#define BSA_FT_REALM_LEN_MAX               100
#define BSA_FTS_SERVICE_NAME_LEN_MAX        150
#define BSA_FTS_ROOT_PATH_LEN_MAX           255
#define BSA_FTC_LIST_DIR_LEN_MAX           512

/* Structure associated with BSA_FTS_xxx_CMPL_EVT */
typedef struct {
    char name[BSA_FT_FILENAME_MAX]; /* file or folder name. */
    tBSA_FTS_STATUS status;
} tBSA_FTS_OBJECT_EVT;

typedef struct {
    int file_size; /* Total size of file (BSA_FS_LEN_UNKNOWN if unknown) */
    int bytes; /* Number of bytes read or written since last progress event */
} tBSA_FTS_PROGRESS_EVT;

typedef struct {

    UINT8 userid[BSA_FT_USER_ID_LEN_MAX];
    UINT8 userid_len;
    BOOLEAN userid_required; /* TRUE if user ID is required in response (re-challenged)  */
} tBSA_FTS_AUTH_EVT;

typedef struct {
    char name[BSA_FT_FILENAME_MAX]; /* file name with fully qualified path */
    int size; /* file size */
    BD_NAME dev_name; /* Name of device, "" if unknown */
    tBSA_FT_OPER oper; /* operation (put) */
    BD_ADDR bd_addr; /* Address of device */
    tBSA_FT_RESPONSE resp_required;
} tBSA_FTS_ACCESS_EVT;

typedef union {
    tBSA_FTS_PROGRESS_EVT prog;
    tBSA_FTS_AUTH_EVT auth;
    tBSA_FTS_ACCESS_EVT access;
    tBSA_FTS_OBJECT_EVT obj;
    BD_ADDR bd_addr;
} tBSA_FTS_MSG;

/* Server callback function */
typedef void tBSA_FTS_CBACK(tBSA_FTS_EVT event, tBSA_FTS_MSG *p_data);

/* Structures used to Enable FTS */
typedef struct {
    tBSA_SEC_AUTH sec_mask;
    char service_name[BSA_FTS_SERVICE_NAME_LEN_MAX];
    char root_path[BSA_FTS_ROOT_PATH_LEN_MAX];
    BOOLEAN enable_authen;
    char realm[BSA_FT_REALM_LEN_MAX];
    tBSA_FTS_CBACK *p_cback;
} tBSA_FTS_ENABLE;

typedef struct {
    UINT8 dummy;
} tBSA_FTS_DISABLE;

typedef struct {
    UINT8 dummy;
} tBSA_FTS_CLOSE;

typedef struct {
    char password[BSA_FT_PASSWORD_LEN_MAX];
    char userid[BSA_FT_USER_ID_LEN_MAX];
} tBSA_FTS_AUTH_RSP;

typedef struct {
    tBSA_FT_OPER oper;
    tBSA_FT_ACCESS access;
    tBSA_FT_ACCESS_MASK access_mask;
    char p_name[BSA_FT_FILENAME_MAX];
} tBSA_FTS_ACCESS;

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
 ** Function         BSA_FtsEnable
 **
 ** Description      Enable the file transfer server.  This function must be
 **                  called before any other functions in the FTS API are called.
 **                  When the enable operation is complete the callback function
 **                  will be called with an BSA_FTS_ENABLE_EVT event.
 **
 **
 ** Returns          int: Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_FtsEnable(tBSA_FTS_ENABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtsEnableInit
 **
 ** Description      Initialize the tBSA_FTS_ENABLE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtsEnableInit(tBSA_FTS_ENABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtsDisable
 **
 ** Description      Disable the file transfer server.  If the server is currently
 **                  connected to a peer device the connection will be closed.
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtsDisable(tBSA_FTS_DISABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtsDisableInit
 **
 ** Description      Initialize the tBSA_FTS_DISABLE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtsDisableInit(tBSA_FTS_DISABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtsClose
 **
 ** Description      Close the current connection.  This function is called if
 **                  the phone wishes to close the connection before the transfer
 **                  is completed.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtsClose(tBSA_FTS_CLOSE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtsCloseInit
 **
 ** Description      Initialize the tBSA_FTS_CLOSE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtsCloseInit(tBSA_FTS_CLOSE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtsAuthRsp
 **
 ** Description      Sends an OBEX authentication challenge to the connected
 **                  OBEX client. Called in response to an BSA_FTS_AUTH_EVT event.
 **                  Used when "enable_authen" is set to TRUE in BSA_FtsEnable().
 **
 **                  Note: If the "userid_required" is TRUE in the BSA_FTS_AUTH_EVT
 **                        event, then p_userid is required, otherwise it is optional.
 **
 **                  p_password  must be less than BSA_FTS_MAX_AUTH_KEY_SIZE (16 bytes)
 **                  p_userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
 **
 ** Returns          int: Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtsAuthRsp(tBSA_FTS_AUTH_RSP *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtsAuthRspInit
 **
 ** Description      Initialize the tBSA_FTS_AUTH_RSP structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtsAuthRspInit(tBSA_FTS_AUTH_RSP *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtsAccess
 **
 ** Description      Sends a reply to an access request event (BSA_FTS_ACCESS_EVT).
 **                  This call MUST be made whenever the event occurs.
 **
 ** Parameters       oper    - operation being accessed.
 **                  access  - BSA_FT_ACCESS_ALLOW or BSA_FT_ACCESS_FORBID
 **                  p_name  - Full path of file to pulled or pushed.
 **
 ** Returns          int: Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtsAccess(tBSA_FTS_ACCESS *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtsAccessInit
 **
 ** Description      Initialize the tBSA_FTS_ACCESS structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtsAccessInit(tBSA_FTS_ACCESS *p_req);



/**************************
**  Client definition
***************************/

/* BSA FTC callback events */
typedef enum {
    BSA_FTC_OPEN_EVT       ,   /* Connection to peer is open. */
    BSA_FTC_CLOSE_EVT      ,   /* Connection to peer closed. */
    BSA_FTC_AUTH_EVT       ,   /* Request for Authentication key and user id */
    BSA_FTC_LIST_EVT       ,   /* Event contains a directory entry (tBSA_FTC_LIST) */
    BSA_FTC_PROGRESS_EVT   ,   /* Number of bytes read or written so far */
    BSA_FTC_PUTFILE_EVT    ,   /* File Put complete */
    BSA_FTC_GETFILE_EVT    ,   /* File Get complete */
    BSA_FTC_THUMBNAIL_EVT   ,  /* BIP responder requests for the thumbnail version */
    BSA_FTC_CHDIR_EVT      ,   /* Change Directory complete */
    BSA_FTC_MKDIR_EVT      ,   /* Make Directory complete */
    BSA_FTC_REMOVE_EVT     ,   /* Remove File/Directory complete */
    BSA_FTC_PHONEBOOK_EVT  ,   /* Report the Application Parameters for BSA_FtcGetPhoneBook response */
    BSA_FTC_COPY_EVT       ,   /* Copy File complete */
    BSA_FTC_MOVE_EVT       ,   /* Move File complete */
    BSA_FTC_PERMISSION_EVT    /* Set File permission complete */
/* Delete complete Event */
} tBSA_FTC_EVT;

#define BSA_FTC_FLAG_NONE       0
#define BSA_FTC_FLAG_BACKUP     1

typedef UINT8 tBSA_FTC_FLAG;

/* Structure associated with BSA_FTS_xxx_EVT */
typedef struct {
    char name[BSA_FT_FILENAME_MAX]; /* file or folder name. */
    tBSA_FTS_STATUS status;
} tBSA_FTC_OBJECT_EVT;

typedef struct {
    int file_size; /* Total size of file (BSA_FS_LEN_UNKNOWN if unknown) */
    int bytes; /* Number of bytes read or written since last progress event */
} tBSA_FTC_PROGRESS_EVT;

typedef struct {
    UINT16 len; /* Total size number of byte */
    UINT16 num_entry; /* number of entries listed */
    UINT8 data[BSA_FTC_LIST_DIR_LEN_MAX]; /* number of entries listed */
    tBSA_STATUS status;
    BOOLEAN final;
    BOOLEAN is_xml;
} tBSA_FTC_LIST_EVT;

typedef struct {

    char  realm[BSA_FT_REALM_LEN_MAX];
    UINT8 realm_len;
    UINT8 realm_charset;
    BOOLEAN userid_required; /* TRUE if user ID is required in response (re-challenged)  */
} tBSA_FTC_AUTH_EVT;

typedef union {
    tBSA_FTC_PROGRESS_EVT prog;
    tBSA_FTC_AUTH_EVT auth;
    tBSA_FTC_OBJECT_EVT obj;
    tBSA_FTC_LIST_EVT list;
    BD_ADDR bd_addr;
    tBSA_STATUS status;
} tBSA_FTC_MSG;

/* Server callback function */
typedef void tBSA_FTC_CBACK(tBSA_FTC_EVT event, tBSA_FTC_MSG *p_data);


typedef struct {
    tBSA_FTC_CBACK *p_cback;
} tBSA_FTC_ENABLE;

typedef struct {
    UINT16  dummy;
} tBSA_FTC_DUMMY;

typedef tBSA_FTC_DUMMY tBSA_FTC_DISABLE;
typedef tBSA_FTC_DUMMY tBSA_FTC_SUSPEND;
typedef tBSA_FTC_DUMMY tBSA_FTC_CLOSE;
typedef tBSA_FTC_DUMMY tBSA_FTC_ABORT;

typedef struct {
    BD_ADDR bd_addr;
    tBSA_SEC_AUTH sec_mask;
    tBSA_SERVICE_MASK services;
    BOOLEAN srm;
    UINT32 nonce;
    BOOLEAN is_xml;  /* list format in case BSA is configured to list after the open */
} tBSA_FTC_OPEN;

typedef struct {
    char src[BSA_FT_FILENAME_MAX]; /* file name with fully qualified path */
    char dest[BSA_FT_FILENAME_MAX]; /* file name with fully qualified path */
} tBSA_FTC_COPY;

typedef tBSA_FTC_COPY tBSA_FTC_MOVE;

typedef struct {
    char src[BSA_FT_FILENAME_MAX];
    UINT8 user;
    UINT8 group;
    UINT8 other;
} tBSA_FTC_SET_PERMISION;

typedef struct {
    char name[BSA_FT_FILENAME_MAX];
} tBSA_FTC_PUT;

typedef struct {
    char local_name[BSA_FT_FILENAME_MAX];
    char remote_name[BSA_FT_FILENAME_MAX];
} tBSA_FTC_GET;

typedef struct {
    char dir[BSA_FT_FILENAME_MAX];
} tBSA_FTC_MK_DIR;

typedef struct {
    char dir[BSA_FT_FILENAME_MAX];
    tBSA_FTC_FLAG flag;
    BOOLEAN is_xml;  /* list format in case BSA is configured to list after the change dir */
} tBSA_FTC_CH_DIR;

typedef struct {
    char dir[BSA_FT_FILENAME_MAX];
    BOOLEAN is_xml;
} tBSA_FTC_LIST_DIR;

typedef tBSA_FTS_AUTH_RSP tBSA_FTC_AUTH_RSP;
typedef tBSA_FTC_PUT tBSA_FTC_REMOVE;

/**************************
**  Client Functions
***************************/
/*******************************************************************************
 **
 ** Function         BSA_FtcEnableInit
 **
 ** Description      Initialize the tBSA_FTC_ENABLE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcEnableInit(tBSA_FTC_ENABLE *p_req);

/*******************************************************************************
**
** Function         BSA_FtcEnable
**
** Description      Enable the file transfer client.  This function must be
**                  called before any other functions in the FTC API are called.
**                  When the enable operation is complete the callback function
**                  will be called with an BSA_FTC_ENABLE_EVT event.
**
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcEnable(tBSA_FTC_ENABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcDisableInit
 **
 ** Description      Initialize the tBSA_FTC_DISABLE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcDisableInit(tBSA_FTC_DISABLE *p_req);

/*******************************************************************************
**
** Function         BSA_FtcDisable
**
** Description      Disable the file transfer client.  If the client is currently
**                  connected to a peer device the connection will be closed.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcDisable(tBSA_FTC_DISABLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcOpenInit
 **
 ** Description      Initialize the tBSA_FTC_OPEN structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcOpenInit(tBSA_FTC_OPEN *p_req);

/*******************************************************************************
**
** Function         BSA_FtcOpen
**
** Description      Open a connection to an FTP, PBAP, OPP or BIP server.
**                  If parameter services is set to use both all services,
**                  the client will attempt to connect to the device using
**                  FTP first and then PBAP, OPP, BIP.
**                  When the connection is open the callback function
**                  will be called with a BSA_FTC_OPEN_EVT.  If the connection
**                  fails or otherwise is closed the callback function will be
**                  called with a BSA_FTC_CLOSE_EVT.
**
**                  If the connection is opened with FTP profile and
**                  bta_ft_cfg.auto_file_list is TRUE , the callback
**                  function will be called with one or more BSA_FTC_LIST_EVT
**                  containing directory list information formatted in XML as
**                  described in the IrOBEX Spec, Version 1.2, section 9.1.2.3.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcOpen(tBSA_FTC_OPEN *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcCloseInit
 **
 ** Description      Initialize the tBSA_FTC_CLOSE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcCloseInit(tBSA_FTC_CLOSE *p_req);

/*******************************************************************************
**
** Function         BSA_FtcClose
**
** Description      Close the current connection to the server. Aborts any
**                  active file transfer.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcClose(tBSA_FTC_CLOSE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcCopyInit
 **
 ** Description      Initialize the tBSA_FTC_AUTH_RSP structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcCopyInit(tBSA_FTC_COPY *p_req);

/*******************************************************************************
**
** Function         BSA_FtcCopy
**
** Description      Invoke a Copy action on the server.
**                  Create a copy of p_src and name it as p_dest
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcCopy(tBSA_FTC_COPY *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcMoveInit
 **
 ** Description      Initialize the tBSA_FTC_MOVE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcMoveInit(tBSA_FTC_MOVE *p_req);

/*******************************************************************************
**
** Function         BSA_FtcMove
**
** Description      Invoke a Move action on the server.
**                  Move/rename p_src to p_dest
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcMove(tBSA_FTC_MOVE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcSetPermissionInit
 **
 ** Description      Initialize the tBSA_FTC_SET_PERMISION structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcSetPermissionInit(tBSA_FTC_SET_PERMISION *p_req);

/*******************************************************************************
**
** Function         BSA_FtcSetPermission
**
** Description      Invoke a SetPermission action on the server.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcSetPermission(tBSA_FTC_SET_PERMISION *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcPutFileInit
 **
 ** Description      Initialize the tBSA_FTC_PUT structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcPutFileInit(tBSA_FTC_PUT *p_req);

/*******************************************************************************
**
** Function         BSA_FtcPutFile
**
** Description      Send a file to the connected server.
**
**                  This function can only be used when the client is connected
**                  in FTP, OPP and BIP mode.
**
** Note:            File name is specified with a fully qualified path.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcPutFile(tBSA_FTC_PUT *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcGetFileInit
 **
 ** Description      Initialize the tBSA_FTC_GET structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcGetFileInit(tBSA_FTC_GET *p_req);
/*******************************************************************************
**
** Function         BSA_FtcGetFile
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
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcGetFile(tBSA_FTC_GET *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcChDirInit
 **
 ** Description      Initialize the tBSA_FTC_CH_DIR structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcChDirInit(tBSA_FTC_CH_DIR *p_req);

/*******************************************************************************
**
** Function         BSA_FtcChDir
**
** Description      Change directory on the peer device.
**
**                  This function can only be used when the client is connected
**                  in FTP and PBAP mode.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcChDir(tBSA_FTC_CH_DIR *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcAuthRspInit
 **
 ** Description      Initialize the tBSA_FTC_AUTH_RSP structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcAuthRspInit (tBSA_FTC_AUTH_RSP *p_req);

/*******************************************************************************
**
** Function         BSA_FtcAuthRsp
**
** Description      Sends a response to an OBEX authentication challenge to the
**                  connected OBEX server. Called in response to an BSA_FTC_AUTH_EVT
**                  event.
**
** Note:            If the "userid_required" is TRUE in the BSA_FTC_AUTH_EVT event,
**                  then p_userid is required, otherwise it is optional.
**
**                  p_password  must be less than BSA_FTC_MAX_AUTH_KEY_SIZE (16 bytes)
**                  p_userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcAuthRsp (tBSA_FTC_AUTH_RSP *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcListDirInit
 **
 ** Description      Initialize the tBSA_FTC_LIST_DIR structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcListDirInit(tBSA_FTC_LIST_DIR *p_req);

/*******************************************************************************
**
** Function         BSA_FtcListDir
**
** Description      Retrieve a directory listing from the peer device.
**                  When the operation is complete the callback function will
**                  be called with one or more BSA_FTC_LIST_EVT events
**                  containing directory list information formatted as described
**                  in the IrOBEX Spec, Version 1.2, section 9.1.2.3.
**
**                  This function can only be used when the client is connected
**                  in FTP mode.
**
** Parameters       p_dir - Name of directory to retrieve listing of.  If NULL,
**                          the current working directory is retrieved.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcListDir(tBSA_FTC_LIST_DIR *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcRemoveInit
 **
 ** Description      Initialize the tBSA_FTC_REMOVE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcRemoveInit(tBSA_FTC_REMOVE *p_req);

/*******************************************************************************
**
** Function         BSA_FtcRemove
**
** Description      Remove a file or directory on the peer device.  When the
**                  operation is complete the status is returned with the
**                  BSA_FTC_REMOVE_EVT event.
**
**                  This function can only be used when the client is connected
**                  in FTP mode.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcRemove(tBSA_FTC_REMOVE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_FtcMkDirInit
 **
 ** Description      Initialize the tBSA_FTC_MK_DIR structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcMkDirInit(tBSA_FTC_MK_DIR *p_req);

/*******************************************************************************
**
** Function         BSA_FtcMkDir
**
** Description      Create a directory on the peer device. When the operation is
**                  complete the status is returned with the BSA_FTC_MKDIR_EVT
**                  event.
**
**                  This function can only be used when the client is connected
**                  in FTP mode.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI  tBSA_STATUS BSA_FtcMkDir(tBSA_FTC_MK_DIR *p_req);



#ifdef __cplusplus
}
#endif






#endif /* BSA_FT_API_H */
