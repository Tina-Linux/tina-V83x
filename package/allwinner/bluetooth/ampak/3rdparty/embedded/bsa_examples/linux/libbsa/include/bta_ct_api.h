/*****************************************************************************
**
**  Name:           bta_ct_api.h
**
**  Description:    This is the public interface file for the cordless telephony
**                  terminal subsystem of BTA, Widcomm's Bluetooth application
**                  layer for mobile phones.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_CT_API_H
#define BTA_CT_API_H

#include "bta_api.h"
#include "tcs_api.h"
#include "tcs_defs.h"
#include "ctp_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* CT Callback events */
#define BTA_CT_ENABLE_EVT       0       /* CT service is enabled. */
#define BTA_CT_REGISTER_EVT     1       /* Service registered */
#define BTA_CT_OPEN_EVT         2       /* Connection to gateway opened */
#define BTA_CT_CLOSE_EVT        3       /* Connection to gateway closes */
#define BTA_CT_ALERTING_EVT     4       /* Ring on other side for orginated call */
#define BTA_CT_INCOMING_EVT     5       /* Incoming call */
#define BTA_CT_CONNECT_EVT      6       /* Call connected */
#define BTA_CT_DISCONNECT_EVT   7       /* Call discconnected  */
#define BTA_CT_INFO_EVT         8       /* Information received */
#define BTA_CT_WUG_CFG_EVT      9       /* WUG configuration event */

typedef UINT8 tBTA_CT_EVT;


#define BTA_CT_SUCCESS          0
#define BTA_CT_FAIL             1

typedef UINT8 tBTA_CT_STATUS;



#define BTA_CT_CALL_NUM_TYPE_MASK       TCS_CALL_NUM_TYPE_MASK
#define BTA_CT_CALL_NUM_UNKNOWN         TCS_CALL_NUM_UNKNOWN
#define BTA_CT_CALL_NUM_INTERNATIONAL   TCS_CALL_NUM_INTERNATIONAL
#define BTA_CT_CALL_NUM_NATIONAL        TCS_CALL_NUM_NATIONAL
#define BTA_CT_CALL_NUM_NETWORK         TCS_CALL_NUM_NETWORK
#define BTA_CT_CALL_NUM_SUBSCRIBER      TCS_CALL_NUM_SUBSCRIBER
#define BTA_CT_CALL_NUM_ABBREVIATED     TCS_CALL_NUM_ABBREVIATED

#define BTA_CT_CALL_NUM_PLAN_MASK       TCS_CALL_NUM_PLAN_MASK
#define BTA_CT_CALL_NP_UNKNOWN          TCS_CALL_NP_UNKNOWN
#define BTA_CT_CALL_NP_ISDN_TEL         TCS_CALL_NP_ISDN_TEL
#define BTA_CT_CALL_NP_DATA             TCS_CALL_NP_DATA
#define BTA_CT_CALL_NP_RESERVE          TCS_CALL_NP_RESERVE
#define BTA_CT_CALL_NP_NATIONAL         TCS_CALL_NP_NATIONAL
#define BTA_CT_CALL_NP_PRIVATE          TCS_CALL_NP_PRIVATE

#define BTA_CT_MAX_DIGITS TCS_MAX_DIGITS


/* typedef of the called party number information element */
typedef tTCS_IE_CALLED_NUM   tBTA_CT_IE_CALLED_NUM;


#define BTA_CT_CALL_PREST_MASK          TCS_CALL_PREST_MASK
#define BTA_CT_CALL_PREST_ALLOW         TCS_CALL_PREST_ALLOW
#define BTA_CT_CALL_PREST_RISTRT        TCS_CALL_PREST_RISTRT
#define BTA_CT_CALL_PREST_NAVAIL        TCS_CALL_PREST_NAVAIL
#define BTA_CT_CALL_PREST_RESERVE       TCS_CALL_PREST_RESERVE
#define BTA_CT_CALL_SCREEN_MASK         TCS_CALL_SCREEN_MASK
#define BTA_CT_CALL_SCREEN_U_NS         TCS_CALL_SCREEN_U_NS
#define BTA_CT_CALL_SCREEN_U_VP         TCS_CALL_SCREEN_U_VP
#define BTA_CT_CALL_SCREEN_U_VF         TCS_CALL_SCREEN_U_VF
#define BTA_CT_CALL_SCREEN_NET          TCS_CALL_SCREEN_NET

