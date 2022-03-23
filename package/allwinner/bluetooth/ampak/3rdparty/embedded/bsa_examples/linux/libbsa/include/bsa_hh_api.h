/*****************************************************************************
 **
 **  Name:          bsa_hh_api.h
 **
 **  Description:   This is the public interface file for HH part of
 **                 the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_HH_API_H
#define BSA_HH_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"
#include "bta_hh_api.h"
/* for BTA_AV_NUM_STRS */
#include "bta_av_api.h"
#include "uipc.h"
#include "bsa_sec_api.h"

/*****************************************************************************
 **  Constants and Type Definitions
 *****************************************************************************/
#ifndef BSA_HH_DEBUG
#define BSA_HH_DEBUG FALSE
#endif


/* BSA HID Host callback events */
typedef enum
{
    BSA_HH_OPEN_EVT, /* Connection Open*/
    BSA_HH_CLOSE_EVT, /* Connection Closed */
    BSA_HH_MIP_START_EVT, /* MIP started*/
    BSA_HH_MIP_STOP_EVT, /* MIP stopped */
    BSA_HH_GET_REPORT_EVT, /* BTA_HhGetReport callback */
    BSA_HH_SET_REPORT_EVT, /* BTA_HhSetReport callback */
    BSA_HH_GET_PROTO_EVT, /* BTA_HhGetProtoMode callback */
    BSA_HH_SET_PROTO_EVT, /* BTA_HhSetProtoMode callback */
    BSA_HH_GET_IDLE_EVT, /* BTA_HhGetIdle comes callback */
    BSA_HH_SET_IDLE_EVT, /* BTA_HhSetIdle finish callback */
    BSA_HH_GET_DSCPINFO_EVT, /* Get report descriptor callback */
    BSA_HH_VC_UNPLUG_EVT
/* virtually unplugged */
} tBSA_HH_EVT;

#define BSA_HH_MAX_KNOWN            HID_HOST_MAX_DEVICES

/* invalid device handle */
#define BSA_HH_INVALID_HANDLE       BTA_HH_INVALID_HANDLE

/* handle values for MIP */
#define BSA_HH_MIP_INVALID_HANDLE       BTA_HH_INVALID_HANDLE
#define BSA_HH_MIP_3DG_HANDLE           HID_HOST_MAX_DEVICES
#define BSA_HH_MIP_A2DP_HANDLE          (BTA_AV_NUM_STRS+1)

/* type of protocol mode */
#define BSA_HH_PROTO_RPT_MODE       BTA_HH_PROTO_RPT_MODE
#define BSA_HH_PROTO_BOOT_MODE      BTA_HH_PROTO_BOOT_MODE
#define BSA_HH_PROTO_UNKNOWN        BTA_HH_PROTO_UNKNOWN
typedef tBTA_HH_PROTO_MODE tBSA_HH_PROTO_MODE;

#define BSA_HH_KEYBD_RPT_ID         BSA_HH_KEYBD_RPT_ID
#define BSA_HH_MOUSE_RPT_ID         BSA_HH_MOUSE_RPT_ID
typedef tBTA_HH_BOOT_RPT_ID tBSA_HH_BOOT_RPT_ID;

/* type of devices, bit mask */
#define BSA_HH_DEVT_UNKNOWN         BTA_HH_DEVT_UNKNOWN
#define BSA_HH_DEVT_JOS             BTA_HH_DEVT_JOS    /* joy stick */
#define BSA_HH_DEVT_GPD             BTA_HH_DEVT_GPD    /* game pad */
#define BSA_HH_DEVT_RMC             BTA_HH_DEVT_RMC    /* remote control */
#define BSA_HH_DEVT_SED             BTA_HH_DEVT_SED    /* sensing device */
#define BSA_HH_DEVT_DGT             BTA_HH_DEVT_DGT    /* Digitizer tablet */
#define BSA_HH_DEVT_CDR             BTA_HH_DEVT_CDR    /* card reader */
#define BSA_HH_DEVT_KBD             BTA_HH_DEVT_KBD    /* keyboard */
#define BSA_HH_DEVT_MIC             BTA_HH_DEVT_MIC    /* pointing device */
#define BSA_HH_DEVT_COM             BTA_HH_DEVT_COM    /* Combo keyboard/pointing */
#define BSA_HH_DEVT_OTHER           BTA_HH_DEVT_OTHER
typedef tBTA_HH_DEVT tBSA_HH_DEVT;

