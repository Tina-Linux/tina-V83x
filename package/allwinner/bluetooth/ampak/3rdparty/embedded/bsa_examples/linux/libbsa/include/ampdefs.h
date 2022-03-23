/****************************************************************************
**
**  Name:       ampdefs.h
**
**   Function:   This file contains AMP protocol definitions
**
**
**
**  Copyright (c) 2008-2009, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef AMPDEFS_H
#define AMPDEFS_H

#include "hcidefs.h"
#include "l2cdefs.h"

/* Used for conformance testing ONLY:  When TRUE lets scriptwrapper overwrite info response */
#ifndef AMP_IOP_TESTING
#define AMP_IOP_TESTING             FALSE
#endif

#ifndef AMP_IOP_TEST_MOVE
#define AMP_IOP_TEST_MOVE           FALSE
#endif

/* AMP command codes
*/
#define AMP_CMD_REJECT                  0x01
#define AMP_DISCOVER_REQUEST            0x02
#define AMP_DISCOVER_RESPONSE           0x03
#define AMP_CHANGE_NOTIFY               0x04
#define AMP_CHANGE_RESPONSE             0x05
#define AMP_GET_INFO_REQUEST            0x06
#define AMP_GET_INFO_RESPONSE           0x07
#define AMP_GET_AMP_ASSOC_REQUEST       0x08
#define AMP_GET_AMP_ASSOC_RESPONSE      0x09
#define AMP_CREATE_PHYS_LINK_REQUEST    0x0A
#define AMP_CREATE_PHYS_LINK_RESPONSE   0x0B
#define AMP_DISCONN_PHYS_LINK_REQUEST   0x0C
#define AMP_DISCONN_PHYS_LINK_RESPONSE  0x0D
#define AMP_MIN_VALID_COMMAND_CODE      AMP_CMD_REJECT                  /* 0x01 */
#define AMP_MAX_VALID_COMMAND_CODE      AMP_DISCONN_PHYS_LINK_RESPONSE  /* 0x0D */

/* BR/EDR controller id for AMP Manager Protocol */
#define AMP_BR_EDR_CONTROLLER_ID        LOCAL_BR_EDR_CONTROLLER_ID

/* controller types for AMP Manager Protocol */
#define AMP_CONTROLLER_TYPE_BR_EDR      HCI_CONTROLLER_TYPE_BR_EDR
#define AMP_CONTROLLER_TYPE_802_11      HCI_CONTROLLER_TYPE_802_11
#define AMP_CONTROLLER_TYPE_ECMA        HCI_CONTROLLER_TYPE_ECMA
#define AMP_CTRLR_TYPE_MIN_VAL          AMP_CONTROLLER_TYPE_BR_EDR
#define AMP_CTRLR_TYPE_MAX_VAL          AMP_CONTROLLER_TYPE_ECMA

/* AMP command reject reason code
*/
#define AMP_CMD_REJ_NOT_RECOGNIZED                  0

/* AMP get info response status codes
*/
#define AMP_GET_INFO_RESP_SUCCESS                   0
#define AMP_GET_INFO_RESP_INVALID_CTRLR_ID          1

/* AMP get AMP ASSOC response status codes
*/
#define AMP_GET_AMP_ASSOC_RESP_SUCCESS              0
#define AMP_GET_AMP_ASSOC_INVALID_CTRLR_ID          1

/* AMP Create Phys Link response status codes
*/
#define AMP_CREATE_PHYS_LINK_SUCCESS_STARTED        0
#define AMP_CREATE_PHYS_LINK_INVALID_CTRLR_ID       1
#define AMP_CREATE_PHYS_LINK_FAIL_CANT_START        2
#define AMP_CREATE_PHYS_LINK_FAIL_CREATE_COLLISION  3
#define AMP_CREATE_PHYS_LINK_DISCONN_REQ_RCVD       4
#define AMP_CREATE_PHYS_LINK_LINK_ALREADY_EXISTS    5
#define AMP_CREATE_PHYS_LINK_FAIL_SEC_VIOLATION     6

/* AMP Cancel Phys Link response status codes
*/
#define AMP_DISCONN_PHYS_LINK_SUCCESS                   0
#define AMP_DISCONN_PHYS_LINK_INVALID_CTRLR_ID          1
#define AMP_DISCONN_PHYS_LINK_CREATE_NOT_IN_PROGRESS    2

/* Minimum MTU size
*/
#define AMP_MIN_MTU_SIZE    670

#define AMP_MY_EXTENDED_FEATURES_MASK   0

/* AMP controller status*/
#define AMP_CTRLR_STAT_POWER_DOWN           HCI_AMP_CTRLR_PHYSICALLY_DOWN
#define AMP_CTRLR_STAT_USABLE_BY_BT_ONLY    HCI_AMP_CTRLR_USABLE_BY_BT
#define AMP_CTRLR_STAT_NO_CAPAC_NOW         HCI_AMP_CTRLR_UNUSABLE_FOR_BT
#define AMP_CTRLR_STAT_LOW_CAP              HCI_AMP_CTRLR_LOW_CAP_FOR_BT
#define AMP_CTRLR_STAT_MED_CAP              HCI_AMP_CTRLR_MED_CAP_FOR_BT
#define AMP_CTRLR_STAT_HIGH_CAP             HCI_AMP_CTRLR_HIGH_CAP_FOR_BT
#define AMP_CTRLR_STAT_FULL_CAP             HCI_AMP_CTRLR_FULL_CAP_FOR_BT
#define AMP_CTRLR_STAT_MIN_VAL              AMP_CTRLR_STAT_POWER_DOWN
#define AMP_CTRLR_STAT_MAX_VAL              AMP_CTRLR_STAT_FULL_CAP

/* Move Channel Response result codes */
#define AMP_L2CAP_MOVE_OK                   L2CAP_MOVE_OK
#define AMP_L2CAP_MOVE_PENDING              L2CAP_MOVE_PENDING
#define AMP_L2CAP_MOVE_SECURITY_BLOCK       L2CAP_MOVE_SECURITY_BLOCK
#define AMP_L2CAP_MOVE_CTRL_ID_NOT_SUPPORT  L2CAP_MOVE_CTRL_ID_NOT_SUPPORT
#define AMP_L2CAP_MOVE_SAME_CTRLR_ID        L2CAP_MOVE_SAME_CTRLR_ID
#define AMP_L2CAP_MOVE_CONFIG_NOT_SUPPORTED L2CAP_MOVE_CONFIG_NOT_SUPPORTED
#define AMP_L2CAP_MOVE_CHAN_COLLISION       L2CAP_MOVE_CHAN_COLLISION
#define AMP_L2CAP_MOVE_NOT_ALLOWED          L2CAP_MOVE_NOT_ALLOWED

/* Move Channel Confirmation result codes */
#define AMP_L2CAP_MOVE_OK_BY_INITIATOR      L2CAP_MOVE_CFM_OK
#define AMP_L2CAP_MOVE_REFUSED_BY_INITIATOR L2CAP_MOVE_CFM_REFUSED

#define AMP_L2CAP_MOVE_ON_HOLD              0xffff
#endif
