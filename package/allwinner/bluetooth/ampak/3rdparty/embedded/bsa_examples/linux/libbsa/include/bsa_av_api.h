/*****************************************************************************
 **
 **  Name:           bsa_av_api.h
 **
 **  Description:    This is the public interface file for the advanced
 **                  audio/video streaming (AV) subsystem of BSA, Widcomm's
 **                  Bluetooth application layer for mobile phones.
 **
 **  Copyright (c) 2004-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_AV_API_H
#define BSA_AV_API_H

#include "bt_target.h"
#include "bta_av_api.h"
#include "uipc.h"

/* for tBSA_STATUS */
#include "bsa_status.h"
#include "a2d_api.h"
#include "a2d_sbc.h"
#include "a2d_m12.h"
#include "a2d_m24.h"

/* for tBSA_SEC_AUTH */
#include "bsa_sec_api.h"

/* For Content Protection definitions */
#include "bta_av_co.h"

/*****************************************************************************
 **  Constants and data types
 *****************************************************************************/

/* codec type */
#define BSA_AV_CODEC_SBC        A2D_MEDIA_CT_SBC        /* SBC media codec type */
#define BSA_AV_CODEC_M12        A2D_MEDIA_CT_M12        /* MPEG-1, 2 Audio media codec type */
#define BSA_AV_CODEC_M24        A2D_MEDIA_CT_M24        /* MPEG-2, 4 AAC media codec type */
#define BSA_AV_CODEC_ATRAC      A2D_MEDIA_CT_ATRAC      /* ATRAC family media codec type */
#define BSA_AV_CODEC_PCM        0x5                     /* Raw PCM */
#define BSA_AV_CODEC_APTX       0x6                     /* apt-X */
#define BSA_AV_CODEC_SEC        0x7                     /* SEC */
#define BSA_AV_CODEC_H263_P0    VDP_MEDIA_CT_H263_P0    /* H.263 baseline (profile 0) */
#define BSA_AV_CODEC_MPEG4      VDP_MEDIA_CT_MPEG4      /* MPEG-4 Visual Simple Profile */
#define BSA_AV_CODEC_H263_P3    VDP_MEDIA_CT_H263_P3    /* H.263 profile 3 */
#define BSA_AV_CODEC_H263_P8    VDP_MEDIA_CT_H263_P8    /* H.263 profile 8 */
typedef UINT8 tBSA_AV_CODEC_ID;

/* AV features masks */
#define BSA_AV_FEAT_RCTG        BTA_AV_FEAT_RCTG      /* remote control target */
#define BSA_AV_FEAT_RCCT        BTA_AV_FEAT_RCCT      /* remote control controller */
#define BSA_AV_FEAT_VENDOR      BTA_AV_FEAT_VENDOR    /* remote control vendor dependent commands */
#define BSA_AV_FEAT_METADATA    BTA_AV_FEAT_METADATA  /* remote control Metadata Transfer command/response */
#define BSA_AV_FEAT_BROWSE      BTA_AV_FEAT_BROWSE    /* use browsing channel */
#define BSA_AV_FEAT_PROTECT     BTA_AV_FEAT_PROTECT   /* streaming media content protection */
typedef UINT16 tBSA_AV_FEAT;

/* AV channel values */
#define BSA_AV_CHNL_MSK         BTA_AV_CHNL_MSK
#define BSA_AV_CHNL_AUDIO       BTA_AV_CHNL_AUDIO   /* audio channel */
#define BSA_AV_CHNL_VIDEO       BTA_AV_CHNL_VIDEO   /* video channel */
#define BSA_AV_CHNL_AUDIO_BCST  0x01                /* Broadcast Audio */
typedef UINT8 tBSA_AV_CHNL;

/* Content Protection Type Id */
#define BSA_AV_CP_ID_NONE       0x0000              /* No Content Protection */
#define BSA_AV_CP_ID_SCMS       BTA_AV_CP_SCMS_T_ID /* SCMS Content Protection */
typedef UINT16 tBSA_AV_CP_ID;

/* SCMS-T flags */
#define BSA_AV_CP_SCMS_COPY_FREE    0x02
#define BSA_AV_CP_SCMS_COPY_ONCE    0x01
#define BSA_AV_CP_SCMS_COPY_NEVER   0x00
typedef UINT8 tBSA_AV_CP_SCMS;

typedef UINT8 tBSA_AV_HNDL;

