/*****************************************************************************
**
**  Name:          icp_api.h
**
**  Description:   This file contains the Intercom profile (ICP) API
**                 definitions.
**
**  Copyright (c) 2001-2005, Broadcom Corp., All Rights Reserved.
**  WIDCOMM Bluetooth Core. Proprietary and confidential.
******************************************************************************/
#ifndef ICP_API_H
#define ICP_API_H

#include "profiles_api.h"
#include "tcs_api.h"

/*****************************************************************************
**  Constants
*****************************************************************************/
/* Profile Version Numbers */
#define ICP_VERSION_1_0     0x0100
#define ICP_VERSION_1_2     0x0120      /* includes eSCO */

/*****************************************************************************
**  Type Definitions
*****************************************************************************/

/* define ICP error codes - lower value error codes are reserved for TCS */
enum
{
    ICP_SUCCESS         = BT_PASS, /* 0 */
    ICP_CMD_STARTED     = 0x11,
    ICP_BUSY            = 0x12,
    ICP_INVALID_PARAM   = 0x13,
    ICP_TCS_ERROR       = 0x14,
    ICP_BTM_ERROR       = 0x15,
    ICP_NO_RESOURCES    = 0x16,
    ICP_NOT_ALLOWED     = 0x17,
    ICP_ERR_SDP_REG     = 0x18,
    ICP_ERR_SDP_PROTO   = 0x19,
    ICP_ERR_SDP_CLASSID = 0x1a,
    ICP_ERR_SDP_ATTR    = 0x1b,
    ICP_ERR_SDP_PROFILE = 0x1d,
    ICP_TIMEOUT         = 0x1e,
    ICP_NOT_AVAILABLE   = 0x1f,
    ICP_L2CAP_ERROR     = 0x20,
    ICP_BAD_STATE       = 0x21,
    ICP_ERR_PROFILE     = 0x22,
    ICP_UNSUPPORTED     = 0x23,
    ICP_ERR_MAX         = 0x24
};
typedef UINT8 tICP_STATUS;


/* define status for tICP_STS_CBACK  */
enum
{                           /* message pointer */
    ICP_STS_CONNECTED,      /* NULL */
    ICP_STS_CALL_IND,       /* tTCS_SETUP_DATA */
    ICP_STS_CALL_ALERTING,  /* tTCS_IE_CO_SPEC */
    ICP_STS_CALL_INFO,      /* tTCS_INFO_IND_DATA */
    ICP_STS_CALL_CONNECT,   /* tTCS_IE_CO_SPEC */
    ICP_STS_CALL_END,       /* tTCS_END_CALL_DATA */
    ICP_STS_CALL_RELEASED,  /* tTCS_END_CALL_DATA */
    ICP_STS_CALL_FAILED,    /* NULL */
    ICP_STS_DISCONNECTED,   /* NULL */
    ICP_NUMBER_STS
};
typedef UINT8 tICP_STS_EVT;

/* callback function for status */
typedef void (tICP_STS_CBACK) (UINT8 event, void *message);

typedef struct
{
    UINT8               clear_reason;
    tTCS_IE_CO_SPEC     *p_cs;
} tICP_END_INFO;

typedef struct
{
    BD_ADDR addr;
    tTCS_IE_BEARER_CAP  *p_bear;
    tTCS_IE_CO_SPEC     *p_cs;
} tICP_CALL_INFO;

typedef struct
{
    UINT8 keypad_char;
    tTCS_IE_CALLING_NUM *p_clg_ie;
    tTCS_IE_CALLED_NUM  *p_cn;
    tTCS_IE_AUDIO_CONTROL *p_ac;
    tTCS_IE_CO_SPEC     *p_cs;
} tICP_INFO;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
**
** Function         ICP_Init
**
** Description      This function is called at stack startup to allocate memory
**                  for the control block (if using dynamic memory), and
**                  initializes it.
**
**                  NOTE:  This function MUST be called prior to calling any
**                         other ICP API function.
**
** Returns          void
**
******************************************************************************/
ICP_API extern void ICP_Init(void);

/*****************************************************************************
**
** Function     ICP_SetSecurity
**
** Description  Sets the security level as input for ICP
**
** Returns      ICP_SUCCESS, if successful. Otherwise, error code.
**
******************************************************************************/
ICP_API extern tICP_STATUS ICP_SetSecurity (UINT8 *, UINT8);

/*****************************************************************************
**
** Function     ICP_Register
**
** Description  This function registers an application with ICP to set up
**              the call-control related callbacks.
**
** Returns      ICP_SUCCESS, if successful. Otherwise, error code.
**
******************************************************************************/
ICP_API extern tICP_STATUS ICP_Register(tICP_STS_CBACK *, UINT8 *, UINT8 *);

/*****************************************************************************
**
** Function     ICP_Deregister
**
** Description  Application should call this function to de-register from ICP.
**
** Returns      ICP_NOT_ALLOWED, if there is still ACTIVE calls.
**              ICP_SUCCESS, otherwise.
**
******************************************************************************/
ICP_API extern tICP_STATUS ICP_Deregister (BOOLEAN);

/*****************************************************************************
**
** Function     ICP_AcceptCall
**
** Description  This function is to accept or reject the invitation to begin
**              an intercom call.
**
** Returns      ICP_SUCCESS, if successful. Otherwise, error code.
**
******************************************************************************/
ICP_API extern tICP_STATUS ICP_AcceptCall(void);

/*****************************************************************************
**
** Function     ICP_ConfirmConnect
**
** Description  This function is to accept or reject the invitation to begin
**              an intercom call.
**
** Returns      ICP_SUCCESS, if successful. Otherwise, error code.
**
******************************************************************************/
ICP_API extern tICP_STATUS ICP_ConfirmConnect(tTCS_IE_CO_SPEC *);

/*****************************************************************************
**
** Function     ICP_ConfirmCall
**
** Description  This function is used to send alerting.
**
** Returns      ICP_SUCCESS, if successful. Otherwise, error code.
**
******************************************************************************/
ICP_API extern tICP_STATUS ICP_ConfirmCall(void);

/*****************************************************************************
**
** Function     ICP_EndCall
**
** Description  This function is used to end or reject a call.
**
** Returns      ICP_SUCCESS, if successful. Otherwise, error code.
**
******************************************************************************/
ICP_API extern tICP_STATUS ICP_EndCall (tICP_END_INFO *);

/*****************************************************************************
**
** Function     ICP_InitiateCall
**
** Description  This API is used to make an Intercom call to a remote device.
**
** Returns      ICP_SUCCESS, if successful. Otherwise, error code.
**
******************************************************************************/
ICP_API extern tICP_STATUS ICP_InitiateCall( tICP_CALL_INFO *);

/*****************************************************************************
**
** Function     ICP_SendInfo
**
** Description  This function is used to send information.
**
** Returns      ICP_SUCCESS, if successful. Otherwise, error code.
**
******************************************************************************/
ICP_API extern tICP_STATUS ICP_SendInfo(tICP_INFO *);

/*****************************************************************************
**
**  Function        ICP_SetTraceLevel
**
**  Description     This function changes the trace level for debugging.
**
**  Returns         void
**
*****************************************************************************/
ICP_API extern void ICP_SetTraceLevel(UINT8 level);

#ifdef __cplusplus
}
#endif

#endif  /* ICP_API_H */
