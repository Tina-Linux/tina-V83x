/*****************************************************************************
**
**  Name:           bta_rc_api.h
**
**  Description:    BTA stand-alone AVRC API (BTA_RC).
**
**                  BTA_RC is intended for use platforms that do not include
**                  AV source (BTA_AV) nor AV sink (BTA_AVK).
**
**                  For platforms that include the BTA_AV or BTA_AVK module,
**                  the RC APIs from those respective modules should be
**                  used instead.
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_RC_API_H
#define BTA_RC_API_H

#include "avrc_api.h"
#include "avdt_api.h"
#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* Feature mask definitions (for BTA_RcEnable) */
#define BTA_RC_FEAT_CONTROL         0x0001  /* controller role */
#define BTA_RC_FEAT_TARGET          0x0002  /* target role */
#define BTA_RC_FEAT_VENDOR          0x0004  /* vendor dependent commands */
#define BTA_RC_FEAT_METADATA        0x0008  /* metadata Transfer */
#define BTA_RC_FEAT_BROWSE          0x0010  /* browsing channel support */
#define BTA_RC_FEAT_INT             0x0020  /* initiator */
#define BTA_RC_FEAT_ACP             0x0040  /* acceptor */
typedef UINT32  tBTA_RC_FEAT;

/* Reserved value for invalid bta_rc handle */
#define BTA_RC_INVALID_HANDLE   0xFF

/* Default subunit extension code for use with BTA_RcSubunitInfoCmd */
#define BTA_RC_SUBUNIT_DEFAULT_EXTENSION_CODE   0x07

/* Event definitions for tBTA_RC_CBACK (common CT/TG events) */
#define BTA_RC_ENABLE_EVT               1       /* AVRC enabled */
#define BTA_RC_DISABLE_EVT              2       /* AVRC disabled */
#define BTA_RC_OPEN_ACP_EVT             3       /* AVRC acceptor opened */
#define BTA_RC_CLOSE_ACP_EVT            4       /* AVRC acceptor closed */
#define BTA_RC_CONNECTED_EVT            5       /* AVRC connected */
#define BTA_RC_DISCONNECTED_EVT         6       /* AVRC disconncted */

/* Event definitons for tBTA_RC_CBACK (CT events) */
#define BTA_RC_UNIT_INFO_RSP_EVT        7       /* Received response for UNIT INFO command */
#define BTA_RC_SUBUNIT_INFO_RSP_EVT     8       /* Received response for SUBUNIT INFO command */
#define BTA_RC_PASS_THRU_RSP_EVT        9       /* Received response for PASSTHROUGH command */
#define BTA_RC_VENDOR_RSP_EVT           10      /* Received response for VENDOR DEPENDENT command */
#define BTA_RC_METADATA_RSP_EVT         11      /* Received response for METADATA command */
#define BTA_RC_BROWSE_RSP_EVT           12      /* Received response for BROWSING command */
#define BTA_RC_CMD_TOUT_EVT             13      /* Timeout waiting for command response */

/* Event definitons for tBTA_RC_CBACK (TG events) */
#define BTA_RC_UNIT_INFO_CMD_EVT        14      /* Received UNIT INFO command */
#define BTA_RC_SUBUNIT_INFO_CMD_EVT     15      /* Received SUBUNIT INFO command */
#define BTA_RC_PASS_THRU_CMD_EVT        16      /* Received PASSTHROUGH command */
#define BTA_RC_VENDOR_CMD_EVT           17      /* Received VENDOR DEPENDENT command */
#define BTA_RC_METADATA_CMD_EVT         18      /* Received PASS THROUGH command */
#define BTA_RC_BROWSE_CMD_EVT           19      /* Received BROWSING command */

#define BTA_RC_PEER_FEAT_EVT            20      /* Peer features notification (sdp results for peer initiated AVRC connection) */

typedef UINT8 tBTA_RC_EVT;