/* operation id list for BSA_AvRemoteCmd */
#define BSA_AV_RC_SELECT      AVRC_ID_SELECT      /* select */
#define BSA_AV_RC_UP          AVRC_ID_UP          /* up */
#define BSA_AV_RC_DOWN        AVRC_ID_DOWN        /* down */
#define BSA_AV_RC_LEFT        AVRC_ID_LEFT        /* left */
#define BSA_AV_RC_RIGHT       AVRC_ID_RIGHT       /* right */
#define BSA_AV_RC_RIGHT_UP    AVRC_ID_RIGHT_UP    /* right-up */
#define BSA_AV_RC_RIGHT_DOWN  AVRC_ID_RIGHT_DOWN  /* right-down */
#define BSA_AV_RC_LEFT_UP     AVRC_ID_LEFT_UP     /* left-up */
#define BSA_AV_RC_LEFT_DOWN   AVRC_ID_LEFT_DOWN   /* left-down */
#define BSA_AV_RC_ROOT_MENU   AVRC_ID_ROOT_MENU   /* root menu */
#define BSA_AV_RC_SETUP_MENU  AVRC_ID_SETUP_MENU  /* setup menu */
#define BSA_AV_RC_CONT_MENU   AVRC_ID_CONT_MENU   /* contents menu */
#define BSA_AV_RC_FAV_MENU    AVRC_ID_FAV_MENU    /* favorite menu */
#define BSA_AV_RC_EXIT        AVRC_ID_EXIT        /* exit */
#define BSA_AV_RC_0           AVRC_ID_0           /* 0 */
#define BSA_AV_RC_1           AVRC_ID_1           /* 1 */
#define BSA_AV_RC_2           AVRC_ID_2           /* 2 */
#define BSA_AV_RC_3           AVRC_ID_3           /* 3 */
#define BSA_AV_RC_4           AVRC_ID_4           /* 4 */
#define BSA_AV_RC_5           AVRC_ID_5           /* 5 */
#define BSA_AV_RC_6           AVRC_ID_6           /* 6 */
#define BSA_AV_RC_7           AVRC_ID_7           /* 7 */
#define BSA_AV_RC_8           AVRC_ID_8           /* 8 */
#define BSA_AV_RC_9           AVRC_ID_9           /* 9 */
#define BSA_AV_RC_DOT         AVRC_ID_DOT         /* dot */
#define BSA_AV_RC_ENTER       AVRC_ID_ENTER       /* enter */
#define BSA_AV_RC_CLEAR       AVRC_ID_CLEAR       /* clear */
#define BSA_AV_RC_CHAN_UP     AVRC_ID_CHAN_UP     /* channel up */
#define BSA_AV_RC_CHAN_DOWN   AVRC_ID_CHAN_DOWN   /* channel down */
#define BSA_AV_RC_PREV_CHAN   AVRC_ID_PREV_CHAN   /* previous channel */
#define BSA_AV_RC_SOUND_SEL   AVRC_ID_SOUND_SEL   /* sound select */
#define BSA_AV_RC_INPUT_SEL   AVRC_ID_INPUT_SEL   /* input select */
#define BSA_AV_RC_DISP_INFO   AVRC_ID_DISP_INFO   /* display information */
#define BSA_AV_RC_HELP        AVRC_ID_HELP        /* help */
#define BSA_AV_RC_PAGE_UP     AVRC_ID_PAGE_UP     /* page up */
#define BSA_AV_RC_PAGE_DOWN   AVRC_ID_PAGE_DOWN   /* page down */
#define BSA_AV_RC_POWER       AVRC_ID_POWER       /* power */
#define BSA_AV_RC_VOL_UP      AVRC_ID_VOL_UP      /* volume up */
#define BSA_AV_RC_VOL_DOWN    AVRC_ID_VOL_DOWN    /* volume down */
#define BSA_AV_RC_MUTE        AVRC_ID_MUTE        /* mute */
#define BSA_AV_RC_PLAY        AVRC_ID_PLAY        /* play */
#define BSA_AV_RC_STOP        AVRC_ID_STOP        /* stop */
#define BSA_AV_RC_PAUSE       AVRC_ID_PAUSE       /* pause */
#define BSA_AV_RC_RECORD      AVRC_ID_RECORD      /* record */
#define BSA_AV_RC_REWIND      AVRC_ID_REWIND      /* rewind */
#define BSA_AV_RC_FAST_FOR    AVRC_ID_FAST_FOR    /* fast forward */
#define BSA_AV_RC_EJECT       AVRC_ID_EJECT       /* eject */
#define BSA_AV_RC_FORWARD     AVRC_ID_FORWARD     /* forward */
#define BSA_AV_RC_BACKWARD    AVRC_ID_BACKWARD    /* backward */
#define BSA_AV_RC_ANGLE       AVRC_ID_ANGLE       /* angle */
#define BSA_AV_RC_SUBPICT     AVRC_ID_SUBPICT     /* subpicture */
#define BSA_AV_RC_F1          AVRC_ID_F1          /* F1 */
#define BSA_AV_RC_F2          AVRC_ID_F2          /* F2 */
#define BSA_AV_RC_F3          AVRC_ID_F3          /* F3 */
#define BSA_AV_RC_F4          AVRC_ID_F4          /* F4 */
#define BSA_AV_RC_F5          AVRC_ID_F5          /* F5 */
#define BSA_AV_RC_VENDOR      AVRC_ID_VENDOR      /* vendor unique */

/* Define the PDUs carried in the vendor dependent data */
#define BSA_AV_RC_VD_GET_CAPABILITIES               AVRC_PDU_GET_CAPABILITIES
#define BSA_AV_RC_VD_LIST_PLAYER_APP_ATTR           AVRC_PDU_LIST_PLAYER_APP_ATTR
#define BSA_AV_RC_VD_LIST_PLAYER_APP_VALUES         AVRC_PDU_LIST_PLAYER_APP_VALUES
#define BSA_AV_RC_VD_GET_CUR_PLAYER_APP_VALUE       AVRC_PDU_GET_CUR_PLAYER_APP_VALUE
#define BSA_AV_RC_VD_SET_PLAYER_APP_VALUE           AVRC_PDU_SET_PLAYER_APP_VALUE
#define BSA_AV_RC_VD_GET_PLAYER_APP_ATTR_TEXT       AVRC_PDU_GET_PLAYER_APP_ATTR_TEXT
#define BSA_AV_RC_VD_GET_PLAYER_APP_VALUE_TEXT      AVRC_PDU_GET_PLAYER_APP_VALUE_TEXT
#define BSA_AV_RC_VD_INFORM_DISPLAY_CHARSET         AVRC_PDU_INFORM_DISPLAY_CHARSET
#define BSA_AV_RC_VD_INFORM_BATTERY_STAT_OF_CT      AVRC_PDU_INFORM_BATTERY_STAT_OF_CT
#define BSA_AV_RC_VD_GET_ELEMENT_ATTR               AVRC_PDU_GET_ELEMENT_ATTR
#define BSA_AV_RC_VD_GET_PLAY_STATUS                AVRC_PDU_GET_PLAY_STATUS
#define BSA_AV_RC_VD_REGISTER_NOTIFICATION          AVRC_PDU_REGISTER_NOTIFICATION
#define BSA_AV_RC_VD_REQUEST_CONTINUATION_RSP       AVRC_PDU_REQUEST_CONTINUATION_RSP
#define BSA_AV_RC_VD_ABORT_CONTINUATION_RSP         AVRC_PDU_ABORT_CONTINUATION_RSP
/* added in 1.4 */
#define BSA_AV_RC_VD_SET_ABSOLUTE_VOLUME            AVRC_PDU_SET_ABSOLUTE_VOLUME
#define BSA_AV_RC_VD_SET_ADDRESSED_PLAYER           AVRC_PDU_SET_ADDRESSED_PLAYER
#define BSA_AV_RC_VD_SET_BROWSED_PLAYER             AVRC_PDU_SET_BROWSED_PLAYER
#define BSA_AV_RC_VD_GET_FOLDER_ITEMS               AVRC_PDU_GET_FOLDER_ITEMS
#define BSA_AV_RC_VD_CHANGE_PATH                    AVRC_PDU_CHANGE_PATH
#define BSA_AV_RC_VD_GET_ITEM_ATTRIBUTES            AVRC_PDU_GET_ITEM_ATTRIBUTES
#define BSA_AV_RC_VD_PLAY_ITEM                      AVRC_PDU_PLAY_ITEM
#define BSA_AV_RC_VD_SEARCH                         AVRC_PDU_SEARCH
#define BSA_AV_RC_VD_ADD_TO_NOW_PLAYING             AVRC_PDU_ADD_TO_NOW_PLAYING
#define BSA_AV_RC_VD_GENERAL_REJECT                 AVRC_PDU_GENERAL_REJECT
typedef UINT8 tBSA_AV_RC;

