/*****************************************************************************
**
**  Name:          amp_api.h
**
**  Description:   this file contains the AMP API definitions
**
**
**
**  Copyright (c) 2008-2010, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
******************************************************************************/

#ifndef AMP_API_H
#define AMP_API_H

#ifndef AMP_API
#define AMP_API
#endif

#include "l2c_api.h"
#include "ampdefs.h"

#ifdef __cplusplus
extern "C" {
#endif


#define AMP_SERV_TYPE_NO_TRAFF      HCI_SERVICE_NO_TRAFFIC
#define AMP_SERV_TYPE_BEST_EFFORT   HCI_SERVICE_BEST_EFFORT
#define AMP_SERV_TYPE_GUARANTEED    HCI_SERVICE_GUARANTEED
#define AMP_SERV_TYPE_DEFAULT       AMP_SERV_TYPE_BEST_EFFORT

/* AMP_MoveReq(...) return status codes */
#define AMP_MOVE_FAIL           0   /* can't start move operation */
#define AMP_MOVE_IN_PROGRESS    1   /* move in progress (not necessary by this request) */
#define AMP_MOVE_NO_RESOURCES   2   /* CID is already on the connection that best provides for the selected criteria */


/***********************************************************************************
**                              AMP ---- Application
************************************************************************************/


/* Define the callback function prototypes for applications
*/

/* Connection indication callback prototype. Parameters are
**              BD Address of remote
**              Local CID assigned to the connection
**              PSM that the remote wants to connect to
**              Identifier that the remote sent
*/
typedef void (tAMP_CONNECT_IND_CB) (BD_ADDR, UINT16, UINT16, UINT8);


/* Connection confirmation callback prototype. Parameters are
**              Local CID
**              Result - 0 = connected, non-zero means failure reason
*/
typedef void (tAMP_CONNECT_CFM_CB) (UINT16, UINT16);


/* Connection pending callback prototype. Parameters are
**              Local CID
*/
typedef void (tAMP_CONNECT_PND_CB) (UINT16);


/* Configuration indication callback prototype. Parameters are
**              Local CID assigned to the connection
**              Pointer to configuration info
*/
typedef void (tAMP_CONFIG_IND_CB) (UINT16, tL2CAP_CFG_INFO *);


/* Configuration confirm callback prototype. Parameters are
**              Local CID assigned to the connection
**              Pointer to configuration info
*/
typedef void (tAMP_CONFIG_CFM_CB) (UINT16, tL2CAP_CFG_INFO *);


/* Disconnect indication callback prototype. Parameters are
**              Local CID
**              Boolean whether upper layer should ack this
*/
typedef void (tAMP_DISCONNECT_IND_CB) (UINT16, BOOLEAN);


/* Disconnect confirm callback prototype. Parameters are
**              Local CID
**              Result
*/
typedef void (tAMP_DISCONNECT_CFM_CB) (UINT16, UINT16);


/* Move indication callback prototype. Parameters are
**              Local CID
**              Local Controller ID to move to
*/
typedef void (tAMP_MOVE_IND_CB) (UINT16, UINT8);


/* Move response callback prototype. Parameters are
**              Local CID
**              Result
*/
typedef void (tAMP_MOVE_RSP_CB) (UINT16, UINT8);


/* Move confirmation callback prototype. Parameters are
**              Local CID
**              Result
*/
typedef void (tAMP_MOVE_CFM_CB) (UINT16, UINT8);

/* Move confirmation response callback prototype. Parameter is
**              Local CID
*/
typedef void (tAMP_MOVE_CFM_RSP_CB) (UINT16);


/* QOS Violation indication callback prototype. Parameters are
**              BD Address of violating device
*/
typedef void (tAMP_QOS_VIOLATION_IND_CB) (BD_ADDR);


/* Data received indication callback prototype. Parameters are
**              Local CID
**              Address of buffer queue
*/
typedef void (tAMP_DATA_IND_CB) (UINT16, BT_HDR *);


/* Echo response callback prototype. Note that this is not included in the
** registration information, but is passed to L2CAP as part of the API to
** actually send an echo request. Parameters are
**              Result
*/
typedef void (tAMP_ECHO_RSP_CB) (UINT16);



/* Congestion status callback protype. This callback is optional. If
** an application tries to send data when the transmit queue is full,
** the data will anyways be dropped. The parameter is:
**              Local CID
**              TRUE if congested, FALSE if uncongested
*/
typedef void (tAMP_CONGESTION_STATUS_CB) (UINT16, BOOLEAN);


/* Transmit complete callback protype. This callback is optional. If
** an application tries to send data when the transmit queue is full,
** the data will anyways be dropped. The parameter is:
**              Local CID
**              Number of bytes queued to L2CAP for transmission
*/
typedef void (tAMP_TX_COMPLETE_CB) (UINT16, UINT16);


#if AMP_IOP_TESTING == TRUE
/* Callback used for test. Is called after L2CAP creates AMP signaling channel */
typedef void (tAMP_TEST_CREATE_AMP_CHANNEL_CB) (void);
typedef void (tAMP_TEST_ILLEGAL_COMMAND_CB) (void);
typedef void (tAMP_TEST_DISCOVER_REQUEST_CB) (void);
typedef void (tAMP_TEST_CHANGE_NOTIFY_CB) (void);
typedef void (tAMP_TEST_INFO_REQUEST_CB) (void);
typedef void (tAMP_TEST_INFO_REQUEST_BAD_AMP_ID_CB) (void);
typedef void (tAMP_TEST_ASSOC_REQUEST_CB) (void);
typedef void (tAMP_TEST_ASSOC_REQUEST_BAD_AMP_ID_CB) (void);
typedef void (tAMP_TEST_CR_PHYS_LINK_REQUEST_CB) (void);
typedef void (tAMP_TEST_DISC_PHYS_LINK_REQUEST_CB) (void);
#endif

#if AMP_IOP_TEST_MOVE == TRUE
typedef void (tAMP_TEST_PHYS_LINK_CREATED_CB) (UINT8);
typedef void (tAMP_TEST_PHYS_LINK_ACCEPTED_CB) (UINT8);
typedef void (tAMP_TEST_PHYS_LINK_DISCONNECTED_CB) (void);
typedef void (tAMP_TEST_ACCEPT_PROC_START_CB) (void);
typedef void (tAMP_TEST_AMP_PROT_CR_PH_LINK_RSP_CB) (UINT8);
#endif

/* Define the structure that applications use to register with
** AMP
*/
typedef struct
{
    tAMP_CONNECT_IND_CB        *pAMP_ConnectInd_Cb;
    tAMP_CONNECT_CFM_CB        *pAMP_ConnectCfm_Cb;
    tAMP_CONNECT_PND_CB        *pAMP_ConnectPnd_Cb;
    tAMP_CONFIG_IND_CB         *pAMP_ConfigInd_Cb;
    tAMP_CONFIG_CFM_CB         *pAMP_ConfigCfm_Cb;
    tAMP_DISCONNECT_IND_CB     *pAMP_DisconnectInd_Cb;
    tAMP_DISCONNECT_CFM_CB     *pAMP_DisconnectCfm_Cb;

    tAMP_MOVE_IND_CB           *pAMP_MoveInd_Cb;
    tAMP_MOVE_RSP_CB           *pAMP_MoveRsp_Cb;
    tAMP_MOVE_CFM_CB           *pAMP_MoveCfm_Cb;
    tAMP_MOVE_CFM_RSP_CB       *pAMP_MoveCfmRsp_Cb;

    tAMP_QOS_VIOLATION_IND_CB  *pAMP_QoSViolationInd_Cb;
    tAMP_DATA_IND_CB           *pAMP_DataInd_Cb;
    tAMP_CONGESTION_STATUS_CB  *pAMP_CongestionStatus_Cb;
    tAMP_TX_COMPLETE_CB        *pAMP_TxComplete_Cb;

#if AMP_IOP_TESTING == TRUE
    tAMP_TEST_CREATE_AMP_CHANNEL_CB *pAMP_TestCreateAMPChannel_Cb;
    tAMP_TEST_ILLEGAL_COMMAND_CB    *pAMP_TestIllegalCommand_Cb;
    tAMP_TEST_DISCOVER_REQUEST_CB   *pAMP_TestDiscoverRequest_Cb;
    tAMP_TEST_CHANGE_NOTIFY_CB      *pAMP_TestChangeNotify_Cb;
    tAMP_TEST_INFO_REQUEST_CB       *pAMP_TestInfoRequest_Cb;
    tAMP_TEST_INFO_REQUEST_BAD_AMP_ID_CB    *pAMP_TestInfoRequestBadAMPId_Cb;
    tAMP_TEST_ASSOC_REQUEST_CB       *pAMP_TestAssocRequest_Cb;
    tAMP_TEST_ASSOC_REQUEST_BAD_AMP_ID_CB    *pAMP_TestAssocRequestBadAMPId_Cb;
    tAMP_TEST_CR_PHYS_LINK_REQUEST_CB   *pAMP_TestCrPhysLinkRequest_Cb;
    tAMP_TEST_DISC_PHYS_LINK_REQUEST_CB *pAMP_TestDiscPhysLinkRequest_Cb;
#endif

#if AMP_IOP_TEST_MOVE == TRUE
    tAMP_TEST_PHYS_LINK_CREATED_CB         *pAMP_TestPhysLinkCreated_Cb;
    tAMP_TEST_PHYS_LINK_ACCEPTED_CB        *pAMP_TestPhysLinkAccepted_Cb;
    tAMP_TEST_PHYS_LINK_DISCONNECTED_CB    *pAMP_TestPhysLinkDisconnected_Cb;
    tAMP_TEST_ACCEPT_PROC_START_CB         *pAMP_TestAcceptProcStart_Cb;
    tAMP_TEST_AMP_PROT_CR_PH_LINK_RSP_CB   *pAMP_TestAMPProtCrPhLinkRsp_Cb;
#endif

} tAMP_APPL_INFO;


/* Define the AMP interface functions
*/

/*******************************************************************************
**
** Function         AMP_Register
**
** Description      Other tasks call this function to register for AMP manager
**                  services.
**                  If a task successfully registers with AMP manager, AMP manager
**                  registers with L2CAP to represent this task.
**
** Returns          the (real or virtual) PSM to use
**
*******************************************************************************/
AMP_API extern UINT16   AMP_Register (UINT16 psm, tAMP_APPL_INFO *p_cb_info, tAMP_CRITERIA default_criteria);

/*******************************************************************************
**
** Function         AMP_Deregister
**
** Description      Other tasks call this function to de-register for AMP manager
**                  services.
**                  The caller is deregistered with AMP manager, AMP manager deregisters
**                  with L2CAP to represent this task.
**
** Returns          void
**
*******************************************************************************/
AMP_API extern void     AMP_Deregister (UINT16 psm);

/*******************************************************************************
**
** Function         AMP_SetAsThreshold
**
** Description      Applications call this function to set the auto switching
**                  threshold.
**
**                  threshold_on_br_edr : move to AMP if it is less than number of bytes
**                       transfered on BR/EDR during AMP_BR_EDR_AS_COUNT_ARRAY_SIZE*AMP_AS_TOUT_ON_BR_EDR
**                  threshold_on_amp : move to BR/EDR if it is greater than number of bytes
**                       transfered on AMP during AMP_AMP_AS_COUNT_ARRAY_SIZE*AMP_AS_TOUT_ON_AMP
**
** Returns:         void
**
*******************************************************************************/
AMP_API extern void AMP_SetAsThreshold (UINT32 threshold_on_br_edr, UINT32 threshold_on_amp);

/*******************************************************************************
**
** Function         AMP_SetNextConnCriteria
**
** Description      Applications call this function to set the criteria for an
**                  upcoming AMP outbound connection.
**
**                  Note that the connection is not established at this time, but
**                  connection establishment will be started next and should use
**                  the criteria set here.
**
**                  This criteria setting is a 1-shot, and will be reset once the
**                  next connection establishment is started.
**
** Returns          FALSE   if there is no space to save the triplet {dest_bda/uuid/criteria}.
**                  TRUE    if the triplet is saved.
**
*******************************************************************************/
AMP_API extern BOOLEAN  AMP_SetNextConnCriteria (BD_ADDR dest_bda, tBT_UUID *p_uuid, tAMP_CRITERIA con_criteria);

/*******************************************************************************
**
** Function         AMP_ChangeConnCriteria
**
** Description      Applications call this function to set the criteria for an
**                  existing AMP connection.
**
**                  Note that more than one connection may exist at the same time
**                  for the same profile (e.g FTP), and the criteria is updated
**                  for ALL connections to the peer BDA with a matching UUID.
**
** Returns          FALSE   if no existing connection has the given UUID.
**                  TRUE    if at least one existing connection has the given UUID.
**                          In this case the new criteria is applied to the existing
**                          connection.
**
*******************************************************************************/
AMP_API extern BOOLEAN  AMP_ChangeConnCriteria  (BD_ADDR dest_bda, tBT_UUID *p_uuid, tAMP_CRITERIA new_criteria);

/*******************************************************************************
**
** Function         AMP_GetCtrlrUsedForConn
**
** Description      Applications call this function to read which controller
**                  ID is being used by the first found existing AMP connection
**                  to the dest_bda with the UUID (there can be more than one such
**                  connection).
**
** Returns          Controller ID of the found connection (starting from 1) or 0 if
**                  no connection is found.
**
*******************************************************************************/
AMP_API extern UINT8    AMP_GetCtrlrUsedForConn (BD_ADDR dest_bda, tBT_UUID *p_uuid);

/*******************************************************************************
**
** Function         AMP_ConnectReq
**
** Description      Higher layers call this function to create an AMP connection.
**                  Note that the connection is not established at this time, but
**                  connection establishment gets started. The callback function
**                  will be invoked when connection establishes or fails.
**
** Returns:         CID of the connection if the processing is passed to L2CAP,
**                  or 0 if connection creation fails to start.
**
*******************************************************************************/
AMP_API extern UINT16   AMP_ConnectReq (UINT16 psm, BD_ADDR bd_addr,
                                        tL2CAP_ERTM_INFO *p_ertm_info, tBT_UUID *p_uuid);

/*******************************************************************************
**
** Function         AMP_ConnectRsp
**
** Description      Higher layers call this function to send Connection response
**                  to the connection initiator.
**
** Returns:         The result of L2CAP processing of the request.
**
*******************************************************************************/
AMP_API extern BOOLEAN  AMP_ConnectRsp (BD_ADDR bd_addr, UINT8 id, UINT16 lcid, UINT16 result,
                                        UINT16 status, tL2CAP_ERTM_INFO *p_ertm_info, tBT_UUID *p_uuid);

/*******************************************************************************
**
** Function         AMP_ConfigReq
**
** Description      Higher layers call this function to send configuration request.
**
** Returns:         TRUE if the request is sent, else FALSE.
**
*******************************************************************************/
AMP_API extern BOOLEAN  AMP_ConfigReq (UINT16 cid, tL2CAP_CFG_INFO *p_cfg);

/*******************************************************************************
**
** Function         AMP_ConfigRsp
**
** Description      Higher layers call this function to send a configuration
**                  response.
**
** Returns:         FALSE   if the response can't be processed.
**                  TRUE    in all other cases.
**
*******************************************************************************/
AMP_API extern BOOLEAN  AMP_ConfigRsp (UINT16 cid, tL2CAP_CFG_INFO *p_cfg);

/*******************************************************************************
**
** Function         AMP_DataWrite
**
** Description      Higher layers call this function to send data to the far end.
**
** Returns:         L2CAP return values (L2CAP_DW_SUCCESS/L2CAP_DW_CONGESTED/L2CAP_DW_FAILED).
**
*******************************************************************************/
AMP_API extern UINT8    AMP_DataWrite (UINT16 cid, BT_HDR *p_data);

/*******************************************************************************
**
** Function         AMP_MoveReq
**
** Description      Higher layers call this function to start to move logical
**                  channel associated with the cid.
**                  For now the move rules:
**                  - if the cid L2CAP channel now on BR_EDR then go to AMP
**                  - else (i.e. now on AMP) go to BR_EDR.
**
** Returns:         The value that indicates how/if move started (AMP_MOVE_FAIL,
**                  AMP_MOVE_IN_PROGRESS, AMP_MOVE_NO_RESOURCES.
**
**                  If parameter 2 is AMP_USE_CURRENT_CRITERIA, the criteria is
**                  not changed, otherwise it is changed to the new value.
**
** Note:            The function is not used at the moment.
**
*******************************************************************************/
AMP_API extern UINT16   AMP_MoveReq (UINT16 cid, tAMP_CRITERIA new_criteria);

/*******************************************************************************
**
** Function         AMP_MoveRsp
**
** Description      Higher layers call this function to indicate to AMP manager
**                  how to treat move indication received from the opposite side.
**
** Returns:         Void
**
** Note:            At the moment the function is used only for Conformation tests.
**                  Maybe it has to be in Testing section.
*******************************************************************************/
AMP_API extern void     AMP_MoveRsp (UINT16 cid, UINT16 result);

/*******************************************************************************
**
** Function         AMP_MoveCfm
**
** Description      Higher layers call this function after processing move response
**                  received from AMP manager.
**
** Returns:         Void
**
** Note:            At the moment the function is used only for Conformation tests.
**                  Maybe it has to be in Testing section.
*******************************************************************************/
AMP_API extern void     AMP_MoveCfm (UINT16 cid, UINT16 result);

/*******************************************************************************
**
** Function         AMP_DisconnectReq
**
** Description      Higher layers call this function to disconnect a channel.
**                  The function calls:
**                  - AMP SM to start to release logical channel (AMP entity)
**                  - L2CAP to start to release CID.
**
** Returns:         Value provided by L2CAP
**
*******************************************************************************/
AMP_API extern BOOLEAN  AMP_DisconnectReq (UINT16 cid);

/*******************************************************************************
**
** Function         AMP_DisconnectRsp
**
** Description      Higher layers call this function to acknowledge the
**                  disconnection of a channel. The function calls AMP SM
**                  to start to release logical channel for the CID.
**                  Disconnect Response will be passed to L2CAP by AMP manager.
**
** Returns:         L2CAP provided value
**
*******************************************************************************/
AMP_API extern BOOLEAN  AMP_DisconnectRsp (UINT16 cid);

/*******************************************************************************
**
** Function         AMP_SetTraceLevel
**
** Description      This function sets the trace level for AMP. If called with
**                  a value of 0xFF, it simply reads the current trace level.
**
** Returns          the new (current) trace level
**
*******************************************************************************/
AMP_API extern UINT8 AMP_SetTraceLevel (UINT8 new_level);

/*******************************************************************************
**
** Function         AMP_GetMaxPacketSize
**
** Description      Higher layers call this function to find out max packet size
**                  which can be used for the connection associated with the cid.
**
** Returns:         For AMP controller the maximum size of L2CAP PDU, for BR/EDR
**                  controller the maximum packet size which may be provided for
**                  transmission or reception via the controller.
**
*******************************************************************************/
AMP_API extern UINT16   AMP_GetMaxPacketSize (BD_ADDR bd_addr, UINT16 cid);

/*******************************************************************************
**
** Function         AMP_AddAMPCtrlr
**
** Description      The function emulates insertion of AMP card.
**
*******************************************************************************/
AMP_API extern void AMP_AddAMPCtrlr(UINT8 card_idx);

/*******************************************************************************
**
** Function         AMP_RemoveAMPCtrlr
**
** Description      The function emulates rem Removal of AMP card.
**
*******************************************************************************/
AMP_API extern void AMP_RemoveAMPCtrlr(UINT8 card_idx);

#if AMP_IOP_TESTING == TRUE
AMP_API extern BOOLEAN  AMP_TestRegister (UINT16 psm, tAMP_APPL_INFO *p_cb_info, UINT8 specific_test);
AMP_API extern void     AMP_CreateAMPChan(BD_ADDR bd_addr, BOOLEAN detect_discov_req, BOOLEAN test_change_notify);
AMP_API extern void     AMP_TestIllegalCommand(UINT8 illegal_code);
//AMP_API extern void     AMP_TestDiscoverRequest(void);
AMP_API extern void AMP_TestDiscoverRequest(UINT16 mtu_size, UINT16 *ext_f_mask, UINT8 num_words);
//AMP_API extern void     AMP_TestDiscoverRequest(BD_ADDR bd_addr);
AMP_API extern void     AMP_TestChangeRequest(UINT8 ctrl_id, UINT8 ctrl_type, UINT8 ctrl_state);
AMP_API extern void     AMP_TestInfoRequest(UINT8 rc_id, BOOLEAN use_this_rc_id);
//AMP_API extern void     AMP_TestInfoRequest(BD_ADDR bd_addr);
AMP_API extern void     AMP_TestAssocRequest(UINT8 rc_id, BOOLEAN use_this_rc_id);
AMP_API extern void     AMP_TestCrPhysLinkRequest(BD_ADDR bd_addr);
#endif

#if AMP_IOP_TEST_MOVE == TRUE
AMP_API extern void     AMP_TestConfRspOKInsteadOfPending(void);
AMP_API extern void     AMP_TestConfRspFailureAfterPending(void);
AMP_API extern BOOLEAN  AMP_TestMoveRegister (UINT16 psm, tAMP_APPL_INFO *p_cb_info);
AMP_API extern void     AMP_TestMoveRequestRefused(void);
AMP_API extern void     AMP_TestMoveRequestRefusedAfterPending(void);
AMP_API extern void     AMP_TestMoveFinalRspRefused(void);
AMP_API extern void     AMP_TestMoveCollision(void);
AMP_API extern void     AMP_TestCreatePhysLinkOnly(void);
AMP_API extern void     AMP_TestNoResponseToAMPProtCreatePhysLinkRequest(void);
AMP_API extern void     AMP_TestCorruptGampKey(void);
AMP_API extern void     AMP_TestSendPLDiscImmedAfterPLCreate(void);
AMP_API extern void     AMP_TestSendPLConnReqAgainAfterPLCreateIsCompleted(void);
AMP_API extern void     AMP_TestSendPLConnReqWithUnsuppAMPId(void);
AMP_API extern void     AMP_TestSendCnfRspPendingBeforeProcCnfInd(void);
AMP_API extern void     AMP_TestRspToCreateAndDisconTogether(void);
#endif /* #if AMP_IOP_TEST_MOVE == TRUE */

#ifdef __cplusplus
}
#endif


#endif
