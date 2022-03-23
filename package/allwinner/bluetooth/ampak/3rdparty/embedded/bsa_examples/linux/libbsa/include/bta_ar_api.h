/*****************************************************************************
**
**  Name:           bta_ar_api.h
**
**  Description:    This is the public interface file for the simulatenous
**                  advanced audio/video streaming (AV) source and sink of BTA,
**                  Widcomm's Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2004-2008, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AR_API_H
#define BTA_AR_API_H

#include "avdt_api.h"
#include "avct_api.h"
#include "avrc_api.h"
#include "sdp_api.h"
#include "bta_av_api.h"
#include "bta_sys.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* This event signal to AR user that other profile is connected */
#define BTA_AR_AVDT_CONN_EVT    (AVDT_MAX_EVT + 1)

/*******************************************************************************
**
** Function         bta_ar_init
**
** Description      This function is called from bta_sys_init().
**                  to initialize the control block
**
** Returns          void
**
*******************************************************************************/
extern void bta_ar_init(void);

/*******************************************************************************
**
** Function         bta_ar_reg_avdt
**
** Description      This function is called to register to AVDTP.
**
** Returns          void
**
*******************************************************************************/
extern void bta_ar_reg_avdt(tAVDT_REG *p_reg, tAVDT_CTRL_CBACK *p_cback, tBTA_SYS_ID sys_id);

/*******************************************************************************
**
** Function         bta_ar_dereg_avdt
**
** Description      This function is called to de-register from AVDTP.
**
** Returns          void
**
*******************************************************************************/
extern void bta_ar_dereg_avdt(tBTA_SYS_ID sys_id);

/*******************************************************************************
**
** Function         bta_ar_avdt_conn
**
** Description      This function is called to let ar know that some AVDTP profile
**                  is connected for this sys_id.
**                  If the other sys modules started a timer for PENDING_EVT,
**                  the timer can be stopped now.
**
** Returns          void
**
*******************************************************************************/
extern void bta_ar_avdt_conn(tBTA_SYS_ID sys_id, BD_ADDR bd_addr);

/*******************************************************************************
**
** Function         bta_ar_reg_avct
**
** Description      This function is called to register to AVCTP.
**
** Returns          void
**
*******************************************************************************/
extern void bta_ar_reg_avct(UINT16 mtu, UINT16 mtu_br, UINT8 sec_mask, tBTA_SYS_ID sys_id);

/*******************************************************************************
**
** Function         bta_ar_dereg_avct
**
** Description      This function is called to deregister from AVCTP.
**
** Returns          void
**
*******************************************************************************/
extern void bta_ar_dereg_avct(tBTA_SYS_ID sys_id);

/******************************************************************************
**
** Function         bta_ar_reg_avrc
**
** Description      This function is called to register an SDP record for AVRCP.
**
** Returns          void
**
******************************************************************************/
extern void bta_ar_reg_avrc(UINT16 service_uuid, char *p_service_name,
							 char *p_provider_name, UINT16 categories, tBTA_SYS_ID sys_id);

/******************************************************************************
**
** Function         bta_ar_dereg_avrc
**
** Description      This function is called to de-register/delete an SDP record for AVRCP.
**
** Returns          void
**
******************************************************************************/
extern void bta_ar_dereg_avrc(UINT16 service_uuid, tBTA_SYS_ID sys_id);


/*******************************************************************************
**
** Function         bta_ar_avdt_cback
**
** Description      This function is called to notify bta_ar of AVDT events
**
** Returns          void
**
*******************************************************************************/
extern void bta_ar_avdt_cback(UINT8 handle, BD_ADDR bd_addr, UINT8 event, tAVDT_CTRL *p_data);

#ifdef __cplusplus
}
#endif

#endif /* BTA_AR_API_H */