/* Application IDs */
#define BSA_HH_APP_ID_MI            BTA_HH_APP_ID_MI
#define BSA_HH_APP_ID_KB            BTA_HH_APP_ID_KB
#define BSA_HH_APP_ID_RMC           BTA_HH_APP_ID_RMC
#define BSA_HH_APP_ID_3DSG          BTA_HH_APP_ID_3DSG
#define BSA_HH_APP_ID_GPAD          BTA_HH_APP_ID_GPAD

/* Attributes mask values to be used in HID_HostAddDev API */
#define BSA_HH_VIRTUAL_CABLE        HID_VIRTUAL_CABLE
#define BSA_HH_NORMALLY_CONNECTABLE HID_NORMALLY_CONNECTABLE
#define BSA_HH_RECONN_INIT          HID_RECONN_INIT
#define BSA_HH_SDP_DISABLE          HID_SDP_DISABLE
#define BSA_HH_BATTERY_POWER        HID_BATTERY_POWER
#define BSA_HH_REMOTE_WAKE          HID_REMOTE_WAKE
#define BSA_HH_SUP_TOUT_AVLBL       HID_SUP_TOUT_AVLBL
#define BSA_HH_SEC_REQUIRED         HID_SEC_REQUIRED
typedef tBTA_HH_ATTR_MASK tBSA_HH_ATTR_MASK;

/*
 * HID_CONTROL operation code used in BSA_HhGetReport and BSA_HhSetReport
 */
#define BSA_HH_RPTT_RESRV           BTA_HH_RPTT_RESRV   /* reserved */
#define BSA_HH_RPTT_INPUT           BTA_HH_RPTT_INPUT   /* input report */
#define BSA_HH_RPTT_OUTPUT          BTA_HH_RPTT_OUTPUT  /* output report */
#define BSA_HH_RPTT_FEATURE         BTA_HH_RPTT_FEATURE /* feature report */
typedef tBTA_HH_RPT_TYPE tBSA_HH_RPT_TYPE;

/*
 * HID_CONTROL operation code used in BSA_HhSendCtrl
 */
#define BSA_HH_CTRL_NOP                     BTA_HH_CTRL_NOP                     /* mapping from BTE */
#define BSA_HH_CTRL_HARD_RESET              BTA_HH_CTRL_HARD_RESET              /* hard reset */
#define BSA_HH_CTRL_SOFT_RESET              BTA_HH_CTRL_SOFT_RESET              /* soft reset */
#define BSA_HH_CTRL_SUSPEND                 BTA_HH_CTRL_SUSPEND                 /* enter suspend */
#define BSA_HH_CTRL_EXIT_SUSPEND            BTA_HH_CTRL_EXIT_SUSPEND            /* exit suspend */
#define BSA_HH_CTRL_VIRTUAL_CABLE_UNPLUG    BTA_HH_CTRL_VIRTUAL_CABLE_UNPLUG    /* virtual unplug */
typedef tBTA_HH_TRANS_CTRL_TYPE tBSA_HH_TRANS_CTRL_TYPE;

/*
 * Control/Shift/ALT and window key
 */
#define BSA_HH_MOD_CTRL_KEY         BTA_HH_MOD_CTRL_KEY
#define BSA_HH_MOD_SHFT_KEY         BTA_HH_MOD_SHFT_KEY
#define BSA_HH_MOD_ALT_KEY          BTA_HH_MOD_ALT_KEY
#define BSA_HH_MOD_GUI_KEY          BTA_HH_MOD_GUI_KEY
#define BSA_HH_MOD_MAX_KEY          BTA_HH_MOD_MAX_KEY

/*
 * Bit field used to enable Broadcom's Specific Features
 */
#define BSA_HH_NO_FEATURE           0x00    /* No Feature enabled */
#define BSA_HH_UCD_FEATURE          0x01    /* Unicast Connectionless Data */
#define BSA_HH_BRR_FEATURE          0x02    /* Broadcom Remote Reconnect */
#define BSA_HH_3DSG_FEATURE         0x04    /* Broadcom 3D Stereo Glasses */
typedef UINT8 tBSA_HH_FEATURE_MASK;

