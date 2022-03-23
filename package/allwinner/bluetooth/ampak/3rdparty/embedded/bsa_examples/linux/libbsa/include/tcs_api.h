/*****************************************************************************
**
**  Name:          tcs_api.h
**
**  Description:   this file contains the TCS API definitions
**
**
**  Copyright (c) 2001-2005, Broadcom Corp., All Rights Reserved.
**  WIDCOMM Bluetooth Core. Proprietary and confidential.
******************************************************************************/
#ifndef TCS_API_H
#define TCS_API_H

#include "l2c_api.h"


/*****************************************************************************
**  Constants and Type Definitions
*****************************************************************************/

#define TCS_ALL_L2CH    0xffff
#define TCS_BCST_ANY    TCS_MAX_NUM_SIMUL_CALLS

/* Define TCS return codes
*/
enum
{
    TCS_OK,                         /* All OK                            */
    TCS_COMMAND_STARTED,
    TCS_BAD_PARAM,                  /* Missing or bad registration param */
    TCS_BAD_HANDLE,                 /* Invalid handle passed to TCS      */
    TCS_NO_RESOURCES,               /* No resources for call             */
    TCS_UNKNOWN_CALLED,             /* Unknown called party              */
    TCS_NO_L2CAP_CONN,              /* L2CAP connection not established  */
    TCS_NOT_ALLOWED,                /* Operation is not allowed in this state */
    TCS_BUSY                        /* Operation failed - TCS busy       */

};
typedef UINT8 tTCS_STATUS;

/* Define some protocol sizes
*/
#define TCS_MAX_DIGITS              24
#define TCS_WUG_NUMBER_LEN          2

/* Define an invalid handle, returned if origination fails
*/
#define TCS_INVALID_HANDLE          0xFFFF


/* Define the values of call class
*/
enum
{
    TCS_CALL_CLASS_EXTERNAL,
    TCS_CALL_CLASS_INTERCOM,
    TCS_CALL_CLASS_SERVICE,
    TCS_CALL_CLASS_EMERGENCY

};

typedef UINT8 tTCS_CALL_CLASS;

/* Define the values of the link type in the bearer capability IE
*/
enum
{
    TCS_BC_LINK_TYPE_SCO = 0,
    TCS_BC_LINK_TYPE_ACL,
    TCS_BC_LINK_TYPE_NONE,
    TCS_BC_LINK_TYPE_ESCO
};

typedef UINT8 tTCS_BC_LINK_TYPE;

/* Define the values of the value in the signal IE
*/
enum
{
    TCS_SIGNAL_EXTERNAL_CALL,
    TCS_SIGNAL_INTERNAL_CALL,
    TCS_SIGNAL_CALLBACK
};

typedef UINT8 tTCS_SIGNAL;

/* Define the possible DTMF command/response message types
*/
enum
{
    TCS_START_DTMF,
    TCS_START_DTMF_ACK,
    TCS_START_DTMF_REJ,
    TCS_STOP_DTMF,
    TCS_STOP_DTMF_ACK
};
typedef UINT8 tTCS_DTMF_MSGS;

/* Define the structure of the audio control information element.
*/
typedef struct
{
    UINT8       len;

#define TCS_AUD_CTRL_VOL_INC    0x00            /* For first byte of info */
#define TCS_AUD_CTRL_VOL_DEC    0x01
#define TCS_AUD_CTRL_MIC_INC    0x02
#define TCS_AUD_CTRL_MIC_DEC    0x03
#define TCS_AUD_COMPY_MASK      0x40

    UINT8       info[TCS_MAX_AUDIO_CTL_LEN];

} tTCS_IE_AUDIO_CONTROL;