/* operation id list for BTA_RcPassThroughCmd */
#define BTA_RC_OP_SELECT        AVRC_ID_SELECT      /* select */
#define BTA_RC_OP_UP            AVRC_ID_UP          /* up */
#define BTA_RC_OP_DOWN          AVRC_ID_DOWN        /* down */
#define BTA_RC_OP_LEFT          AVRC_ID_LEFT        /* left */
#define BTA_RC_OP_RIGHT         AVRC_ID_RIGHT       /* right */
#define BTA_RC_OP_RIGHT_UP      AVRC_ID_RIGHT_UP    /* right-up */
#define BTA_RC_OP_RIGHT_DOWN    AVRC_ID_RIGHT_DOWN  /* right-down */
#define BTA_RC_OP_LEFT_UP       AVRC_ID_LEFT_UP     /* left-up */
#define BTA_RC_OP_LEFT_DOWN     AVRC_ID_LEFT_DOWN   /* left-down */
#define BTA_RC_OP_ROOT_MENU     AVRC_ID_ROOT_MENU   /* root menu */
#define BTA_RC_OP_SETUP_MENU    AVRC_ID_SETUP_MENU  /* setup menu */
#define BTA_RC_OP_CONT_MENU     AVRC_ID_CONT_MENU   /* contents menu */
#define BTA_RC_OP_FAV_MENU      AVRC_ID_FAV_MENU    /* favorite menu */
#define BTA_RC_OP_EXIT          AVRC_ID_EXIT        /* exit */
#define BTA_RC_OP_0             AVRC_ID_0           /* 0 */
#define BTA_RC_OP_1             AVRC_ID_1           /* 1 */
#define BTA_RC_OP_2             AVRC_ID_2           /* 2 */
#define BTA_RC_OP_3             AVRC_ID_3           /* 3 */
#define BTA_RC_OP_4             AVRC_ID_4           /* 4 */
#define BTA_RC_OP_5             AVRC_ID_5           /* 5 */
#define BTA_RC_OP_6             AVRC_ID_6           /* 6 */
#define BTA_RC_OP_7             AVRC_ID_7           /* 7 */
#define BTA_RC_OP_8             AVRC_ID_8           /* 8 */
#define BTA_RC_OP_9             AVRC_ID_9           /* 9 */
#define BTA_RC_OP_DOT           AVRC_ID_DOT         /* dot */
#define BTA_RC_OP_ENTER         AVRC_ID_ENTER       /* enter */
#define BTA_RC_OP_CLEAR         AVRC_ID_CLEAR       /* clear */
#define BTA_RC_OP_CHAN_UP       AVRC_ID_CHAN_UP     /* channel up */
#define BTA_RC_OP_CHAN_DOWN     AVRC_ID_CHAN_DOWN   /* channel down */
#define BTA_RC_OP_PREV_CHAN     AVRC_ID_PREV_CHAN   /* previous channel */
#define BTA_RC_OP_SOUND_SEL     AVRC_ID_SOUND_SEL   /* sound select */
#define BTA_RC_OP_INPUT_SEL     AVRC_ID_INPUT_SEL   /* input select */
#define BTA_RC_OP_DISP_INFO     AVRC_ID_DISP_INFO   /* display information */
#define BTA_RC_OP_HELP          AVRC_ID_HELP        /* help */
#define BTA_RC_OP_PAGE_UP       AVRC_ID_PAGE_UP     /* page up */
#define BTA_RC_OP_PAGE_DOWN     AVRC_ID_PAGE_DOWN   /* page down */
#define BTA_RC_OP_POWER         AVRC_ID_POWER       /* power */
#define BTA_RC_OP_VOL_UP        AVRC_ID_VOL_UP      /* volume up */
#define BTA_RC_OP_VOL_DOWN      AVRC_ID_VOL_DOWN    /* volume down */
#define BTA_RC_OP_MUTE          AVRC_ID_MUTE        /* mute */
#define BTA_RC_OP_PLAY          AVRC_ID_PLAY        /* play */
#define BTA_RC_OP_STOP          AVRC_ID_STOP        /* stop */
#define BTA_RC_OP_PAUSE         AVRC_ID_PAUSE       /* pause */
#define BTA_RC_OP_RECORD        AVRC_ID_RECORD      /* record */
#define BTA_RC_OP_REWIND        AVRC_ID_REWIND      /* rewind */
#define BTA_RC_OP_FAST_FOR      AVRC_ID_FAST_FOR    /* fast forward */
#define BTA_RC_OP_EJECT         AVRC_ID_EJECT       /* eject */
#define BTA_RC_OP_FORWARD       AVRC_ID_FORWARD     /* forward */
#define BTA_RC_OP_BACKWARD      AVRC_ID_BACKWARD    /* backward */
#define BTA_RC_OP_ANGLE         AVRC_ID_ANGLE       /* angle */
#define BTA_RC_OP_SUBPICT       AVRC_ID_SUBPICT     /* subpicture */
#define BTA_RC_OP_F1            AVRC_ID_F1          /* F1 */
#define BTA_RC_OP_F2            AVRC_ID_F2          /* F2 */
#define BTA_RC_OP_F3            AVRC_ID_F3          /* F3 */
#define BTA_RC_OP_F4            AVRC_ID_F4          /* F4 */
#define BTA_RC_OP_F5            AVRC_ID_F5          /* F5 */

