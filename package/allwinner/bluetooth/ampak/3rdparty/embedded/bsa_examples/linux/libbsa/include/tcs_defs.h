/*****************************************************************************/
/*                                                                           */
/*  Name:          tcs_defs.h                                                */
/*                                                                           */
/*  Description:   this file contains the TCS protocol definitions           */
/*                                                                           */
/*                                                                           */
/*  Date        Modification                                                 */
/*  ------------------------                                                 */
/*  2/16/00     Ash     Create                                               */
/*                                                                           */
/*                                                                           */
/*  Copyright (c) 2000-2004, WIDCOMM Inc., All Rights Reserved.              */
/*  WIDCOMM Bluetooth Core. Proprietary and confidential.                    */
/*****************************************************************************/


#ifndef TCS_DEFS_H
#define TCS_DEFS_H

/* Define TCS clearing causes.
*/
#define TCS_CAUSE_UNKNOWN      0
#define TCS_CCLR_UNA_NO        1            /* Unassigned number                        */
#define TCS_CCLR_NOR_DST       3            /* No route to destination                  */
#define TCS_CCLR_USR_BUSY     17            /* User busy                                */
#define TCS_CCLR_NO_RESP      18            /* No user responding                       */
#define TCS_CCLR_NO_ANSWER    19            /* No answer from user (user alerted)       */
#define TCS_CCLR_SUB_ABSENT   20            /* Subscriber absent                        */
#define TCS_CCLR_CALL_REJ     21            /* Call rejected by user                    */
#define TCS_CCLR_NO_CHG       22            /* Number changed                           */
#define TCS_CCLR_NON_SEL_USR  26            /* Non selected user call clearing          */
#define TCS_CCLR_INV_NO_FMT   28            /* Invalid number format(incomplete number) */
#define TCS_CCLR_CHAN_UNAVAIL 34            /* No circuit/channel available             */
#define TCS_CCLR_RQ_CHAN_UNAV 44            /* Requested circuit/channel not available  */
#define TCS_CCLR_BEAR_UNAV    58            /* Bearer capability not presently available */
#define TCS_CCLR_BEAR_UNIMP   65            /* Bearer capability not implemented        */
#define TCS_CCLR_TIMR_EXPIRE  102           /* Recovery on timer expiry                 */


/* GM listen reject cause
*/
#define TCS_GM_REJ_BAD_NO       1
#define TCS_GM_REJ_USR_BUSY     17
#define TCS_GM_REJ_USR_ABSENT   20


/* Define the masks and types for the protocol descriminator. The first
** byte of each message is the message type, and the high two bits of that
** determine whether it is a CC, GM or CL message.
*/
#define TCS_PROTO_DISCRIM_MASK    0xE0
#define TCS_MSG_TYPE_MASK         0x1F

#define TCS_PROTO_CC              0x00
#define TCS_PROTO_GM              0x20
#define TCS_PROTO_CL              0x40

/* Define the Call Control message types
*/
#define TCS_CC_MSG_ALERTING       (TCS_PROTO_CC + 0x00)
#define TCS_CC_MSG_CALL_PROCEED   (TCS_PROTO_CC + 0x01)
#define TCS_CC_MSG_CONNECT        (TCS_PROTO_CC + 0x02)
#define TCS_CC_MSG_CONNECT_ACK    (TCS_PROTO_CC + 0x03)
#define TCS_CC_MSG_PROGRESS       (TCS_PROTO_CC + 0x04)
#define TCS_CC_MSG_SETUP          (TCS_PROTO_CC + 0x05)
#define TCS_CC_MSG_SETUP_ACK      (TCS_PROTO_CC + 0x06)
#define TCS_CC_MSG_DISCONNECT     (TCS_PROTO_CC + 0x07)
#define TCS_CC_MSG_RELEASE        (TCS_PROTO_CC + 0x08)
#define TCS_CC_MSG_REL_COMPLETE   (TCS_PROTO_CC + 0x09)
#define TCS_CC_MSG_INFORMATION    (TCS_PROTO_CC + 0x0A)
#define TCS_CC_MSG_START_DTMF     (TCS_PROTO_CC + 0x10)
#define TCS_CC_MSG_START_DTMF_ACK (TCS_PROTO_CC + 0x11)
#define TCS_CC_MSG_START_DTMF_REJ (TCS_PROTO_CC + 0x12)
#define TCS_CC_MSG_STOP_DTMF      (TCS_PROTO_CC + 0x13)
#define TCS_CC_MSG_STOP_DTMF_ACK  (TCS_PROTO_CC + 0x14)

/* Define the Group Management messages
*/
#define TCS_GM_MSG_INFO_SUG       (TCS_PROTO_GM + 0x00)
#define TCS_GM_MSG_INFO_ACPT      (TCS_PROTO_GM + 0x01)
#define TCS_GM_MSG_LISTEN_REQ     (TCS_PROTO_GM + 0x02)
#define TCS_GM_MSG_LISTEN_ACPT    (TCS_PROTO_GM + 0x03)
#define TCS_GM_MSG_LISTEN_SUG     (TCS_PROTO_GM + 0x04)
#define TCS_GM_MSG_LISTEN_REJ     (TCS_PROTO_GM + 0x05)
#define TCS_GM_MSG_ACCESS_REQ     (TCS_PROTO_GM + 0x06)
#define TCS_GM_MSG_ACCESS_ACPT    (TCS_PROTO_GM + 0x07)
#define TCS_GM_MSG_ACCESS_REJ     (TCS_PROTO_GM + 0x08)
#define TCS_GM_MSG_LISTEN_ACPT_ACK (TCS_PROTO_GM + 0x10)