/* Define the structure of the bearer capability information element.
*/
typedef struct
{
    tTCS_BC_LINK_TYPE   link_type;

    UINT8               info_len;

#define TCS_BR_PACKET_MASK      0x1F            /* These definitions if type = SCO */
#define TCS_BR_PACKET_HV1_EV3   0x05
#define TCS_BR_PACKET_HV2_EV4   0x06
#define TCS_BR_PACKET_HV3_EV5   0x07            /* Default */
#define TCS_BR_PACKET_DV        0x08
#define TCS_BR_USER_IFO_MASK    0xE0
#define TCS_BR_USER_IFO_CVSD    0x20            /* Default */
#define TCS_BR_USER_IFO_PCMA    0x40
#define TCS_BR_USER_IFO_PCMU    0x60
#define TCS_BR_USER_IFO_TRANS   0x80            /* Transparent */

#define TCS_BR_SCO_INFO_LEN     1
#define TCS_BR_ACL_INFO_LEN     23

    UINT8       type_info[TCS_BR_ACL_INFO_LEN];  /* If SCO, only 1 byte used */

} tTCS_IE_BEARER_CAP;

typedef struct
{
    tTCS_BC_LINK_TYPE link_type;
    UINT16 link_id;  /* SCO Handle at the BT-HCI interface or l2cap CID in case of ACL bearer */
} tTCS_BEARER_ID;


enum {
    TCS_NO_PROC_CODE,
    TCS_INCOMP_CALLED_NUM,
    TCS_PROCESSING_CALL,
    TCS_CONFIRM_CALL,
    TCS_ACCEPT_CALL
};

#define    TCS_MAKE_INBAND_TONES_AVLBL 0x0001
#define    TCS_USE_EXISTING_BEARER 0x0002
#define    TCS_NEGOTIATE_BEARER 0x0004
#define    TCS_SETUP_NEW_BEARER 0x0008
#define    TCS_RELEASE_BEARER  0x0010

typedef struct
{
    UINT16 action_mask;             /* bearer set up option */
    UINT16 bearer_handle;           /* if use of existing bearer */
    tTCS_IE_BEARER_CAP  bearer_info;/* bearer info to negotiate */
} tTCS_CALL_ACT_OPTS;

/* Define the structure of the called party number information element.
*/
typedef struct
{
#define TCS_CALL_NUM_TYPE_MASK      0x70
#define TCS_CALL_NUM_UNKNOWN        0x00
#define TCS_CALL_NUM_INTERNATIONAL  0x10
#define TCS_CALL_NUM_NATIONAL       0x20
#define TCS_CALL_NUM_NETWORK        0x30
#define TCS_CALL_NUM_SUBSCRIBER     0x40
#define TCS_CALL_NUM_ABBREVIATED    0x60

    UINT8       num_type;

#define TCS_CALL_NUM_PLAN_MASK  0x0F
#define TCS_CALL_NP_UNKNOWN     0x00
#define TCS_CALL_NP_ISDN_TEL    0x01
#define TCS_CALL_NP_DATA        0x03
#define TCS_CALL_NP_RESERVE     0x04
#define TCS_CALL_NP_NATIONAL    0x08
#define TCS_CALL_NP_PRIVATE     0x09

    UINT8       num_plan;

    UINT8       num_len;
    UINT8       num[TCS_MAX_DIGITS];

} tTCS_IE_CALLED_NUM;


/* Define the structure of the calling party number information element.
*/
typedef struct
{
    UINT8       num_type;           /* See definitions for called number */
    UINT8       num_plan;           /* See definitions for called number */

#define TCS_CALL_PREST_MASK     0x60
#define TCS_CALL_PREST_ALLOW    0x00
#define TCS_CALL_PREST_RISTRT   0x20
#define TCS_CALL_PREST_NAVAIL   0x40
#define TCS_CALL_PREST_RESERVE  0x60
#define TCS_CALL_SCREEN_MASK    0x03
#define TCS_CALL_SCREEN_U_NS    0x00        /* user provided, not screened */
#define TCS_CALL_SCREEN_U_VP    0x01        /* user provided, verified & passed */
#define TCS_CALL_SCREEN_U_VF    0x02        /* user provided, verified & failed */
#define TCS_CALL_SCREEN_NET     0x03        /* network provided */

    UINT8       pres_screen;

    UINT8       num_len;
    UINT8       num[TCS_MAX_DIGITS];

} tTCS_IE_CALLING_NUM;


