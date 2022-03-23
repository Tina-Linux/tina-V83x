/*****************************************************************************
**
**  Name:           bsa_fm_api.h
**
**  Description:    This is the public interface file for the FM subsystem of
**                  BTA, Broadcom's Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2006, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BSA_FM_API_H
#define BSA_FM_API_H

#include "bta_api.h"
#include "bsa_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* Extra Debug Code */
#ifndef BSA_FM_DEBUG
#define BSA_FM_DEBUG           FALSE
#endif

#ifndef BSA_FM_AF_INCLUDED
#define BSA_FM_AF_INCLUDED      TRUE
#endif

#ifndef BSA_FM_10KHZ_INCLUDED
#define BSA_FM_10KHZ_INCLUDED      TRUE
#endif

/* Scanning step option provided by software algorithm */
#ifndef BSA_FM_SCAN_STEP_ALGO
#define BSA_FM_SCAN_STEP_ALGO           TRUE
#endif

/* noise floor estimation */
#ifndef     BSA_FM_NFE_DEFAILT
#define     BSA_FM_NFE_DEFAILT      93      /* default Noise floor value */
#endif
#ifndef     BSA_FM_NFE_THRESH
#define     BSA_FM_NFE_THRESH       0x32    /* default NFE threshold 53 db */
#endif
#ifndef     BSA_FM_NFE_SNR_STEREO
#define     BSA_FM_NFE_SNR_STEREO   19      /* Stereo audio mode SNR level above NFL */
#endif
#ifndef     BSA_FM_NFE_SNR_MONO
#define     BSA_FM_NFE_SNR_MONO     10      /* Mono audio mode SNR level above NFL */
#endif

/* Max AF number */
#ifndef BSA_FM_AF_MAX_NUM
#define BSA_FM_AF_MAX_NUM   25
#endif

/* preset channel register max size 248 bytes, each channel will take 4/5 bytes depend on search mode
   so max preset channel n has to be 5*n <= 248. set default to be 48 */
#define BSA_FM_MAX_PRESET_STA       48      /* same number defined as I2C_PRESET_STA_MAX */

enum
{
    BSA_FM_ENABLE_EVT,          /* 0 BTA FM is enabled */
    BSA_FM_DISABLE_EVT,         /* 1 BTA FM is disabled */
    BSA_FM_RDS_UPD_EVT,         /* 2 read RDS data event */
    BSA_FM_SEARCH_CMPL_EVT,     /* 3 FM scanning is completed event */
    BSA_FM_TUNE_EVT,            /* 4 FM TUNE event */
    BSA_FM_AUD_MODE_EVT,        /* 5 Set audio mode completed  */
    BSA_FM_MUTE_AUD_EVT,        /* 6 audio mute/unmute event */
    BSA_FM_SCAN_STEP_EVT,       /* 7 config scan step */
    BSA_FM_RDS_MODE_EVT,        /* 8 set RDS mode event */
    BSA_FM_AUD_PATH_EVT,        /* 9 set audio path completed */
    BSA_FM_AUD_DATA_EVT,        /* 10 audio quality live updating call back event */
    BSA_FM_SET_DEEMPH_EVT,      /* 11 configure deempahsis parameter event */
    BSA_FM_SET_REGION_EVT,      /* 12 set band region */
    BSA_FM_NFL_EVT,                /* 13 noise floor estimation event */
    BSA_FM_RDS_TYPE_EVT,        /* 14 Set RDS type event */
    BSA_FM_CFG_BLEND_MUTE_EVT,  /* 15 stereo/mono blend and soft mute configuration event */
    BSA_FM_VOLUME_EVT,          /* 16 volume event */
    BSA_FM_SEARCH_EVT,            /* 17 search event */
    BSA_FM_AF_JMP_EVT           /* 18 AF jump event */
};


typedef UINT8  tBSA_FM_EVT;
typedef UINT16  tBSA_FM_RDS_EVT;