/*
 * Command Request for Set and Get operations
 */
#define BSA_HH_NO_REQ                0       /* No request  */
#define BSA_HH_PROTO_REQ             1       /* Request to Get/Set Proto Mode */
#define BSA_HH_IDLE_REQ              2       /* Request to Get/Set Idle Mode */
#define BSA_HH_REPORT_REQ            3       /* Request to Get/Set Report */
#define BSA_HH_DSCP_REQ              4       /* Request to Get/Set Report */
#define BSA_HH_3DSG_ON_REQ           5       /* Request to Enable 3D Stereo Glasses */
#define BSA_HH_3DSG_OFF_REQ          6       /* Request to Disable 3D Stereo Glasses */
#define BSA_HH_REPORT_NO_SIZE_REQ    7       /* Request to Get/Set Report without size info*/
#define BSA_HH_SET_PRIO_EXT_REQ      8       /* Request to Set Priority Ext */

typedef UINT8 tBSA_HH_REQ;

/*
 * The maximum size which can be sent to the server is limited by the
 * UIPC API implementation (based on socket or fifo).
 * If you need to increase this sizes, you have to increase BSA_MAX_MSG_SIZE
 *
 */
#define BSA_HH_DSCPINFO_SIZE_MAX    800

#ifndef BSA_HH_DATA_SIZE_MAX
#define BSA_HH_DATA_SIZE_MAX        800
#endif


#define BSA_HH_REPORT_SIZE_MAX      (HID_HOST_MTU - HID_HDR_LEN)

#ifndef BSA_HH_GET_REPORT_SIZE_MAX
#define BSA_HH_GET_REPORT_SIZE_MAX        BSA_HH_REPORT_SIZE_MAX
#endif

#define BSA_HH_BRR_DATA_SIZE        9

/*
 * Broadcom specific HH parameters
 */
#define BSA_HH_BRCM_TBFC_PAGE       0x01    /* Use TBFC Page instead of regular Page */
typedef UINT8 tBSA_HH_BRCM_MASK;

typedef UINT8 tBSA_HH_HANDLE;

typedef struct
{
    UINT16 length;
    UINT8 data[BSA_HH_REPORT_SIZE_MAX];
} tBSA_HH_REPORT_DATA;

typedef struct
{
    UINT16 vendor_id;
    UINT16 product_id;
    UINT16 version;
    UINT16 ssr_max_latency; /* SSR maximum latency from SDP record */
    UINT16 ssr_min_tout; /* SSR minimum timeout from SDP record */
    UINT16 supervision_tout; /* Supervision timeout from SDP record */
} tBSA_HH_PEERINFO;

typedef struct
{
    UINT16 length;
    UINT8 data[BSA_HH_DSCPINFO_SIZE_MAX];
} tBSA_HH_DSCPINFO;

typedef struct
{
    UINT16 length;
    UINT8 data[BSA_HH_DATA_SIZE_MAX];
} tBSA_HH_DATA;

typedef struct
{
    UINT8 data[BSA_HH_BRR_DATA_SIZE];
} tBSA_HH_BRR_CFG;
/*
 * Structures containing event message data
 */

/* callback event data for BSA_HH_OPEN_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    BD_ADDR bd_addr; /* HID device bd address */
    tBSA_HH_HANDLE handle; /* device handle */
    tUIPC_CH_ID uipc_channel; /* UIPC channel for Reports */
    tBSA_HH_BRR_CFG brr_cfg; /* BRR config data */
    tBSA_HH_PROTO_MODE mode; /* Operation mode (Report/Boot) */
    UINT8 sub_class;         /* sub class */
    tBSA_HH_ATTR_MASK attr_mask; /* attribute mask */
    BOOLEAN le_hid;          /* LE hid */
} tBSA_HH_OPEN_MSG;

/* callback event data for BSA_HH_CLOSE_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    tBSA_HH_HANDLE handle; /* device handle */
} tBSA_HH_CLOSE_MSG;

