/*****************************************************************************
 **
 **  Name:           bsa_avk_api.h
 **
 **  Description:    This is the public interface file for the advanced
 **                  audio/video streaming (AV) subsystem of BSA, Broadcom Simple
 **                  Bluetooth application layer.
 **
 **  Copyright (c) 2004-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_AVK_API_H
#define BSA_AVK_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"
#include "avrc_api.h"
#include "avdt_api.h"
#include "a2d_api.h"
#include "a2d_sbc.h"
#include "a2d_m12.h"
#include "a2d_m24.h"
#include "vdp_api.h"
#include "bta_avk_api.h"
#include "bta_rc_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
#define BSA_RC_MAX_PARAM_LEN     256

/* AV features masks */
#define BSA_AVK_FEAT_RCTG      BTA_RC_FEAT_TARGET        /* remote control target */
#define BSA_AVK_FEAT_RCCT      BTA_RC_FEAT_CONTROL       /* remote control controller */
#define BSA_AVK_FEAT_PROTECT   BTA_AVK_FEAT_PROTECT      /* streaming media contect protection */
#define BSA_AVK_FEAT_VENDOR    BTA_RC_FEAT_VENDOR        /* remote control vendor dependent commands */
#define BSA_AVK_FEAT_METADATA  BTA_RC_FEAT_METADATA      /* AVRC Metadata transfer supported */
#define BSA_AVK_FEAT_BROWSE    BTA_RC_FEAT_BROWSE        /* browsing channel support */
#define BSA_AVK_FEAT_REPORT    BTA_AVK_FEAT_REPORT       /* use reporting service for VDP */
#define BSA_AVK_FEAT_DELAY_RPT BTA_AVK_FEAT_DELAY_RPT    /* use delay reporting */

typedef UINT16 tBSA_AVK_FEAT;

/* AV channel values */
#define BSA_AVK_CHNL_AUDIO       0  /* audio channel */
#define BSA_AVK_CHNL_VIDEO       1  /* video channel */
typedef UINT8 tBSA_AVK_CHNL;

/* AV handle type */
typedef UINT8 tBSA_AVK_HNDL;

/* maximum number of streams created: 1 for audio, 1 for video */
#ifndef BSA_AVK_NUM_STRS
#define BSA_AVK_NUM_STRS         2
#endif

#ifndef BSA_AVK_MAX_SEPS
#define BSA_AVK_MAX_SEPS         2
#endif

/* codec type */
#define BSA_AVK_CODEC_SBC        A2D_MEDIA_CT_SBC        /* SBC media codec type */
#define BSA_AVK_CODEC_M12        A2D_MEDIA_CT_M12        /* MPEG-1, 2 Audio media codec type */
#define BSA_AVK_CODEC_M24        A2D_MEDIA_CT_M24        /* MPEG-2, 4 AAC media codec type */
#define BSA_AVK_CODEC_ATRAC      A2D_MEDIA_CT_ATRAC      /* ATRAC family media codec type */
#define BSA_AVK_CODEC_PCM        0x5                     /* ram pcm */
#define BSA_AVK_CODEC_APTX       0x6                     /* apt-X */
#define BSA_AVK_CODEC_SEC        0x7                     /* SEC - Samsung Codec */
#define BSA_AVK_CODEC_H263_P0    VDP_MEDIA_CT_H263_P0    /* H.263 baseline (profile 0) */
#define BSA_AVK_CODEC_MPEG4      VDP_MEDIA_CT_MPEG4      /* MPEG-4 Visual Simple Profile */
#define BSA_AVK_CODEC_H263_P3    VDP_MEDIA_CT_H263_P3    /* H.263 profile 3 */
#define BSA_AVK_CODEC_H263_P8    VDP_MEDIA_CT_H263_P8    /* H.263 profile 8 */
#define BSA_AVK_CODEC_VEND       VDP_MEDIA_CT_VEND       /* Non-VDP */

typedef UINT8 tBSA_AVK_CODEC;

/* operation id list for BSA_AvkRemoteCmd */
#define BSA_AVK_RC_SELECT        AVRC_ID_SELECT      /* select */
#define BSA_AVK_RC_UP            AVRC_ID_UP          /* up */
#define BSA_AVK_RC_DOWN          AVRC_ID_DOWN        /* down */
#define BSA_AVK_RC_LEFT          AVRC_ID_LEFT        /* left */
#define BSA_AVK_RC_RIGHT         AVRC_ID_RIGHT       /* right */
#define BSA_AVK_RC_RIGHT_UP      AVRC_ID_RIGHT_UP    /* right-up */
#define BSA_AVK_RC_RIGHT_DOWN    AVRC_ID_RIGHT_DOWN  /* right-down */
#define BSA_AVK_RC_LEFT_UP       AVRC_ID_LEFT_UP     /* left-up */
#define BSA_AVK_RC_LEFT_DOWN     AVRC_ID_LEFT_DOWN   /* left-down */
#define BSA_AVK_RC_ROOT_MENU     AVRC_ID_ROOT_MENU   /* root menu */
#define BSA_AVK_RC_SETUP_MENU    AVRC_ID_SETUP_MENU  /* setup menu */
#define BSA_AVK_RC_CONT_MENU     AVRC_ID_CONT_MENU   /* contents menu */
#define BSA_AVK_RC_FAV_MENU      AVRC_ID_FAV_MENU    /* favorite menu */
#define BSA_AVK_RC_EXIT          AVRC_ID_EXIT        /* exit */
#define BSA_AVK_RC_0             AVRC_ID_0           /* 0 */
#define BSA_AVK_RC_1             AVRC_ID_1           /* 1 */
#define BSA_AVK_RC_2             AVRC_ID_2           /* 2 */
#define BSA_AVK_RC_3             AVRC_ID_3           /* 3 */
#define BSA_AVK_RC_4             AVRC_ID_4           /* 4 */
#define BSA_AVK_RC_5             AVRC_ID_5           /* 5 */
#define BSA_AVK_RC_6             AVRC_ID_6           /* 6 */
#define BSA_AVK_RC_7             AVRC_ID_7           /* 7 */
#define BSA_AVK_RC_8             AVRC_ID_8           /* 8 */
#define BSA_AVK_RC_9             AVRC_ID_9           /* 9 */
#define BSA_AVK_RC_DOT           AVRC_ID_DOT         /* dot */
#define BSA_AVK_RC_ENTER         AVRC_ID_ENTER       /* enter */
#define BSA_AVK_RC_CLEAR         AVRC_ID_CLEAR       /* clear */
#define BSA_AVK_RC_CHAN_UP       AVRC_ID_CHAN_UP     /* channel up */
#define BSA_AVK_RC_CHAN_DOWN     AVRC_ID_CHAN_DOWN   /* channel down */
#define BSA_AVK_RC_PREV_CHAN     AVRC_ID_PREV_CHAN   /* previous channel */
#define BSA_AVK_RC_SOUND_SEL     AVRC_ID_SOUND_SEL   /* sound select */
#define BSA_AVK_RC_INPUT_SEL     AVRC_ID_INPUT_SEL   /* input select */
#define BSA_AVK_RC_DISP_INFO     AVRC_ID_DISP_INFO   /* display information */
#define BSA_AVK_RC_HELP          AVRC_ID_HELP        /* help */
#define BSA_AVK_RC_PAGE_UP       AVRC_ID_PAGE_UP     /* page up */
#define BSA_AVK_RC_PAGE_DOWN     AVRC_ID_PAGE_DOWN   /* page down */
#define BSA_AVK_RC_POWER         AVRC_ID_POWER       /* power */
#define BSA_AVK_RC_VOL_UP        AVRC_ID_VOL_UP      /* volume up */
#define BSA_AVK_RC_VOL_DOWN      AVRC_ID_VOL_DOWN    /* volume down */
#define BSA_AVK_RC_MUTE          AVRC_ID_MUTE        /* mute */
#define BSA_AVK_RC_PLAY          AVRC_ID_PLAY        /* play */
#define BSA_AVK_RC_STOP          AVRC_ID_STOP        /* stop */
#define BSA_AVK_RC_PAUSE         AVRC_ID_PAUSE       /* pause */
#define BSA_AVK_RC_RECORD        AVRC_ID_RECORD      /* record */
#define BSA_AVK_RC_REWIND        AVRC_ID_REWIND      /* rewind */
#define BSA_AVK_RC_FAST_FOR      AVRC_ID_FAST_FOR    /* fast forward */
#define BSA_AVK_RC_EJECT         AVRC_ID_EJECT       /* eject */
#define BSA_AVK_RC_FORWARD       AVRC_ID_FORWARD     /* forward */
#define BSA_AVK_RC_BACKWARD      AVRC_ID_BACKWARD    /* backward */
#define BSA_AVK_RC_ANGLE         AVRC_ID_ANGLE       /* angle */
#define BSA_AVK_RC_SUBPICT       AVRC_ID_SUBPICT     /* subpicture */
#define BSA_AVK_RC_F1            AVRC_ID_F1          /* F1 */
#define BSA_AVK_RC_F2            AVRC_ID_F2          /* F2 */
#define BSA_AVK_RC_F3            AVRC_ID_F3          /* F3 */
#define BSA_AVK_RC_F4            AVRC_ID_F4          /* F4 */
#define BSA_AVK_RC_F5            AVRC_ID_F5          /* F5 */