enum
{
    BSA_FM_OK,
    BSA_FM_SCAN_RSSI_LOW,
    BSA_FM_SCAN_FAIL,
    BSA_FM_SCAN_ABORT,
    BSA_FM_SCAN_NO_RES,
    BSA_FM_ERR,
    BSA_FM_UNSPT_ERR,
    BSA_FM_FLAG_TOUT_ERR,
    BSA_FM_FREQ_ERR,
    BSA_FM_VCMD_ERR,
    BSA_FM_BUSY
};
typedef UINT8   tBSA_FM_STATUS;

/* FM band region, bit 0, 1,2 of func_mask */
#ifndef BSA_MAX_REGION_SETTING
#if BSA_FM_10KHZ_INCLUDED == TRUE
#define BSA_MAX_REGION_SETTING  5   /* max region code defined */
#else
#define BSA_MAX_REGION_SETTING  3   /* max region code defined */
#endif
#endif

/* FM function mask */
#define     BSA_FM_REGION_NA    0x00        /* bit0/bit1/bit2: north america */
#define     BSA_FM_REGION_EUR   0x01        /* bit0/bit1/bit2: Europe           */
#define     BSA_FM_REGION_JP    0x02        /* bit0/bit1/bit2: Japan            */
#if BSA_FM_10KHZ_INCLUDED
#define     BSA_FM_REGION_RUS   0x03        /* bit0/bit1/bit2: Russia with extended        */
#define     BSA_FM_REGION_CHN   0x04        /* bit0/bit1/bit2: China: 76.0 ~ 108.0  */
#endif
#define     BSA_FM_RDS_BIT      1<<4        /* bit4: RDS functionality */
#define     BSA_FM_RBDS_BIT     1<<5        /* bit5: RBDS functionality, exclusive with RDS bit */
#define     BSA_FM_AF_BIT       1<<6        /* bit6: AF functionality */
typedef UINT8   tBSA_FM_FUNC_MASK;

/* low 3 bits (bit0, 1, 2)of FUNC mask is region code */
#if BSA_FM_10KHZ_INCLUDED
#define     BSA_FM_REGION_MAX           BSA_FM_REGION_CHN
#define     BSA_FM_REGION_MASK          (BSA_FM_REGION_NA | BSA_FM_REGION_EUR | BSA_FM_REGION_RUS | BSA_FM_REGION_CHN)
#else
#define     BSA_FM_REGION_MAX           BSA_FM_REGION_JP
#define     BSA_FM_REGION_MASK          (BSA_FM_REGION_NA | BSA_FM_REGION_EUR)             /* low 3 bits (bit0, 1)of FUNC mask is region code */
#endif

typedef UINT8  tBSA_FM_REGION_CODE;

#define     BSA_FM_DEEMPHA_50U      0       /* 6th bit in FM_AUDIO_CTRL0 set to 0, Europe default */
#define     BSA_FM_DEEMPHA_75U      1<<6    /* 6th bit in FM_AUDIO_CTRL0 set to 1, US  default */
typedef UINT8 tBSA_FM_DEEMPHA_TIME;

#define BSA_FM_GET_FREQ(x)  ((UINT16)((x * 10) - 64000))

enum
{
    BSA_FM_SCH_RDS_NONE,
    BSA_FM_SCH_RDS_PTY,
    BSA_FM_SCH_RDS_TP
};
typedef UINT8 tBSA_FM_SCH_RDS_TYPE;

typedef struct
{
    tBSA_FM_SCH_RDS_TYPE    cond_type;
    UINT8                   cond_val;
} tBSA_FM_SCH_RDS_COND;

/* FM audio output mode */
enum
{
    BSA_FM_AUTO_MODE,       /* auto blend by default */
    BSA_FM_STEREO_MODE,     /* manual stereo switch */
    BSA_FM_MONO_MODE,       /* manual mono switch */
    BSA_FM_SWITCH_MODE      /* auto stereo, and switch activated */
};
typedef UINT8  tBSA_FM_AUDIO_MODE;

/* FM audio output quality */
#define BSA_FM_STEREO_ACTIVE    0x01     /* audio stereo detected */
#define BSA_FM_MONO_ACTIVE      0x02     /* audio mono */
#define BSA_FM_BLEND_ACTIVE     0x04     /* stereo blend active */