/* state flag for pass through command */
#define BSA_AV_STATE_PRESS      0    /* key pressed */
#define BSA_AV_STATE_RELEASE    1  /* key released */
typedef UINT8 tBSA_AV_STATE;

typedef UINT8 tBSA_AV_RC_HNDL;

/* command codes for BSA_AvVendorCmd */
#define BSA_AV_CMD_CTRL         0
#define BSA_AV_CMD_STATUS       1
#define BSA_AV_CMD_SPEC_INQ     2
#define BSA_AV_CMD_NOTIF        3
#define BSA_AV_CMD_GEN_INQ      4
typedef UINT8 tBSA_AV_CMD;

/* AV callback events */
#define BSA_AV_OPEN_EVT         0       /* connection opened */
#define BSA_AV_CLOSE_EVT        1       /* connection closed */
#define BSA_AV_START_EVT        2       /* stream data transfer started */
#define BSA_AV_STOP_EVT         3       /* stream data transfer stopped */
#define BSA_AV_RC_OPEN_EVT      4       /* remote control channel open */
#define BSA_AV_RC_CLOSE_EVT     5       /* remote control channel closed */
#define BSA_AV_REMOTE_CMD_EVT   6       /* remote control command */
#define BSA_AV_REMOTE_RSP_EVT   7       /* remote control response */
#define BSA_AV_VENDOR_CMD_EVT   8       /* vendor specific command */
#define BSA_AV_VENDOR_RSP_EVT   9       /* vendor specific response */
#define BSA_AV_META_MSG_EVT     10      /* metadata messages */
#define BSA_AV_PENDING_EVT      16      /* incoming connection pending:
                                         * signal channel is open and stream is not open
                                         * after BTA_AV_SIG_TIME_VAL ms */
#define BSA_AV_FEAT_EVT         19      /* Peer feature event */
#define BSA_AV_DELAY_RPT_EVT    20      /* Delay report event */
typedef UINT8 tBSA_AV_EVT;

#define BSA_AV_FEEDING_ASYNCHRONOUS 0   /* asynchronous feeding, use tx av timer */
#define BSA_AV_FEEDING_SYNCHRONOUS  1   /* synchronous feeding, no av tx timer */

#define BSA_AV_MAX_SYNCHRONOUS_LATENCY 80 /* max latency in ms for BSA_AV_FEEDING_SYNCHRONOUS */
#define BSA_AV_MIN_SYNCHRONOUS_LATENCY 4 /* min latency in ms for BSA_AV_FEEDING_SYNCHRONOUS */
typedef UINT8 tBSA_AV_FEEDING_MODE;

#define BSA_AV_CHANNEL_MODE_MONO    A2D_SBC_IE_CH_MD_MONO
#define BSA_AV_CHANNEL_MODE_STEREO  A2D_SBC_IE_CH_MD_STEREO
#define BSA_AV_CHANNEL_MODE_JOINT   A2D_SBC_IE_CH_MD_JOINT
#define BSA_AV_CHANNEL_MODE_DUAL    A2D_SBC_IE_CH_MD_DUAL
typedef UINT8 tBSA_AV_CHANNEL_MODE;


/* Maximum busy level */
#define BSA_MEDIA_BUSY_LEVEL_MAX 5

/* Define the default rates associated with different busy level */
#ifndef BSA_AV_SBC_RATE_LEVEL_0
#define BSA_AV_SBC_RATE_LEVEL_0 250
#endif
#ifndef BSA_AV_SBC_RATE_LEVEL_1
#define BSA_AV_SBC_RATE_LEVEL_1 250
#endif
#ifndef BSA_AV_SBC_RATE_LEVEL_2
#define BSA_AV_SBC_RATE_LEVEL_2 220
#endif
#ifndef BSA_AV_SBC_RATE_LEVEL_3
#define BSA_AV_SBC_RATE_LEVEL_3 180
#endif
#ifndef BSA_AV_SBC_RATE_LEVEL_4
#define BSA_AV_SBC_RATE_LEVEL_4 180
#endif
#ifndef BSA_AV_SBC_RATE_LEVEL_5
#define BSA_AV_SBC_RATE_LEVEL_5 100
#endif

#ifndef BSA_AV_SEC_RATE_LEVEL_0
#define BSA_AV_SEC_RATE_LEVEL_0     348
#endif
#ifndef BSA_AV_SEC_RATE_LEVEL_1
#define BSA_AV_SEC_RATE_LEVEL_1     300
#endif
#ifndef BSA_AV_SEC_RATE_LEVEL_2
#define BSA_AV_SEC_RATE_LEVEL_2     250
#endif
#ifndef BSA_AV_SEC_RATE_LEVEL_3
#define BSA_AV_SEC_RATE_LEVEL_3     200
#endif
#ifndef BSA_AV_SEC_RATE_LEVEL_4
#define BSA_AV_SEC_RATE_LEVEL_4     150
#endif
#ifndef BSA_AV_SEC_RATE_LEVEL_5
#define BSA_AV_SEC_RATE_LEVEL_5     100
#endif