typedef UINT8 tBTA_RC_OP;

/* command codes for BTA_RcVendorCmd and BTA_RcMetaCmd */
#define BTA_RC_CMD_CTRL         AVRC_CMD_CTRL
#define BTA_RC_CMD_STATUS       AVRC_CMD_STATUS
#define BTA_RC_CMD_SPEC_INQ     AVRC_CMD_SPEC_INQ
#define BTA_RC_CMD_NOTIF        AVRC_CMD_NOTIF
#define BTA_RC_CMD_GEN_INQ      AVRC_CMD_GEN_INQ

typedef UINT8 tBTA_RC_CMD;

/* response codes for BTA_RcVendorRsp and BTA_RcMetaRsp */
#define BTA_RC_RSP_NOT_IMPL     AVRC_RSP_NOT_IMPL
#define BTA_RC_RSP_ACCEPT       AVRC_RSP_ACCEPT
#define BTA_RC_RSP_REJ          AVRC_RSP_REJ
#define BTA_RC_RSP_IN_TRANS     AVRC_RSP_IN_TRANS
#define BTA_RC_RSP_IMPL_STBL    AVRC_RSP_IMPL_STBL
#define BTA_RC_RSP_CHANGED      AVRC_RSP_CHANGED
#define BTA_RC_RSP_INTERIM      AVRC_RSP_INTERIM

typedef UINT8 tBTA_RC_RSP;

/* state flag for pass through command */
#define BTA_RC_STATE_PRESS      AVRC_STATE_PRESS    /* key pressed */
#define BTA_RC_STATE_RELEASE    AVRC_STATE_RELEASE  /* key released */

typedef UINT8 tBTA_RC_STATE;

/* Info from peer's AVRC SDP record (included in BTA_RC_OPEN_EVT) */
typedef struct
{
    UINT16 version;         /* AVRCP version */
    UINT16 features;        /* Supported features (see AVRC_SUPF_* definitions in avrc_api.h) */
} tBTA_RC_PEER_INFO;

typedef struct
{
    tBTA_RC_FEAT        roles;  /* BTA_RC_FEAT_CONTROL or BTA_RC_FEAT_TARGET */
    tBTA_RC_PEER_INFO   tg;     /* Peer TG role info */
    tBTA_RC_PEER_INFO   ct;     /* Peer CT role info */
} tBTA_RC_PEER_FEAT;

/* Data for BTA_RC_CONNECTED_EVT */
typedef struct
{
    tBTA_STATUS         status;
    BD_ADDR             bd_addr;
    tBTA_RC_PEER_FEAT   peer_feat;
} tBTA_RC_CONNECTED;

/* Data for BTA_RC_UNIT_INFO_CMD_EVT */
typedef struct
{
    UINT8   label;
} tBTA_RC_UNIT_INFO_CMD;

/* Data for BTA_RC_UNIT_INFO_RSP_EVT */
typedef struct
{
    UINT8   label;
    UINT8   unit_type;
    UINT8   unit;
    UINT32  company_id;
} tBTA_RC_UNIT_INFO_RSP;

/* Data for BTA_RC_SUBUNIT_INFO_CMD_EVT */
typedef struct
{
    UINT8   label;
    UINT8   page;
} tBTA_RC_SUBUNIT_INFO_CMD;