typedef UINT8 tBSA_AVK_RC;

/* state flag for pass through command */
#define BSA_AVK_STATE_PRESS      AVRC_STATE_PRESS    /* key pressed */
#define BSA_AVK_STATE_RELEASE    AVRC_STATE_RELEASE  /* key released */

typedef UINT8 tBSA_AVK_STATE;

/* AV callback events */
#define BSA_AVK_OPEN_EVT         2       /* connection opened */
#define BSA_AVK_CLOSE_EVT        3       /* connection closed */
#define BSA_AVK_START_EVT        4       /* stream data transfer started */
#define BSA_AVK_STOP_EVT         5       /* stream data transfer stopped */
#define BSA_AVK_RC_OPEN_EVT      8       /* remote control channel open */
#define BSA_AVK_RC_PEER_OPEN_EVT 9       /* remote control channel open by peer */
#define BSA_AVK_RC_CLOSE_EVT     10      /* remote control channel closed */
#define BSA_AVK_REMOTE_CMD_EVT   11      /* remote control command */
#define BSA_AVK_REMOTE_RSP_EVT   12      /* remote control response */
#define BSA_AVK_VENDOR_CMD_EVT   13      /* vendor dependent remote control command */
#define BSA_AVK_VENDOR_RSP_EVT   14      /* vendor dependent remote control response */
#define BSA_AVK_CP_INFO_EVT      18      /* content protection message */

#define BSA_AVK_REGISTER_NOTIFICATION_EVT   19      /* reg notfn response */
#define BSA_AVK_LIST_PLAYER_APP_ATTR_EVT    20      /* list player attr response */
#define BSA_AVK_LIST_PLAYER_APP_VALUES_EVT  21      /* list player value response */
#define BSA_AVK_SET_PLAYER_APP_VALUE_EVT    22      /* set player value response */
#define BSA_AVK_GET_PLAYER_APP_VALUE_EVT    23      /* get player value response */
#define BSA_AVK_GET_PLAYER_ATTR_TEXT_EVT    24      /* get player attr text response */
#define BSA_AVK_GET_PLAYER_ATTR_VALUE_EVT   25      /* get player value text response */
#define BSA_AVK_GET_ELEM_ATTR_EVT           26      /* get element attrubute response */
#define BSA_AVK_GET_PLAY_STATUS_EVT         27      /* get plays status response */
#define BSA_AVK_SET_ABSOLUTE_VOLUME_EVT     28      /* set abs vol esponse */
#define BSA_AVK_SET_ADDRESSED_PLAYER_EVT    29      /* set addressed player response */
#define BSA_AVK_SET_BROWSED_PLAYER_EVT      30      /* set browsed player response */
#define BSA_AVK_GET_FOLDER_ITEMS_EVT        31      /* get folder items response */
#define BSA_AVK_CHANGE_PATH_EVT             32      /* change path response */
#define BSA_AVK_GET_ITEM_ATTR_EVT           33      /* get item attr response */
#define BSA_AVK_PLAY_ITEM_EVT               34      /* play item response */
#define BSA_AVK_ADD_TO_NOW_PLAYING_EVT      36      /* add to now playing response */
#define BSA_AVK_SET_ABS_VOL_CMD_EVT         37      /* set abs vol command */
#define BSA_AVK_REG_NOTIFICATION_CMD_EVT    38      /* reg notification cmd */

/* Define the PDUs carried in the vendor dependant data */
#define BSA_AVK_RC_VD_GET_CAPABILITIES               AVRC_PDU_GET_CAPABILITIES
#define BSA_AVK_RC_VD_LIST_PLAYER_APP_ATTR           AVRC_PDU_LIST_PLAYER_APP_ATTR
#define BSA_AVK_RC_VD_LIST_PLAYER_APP_VALUES         AVRC_PDU_LIST_PLAYER_APP_VALUES
#define BSA_AVK_RC_VD_GET_CUR_PLAYER_APP_VALUE       AVRC_PDU_GET_CUR_PLAYER_APP_VALUE
#define BSA_AVK_RC_VD_SET_PLAYER_APP_VALUE           AVRC_PDU_SET_PLAYER_APP_VALUE
#define BSA_AVK_RC_VD_GET_PLAYER_APP_ATTR_TEXT       AVRC_PDU_GET_PLAYER_APP_ATTR_TEXT
#define BSA_AVK_RC_VD_GET_PLAYER_APP_VALUE_TEXT      AVRC_PDU_GET_PLAYER_APP_VALUE_TEXT
#define BSA_AVK_RC_VD_INFORM_DISPLAY_CHARSET         AVRC_PDU_INFORM_DISPLAY_CHARSET
#define BSA_AVK_RC_VD_INFORM_BATTERY_STAT_OF_CT      AVRC_PDU_INFORM_BATTERY_STAT_OF_CT
#define BSA_AVK_RC_VD_GET_ELEMENT_ATTR               AVRC_PDU_GET_ELEMENT_ATTR
#define BSA_AVK_RC_VD_GET_PLAY_STATUS                AVRC_PDU_GET_PLAY_STATUS
#define BSA_AVK_RC_VD_REGISTER_NOTIFICATION          AVRC_PDU_REGISTER_NOTIFICATION
#define BSA_AVK_RC_VD_REQUEST_CONTINUATION_RSP       AVRC_PDU_REQUEST_CONTINUATION_RSP
#define BSA_AVK_RC_VD_ABORT_CONTINUATION_RSP         AVRC_PDU_ABORT_CONTINUATION_RSP
/* added in 1.4 */
#define BSA_AVK_RC_VD_SET_ABSOLUTE_VOLUME            AVRC_PDU_SET_ABSOLUTE_VOLUME
#define BSA_AVK_RC_VD_SET_ADDRESSED_PLAYER           AVRC_PDU_SET_ADDRESSED_PLAYER
#define BSA_AVK_RC_VD_SET_BROWSED_PLAYER             AVRC_PDU_SET_BROWSED_PLAYER
#define BSA_AVK_RC_VD_GET_FOLDER_ITEMS               AVRC_PDU_GET_FOLDER_ITEMS
#define BSA_AVK_RC_VD_CHANGE_PATH                    AVRC_PDU_CHANGE_PATH
#define BSA_AVK_RC_VD_GET_ITEM_ATTRIBUTES            AVRC_PDU_GET_ITEM_ATTRIBUTES
#define BSA_AVK_RC_VD_PLAY_ITEM                      AVRC_PDU_PLAY_ITEM
#define BSA_AVK_RC_VD_SEARCH                         AVRC_PDU_SEARCH
#define BSA_AVK_RC_VD_ADD_TO_NOW_PLAYING             AVRC_PDU_ADD_TO_NOW_PLAYING
#define BSA_AVK_RC_VD_GENERAL_REJECT                 AVRC_PDU_GENERAL_REJECT