typedef UINT8  tBSA_FM_AUDIO_QUALITY;

/* FM audio routing configuration */
#define BSA_FM_AUDIO_NONE       BTA_DM_ROUTE_NONE       /* No FM audio output */
#define BSA_FM_AUDIO_DAC        BTA_DM_ROUTE_DAC        /* routing FM over analog output */
#define BSA_FM_AUDIO_I2S        BTA_DM_ROUTE_I2S        /* routing FM over digital (I2S) output */
#define BSA_FM_AUDIO_BT_MONO    BTA_DM_ROUTE_BT_MONO    /* routing FM over SCO */
#define BSA_FM_AUDIO_BT_STEREO  BTA_DM_ROUTE_BT_STEREO  /* routing FM over BT Stereo */

typedef UINT8 tBSA_FM_AUDIO_PATH;

/* scan mode */
#define BSA_FM_PRESET_SCAN      I2C_FM_SEARCH_PRESET        /* preset scan : bit0 = 1 */
#define BSA_FM_NORMAL_SCAN      I2C_FM_SEARCH_NORMAL        /* normal scan : bit0 = 0 */
typedef UINT8 tBSA_FM_SCAN_METHOD;

/* frequency scanning direction */
#define BSA_FM_SCAN_DOWN        0x00        /* bit7 = 0 scanning toward lower frequency */
#define BSA_FM_SCAN_UP          0x80        /* bit7 = 1 scanning toward higher frequency */
typedef UINT8 tBSA_FM_SCAN_DIR;

#define BSA_FM_SCAN_FULL        (BSA_FM_SCAN_UP | BSA_FM_NORMAL_SCAN|0x02)       /* full band scan */
#define BSA_FM_FAST_SCAN        (BSA_FM_SCAN_UP | BSA_FM_PRESET_SCAN)       /* use preset scan */
#define BSA_FM_SCAN_NONE        0xff

typedef UINT8 tBSA_FM_SCAN_MODE;

#define  BSA_FM_STEP_100KHZ     0x00
#define  BSA_FM_STEP_50KHZ      0x10
typedef UINT8   tBSA_FM_STEP_TYPE;

/* minimum and maximum value of start/stop SNR for stereo/mono blend */
#define     BSA_FM_START_SNR_MIN        0
#define     BSA_FM_START_SNR_MAX        63

#define     BSA_FM_STOP_SNR_MIN         0
#define     BSA_FM_STOP_SNR_MAX         63
/* minimum and maximum value of start/stop RSSI for stereo/mono blend */
#define     BSA_FM_START_RSSI_MIN       -128
#define     BSA_FM_START_RSSI_MAX       127

#define     BSA_FM_STOP_RSSI_MIN        -128
#define     BSA_FM_STOP_RSSI_MAX        127
/* minimum and maximum value of start SNR for soft mute */
#define     BSA_FM_START_MUTE_MIN       0
#define     BSA_FM_START_MUTE_MAX       63
/* minimum and maximum value of ultimate mute attenuation level */
#define     BSA_FM_STOP_ATTEN_MIN       -128
#define     BSA_FM_STOP_ATTEN_MAX       127
/* minimum and maximum value of mute rate */
#define     BSA_FM_MUTE_RATE_MIN        0
#define     BSA_FM_MUTE_RATE_MAX        63
/* minimum and maximum value of SNR reading adjustment */
#define     BSA_FM_SNR40_MIN            -128
#define     BSA_FM_SNR40_MAX            127

/* maximum FM audio volume */
#define     BSA_FM_VOLUME_MAX       0xff

/* default FM volume*/
#ifndef BSA_FM_VOLUME_DEFAULT
#define        BSA_FM_VOLUME_DEFAULT   0x50
#endif

typedef struct
{
    UINT8       start_snr;  /* Above this SNR, the chip will be in stereo mode
                               if the RSSI is high enough, valid range 0 -63 default 43 */
    UINT8       stop_snr;   /* Below this SNR, the chip will be in MONO mode. valid range 0 -63, default 20*/
    INT8        start_rssi; /* RSSI to start blending, assuming the FMRX_START_SNR
                               is exceeded. valid range -128 ~ 127, default - 56 */
    INT8        stop_rssi;   /* RSSI to stop blending, assuming FMRX_STOP_SNR is
                               not yet exceeded. valid range -128 ~ 127, default - 85 */
}tBSA_FM_BLEND_CFG;

