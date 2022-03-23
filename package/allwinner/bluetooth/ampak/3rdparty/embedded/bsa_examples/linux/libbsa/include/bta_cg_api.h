/*****************************************************************************
**
**  Name:           bta_cg_api.h
**
**  Description:    This is the public interface file for the cordless telephony
**                  gateway subsystem of BTA.
**
**  Copyright (c) 2005, Broadcom, All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_CG_API_H
#define BTA_CG_API_H

#include "bta_api.h"
#include "tcs_api.h"
#include "tcs_defs.h"
#include "ctp_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* CG Callback events */
#define BTA_CG_ENABLE_EVT       0       /* CG service is enabled. */
#define BTA_CG_REGISTER_EVT     1       /* Service registered */
#define BTA_CG_TL_OPEN_EVT      2       /* Connection from TL */
#define BTA_CG_TL_CLOSE_EVT     3       /* Disconnection from TL */
#define BTA_CG_DEREGISTER_EVT   4       /* Service deregistered */

#define BTA_CG_CC_INCOMING_ALERT_EVT   5    /* Incoming call alert from TL */
#define BTA_CG_CC_CONNECT_EVT          6    /* Call connect event */
#define BTA_CG_CC_OUTGOING_EVT         7    /* Outgoing call indication from TL */
#define BTA_CG_CC_OUTGOING_INFO_EVT    8    /* Outgoing call information from TL for post dial*/
#define BTA_CG_CC_DISCONNECT_EVT       9    /* Call discconnected  */
#define BTA_CG_CC_DTMF_EVT             10   /* DTMF event*/
#define BTA_CG_CC_INCOMING_SETUP_EVT   11   /* Incoming call setup event to pass tcs handle out*/

typedef UINT8 tBTA_CG_EVT;


/* typedef of the TCS Link Type */
#define BTA_CG_LINK_TYPE_SCO    TCS_BC_LINK_TYPE_SCO
#define BTA_CG_LINK_TYPE_ACL    TCS_BC_LINK_TYPE_ACL
#define BTA_CG_LINK_TYPE_NONE   TCS_BC_LINK_TYPE_NONE
#define BTA_CG_LINK_TYPE_ESCO   TCS_BC_LINK_TYPE_ESCO

typedef tTCS_BC_LINK_TYPE tBTA_CG_LINK_TYPE;


#define BTA_CG_CALL_NUM_TYPE_MASK       TCS_CALL_NUM_TYPE_MASK
#define BTA_CG_CALL_NUM_UNKNOWN         TCS_CALL_NUM_UNKNOWN
#define BTA_CG_CALL_NUM_INTERNATIONAL   TCS_CALL_NUM_INTERNATIONAL
#define BTA_CG_CALL_NUM_NATIONAL        TCS_CALL_NUM_NATIONAL
#define BTA_CG_CALL_NUM_NETWORK         TCS_CALL_NUM_NETWORK
#define BTA_CG_CALL_NUM_SUBSCRIBER      TCS_CALL_NUM_SUBSCRIBER
#define BTA_CG_CALL_NUM_ABBREVIATED     TCS_CALL_NUM_ABBREVIATED

#define BTA_CG_CALL_NUM_PLAN_MASK       TCS_CALL_NUM_PLAN_MASK
#define BTA_CG_CALL_NP_UNKNOWN          TCS_CALL_NP_UNKNOWN
#define BTA_CG_CALL_NP_ISDN_TEL         TCS_CALL_NP_ISDN_TEL
#define BTA_CG_CALL_NP_DATA             TCS_CALL_NP_DATA
#define BTA_CG_CALL_NP_RESERVE          TCS_CALL_NP_RESERVE
#define BTA_CG_CALL_NP_NATIONAL         TCS_CALL_NP_NATIONAL
#define BTA_CG_CALL_NP_PRIVATE          TCS_CALL_NP_PRIVATE

#define BTA_CG_MAX_DIGITS TCS_MAX_DIGITS


/* typedef of the called party number information element */
typedef tTCS_IE_CALLED_NUM   tBTA_CG_IE_CALLED_NUM;