/* Define the events that can be registered for notifications
*/
#define BSA_AVK_RC_EVT_PLAY_STATUS_CHANGE             AVRC_EVT_PLAY_STATUS_CHANGE
#define BSA_AVK_RC_EVT_TRACK_CHANGE                   AVRC_EVT_TRACK_CHANGE
#define BSA_AVK_RC_EVT_TRACK_REACHED_END              AVRC_EVT_TRACK_REACHED_END
#define BSA_AVK_RC_EVT_TRACK_REACHED_START            AVRC_EVT_TRACK_REACHED_START
#define BSA_AVK_RC_EVT_PLAY_POS_CHANGED               AVRC_EVT_PLAY_POS_CHANGED
#define BSA_AVK_RC_EVT_BATTERY_STATUS_CHANGE          AVRC_EVT_BATTERY_STATUS_CHANGE
#define BSA_AVK_RC_EVT_SYSTEM_STATUS_CHANGE           AVRC_EVT_SYSTEM_STATUS_CHANGE
#define BSA_AVK_RC_EVT_APP_SETTING_CHANGE             AVRC_EVT_APP_SETTING_CHANGE
/* added in AVRCP 1.4 */
#define BSA_AVK_RC_EVT_NOW_PLAYING_CHANGE             AVRC_EVT_NOW_PLAYING_CHANGE
#define BSA_AVK_RC_EVT_AVAL_PLAYERS_CHANGE            AVRC_EVT_AVAL_PLAYERS_CHANGE
#define BSA_AVK_RC_EVT_ADDR_PLAYER_CHANGE             AVRC_EVT_ADDR_PLAYER_CHANGE
#define BSA_AVK_RC_EVT_UIDS_CHANGE                    AVRC_EVT_UIDS_CHANGE
#define BSA_AVK_RC_EVT_VOLUME_CHANGE                  AVRC_EVT_VOLUME_CHANGE

typedef UINT8 tBSA_AVK_EVT;

/* command codes for BSA_AvkVendorCmd */
#define BSA_AVK_CMD_CTRL          BTA_RC_CMD_CTRL
#define BSA_AVK_CMD_STATUS        BTA_RC_CMD_STATUS
#define BSA_AVK_CMD_SPEC_INQ      BTA_RC_CMD_SPEC_INQ
#define BSA_AVK_CMD_NOTIF         BTA_RC_CMD_NOTIF
#define BSA_AVK_CMD_GEN_INQ       BTA_RC_CMD_GEN_INQ

typedef UINT8 tBSA_AVK_CMD;

/* response codes for BSA_AvkVendorRsp */
#define BSA_AVK_RSP_NOT_IMPL      BTA_RC_RSP_NOT_IMPL
#define BSA_AVK_RSP_ACCEPT        BTA_RC_RSP_ACCEPT
#define BSA_AVK_RSP_REJ           BTA_RC_RSP_REJ
#define BSA_AVK_RSP_IN_TRANS      BTA_RC_RSP_IN_TRANS
#define BSA_AVK_RSP_IMPL_STBL     BTA_RC_RSP_IMPL_STBL
#define BSA_AVK_RSP_CHANGED       BTA_RC_RSP_CHANGED
#define BSA_AVK_RSP_INTERIM       BTA_RC_RSP_INTERIM

typedef UINT8 tBSA_AVK_CODE;

#define BSA_AVK_CP_SCMS_T_ID     2
#define BSA_AVK_CP_DTCP_ID       1
#define BSA_AVK_CP_NONE_ID       0

typedef UINT16 tBSA_AVK_CP_ID;

#define BSA_AVK_CP_SCMS_COPY_UNDEFINED  4
#define BSA_AVK_CP_SCMS_COPY_FREE       2
#define BSA_AVK_CP_SCMS_COPY_ONCE       1
#define BSA_AVK_CP_SCMS_COPY_NEVER      0

typedef UINT8 tBSA_AVK_CP_SCMS_FLAG;

/*Max number of simultneous AVK connections*/
#define BSA_MAX_AVK_CONNECTIONS    1

typedef struct
{
    UINT16 sampling_freq;   /* 44100, 48000 etc */
    UINT16 num_channel;     /* 1 for mono or 2 stereo */
    UINT8  bit_per_sample;  /* Number of bits per sample (8, 16) */
}tBSA_AVK_MEDIA_RECEIVE_CFG_PCM;

typedef union
{
    tBSA_AVK_MEDIA_RECEIVE_CFG_PCM pcm;/* raw PCM receiving format */
    tA2D_SBC_CIE sbc;               /* SBC receiving format */
    tA2D_M12_CIE mp3;               /* MPEG-1, 2 Audio Codec Information Element */
    tA2D_M24_CIE aac;               /* MPEG-2, 4 AAC Codec Information Element */
}tBSA_AVK_MEDIA_RECEIVE_CFG;

typedef struct
{
    tBTA_AVK_CODEC format;
    tBSA_AVK_MEDIA_RECEIVE_CFG cfg;
} tBSA_AVK_MEDIA_RECEIVING;

/* Structures/union used to configure the supported Rx format (audio and Video) */
typedef struct
{
    BOOLEAN pcm_supported;      /* True if application can receive PCM */
    BOOLEAN sbc_supported;      /* True if application can receive SBC */
    BOOLEAN mp3_supported;      /* True if application can receive MP3 */
    BOOLEAN aac_supported;      /* True if application can receive AAC */
    BOOLEAN sec_supported;      /* True if application can receive SEC */
}tBSA_AVK_MEDIA_AUDIO_RX_SUP_FORMAT;

typedef struct
{
    BOOLEAN mpeg4_supported;      /* True if application can receive SBC */
    BOOLEAN h263_p0_supported;      /* True if application can receive PCM */
    BOOLEAN h263_p3_supported;      /* True if application can receive MP3 */
    BOOLEAN h263_p8_supported;      /* True if application can receive AAC */
}tBSA_AVK_MEDIA_VIDEO_RX_SUP_FORMAT;

typedef union
{
    tBSA_AVK_MEDIA_AUDIO_RX_SUP_FORMAT audio;
    tBSA_AVK_MEDIA_VIDEO_RX_SUP_FORMAT video;
}tBSA_AVK_MEDIA_RX_SUP_FORMAT;


/*
 * Structures used for parameters (transport)
 */

#define BSA_AVK_SERVICE_NAME_LEN_MAX    128
#define BSA_AVK_VENDOR_SIZE_MAX         BTA_MAX_VENDOR_DEPENDENT_DATA_LEN
#define BSA_AVK_REMOTE_CMD_MAX          100
#define BSA_AVK_PLAYER_SETTINGS_MAX         8

/* data associated with BSA_AVK_OPEN_MSG */
typedef struct
{
    tBSA_STATUS status;
    tBSA_AVK_CHNL channel;
    BD_ADDR bd_addr;
    BOOLEAN         initiator; /* connection initiator, local TRUE, peer FALSE */
    UINT8   ccb_handle;
} tBSA_AVK_OPEN_MSG;

/* data associated with BSA_AVK_CLOSE_MSG */
typedef struct
{
    tBSA_STATUS  status;
    tBSA_AVK_CHNL channel;
    BD_ADDR  bd_addr;
    UINT8  ccb_handle;
} tBSA_AVK_CLOSE_MSG;

/* data associated with BSA_AVK_START_MSG */
typedef struct
{
    tBSA_STATUS status;
    tBSA_AVK_MEDIA_RECEIVING media_receiving;
    BD_ADDR         bd_addr;
    UINT8           ccb_handle;
    BOOLEAN     streaming; /* TRUE if start streaming, FALSE if stream open only*/
    BOOLEAN     discarded; /* TRUE if received stream is discarded because another stream is active */
} tBSA_AVK_START_MSG;