typedef struct
{
    UINT8       start_mute; /* SNR to start muting. Below this SNR the chip will
                               partially mute until the ultimate mute attenuation
                               is reached. valid range 0 - 63, default 19 */
    INT8       stop_atten; /* Ultimate mute attenuation level. valid range -128 ~ 127, default 26 */
    UINT8      mute_rate;  /* Rate at which muting occurs valid range 0 - 63, default 8 */
    INT8       snr40;     /* constant to add to SNR reading to get the actual SNR,
                               i.e. snr40 - snr_read = 40. valid range -128 ~ 127, default -77 */
}tBSA_FM_SOFT_MUTE_CFG;
/* struct of stereo/mono audio blend and soft mute parameters */
typedef struct
{
    tBSA_FM_BLEND_CFG       blend;  /* stereo/mono blend threshold */
    tBSA_FM_SOFT_MUTE_CFG   soft_mute; /* soft mute parameters */

}tBSA_FM_BLEND_MUTE;

/* search criteria type */
#define BSA_FM_CTYPE_SNR        (1)         /* SNR criteria */
#define BSA_FM_CTYPE_QUALITY    (2)         /* preset scan quality (COS slope) threshold criteria */
typedef UINT8 tBSA_FM_CTYPE;

/* maximum SNR value */
#define BSA_FM_SNR_MAX      31

/* maximum COS value */
#define BSA_FM_COS_MAX      127
#define BSA_FM_COS_MIN      -128

typedef union
{
    INT8   snr;            /* valid SNR value range from 0 ~ BSA_FM_SNR_MAX */
    INT8   cos;            /* valid COS slope value range from BSA_FM_COS_MIN ~ BSA_FM_COS_MAX */
}tBSA_FM_CVALUE;


/* RDS mode */
enum
{
    BSA_FM_RDS,
    BSA_FM_RBDS
};
typedef UINT8 tBSA_FM_RDS_B;

enum
{
    BSA_FM_AF_FM = 1,
    BSA_FM_AF_LF,
    BSA_FM_AF_MF
};
typedef UINT8 tBSA_FM_AF_TYPE;

/* AF structure */
typedef struct
{
    UINT16              num_af;     /* number of AF in list */
    tBSA_FM_AF_TYPE     af_type[BSA_FM_AF_MAX_NUM];
    UINT16              af_list[BSA_FM_AF_MAX_NUM];
}tBSA_FM_AF_LIST;

typedef struct
{
    UINT16              pi_code;          /* currently tuned frequency PI code */
    tBSA_FM_AF_LIST     af_list;          /* AF frequency list */
    UINT8               af_thresh;        /* AF jump RSSI threshold*/
} tBSA_FM_AF_PARAM;

enum
{
    BSA_FM_NFL_LOW,
    BSA_FM_NFL_MED,
    BSA_FM_NFL_FINE
};
typedef UINT8 tBSA_FM_NFE_LEVL;

/* channel tunning/scanning call back data */
typedef struct
{
    tBSA_FM_STATUS      status;         /* operation status */
    UINT8               rssi;
    UINT16              freq;           /* tuned frequency */
}tBSA_FM_CHNL_DATA;


/* set FM audio mode callback data */
typedef struct
{
    tBSA_FM_STATUS      status;     /* operation status */
    tBSA_FM_AUDIO_MODE  audio_mode; /* audio mode */
}tBSA_FM_MODE_INFO;

/* audio quality live updating call back data */
typedef struct
{
    tBSA_FM_STATUS          status;     /* operation status */
    UINT8                   rssi;       /* rssi strength    */
    tBSA_FM_AUDIO_QUALITY   audio_mode; /* audio mode       */
}tBSA_FM_AUD_DATA;