/* Define the structure of the keypad facility information element.
*/
typedef UINT8 tTCS_IE_KEYPAD_FACILITY;
#define TCS_NO_KEYPAD_CHAR 0xFF

/* Define the structure of the company specific information element.
*/
typedef struct
{
    UINT8       len;  /* IE length = Length of bytes in p_info + 2 */
    UINT8       company_id1;
    UINT8       company_id2;

    UINT8       *p_info;

} tTCS_IE_CO_SPEC;


/*****************************************************************************
** Callback Types and Definitions
*****************************************************************************/

/* Incoming call indication callback prototype. Parameters are
**              TCS Handle
**              Call Class
**              Sending Complete Flag
**              Pointer to Bearer Capability IE (or NULL if none)
**              Signal
**              Pointer to Calling Party Number IE (or NULL if none)
**              Pointer to Called  Party Number IE (or NULL if none)
**              Pointer to Company Specific Information IE (or NULL if none)
*/

enum {
    TCS_SETUP,                  /* Incoming Call */
    TCS_BEARER_NEG_REQ,         /* Bearer Negotiaition for a call */
    TCS_INBAND_TONES_AVLBL,     /* Inband-tones/announcement indication */
    TCS_CLI_INFO,               /* Caller ID information */
    TCS_SETUP_ACK,              /* Need more digits */
    TCS_CALL_PROCEEDING,        /* Call is proceeding */
    TCS_INFORMATION,            /* Information such as digits etc. */
    TCS_ALERTING,               /* Alerting */
    TCS_CONNECT,                /* Connect */
    TCS_DISCONNECT,             /* Disconnect */
    TCS_RELEASED,               /* Release/Release Complete */
    TCS_DTMF_IND,
    TCS_REGISTER_RECALL,
    TCS_BEARER_COMPLETE,        /* Returned when TCS is given to setup the bearer */
    TCS_BEARER_DISCONNECTED,
    TCS_TIMEOUT,                /* Call clearing initiated due to timeout */
    TCS_BEARER_FAILURE,
    TCS_AUDIO_CTRL,             /* Audio Control */
    TCS_PAGE_NO_RESP
};

typedef struct
{
    BD_ADDR             remote_addr;
    tTCS_CALL_CLASS     call_class;
    BOOLEAN             digits_complete;
    tTCS_IE_BEARER_CAP  *p_bearer_cap_ie;
    tTCS_SIGNAL         *signal_value;
    tTCS_IE_CALLED_NUM  *p_cld_num_ie;
    tTCS_IE_CO_SPEC     *p_company_specific_ie;
} tTCS_SETUP_DATA;


/* Call disconnected callback prototype. Parameters are
**              TCS Handle
**              Cause (or zero if no cause received)
**              BOOLEAN if inband tones are available
**              Pointer to Company Specific Information IE (or NULL if none)
*/
typedef struct
{
    UINT8            cause;
    tTCS_IE_CO_SPEC  *p_company_specific_ie;
} tTCS_END_CALL_DATA;

/* Call released callback prototype. Parameters are
**              TCS Handle
**              Pointer to Company Specific Information IE (or NULL if none)
*/
typedef struct
{
    tTCS_IE_CO_SPEC *p_company_specific_ie;
} tTCS_RELEASED_DATA;

/* Remote alerting callback prototype. Parameters are
**              TCS Handle
**              Pointer to Company Specific Information IE (or NULL if none)
*/
typedef struct
{
    tTCS_IE_CO_SPEC *p_company_specific_ie;
} tTCS_REM_ALERTING_DATA;

