/*****************************************************************************
**
**  Name:           bta_mse_ci.h
**
**  Description:    This is the interface file for the Message Server Equipment
**                  (MSE) subsystem call-out functions.
**
**  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_MSE_CI_H
#define BTA_MSE_CI_H

#include "bta_api.h"
#include "bta_ma_def.h"
#include "bta_mse_api.h"
#include "bta_mse_co.h"

/*******************************************************************************
**
** Function         bta_mse_ci_get_folder_entry
**
** Description      This function is called in response to the
**                  bta_mse_co_get_folder_entry call-out function.
**
** Parameters       mas_session_id - MAS session ID
**                  status - BTA_MA_STATUS_OK if p_entry points to a valid entry.
**                           BTA_MA_STATUS_EODIR if no more entries (p_entry is ignored).
**                           BTA_MA_STATUS_FAIL if any errors have occurred.
**                  evt    - evt from the call-out function
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mse_ci_get_folder_entry(tBTA_MA_SESS_HANDLE  mas_session_id,
                                                tBTA_MA_STATUS status,
                                                UINT16 evt);
/*******************************************************************************
**
** Function         bta_mse_ci_get_msg_list_info
**
** Description      This function is called in response to the
**                  bta_mse_co_get_msg_list_info call-out function.
**
** Parameters       mas_session_id - MAS session ID
**                  status - BTA_MA_STATUS_OK operation is successful.
**                           BTA_MA_STATUS_FAIL if any errors have occurred.
**                  evt    - evt from the call-out function
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mse_ci_get_msg_list_info(tBTA_MA_SESS_HANDLE  mas_session_id,
                                                 tBTA_MA_STATUS status,
                                                 UINT16 evt);
/*******************************************************************************
**
** Function         bta_mse_ci_get_msg_list_entry
**
** Description      This function is called in response to the
**                  bta_mse_co_get_msg_list_entry call-out function.
**
** Parameters       mas_session_id - MAS session ID
**                  status - BTA_MA_STATUS_OK if p_entry points to a valid entry.
**                           BTA_MA_STATUS_EODIR if no more entries (p_entry is ignored).
**                           BTA_MA_STATUS_FAIL if any errors have occurred.
**                  evt    - evt from the call-out function
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mse_ci_get_msg_list_entry(tBTA_MA_SESS_HANDLE  mas_session_id,
                                                  tBTA_MA_STATUS status,
                                                  UINT16 evt);
/*******************************************************************************
**
** Function         bta_mse_ci_get_msg
**
** Description      This function is called in response to the
**                  bta_mse_co_get_msg call-out function.
**
** Parameters       mas_session_id - MAS session ID
**                  status - BTA_MA_STATUS_OK if p_msg points to a valid bmessage.
**                           BTA_MA_STATUS_FAIL if any errors have occurred.
**                  filled_buff_size - size of the filled buffer
**                  multi_pkt_status - BTA_MA_MPKT_STATUS_MORE - need to get more packets
**                                     BTA_MA_MPKT_STATUS_LAST - last packet of the bMessage
**                  frac_deliver_status -  BTA_MA_FRAC_DELIVER_MORE - other fractions following
**                                                                    this bMessage
**                                         BTA_MA_FRAC_DELIVER_LAST - Last fraction
**                  evt    - evt from the call-out function
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mse_ci_get_msg(tBTA_MA_SESS_HANDLE  mas_session_id,
                                       tBTA_MA_STATUS status,
                                       UINT16 filled_buff_size,
                                       tBTA_MA_MPKT_STATUS multi_pkt_status,
                                       tBTA_MA_FRAC_DELIVER frac_deliver_status,
                                       UINT16 evt);
/*******************************************************************************
**
** Function         bta_mse_ci_set_msg_delete_status
**
** Description      This function is called in response to the
**                  bta_mse_co_set_msg_delete_status call-out function.
**
** Parameters       mas_session_id - MAS session ID
**                  status - BTA_MA_STATUS_OK if operation is successful.
**                           BTA_MA_STATUS_FAIL if any errors have occurred.
**                  evt    - evt from the call-out function
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mse_ci_set_msg_delete_status(tBTA_MA_SESS_HANDLE  mas_session_id,
                                                     tBTA_MA_STATUS status,
                                                     UINT16 evt);
/*******************************************************************************
**
** Function         bta_mse_ci_push_msg
**
** Description      This function is called in response to the
**                  bta_mse_co_push_msg call-out function.
**
** Parameters       mas_session_id - MAS session ID
**                  status - BTA_MA_STATUS_OK if the message upload is successful.
**                           BTA_MA_STATUS_FAIL if any errors have occurred.
**                  last_packet - last packet of a multi-packet message
**                  handle - message handle for the uploaded message if
**                           status is BTA_MA_OK and last_packet is TRUE
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_mse_ci_push_msg(tBTA_MA_SESS_HANDLE  mas_session_id,
                                        tBTA_MA_STATUS status,
                                        BOOLEAN last_packet,
                                        tBTA_MA_MSG_HANDLE handle,
                                        UINT16 evt);
#endif /* BTA_MSE_CI_H */