typedef struct
{
    tBSA_FM_STATUS          status;     /* operation status         */
    tBSA_FM_DEEMPHA_TIME    time_const; /* deemphasis parameter     */
}tBSA_FM_DEEMPH_DATA;

/* set FM audio mode callback data */
typedef struct
{
    tBSA_FM_STATUS      status;     /* operation status */
    BOOLEAN             rds_on;
    BOOLEAN             af_on;  /* audio mode */
}tBSA_FM_RDS_MODE_INFO;

typedef struct
{
    UINT8               rssi;
    UINT16              freq;
    INT8                cos; /* carrier offset */
    INT8                snr; /* only valid when preset sscan mode 3 is used, otherwise set to 0 */
}tBSA_FM_SCAN_DAT;

typedef struct
{
    tBSA_FM_STATUS      status;
    BOOLEAN             is_mute;
}tBSA_FM_MUTE_STAT;

typedef struct
{
    tBSA_FM_STATUS          status;
    tBSA_FM_REGION_CODE     region;
}tBSA_FM_REGION_INFO;

typedef struct
{
    tBSA_FM_STATUS          status;
    tBSA_FM_RDS_B           type;
}tBSA_FM_RDS_TYPE;

typedef struct
{
    tBSA_FM_STATUS          status;
    UINT8                   rssi;
}tBSA_FM_NFE;

typedef struct
{
    tBSA_FM_STATUS      status;
    UINT16              volume;
}tBSA_FM_VOLUME;

/* Union of all FM callback structures */
typedef union
{
    tBSA_FM_STATUS          status;             /* BSA_FM_DISABLE_EVT/
                                                   BSA_FM_ENABLE_EVT/
                                                   BSA_FM_AUD_PATH_EVT/
                                                   BSA_FM_RDS_MODE_EVT
                                                   BSA_FM_AUD_PATH_EVT
                                                   BSA_FM_RDS_UPD_EVT
                                                   BSA_FM_CFG_BLEND_MUTE_EVT call back data*/
    tBSA_FM_CHNL_DATA       chnl_info;          /* BSA_FM_TUNE_EVT/BSA_FM_SEARCH_CMPL_EVT */
    tBSA_FM_MODE_INFO       mode_info;          /* BSA_FM_AUD_MODE_EVT */
    tBSA_FM_AUD_DATA        audio_data;         /* BSA_FM_AUD_DATA_EVT call back data */
    tBSA_FM_DEEMPH_DATA     deemphasis;         /* BSA_FM_SET_DEEMPH_EVT call back data */
    tBSA_FM_RDS_MODE_INFO   rds_mode;           /* BSA_FM_RDS_MODE_EVT call back data */
    tBSA_FM_SCAN_DAT        scan_data;          /* BSA_FM_SEARCH_EVT call back data */
    tBSA_FM_MUTE_STAT       mute_stat;          /* BSA_FM_MUTE_AUD_EVT call back data */
    tBSA_FM_STEP_TYPE       scan_step;          /* BSA_FM_SCAN_STEP_EVT callback data */
    tBSA_FM_REGION_INFO     region_info;        /* BSA_FM_SET_REGION_EVT callback data */
    tBSA_FM_RDS_TYPE        rds_type;
    tBSA_FM_NFE             nfloor;
    tBSA_FM_VOLUME          volume;             /* BSA_FM_VOLUME_EVT */
} tBSA_FM;

/* run-time configuration struct */
typedef struct
{
    UINT16                  low_bound;      /* lowest frequency boundary */
    UINT16                  high_bound;     /* highest frequency boundary */
    tBSA_FM_DEEMPHA_TIME    deemphasis;     /* FM de-emphasis time constant */
    UINT8                   scan_step;      /* scanning step */
} tBSA_FM_CFG_ENTY;

typedef struct
{
    tBSA_FM_CFG_ENTY        reg_cfg_tbl[BSA_MAX_REGION_SETTING];
                                            /* region related setting table */
    INT32                   aud_timer_value;/* audio quality live updating timer
                                               value, must be a non-0 positive value */
    INT32                   rds_timer_value;/* RDS data update timer */
    INT32                   enable_delay_val;/* FM enable delay time value, platform dependant */
    UINT8                   rds_read_size;  /* how many RDS tuples per read */
    UINT8                   max_station;    /* maximum number of FM stations for a full band scan
                                                can not exceed BSA_FM_MAX_PRESET_STA */
} tBSA_FM_CFG;