/* Data for BTA_RC_SUBUNIT_INFO_RSP_EVT */
typedef struct
{
    UINT8   label;
    UINT8   page;
    UINT8   subunit_type[AVRC_SUB_TYPE_LEN];
    BOOLEAN panel;      /* TRUE if the panel subunit type is in the  subunit_type array, FALSE otherwise. */
} tBTA_RC_SUBUNIT_INFO_RSP;

/* Data for BTA_RC_PASS_THRU_CMD_EVT */
typedef struct
{
    UINT8           label;
    UINT8           op_id;          /* Operation ID (see AVRC_ID_* defintions in avrc_defs.h) */
    UINT8           key_state;      /* AVRC_STATE_PRESS or AVRC_STATE_RELEASE */
    tAVRC_HDR       hdr;            /* Message header. */
    UINT8           len;
    UINT8           *p_data;
} tBTA_RC_PASS_THRU_CMD;

/* Data for BTA_RC_PASS_THRU_RSP_EVT */
typedef struct
{
    UINT8           label;
    UINT8           op_id;          /* Operation ID (see AVRC_ID_* defintions in avrc_defs.h) */
    UINT8           key_state;      /* AVRC_STATE_PRESS or AVRC_STATE_RELEASE */
    UINT8           rsp_code;       /* AVRC response code (see AVRC_RSP_* definitions in avrc_defs.h) */
    UINT8           len;
    UINT8           *p_data;
} tBTA_RC_PASS_THRU_RSP;

/* Data for BTA_RC_CMD_TOUT_EVT */
typedef struct
{
    UINT8           label;          /* label of command that timed out */
} tBTA_RC_CMD_TOUT;

/* Data for BTA_RC_VENDOR_CMD_EVT / BTA_RC_VENDOR_RSP_EVT */
typedef struct
{
    UINT8           label;
    UINT32          company_id;
    tAVRC_HDR       hdr;            /* AVRC header (AVRC cmd/rsp type, subunit_type, subunit_id - see avrc_defs.h)  */
    UINT16          len;            /* Max vendor dependent message is 512 */
    UINT8           *p_data;
} tBTA_RC_VENDOR;

/* Data for BTA_RC_METADATA_CMD_EVT / BTA_RC_METADATA_RSP_EVT / */
/* Data for BTA_RC_BROWSE_CMD_EVT / BTA_RC_BROWSE_RSP_EVT / */
typedef struct
{
    UINT8           label;
    UINT8           code;           /* AVRC command or response code (see AVRC_CMD_* or AVRC_RSP_* definitions in avrc_defs.h) */
    tAVRC_MSG       *p_msg;
    UINT16          len;
    UINT8           *p_data;
} tBTA_RC_METADATA;


typedef union
{
    tBTA_STATUS             status;             /* BTA_RC_ENABLE_EVT, BTA_RC_DISABLE_EVT, BTA_RC_DISCONNECTED_EVT */
    tBTA_RC_CONNECTED       connected;          /* BTA_RC_CONNECTED_EVT */
    tBTA_RC_UNIT_INFO_CMD   unit_info_cmd;      /* BTA_RC_UNIT_INFO_CMD_EVT */
    tBTA_RC_UNIT_INFO_RSP   unit_info_rsp;      /* BTA_RC_UNIT_INFO_RSP_EVT */
    tBTA_RC_SUBUNIT_INFO_CMD subunit_info_cmd;  /* BTA_RC_SUBUNIT_INFO_CMD_EVT */
    tBTA_RC_SUBUNIT_INFO_RSP subunit_info_rsp;  /* BTA_RC_SUBUNIT_INFO_RSP_EVT */
    tBTA_RC_PASS_THRU_CMD   pass_thru_cmd;      /* BTA_RC_PASS_THRU_CMD_EVT */
    tBTA_RC_PASS_THRU_RSP   pass_thru_rsp;      /* BTA_RC_PASS_THRU_RSP_EVT */
    tBTA_RC_VENDOR          vendor_msg;         /* BTA_RC_VENDOR_CMD_EVT, BTA_RC_VENDOR_RSP_EVT */
    tBTA_RC_METADATA        meta_msg;           /* BTA_RC_METADATA_CMD_EVT, BTA_RC_METADATA_RSP_EVT */
    tBTA_RC_METADATA        browse_msg;         /* BTA_RC_BROWSE_CMD_EVT, BTA_RC_BROWSE_RSP_EVT */
    tBTA_RC_CMD_TOUT        cmd_tout;           /* BTA_RC_CMD_TOUT_EVT */
    tBTA_RC_PEER_FEAT       peer_feat;          /* BTA_RC_PEER_FEAT_EVT */
} tBTA_RC;

