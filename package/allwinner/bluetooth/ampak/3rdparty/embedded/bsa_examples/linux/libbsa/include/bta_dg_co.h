/*****************************************************************************
**
**  Name:           bta_dg_co.h
**
**  Description:    This is the interface file for data gateway call-out
**                  functions.
**
**  Copyright (c) 2003, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_DG_CO_H
#define BTA_DG_CO_H

#include "bta_dg_api.h"
#include "l2c_api.h"
#include "rfcdefs.h"

/*****************************************************************************
**  Constants
*****************************************************************************/

/* RS-232 Signal Mask */
#define BTA_DG_DTRDSR           0x01        /* DTR/DSR signal. */
#define BTA_DG_RTSCTS           0x02        /* RTS/CTS signal. */
#define BTA_DG_RI               0x04        /* Ring indicator signal. */
#define BTA_DG_CD               0x08        /* Carrier detect signal. */

/* RS-232 Signal Values */
#define BTA_DG_DTRDSR_ON        0x01        /* DTR/DSR signal on. */
#define BTA_DG_DTRDSR_OFF       0x00        /* DTR/DSR signal off. */
#define BTA_DG_RTSCTS_ON        0x02        /* RTS/CTS signal on. */
#define BTA_DG_RTSCTS_OFF       0x00        /* RTS/CTS signal off. */
#define BTA_DG_RI_ON            0x04        /* Ring indicator signal on. */
#define BTA_DG_RI_OFF           0x00        /* Ring indicator signal off. */
#define BTA_DG_CD_ON            0x08        /* Carrier detect signal on. */
#define BTA_DG_CD_OFF           0x00        /* Carrier detect signal off. */

/* Data Flow Mask */
#define BTA_DG_RX_PUSH_BUF      0x01        /* RX push with zero copy. */
#define BTA_DG_RX_PULL          0x02        /* RX pull. */
#define BTA_DG_TX_PUSH          0x00        /* TX push. */
#define BTA_DG_TX_PUSH_BUF      0x10        /* TX push with zero copy. */
#define BTA_DG_TX_PULL          0x20        /* TX pull. */

/* BT_HDR buffer offset */
#define BTA_DG_MIN_OFFSET       (L2CAP_MIN_OFFSET + RFCOMM_MIN_OFFSET)

/*****************************************************************************
**  Function Declarations
*****************************************************************************/

/*******************************************************************************
**
** Function         bta_dg_co_init
**
** Description      This callout function is executed by DG when a server is
**                  started by calling BTA_DgListen().  This function can be
**                  used by the phone to initialize data paths or for other
**                  initialization purposes.  The function must return the
**                  data flow mask as described below.
**
**
** Returns          Data flow mask.
**
*******************************************************************************/
BTA_API extern UINT8 bta_dg_co_init(UINT16 handle, UINT8 app_id);

/*******************************************************************************
**
** Function         bta_dg_co_open
**
** Description      This function is executed by DG when a connection to a
**                  server is opened.  The phone can use this function to set
**                  up data paths or perform any required initialization or
**                  set up particular to the connected service.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_co_open(UINT16 handle, UINT8 app_id, tBTA_SERVICE_ID service, UINT16 mtu);

/*******************************************************************************
**
** Function         bta_dg_co_close
**
** Description      This function is called by DG when a connection to a
**                  server is closed.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_co_close(UINT16 handle, UINT8 app_id);

/*******************************************************************************
**
** Function         bta_dg_co_tx_path
**
** Description      This function is called by DG to transfer data on the
**                  TX path; that is, data being sent from BTA to the phone.
**                  This function is used when the TX data path is configured
**                  to use the pull interface.  The implementation of this
**                  function will typically call Bluetooth stack functions
**                  PORT_Read() or PORT_ReadData() to read data from RFCOMM
**                  and then a platform-specific function to send data that
**                  data to the phone.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_co_tx_path(UINT16 handle, UINT8 app_id);

/*******************************************************************************
**
** Function         bta_dg_co_rx_path
**
** Description      This function is called by DG to transfer data on the
**                  RX path; that is, data being sent from the phone to BTA.
**                  This function is used when the RX data path is configured
**                  to use the pull interface.  The implementation of this
**                  function will typically call a platform-specific function
**                  to read data from the phone and then call Bluetooth stack
**                  functions PORT_Write() or PORT_WriteData() to send data
**                  to RFCOMM.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_co_rx_path(UINT16 handle, UINT8 app_id, UINT16 mtu);

/*******************************************************************************
**
** Function         bta_dg_co_tx_write
**
** Description      This function is called by DG to send data to the phone
**                  when the TX path is configured to use a push interface.
**                  The implementation of this function must copy the data to
**                  the phone's memory.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_co_tx_write(UINT16 handle, UINT8 app_id, UINT8 *p_data, UINT16 len);

/*******************************************************************************
**
** Function         bta_dg_co_tx_writebuf
**
** Description      This function is called by DG to send data to the phone
**                  when the TX path is configured to use a push interface with
**                  zero copy.  The phone must free the buffer using function
**                  GKI_freebuf() when it is through processing the buffer.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_co_tx_writebuf(UINT16 handle, UINT8 app_id, BT_HDR *p_buf);

/*******************************************************************************
**
** Function         bta_dg_co_rx_flow
**
** Description      This function is called by DG to enable or disable
**                  data flow on the RX path when it is configured to use
**                  a push interface.  If data flow is disabled the phone must
**                  not call bta_dg_ci_rx_write() or bta_dg_ci_rx_writebuf()
**                  until data flow is enabled again.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_co_rx_flow(UINT16 handle, UINT8 app_id, BOOLEAN enable);

/*******************************************************************************
**
** Function         bta_dg_co_control
**
** Description      This function is called by DG to send RS-232 signal
**                  information to the phone.  This function allows these
**                  signals to be propagated from the RFCOMM channel to the
**                  phone.  If the phone does not use these signals the
**                  implementation of this function can do nothing.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void bta_dg_co_control(UINT16 handle, UINT8 app_id, UINT8 signals, UINT8 values);

#endif /* BTA_DG_CO_H */