/* data associated with tBSA_AVK_STOP_MSG */
typedef struct
{
    tBSA_STATUS status;
    BD_ADDR     bd_addr;
    UINT8       ccb_handle;
    BOOLEAN     streaming;   /* TRUE if stop streaming, FALSE if stream close only*/
    BOOLEAN     suspended;   /* TRUE if the event occured as a result of stream got suspended */
} tBSA_AVK_STOP_MSG;

/* data associated with BSA_AVK_RC_OPEN_MSG */
typedef struct
{
    UINT8   rc_handle;
    tBSA_AVK_FEAT peer_features;
    UINT16 peer_version; /* peer AVRCP version */
    tBSA_STATUS  status;
    BD_ADDR  bd_addr;
} tBSA_AVK_RC_OPEN_MSG;

/* data associated with BSA_AVK_RC_CLOSE_MSG */
typedef struct
{
    UINT8   rc_handle;
    tBSA_STATUS  status;
} tBSA_AVK_RC_CLOSE_MSG;

/* Data for BSA_AVK_REMOTE_CMD_EVT */
typedef struct
{
    UINT8           rc_handle;
    UINT8           label;
    UINT8           op_id;          /* Operation ID (see AVRC_ID_* defintions in avrc_defs.h) */
    UINT8           key_state;      /* AVRC_STATE_PRESS or AVRC_STATE_RELEASE */
    tAVRC_HDR       hdr;            /* Message header. */
    UINT8           len;
    UINT8           data[BSA_AVK_REMOTE_CMD_MAX];
} tBSA_AVK_REMOTE_REQ_MSG;

/* data associated with BSA_AVK_REMOTE_RSP_MSG */
typedef struct
{
    UINT8 rc_handle;
    tBSA_AVK_RC rc_id;
    tBSA_AVK_STATE key_state;
    UINT8 len;
} tBSA_AVK_REMOTE_RSP_MSG;

/* data type for BTA_AVK_API_VENDOR_CMD_EVT and RSP */
typedef struct
{
    UINT16          len;
    UINT8           label;
    tBSA_AVK_CODE    code;
    UINT32          company_id;
    UINT8           data[BSA_AVK_VENDOR_SIZE_MAX];
} tBSA_AVK_VENDOR_CMD_MSG;

typedef tBSA_AVK_VENDOR_CMD_MSG tBSA_AVK_VENDOR_RSP_MSG;

/* data associated with tBSA_AVK_CP_INFO_MSG */
typedef struct
{
    tBSA_AVK_CP_ID id;
    union
    {
        tBSA_AVK_CP_SCMS_FLAG  scmst_flag;
    } info;
} tBSA_AVK_CP_INFO_MSG;

#ifndef BSA_AVK_MEDIA_MAX_BUF_LEN
#define BSA_AVK_MEDIA_MAX_BUF_LEN (128*14)
#endif

/* data associated with UIPC media data */
typedef struct
{
    BT_HDR          hdr;
    UINT8           multimedia[BSA_AVK_MEDIA_MAX_BUF_LEN];
} tBSA_AVK_MEDIA;

/* data for meta data items */
#define BSA_AVK_ATTR_STR_LEN_MAX 102

/* media string */
typedef struct
{
    UINT8       data[BSA_AVK_ATTR_STR_LEN_MAX];
    UINT16      charset_id;
    UINT16      str_len;
} tBSA_AVK_STRING;

/* attibute entry */
typedef struct
{
    UINT32             attr_id;
    tBSA_AVK_STRING    name;
} tBSA_AVK_ATTR_ENTRY;

/* data for get element attribute response */
#define BSA_AVK_ELEMENT_ATTR_MAX 7

typedef struct
{
    tAVRC_STS               status;
    UINT8                   num_attr;
    tBSA_AVK_ATTR_ENTRY     attr_entry[BSA_AVK_ELEMENT_ATTR_MAX];
    UINT8       handle;
} tBSA_AVK_GET_ELEMENT_ATTR_MSG;

/* data for current app settings response */
#define BSA_AVK_APP_SETTING_MAX 6

typedef struct
{
    UINT8       pdu;
    tAVRC_STS   status;
    UINT8       opcode;         /* Op Code (copied from avrc_cmd.opcode by AVRC_BldResponse user. invalid one to generate according to pdu) */
    UINT8       num_val;
    tAVRC_APP_SETTING   vals[BSA_AVK_APP_SETTING_MAX];
    UINT8       handle;
} tBSA_GET_CUR_APP_VALUE_MSG;

/* data for set browsed player response */
typedef struct
{
    tAVRC_STS               status;
    UINT16                  uid_counter;
    UINT32                  num_items;
    UINT8                   folder_depth;
    tBSA_AVK_STRING         folder;
    BOOLEAN                 final; /*true if last entry*/
    UINT8                   handle;
} tBSA_AVK_SET_BR_PLAYER_MSG;

/* player item data */
typedef struct
{
    UINT16              player_id;      /* A unique identifier for this media player.*/
    UINT8               major_type;     /* Use AVRC_MJ_TYPE_AUDIO, AVRC_MJ_TYPE_VIDEO, AVRC_MJ_TYPE_BC_AUDIO, or AVRC_MJ_TYPE_BC_VIDEO.*/
    UINT32              sub_type;       /* Use AVRC_SUB_TYPE_NONE, AVRC_SUB_TYPE_AUDIO_BOOK, or AVRC_SUB_TYPE_PODCAST*/
    UINT8               play_status;    /* Use AVRC_PLAYSTATE_STOPPED, AVRC_PLAYSTATE_PLAYING, AVRC_PLAYSTATE_PAUSED, AVRC_PLAYSTATE_FWD_SEEK,
                                            AVRC_PLAYSTATE_REV_SEEK, or AVRC_PLAYSTATE_ERROR*/
    tAVRC_FEATURE_MASK  features;       /* Supported feature bit mask*/
    tBSA_AVK_STRING     name;           /* The player name, name length and character set id.*/
} tBSA_AVRC_ITEM_PLAYER;

/* folder item data */
typedef struct
{
    tAVRC_UID           uid;            /* The uid of this folder */
    UINT8               type;           /* Use AVRC_FOLDER_TYPE_MIXED, AVRC_FOLDER_TYPE_TITLES,
                                           AVRC_FOLDER_TYPE_ALNUMS, AVRC_FOLDER_TYPE_ARTISTS, AVRC_FOLDER_TYPE_GENRES,
                                           AVRC_FOLDER_TYPE_PLAYLISTS, or AVRC_FOLDER_TYPE_YEARS.*/
    BOOLEAN             playable;       /* TRUE, if the folder can be played. */
    tBSA_AVK_STRING     name;           /* The folder name, name length and character set id. */
} tBSA_AVRC_ITEM_FOLDER;

/* media item data */
typedef struct
{
    tAVRC_UID           uid;            /* The uid of this media element item */
    UINT8               type;           /* Use AVRC_MEDIA_TYPE_AUDIO or AVRC_MEDIA_TYPE_VIDEO. */
    tBSA_AVK_STRING     name;           /* The media name, name length and character set id. */
    UINT8               attr_count;     /* The number of attributes in p_attr_list */
    tBSA_AVK_ATTR_ENTRY attr_entry[BSA_AVK_ELEMENT_ATTR_MAX];    /* Attribute entry list. */
} tBSA_AVRC_ITEM_MEDIA;

/* avrcp item (player,folder or media) */
typedef struct
{
    UINT8                   item_type;  /* AVRC_ITEM_PLAYER, AVRC_ITEM_FOLDER, or AVRC_ITEM_MEDIA */
    union
    {
        tBSA_AVRC_ITEM_PLAYER   player;     /* The properties of a media player item.*/
        tBSA_AVRC_ITEM_FOLDER   folder;     /* The properties of a folder item.*/
        tBSA_AVRC_ITEM_MEDIA    media;      /* The properties of a media item.*/
    } u;
} tBSA_AVRC_ITEM;

/* data for get folder items response */
typedef struct
{
    tAVRC_STS               status;
    UINT16                  uid_counter;
    UINT16                  item_count;
    tBSA_AVRC_ITEM          item;
    BOOLEAN                 final; /*true if last entry*/
    UINT8                   handle;
} tBSA_AVK_GET_ITEMS_MSG;