/* typedef of the calling party number information element */
typedef tTCS_IE_CALLING_NUM  tBTA_CT_IE_CALLING_NUM;


#define BTA_CT_CALL_CLASS_EXTERNAL      TCS_CALL_CLASS_EXTERNAL
#define BTA_CT_CALL_CLASS_INTERCOM      TCS_CALL_CLASS_INTERCOM
#define BTA_CT_CALL_CLASS_SERVICE       TCS_CALL_CLASS_SERVICE
#define BTA_CT_CALL_CLASS_EMERGENCY     TCS_CALL_CLASS_EMERGENCY
#define BTA_CT_CALL_CLASS_INTERCOM_WUG  (TCS_CALL_CLASS_EMERGENCY + 1)

/* typedef of the call class */
typedef tTCS_CALL_CLASS  tBTA_CT_CALL_CLASS;

/* typedef of the TCS Link Type */
#define BTA_CT_LINK_TYPE_SCO    TCS_BC_LINK_TYPE_SCO
#define BTA_CT_LINK_TYPE_ACL    TCS_BC_LINK_TYPE_ACL
#define BTA_CT_LINK_TYPE_NONE   TCS_BC_LINK_TYPE_NONE
#define BTA_CT_LINK_TYPE_ESCO   TCS_BC_LINK_TYPE_ESCO
typedef tTCS_BC_LINK_TYPE tBTA_CT_LINK_TYPE;

#define BTA_CT_MAX_CO_SPEC_LEN      40

/* typedef of company specific information */
typedef struct
{

    UINT8       len;  /* IE length = Length of bytes in p_info + 2 */
    UINT8       company_id1;
    UINT8       company_id2;
    UINT8       info[BTA_CT_MAX_CO_SPEC_LEN];

} tBTA_CT_IE_CO_SPEC;

#define BTA_CT_SIGNAL_EXTERNAL_CALL     TCS_SIGNAL_EXTERNAL_CALL
#define BTA_CT_SIGNAL_INTERNAL_CALL     TCS_SIGNAL_INTERNAL_CALL
#define BTA_CT_SIGNAL_CALLBACK          TCS_SIGNAL_CALLBACK

/* typedef of signal information element */
typedef tTCS_SIGNAL   tBTA_CT_SIGNAL;

#define BTA_CT_AUD_CTRL_VOL_INC         TCS_AUD_CTRL_VOL_INC
#define BTA_CT_AUD_CTRL_VOL_DEC         TCS_AUD_CTRL_VOL_DEC
#define BTA_CT_AUD_CTRL_MIC_INC         TCS_AUD_CTRL_MIC_INC
#define BTA_CT_AUD_CTRL_MIC_DEC         TCS_AUD_CTRL_MIC_DEC
#define BTA_CT_AUD_COMPY_MASK           TCS_AUD_COMPY_MASK

#define BTA_CT_MAX_AUDIO_CTL_LEN    TCS_MAX_AUDIO_CTL_LEN

/* typedef of audio control information element */
typedef tTCS_IE_AUDIO_CONTROL   tBTA_CT_IE_AUDIO_CONTROL;

/*typedef  the keypad facility information element.*/
typedef tTCS_IE_KEYPAD_FACILITY tBTA_CT_IE_KEYPAD_FACILITY;

#define BTA_CT_NO_KEYPAD_CHAR TCS_NO_KEYPAD_CHAR



#define BTA_CT_IE_CALLING_NUM_MASK          0x01
#define BTA_CT_IE_CALLED_NUM_MASK           0x02
#define BTA_CT_IE_AUDIO_CONTROL_MASK        0x04
#define BTA_CT_IE_CO_SPEC_MASK              0x08
#define BTA_CT_IE_KEYPAD_FACILITY_MASK      0x10
#define BTA_CT_IE_CAUSE_MASK                0x20

typedef UINT8 tBTA_CT_IE_MASK;