/* BTA_RC callback */
typedef void (tBTA_RC_CBACK)(tBTA_RC_EVT event, UINT8 handle, tBTA_RC *p_data);

/*****************************************************************************
**  Configuration structures for CT and TG
*****************************************************************************/
typedef struct
{

    UINT32  company_id;         /* Used for UNIT_INFO response */
    UINT16  mtu;

    /* SDP configuration for controller */
    UINT8   *ct_service_name;
    UINT8   *ct_provider_name;
    UINT16  ct_categories;

    /* SDP configuration for target */
    UINT8   *tg_service_name;
    UINT8   *tg_provider_name;
    UINT16  tg_categories;
} tBTA_RC_CFG;

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/


/*****************************************************************************
**  Common Controller/Target role APIs
*****************************************************************************/

/*******************************************************************************
**
** Function         BTA_RcEnable
**
** Description      Enable the AVRC module.
**
**                  Initialize the BTA_RC module, registers callback for BTA_RC
**                  event notifications. Add AVRC service to SDP database.
**
**                  BTA_RC_ENABLE_EVT will report the status of the operation.
**
**                  NOTE:
**                  BTA_RC is intended for use platforms that do not include
**                  AV source (BTA_AV) nor AV sink (BTA_AVK).
**
**                  For platforms that include the BTA_AV or BTA_AVK module,
**                  the RC APIs from those respective modules should be
**                  used instead.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcEnable(tBTA_SEC sec_mask, tBTA_RC_FEAT features,
                          tBTA_RC_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_RcDisable
**
** Description      Disable the stand-alone AVRC service.
**
** Note             If there is an active AVRCP connection, it will be disconnected
**                  first. Then AVRC acceptor connection will be closed if there is any.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcDisable(void);

/*******************************************************************************
**
** Function         BTA_RcOpenAcceptor
**
** Description      Open an AVRCP connection as acceptor
**
**                  Role may be controller (BTA_RC_FEAT_CONTROL) or
**                  target (BTA_RC_FEAT_TARGET) or both.
**                  BTA_RC_OPEN_ACP_EVT will report the status of the operation.
**
** Note             This function cannot be used to initiate a AVRC connection.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcOpenAcceptor(tBTA_RC_FEAT role);

/*******************************************************************************
**
** Function         BTA_RcCloseAcceptor
**
** Description      Close an AVRCP connection as acceptor
**                  BTA_RC_CLOSE_ACP_EVT will report the status of the operation.
**
** Note             If there is an active AVRCP connection, it will be disconnected
**                  first. Then AVRC acceptor connection will be closed.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcCloseAcceptor(UINT8 handle);

/*******************************************************************************
**
** Function         BTA_RcConnect
**
** Description      Open an AVRCP connection as initiator
**
**                  Role may be controller (BTA_RC_FEAT_CONTROL) or
**                  target (BTA_RC_FEAT_TARGET) or both.
**                  BTA_RC_CONNECTED_EVT will report the status of the operation.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcConnect(BD_ADDR peer_addr, tBTA_RC_FEAT role);

/*******************************************************************************
**
** Function         BTA_RcDisconnect
**
** Description      Disconnect an active AVRCP connection
**                  BTA_RC_DISCONNECTED_EVT will report the status of the operation.
**
**
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcDisconnect(UINT8 handle);


/*****************************************************************************
**  Controller role APIs
*****************************************************************************/