/* Define the Media Attribute IDs
*/
#define BSA_AVRC_MEDIA_ATTR_ID_TITLE                 AVRC_MEDIA_ATTR_ID_TITLE
#define BSA_AVRC_MEDIA_ATTR_ID_ARTIST                AVRC_MEDIA_ATTR_ID_ARTIST
#define BSA_AVRC_MEDIA_ATTR_ID_ALBUM                 AVRC_MEDIA_ATTR_ID_ALBUM
#define BSA_AVRC_MEDIA_ATTR_ID_TRACK_NUM             AVRC_MEDIA_ATTR_ID_TRACK_NUM
#define BSA_AVRC_MEDIA_ATTR_ID_NUM_TRACKS            AVRC_MEDIA_ATTR_ID_NUM_TRACKS
#define BSA_AVRC_MEDIA_ATTR_ID_GENRE                 AVRC_MEDIA_ATTR_ID_GENRE
#define BSA_AVRC_MEDIA_ATTR_ID_PLAYING_TIME          AVRC_MEDIA_ATTR_ID_PLAYING_TIME        /* in milliseconds */
#define BSA_AVRC_MAX_ATTR_COUNT                      AVRC_MAX_NUM_MEDIA_ATTR_ID             /* 7 */


/* Define the possible values of play state
*/

#define BSA_AVRC_PLAYSTATE_STOPPED                  AVRC_PLAYSTATE_STOPPED      /* Stopped */
#define BSA_AVRC_PLAYSTATE_PLAYING                  AVRC_PLAYSTATE_PLAYING      /* Playing */
#define BSA_AVRC_PLAYSTATE_PAUSED                   AVRC_PLAYSTATE_PAUSED       /* Paused  */
#define BSA_AVRC_PLAYSTATE_FWD_SEEK                 AVRC_PLAYSTATE_FWD_SEEK     /* Fwd Seek */
#define BSA_AVRC_PLAYSTATE_REV_SEEK                 AVRC_PLAYSTATE_REV_SEEK     /* Rev Seek */
#define BSA_AVRC_PLAYSTATE_ERROR                    AVRC_PLAYSTATE_ERROR        /* 0xFF: Error*/

#define tBSA_CHG_PATH_CMD           tAVRC_CHG_PATH_CMD
#define tBSA_PLAY_ITEM_CMD          tAVRC_PLAY_ITEM_CMD
#define tBSA_UID                    tAVRC_UID
#define tBSA_FULL_NAME              tAVRC_FULL_NAME
#define tBSA_STS                    tAVRC_STS
#define tBSA_ITEM_PLAYER            tAVRC_ITEM_PLAYER
#define tBSA_ITEM_FOLDER            tAVRC_ITEM_FOLDER
#define tBSA_ITEM_MEDIA             tAVRC_ITEM_MEDIA
#define tBSA_NOTIF_RSP_PARAM        tAVRC_NOTIF_RSP_PARAM
#define tBSA_CHG_PATH_RSP           tAVRC_CHG_PATH_RSP
#define tBSA_RSP                    tAVRC_RSP
#define tBSA_ATTR_ENTRY             tAVRC_ATTR_ENTRY

/* registered for notifications
*/
#define BSA_AVRC_EVT_PLAY_STATUS_CHANGE     AVRC_EVT_PLAY_STATUS_CHANGE
#define BSA_AVRC_EVT_TRACK_CHANGE           AVRC_EVT_TRACK_CHANGE
#define BSA_AVRC_EVT_TRACK_REACHED_END      AVRC_EVT_TRACK_REACHED_END
#define BSA_AVRC_EVT_TRACK_REACHED_START    AVRC_EVT_TRACK_REACHED_START
#define BSA_AVRC_EVT_PLAY_POS_CHANGED       AVRC_EVT_PLAY_POS_CHANGED
#define BSA_AVRC_EVT_BATTERY_STATUS_CHANGE  AVRC_EVT_BATTERY_STATUS_CHANGE
#define BSA_AVRC_EVT_SYSTEM_STATUS_CHANGE   AVRC_EVT_SYSTEM_STATUS_CHANGE
#define BSA_AVRC_EVT_APP_SETTING_CHANGE     AVRC_EVT_APP_SETTING_CHANGE
#define BSA_AVRC_EVT_NOW_PLAYING_CHANGE     AVRC_EVT_NOW_PLAYING_CHANGE
#define BSA_AVRC_EVT_AVAL_PLAYERS_CHANGE    AVRC_EVT_AVAL_PLAYERS_CHANGE
#define BSA_AVRC_EVT_ADDR_PLAYER_CHANGE     AVRC_EVT_ADDR_PLAYER_CHANGE
#define BSA_AVRC_EVT_UIDS_CHANGE            AVRC_EVT_UIDS_CHANGE
#define BSA_AVRC_EVT_VOLUME_CHANGE          AVRC_EVT_VOLUME_CHANGE
/*
 * Structure used to configure the AV codec capabilities/config
 */
typedef struct
{
    tBSA_AV_CODEC_ID id;            /* Codec ID (in terms of BSA) */
    UINT8 info[AVDT_CODEC_SIZE];    /* Codec info (can be config or capabilities) */
} tBSA_AV_CODEC_INFO;

/*
 * Structure used to configure the AV media feeding
 */
typedef struct
{
    UINT16 sampling_freq;   /* 44100, 48000 etc */
    UINT16 num_channel;     /* 1 for mono or 2 stereo */
    UINT8  bit_per_sample;  /* Number of bits per sample (8, 16) */
} tBSA_AV_MEDIA_FEED_CFG_PCM;

typedef struct
{
    UINT16 sampling_freq;           /* 44100, 48000 etc */
    tBSA_AV_CHANNEL_MODE ch_mode;   /* Channel mode */
} tBSA_AV_MEDIA_FEED_CFG_APTX;

typedef struct
{
    UINT16 sampling_freq;           /* 44100, 48000 etc */
    tBSA_AV_CHANNEL_MODE ch_mode;   /* Channel mode */
} tBSA_AV_MEDIA_FEED_CFG_SEC;

typedef union
{
    tBSA_AV_MEDIA_FEED_CFG_PCM pcm;     /* raw PCM feeding format */
    tBSA_AV_MEDIA_FEED_CFG_APTX aptx;   /* apt-X feeding format */
    tBSA_AV_MEDIA_FEED_CFG_SEC sec;     /* SEC feeding format */
    tA2D_SBC_CIE sbc;                   /* SBC feeding format */
    tA2D_M12_CIE mp3;                   /* MPEG-1, 2 Audio Codec Information Element */
    tA2D_M24_CIE aac;                   /* MPEG-2, 4 AAC Codec Information Element */
}tBSA_AV_MEDIA_FEED_CFG;