#define BTA_CG_CALL_PREST_MASK          TCS_CALL_PREST_MASK
#define BTA_CG_CALL_PREST_ALLOW         TCS_CALL_PREST_ALLOW
#define BTA_CG_CALL_PREST_RISTRT        TCS_CALL_PREST_RISTRT
#define BTA_CG_CALL_PREST_NAVAIL        TCS_CALL_PREST_NAVAIL
#define BTA_CG_CALL_PREST_RESERVE       TCS_CALL_PREST_RESERVE
#define BTA_CG_CALL_SCREEN_MASK         TCS_CALL_SCREEN_MASK
#define BTA_CG_CALL_SCREEN_U_NS         TCS_CALL_SCREEN_U_NS
#define BTA_CG_CALL_SCREEN_U_VP         TCS_CALL_SCREEN_U_VP
#define BTA_CG_CALL_SCREEN_U_VF         TCS_CALL_SCREEN_U_VF
#define BTA_CG_CALL_SCREEN_NET          TCS_CALL_SCREEN_NET

/* typedef of the calling party number information element */
typedef tTCS_IE_CALLING_NUM  tBTA_CG_IE_CALLING_NUM;


#define BTA_CG_CALL_CLASS_EXTERNAL      TCS_CALL_CLASS_EXTERNAL
#define BTA_CG_CALL_CLASS_INTERCOM      TCS_CALL_CLASS_INTERCOM
#define BTA_CG_CALL_CLASS_SERVICE       TCS_CALL_CLASS_SERVICE
#define BTA_CG_CALL_CLASS_EMERGENCY     TCS_CALL_CLASS_EMERGENCY
#define BTA_CG_CALL_CLASS_INTERCOM_WUG  (TCS_CALL_CLASS_EMERGENCY + 1)

/* typedef of the call class */
typedef tTCS_CALL_CLASS  tBTA_CG_CALL_CLASS;

#define BTA_CG_MAX_CO_SPEC_LEN      40

/* typedef of company specific information */
typedef struct
{

    UINT8       len;  /* IE length = Length of bytes in p_info + 2 */
    UINT8       company_id1;
    UINT8       company_id2;
    UINT8       info[BTA_CG_MAX_CO_SPEC_LEN];

} tBTA_CG_IE_CO_SPEC;

#define BTA_CG_SIGNAL_EXTERNAL_CALL     TCS_SIGNAL_EXTERNAL_CALL
#define BTA_CG_SIGNAL_INTERNAL_CALL     TCS_SIGNAL_INTERNAL_CALL
#define BTA_CG_SIGNAL_CALLBACK          TCS_SIGNAL_CALLBACK

/* typedef of signal information element */
typedef tTCS_SIGNAL   tBTA_CG_SIGNAL;
typedef tTCS_DTMF_MSGS tBTA_CG_DTMF_MSGS;

#define BTA_CG_AUD_CGRL_VOL_INC         TCS_AUD_CGRL_VOL_INC
#define BTA_CG_AUD_CGRL_VOL_DEC         TCS_AUD_CGRL_VOL_DEC
#define BTA_CG_AUD_CGRL_MIC_INC         TCS_AUD_CGRL_MIC_INC
#define BTA_CG_AUD_CGRL_MIC_DEC         TCS_AUD_CGRL_MIC_DEC
#define BTA_CG_AUD_COMPY_MASK           TCS_AUD_COMPY_MASK

#define BTA_CG_MAX_AUDIO_CGL_LEN    TCS_MAX_AUDIO_CGL_LEN

/* typedef of audio control information element */
typedef tTCS_IE_AUDIO_CONTROL   tBTA_CG_IE_AUDIO_CONTROL;

/*typedef  the keypad facility information element.*/
typedef tTCS_IE_KEYPAD_FACILITY tBTA_CG_IE_KEYPAD_FACILITY;

#define BTA_CG_NO_KEYPAD_CHAR TCS_NO_KEYPAD_CHAR


#define BTA_CG_IE_CALLING_NUM_MASK          0x01
#define BTA_CG_IE_CALLED_NUM_MASK           0x02
#define BTA_CG_IE_AUDIO_CONTROL_MASK        0x04
#define BTA_CG_IE_CO_SPEC_MASK              0x08
#define BTA_CG_IE_KEYPAD_FACILITY_MASK      0x10
#define BTA_CG_IE_CAUSE_MASK                0x20

typedef UINT8 tBTA_CG_IE_MASK;