/* Connectionless messages */
#define TCS_CL_MSG_INFO           (TCS_PROTO_CL + 0x00)


/* Define the types for all the information elements
*/
/* Information element identifier */
#define TCS_IE_LEN_MASK      0x80       /* if clear, the info is more > 2 bytes */
#define TCS_IE_DBL_MASK      0xC0       /* if set the info is double byte       */

#define TCS_IE_TYPE_SEND_COMP      0xA1
#define TCS_IE_TYPE_CALL_CLASS     0xC0
#define TCS_IE_TYPE_CAUSE          0xC1
#define TCS_IE_TYPE_PROG_IND       0xC2
#define TCS_IE_TYPE_SIGNAL         0xC3
#define TCS_IE_TYPE_KEYPAD_FAC     0xC4
#define TCS_IE_TYPE_SCO_HANDLE     0xC5
#define TCS_IE_TYPE_CLK_OFFSET     0x00
#define TCS_IE_TYPE_CFG_DATA       0x01
#define TCS_IE_TYPE_BEARER_CAP     0x02
#define TCS_IE_TYPE_DEST_CID       0x03
#define TCS_IE_TYPE_CALLING_NUM    0x04
#define TCS_IE_TYPE_CALLED_NUM     0x05
#define TCS_IE_TYPE_AUD_CTRL       0x06
#define TCS_IE_TYPE_COMPANY_SPEC   0x07
#define TCS_T303_TIMEOUT 0

#if TCS_LEAN == FALSE

#define TCS_T304_TIMEOUT 1
#define TCS_T310_TIMEOUT 2
#define TCS_T301_TIMEOUT 3
#define TCS_T302_TIMEOUT 4
#define TCS_T313_TIMEOUT 5
#define TCS_T305_TIMEOUT 6
#define TCS_T308_TIMEOUT 7

#else

#define TCS_T313_TIMEOUT 1
#define TCS_T305_TIMEOUT 2
#define TCS_T308_TIMEOUT 3

#endif

#define TCS_T401_TIMEOUT 1
#define TCS_T402_TIMEOUT 2
#define TCS_T403_TIMEOUT 3
#define TCS_T404_TIMEOUT 4
#define TCS_T405_TIMEOUT 5
#define TCS_T406_TIMEOUT 6

#define TCS_INIT_CD_TIMEOUT 9

/* Define the structure of the WUG configuration data information element.
** The data is an array, so the structure is not shown.
*/
typedef struct
{
    UINT8       len;
    UINT8       info[1];

} tTCS_IE_WUG_CONFIG;

#define TCS_IE_SEND_COMP_PRESENT      0x0001
#define TCS_IE_CALL_CLASS_PRESENT     0x0002
#define TCS_IE_CAUSE_PRESENT          0x0004
#define TCS_IE_PROG_IND_PRESENT       0x0008
#define TCS_IE_SIGNAL_PRESENT         0x0010
#define TCS_IE_KEYPAD_FAC_PRESENT     0x0020
#define TCS_IE_SCO_HANDLE_PRESENT     0x0040
#define TCS_IE_CLK_OFFSET_PRESENT     0x0080
#define TCS_IE_CFG_DATA_PRESENT       0x0100
#define TCS_IE_BEARER_CAP_PRESENT     0x0200
#define TCS_IE_DEST_CID_PRESENT       0x0400
#define TCS_IE_CALLING_NUM_PRESENT    0x0800
#define TCS_IE_CALLED_NUM_PRESENT     0x1000
#define TCS_IE_AUD_CTRL_PRESENT       0x2000
#define TCS_IE_COMPANY_SPEC_PRESENT   0x4000

/* Define a structure to hold the parsed IEs (all the logical IEs)
*/
typedef struct
{
    UINT8                   msg_type;
    UINT32                  ie_present_bitmap;

    tTCS_IE_AUDIO_CONTROL   audio_control;
    tTCS_IE_BEARER_CAP      bearer_cap;
    tTCS_CALL_CLASS         call_class;
    tTCS_IE_CALLED_NUM      called_num;
    tTCS_IE_CALLING_NUM     calling_num;
    UINT8                   cause;
    UINT16                  clock_offset;
    tTCS_IE_CO_SPEC         company_info;
    tTCS_IE_WUG_CONFIG      wug_cfg;
    tTCS_IE_KEYPAD_FACILITY keypad_fac;

#define TCS_PROG_DESCR_IN_BAND_AVAIL    0x08

    UINT8                   progress_ind;
    tTCS_BEARER_ID          bearer_id;
    tTCS_SIGNAL             signal;

} tTCS_CC_PARSED_MSG;


#endif