/* callback event data for BSA_HH_MIP_START_EVT event */
typedef struct
{
    tBSA_HH_HANDLE handle; /* device handle */
} tBSA_HH_MIP_START_MSG;

/* callback event data for BSA_HH_MIP_STOP_EVT event */
typedef struct
{
    tBSA_HH_HANDLE handle; /* device handle */
} tBSA_HH_MIP_STOP_MSG;


/* callback event data for BSA_HH_GET_RPT_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    tBSA_HH_HANDLE handle; /* device handle */
    tBSA_HH_REPORT_DATA report;
} tBSA_HH_GET_REPORT_MSG;

/* callback event data for BSA_HH_SET_RPT_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    tBSA_HH_HANDLE handle; /* device handle */
} tBSA_HH_SET_REPORT_MSG;

/* callback event data for BSA_HH_GET_PROTO_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    tBSA_HH_HANDLE handle; /* device handle */
    tBSA_HH_PROTO_MODE mode; /* Proto mode */
} tBSA_HH_GET_PROTO_MSG;

/* callback event data for BSA_HH_SET_PROTO_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    tBSA_HH_HANDLE handle; /* device handle */
} tBSA_HH_SET_PROTO_MSG;

/* callback event data for BSA_HH_GET_IDLE_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    tBSA_HH_HANDLE handle; /* device handle */
    UINT16 idle; /* Proto mode */
} tBSA_HH_GET_IDLE_MSG;

/* callback event data for BSA_HH_SET_IDLE_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    tBSA_HH_HANDLE handle; /* device handle */
} tBSA_HH_SET_IDLE_MSG;

/* callback event data for BSA_HH_GET_DSCPINFO_EVT */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    tBSA_HH_HANDLE handle; /* device handle */
    tBSA_HH_PEERINFO peerinfo;
    tBSA_HH_DSCPINFO dscpdata; /* DSCP Data */
} tBSA_HH_DSCPINFO_MSG;

/* callback event data for BSA_HH_VC_UNPLUG_EVT event */
typedef struct
{
    tBSA_STATUS status; /* operation status */
    tBSA_HH_HANDLE handle; /* device handle */
} tBSA_HH_VC_UNPLUG_MSG;

#if 0
/* parsed boot mode keyboard report */
typedef struct
{
    UINT8 this_char[6]; /* virtual key code */
    BOOLEAN mod_key[BSA_HH_MOD_MAX_KEY];
    /* ctrl, shift, Alt, GUI */
    /* modifier key: is Shift key pressed */
    /* modifier key: is Ctrl key pressed */
    /* modifier key: is Alt key pressed */
    /* modifier key: GUI up/down */
    BOOLEAN caps_lock; /* is caps locked */
    BOOLEAN num_lock; /* is Num key pressed */
}tBSA_HH_KEYBD_RPT;

/* parsed boot mode mouse report */
typedef struct
{
    UINT8 mouse_button; /* mouse button is clicked */
    INT8 delta_x; /* displacement x */
    INT8 delta_y; /* displacement y */
}tBSA_HH_MICE_RPT;

#define BSA_MAX_BOOT_RPT_SIZE 100

typedef struct
{
    UINT8 handle;
    UINT8 data[BSA_MAX_BOOT_RPT_SIZE];
    UINT16 length;
    tBSA_HH_PROTO_MODE mode;
    UINT8 sub_class;
    UINT8 ctry_code;
}tBSA_HH_RPT;

/* parsed Boot report */
typedef struct
{
    tBSA_HH_BOOT_RPT_ID dev_type; /* type of device report */
    union
    {
        tBSA_HH_KEYBD_RPT keybd_rpt; /* keyboard report */
        tBSA_HH_MICE_RPT mice_rpt;   /* mouse report */
    }data_rpt;
}tBSA_HH_PARSED_BOOT_RPT;

#endif