#define BTA_CT_CAUSE_UNKNOWN     TCS_CAUSE_UNKNOWN
#define BTA_CT_CCLR_UNA_NO       TCS_CCLR_UNA_NO                 /* Unassigned number                        */
#define BTA_CT_CCLR_NOR_DST      TCS_CCLR_NOR_DST                /* No route to destination                  */
#define BTA_CT_CCLR_USR_BUSY     TCS_CCLR_USR_BUSY               /* User busy                                */
#define BTA_CT_CCLR_NO_RESP      TCS_CCLR_NO_RESP                /* No user responding                       */
#define BTA_CT_CCLR_NO_ANSWER    TCS_CCLR_NO_ANSWER              /* No answer from user (user alerted)       */
#define BTA_CT_CCLR_SUB_ABSENT   TCS_CCLR_SUB_ABSENT             /* Subscriber absent                        */
#define BTA_CT_CCLR_CALL_REJ     TCS_CCLR_CALL_REJ               /* Call rejected by user                    */
#define BTA_CT_CCLR_NO_CHG       TCS_CCLR_NO_CHG                 /* Number changed                           */
#define BTA_CT_CCLR_NON_SEL_USR  TCS_CCLR_NON_SEL_USR            /* Non selected user call clearing          */
#define BTA_CT_CCLR_INV_NO_FMT   TCS_CCLR_INV_NO_FMT             /* Invalid number format(incomplete number) */
#define BTA_CT_CCLR_CHAN_UNAVAIL TCS_CCLR_CHAN_UNAVAIL           /* No circuit/channel available             */
#define BTA_CT_CCLR_RQ_CHAN_UNAV TCS_CCLR_RQ_CHAN_UNAV           /* Requested circuit/channel not available  */
#define BTA_CT_CCLR_BEAR_UNAV    TCS_CCLR_BEAR_UNAV              /* Bearer capability not presently available */
#define BTA_CT_CCLR_BEAR_UNIMP   TCS_CCLR_BEAR_UNIMP             /* Bearer capability not implemented        */
#define BTA_CT_CCLR_TIMR_EXPIRE  TCS_CCLR_TIMR_EXPIRE            /* Recovery on timer expiry */


/* typedef for call clear reason */
typedef UINT8 tBTA_CT_CLEAR_REASON;

#define BTA_MAX_WUG_MEMBERS TCS_MAX_WUG_MEMBERS

typedef tTCS_WUG_MMB_EXTN tBTA_CT_WUG_MMB_EXTN;

/* info about other WUG members */
typedef struct
{
    tBTA_CT_WUG_MMB_EXTN extn;
    BD_ADDR    addr;
    LINK_KEY    key;

} tBTA_CT_WUG_MMB_DB[BTA_MAX_WUG_MEMBERS];


typedef struct
{
    tBTA_CT_IE_MASK             ie_mask;
    tBTA_CT_IE_CALLING_NUM      clg_num_ie;
    tBTA_CT_IE_CALLED_NUM       cld_num_ie;
    tBTA_CT_IE_AUDIO_CONTROL    audio_ctl_ie;
    tBTA_CT_IE_CO_SPEC          co_spec_ie;
    tBTA_CT_IE_KEYPAD_FACILITY  keypad_fac_ie;
    tBTA_CT_CLEAR_REASON        cause;

} tBTA_CT_IE;


/* Event associated with BTA_CT_REGISTER_EVT */
typedef struct
{
    UINT8          handle;     /* Handle associated with the gateway. */
    UINT8          app_id;     /* ID associated with call to BTA_CtGwRegister() or BTA_CtIcRegister() */
} tBTA_CT_REGISTER;



/* data associated with BTA_CT_OPEN_EVT */
typedef struct
{
    UINT8      handle;
    UINT8      app_id;
    tBTA_CT_STATUS status;

} tBTA_CT_OPEN;


/* data associated with BTA_CT_CLOSE_EVT */
typedef struct
{
    UINT8      handle;
    UINT8      app_id;
} tBTA_CT_CLOSE;


/* data associated with BTA_CT_ALERTING_EVT */
typedef struct
{
    UINT8               handle;
    UINT8               app_id;
    tBTA_CT_IE         *p_ie;

} tBTA_CT_ALERT;

/* data associated with BTA_CT_INCOMING_EVT */
typedef struct
{
    UINT8                   handle;
    UINT8                   app_id;
    BD_ADDR                 bd_addr;
    tBTA_CT_CALL_CLASS      call_class;
    tBTA_CT_IE             *p_ie;
    tBTA_CT_SIGNAL          signal_value;

} tBTA_CT_INCOMING;

/* data associated with BTA_CT_CONNECT_EVT */
typedef struct
{
    UINT8       handle;
    UINT8       app_id;
    tBTA_CT_IE *p_ie;

} tBTA_CT_CONNECT;

/* data associated with BTA_CT_DISCONNECT_EVT */
typedef struct
{
    UINT8           handle;
    UINT8          app_id;
    tBTA_CT_IE     *p_ie;

} tBTA_CT_DISCONNECT;


