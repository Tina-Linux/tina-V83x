/*****************************************************************************
 **
 **  Name:           bsa_disc_api.h
 **
 **  Description:    This is the public interface file for discovery part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_DISC_API_H
#define BSA_DISC_API_H

/* for BT and BTA types */
#include "bta_api.h"
/* for tBSA_STATUS */
#include "bsa_status.h"
/* for security types */
#include "bsa_sec_api.h"
/*
 * Discovery data and messages
 */

/* Discovery callback events */
typedef enum
{
    BSA_DISC_NEW_EVT, /* a New Device has been discovered */
    BSA_DISC_CMPL_EVT, /* End of Discovery */
    BSA_DISC_DEV_INFO_EVT, /* Device Info discovery event */
    BSA_DISC_REMOTE_NAME_EVT  /* Read remote device name event */
} tBSA_DISC_EVT;

/* VID definitions */
#define BSA_DISC_VID_SRC_BT     0x01    /* The VID is a Bluetooth Company Id */
#define BSA_DISC_VID_SRC_USB    0x02    /* The VID is an USB Vendor Id */

/* Maximum number of Vid/Pid returned */
#define BSA_DISC_VID_PID_MAX    1

/* Vendor and Product Identification of the peer device */
typedef struct
{
    BOOLEAN valid; /* TRUE if this entry is valid */
    UINT16 vendor_id_source; /* Indicate if the vendor field is BT or USB */
    UINT16 vendor; /* Vendor Id of the peer device */
    UINT16 product; /* Product Id of the peer device */
    UINT16 version; /* Version of the peer device */
} tBSA_DISC_VID_PID;

#define BSA_EIR_DATA_LENGTH     HCI_EXT_INQ_RESPONSE_LEN

typedef struct
{
    BD_ADDR bd_addr; /* BD address peer device. */
    DEV_CLASS class_of_device; /* Class of Device */
    BD_NAME name; /* Name of peer device. */
    int rssi; /* The rssi value */
    tBSA_SERVICE_MASK services; /* Service discovery discovered */
    tBSA_DISC_VID_PID eir_vid_pid[BSA_DISC_VID_PID_MAX];
    UINT8 eir_data[BSA_EIR_DATA_LENGTH];  /* Full EIR data */
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    UINT8 inq_result_type;
    UINT8 ble_addr_type;
    tBT_DEVICE_TYPE device_type;
#endif
} tBSA_DISC_REMOTE_DEV;

typedef struct
{
    BOOLEAN in_use; /* TRUE is this element is used, FALSE otherwise */
    tBSA_DISC_REMOTE_DEV device; /* Device Info */
} tBSA_DISC_DEV;

typedef struct
{
    tBSA_STATUS status; /* Status of the request */
    BD_ADDR bd_addr;    /* BD address peer device. */
    UINT8 index;
    UINT16 spec_id;
    BOOLEAN primary;
    UINT16 vendor;
    UINT16 vendor_id_source;
    UINT16 product;
    UINT16 version;
} tBSA_DISC_DEV_INFO_MSG;

typedef struct
{
    UINT16      status;
    BD_ADDR     bd_addr;
    UINT16      length;
    BD_NAME     remote_bd_name;
} tBSA_DISC_READ_REMOTE_NAME_MSG;

/* Structure associated with BSA_DISC_NEW_EVT */
typedef tBSA_DISC_REMOTE_DEV tBSA_DISC_NEW_MSG;

/* Union of all Discovery callback structures */
typedef union
{
    tBSA_DISC_NEW_MSG disc_new; /* a New Device has been discovered */
    tBSA_DISC_DEV_INFO_MSG dev_info; /* Device Info of a device */
    tBSA_DISC_READ_REMOTE_NAME_MSG remote_name; /* Name of Remote device */
} tBSA_DISC_MSG;

/* Discovery callback */
typedef void ( tBSA_DISC_CBACK)(tBSA_DISC_EVT event, tBSA_DISC_MSG *p_data);

/*
 * Structures
 */

/*
 * This structure is used to ask the BT stack to discover (inquiry & remote_name)
 * If the cback field is NULL, the BSA_DmDiscoverStart will be blocking
 * If it's not null, the callback fct will be called once for each new
 * device discovered and at the end of the discovery process.
 * The device field will be used only in blocking mode (cback = NULL)
 */
#define BSA_DM_INQ_CLR          BTA_DM_INQ_CLR          /* Clear inquiry filter. */
#define BSA_DM_INQ_DEV_CLASS    BTA_DM_INQ_DEV_CLASS  /* Filter on device class. */
#define BSA_DM_INQ_BD_ADDR      BTA_DM_INQ_BD_ADDR    /* Filter on a specific  BD address. */
typedef tBTA_DM_INQ_FILT        tBSA_DM_INQ_FILT;/* Filter condition type. */