/* Union of data associated with HD callback */
typedef union
{
    tBSA_HH_OPEN_MSG open; /* BSA_HH_OPEN_EVT */
    tBSA_HH_CLOSE_MSG close; /* BSA_HH_CLOSE_EVT */
    tBSA_HH_GET_REPORT_MSG get_report; /* BSA_HH_GET_RPT_EVT */
    tBSA_HH_SET_REPORT_MSG set_report; /* BSA_HH_SET_RPT_EVT */
    tBSA_HH_GET_PROTO_MSG get_proto; /* BSA_HH_GET_PROTO_EVT */
    tBSA_HH_SET_PROTO_MSG set_proto; /* BSA_HH_SET_PROTO_EVT */
    tBSA_HH_GET_IDLE_MSG get_idle; /* BSA_HH_GET_IDLE_EVT */
    tBSA_HH_SET_IDLE_MSG set_idle; /* BSA_HH_SET_IDLE_EVT */
    tBSA_HH_DSCPINFO_MSG dscpinfo; /* BSA_HH_GET_DSCPINFO_EVT */
    tBSA_HH_VC_UNPLUG_MSG v_unplug; /* BSA_HH_VC_UNPLUG_EVT */
    tBSA_HH_MIP_START_MSG mip_start; /* BSA_HH_MIP_START_EVT */
    tBSA_HH_MIP_STOP_MSG mip_stop; /* BSA_HH_MIP_STOP_EVT */
} tBSA_HH_MSG;

/* BSA HH callback function */
typedef void (tBSA_HH_CBACK)(tBSA_HH_EVT event, tBSA_HH_MSG *p_data);

/*
 * Structure used to receive HID Reports via UIPC channel
 */
typedef struct
{
    UINT16 report_len;
    UINT8 handle;
    BD_ADDR bd_addr;
    tBSA_HH_PROTO_MODE mode;
    UINT8 sub_class;
    UINT8 ctry_code;
    tBSA_HH_REPORT_DATA report_data;
} tBSA_HH_REPORT;

/* data type HH report */
typedef struct
{
    BT_HDR hdr;
    tBSA_HH_REPORT report;
} tBSA_HH_UIPC_REPORT;

/* Structure used to send 3DSG offsets */
typedef struct
{
    UINT16 left_open;
    UINT16 left_close;
    UINT16 right_open;
    UINT16 right_close;
} tBSA_HH_3DSG_OFFSET;

/* Structure used to set priority with direction param */
typedef struct
{
    BD_ADDR bd_addr;
    UINT8 priority;
    UINT8 direction;
} tBSA_HH_SET_PRIO_EXT;

/*
 * Structures use to pass parameters to BSA API functions
 */
typedef struct
{
    tBSA_SEC_AUTH sec_mask;
    tBSA_HH_CBACK *p_cback;
    tUIPC_CH_ID uipc_channel;
} tBSA_HH_ENABLE;

typedef struct
{
    int dummy; /* May be needed for some compilers */
} tBSA_HH_DISABLE;

typedef struct
{
    BD_ADDR bd_addr;
    tBSA_HH_PROTO_MODE mode;
    tBSA_SEC_AUTH sec_mask;
    tBSA_HH_BRCM_MASK brcm_mask;
} tBSA_HH_OPEN;

typedef struct
{
    tBSA_HH_HANDLE handle;
} tBSA_HH_CLOSE;

//#if 0
typedef struct
{
    tBSA_HH_HANDLE handle;
    tBSA_HH_PROTO_MODE mode;
} tBSA_HH_SET_PROTO;

typedef struct
{
    tBSA_HH_HANDLE handle;
} tBSA_HH_GET_PROTO;

typedef struct
{
    tBSA_HH_HANDLE handle;
    tBSA_HH_RPT_TYPE report_type;
    tBSA_HH_REPORT_DATA report;
} tBSA_HH_SET_REPORT;

typedef struct
{
    tBSA_HH_HANDLE handle;
    tBSA_HH_RPT_TYPE report_type;
    UINT8 report_id;
} tBSA_HH_GET_REPORT;

typedef struct
{
    tBSA_HH_HANDLE handle;
    UINT16 idle;
} tBSA_HH_SET_IDLE;

typedef struct
{
    tBSA_HH_HANDLE handle;
} tBSA_HH_GET_IDLE;

typedef struct
{
    tBSA_HH_HANDLE handle;
} tBSA_HH_GET_DSCPINFO;