/* BTA FM callback function */
typedef void (tBSA_FM_CBACK)(tBSA_FM_EVT event, tBSA_FM *p_data);
typedef void (tBSA_FM_RDS_CBACK)(tBSA_FM_EVT event, UINT8*p_data);

typedef struct
{
    tBSA_FM_CBACK * p_cback;
    tBSA_FM_RDS_CBACK * p_rds_cback;
    tBSA_FM_FUNC_MASK func_mask;
    UINT8 app_id;
} tBSA_FM_ENABLE;

typedef struct
{
    int stat;
} tBSA_FM_ENABLE_MSG;

typedef union {
    tBSA_FM_ENABLE_MSG enable;
} tBSA_FM_MSG;

#ifndef tBSA_COMBO_SRCH_REQ
typedef struct
{
    UINT16 start_freq;
    UINT16 end_freq;
    UINT8 rssi;
    tBSA_FM_SCAN_DIR direction;
    tBSA_FM_SCAN_METHOD scn_method;
    BOOLEAN multi_chnl;
    tBSA_FM_SCH_RDS_COND *p_rds_cond;
} tBSA_COMBO_SRCH_REQ ;
#endif

#ifndef tBSA_FM_SRCH_CRITERIA_REQ
typedef struct
{
    tBSA_FM_CTYPE criteria_type;
    tBSA_FM_CVALUE *p_value ;
} tBSA_FM_SRCH_CRITERIA_REQ;
#endif

#ifndef tBSA_FM_MSGID_SRCH_CMD_REQ
typedef struct
{
    tBSA_FM_SCAN_MODE scn_mode;
    UINT8 rssi_thresh;
    tBSA_FM_SCH_RDS_COND *p_rds_cond;
} tBSA_FM_MSGID_SRCH_CMD_REQ;
#endif

#ifndef tBSA_FM_MSGID_SET_RDS_CMD_REQ
typedef struct
{
    BOOLEAN rds_on;
    BOOLEAN af_on;
    tBSA_FM_AF_PARAM af_struct;
} tBSA_FM_MSGID_SET_RDS_CMD_REQ;
#endif

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

tBSA_STATUS BSA_FmEnableInit(tBSA_FM_ENABLE *p_req);

/*******************************************************************************
**
** Function         BSA_FmEnable
**
** Description      Enable FM fuctionality.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_FmEnable(tBSA_FM_ENABLE *p_req);

/*******************************************************************************
**
** Function         BSA_FmDisable
**
** Description      Disable FM functionality.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
void BSA_FmDisable(void);

/*******************************************************************************
**
** Function         BSA_FmReadRDS
**
** Description      Read RDS information. For every BSA_FmReadRDS API call,
**                  will receive one or more application BSA_FM_RDS_UPD_EVT event
**                  call back. For each call back event, 50 or less tuples of RDS
**                  data will be returned.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
void BSA_FmReadRDS(void);

/*******************************************************************************
**
** Function         BSA_FmSetSchCriteriaInit
**
** Description      Initial struct for BSA_FmSetSchCriteria
**
** Parameters       struct for BSA_FmSetSchCriteria:
**                  criteria_type: criteria type, can be SNR or preset quality.
**                  p_value: pointer to search criteria value.
**
** Returns          void
**
*******************************************************************************/
void BSA_FmSetSchCriteriaInit(tBSA_FM_SRCH_CRITERIA_REQ *req);

/*******************************************************************************
**
** Function         BSA_FmSetSchCriteria
**
** Description      Set FM search criteria if any special SNR condition wants to
**                  be used.
**
** Parameters       struct for
**                  criteria_type: criteria type, can be SNR or preset quality.
**                  p_value: pointer to search criteria value.
**
** Returns          void
**
*******************************************************************************/
void BSA_FmSetSchCriteria(tBSA_FM_SRCH_CRITERIA_REQ *req);