typedef tBTA_DM_INQ_COND        tBSA_DM_INQ_COND;    /* Filter condition data. */

/* Inquiry Modes */
#define BSA_DM_GENERAL_INQUIRY  BTA_DM_GENERAL_INQUIRY           /* Perform general inquiry. */
#define BSA_DM_LIMITED_INQUIRY  BTA_DM_LIMITED_INQUIRY           /* Perform limited inquiry. */

#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
#define BSA_BLE_INQUIRY_NONE    BTA_BLE_INQUIRY_NONE
#define BSA_BLE_GENERAL_INQUIRY BTA_BLE_GENERAL_INQUIRY    /* Perform LE general inquiry. */
#define BSA_BLE_LIMITED_INQUIRY BTA_BLE_LIMITED_INQUIRY    /* Perform LE limited inquiry. */
#endif

typedef tBTA_DM_INQ_MODE        tBSA_DM_INQ_MODE;

/* Proprietary Discovery filters */
#define BSA_DISC_BRCM_3DTV_MASTER_FILTER  0x01    /* Filter on 3DTV Master devices */
typedef UINT8 tBSA_DISC_BRCM_FILTER;

/* Update parameter of the Discovery Start API */
/* To indicate when BSA_DISC_NEW_EVT must be received */
#define BSA_DISC_UPDATE_END     0   /* Send update at the end of the process (default) */
#define BSA_DISC_UPDATE_ANY     1   /* Send update upon any modification (name, rssi, etc) */
#define BSA_DISC_UPDATE_NAME    2   /* Send update when Name is available */
typedef UINT8                   tBSA_DISC_UPDATE;

/* Inquiry Parameters */
typedef struct
{
    int duration;                       /* Multiple of 1.28 seconds */
    tBSA_DISC_CBACK *cback;             /* Callback for new devices found and completion */
    int nb_devices;                     /* Max number of elements in devices table */
    tBSA_DM_INQ_FILT filter_type;       /* Filter condition type */
    tBSA_DM_INQ_COND filter_cond;       /* Filter condition data */
    tBSA_SERVICE_MASK services;         /* Services to look for */
    tBSA_DM_INQ_MODE mode;              /* Inquiry mode */
    BOOLEAN device_info;                /* */
    tBSA_DISC_BRCM_FILTER brcm_filter;  /* Broadcom specific filters */
    BOOLEAN skip_name_request;          /* If TRUE, server will not perform any remote name request */
    tBSA_DISC_UPDATE update;            /* Update mode */
    INT8 inq_tx_power;                  /* Inquiry transmit power */
} tBSA_DISC_START;

typedef struct
{
    int dummy; /* May be needed for some compilers */
} tBSA_DISC_ABORT;

/* Device Info parameters */
typedef struct
{
    BD_ADDR bd_addr; /* BD address peer device */
    tBSA_DISC_CBACK *cback; /* Callback for Device Info */
} tBSA_DISC_DEV_INFO;

/* Read Remote Device Name parameters */
typedef struct
{
    BD_ADDR bd_addr; /* BD address peer device */
    tBSA_DISC_CBACK *cback; /* Callback for Remote name */
    tBSA_TRANSPORT transport;
}tBSA_DISC_READ_REMOTE_NAME;
/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function         BSA_DiscStartInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DiscStartInit(tBSA_DISC_START *p_disc_start);

/*******************************************************************************
 **
 ** Function       BSA_DiscStart
 **
 ** Description    Discovers devices and associated services.
 **                Table element's with in_use field set to FALSE will be filled
 **                with discovered device's parameters.
 **                The in_use fields will be set to TRUE
 **
 ** Parameters     pointer on discovery parameters structure
 **
 ** Returns        tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DiscStart(tBSA_DISC_START *p_disc_start);

/*******************************************************************************
 **
 ** Function         BSA_DiscAbortInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DiscAbortInit(tBSA_DISC_ABORT*p_disc_abort);

/*******************************************************************************
 **
 ** Function       BSA_DiscAbort
 **
 ** Description    Cancels a device Discovery request
 **
 ** Parameters     Pointer on structure containing API parameters
 **
 ** Returns        tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DiscAbort(tBSA_DISC_ABORT *p_disc_abort);

/*******************************************************************************
**
** Function         BSA_ReadRemoteNameInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_ReadRemoteNameInit(tBSA_DISC_READ_REMOTE_NAME *p_read_remote_name);

/*******************************************************************************
**
** Function       BSA_ReadRemoteName
**
** Description    Read Remote Device Name
**
** Parameters     Pointer on structure containing API parameters
**
** Returns        tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_ReadRemoteName(tBSA_DISC_READ_REMOTE_NAME *p_read_remote_name);

#ifdef __cplusplus
}
#endif

#endif