//#endif
/* This structure is used to Set ProtoMode, Report, Idle and 3DSG */
typedef struct
{
    tBSA_HH_REQ request;
    tBSA_HH_HANDLE handle;
    union
    {
        tBSA_HH_PROTO_MODE mode;
        struct
        {
            tBSA_HH_RPT_TYPE report_type;
            tBSA_HH_REPORT_DATA report;
        } set_report;
        UINT16 idle;
        tBSA_HH_3DSG_OFFSET offset_3dsg;
        tBSA_HH_SET_PRIO_EXT prio;
    } param;
} tBSA_HH_SET;

/* This structure is used to Get ProtoMode, Report, Idle, Dscp */
typedef struct
{
    tBSA_HH_REQ request;
    tBSA_HH_HANDLE handle;
    union
    {
        struct
        {
            tBSA_HH_RPT_TYPE report_type;
            UINT8 report_id;
        } get_report;
    } param;
} tBSA_HH_GET;


typedef struct
{
    tBSA_HH_HANDLE handle;
    tBSA_HH_TRANS_CTRL_TYPE ctrl_type;

} tBSA_HH_SEND_CTRL;

typedef struct
{
    tBSA_HH_HANDLE handle;
    tBSA_HH_RPT_TYPE report_type;
    tBSA_HH_DATA data;
} tBSA_HH_SEND_DATA;

typedef struct
{
    BD_ADDR bd_addr;
    tBSA_HH_ATTR_MASK attr_mask;
    UINT8 sub_class;
    tBSA_HH_PEERINFO peerinfo;
    tBSA_HH_FEATURE_MASK feature_mask;
    tBSA_HH_BRR_CFG brr_cfg; /* BRR config data */
    tBSA_HH_DSCPINFO dscp_data; /* DSCP Data */
    BOOLEAN enable_mip;
    tBSA_HH_HANDLE handle; /* Returned */
    BOOLEAN no_peer_info;
    UINT8 app_id;
} tBSA_HH_ADD_DEV;

