/*****************************************************************************
 **
 **  Name:           uipc_bsa.h
 **
 **  Description:    UIPC IO Control
 **
 **  Copyright (c) 2011-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_BSA_H
#define UIPC_BSA_H

#define UIPC_CH_ID_BAD 0xFF

/*
 * UIPC IOCTL Requests
 */
enum
{
    UIPC_REQ_TX_FLUSH = 1,      /* Request to flush the TX FIFO */
    UIPC_REQ_RX_FLUSH,          /* Request to flush the RX FIFO */

    UIPC_WRITE_BLOCK,           /* Make write blocking */
    UIPC_WRITE_NONBLOCK,        /* Make write non blocking */

    UIPC_REG_CBACK,             /* Set a new call back */
    UIPC_SET_RX_WM,             /* Set Rx water mark */

    UIPC_REQ_TX_READY,          /* Request an indication when Tx ready */
    UIPC_REQ_RX_READY,          /* Request an indication when Rx data ready */

    UIPC_RESET,                 /* Reset the UIPC interface */
    UIPC_READ_ERROR             /* Read the UIPC error */
};

/*
 * UIPC layer_specific status definitions
 */
#define UIPC_LS_TX_FLOW_OFF            0
#define UIPC_LS_TX_FAIL                1


/* Events generated */
#define UIPC_OPEN_EVT                       0x01
#define UIPC_CLOSE_EVT                      0x02
#define UIPC_RX_DATA_EVT                    0x03
#define UIPC_RX_DATA_READY_EVT              0x04
#define UIPC_TX_DATA_READY_EVT              0x05

/* UIPC Errors used by BSA */
#define UIPC_NOERR              0x00        /* No error */
#define UIPC_ENOMEM             0x01        /* Not enough memory */
#define UIPC_EINVAL             0x02        /* Invalid parameter */

/* UIPC Channels used by BSA */
#define UIPC_CH_ID_CTL          UIPC_CH_ID_2        /* BSA control */
#define UIPC_CH_ID_HH           UIPC_CH_ID_3        /* HID Host */
#define UIPC_CH_ID_AV_AUDIO     UIPC_CH_ID_4        /* AV Audio */
#define UIPC_CH_ID_AV_VIDEO     UIPC_CH_ID_5        /* AV Video */
#define UIPC_CH_ID_AVK_AUDIO    UIPC_CH_ID_6        /* AVK Audio */
#define UIPC_CH_ID_AVK_VIDEO    UIPC_CH_ID_7        /* AVK Video */
#define UIPC_CH_ID_SCO_TX       UIPC_CH_ID_8        /* AG TX */
#define UIPC_CH_ID_SCO_RX       UIPC_CH_ID_9        /* AG RX */
/* SCO channel ID that will be used by application */
#define UIPC_CH_ID_SCO          UIPC_CH_ID_SCO_TX
/* DataGateway: 20 channels by default */
#ifndef UIPC_CH_ID_DG_NB
#define UIPC_CH_ID_DG_NB 20
#endif
#define UIPC_CH_ID_DG_FIRST UIPC_CH_ID_10
#define UIPC_CH_ID_DG_LAST (UIPC_CH_ID_DG_FIRST + UIPC_CH_ID_DG_NB - 1)
/* Health: 4 channels by default */
#ifndef UIPC_CH_ID_HL_NB
#define UIPC_CH_ID_HL_NB 4
#endif

#define UIPC_CH_ID_HL_FIRST (UIPC_CH_ID_DG_LAST + 1)
#define UIPC_CH_ID_HL_LAST (UIPC_CH_ID_HL_FIRST + UIPC_CH_ID_HL_NB - 1)

#define UIPC_CH_ID_NSA  (UIPC_CH_ID_HL_LAST + 1)               /* NSA Communication */
/* Broadcast AV: 2 channels by default */
#define UIPC_CH_ID_BAV_1 (UIPC_CH_ID_NSA + 1)
#define UIPC_CH_ID_BAV_2 (UIPC_CH_ID_BAV_1 + 1)

/*define here Ch IDs used by NSA*/
#define UIPC_FIRST_NSA_CH_ID (UIPC_CH_ID_BAV_2 + 1)
#define UIPC_NSA_CH_ID_CTL UIPC_FIRST_NSA_CH_ID        /* NSA control */
#define UIPC_LAST_NSA_CH_ID UIPC_NSA_CH_ID_CTL

/*define here Ch IDs used by PBC & MCE*/
#define UIPC_CH_ID_PBC ( UIPC_LAST_NSA_CH_ID + 1)        /* PBC */
#define UIPC_CH_ID_PBS ( UIPC_CH_ID_PBC + 1)        /* PBS */
#define UIPC_CH_ID_MCE_RX ( UIPC_CH_ID_PBS + 1)        /* MCE RX Client Recv channel*/
#define UIPC_CH_ID_MCE_TX (UIPC_CH_ID_MCE_RX + 1)        /* MCE TX Client Transmit channel*/
#define UIPC_CH_ID_SAC (UIPC_CH_ID_MCE_TX + 1)          /* SAC */

/* PAN */
#ifndef UIPC_CH_ID_PAN_NB
#define UIPC_CH_ID_PAN_NB 7
#endif
#define UIPC_CH_ID_PAN_FIRST (UIPC_CH_ID_SAC + 1)
#define UIPC_CH_ID_PAN_LAST (UIPC_CH_ID_PAN_FIRST + UIPC_CH_ID_PAN_NB - 1)

#define UIPC_FIRST_BSA_CH_ID UIPC_CH_ID_CTL
#define UIPC_LAST_BSA_CH_ID UIPC_CH_ID_PAN_LAST

#endif