/*******************************************************************************
**
** Function         BTA_RcUnitInfoCmd
**
** Description      Send a UNIT INFO command.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcUnitInfoCmd(UINT8 handle, UINT8 label);

/*******************************************************************************
**
** Function         BTA_RcSubunitInfoCmd
**
** Description      Send a SUBINIT INFO command.
**
**                  NOTE: extension_code is for future use, per AV/C specifications.
**                        BTA_RC_SUBUNIT_DEFAULT_EXTENSION_CODE may be used.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcSubunitInfoCmd(UINT8 handle, UINT8 label, UINT8 page,
                                  UINT8 extension_code);

/*******************************************************************************
**
** Function         BTA_RcVendorCmd
**
** Description      Send a vendor dependent remote control command.
**
**                  See AVRC_CMD_* in avrc_defs.h for available commands.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcVendorCmd(UINT8 handle, UINT8 label,  UINT8 avrc_cmd,
                             UINT32 company_id, UINT8 subunit_type, UINT8 subunit_id,
                             UINT8 *p_data, UINT16 len);

/*******************************************************************************
**
** Function         BTA_RcPassThroughCmd
**
** Description      Send a PASS_THROUGH command
**
**                  See AVRC_ID_* in avrc_defs.h for available opcode IDs
**                      AVRC_STATE_* for key states definitions
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcPassThroughCmd(UINT8 handle, UINT8 label, tBTA_RC_OP op_id,
                                  tBTA_RC_STATE key_state, UINT8 *p_data, UINT16 len);

/*******************************************************************************
**
** Function         BTA_RcMetaCmd
**
** Description      Send a Metadata/Advanced Control command.
**
**                  See AVRC_CMD_* in avrc_defs.h for available commands.
**
**                  The message contained in p_pkt can be composed with AVRC
**                  utility functions.
**
**                  This function can only be used if RC is enabled with feature
**                  BTA_RC_FEAT_METADATA.
**
**                  This message is sent only when the peer supports the TG role.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcMetaCmd(UINT8 handle, UINT8 label, tBTA_RC_CMD avrc_cmd, BT_HDR *p_pkt);


/*****************************************************************************
**  Target role APIs
*****************************************************************************/

/*******************************************************************************
**
** Function         BTA_RcUnitInfoRsp
**
** Description      Send a UNIT INFO response.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcUnitInfoRsp(UINT8 handle, UINT8 label, UINT8 unit_type, UINT8 unit);

/*******************************************************************************
**
** Function         BTA_RcSubunitInfoRsp
**
** Description      Send a SUBINIT INFO response.
**
**                  NOTE: extension_code is for future use, per AV/C specifications.
**                        BTA_RC_SUBUNIT_DEFAULT_EXTENSION_CODE may be used.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcSubunitInfoRsp(UINT8 handle, UINT8 label, UINT8 page,
                                  UINT8 extension_code,
                                  UINT8 subunit_type[AVRC_SUB_TYPE_LEN]);

/*******************************************************************************
**
** Function         BTA_RcVendorRsp
**
** Description      Send a vendor dependent remote control response.
**
**                  See AVRC_RSP_* in avrc_defs.h for response code definitions
**
**                  This function must be called if a BTA_RC_VENDOR_CMD_EVT
**                  is received. This function can only be used if RC is
**                  enabled with feature BTA_RC_FEAT_VENDOR.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcVendorRsp(UINT8 handle, UINT8 label, tBTA_RC_RSP avrc_rsp,
                             UINT32 company_id, UINT8 subunit_type, UINT8 subunit_id,
                             UINT8 *p_data, UINT16 len);


/*******************************************************************************
**
** Function         BTA_RcPassThroughRsp
**
** Description      Send a PASS_THROUGH response
**
**                  See AVRC_ID_* in avrc_defs.h for available opcode IDs
**                      AVRC_RSP_* for response code definitions
**                      AVRC_STATE_* for key states definitions
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcPassThroughRsp(UINT8 handle, UINT8 label, tBTA_RC_OP op_id,
                                  tBTA_RC_RSP avrc_rsp, tBTA_RC_STATE key_state,
                                  UINT8 *p_data, UINT16 len);

/*******************************************************************************
**
** Function         BTA_RcMetaRsp
**
** Description      Send a Metadata response.
**
**                  See AVRC_RSP_* in avrc_defs.h for response code definitions
**
**                  The message contained in p_pkt can be composed with AVRC
**                  utility functions.
**
**                  This function can only be used if RC is enabled with feature
**                  BTA_RC_FEAT_METADATA.
**
** Returns          void
**
*******************************************************************************/
BTA_API void BTA_RcMetaRsp(UINT8 handle, UINT8 label, tBTA_RC_RSP avrc_rsp,
                           BT_HDR *p_pkt);

#ifdef __cplusplus
}
#endif

#endif /* BTA_AV_API_H */