#define BTA_CG_CAUSE_UNKNOWN     TCS_CAUSE_UNKNOWN
#define BTA_CG_CCLR_UNA_NO       TCS_CCLR_UNA_NO                 /* Unassigned number                        */
#define BTA_CG_CCLR_NOR_DST      TCS_CCLR_NOR_DST                /* No route to destination                  */
#define BTA_CG_CCLR_USR_BUSY     TCS_CCLR_USR_BUSY               /* User busy                                */
#define BTA_CG_CCLR_NO_RESP      TCS_CCLR_NO_RESP                /* No user responding                       */
#define BTA_CG_CCLR_NO_ANSWER    TCS_CCLR_NO_ANSWER              /* No answer from user (user alerted)       */
#define BTA_CG_CCLR_SUB_ABSENT   TCS_CCLR_SUB_ABSENT             /* Subscriber absent                        */
#define BTA_CG_CCLR_CALL_REJ     TCS_CCLR_CALL_REJ               /* Call rejected by user                    */
#define BTA_CG_CCLR_NO_CHG       TCS_CCLR_NO_CHG                 /* Number changed                           */
#define BTA_CG_CCLR_NON_SEL_USR  TCS_CCLR_NON_SEL_USR            /* Non selected user call clearing          */
#define BTA_CG_CCLR_INV_NO_FMT   TCS_CCLR_INV_NO_FMT             /* Invalid number format(incomplete number) */
#define BTA_CG_CCLR_CHAN_UNAVAIL TCS_CCLR_CHAN_UNAVAIL           /* No circuit/channel available             */
#define BTA_CG_CCLR_RQ_CHAN_UNAV TCS_CCLR_RQ_CHAN_UNAV           /* Requested circuit/channel not available  */
#define BTA_CG_CCLR_BEAR_UNAV    TCS_CCLR_BEAR_UNAV              /* Bearer capability not presently available */
#define BTA_CG_CCLR_BEAR_UNIMP   TCS_CCLR_BEAR_UNIMP             /* Bearer capability not implemented        */
#define BTA_CG_CCLR_TIMR_EXPIRE  TCS_CCLR_TIMR_EXPIRE            /* Recovery on timer expiry */


/* typedef for call clear reason */
typedef UINT8 tBTA_CG_CLEAR_REASON;

#define BTA_MAX_WUG_MEMBERS TCS_MAX_WUG_MEMBERS

typedef tCTP_GW_INFO tBTA_CG_INFO;

typedef tCTP_GW_WUG_PARAMS tBTA_CG_WUG_PARAMS;


typedef struct
{
    tBTA_CG_IE_MASK             ie_mask;
    tBTA_CG_IE_CALLING_NUM      clg_num_ie;
    tBTA_CG_IE_CALLED_NUM       cld_num_ie;
    tBTA_CG_IE_AUDIO_CONTROL    audio_ctl_ie;
    tBTA_CG_IE_CO_SPEC          co_spec_ie;
    tBTA_CG_IE_KEYPAD_FACILITY  keypad_fac_ie;
    tBTA_CG_CLEAR_REASON        cause;

} tBTA_CG_IE;


/* Event associated with BTA_CG_REGISTER_EVT */
typedef struct
{
    UINT8          app_id;     /* ID associated with call to BTA_CGRegister() */
} tBTA_CG_REGISTER;

/* Event associated with BTA_CG_DEREGISTER_EVT */
typedef struct
{
    UINT8          app_id;     /* ID associated with call to BTA_CGDeRegister() */
} tBTA_CG_DEREGISTER;

/* Event associated with BTA_CG_TL_CONNECTED_FULL_EVT */
typedef struct
{
    UINT8      index;          /* index associated with the connected TL */
    UINT8      app_id;
    BD_ADDR    bd_addr;        /* BD address of the TL */
} tBTA_CG_TL_OPEN;

/* Event associated with BTA_CG_TL_DISCONNECT_EVT */
typedef struct
{
    UINT8      index;
    UINT8      app_id;
    BD_ADDR    bd_addr;        /* BD address of the TL */
} tBTA_CG_TL_CLOSE;

/* Data associated with BTA_CG_CC_INCOMING_ALERT_EVT */
typedef struct
{
    UINT8               index;
    UINT8               app_id;
    UINT8               line_num;
    UINT16              cc_handle;
    tBTA_CG_IE          *p_ie;

} tBTA_CG_CC_INCOMING_ALERT;

/* Data associated with BTA_CG_CC_OUTGOING_EVT */
typedef struct
{
    UINT8                   index;
    UINT8                   app_id;
    UINT8                   line_num;
    UINT16                  cc_handle;
    BD_ADDR                 bd_addr;
    tBTA_CG_CALL_CLASS      call_class;
    tBTA_CG_IE              *p_ie;
    tBTA_CG_SIGNAL          signal_value;

} tBTA_CG_CC_OUTGOING;