/* DTMF digit control callback prototype. Parameters are
**              TCS Handle
**              Parameter
**
** If type == TCS_START_DTMF, TCS_START_DTMF_ACK, TCS_STOP_DTMF or TCS_STOP_DTMF_ACK
** them parameter is keypad digit.
**
** If type == TCS_START_DTMF_REJ, then parameter is reject cause (if present)
** or zero if not present.
**
*/
typedef struct
{
    tTCS_DTMF_MSGS  type;
    UINT8           param;
} tTCS_DTMF_DATA;

/* Received Info packet callback prototype. Parameters are
**              TCS Handle
**              Pointer to Keypad Facility IE (or NULL if none)
**              Pointer to Called Party Number IE (or NULL if none)
**              Pointer to Audio Control IE (or NULL if none)
**              Pointer to Company Specific Information IE (or NULL if none)
*/
typedef struct
{
    BOOLEAN                  digits_complete;
    tTCS_IE_KEYPAD_FACILITY  *p_keypad_fac_ie;
    tTCS_IE_CALLING_NUM      *p_clg_num_ie;
    tTCS_IE_CALLED_NUM       *p_cld_num_ie;
    tTCS_IE_AUDIO_CONTROL    *p_audio_ctl_ie;
    tTCS_IE_CO_SPEC *p_company_specific_ie;
} tTCS_INFO_IND_DATA;


typedef void (tTCS_CC_CB) (UINT16 cc_handle, UINT8 event_code, void * message);
typedef tL2CAP_APPL_INFO tTCS_L2C_CBACKS ;

typedef UINT16 (tTCS_PAGE_RESP_CB) ( UINT8 event, UINT16 handle, BD_ADDR addr, tTCS_IE_CO_SPEC *p_company_specific_ie ) ;

typedef struct
{
    UINT8 digit1;
    UINT8 digit2;
} tTCS_WUG_MMB_EXTN;

enum {
    TCS_WUG_EMPTY,      /* this entry is unused*/
    TCS_WUG_OUTSYNC,    /* out of sync */
    TCS_WUG_CONFIGURED
};
typedef UINT8 tTCS_WUG_STATUS;

typedef struct
{
    tTCS_WUG_STATUS status;

    tTCS_WUG_MMB_EXTN extn;
    BD_ADDR    addr;

    LINK_KEY    keys[TCS_MAX_WUG_MEMBERS];/* Link key at index n corresponds to the WUG member at index n in the tTCS_WUG_MST_DB. The link key for self and empty entries are ignored */

}    tTCS_WUG_MST_DB[TCS_MAX_WUG_MEMBERS];


typedef struct
{
    tTCS_WUG_MMB_EXTN extn;
    BD_ADDR    addr;
    LINK_KEY    key;
}    tTCS_WUG_MMB_DB[TCS_MAX_WUG_MEMBERS];

enum {
    TCS_FIMA_SUGGESTED,
    TCS_FIMA_REJECTED,
    TCS_FIMA_PAGE,
    TCS_FIMA_SCAN,
    TCS_FIMA_TIMEOUT,
    TCS_FIMA_INIT_SUCCESS /* Provides UINT16 data containing cc handle for the call */
};
typedef UINT8 tTCS_FIMS_CBACK_CODE;

typedef struct
{
    UINT8 cause;
    tTCS_IE_CO_SPEC *p_cs;
} tTCS_FIMA_REJECTED_DATA;

typedef struct
{
    BD_ADDR page_addr;
    BOOLEAN detach;
    UINT16  clk_offset;
    tTCS_IE_CO_SPEC *p_cs;
} tTCS_FIMA_PAGE_DATA;


