/*****************************************************************************
**
**  Name:          ctp_api.h
**
**  Description:   this file contains the CTP API definitions
**
**  Copyright (c) 2001-2005, Broadcom Corp., All Rights Reserved.
**  WIDCOMM Bluetooth Core. Proprietary and confidential.
******************************************************************************/
#ifndef CTP_API_H
#define CTP_API_H

#include "profiles_api.h"
#include "tcs_api.h"

/*****************************************************************************
**  Constants
*****************************************************************************/
/* Profile Version Numbers */
#define CTP_VERSION_1_0     0x0100
#define CTP_VERSION_1_2     0x0120      /* includes eSCO */

/* CTP line numbers */
#define CTP_LINE_UNKNOWN     0
#define CTP_LINE_INTERCOM    0x40
#define CTP_LINE_DATA_EXCHG  0x80

/* external network attributes */
#define CTP_EXT_NW_ANY      0
#define CTP_EXT_NW_PSTN     1
#define CTP_EXT_NW_ISDN     2
#define CTP_EXT_NW_GSM      3
#define CTP_EXT_NW_CDMA     4
#define CTP_EXT_NW_ANALG    5
#define CTP_EXT_NW_PKTSW    6
#define CTP_EXT_NW_OTHER    7


/*****************************************************************************
**  Type Definitions
*****************************************************************************/

/* define CTP error codes - lower value error codes are reserved for TCS */
enum
{
    CTP_SUCCESS         = BT_PASS, /* 0 */
    CTP_CMD_STARTED     = 0x11,
    CTP_BUSY            = 0x12,
    CTP_INVALID_PARAM   = 0x13,
    CTP_TCS_ERROR       = 0x14,
    CTP_BTM_ERROR       = 0x15,
    CTP_NO_RESOURCES    = 0x16,
    CTP_NOT_ALLOWED     = 0x17,
    CTP_ERR_SDP_REG     = 0x18,
    CTP_ERR_SDP_PROTO   = 0x19,
    CTP_ERR_SDP_CLASSID = 0x1a,
    CTP_ERR_SDP_ATTR    = 0x1b,
    CTP_ERR_SDP_PROFILE = 0x1d,
    CTP_TIMEOUT         = 0x1e,
    CTP_NOT_AVAILABLE   = 0x1f,
    CTP_L2CAP_ERROR     = 0x20,
    CTP_BAD_STATE       = 0x21,
    CTP_ERR_PROFILE     = 0x22,
    CTP_UNSUPPORTED     = 0x23,
    CTP_ERR_MAX         = 0x24
};
typedef UINT8 tCTP_STATUS;

/* define status for tCTP_TL_CBACK AND tCTP_GW_CBACK */
enum
{
    CTP_STS_GW_FOUND,
    CTP_STS_GW_NOT_FOUND,
    CTP_STS_CONNECTED_LIMITED,
    CTP_STS_CONNECTED_FULL,
    CTP_STS_LOST_GATEWAY,
    CTP_STS_LOST_TERMINAL,
    CTP_STS_DISCONNECTED,
    CTP_STS_FIMA_CONNECTED,
    CTP_STS_FIMA_DISCONNECTED,
    CTP_STS_FIMA_FAILED,
    CTP_STS_FIMA_IC_TIMEOUT,
    CTP_NUMBER_STS
};
typedef UINT8 tCTP_CBACK_STS;

typedef struct
{
    UINT16  id;
    UINT8   type;
    UINT8   len;                        /* Number of bytes in the entry */
    UINT8   value[SDP_MAX_ATTR_LEN];    /* Contents of the entry        */
} tCTP_ATTRIBUTE;

typedef struct
{
    BD_ADDR gw_addr;
    UINT16  gw_version;     /* Profile Version of gateway */
    UINT8   service_name[BT_MAX_SERVICE_NAME_LEN+1];
    UINT8   provider_name[BT_MAX_SERVICE_NAME_LEN+1];
    UINT8   ext_network;
    tCTP_ATTRIBUTE  gw_attr;
} tCTP_GW_INFO;

typedef struct
{
    tTCS_WUG_MST_DB     init_wug_db;
    tTCS_ACCESS_CBACK  *ac_cb;
    BOOLEAN             keep_wug;
}	tCTP_GW_WUG_PARAMS;

