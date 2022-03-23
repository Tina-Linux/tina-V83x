/******************************************************************************
**
**  File Name:   bta_hd_api.h
**
**  Description: This is the interface header file for the HID Device service.
**
**  Copyright (c) 2002-2004, WIDCOMM Inc., All Rights Reserved.
**  WIDCOMM Bluetooth Core. Proprietary and confidential.
**
******************************************************************************/

#ifndef BTA_HD_API_H
#define BTA_HD_API_H

#include "bt_types.h"
#include "bta_api.h"
#include "hidd_api.h"

/*****************************************************************************/
/*                            C O N S T A N T S                              */
/*****************************************************************************/
/* status values */
#define BTA_HD_SUCCESS          0       /* operation successful */
#define BTA_HD_FAIL             1       /* generic failure */
#define BTA_HD_FAIL_SDP         2       /* service not found */

typedef UINT8 tBTA_HD_STATUS;

/* HD callback events */
#define BTA_HD_ENABLE_EVT       0       /* HD enabled */
#define BTA_HD_OPEN_EVT         1       /* connection opened */
#define BTA_HD_CLOSE_EVT        2       /* connection closed */
#define BTA_HD_UNPLUG_EVT       3       /* unplug */
#define BTA_HD_DATA_EVT         4       /* Data received */
#define BTA_HD_DATC_EVT         5       /* Data continueation received */
#define BTA_HD_DISABLE_EVT      6       /* HD Disabled */ /* BSA_SPECIFIC */


typedef UINT8 tBTA_HD_EVT;

enum
{
    BTA_HD_REPT_ID_SPEC,        /* 0 */
    BTA_HD_REPT_ID_KBD,         /* 1: regular keyboard */
    BTA_HD_REPT_ID_MOUSE,       /* 2: mouse */
    BTA_HD_REPT_ID_CONSUMER
};
typedef UINT8 tBTA_HD_REPT_ID;
#define BTA_HD_REPT_ID_MAX     BTA_HD_REPT_ID_CONSUMER


#define BTA_HD_KBD_REPT_SIZE        9
#define BTA_HD_MOUSE_REPT_SIZE      5

#ifndef BTA_HD_CUST_REPT_SIZE
#define BTA_HD_CUST_REPT_SIZE      100 /* BSA_SPECIFIC */
#endif

/* Modifier Keys definition */
#define BTA_HD_MDF_LCTRL            0x01    /* Left CTRL */
#define BTA_HD_MDF_LSHIFT           0x02    /* Left SHIFT */
#define BTA_HD_MDF_LALT             0x04    /* Left ALT */
#define BTA_HD_MDF_LGUI             0x08    /* Left GUI */
#define BTA_HD_MDF_RCTRL            0x10    /* Right CTRL */
#define BTA_HD_MDF_RSHIFT           0x20    /* Right SHIFT */
#define BTA_HD_MDF_RALT             0x40    /* Right ALT */
#define BTA_HD_MDF_RGUI             0x80    /* Right GUI */

/* keycode definition -
 * See USB HID Usage Tables section 10: Keyboard/Keypad Page (0x07) */