/* Data associated with BTA_CG_CC_CONNECT */
typedef struct
{
    UINT8       index;
    UINT8       app_id;
    UINT8       line_num;
    UINT16      cc_handle;
    tBTA_CG_IE *p_ie;

} tBTA_CG_CC_CONNECT;

/* Data associated with BTA_CG_CC_DISCONNECT */
typedef struct
{
    UINT8           index;
    UINT8           app_id;
    UINT8           line_num;
    UINT16          cc_handle;
    tBTA_CG_IE      *p_ie;

} tBTA_CG_CC_DISCONNECT;


typedef struct
{
    UINT8               index;
    UINT8               app_id;
    UINT8               line_num;
    UINT16              cc_handle;
    tBTA_CG_IE          *p_ie;

} tBTA_CG_CC_OUTGOING_INFO;


/* data type for BTA_CG_CC_DTMF_EVT */
typedef struct
{
    UINT8               index;
    UINT8               app_id;
    UINT8               line_num;
    UINT16              cc_handle;
    tBTA_CG_DTMF_MSGS   dtmf_type;
    UINT8               dtmf_param;

} tBTA_CG_CC_DTMF;

/* Data associated with BTA_CG_CC_INCOMING_SETUP_EVT to pass tcs handle out to app */
typedef struct
{
    UINT8               index;
    UINT8               app_id;
    UINT8               line_num;
    UINT16              cc_handle;

} tBTA_CG_CC_INCOMING_SETUP;

/* Union of all CG callback structures */
typedef union
{
    tBTA_CG_REGISTER      reg;
    tBTA_CG_TL_OPEN       tl_open;
    tBTA_CG_TL_CLOSE      tl_close;
    tBTA_CG_DEREGISTER    dereg;

    tBTA_CG_CC_INCOMING_ALERT  incoming_alert;
    tBTA_CG_CC_CONNECT         connect_cc;
    tBTA_CG_CC_OUTGOING        outgoing;
    tBTA_CG_CC_OUTGOING_INFO   outgoing_info;
    tBTA_CG_CC_DISCONNECT      disconnect_cc;
    tBTA_CG_CC_DTMF            dtmf;
    tBTA_CG_CC_INCOMING_SETUP  incoming_setup;

} tBTA_CG;

/* CG callback */
typedef void (tBTA_CG_CBACK)(tBTA_CG_EVT event, tBTA_CG *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_CGEnable
**
** Description      Enable the cordless telephony gateway. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_CG_ENABLE_EVT. This function must
**                  be called before other funCGion in the CG API are
**                  called.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CGEnable(UINT8 cg_sec_mask, tBTA_CG_CBACK *p_cback);



/*******************************************************************************
**
** Function         BTA_CGDisable
**
** Description      Disable the cordless telephony gateway
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CGDisable(void);


/*******************************************************************************
**
** Function         BTA_CGRegister
**
** Description      BTA CG Register CTP gateway
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CGRegister(tBTA_CG_INFO *gw_info, tBTA_CG_WUG_PARAMS *wug_params);


/*******************************************************************************
**
** Function         BTA_CGDeregister
**
** Description      BTA CG Deregister CTP gateway
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CGDeregister(UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_CGOriginate
**
** Description      Orginates a call through a CG gateway to TL
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CGOriginateIncomingCall(UINT8 tl_index, tBTA_CG_CALL_CLASS call_class,
                                    tBTA_CG_IE_CALLED_NUM *p_cld_ie,
                                    tBTA_CG_IE_CALLING_NUM *p_clg_ie,
                                    tBTA_CG_IE_CO_SPEC *p_cs);


/*******************************************************************************
**
** Function         BTA_CGAnswer
**
** Description      Answers an call
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CGAnswerOutgoing(UINT8 cc_handle, tBTA_CG_IE_CO_SPEC *p_cs);

/*******************************************************************************
**
** Function         BTA_CGEnd
**
** Description      Ends a connected call.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CGEnd(UINT8 cc_handle, tBTA_CG_CLEAR_REASON clear_reason, tBTA_CG_IE_CO_SPEC *p_cs);



/*******************************************************************************
**
** Function         BTA_CGGetTLAddr
**
** Description      Sends a TL BD address from TL index
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CGGetTLAddr (UINT8 tl_index, BD_ADDR addr);

BTA_API extern void show_bta_cg_scb(void);


#ifdef __cplusplus
}
#endif

#endif /* BTA_CG_API_H */