/* data for registered notification response */
typedef struct
{
    tAVRC_REG_NOTIF_RSP     rsp;
    UINT8                   handle;
} tBSA_AVK_REG_NOTIF_MSG;

/* data for list app attr response */
typedef struct
{
    tAVRC_LIST_APP_ATTR_RSP     rsp;
    UINT8                   handle;
} tBSA_AVK_LIST_APP_ATTR_MSG;

/* data for list app attr value response */
typedef struct
{
    tAVRC_LIST_APP_VALUES_RSP     rsp;
    UINT8                   handle;
} tBSA_AVK_LIST_APP_VALUES_MSG;

/* data for set player app value response */
typedef struct
{
    tAVRC_RSP     rsp;
    UINT8         handle;
} tBSA_AVK_SET_PLAYER_APP_VALUE_MSG;

/* data for set play status response */
typedef struct
{
    tAVRC_GET_PLAY_STATUS_RSP     rsp;
    UINT8         handle;
} tBSA_AVK_GET_PLAY_STATUS_MSG;

/* data for set addressed player response */
typedef struct
{
    tAVRC_RSP     rsp;
    UINT8         handle;
} tBSA_AVK_SET_ADDRESSED_PLAYER_MSG;

/* data for set change path response */
typedef struct
{
    tAVRC_CHG_PATH_RSP  rsp;
    UINT8               handle;
} tBSA_AVK_CHG_PATH_MSG;

/* data for get element attr response */
typedef struct
{
    tBSA_AVK_GET_ELEMENT_ATTR_MSG  rsp;
    UINT8               handle;
} tBSA_AVK_GET_ITEM_ATTR_MSG;

/* data for play item response */
typedef struct
{
    tAVRC_RSP  rsp;
    UINT8      handle;
} tBSA_AVK_PLAY_ITEM_MSG;

/* data for now playing response */
typedef struct
{
    tAVRC_RSP  rsp;
    UINT8      handle;
} tBSA_AVK_NOW_PLAYING_MSG;

/* data for abs volume command */
typedef struct
{
    tAVRC_SET_VOLUME_CMD abs_volume_cmd;
    UINT8                handle;
    UINT8                label;
} tBSA_AVK_ABS_VOLUME_CMD_MSG;

/* data for register notification cmd */
typedef struct
{
    tAVRC_REG_NOTIF_CMD reg_notif_cmd;
    UINT8               handle;
    UINT8               label;
} tBSA_AVK_REG_NOTIF_CMD_MSG;

/* union of data associated with AV callback */
typedef union
{
    tBSA_AVK_OPEN_MSG open;
    tBSA_AVK_CLOSE_MSG close;
    tBSA_AVK_START_MSG start;
    tBSA_AVK_STOP_MSG stop;
    tBSA_AVK_RC_OPEN_MSG rc_open;
    tBSA_AVK_RC_CLOSE_MSG rc_close;
    tBSA_AVK_REMOTE_REQ_MSG remote_cmd;
    tBSA_AVK_REMOTE_RSP_MSG remote_rsp;
    tBSA_AVK_VENDOR_CMD_MSG vendor_cmd;
    tBSA_AVK_VENDOR_RSP_MSG vendor_rsp;
    tBSA_AVK_CP_INFO_MSG cp_info;

    /* meta commands responses */
    tBSA_AVK_REG_NOTIF_MSG              reg_notif;
    tBSA_AVK_REG_NOTIF_CMD_MSG          reg_notif_cmd;
    tBSA_AVK_LIST_APP_ATTR_MSG          list_app_attr;
    tBSA_AVK_LIST_APP_VALUES_MSG        list_app_values;
    tBSA_AVK_SET_PLAYER_APP_VALUE_MSG   set_app_val;
    tBSA_GET_CUR_APP_VALUE_MSG          get_cur_app_val;
    tBSA_AVK_GET_ELEMENT_ATTR_MSG       elem_attr;
    tBSA_AVK_GET_PLAY_STATUS_MSG        get_play_status;
    tBSA_AVK_ABS_VOLUME_CMD_MSG         abs_volume;

    /* browsing command responses */
    tBSA_AVK_SET_ADDRESSED_PLAYER_MSG   addr_player;
    tBSA_AVK_SET_BR_PLAYER_MSG          br_player;
    tBSA_AVK_GET_ITEMS_MSG              get_items;
    tBSA_AVK_CHG_PATH_MSG               chg_path;
    tBSA_AVK_GET_ITEM_ATTR_MSG          item_attr;
    tBSA_AVK_PLAY_ITEM_MSG              play_item;
    tBSA_AVK_NOW_PLAYING_MSG            now_playing;
} tBSA_AVK_MSG;

/* AV callback */
typedef void (tBSA_AVK_CBACK)(tBSA_AVK_EVT event, tBSA_AVK_MSG *p_data);

/* data associated with BSA_AvkEnable */
typedef struct
{
    tBSA_SEC_AUTH sec_mask;
    tBSA_AVK_FEAT features;
    tBSA_AVK_CBACK *p_cback;
} tBSA_AVK_ENABLE;

/* data associated with BSA_AvkDisable */
typedef struct
{
    UINT16 dummy;
} tBSA_AVK_DISABLE;

/* bitmask of AVRCP notifications to be registered */
typedef UINT32 tBSA_AVK_REG_NOTIFICATIONS;

/* data associated with BSA_AvkRegister */
typedef struct
{
    tUIPC_CH_ID uipc_channel;
    char service_name[BSA_AVK_SERVICE_NAME_LEN_MAX];
    tBSA_AVK_MEDIA_RX_SUP_FORMAT media_sup_format;
    tBSA_AVK_REG_NOTIFICATIONS reg_notifications;
} tBSA_AVK_REGISTER;

/* data associated with BSA_AvkDeregister */
typedef struct
{
    tBSA_AVK_CHNL channel;
} tBSA_AVK_DEREGISTER;

/* data associated with BSA_AvkOpen */
typedef struct
{
    BD_ADDR bd_addr;
    tBSA_AVK_CHNL channel;
    tBSA_SEC_AUTH sec_mask;
} tBSA_AVK_OPEN;

/* data associated with BSA_AvkClose */
typedef struct
{
    UINT8 ccb_handle;   /*AVDT*/
    UINT8 rc_handle;    /*AVRCP*/
} tBSA_AVK_CLOSE;

typedef struct
{
    UINT8 ccb_handle;   /*AVDT*/
} tBSA_AVK_CLOSE_STR;

/* data associated with BSA_AvkStart */
typedef struct
{
    tBSA_AVK_MEDIA_RECEIVING media_receiving;
} tBSA_AVK_START;

/* data associated with BSA_AvkStop */
typedef struct
{
    BOOLEAN pause;
} tBSA_AVK_STOP;

/* data associated with BSA_AvkRemoteCmd */
typedef struct
{
    UINT8 label;
    tBSA_AVK_RC rc_id;        /* remote contreol opperation id */
    tBSA_AVK_STATE key_state; /* key press key release state */
    UINT8 rc_handle;          /* connected RC handle */
    UINT8 data[BSA_RC_MAX_PARAM_LEN];
    UINT16 length;
} tBSA_AVK_REM_CMD;

/* data associated with BSA_AvkRemoteRsp */
typedef struct
{
    UINT8 label;
    tBSA_AVK_RC op_id;        /* remote contreol opperation id */
    tBSA_AVK_STATE key_state; /* key press key release state */
    UINT8 handle;          /* connected RC handle */
    tBTA_RC_RSP avrc_rsp;
    UINT8 data[BSA_RC_MAX_PARAM_LEN];
    UINT16 length;
} tBSA_AVK_REM_RSP;

/* data associated with BSA_AvkVendorCmd */
typedef struct
{
    tBSA_AVK_RC rc_handle;
    UINT8 ctype;
    UINT8 label;
    UINT8 data[BSA_AVK_VENDOR_SIZE_MAX];
    UINT16 length;
} tBSA_AVK_VEN_CMD;