#define BTA_HD_KEYCODE_A            0x04    /* a A */
#define BTA_HD_KEYCODE_B            0x05    /* b B */
#define BTA_HD_KEYCODE_C            0x06    /* c C */
#define BTA_HD_KEYCODE_D            0x07    /* d D */
#define BTA_HD_KEYCODE_E            0x08    /* e E */
#define BTA_HD_KEYCODE_F            0x09    /* f F */
#define BTA_HD_KEYCODE_G            0x0A    /* g G */
#define BTA_HD_KEYCODE_H            0x0B    /* h H */
#define BTA_HD_KEYCODE_I            0x0C    /* i I */
#define BTA_HD_KEYCODE_J            0x0D    /* j J */
#define BTA_HD_KEYCODE_K            0x0E    /* k K */
#define BTA_HD_KEYCODE_L            0x0F    /* l L */
#define BTA_HD_KEYCODE_M            0x10    /* m M */
#define BTA_HD_KEYCODE_N            0x11    /* n N */
#define BTA_HD_KEYCODE_O            0x12    /* o O */
#define BTA_HD_KEYCODE_P            0x13    /* p P */
#define BTA_HD_KEYCODE_Q            0x14    /* q Q */
#define BTA_HD_KEYCODE_R            0x15    /* r R */
#define BTA_HD_KEYCODE_S            0x16    /* s S */
#define BTA_HD_KEYCODE_T            0x17    /* t T */
#define BTA_HD_KEYCODE_U            0x18    /* u U */
#define BTA_HD_KEYCODE_V            0x19    /* v V */
#define BTA_HD_KEYCODE_W            0x1A    /* w W */
#define BTA_HD_KEYCODE_X            0x1B    /* x X */
#define BTA_HD_KEYCODE_Y            0x1C    /* y Y */
#define BTA_HD_KEYCODE_Z            0x1D    /* z Z */
#define BTA_HD_KEYCODE_1            0x1E    /* 1 ! */
#define BTA_HD_KEYCODE_2            0x1F    /* 2 @ */
#define BTA_HD_KEYCODE_3            0x20    /* 3 # */
#define BTA_HD_KEYCODE_4            0x21    /* 4 $ */
#define BTA_HD_KEYCODE_5            0x22    /* 5 % */
#define BTA_HD_KEYCODE_6            0x23    /* 6 ^ */
#define BTA_HD_KEYCODE_7            0x24    /* 7 & */
#define BTA_HD_KEYCODE_8            0x25    /* 8 * */
#define BTA_HD_KEYCODE_9            0x26    /* 9 ( */
#define BTA_HD_KEYCODE_0            0x27    /* 0 ) */
#define BTA_HD_KEYCODE_ENTER        0x28    /* ENTER     */
#define BTA_HD_KEYCODE_ESC          0x29    /* ESC       */
#define BTA_HD_KEYCODE_BACKSPACE    0x2A    /* BACKSPACE */
#define BTA_HD_KEYCODE_TAB          0x2B    /* TAB       */
#define BTA_HD_KEYCODE_SPACE        0x2C    /* SPACE     */
#define BTA_HD_KEYCODE_MINUS        0x2D    /* - _ */
#define BTA_HD_KEYCODE_EQUAL        0x2E    /* = + */
#define BTA_HD_KEYCODE_LBRACKET     0x2F    /* [ { */
#define BTA_HD_KEYCODE_RBRACKET     0x30    /* ] } */
#define BTA_HD_KEYCODE_BACKSLASH    0x31    /* \ | */
#define BTA_HD_KEYCODE_SEMICOLUMN   0x33    /* ; : */
#define BTA_HD_KEYCODE_QUOTE        0x34    /* ' " */
#define BTA_HD_KEYCODE_TILT         0x35    /* ` ~ */
#define BTA_HD_KEYCODE_COMMA        0x36    /* , < */
#define BTA_HD_KEYCODE_DIR          0x37    /* . > */
#define BTA_HD_KEYCODE_SLASH        0x38    /* / ? */
#define BTA_HD_KEYCODE_F1           0x3A    /* F1          */
#define BTA_HD_KEYCODE_F2           0x3B    /* F2          */
#define BTA_HD_KEYCODE_F3           0x3C    /* F3          */
#define BTA_HD_KEYCODE_F4           0x3D    /* F4          */
#define BTA_HD_KEYCODE_F5           0x3E    /* F5          */
#define BTA_HD_KEYCODE_F6           0x3F    /* F6          */
#define BTA_HD_KEYCODE_F7           0x40    /* F7          */
#define BTA_HD_KEYCODE_F8           0x41    /* F8          */
#define BTA_HD_KEYCODE_F9           0x42    /* F9          */
#define BTA_HD_KEYCODE_F10          0x43    /* F10         */
#define BTA_HD_KEYCODE_F11          0x44    /* F11         */
#define BTA_HD_KEYCODE_F12          0x45    /* F12         */
#define BTA_HD_KEYCODE_HOME         0x4A    /* HOME        */
#define BTA_HD_KEYCODE_PAGEUP       0x4B    /* PAGE UP     */
#define BTA_HD_KEYCODE_END          0x4D    /* END         */
#define BTA_HD_KEYCODE_PAGEDOWN     0x4E    /* PAGE DOWN   */
#define BTA_HD_KEYCODE_RIGHTARROW   0x4F    /* RIGHT ARROW */
#define BTA_HD_KEYCODE_LEFTARROW    0x50    /* LEFT ARROW  */
#define BTA_HD_KEYCODE_DOWNARROW    0x51    /* DOWN ARROW  */
#define BTA_HD_KEYCODE_UPARROW      0x52    /* UP ARROW    */
#define BTA_HD_KEYCODE_POPUP        0x65    /* POPUP Menu  */


#define BTA_HD_PROTOCOL_REPORT          HID_PAR_PROTOCOL_REPORT         /*(0x01) */
#define BTA_HD_PROTOCOL_BOOT            HID_PAR_PROTOCOL_BOOT_MODE      /*(0x00) */
typedef UINT8 tBTA_HD_PROTO;