/*******************************************************************************
**
** Function         BSA_FmSearchFreqInit
**
** Description      Init struct for BSA_FmSearchFreq
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
void BSA_FmSearchFreqInit(tBSA_FM_MSGID_SRCH_CMD_REQ * req);

/*******************************************************************************
**
** Function         BSA_FmSearchFreq
**
** Description      Scan FM toward higher/lower frequency for next clear channel.
**                  If no clear channel is found, BSA_FM_SEARCH_EVT event with
**                  BSA_FM_SCAN_FAIL status will be returned.
**
** Parameters       struct for BSA_FmSearchFreq
**
** Returns          void
**
*******************************************************************************/
void BSA_FmSearchFreq(tBSA_FM_MSGID_SRCH_CMD_REQ * req);

/*******************************************************************************
**
** Function         BSA_FmTuneFreq
**
** Description      Tune to a designated frequency, BSA_FM_TUNE_EVT will be returned
**                  when tuning is finished.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
void BSA_FmTuneFreq (UINT16 freq);

/*******************************************************************************
**
** Function         BSA_FmSearchAbort
**
** Description      Abort a frequency scanning operation. BSA_FM_SEARCH_EVT will
**                  be sent to application when operation is aborted, and the
**                  stopping frequency is reported with the event.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
void BSA_FmSearchAbort (void);

/*******************************************************************************
**
** Function         BSA_FmSetAudioMode
**
** Description      Configure FM audio mode to be mono/stereo/blend. When operation
**                  finishes, event BSA_FM_AUD_MODE_EVT will be returned.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
void BSA_FmSetAudioMode ( tBSA_FM_AUDIO_MODE mode);


/*******************************************************************************
**
** Function         BSA_FmSetRDSModeInit
**
** Description      Init struct for BSA_FmSetRDSMode
**
** Parameters       struct for BSA_FmSetRDSMode
**
** Returns          void
**
*******************************************************************************/
void BSA_FmSetRDSModeInit (tBSA_FM_MSGID_SET_RDS_CMD_REQ *req);

/*******************************************************************************
**
** Function         BSA_FmSetRDSMode
**
** Description      Turn on/off RDS feature and AF algorithm.
**
** Parameters       struct for BSA_FmSetRDSMode
**
** Returns          void
**
*******************************************************************************/
void BSA_FmSetRDSMode (tBSA_FM_MSGID_SET_RDS_CMD_REQ *req);

/*******************************************************************************
**
** Function         BSA_FmConfigAudioPath
**
** Description      Set FM audio path. When configuring FM audio over SCO, an
**                  existing SCO connection is requested.
**
** Parameters       audio_path: Bitmask of the paths to be turned ON.
**
**
** Returns          void
**
*******************************************************************************/
void BSA_FmConfigAudioPath(tBSA_FM_AUDIO_PATH audio_path);

/*******************************************************************************
**
** Function         BSA_FmReadAudioQuality
**
** Description      Turn on/off live audio data updating.
**
** Parameters       turn_on: TRUE: updating audio quality data(rssi, mode).
**                           FALSE: stop updaing audio quality data.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
void BSA_FmReadAudioQuality(BOOLEAN turn_on);


/*******************************************************************************
**
** Function         BSA_FmConfigDeemphasis
**
** Description      Config deemphasis parameter.
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
void BSA_FmConfigDeemphasis(tBSA_FM_DEEMPHA_TIME time_const);

/*******************************************************************************
**
** Function         BSA_FmMute
**
** Description      Mute/Unmute FM audio
**
** Parameters       TRUE:  mute audio
**                  FALSE: unmute audio
**
** Returns          void
**
*******************************************************************************/
void BSA_FmMute(BOOLEAN mute);