typedef struct
{
    tBSA_AV_CODEC_ID format;        /* Media codec identifier */
    tBSA_AV_MEDIA_FEED_CFG cfg;     /* Media codec configuration */
} tBSA_AV_MEDIA_FEEDINGS;

/*
 * Structures used for parameters (transport)
 */
#define BSA_AV_META_INFO_LEN_MIN    32
#define BSA_AV_META_INFO_LEN_MAX    64
#define BSA_AV_SERVICE_NAME_LEN_MAX 128
#define BSA_AV_VENDOR_SIZE_MAX      BTA_MAX_VENDOR_DEPENDENT_DATA_LEN


/* data associated with BSA_AV_OPEN_EVT */
typedef struct
{
    tBSA_STATUS status;
    tBSA_AV_CHNL channel;
    tBSA_AV_HNDL handle;
    BD_ADDR bd_addr;
    BOOLEAN cp_supported;
    BOOLEAN aptx_supported; /* apt-X supported */
    BOOLEAN aptx_cp_supported; /* apt-X supported */
    BOOLEAN sec_supported; /* SEC supported */
} tBSA_AV_OPEN_MSG;

typedef struct
{
    BD_ADDR         bd_addr;
} tBSA_AV_PEND_MSG;

/* data associated with BSA_AV_CLOSE_EVT */
typedef struct
{
    tBSA_STATUS status;
    tBSA_AV_CHNL channel;
    tBSA_AV_HNDL handle;
} tBSA_AV_CLOSE_MSG;

/* data associated with BSA_AV_DELAY_RPT_EVT */
typedef struct
{
    tBSA_AV_CHNL channel;
    tBSA_AV_HNDL handle;
    UINT16 delay;
} tBSA_AV_DELAY_MSG;

/* data associated with BSA_AV_START_EVT */
typedef struct
{
    tBSA_STATUS status;
    tBSA_AV_CHNL channel;
    tUIPC_CH_ID uipc_channel;
    tBSA_AV_MEDIA_FEEDINGS media_feeding;
    BOOLEAN cp_enabled;
    UINT8 cp_flag;
} tBSA_AV_START_MSG;

/* data associated with BSA_AV_STOP_EVT */
typedef struct
{
    tBSA_STATUS status;
    tBSA_AV_CHNL channel;
    BOOLEAN pause;
    tUIPC_CH_ID uipc_channel;
} tBSA_AV_STOP_MSG;

/* data associated with BSA_AV_RC_OPEN_EVT */
typedef struct
{
    tBSA_STATUS status;
    tBSA_AV_RC_HNDL rc_handle;
    tBSA_AV_FEAT peer_features;
    BD_ADDR peer_addr;
} tBSA_AV_RC_OPEN_MSG;

/* data associated with BSA_AV_RC_CLOSE_EVT */
typedef struct
{
    tBSA_AV_RC_HNDL rc_handle;
    BD_ADDR peer_addr;
} tBSA_AV_RC_CLOSE_MSG;

/* data associated with BSA_AV_REMOTE_CMD_EVT */
typedef struct
{
    tBSA_AV_RC_HNDL rc_handle;
    tBSA_AV_RC rc_id;
    tBSA_AV_STATE key_state;
} tBSA_AV_REMOTE_CMD_MSG;

/* data associated with BSA_AV_REMOTE_RSP_EVT */
typedef struct
{
    tBSA_AV_RC_HNDL rc_handle;
    tBSA_AV_RC rc_id;
    tBSA_AV_STATE key_state;
} tBSA_AV_REMOTE_RSP_MSG;

/* data associated with BSA_AV_VENDOR_CMD_EVT */
typedef struct
{
    UINT8           rc_handle;
    UINT16          len;            /* Max vendor dependent message is BSA_AV_VENDOR_SIZE_MAX */
    UINT8           label;
    tBSA_AV_CMD     code;
    UINT32          company_id;
    UINT8           data[BSA_AV_VENDOR_SIZE_MAX];
} tBSA_AV_VENDOR_CMD_MSG;

typedef tBSA_AV_VENDOR_CMD_MSG tBSA_AV_VENDOR_RSP_MSG;

/* GetElemAttrs */
typedef struct
{
    UINT8       pdu;
    UINT8       opcode;                         /* Op Code (assigned by AVRC_BldCommand according to pdu) */
    UINT8       num_attr;
    UINT8       label;
    UINT32      attrs[BSA_AVRC_MAX_ATTR_COUNT];
} tBSA_AV_META_GET_ELEM_ATTRS_CMD;

/* GetFolderItems */
typedef struct
{
    UINT8       scope;
    UINT32      start_item;
    UINT32      end_item;
    UINT8       attr_count;
} tBSA_AV_META_GET_FOLDER_ITEMS_CMD;

/* RegNotify */
typedef struct
{
    UINT8       event_id;
    UINT32      param;
} tBSA_AV_REG_NOTIF_CMD;

/* GetItemAttr */
typedef struct
{
    UINT8       pdu;
    UINT8       opcode;                         /* Op Code (assigned by AVRC_BldCommand according to pdu) */
    UINT8       scope;
    tBSA_UID   uid;
    UINT16      uid_counter;
    UINT8       num_attr;
    UINT8       label;
    UINT32      attrs[BSA_AVRC_MAX_ATTR_COUNT];
} tBSA_AV_META_GET_ITEM_ATTRS_CMD;

/* data associated with BSA_AV_META_MSG_EVT */
typedef struct
{
    tBSA_AV_RC_HNDL rc_handle;
    UINT8           opcode;                         /* Op Code (assigned by AVRC_BldCommand according to pdu) */
    UINT8           pdu;
    UINT8           label;
    UINT32          company_id;
    UINT16          player_id;

    union{
    tBSA_AV_META_GET_ELEM_ATTRS_CMD     get_elem_attrs;
    tBSA_AV_META_GET_FOLDER_ITEMS_CMD   get_folder_items;
    tBSA_AV_REG_NOTIF_CMD               reg_notif;
    tBSA_CHG_PATH_CMD                  change_path;
    tBSA_PLAY_ITEM_CMD                 play_item;
    tBSA_AV_META_GET_ITEM_ATTRS_CMD     get_item_attrs;
    }param;

} tBSA_AV_META_MSG_MSG;