typedef struct
{
    tTCS_WUG_MMB_DB     init_wug_db;
    UINT8               num_members;
    tTCS_WUGCFG_CBACK   *cfg_cb;
    tTCS_FIMA_CBACK     *fima_cb;
}	tCTP_TL_WUG_PARAMS;


/*****************************************************************************
**  Callback Function Definitions
*****************************************************************************/

/* Gateway call control callback */
typedef void (tCTP_GW_CC_CBACK) (UINT8 index, UINT8 line_num, UINT16 cc_handle,
                                 UINT8 event, void *message);

/* Gateway incoming data callback */
typedef void (tCTP_GW_DATA_CBACK) (UINT8 index, UINT16 length, void *data);

/* Gateway status callback */
typedef void (tCTP_GW_CBACK) (UINT8 index, UINT16 status);

/* Terminal call control callback */
typedef void (tCTP_TL_CC_CBACK) (UINT8 line_num, UINT16 cc_handle, UINT8 event,
                                 void *message);

/* Terminal incoming data callback */
typedef void (tCTP_TL_DATA_CBACK) (UINT16 length, void *data);

/* Terminal status callback */
typedef void (tCTP_TL_CBACK) (UINT16 status);

/* Terminal registration parameters */
typedef struct
{
    tCTP_GW_INFO        gw_info;
    tCTP_TL_CC_CBACK   *lines_cc_cback;
    tCTP_TL_DATA_CBACK *data_cback;
    tCTP_TL_WUG_PARAMS  wug_params;
    tCTP_TL_CBACK      *tl_cback;
    tTCS_PAGE_RESP_CB  *pr_cback;
} tCTP_TL_REG_PARAMS;

/* Gateway registration parameters */
typedef struct
{
    tCTP_GW_INFO        gw_info;
    tCTP_GW_CC_CBACK   *lines_cc_cback;
    tCTP_GW_DATA_CBACK *data_cback;
    tCTP_GW_WUG_PARAMS  wug_params;
    tCTP_GW_CBACK      *gw_cback;
    tTCS_PAGE_RESP_CB  *pr_cback;
} tCTP_GW_REG_PARAMS;


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
**
** Function         CTP_SetSecurity
**
** Description      Sets the security level as input for CTP and ICP
**
** Returns          CTP_SUCCESS, if successful. Otherwise, error code.
**
*****************************************************************************/
CTP_API extern tCTP_STATUS CTP_SetSecurity (UINT8 * service_name, UINT8 sec_level);

/*****************************************************************************
**
** Function         CTP_IsDataExchgOn
**
** Description      Retrieves the data exchange flag
**
** Returns          TRUE if on, otherwise FALSE (or if disabled).
**
*****************************************************************************/
CTP_API extern BOOLEAN CTP_IsDataExchgOn (void);

/*****************************************************************************
**
**  Function        CTP_SetTraceLevel
**
**  Description     This function changes the trace level (debugging levels)
**
**  Returns         void
**
*****************************************************************************/
CTP_API extern void CTP_SetTraceLevel(UINT8 level);

/*******************************************************************************
**
** Function         CTP_Init
**
** Description      This function is called once at stack startup to allocate
**                  (if using dynamic memory) and initialize the
**                  control block, and initializes the
**                  debug tracing level.
**
** Returns          void
**
*******************************************************************************/
CTP_API extern void CTP_Init(void);

/*******************************************************************************
**
** Function     CTP_TL_Register
**
** Description  Register CTP as a terminal.
**              1. The application should call this to register with CTP
**                 before using the service from CTP.
**              2. CTP keeps the callback routines and registers with TCS and
**                 sets the Telephony bit in COD and
**              3. registers the default security level with the security manager
**
** Notes        CTP TL assumes that the application would set the desired
**              parible mode, connectibility mode and discoverbility mode.
**
** Returns      CTP_SUCCESS, if successful. Otherwise, error code.
**
*******************************************************************************/
CTP_API extern tCTP_STATUS CTP_TL_Register(tCTP_GW_INFO *p_gw_info,
                                           tCTP_TL_CC_CBACK *lines_cc_cback,
                                           tCTP_TL_DATA_CBACK *data_cb,
                                           tCTP_TL_WUG_PARAMS *p_wug_params,
                                           tCTP_TL_CBACK *tl_cb,
                                           tTCS_PAGE_RESP_CB  *pr_cback);