typedef struct
{
    BD_ADDR bd_addr;
    BOOLEAN disable_mip;
} tBSA_HH_REMOVE_DEV;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/*******************************************************************************
 **
 ** Function         BSA_HhEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhEnableInit(tBSA_HH_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function         BSA_HhEnable
 **
 ** Description      This function enable HID host and registers HID-Host with
 **                  lower layers.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhEnable(tBSA_HH_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function         BSA_HhDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhDisableInit(tBSA_HH_DISABLE *p_enable);

/*******************************************************************************
 **
 ** Function         BSA_HhDisable
 **
 ** Description      This function is called when the host is about power down.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhDisable(tBSA_HH_DISABLE *p_enable);

/*******************************************************************************
 **
 ** Function         BSA_HhOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhOpenInit(tBSA_HH_OPEN *p_open);

/*******************************************************************************
 **
 ** Function         BSA_HhOpen
 **
 ** Description      This function is called to open an HH connection  to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhOpen(tBSA_HH_OPEN *p_open);

/*******************************************************************************
 **
 ** Function         BSA_HhCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhCloseInit(tBSA_HH_CLOSE *p_close);

/*******************************************************************************
 **
 ** Function         BSA_HhClose
 **
 ** Description      This function disconnects the device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhClose(tBSA_HH_CLOSE *p_close);

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
tBSA_STATUS BSA_HhSetProtoModeInit(tBSA_HH_SET_PROTO *p_set_proto);

/*******************************************************************************
 **
 ** Function         BSA_HhSetProtoMode
 **
 ** Description      This function set the protocol mode at specified HID handle
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetProtoMode(tBSA_HH_SET_PROTO *p_set_proto);

/*******************************************************************************
 **
 ** Function         BSA_HhGetProtoModeInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetProtoModeInit(tBSA_HH_GET_PROTO *p_get_proto);

/*******************************************************************************
 **
 ** Function         BSA_HhGetProtoMode
 **
 ** Description      This function get the protocol mode of a specified HID device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetProtoMode(tBSA_HH_GET_PROTO *p_get_proto);

/*******************************************************************************
 **
 ** Function         BSA_HhSetReportInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetReportInit(tBSA_HH_SET_REPORT *p_set_report);

/*******************************************************************************
 **
 ** Function         BSA_HhSetReport
 **
 ** Description      send SET_REPORT to device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetReport(tBSA_HH_SET_REPORT *p_set_report);

/*******************************************************************************
 **
 ** Function         BSA_HhGetReportInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetReportInit(tBSA_HH_GET_REPORT *p_get_report);

/*******************************************************************************
 **
 ** Function         BSA_HhGetReport
 **
 ** Description      Send a GET_REPORT to HID device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetReport(tBSA_HH_GET_REPORT *p_get_report);

/*******************************************************************************
 **
 ** Function         BSA_HhSetIdleInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetIdleInit(tBSA_HH_SET_IDLE *p_set_idle);

/*******************************************************************************
 **
 ** Function         BSA_HhSetIdle
 **
 ** Description      send SET_IDLE to device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetIdle(tBSA_HH_SET_IDLE *p_set_idle);

/*******************************************************************************
 **
 ** Function         BSA_HhGetIdleInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetIdleInit(tBSA_HH_GET_IDLE *p_get_idle);

/*******************************************************************************
 **
 ** Function         BSA_HhGetIdle
 **
 ** Description      Send a GET_IDLE to HID device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetIdle(tBSA_HH_GET_IDLE *p_get_idle);

/*******************************************************************************
 **
 ** Function         BSA_HhSendCtrlInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSendCtrlInit(tBSA_HH_SEND_CTRL *p_send_ctrl);

/*******************************************************************************
 **
 ** Function         BSA_HhSendCtrl
 **
 ** Description      Send HID_CONTROL request to a HID device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSendCtrl(tBSA_HH_SEND_CTRL *p_send_ctrl);

/*******************************************************************************
 **
 ** Function         BSA_HhSendDataInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSendDataInit(tBSA_HH_SEND_DATA *p_send_data);

/*******************************************************************************
 **
 ** Function         BSA_HhSendData
 **
 ** Description      Send DATA transaction to a HID device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSendData(tBSA_HH_SEND_DATA *p_send_data);

/*******************************************************************************
 **
 ** Function         BSA_HhGetDscpInfoInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetDscpInfoInit(tBSA_HH_GET_DSCPINFO *p_get_dscpinfo);

/*******************************************************************************
 **
 ** Function         BSA_HhGetDscpInfo
 **
 ** Description      Get report descriptor of the device
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetDscpInfo(tBSA_HH_GET_DSCPINFO *p_get_dscpinfo);

/*******************************************************************************
 **
 ** Function         BSA_HhGetInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetInit(tBSA_HH_GET *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HhGet
 **
 ** Description      HID Get request
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGet(tBSA_HH_GET *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HhSetInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetInit(tBSA_HH_SET *p_req);

/*******************************************************************************
 **
 ** Function         BSA_HhSet
 **
 ** Description      HID Set request
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSet(tBSA_HH_SET *p_req);


/*******************************************************************************
 **
 ** Function         BSA_HhAddDevInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhAddDevInit(tBSA_HH_ADD_DEV *p_add_dev);

/*******************************************************************************
 **
 ** Function         BSA_HhAddDev
 **
 ** Description      Add a virtually cabled device into HID-Host device list
 **                  to manage and assign a device handle for future API call,
 **                  host application call this API at start-up to initialize its
 **                  virtually cabled devices.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhAddDev(tBSA_HH_ADD_DEV *p_add_dev);

/*******************************************************************************
 **
 ** Function         BSA_HhRemoveDevInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhRemoveDevInit(tBSA_HH_REMOVE_DEV *p_rmv_dev);

/*******************************************************************************
 **
 ** Function        BSA_HhRemoveDev
 **
 ** Description     Remove a device from the HID host devices list.
 **
 ** Returns         tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhRemoveDev(tBSA_HH_REMOVE_DEV *p_rmv_dev);

/*******************************************************************************
 **
 **           Parsing Utility Functions
 **
 *******************************************************************************/
/*******************************************************************************
 **
 ** Function         BSA_HhParseBootRpt
 **
 ** Description      This utility function parse a boot mode report.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
#if 0
tBSA_STATUS BSA_HhParseBootRpt(tBSA_HH_BOOT_RPT *p_data, UINT8 *p_report,
        UINT16 report_len);
#endif

#ifdef __cplusplus
}
#endif

#endif