/* data associated with BSA_AvkCancel */
typedef struct
{
    UINT8 ccb_handle;

} tBSA_AVK_CANCEL_CMD;

/* data associated with BSA_AvkListPlayerAttrCmd */
typedef struct
{
    UINT8 label;
    UINT8 rc_handle;          /* connected RC handle */
} tBSA_AVK_LIST_PLAYER_ATTR;

/* data associated with BSA_AvkListPlayerValuesCmd */
typedef struct
{
    UINT8 label;
    UINT8 rc_handle;          /* connected RC handle */
    UINT8 attr;
} tBSA_AVK_LIST_PLAYER_VALUES;

/* data associated with BSA_AvkGetPlayerValueCmd */
typedef struct
{
    UINT8 label;
    UINT8 rc_handle;          /* connected RC handle */
    UINT8 attrs[BSA_AVK_PLAYER_SETTINGS_MAX];
    UINT8 num_attr;
} tBSA_AVK_GET_PLAYER_VALUE;


/* data associated with BSA_AvkSetPlayerValueCmd */
typedef struct
{
    UINT8 label;
    UINT8 rc_handle;          /* connected RC handle */
    UINT8 num_attr;
    UINT8 player_attr[BSA_AVK_PLAYER_SETTINGS_MAX];
    UINT8 player_value[BSA_AVK_PLAYER_SETTINGS_MAX];
} tBSA_AVK_SET_PLAYER_VALUE;

/* data associated with BSA_AvkGetPlayerAttrTextCmd */
typedef tBSA_AVK_GET_PLAYER_VALUE tBSA_AVK_GET_PLAYER_ATTR_TEXT;

/* data associated with BSA_AvkGetPlayerValueTextCmd */
typedef struct
{
    UINT8 label;
    UINT8 rc_handle;          /* connected RC handle */
    UINT8 attr_id;
    UINT8 attrs[BSA_AVK_PLAYER_SETTINGS_MAX];
    UINT8 num_val;
} tBSA_AVK_GET_PLAYER_VALUE_TEXT;



/* data associated with BSA_AvkGetElementAttrCmd */
typedef struct
{
    UINT8 label;
    UINT8 rc_handle;          /* connected RC handle */
    UINT8 element_id;
    UINT32 attrs[BSA_AVK_PLAYER_SETTINGS_MAX];
    UINT8 num_attr;
} tBSA_AVK_GET_ELEMENT_ATTR;

/* data associated with BSA_AvkGetPlayStatusCmd */
typedef tBSA_AVK_LIST_PLAYER_ATTR tBSA_AVK_GET_PLAY_STATUS;

/* data associated with BSA_AvkSetAddressedPlayerCmd */
typedef struct
{
    UINT8 label;
    UINT8 rc_handle;          /* connected RC handle */
    UINT16      player_id;
} tBSA_AVK_SET_ADDR_PLAYER;

/* data associated with BSA_AvkSetBrowsedPlayerCmd */
typedef tBSA_AVK_SET_ADDR_PLAYER tBSA_AVK_SET_BROWSED_PLAYER;

/* data associated with BSA_AvkChangePathCmd */
typedef struct
{
    UINT8 label;
    UINT8 rc_handle;
    UINT16      uid_counter;
    UINT8       direction;
    tAVRC_UID   folder_uid;
} tBSA_AVK_CHG_PATH;


/* data associated with BSA_AvkGetFolderItemsCmd */
typedef struct
{
    UINT8       label;
    UINT8       rc_handle;          /* connected RC handle */
    UINT8       scope;
    UINT32      start_item;
    UINT32      end_item;
    UINT8       attr_count;
    UINT32       attrs[BSA_AVK_PLAYER_SETTINGS_MAX];

} tBSA_AVK_GET_FOLDER_ITEMS;


/* data associated with BSA_AvkGetItemsAttrCmd */
typedef struct
{
    UINT8       label;
    UINT8       rc_handle;          /* connected RC handle */
    UINT8       scope;
    tAVRC_UID   uid;
    UINT16      uid_counter;
    UINT8       attr_count;
    UINT32      attrs[BSA_AVK_PLAYER_SETTINGS_MAX];

} tBSA_AVK_GET_ITEMS_ATTR;

/* data associated with BSA_AvkPlayItemCmd */
typedef struct
{
    UINT8       label;
    UINT8       rc_handle;          /* connected RC handle */
    UINT8       scope;
    tAVRC_UID   uid;
    UINT16      uid_counter;
} tBSA_AVK_PLAY_ITEM;


/* data associated with BSA_AvkAddToPlayCmd */
typedef struct
{
    UINT8       label;
    UINT8       rc_handle;          /* connected RC handle */
    UINT8       scope;
    tAVRC_UID   uid;
    UINT16      uid_counter;
} tBSA_AVK_ADD_TO_PLAY;

/* data associated with BSA_AvkSetAbsVolRsp */
typedef struct
{
    UINT8       label;
    UINT8       volume;
    UINT8       rc_handle;          /* connected RC handle */
} tBSA_AVK_SET_ABS_VOLUME_RSP;

/* data associated with BSA_AvkRegNotfnRsp */
typedef struct
{
    UINT8                 label;
    tAVRC_REG_NOTIF_RSP   reg_notf;
    UINT8                 rc_handle;          /* connected RC handle */
    tBTA_AV_CODE          code;
} tBSA_AVK_REG_NOTIF_RSP;

/* data associated with BSA_AvkRelayAudio */
typedef struct
{
    BOOLEAN audio_relay;            /* TRUE: To relay incoming AVK audio as AV source */
} tBSA_AVK_RELAY_AUDIO;

/* data associated with BSA_AvkDelayReport */
typedef struct
{
    UINT16 delay;  /* delay value:in unit as 0.1 ms */
} tBSA_AVK_DELAY_REPORT;

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/