typedef struct
{
    UINT8              rc_handle;
    tBTA_AV_FEAT       peer_features;
    tBTA_AV_RC_INFO    tg;                 /* Peer TG role info */
    tBTA_AV_RC_INFO    ct;                 /* Peer CT role info */
} tBSA_AV_RC_FEAT_MSG;

/* union of data associated with AV callback */
typedef union
{
    tBSA_AV_OPEN_MSG open;
    tBSA_AV_CLOSE_MSG close;
    tBSA_AV_DELAY_MSG delay;
    tBSA_AV_START_MSG start;
    tBSA_AV_STOP_MSG stop;
    tBSA_AV_RC_OPEN_MSG rc_open;
    tBSA_AV_RC_CLOSE_MSG rc_close;
    tBSA_AV_REMOTE_CMD_MSG remote_cmd;
    tBSA_AV_REMOTE_RSP_MSG remote_rsp;
    tBSA_AV_VENDOR_CMD_MSG vendor_cmd;
    tBSA_AV_VENDOR_RSP_MSG vendor_rsp;
    tBSA_AV_PEND_MSG       pend;
    tBSA_AV_META_MSG_MSG meta_msg;
    tBSA_AV_RC_FEAT_MSG rc_feat;
} tBSA_AV_MSG;

/* AV callback */
typedef void (tBSA_AV_CBACK)(tBSA_AV_EVT event, tBSA_AV_MSG *p_data);

/* data associated with BSA_AvEnable */
typedef struct
{
    tBSA_SEC_AUTH sec_mask;         /* Security type */
    tBSA_AV_FEAT features;          /* Supported AV features */
    tBSA_AV_CODEC_INFO aptx_caps;   /* apt-X capabilities (if id=apt-X apt-X support enabled) */
    tBSA_AV_CODEC_INFO sec_caps;    /* SEC capabilities (if id=SEC SEC support enabled) */
    tBSA_AV_CBACK *p_cback;         /* Callback */
} tBSA_AV_ENABLE;

/* data associated with BSA_AvDisable */
typedef struct
{
    UINT16 dummy;
} tBSA_AV_DISABLE;

/* data associated with BSA_AvRegister */
typedef struct
{
    tBSA_AV_CHNL channel;           /* in: AV channel */
    UINT8 lt_addr;                  /* in: lt_addr (for Broadcast channels) */
    tBSA_AV_HNDL handle;            /* out: AV handle */
    tUIPC_CH_ID uipc_channel;       /* out: uipc channel */
    char service_name[BSA_AV_SERVICE_NAME_LEN_MAX];
} tBSA_AV_REGISTER;

/* data associated with BSA_AvDeregister */
typedef struct
{
    UINT8 handle;       /* AV handle */
} tBSA_AV_DEREGISTER;

/* data associated with BSA_AvOpen */
typedef struct
{
    BD_ADDR bd_addr;        /* Address of the device to connect to */
    tBSA_AV_HNDL handle;    /* AV handle to use to connect */
    BOOLEAN use_rc;         /* Indicate if RC must also be opened */
    tBSA_SEC_AUTH sec_mask; /* Security type */
} tBSA_AV_OPEN;

/* data associated with BSA_AvClose */
typedef struct
{
    tBSA_AV_HNDL handle;
} tBSA_AV_CLOSE;

/* data associated with BSA_AvStart */
typedef struct
{
    tBSA_AV_MEDIA_FEEDINGS media_feeding;   /* feeding formating info */
    tBSA_AV_FEEDING_MODE feeding_mode;      /* synchronous or asynchronous */
    UINT32 latency;                         /* latency in ms, only apply to BSA_AV_FEEDING_SYNCHRONOUS mode */
    tBSA_AV_CP_ID cp_id;                    /* Content Protection Id (None/SCMS) */
    tBSA_AV_CP_SCMS scmst_flag;             /* SCMS-T flag to use for the stream */
    tUIPC_CH_ID uipc_channel;               /* uipc channel */
} tBSA_AV_START;

/* data associated with BSA_AvStop */
typedef struct
{
    BOOLEAN pause;
    tUIPC_CH_ID uipc_channel;               /* uipc channel */
} tBSA_AV_STOP;

/* data associated with BSA_AvRemoteCmd */
typedef struct
{
    tBSA_AV_RC_HNDL rc_handle;
    UINT8 label;
    tBSA_AV_RC rc_id;
    tBSA_AV_STATE key_state;
} tBSA_AV_REM_CMD;

/* data associated with BSA_AvVendorCmd */

typedef struct
{
    tBSA_AV_RC_HNDL rc_handle;
    UINT8 ctype;
    UINT8 label;
    UINT8 data[BSA_AV_VENDOR_SIZE_MAX];
    UINT16 length;
} tBSA_AV_VEN_CMD;

typedef tBSA_AV_VEN_CMD tBSA_AV_VEN_RSP;


typedef struct
{
    UINT32              attr_id;
    /* Use BSA_AVRC_MEDIA_ATTR_ID_TITLE, BSA_AVRC_MEDIA_ATTR_ID_ARTIST, BSA_AVRC_MEDIA_ATTR_ID_ALBUM,
    BSA_AVRC_MEDIA_ATTR_ID_TRACK_NUM, BSA_AVRC_MEDIA_ATTR_ID_NUM_TRACKS, BSA_AVRC_MEDIA_ATTR_ID_GENRE,
    BSA_AVRC_MEDIA_ATTR_ID_PLAYING_TIME */
    tBSA_FULL_NAME     name;           /* The attribute value, value length and character set id. */
    UINT16              charset_id;
    UINT8               str[BSA_AV_META_INFO_LEN_MAX];
} tBSA_AV_ATTR_ENTRY;



typedef struct {
    UINT8       opcode;
    UINT8       num_attrs;
    tBSA_AV_ATTR_ENTRY attrs[BSA_AVRC_MAX_ATTR_COUNT];
} tBSA_AV_META_GET_ELEM_ATTR_RSP;