/* data associated with BTA_CT_INFO_EVT */
typedef struct
{
    UINT8               handle;
    UINT8               app_id;
    tBTA_CT_IE         *p_ie;

} tBTA_CT_INFO;


/* data associated with BTA_CT_WUG_CFG_EVT */
typedef struct
{
    UINT8               handle;
    UINT8               app_id;
    UINT8               num_members;
    tBTA_CT_WUG_MMB_DB  *p_mmb_db;
    tBTA_CT_IE_CO_SPEC  *p_cs;

} tBTA_CT_WUG_CFG;


/* Union of all CT callback structures */
typedef union
{
    tBTA_CT_REGISTER    reg;
    tBTA_CT_OPEN        open;
    tBTA_CT_CLOSE       close;
    tBTA_CT_ALERT       alert;
    tBTA_CT_INCOMING    incoming;
    tBTA_CT_CONNECT     connect;
    tBTA_CT_DISCONNECT  disconnect;
    tBTA_CT_INFO        info;
    tBTA_CT_WUG_CFG     wug_cfg;

} tBTA_CT;

/* CT callback */
typedef void (tBTA_CT_CBACK)(tBTA_CT_EVT event, tBTA_CT *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_CtEnable
**
** Description      Enable the cordless telephony terminal. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_CT_ENABLE_EVT. This function must
**                  be called before other function in the CT API are
**                  called.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CtEnable(tBTA_SEC ct_sec_mask, tBTA_SEC ic_sec_mask, tBTA_CT_CBACK *p_cback);



/*******************************************************************************
**
** Function         BTA_CtDisable
**
** Description      Disable the cordless telephony terminal
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CtDisable(void);

/*******************************************************************************
**
** Function         BTA_CtIntercomEnable
**
** Description      This function has to be called before making an intercom call.
**                  Starts Intercom service. Other devices will be able to
**                  connect to the service.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CtIcRegister(char *p_service_name, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_CtIntercomDisable
**
** Description      Disables Intercom service
**
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CtIcDeregister(UINT8 handle);



/*******************************************************************************
**
** Function         BTA_CtGwRegister
**
** Description      Opens a connection to the Cordless telephony gateway
**                  specified by the bd_addr. When connection is open
**                  the callback is called with a BTA_CT_OPEN_EVT.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CtGwRegister( BD_ADDR bd_addr, BOOLEAN wug_member,
                        tBTA_CT_WUG_MMB_DB *p_init_wug_db, UINT8 wug_db_len, UINT8 app_id);



/*******************************************************************************
**
** Function         BTA_CtGwDeregister
**
** Description      Close the connection to the cordless gateway. When connection
**                  is closed the callback is called with BTA_CT_CLOSE_EVT
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CtGwDeregister(UINT8 handle);

/*******************************************************************************
**
** Function         BTA_CtOriginate
**
** Description      Orginates a call through a CTP gateway or initiates a call
**                  to an Intercom terminal. Before initiating a call through
**                  a CTP gateway, connection should have been opened using
**                  BTA_CtOpen.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CtOriginate(UINT8 handle, tBTA_CT_CALL_CLASS call_class,
                                    tBTA_CT_IE_CALLED_NUM *p_cld_ie,
                                    tBTA_CT_IE_CALLING_NUM *p_clg_ie,
                                    tBTA_CT_IE_CO_SPEC *p_cs);


/*******************************************************************************
**
** Function         BTA_CtAnswer
**
** Description      Answers an incoming call
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CtAnswer(UINT8 handle, tBTA_CT_IE_CO_SPEC *p_cs);

/*******************************************************************************
**
** Function         BTA_CtEnd
**
** Description      Ends a connected call. Can also be used for rejecting an
**                  incoming call
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CtEnd(UINT8 handle, tBTA_CT_CLEAR_REASON clear_reason, tBTA_CT_IE_CO_SPEC *p_cs);

/*******************************************************************************
**
** Function         BTA_CtFlash
**
** Description      Sends a hook flash (Register recall)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CtFlash(UINT8 handle);

/*******************************************************************************
**
** Function         BTA_CtDtmf
**
** Description      Sends a DTMF digit
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_CtDtmf(UINT8 handle, UINT8 dtmf, BOOLEAN key_status);


#ifdef __cplusplus
}
#endif

#endif /* BTA_CT_API_H */