/*******************************************************************************
 **
 ** Function         BSA_AvkEnableInit
 **
 ** Description      Init a structure tBSA_AVK_ENABLE to be used with BSA_AvkEnable
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkEnableInit(tBSA_AVK_ENABLE *pEnable);

/*******************************************************************************
**
** Function         BSA_AvkEnable
**
** Description      Enable the advanced audio/video service. When the enable
**                  operation is complete function will returns BSA_SUCCESS. This function must
**                  be called before other function in the AV API are
**                  called.
**
** Returns          void
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkEnable(tBSA_AVK_ENABLE *pEnable);

/*******************************************************************************
 **
 ** Function         BSA_AvkDisableInit
 **
 ** Description      Init structure tBSA_AVK_DISABLE to be used with BSA_AvkDisable
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkDisableInit(tBSA_AVK_DISABLE *p_disable);

/*******************************************************************************
**
** Function         BSA_AvkDisable
**
** Description      Disable the advanced audio/video service.
**
**
** Returns          void
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkDisable(tBSA_AVK_DISABLE *pDisable);


/*******************************************************************************
 **
 ** Function         BSA_AvkRegisterInit
 **
 ** Description      Init structure tBSA_AVK_DEREGISTER to be used with BSA_AvkRegister
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRegisterInit(tBSA_AVK_REGISTER *p_register);

/*******************************************************************************
**
** Function         BSA_AvkRegister
**
** Description      Register the audio or video service to stack. When the
**                  operation is complete the function returns BSA_SUCCESS. This function must
**                  be called before AVDT stream is open.
**
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRegister(tBSA_AVK_REGISTER *pRegister);

/*******************************************************************************
 **
 ** Function         BSA_AvkDeregisterInit
 **
 ** Description      Init structure tBSA_AVK_DEREGISTER to be used with BSA_AvkDeregister
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkDeregisterInit(tBSA_AVK_DEREGISTER *p_deregister);

/*******************************************************************************
**
** Function         BSA_AvkDeregister
**
** Description      Deregister the audio or video service
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkDeregister(tBSA_AVK_DEREGISTER *pRegister);

/*******************************************************************************
 **
 ** Function         BSA_AvkOpenInit
 **
 ** Description      Init structure tBSA_AVK_OPEN to be used with BSA_AvkOpen
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkOpenInit(tBSA_AVK_OPEN * p_open);

/*******************************************************************************
**
** Function         BSA_AvkOpen
**
** Description      Opens an advanced audio/video connection to a peer device.
**                  When connection is open callback function is called
**                  with a BSA_AVK_OPEN_EVT for each channel.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkOpen(tBSA_AVK_OPEN *p_open);

/*******************************************************************************
 **
 ** Function         BSA_AvkCloseInit
 **
 ** Description      Init structure tBSA_AVK_CLOSE to be used with BSA_AvkClose
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCloseInit(tBSA_AVK_CLOSE *p_close);

/*******************************************************************************
**
** Function         BSA_AvkClose
**
** Description      Close the current streams.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkClose(tBSA_AVK_CLOSE *pClose);

/*******************************************************************************
 **
 ** Function         BSA_AvkStartInit
 **
 ** Description      Init sturcture p_close to be used in BSA_AvkClose
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_AvkStartInit(tBSA_AVK_START * p_start);

/*******************************************************************************
**
** Function         BSA_AvkStart
**
** Description      Start audio/video stream data transfer.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkStart(tBSA_AVK_START *pStart);

/*******************************************************************************
 **
 ** Function         BSA_AvkStopInit
 **
 ** Description      Init structure tBSA_AVK_STOP to be used with BSA_AvkStopInit
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkStopInit(tBSA_AVK_STOP *p_stop);

/*******************************************************************************
**
** Function         BSA_AvkStop
**
** Description      Stop audio/video stream data transfer.
**                  If suspend is TRUE, this function sends AVDT suspend signal
**                  to the connected peer(s).
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkStop(tBSA_AVK_STOP *pStop);


/*******************************************************************************
**
** Function         BSA_AvkRemoteCmdInit
**
** Description      Init a structure tBSA_AVK_REM_CMD to be used with BSA_AvkRemoteCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRemoteCmdInit(tBSA_AVK_REM_CMD *pRemCmd);

/*******************************************************************************
**
** Function         BSA_AvkRemoteCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_RCCT.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRemoteCmd(tBSA_AVK_REM_CMD *pRemCmd);

/*******************************************************************************
**
** Function         BSA_AvkVendorCmdInit
**
** Description      Init a structure tBSA_AVK_VEN_CMD to be used with BSA_AvkVendorCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkVendorCmdInit(tBSA_AVK_VEN_CMD *pVenCmd);

/*******************************************************************************
**
** Function         BSA_AvkVendorCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_RCCT.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkVendorCmd(tBSA_AVK_VEN_CMD *pVenCmd);

/*******************************************************************************
**
** Function         BSA_AvkCancelCmdInit
**
** Description      Init a structure tBSA_AVK_VEN_CMD to be used with BSA_AvkCancelCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCancelCmdInit(tBSA_AVK_CANCEL_CMD *pCmd);

/*******************************************************************************
**
** Function         BSA_AvkCancelCmd
**
** Description      Send a command to cancel connection.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCancelCmd(tBSA_AVK_CANCEL_CMD *pCmd);


/*******************************************************************************
**
** Function         BSA_AvkListPlayerAttrCmdInit
**
** Description      Init a structure tBSA_AVK_LIST_PLAYER_ATTR to be used with BSA_AvkListPlayerAttrCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkListPlayerAttrCmdInit(tBSA_AVK_LIST_PLAYER_ATTR *pListPlayerAttrCmd);


/*******************************************************************************
**
** Function         BSA_AvkListPlayerAttrCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature tBSA_AVK_LIST_PLAYER_ATTR.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkListPlayerAttrCmd(tBSA_AVK_LIST_PLAYER_ATTR *pListPlayerAttrCmd);


/*******************************************************************************
**
** Function         BSA_AvkListPlayerValuesCmdInit
**
** Description      Init a structure tBSA_AVK_LIST_PLAYER_VALUES to be used with BSA_AvkListPlayerValuesCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkListPlayerValuesCmdInit(tBSA_AVK_LIST_PLAYER_VALUES *pListPlayerValuesCmd);

/*******************************************************************************
**
** Function         BSA_AvkListPlayerValuesCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature tBSA_AVK_LIST_PLAYER_VALUES.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkListPlayerValuesCmd(tBSA_AVK_LIST_PLAYER_VALUES *pListPlayerValuesCmd);

/*******************************************************************************
**
** Function         BSA_AvkGetPlayerValueCmdInit
**
** Description      Init a structure tBSA_AVK_GET_PLAYER_VALUE to be used with BSA_AvkGetPlayerValueCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerValueCmdInit(tBSA_AVK_GET_PLAYER_VALUE *pGetPlayerValueCmd);

/*******************************************************************************
**
** Function         BSA_AvkGetPlayerValueCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature tBSA_AVK_GET_PLAYER_VALUE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerValueCmd(tBSA_AVK_GET_PLAYER_VALUE *pGetPlayerValueCmd);

/*******************************************************************************
**
** Function         BSA_AvkSetPlayerValueCmdInit
**
** Description      Init a structure tBSA_AVK_SET_PLAYER_VALUE to be used with BSA_AvkSetPlayerValueCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetPlayerValueCmdInit(tBSA_AVK_SET_PLAYER_VALUE *pSetPlayerValueCmd);


/*******************************************************************************
**
** Function         BSA_AvkSetPlayerValueCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_METADATA.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetPlayerValueCmd(tBSA_AVK_SET_PLAYER_VALUE *pSetPlayerValueCmd);



/*******************************************************************************
**
** Function         BSA_AvkGetPlayerAttrTextCmdInit
**
** Description      Init a structure tBSA_AVK_GET_PLAYER_ATTR_TEXT to be used with BSA_AvkGetPlayerAttrTextCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerAttrTextCmdInit(tBSA_AVK_GET_PLAYER_ATTR_TEXT *pGetPlayerAttrTextCmd);

/*******************************************************************************
**
** Function         BSA_AvkGetPlayerAttrTextCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_METADATA.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerAttrTextCmd(tBSA_AVK_GET_PLAYER_ATTR_TEXT *pGetPlayerAttrTextCmd);


/*******************************************************************************
**
** Function         BSA_AvkGetPlayerValueTextCmdInit
**
** Description      Init a structure tBSA_AVK_GET_PLAYER_VALUE_TEXT to be used with BSA_AvkGetPlayerValueTextCmdInit
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerValueTextCmdInit(tBSA_AVK_GET_PLAYER_VALUE_TEXT *pGetPlayerValueTextCmd);

/*******************************************************************************
**
** Function         BSA_AvkGetPlayerValueTextCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_METADATA.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerValueTextCmd(tBSA_AVK_GET_PLAYER_VALUE_TEXT *pGetPlayerValueTextCmd);


/*******************************************************************************
**
** Function         BSA_AvkGetElementAttrCmdInit
**
** Description      Init a structure tBSA_AVK_GET_ELEMENT_ATTR to be used with BSA_AvkGetElementAttrCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetElementAttrCmdInit(tBSA_AVK_GET_ELEMENT_ATTR *pGetElemAttrCmd);


/*******************************************************************************
**
** Function         BSA_AvkGetElementAttrCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_METADATA.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetElementAttrCmd(tBSA_AVK_GET_ELEMENT_ATTR *pGetElemAttrCmd);

/*******************************************************************************
**
** Function         BSA_AvkGetPlayStatusCmdInit
**
** Description      Init a structure tBSA_AVK_GET_PLAY_STATUS to be used with BSA_AvkGetPlayStatusCmdInit
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayStatusCmdInit(tBSA_AVK_GET_PLAY_STATUS *pPlayStatusCmd);

/*******************************************************************************
**
** Function         BSA_AvkGetPlayStatusCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_METADATA.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayStatusCmd(tBSA_AVK_GET_PLAY_STATUS *pPlayStatusCmd);

/*******************************************************************************
**
** Function         BSA_AvkSetAddressedPlayerCmdInit
**
** Description      Init a structure tBSA_AVK_SET_ADDR_PLAYER to be used with BSA_AvkSetAddressedPlayerCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetAddressedPlayerCmdInit(tBSA_AVK_SET_ADDR_PLAYER *pAddrPlayerCmd);

/*******************************************************************************
**
** Function         BSA_AvkSetAddressedPlayerCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetAddressedPlayerCmd(tBSA_AVK_SET_ADDR_PLAYER *pAddrPlayerCmd);

/*******************************************************************************
**
** Function         BSA_AvkSetBrowsedPlayerCmdInit
**
** Description      Init a structure tBSA_AVK_SET_BROWSED_PLAYER to be used with BSA_AvkSetBrowsedPlayerCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetBrowsedPlayerCmdInit(tBSA_AVK_SET_BROWSED_PLAYER *pBrowsedPlayerCmd);


/*******************************************************************************
**
** Function         BSA_AvkSetBrowsedPlayerCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetBrowsedPlayerCmd(tBSA_AVK_SET_BROWSED_PLAYER *pBrowsedPlayerCmd);

/*******************************************************************************
**
** Function         BSA_AvkChangePathCmdInit
**
** Description      Init a structure tBSA_AVK_CHG_PATH to be used with BSA_AvkChangePathCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkChangePathCmdInit(tBSA_AVK_CHG_PATH *pChangePathCmd);


/*******************************************************************************
**
** Function         BSA_AvkChangePathCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkChangePathCmd(tBSA_AVK_CHG_PATH *pChangePathCmd);


/*******************************************************************************
**
** Function         BSA_AvkGetFolderItemsCmdInit
**
** Description      Init a structure tBSA_AVK_GET_FOLDER_ITEMS to be used with BSA_AvkGetFolderItemsCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetFolderItemsCmdInit(tBSA_AVK_GET_FOLDER_ITEMS *pGetFolderItemsCmd);

/*******************************************************************************
**
** Function         BSA_AvkGetFolderItemsCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetFolderItemsCmd(tBSA_AVK_GET_FOLDER_ITEMS *pGetFolderItemsCmd);

/*******************************************************************************
**
** Function         BSA_AvkGetItemsAttrCmdInit
**
** Description      Init a structure tBSA_AVK_GET_ITEMS_ATTR to be used with BSA_AvkGetItemsAttrCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetItemsAttrCmdInit(tBSA_AVK_GET_ITEMS_ATTR *pGetItemsAttrCmd);


/*******************************************************************************
**
** Function         BSA_AvkGetItemsAttrCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetItemsAttrCmd(tBSA_AVK_GET_ITEMS_ATTR *pGetItemsAttrCmd);

/*******************************************************************************
**
** Function         BSA_AvkPlayItemCmdInit
**
** Description      Init a structure tBSA_AVK_PLAY_ITEM to be used with BSA_AvkPlayItemCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkPlayItemCmdInit(tBSA_AVK_PLAY_ITEM *pPlayItemCmd);

/*******************************************************************************
**
** Function         BSA_AvkPlayItemCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkPlayItemCmd(tBSA_AVK_PLAY_ITEM *pPlayItemCmd);

/*******************************************************************************
**
** Function         BSA_AvkAddToPlayCmdInit
**
** Description      Init a structure tBSA_AVK_ADD_TO_PLAY to be used with BSA_AvkAddToPlayCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkAddToPlayCmdInit(tBSA_AVK_ADD_TO_PLAY *pAddToPlayCmd);

/*******************************************************************************
**
** Function         BSA_AvkAddToPlayCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkAddToPlayCmd(tBSA_AVK_ADD_TO_PLAY *pAddToPlayCmd);

/*******************************************************************************
 **
 ** Function         BSA_AvkSetAbsVolRspInit
 **
 ** Description      Init a structure (tBSA_AVK_SET_ABS_VOLUME_RSP to be used with
 **                  BSA_AvkSetAbsVolRsp
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetAbsVolumeRspInit(tBSA_AVK_SET_ABS_VOLUME_RSP *pAbsVol);

/*******************************************************************************
 **
 ** Function         BSA_AvkSetAbsVolRsp
 **
 ** Description      Send a abs vol response
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetAbsVolumeRsp(tBSA_AVK_SET_ABS_VOLUME_RSP *pAbsVol);

/*******************************************************************************
 **
 ** Function         BSA_AvkRegNotifRspInit
 **
 ** Description      Init a structure (tBSA_AVK_REG_NOTIF_RSP to be used with
 **                  BSA_AvkRegNotfnRsp
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRegNotifRspInit(tBSA_AVK_REG_NOTIF_RSP *pRegNotf);

/*******************************************************************************
 **
 ** Function         BSA_AvkRegNotifRsp
 **
 ** Description      Send register notification response
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRegNotifRsp(tBSA_AVK_REG_NOTIF_RSP *pRegNotf);

/*******************************************************************************
 **
 ** Function         BSA_AvkOpenRcInit
 **
 ** Description      Init structure tBSA_AVK_OPEN to be used with BSA_AvkOpenRc
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkOpenRcInit(tBSA_AVK_OPEN * p_open);


/*******************************************************************************
 **
 ** Function         BSA_AvkOpenRc
 **
 ** Description      Opens an avrcp controller connection to a peer device. AVK must already be connected.
 **                  When connection is open callback function is called
 **                  with a BSA_AVK_OPEN_RC_EVT.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkOpenRc(tBSA_AVK_OPEN * p_open);

/*******************************************************************************
 **
 ** Function         BSA_AvkCloseRcInit
 **
 ** Description      Init structure tBSA_AVK_CLOSE to be used with BSA_AvkCloseRc
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCloseRcInit(tBSA_AVK_CLOSE *p_close);


/*******************************************************************************
 **
 ** Function         BSA_AvkCloseRc
 **
 ** Description      Close an AVRC (controller) connection
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCloseRc(tBSA_AVK_CLOSE *p_close);


/*******************************************************************************
 **
 ** Function         BSA_AvkCloseStrInit
 **
 ** Description      Init structure tBSA_AVK_CLOSE_STR to be used with BSA_AvkCloseStr
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCloseStrInit(tBSA_AVK_CLOSE_STR *pCloseStr);


/*******************************************************************************
 **
 ** Function         BSA_AvkCloseStr
 **
 ** Description      Close an A2DP Steam connection
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCloseStr(tBSA_AVK_CLOSE_STR *pCloseStr);

/*******************************************************************************
**
** Function         BSA_AvkRemoteRspInit
**
** Description      Init a structure tBSA_AVK_REM_RSP to be used with BSA_AvkRemoteRspInit
**
** Returns          void
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRemoteRspInit(tBSA_AVK_REM_RSP *pRemRsp);

/*******************************************************************************
**
** Function         BSA_AvkRemoteRsp
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_RCCT.
**
** Returns          void
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRemoteRsp(tBSA_AVK_REM_RSP *pRemRsp);

#if (defined(BSA_AVK_AV_AUDIO_RELAY) && (BSA_AVK_AV_AUDIO_RELAY == TRUE))

/*******************************************************************************
**
** Function         BSA_AvkRelayAudioInit
**
** Description      Init a structure tBSA_AVK_RELAY_AUDIO to be used with BSA_AvkRelayAudio
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRelayAudioInit(tBSA_AVK_RELAY_AUDIO *pRelayAudio);


/*******************************************************************************
**
** Function         BSA_AvkRelayAudio
**
** Description      Start/stop audio relay from AVK to AV
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRelayAudio(tBSA_AVK_RELAY_AUDIO *pRelayAudio);

#endif

/*******************************************************************************
 **
 ** Function         BSA_AvkDelayReportInit
 **
 ** Description      Init structure tBSA_AVK_DELAY_REPORT
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_AvkDelayReportInit(tBSA_AVK_DELAY_REPORT *pDelayRpt);

/*******************************************************************************
**
** Function         BSA_AvkDelayReport
**
** Description      send delay report from AVK to AV
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_AvkDelayReport(tBSA_AVK_DELAY_REPORT *pDelayRpt);

#ifdef __cplusplus
}
#endif

#endif /* BSA_AVK_API_H */