typedef struct {
    UINT32  song_len;
    UINT32  song_pos;
    UINT8   play_status;
} tBSA_AV_META_GET_PLAYSTATUS_RSP;


#define BSA_AV_ITEM_NAME_LEN_MAX    64
#define BSA_AVRC_MAX_ITEM_COUNT     5

typedef struct
{
    UINT8                   item_type;  /* AVRC_ITEM_PLAYER, AVRC_ITEM_FOLDER, or AVRC_ITEM_MEDIA */
    union
    {
        tBSA_ITEM_PLAYER   player;     /* The properties of a media player item.*/
        tBSA_ITEM_FOLDER   folder;     /* The properties of a folder item.*/
        tBSA_ITEM_MEDIA    media;      /* The properties of a media item.*/
    } u;
    UINT8                   item_str[BSA_AV_ITEM_NAME_LEN_MAX];
} tBSA_AV_ITEM;

typedef struct
{
    UINT8               opcode;
    tBSA_STS           status;
} tBSA_AV_META_SET_ADDR_PLAYER_RSP;

typedef struct
{
    UINT8               opcode;
    tBSA_STS           status;
    UINT16              scope;
    UINT16              uid_counter;
    UINT16              num_items;
    tBSA_AV_ITEM        item_list[BSA_AVRC_MAX_ITEM_COUNT];
} tBSA_AV_META_GET_ITEMS_RSP;

typedef struct
{
    UINT8                   opcode;
    tBSA_STS               status;
    UINT8                   event_id;
    tBSA_NOTIF_RSP_PARAM   param;
    tBTA_AV_CODE            code;
} tBSA_AV_META_NOTIF_RSP;

typedef struct
{
    UINT8               opcode;
    tBSA_STS           status;
    UINT16              uid_counter;
    UINT32              num_items;
    UINT16              charset_id;
    UINT8               folder_depth;
    UINT8               folder_name_size;
    UINT8               folder_name_str[BSA_AV_ITEM_NAME_LEN_MAX];
} tBSA_AV_META_SET_BROWSED_PLAYER_RSP;

#define BSA_AVRC_PDU_GET_ELEMENT_ATTR               AVRC_PDU_GET_ELEMENT_ATTR    /* Song Info */
#define BSA_AVRC_PDU_GET_PLAY_STATUS                AVRC_PDU_GET_PLAY_STATUS     /* Play Status */
#define BSA_AVRC_PDU_SET_ADDRESSED_PLAYER           AVRC_PDU_SET_ADDRESSED_PLAYER     /* Addressed Player */
#define BSA_AVRC_PDU_GET_FOLDER_ITEMS               AVRC_PDU_GET_FOLDER_ITEMS     /* Get Folder Items */
#define BSA_AVRC_PDU_REGISTER_NOTIFICATION          AVRC_PDU_REGISTER_NOTIFICATION     /* reg notifications */
#define BSA_AVRC_PDU_SET_BROWSED_PLAYER             AVRC_PDU_SET_BROWSED_PLAYER     /* BROWSED Player */
#define BSA_AVRC_PDU_CHANGE_PATH                    AVRC_PDU_CHANGE_PATH     /* change path */
#define BSA_AVRC_PDU_GET_ITEM_ATTRIBUTES            AVRC_PDU_GET_ITEM_ATTRIBUTES     /* get item attr */
#define BSA_AVRC_PDU_PLAY_ITEM                      AVRC_PDU_PLAY_ITEM     /* play item */


/* data associated with BSA_AvMetaRsp */
typedef struct
{
    tBSA_AV_RC_HNDL     rc_handle;
    UINT8               pdu;             /* Song info or Play Status supported */
    UINT8               label;

    union{
    tBSA_AV_META_GET_ELEM_ATTR_RSP  get_elem_attrs;
    tBSA_AV_META_GET_PLAYSTATUS_RSP get_play_status;
    tBSA_AV_META_SET_ADDR_PLAYER_RSP addr_player_status;
    tBSA_AV_META_GET_ITEMS_RSP      get_items_status;
    tBSA_AV_META_NOTIF_RSP          notify_status;
    tBSA_AV_META_SET_BROWSED_PLAYER_RSP browsed_player_status;
    tBSA_CHG_PATH_RSP              change_path;
    tBSA_AV_META_GET_ELEM_ATTR_RSP  get_item_attrs;
    tBSA_RSP                       play_item;
    }param;

} tBSA_AV_META_RSP_CMD;


/* data associated with BSA_AvCloseRc */
typedef struct
{
    tBSA_AV_RC_HNDL rc_handle;
} tBSA_AV_CLOSE_RC;

/* data associated with BSA_AvBusyLevel */
 typedef struct
{
    UINT8 level;
} tBSA_AV_BUSY_LEVEL;