typedef void (tTCS_ACCESS_CBACK) (BD_ADDR addr, tTCS_IE_CO_SPEC *p_cs);
typedef void (tTCS_ACCESS_RES_CBACK) (UINT8 result, tTCS_IE_CO_SPEC *p_cs);
typedef void (tTCS_WUGCFG_CBACK) (UINT8 num_members, tTCS_WUG_MMB_DB *p_mmb_db, tTCS_IE_CO_SPEC *p_cs);
typedef void (tTCS_FIMA_CBACK) (UINT8 code, void *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         TCS_Init
**
** Description      This function is called once at startup to initialize
**                  all the TCS structures. Called once during stack startup.
**                  Must be called before accessing any TCS (or TCS profile)
**                  API functions.
**
** Returns          void
**
*******************************************************************************/
TCS_API extern void TCS_Init (void);


/*******************************************************************************
**
** Function         TCS_RegisterCbacks
**
** Description      This function can be called to register callback functions.
**
** Returns          TCS_BAD_PARAM if certain callbacks are not provided
**                  TCS_OK otherwise
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_RegisterCbacks (tTCS_CC_CB *cc_cb,
                                              tTCS_L2C_CBACKS *l2c_cbacks,
                                              tTCS_PAGE_RESP_CB *map_fn);


/*******************************************************************************
**
** Function         TCS_AddCCResources
**
** Description      When higher layer has setup an l2cap channel that can be used
**                  as a signalling resource by TCS, it calls this to perform the op.
**
** Returns          TCS_NO_RESOURCES if there's no space left for the resource
**                  TCS_OK otherwise
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_AddCCResources (BD_ADDR addr, UINT16 l2cap_cid);


/*******************************************************************************
**
** Function         TCS_RemoveCCResources
**
** Description      When higher layer has disconnected an l2cap channel that was
**                  added as a signalling resource it should call this to remove
**                  the channel.
**
** Returns          TCS_NO_RESOURCES if there's no such resource in the system
**                  TCS_OK otherwise
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_RemoveCCResources (BD_ADDR addr, UINT16 l2cap_cid);


/*******************************************************************************
**
** Function         TCS_OriginateCall
**
** Description      Initiates a call with a given device (bd_addr) This call will
**                  use an l2cap channel added for the device as a sig resource.
**
** Returns          TCS_NO_L2CAP_CONN if no l2cap resource has been added for the
**                                    device.
**                  TCS_NO_RESOURCES if the resources rquired to make call are not
**                                   available.
**                  TCS_OK otherwise
**                  Also returns the handle to the call in tcs_cc_handle if TCS_OK
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_OriginateCall (BD_ADDR addr,
                                              tTCS_IE_CALLED_NUM *p_cld_ie,
                                              BOOLEAN digits_complete,
                                              tTCS_IE_CALLING_NUM *p_clg_ie,
                                              tTCS_IE_BEARER_CAP *p_bearer,
                                              tTCS_CALL_CLASS call_class,
                                              tTCS_IE_CO_SPEC *p_cs,
                                              UINT16 *tcs_cc_handle);


/*******************************************************************************
**
** Function         TCS_PageAll
**
** Description      This can be used to send SETUP message over broadcast channel
**
** Returns          TCS_NO_RESOURCES, TCS_BAD_PARAM or TCS_OK.
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_PageAll (tTCS_IE_CALLED_NUM *p_cld_ie,
                                        BOOLEAN digits_complete,
                                        tTCS_IE_CALLING_NUM *p_clg_ie,
                                        tTCS_IE_BEARER_CAP *p_bearer,
                                        tTCS_CALL_CLASS call_class,
                                        tTCS_IE_CO_SPEC *p_cs,
                                        UINT16 *tcs_bc_handle);


/*******************************************************************************
**
** Function         TCS_EndPage
**
** Description      This is used to initiate termination of a brodcast call.
**
** Returns          TCS_BAD_HANDLE, TCS_COMMAND_STARTED or TCS_OK.
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_EndPage (UINT16 bc_handle, UINT8 clear_reason,
                                        tTCS_IE_CO_SPEC *p_cs);


/*******************************************************************************
**
** Function         TCS_EndCall
**
** Description      This is used to initiate termination of call as well as
**                  clearing of call whose termination is initiated from peer.
**
** Returns          TCS_BAD_HANDLE, TCS_COMMAND_STARTED (in case a bearer is
**                  being set-up) or TCS_OK.
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_EndCall (UINT16 cc_handle, UINT8 clear_reason,
                                        tTCS_CALL_ACT_OPTS *action_info,
                                        tTCS_IE_CO_SPEC *p_cs);


/*******************************************************************************
**
** Function         TCS_ProceedOnIncomingCall
**
** Description      This is used to answer an incoming call as well as
**                  proceed on the call.
**
** Returns          TCS_BAD_HANDLE, TCS_COMMAND_STARTED (in case a bearer is
**                  being set-up), TCS_NOT_ALLOWED, TCS_NO_RESOURCES or TCS_OK.
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_ProceedOnIncomingCall (UINT16 cc_handle,
                                                      UINT8 proceed_code,
                                                      tTCS_CALL_ACT_OPTS *action_info,
                                                      tTCS_IE_CO_SPEC *p_cs);


/*******************************************************************************
**
** Function         TCS_SendInformation
**
** Description      General purpose call to send INFORMATION to peer.
**
** Returns          TCS_BAD_HANDLE or TCS_OK.
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_SendInformation (UINT16 cc_handle,
                                                BOOLEAN complete_num,
                                                UINT8 keypad_char,
                                                tTCS_IE_CALLING_NUM *p_clg_ie,
                                                tTCS_IE_CALLED_NUM *p_cn,
                                                tTCS_IE_AUDIO_CONTROL *p_ac,
                                                tTCS_IE_CO_SPEC *p_cs);


/*******************************************************************************
**
** Function         TCS_ConfirmConnect
**
** Description      This is the hook for the upper layer to confirm that they
**                  have connected the speech/voice path with the SCO link and
**                  be able to send a company specific IE in the connect-ack
**                  message.
**
** Returns          TCS_BAD_HANDLE, TCS_NOT_ALLOWED, TCS_NO_RESOURCES or TCS_OK.
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_ConfirmConnect (UINT16 cc_handle,
                                               tTCS_IE_CO_SPEC *p_cs);


/*******************************************************************************
**
** Function         TCS_FindAddrByHandle
**
** Description      This call gets the upper layer the BD-ADDR of the remote
**                  device.
**
** Returns          TCS_BAD_HANDLE, or TCS_OK.
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_FindAddrByHandle (BD_ADDR rem_addr,
                                                 UINT16 cc_handle);


/*******************************************************************************
**
** Function         TCS_DTMFCommand
**
** Description      This is to send a DTMF message after the call is in active
**                  state.
**
** Returns          TCS_BAD_HANDLE, TCS_NOT_ALLOWED, TCS_BAD_PARAM or TCS_OK.
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_DTMFCommand (UINT16 cc_handle, tTCS_DTMF_MSGS code,
                                            UINT8 param);


/*******************************************************************************
**
** Function         TCS_RegisterRecall
**
** Description      Call this to indicate input of further digits or other action.
**                  Also referred as 'hook flash'.
**
** Returns          TCS_BAD_HANDLE, TCS_NOT_ALLOWED, TCS_BAD_PARAM or TCS_OK.
**
*******************************************************************************/
TCS_API extern tTCS_STATUS TCS_RegisterRecall (UINT16 cc_handle);


/*******************************************************************************
**
** Function         TCS_SetupAsWugMaster
**
** Description      It stores the callback and copies the WUG database
**                  information.  Sets the callback tcs_wug_mkey_comp_cback
**                  with BTM to be called when enable/disable master key
**                  command is completed.
**
** Returns
**                  TCS_OK
**
*******************************************************************************/
TCS_API extern UINT16 TCS_SetupAsWugMaster (tTCS_WUG_MST_DB *init_mst_db,
                                            tTCS_ACCESS_CBACK *ac_cb);


/*******************************************************************************
**
** Function         TCS_WugSetInfoSugCompSpecIE
**
** Description      Whenever INFO_SUG message is sent this company spec IE will be sent.
**
**
*******************************************************************************/
TCS_API extern UINT16 TCS_WugSetInfoSugCompSpecIE(tTCS_IE_CO_SPEC *p_cs);

/*******************************************************************************
**
** Function         TCS_SetupAsWugMember
**
** Description      Stores the callbacks and copies the WUG database information.
**
** Returns
**                  TCS_OK
**
*******************************************************************************/
TCS_API extern UINT16 TCS_SetupAsWugMember (tTCS_WUG_MMB_DB *init_mmb_db,
                                            UINT8 num_members,
                                            tTCS_WUGCFG_CBACK *cfg_cb,
                                            tTCS_FIMA_CBACK *fima_cb);


/*******************************************************************************
**
** Function         TCS_AddWugMember
**
** Description      Adds the WUG member to WUG-database. Generates link keys with
**                  other WUG members. It marks all the WUG members in the
**                  WUG-database as TCS_WUG_OUTSYNC. If any of the members is
**                  connected it sends out the command to disable master link
**                  key. (The configuration distribution is initiated from the
**                  tcs_wug_mkey_comp_cback.)
**
** Returns          FALSE if there's memory to store the member
**                  TRUE indicates success
**
*******************************************************************************/
TCS_API extern BOOLEAN TCS_AddWugMember (BD_ADDR mmb_addr,
                                         tTCS_WUG_MMB_EXTN *p_extn);


/*******************************************************************************
**
** Function         TCS_DeleteWugMember
**
** Description      Removes the WUG member from WUG-database. It marks all the
**                  WUG members in the WUG-database as TCS_WUG_OUTSYNC. If any of
**                  the members is connected it sends out the command to disable
**                  master link key.
**
** Returns          FALSE if there's no such member
**                  TRUE otherwise
**
*******************************************************************************/
TCS_API extern BOOLEAN TCS_DeleteWugMember (BD_ADDR mmb_addr);


/*******************************************************************************
**
** Function         TCS_AddWugResource
**
** Description      This API stores l2cap cid in order to send WUG related
**                  messages on the channel. If called by WUG master, for the
**                  device that is a WUG member and if not marked TCS_WUG_OUTSYNC;
**                  it sends out the command to disable master link key; else if
**                  not WUG member, it sends out the command to enable the
**                  master link key.
**
** Returns          FALSE if there's no space for new resource
**                  TRUE otherwise
**
*******************************************************************************/
TCS_API extern BOOLEAN TCS_AddWugResource (BD_ADDR remote_addr,
                                           UINT16 primary_cid);


/*******************************************************************************
**
** Function         TCS_RemoveWugResource
**
** Description      Clears the l2cap cid that is used for WUG messaging with
**                  the device.  If called by WUG master, it sends out the
**                  command to disable master link key. This would give chance
**                  for configuration distribution to parked members and would
**                  also change the master link key when a device disconnects
**                  from the Master.
**
**                  If called by member it checks if it got disconnected from
**                  master due to FIMA detach for scanning; if so, it issues
**                  BTM_SeConnectability to start page-scanning.
**
** Returns          FALSE if there's no such resource in the system
**                  TRUE otherwise
**
*******************************************************************************/
TCS_API extern BOOLEAN TCS_RemoveWugResource (BD_ADDR remote_addr);


/*******************************************************************************
**
** Function         TCS_WugObtainAccessReq
**
** Description      Sends Obtain Access Rights message to the WUG-master and
**                  starts timer T401.
**
** Returns          TCS_NO_L2CAP_CONN if not connected to WUG-master
**                  TCS_BAD_PARAM if callback is not provided.
**                  TCS_NOT_ALLOWED if obtain access request is in process.
**                  TCS_NO_RESOURCES if there's no buffer to send out the message
**                  TCS_OK when successful
**
*******************************************************************************/
TCS_API extern UINT16 TCS_WugObtainAccessReq (tTCS_ACCESS_RES_CBACK *cb,
                                              tTCS_IE_CO_SPEC *p_cs);


/*******************************************************************************
**
** Function         TCS_WugAccessRightsReply
**
** Description      Sends Obtain Access Rights Accept or Reject message to the
**                  member.
**                  In case of accept makes a call to TCS_AddWugMember.
**
** Returns          TCS_NO_L2CAP_CONN if not connected to member
**                  TCS_NO_RESOURCES if there's no buffer to send out the message
**                  TCS_OK when successful
**
*******************************************************************************/
TCS_API extern UINT16 TCS_WugAccessRightsReply (BD_ADDR addr,BOOLEAN result,
                                                tTCS_WUG_MMB_EXTN *p_extn,
                                                tTCS_IE_CO_SPEC *p_cs);


/*******************************************************************************
**
** Function         TCS_WugInitiateFima
**
** Description      Sends LISTEN_REQUEST message to the master. Starts timer T404.
**
** Returns          TCS_NO_L2CAP_CONN if not connected to WUG-master
**                  TCS_BAD_PARAM if target extension cannot be found in WUG databse.
**                  TCS_NOT_ALLOWED if FIMA procedure is in process.
**                  TCS_NO_RESOURCES if there's no buffer to send out the message
**                  TCS_OK when successful
**
*******************************************************************************/
TCS_API extern UINT16 TCS_WugInitiateFima (tTCS_WUG_MMB_EXTN  *p_extn,
                                           BOOLEAN detach_flag,
                                           tTCS_IE_BEARER_CAP *pb,
                                           tTCS_IE_CO_SPEC *p_cs);


/*******************************************************************************
**
** Function         TCS_WugReplyFima
**
** Description      If result is set to FALSE sends LISTEN-REJECT to the master.
**                  In case of result set to TRUE, reads clock offset, sends
**                   LISTEN ACCEPT, starts the timer T406.
**                  If detach flag is set to true, it will ensure the LISTEN
**                  ACCEPT message is delivered by starting a guard timer (1 sec).
**                  The detach is performed from the master either when the guard
**                  timeout occurs or the master sends an (Widcomm specific)
**                  acknowledgement for the LISTEN ACCEPT message.
**
** Returns          TCS_NO_L2CAP_CONN if not connected to WUG-master
**                  TCS_NOT_ALLOWED if FIMA has not been suggested.
**                  TCS_NO_RESOURCES if there's no buffer to send out the message
**                  TCS_OK when successful
**
*******************************************************************************/
TCS_API extern UINT16 TCS_WugReplyFima (BOOLEAN result,BOOLEAN detach_flag,
                                        tTCS_IE_CO_SPEC *p_cs);


/*******************************************************************************
**
** Function         TCS_WugFimaSetupResult
**
** Description      This will call TCS_AddCCResources if result is set to TRUE.
**                  If the FIMA status is TCS_WUG_FIMA_PAGER, it sends out a SETUP
**                  message by calling TCS_OrigninateCall.
**
** Returns          None
**
*******************************************************************************/
TCS_API extern void TCS_WugFimaSetupResult (BOOLEAN result, BD_ADDR remote_addr,
                                            UINT16 l2cap_cid);


/*******************************************************************************
**
** Function         TCS_SetTraceLevel
**
** Description      This function sets the trace level for TCS.  If called with
**                  a value of 0xFF, it simply returns the current trace level.
**
** Returns          The new or current trace level
**
*******************************************************************************/
TCS_API extern UINT8 TCS_SetTraceLevel (UINT8 new_level);

#ifdef __cplusplus
}
#endif

#endif  /* TCS_API_H */