/*******************************************************************************
**
** Function     CTP_TL_Deregister
**
** Description  Application should call this function to deregister the
**              terminal from CTP.
**
** Returns      CTP_NOT_ALLOWED, if there is still ACTIVE calls.
**              CTP_SUCCESS, otherwise.
**
*******************************************************************************/
CTP_API extern tCTP_STATUS CTP_TL_Deregister (BOOLEAN force);

/*******************************************************************************
**
** Function     CTP_TL_SelectLine
**
** Description  Application should call this function to map the tcs_cc_handle to
**              line_number. Otherwise, the application manages the information.
**
** Returns      CTP_SUCCESS, if successful. Otherwise, error code.
**
*******************************************************************************/
CTP_API extern tCTP_STATUS CTP_TL_SelectLine (UINT8 line_num, UINT16 cc_handle);

/*******************************************************************************
**
** Function     CTP_GW_GetTLAddr
**
** Description  Get the BD address of the connected GW
**
** Returns      none
**
*******************************************************************************/
CTP_API extern void CTP_GW_GetTLAddr (UINT8 index, BD_ADDR addr);

/*******************************************************************************
**
** Function     CTP_TL_GetGWAddr
**
** Description  Get the BD address of the connected GW
**
** Returns      CTP_SUCCESS, if successful. Otherwise, error code.
**
*******************************************************************************/
CTP_API extern void CTP_TL_GetGWAddr (BD_ADDR addr);

/*******************************************************************************
**
** Function     CTP_TL_DataWrite
**
** Description  Write data to GW.
**              If the data length is greater than MTU or buffer size,
**              the data is truncated.
**
** Returns      CTP_SUCCESS, if successful. Otherwise, error code.
**
*******************************************************************************/
CTP_API extern tCTP_STATUS CTP_TL_DataWrite (UINT16 length, UINT8 * data);

/*******************************************************************************
**
** Function     CTP_GW_DataWrite
**
** Description  Write data to TL number (index).
**              If the data length is greater than MTU or buffer size,
**              the data is truncated.
**
** Returns      CTP_SUCCESS, if successful. Otherwise, error code.
**
*******************************************************************************/
CTP_API extern tCTP_STATUS CTP_GW_DataWrite (UINT8 index, UINT16 length,
                                             UINT8 * p_data);

/*******************************************************************************
** Function     CTP_GW_Register
**
** Description  Register CTP as a gateway.
**              1. The application should call this to register with CTP
**                 before using the service from CTP.
**              2. CTP keeps the callback routines and registers with TCS and
**                 sets the Telephony bit in COD and
**              3. registers the default security level with the security manager
**
** Notes        CTP TL assumes that the application would set the desired
**              parible mode, connectibility mode and discoverbility mode.
**
** Returns      CTP_SUCCESS, if successful. Otherwise, error code.
*******************************************************************************/
CTP_API extern tCTP_STATUS CTP_GW_Register(tCTP_GW_INFO *p_gw_info,
                                           tCTP_GW_CC_CBACK *lines_cc_cback,
                                           tCTP_GW_DATA_CBACK *data_cb,
                                           tCTP_GW_WUG_PARAMS *p_wug_params,
                                           tCTP_GW_CBACK *gw_cb,
                                           tTCS_PAGE_RESP_CB *pr_cback);

/*******************************************************************************
**
** Function     CTP_GW_Deregister
**
** Description  Application should call this function to deregister the
**              gateway from CTP.
**
** Returns      CTP_NOT_ALLOWED, if there is still ACTIVE calls.
**              CTP_SUCCESS, otherwise.
**
*******************************************************************************/
CTP_API extern tCTP_STATUS CTP_GW_Deregister (BOOLEAN force);

/*******************************************************************************
**
** Function     CTP_GW_SelectLine
**
** Description  Application should call this function to map the tcs_cc_handle to
**              line_number. Otherwise, the application manages the information.
**
** Returns      CTP_SUCCESS, if successful. Otherwise, error code.
**
*******************************************************************************/
CTP_API extern tCTP_STATUS CTP_GW_SelectLine (UINT8 line_num, UINT16 cc_handle);

#ifdef __cplusplus
}
#endif

#endif  /* CTP_API_H */
