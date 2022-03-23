/*****************************************************************************
 **
 **  Name:          bsa_hd_api.h
 **
 **  Description:   This is the public interface file for HD part of
 **                 the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2015, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_HD_API_H
#define BSA_HD_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"
#include "bta_hd_api.h"
#include "bsa_sec_api.h"

/*****************************************************************************
 **  Constants and Type Definitions
 *****************************************************************************/
#ifndef BSA_HD_DEBUG
#define BSA_HD_DEBUG FALSE
#endif


/* BSA HID Device callback events */
typedef enum
{
    BSA_HD_OPEN_EVT, /* Connection Open*/
    BSA_HD_CLOSE_EVT, /* Connection Closed */
    BSA_HD_DATA_EVT,
    BSA_HD_DATC_EVT,
    BSA_HD_UNPLUG_EVT   /*Unplugged */
} tBSA_HD_EVT;

typedef enum
{
    BSA_HD_REGULAR_KEY,
    BSA_HD_SPECIAL_KEY,
    BSA_HD_MOUSE_DATA,
    BSA_HD_CUSTOMER_DATA
} tBSA_HD_KEY_TYPE;

/* invalid device handle */
#define BSA_HD_INVALID_HANDLE       BTA_HD_INVALID_HANDLE

#define BSA_HD_MAX_KEY_SEQ_SIZE          BTA_HD_KBD_REPT_SIZE
#define BSA_HD_MAX_CUSTOMER_DATA_SIZE    BTA_HD_CUST_REPT_SIZE

#define BSA_HD_MAX_REPORT_SIZE      (HID_DEV_MTU_SIZE - HID_HDR_LEN)

/*
 * Control/Shift/ALT and window key
 */

/* Modifier Keys definition */
#define BSA_HD_MDF_LCTRL_KEY         BTA_HD_MDF_LCTRL    /* Left CTRL */
#define BSA_HD_MDF_LSHFT_KEY         BTA_HD_MDF_LSHIFT   /* Left SHIFT */
#define BSA_HD_MDF_LALT_KEY          BTA_HD_MDF_LALT     /* Left ALT */
#define BSA_HD_MDF_LGUI_KEY          BTA_HD_MDF_LGUI     /* Left GUI */
#define BSA_HD_MDF_RCTRL_KEY         BTA_HD_MDF_RCTRL    /* Right CTRL */
#define BSA_HD_MDF_RSHFT_KEY         BTA_HD_MDF_RSHIFT   /* Right SHIFT */
#define BSA_HD_MDF_RALT_KEY          BTA_HD_MDF_RALT     /* Right ALT */
#define BSA_HD_MDF_RGUI_KEY          BTA_HD_MDF_RGUI     /* Right GUI */

/*
 * Structures containing event message data
 */

/* callback event data for BSA_HD_OPEN_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    BD_ADDR bd_addr; /* HID host bd address */
} tBSA_HD_OPEN_MSG;

/* callback event data for BSA_HD_CLOSE_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    BD_ADDR bd_addr; /* HID host bd address */
} tBSA_HD_CLOSE_MSG;

/* callback event data for BSA_HD_VC_UNPLUG_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
} tBSA_HD_UNPLUG_MSG;

/* callback event data for BSA_HD_REPORT_EVT event */
typedef struct
{
    UINT8    data[BSA_HD_MAX_REPORT_SIZE];
    UINT16   len;
    tBSA_STATUS status; /* operation status */
} tBSA_HD_DATA_MSG;

/* Union of data associated with HD callback */
typedef union
{
    tBSA_HD_OPEN_MSG open; /* BSA_HD_OPEN_EVT */
    tBSA_HD_CLOSE_MSG close; /* BSA_HD_CLOSE_EVT */
    tBSA_HD_UNPLUG_MSG unplug; /* BSA_HD_UNPLUG_EVT */
    tBSA_HD_DATA_MSG data; /* BSA_HD_DATA_EVT */
} tBSA_HD_MSG;

/* BSA HD callback function */
typedef void (tBSA_HD_CBACK)(tBSA_HD_EVT event, tBSA_HD_MSG *p_data);


/*
 * Structures use to pass parameters to BSA API functions
 */
typedef struct
{
    tBSA_SEC_AUTH sec_mask;
    BD_ADDR bd_addr;
    tBSA_HD_CBACK *p_cback;
} tBSA_HD_ENABLE;

typedef struct
{
    int dummy; /* May be needed for some compilers */
} tBSA_HD_DISABLE;

typedef struct
{
    tBSA_SEC_AUTH sec_mask;
} tBSA_HD_OPEN;

typedef struct
{
    int dummy; /* May be needed for some compilers */
} tBSA_HD_CLOSE;

typedef struct
{
    UINT8    modifier;
    UINT8    key_code;
    BOOLEAN  auto_release;
} tBSA_HD_SEND_REGULAR;

typedef struct
{
    UINT8    key_len;
    UINT8    key_seq[BSA_HD_MAX_KEY_SEQ_SIZE];
    BOOLEAN  auto_release;
} tBSA_HD_SEND_SPECIAL;

typedef struct
{
    UINT8    is_left;
    UINT8    is_right;
    UINT8    is_middle;
    UINT8    delta_x;
    UINT8    delta_y;
    UINT8    delta_wheel;
} tBSA_HD_SEND_MOUSE;

typedef struct
{
    UINT8    data_len;
    UINT8    data[BSA_HD_MAX_CUSTOMER_DATA_SIZE];
} tBSA_HD_SEND_CUSTOMER;

typedef struct
{
    UINT8    key_type;
    union
    {
        tBSA_HD_SEND_REGULAR    reg_key;
        tBSA_HD_SEND_SPECIAL    sp_key;
        tBSA_HD_SEND_MOUSE      mouse;
        tBSA_HD_SEND_CUSTOMER   customer;
    } param;
} tBSA_HD_SEND;



/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/*******************************************************************************
 **
 ** Function         BSA_HdEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdEnableInit(tBSA_HD_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function         BSA_HdEnable
 **
 ** Description      This function enable HID device and registers HID-Device with
 **                  lower layers.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdEnable(tBSA_HD_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function         BSA_HdDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdDisableInit(tBSA_HD_DISABLE *p_disble);

/*******************************************************************************
 **
 ** Function         BSA_HdDisable
 **
 ** Description      This function is called when the host is about power down.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdDisable(tBSA_HD_DISABLE *p_disable);

/*******************************************************************************
 **
 ** Function         BSA_HdOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdOpenInit(tBSA_HD_OPEN *p_open);

/*******************************************************************************
 **
 ** Function         BSA_HdOpen
 **
 ** Description      This function is called to open an HH connection  to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdOpen(tBSA_HD_OPEN *p_open);

/*******************************************************************************
 **
 ** Function         BSA_HdCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdCloseInit(tBSA_HD_CLOSE *p_close);

/*******************************************************************************
 **
 ** Function         BSA_HdClose
 **
 ** Description      This function disconnects the device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdClose(tBSA_HD_CLOSE *p_close);

/*******************************************************************************
 **
 ** Function         BSA_HhSetProtoModeInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdSendInit(tBSA_HD_SEND *p_send);

/*******************************************************************************
 **
 ** Function         BSA_HdSend
 **
 ** Description      This function send report to HID Host
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdSend(tBSA_HD_SEND *p_send);
#ifdef __cplusplus
}
#endif

#endif