/* data associated with BTA_HD_OPEN_EVT */
typedef struct
{
    BD_ADDR         bd_addr;
    tBTA_HD_PROTO   proto;
} tBTA_HD_OPEN;

/* data associated with BTA_HD_ENABLE_EVT */
typedef struct
{
    tBTA_HD_STATUS  status;
} tBTA_HD_ENABLE;

/* data associated with BTA_HD_CLOSE_EVT */
typedef struct
{
    BD_ADDR         bd_addr;
} tBTA_HD_CLOSE;

/* data associated with BTA_HD_REPORT_EVT */
typedef struct
{
    UINT8           *p_data;
    UINT16          len;
} tBTA_HD_REPORT;
/* BSA_SPECIFIC */
/* data associated with BTA_HD_DISABLE_EVT */
typedef struct
{
    tBTA_HD_STATUS  status;
} tBTA_HD_DISABLE;

/*****************************************************************************/
/*                   F U N C T I O N     P R O T O T Y P E S                 */
/*****************************************************************************/
/* union of data associated with HD callback */
typedef union
{
    tBTA_HD_ENABLE      enable;
    tBTA_HD_OPEN        open;
    tBTA_HD_CLOSE       close;
    tBTA_HD_REPORT      data;
    tBTA_HD_DISABLE     disable; /* BSA_SPECIFIC */
} tBTA_HD;

/* HD callback */
typedef void (tBTA_HD_CBACK)(tBTA_HD_EVT event, tBTA_HD *p_data);

/* HD configuration structure */
typedef struct
{
    tHID_DEV_QOS_INFO   qos;
    tHID_DEV_SDP_INFO   sdp_info;
    BOOLEAN             use_qos;    /* use QoS */
} tBTA_HD_CFG;

/*******************************************************************************
**
** Function         BTA_HdEnable
**
** Description      Enable the HID Device service. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_HD_ENABLE_EVT. This function must
**                  be called before other function in the HD API are
**                  called.
**
**                  If all bytes of the specified bd_addr are 0xff, the
**                  peer address is considered as unknown. The HID device listens
**                  for incoming connection request.
**                  Otherwise, The HID device initiates a connection toward the
**                  specified bd_addr when BTA_HdOpen() is called.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HdEnable(BD_ADDR bd_addr, tBTA_SEC sec_mask, const char *p_service_name,
                         tBTA_HD_CBACK *p_cback, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_HdDisable
**
** Description      Disable the HID Device service.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HdDisable(void);

/*******************************************************************************
**
** Function         BTA_HdOpen
**
** Description      Opens an HID Device connection to a peer device.
**                  When connection is open, callback function is called
**                  with a BTA_HD_OPEN_EVT.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HdOpen(tBTA_SEC sec_mask);

/*******************************************************************************
**
** Function         BTA_HdClose
**
** Description      Close the current connection a peer device.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HdClose(void);

/*******************************************************************************
**
** Function         BTA_HdSendRegularKey
**
** Description      Send a key report to the connected host.
**                  If auto_release is TRUE, assume the keyboard report must be
**                  a key press. An associated key release report is also sent.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HdSendRegularKey (UINT8 modifier, UINT8 key_code, BOOLEAN auto_release);

/*******************************************************************************
**
** Function         BTA_HdSendSpecialKey
**
** Description      Send a special key report to the connected host.
**                  The report is sent as a keyboard report.
**                  If auto_release is TRUE, assume the keyboard report must be
**                  a key press. An associated key release report is also sent.
**                  If key_len is less than BTA_HD_KBD_REPT_SIZE, the key_seq
**                  is padded with 0 until BTA_HD_KBD_REPT_SIZE.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HdSendSpecialKey (UINT8 key_len, UINT8 * key_seq, BOOLEAN auto_release);

/*******************************************************************************
**
** Function         BTA_HdSendMouseReport
**
** Description      Send a mouse report to the connected host
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_HdSendMouseReport (UINT8 is_left, UINT8 is_right, UINT8 is_middle,
                            INT8 delta_x, INT8 delta_y, INT8 delta_wheel);

/* BSA_SPECIFIC */
/*******************************************************************************
**
** Function         BTA_HdSendCustomerReport
**
** Description      Send a customer data report to the connected host.
**
** Returns          void
**
*******************************************************************************/

BTA_API void BTA_HdSendCustomerReport (UINT8 data_len, UINT8 * data);


#endif  /* BTA_HD_API_H */
