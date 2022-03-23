/*****************************************************************************
 **
 **  Name:           nsa_dm_api.h
 **
 **  Description:    This is the public interface file for the NFC DM subsystem of
 **                  BSA layer.
 **
 **  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_NSA_API_H
#define BSA_NSA_API_H

#include "data_types.h"

/* For the moment, only IPC (socket based) interfaces defined */
#define BSA_NSA_TYPE_HCI_LOCAL  0   /* BSA & NSA are compiled together (Combo chip) */
#define BSA_NSA_TYPE_HID_LOCAL  1   /* BSA & NSA are compiled together */
#define BSA_NSA_TYPE_HCI_IPC    2   /* NSA Server will use our HCI (Combo chip) */
#define BSA_NSA_TYPE_HID_IPC    3   /* NSA Server will use our HID (HID encapsulation) */
typedef UINT8 tBSA_NSA_TYPE;

#define BSA_NSA_PORT_BAD        0xFF    /* Bad Port */

typedef UINT8 tBSA_NSA_PORT;        /* NSA Interface Port */

/* Defines the NSA HID configuration */
typedef struct
{
    BD_ADDR bd_addr;                /* BdAddr of the HID device */
    UINT8   out_report_id;          /* ReportId used to encapsulate NCI Tx packet */
    UINT8   in_report_id;           /* ReportId used to encapsulate NCI Rx packet */
} tBSA_NSA_HID;

typedef struct
{
    tBSA_NSA_TYPE   type;   /* Type of the interface to configure */
    union
    {
        tBSA_NSA_HID hid_ipc;   /* If type is BSA_NSA_TYPE_HID_IPC */
    } nsa_config;
    tBSA_NSA_PORT   port;   /* OUT: NSA port allocated */
} tBSA_NSA_ADD_IF;

typedef struct
{
    tBSA_NSA_PORT   port;   /* NSA port to remove */
} tBSA_NSA_REMOVE_IF;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function         BSA_NsaAddInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_NsaAddIfInit(tBSA_NSA_ADD_IF *p_req);

/*******************************************************************************
 **
 ** Function         BSA_NsaAddIf
 **
 ** Description      This function adds an NSA Interface
 **
 ** Returns          tNSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_NsaAddIf(tBSA_NSA_ADD_IF *p_req);

/*******************************************************************************
 **
 ** Function         BSA_NsaRemoveIfInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_NsaRemoveIfInit(tBSA_NSA_REMOVE_IF *p_req);

/*******************************************************************************
 **
 ** Function         BSA_NsaRemove
 **
 ** Description      This function removes an NSA Interface
 **
 ** Returns          tNSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_NsaRemoveIf(tBSA_NSA_REMOVE_IF *p_req);

#ifdef __cplusplus
}
#endif

#endif /* BSA_NSA_API_H */