/*******************************************************************************
**
** Function         BSA_FmSetScanStep
**
** Description      Configure FM Scanning Step.
**
** Parameters       step: 50KHz/ 100KHz step
**
** Returns          void
**
*******************************************************************************/
void BSA_FmSetScanStep(tBSA_FM_STEP_TYPE step);
 /*******************************************************************************
**
** Function         BSA_FmEstNoiseFloor
**
** Description      Estimate noise floor.
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
void BSA_FmEstNoiseFloor(tBSA_FM_NFE_LEVL level);

/*******************************************************************************
**
** Function         BSA_FmSetRegion
**
** Description      Set a region for band selection.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
void BSA_FmSetRegion (tBSA_FM_REGION_CODE region);


/*******************************************************************************
 **
 ** Function         BSA_FmFmComboSearchInit
 **
 ** Description      Init a structure to be used with BSA_FmFmComboSearch
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_FmComboSearchInit(tBSA_COMBO_SRCH_REQ *p_req);


/*******************************************************************************
**
** Function         BSA_FmComboSearch
**
** Description      A search function let user define frequency range
**                  [start_freq, end_freq](inclusive bracket), mode,
**                  search directions and multiple channel requirement.
**
**
** Parameters       req: combo search parameters as follows:
**                  start_freq: Starting frequency of search operation range.
**                  end_freq: Ending frequency of search operation, if the same
**                            as start_freq, start_freq will be the only freqeucny in
**                            search. When start_freq and end_freq is different and
**                            back to back, will start from start_freq, hit the
**                            boundary and wrap around search through the ending frequency.
**                  rssi: search RSSI threshold.
**                  direction: the direction to search in, it can only be either
**                              BSA_FM_SCAN_DOWN or BSA_FM_SCAN_UP.
**                  scn_method: search method, it can only be either BSA_FM_PRESET_SCAN
**                            or BSA_FM_NORMAL_SCAN.
**                  multi_chnl: Is multiple channels are required, or only find
**                              next valid channel(seek).
**                  p_rds_cond: RDS search oncdition, can not combine with
**                              BSA_FM_PRESET_SCAN mode.
**
** Returns          void
**
*******************************************************************************/
void BSA_FmComboSearch(tBSA_COMBO_SRCH_REQ * req);

/*******************************************************************************
**
** Function         BSA_FmSetRdsRbds
**
** Description      Choose the RDS mode - RDS/RBDS.
**
** Parameters       rds_type: BSA_FM_RDS or BSA_FM_RBDS.
**
** Returns          void
**
*******************************************************************************/
void BSA_FmSetRdsRbds(tBSA_FM_RDS_B  rds_type);

/*******************************************************************************
**
** Function         BSA_FmSetSignalNotifSetting
**
** Description      Configure RSSI value polling interval.
**
** Parameters       poll_interval: polling interval in milli second unit.
**
** Returns          void
**
*******************************************************************************/
tBSA_FM_STATUS BSA_FmSetSignalNotifSetting(INT32 poll_interval);

/*******************************************************************************
**
** Function         BSA_FmConfigBlendSoftMuteParamsInit
**
** Description      Initialize struct for BSA_FmConfigBlendSoftMuteParams
**
** Parameters       tBSA_FM_BLEND_MUTE sturct
**
** Returns          void
**
*******************************************************************************/
void BSA_FmConfigBlendSoftMuteParamsInit(tBSA_FM_BLEND_MUTE *p_config);

/*******************************************************************************
**
** Function         BSA_FmConfigBlendSoftMuteParams
**
** Description      Configure RSSI and SNR threshold for audio blending and soft
**                  mute.
**
** Parameters       p_config: The pointer to the blending and soft mute parameters
**                            structure.
**
** Returns          void
**
*******************************************************************************/
void BSA_FmConfigBlendSoftMuteParams(tBSA_FM_BLEND_MUTE *p_config);

/*******************************************************************************
**
** Function         BSA_FmVolumeControl
**
** Description      This command control the audio volume. The audio volume is
**                  a linear value of fraction of 1, range from 0 to 0xff, where
**                  0xff stands for the highest volume. The default audio volume
**                  on start is 0xff.
**
** Parameters       volume: is an UINT16 type, value range from 0 to 0xff, where
**                          0xff indicates the highest volume.
**
** Returns          void
**
*******************************************************************************/
void BSA_FmVolumeControl(UINT16 volume);


#ifdef __cplusplus
}
#endif

#endif /* BSA_FM_API_H */