/*****************************************************************************
 **  External Function Declarations
 *****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function         BSA_AvEnableInit
 **
 ** Description      Init structure tBSA_AV_ENABLE to be used with BSA_AvEnable
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvEnableInit(tBSA_AV_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function         BSA_AvEnable
 **
 ** Description      Enable the advanced audio/video service. When the enable
 **                  operation is complete the callback function will be
 **                  called with a BSA_AV_ENABLE_EVT. This function must
 **                  be called before other function in the AV API are
 **                  called.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvEnable(tBSA_AV_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function         BSA_AvDisableInit
 **
 ** Description      Init structure tBSA_AV_DISABLE to be used with BSA_AvDisable
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvDisableInit(tBSA_AV_DISABLE *p_disable);

/*******************************************************************************
 **
 ** Function         BSA_AvDisable
 **
 ** Description      Disable the advanced audio/video service.
 **
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvDisable(tBSA_AV_DISABLE *p_disable);

/*******************************************************************************
 **
 ** Function         BSA_AvRegisterInit
 **
 ** Description      Init structure tBSA_AV_DEREGISTER to be used with BSA_AvRegister
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvRegisterInit(tBSA_AV_REGISTER *p_register);

/*******************************************************************************
 **
 ** Function         BSA_AvRegister
 **
 ** Description      Register the audio or video service to stack. When the
 **                  operation is complete the callback function will be
 **                  called with a BSA_AV_REGISTER_EVT. This function must
 **                  be called before AVDT stream is open.
 **
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvRegister(tBSA_AV_REGISTER *p_reg);

/*******************************************************************************
 **
 ** Function         BSA_AvDeregisterInit
 **
 ** Description      Init structure tBSA_AV_DEREGISTER to be used with BSA_AvDeregister
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvDeregisterInit(tBSA_AV_DEREGISTER *p_deregister);

/*******************************************************************************
 **
 ** Function         BSA_AvDeregister
 **
 ** Description      Deregister the audio or video service
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvDeregister(tBSA_AV_DEREGISTER *p_rereg);

/*******************************************************************************
 **
 ** Function         BSA_AvOpenInit
 **
 ** Description      Init structure tBSA_AV_OPEN to be used with BSA_AvOpen
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvOpenInit(tBSA_AV_OPEN * p_open);

/*******************************************************************************
 **
 ** Function         BSA_AvOpen
 **
 ** Description      Opens an advanced audio/video connection to a peer device.
 **                  When connection is open callback function is called
 **                  with a BSA_AV_OPEN_EVT.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvOpen(tBSA_AV_OPEN *p_open);

/*******************************************************************************
 **
 ** Function         BSA_AvCloseInit
 **
 ** Description      Init structure tBSA_AV_CLOSE to be used with BSA_AvClose
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvCloseInit(tBSA_AV_CLOSE *p_close);

/*******************************************************************************
 **
 ** Function         BSA_AvClose
 **
 ** Description      Close the current streams.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvClose(tBSA_AV_CLOSE *p_close);

/*******************************************************************************
 **
 ** Function         BSA_AvStartInit
 **
 ** Description      Init sturcture p_close to be used in BSA_AvClose
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_AvStartInit(tBSA_AV_START * p_start);

/*******************************************************************************
 **
 ** Function         BSA_AvStart
 **
 ** Description      Start audio/video stream data transfer.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_AvStart(tBSA_AV_START *p_start);

/*******************************************************************************
 **
 ** Function         BSA_AvStopInit
 **
 ** Description      Init a structure tBSA_AV_STOP to be used with BSA_AvStopInit
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvStopInit(tBSA_AV_STOP *p_stop);

/*******************************************************************************
 **
 ** Function         BSA_AvStop
 **
 ** Description      Stop audio/video stream data transfer.
 **                  If suspend is TRUE, this function sends AVDT suspend signal
 **                  to the connected peer(s).
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvStop(tBSA_AV_STOP *p_stop);

/*******************************************************************************
 **
 ** Function         BSA_AvRemoteCmdInit
 **
 ** Description      Init a structure tBSA_AV_REM_CMD to be used with BSA_AvRemoteCmd
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvRemoteCmdInit(tBSA_AV_REM_CMD *p_rem_cmd);

/*******************************************************************************
 **
 ** Function         BSA_AvRemoteCmd
 **
 ** Description      Send a remote control command.  This function can only
 **                  be used if AV is enabled with feature BSA_AV_FEAT_RCCT.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvRemoteCmd(tBSA_AV_REM_CMD *p_rem_cmd);

/*******************************************************************************
 **
 ** Function         BSA_AvVendorCmdInit
 **
 ** Description      Init a structure tBSA_AV_VEN_CMD to be used with BSA_AvVendorCmd
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvVendorCmdInit(tBSA_AV_VEN_CMD *p_ven_cmd);

/*******************************************************************************
 **
 ** Function         BSA_AvVendorCmd
 **
 ** Description      Send a vendor control command.  This function can only
 **                  be used if AV is enabled with feature BSA_AV_FEAT_RCCT.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvVendorCmd(tBSA_AV_VEN_CMD *p_ven_cmd);


/*******************************************************************************
 **
 ** Function         BSA_AvVendorRspInit
 **
 ** Description      Init a structure tBSA_AV_VEN_RSP to be used with BSA_AvVendorRsp
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvVendorRspInit(tBSA_AV_VEN_RSP *p_ven_rsp);

/*******************************************************************************
 **
 ** Function         BSA_AvVendorRsp
 **
 ** Description      Send a vendor control command.  This function can only
 **                  be used if AV is enabled with feature BSA_AV_FEAT_RCTG.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvVendorRsp(tBSA_AV_VEN_RSP *p_ven_rsp);

/*******************************************************************************
 **
 ** Function         BSA_AvMetaRspInit
 **
 ** Description      Init a structure tBSA_AV_META_RSP_CMD to be used with BSA_AvMetaRsp
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvMetaRspInit(tBSA_AV_META_RSP_CMD *p_meta_rsp);

/*******************************************************************************
 **
 ** Function         BSA_AvMetaRsp
 **
 ** Description      Send a metador control command.  This function can only
 **                  be used if AV is enabled with feature BSA_AV_FEAT_METADATA.
 **
 **                  BSA_AvMetaRsp currently supports following PDUs
 **                  GetElementAttributes - BSA_AVRC_PDU_GET_ELEMENT_ATTR and
 **                  GetPlayStatus - BSA_AVRC_PDU_GET_PLAY_STATUS
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvMetaRsp(tBSA_AV_META_RSP_CMD *p_meta_rsp);

/*******************************************************************************
 **
 ** Function         BSA_AvCloseRcInit
 **
 ** Description      Init structure tBSA_AV_CLOSE_RC to be used with BSA_AvCloseRc
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvCloseRcInit(tBSA_AV_CLOSE_RC *p_close_rc);

/*******************************************************************************
 **
 ** Function         BSA_AvCloseRc
 **
 ** Description      Close an AVRCP connection
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvCloseRc(tBSA_AV_CLOSE_RC *p_close_rc);

/*******************************************************************************
 **
 ** Function         BSA_AvBusyLevelInit
 **
 ** Description      Init structure tBSA_AV_BUSY_LEVEL
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvBusyLevelInit(tBSA_AV_BUSY_LEVEL *blevel);

/*******************************************************************************
 **
 ** Function         BSA_AvBusyLevel
 **
 ** Description      Change busy level
 **
 ** Returns          BSA_AvBusyLevel
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvBusyLevel(tBSA_AV_BUSY_LEVEL *p_req);

#ifdef __cplusplus
}
#endif

#endif /* BSA_AV_API_H */
