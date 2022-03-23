/*****************************************************************************
**
**  Name:           bta_fm_api.h
**
**  Description:    This is the public interface file for the FM subsystem of
**                  BTA, Broadcom's Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2006, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_FM_API_H
#define BTA_FM_API_H

#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* Extra Debug Code */
#ifndef BTA_FM_DEBUG
#define BTA_FM_DEBUG           FALSE
#endif

#ifndef BTA_FM_AF_INCLUDED
#define BTA_FM_AF_INCLUDED      TRUE
#endif

#ifndef BTA_FM_10KHZ_INCLUDED
#define BTA_FM_10KHZ_INCLUDED      TRUE
#endif

/* Scanning step option provided by software algorithm */
#ifndef BTA_FM_SCAN_STEP_ALGO
#define BTA_FM_SCAN_STEP_ALGO           TRUE
#endif

/* noise floor estimation */
#ifndef     BTA_FM_NFE_DEFAILT
#define     BTA_FM_NFE_DEFAILT      93      /* default Noise floor value */
#endif
#ifndef     BTA_FM_NFE_THRESH
#define     BTA_FM_NFE_THRESH       0x32    /* default NFE threshold 53 db */
#endif
#ifndef     BTA_FM_NFE_SNR_STEREO
#define     BTA_FM_NFE_SNR_STEREO   19      /* Stereo audio mode SNR level above NFL */
#endif
#ifndef     BTA_FM_NFE_SNR_MONO
#define     BTA_FM_NFE_SNR_MONO     10      /* Mono audio mode SNR level above NFL */
#endif

/* Max AF number */
#ifndef BTA_FM_AF_MAX_NUM
#define BTA_FM_AF_MAX_NUM   25
#endif

/* preset channel register max size 248 bytes, each channel will take 4/5 bytes depend on search mode
   so max preset channel n has to be 5*n <= 248. set default to be 48 */
#define BTA_FM_MAX_PRESET_STA       48      /* same number defined as I2C_PRESET_STA_MAX */

enum
{
    BTA_FM_ENABLE_EVT,          /* 0 BTA FM is enabled */
    BTA_FM_DISABLE_EVT,         /* 1 BTA FM is disabled */
    BTA_FM_RDS_UPD_EVT,         /* 2 read RDS data event */
    BTA_FM_SEARCH_CMPL_EVT,     /* 3 FM scanning is completed event */
    BTA_FM_TUNE_EVT,			/* 4 FM TUNE event */
    BTA_FM_AUD_MODE_EVT,        /* 5 Set audio mode completed  */
    BTA_FM_MUTE_AUD_EVT,        /* 6 audio mute/unmute event */
    BTA_FM_SCAN_STEP_EVT,       /* 7 config scan step */
    BTA_FM_RDS_MODE_EVT,        /* 8 set RDS mode event */
    BTA_FM_AUD_PATH_EVT,        /* 9 set audio path completed */
    BTA_FM_AUD_DATA_EVT,        /* 10 audio quality live updating call back event */
    BTA_FM_SET_DEEMPH_EVT,      /* 11 configure deempahsis parameter event */
    BTA_FM_SET_REGION_EVT,      /* 12 set band region */
    BTA_FM_NFL_EVT,				/* 13 noise floor estimation event */
    BTA_FM_RDS_TYPE_EVT,		/* 14 Set RDS type event */
    BTA_FM_CFG_BLEND_MUTE_EVT,  /* 15 stereo/mono blend and soft mute configuration event */
    BTA_FM_VOLUME_EVT,          /* 16 volume event */
    BTA_FM_SEARCH_EVT,		    /* 17 search event */
    BTA_FM_AF_JMP_EVT           /* 18 AF jump event */
};


typedef UINT8  tBTA_FM_EVT;

enum
{
    BTA_FM_OK,
    BTA_FM_SCAN_RSSI_LOW,
    BTA_FM_SCAN_FAIL,
    BTA_FM_SCAN_ABORT,
    BTA_FM_SCAN_NO_RES,
    BTA_FM_ERR,
    BTA_FM_UNSPT_ERR,
    BTA_FM_FLAG_TOUT_ERR,
    BTA_FM_FREQ_ERR,
    BTA_FM_VCMD_ERR,
    BTA_FM_BUSY
};
typedef UINT8   tBTA_FM_STATUS;

/* FM band region, bit 0, 1,2 of func_mask */
#ifndef BTA_MAX_REGION_SETTING
#if BTA_FM_10KHZ_INCLUDED == TRUE
#define BTA_MAX_REGION_SETTING  5   /* max region code defined */
#else
#define BTA_MAX_REGION_SETTING  3   /* max region code defined */
#endif
#endif

/* FM function mask */
#define     BTA_FM_REGION_NA    0x00        /* bit0/bit1/bit2: north america */
#define     BTA_FM_REGION_EUR   0x01        /* bit0/bit1/bit2: Europe           */
#define     BTA_FM_REGION_JP    0x02        /* bit0/bit1/bit2: Japan            */
#if BTA_FM_10KHZ_INCLUDED
#define     BTA_FM_REGION_RUS   0x03        /* bit0/bit1/bit2: Russia with extended        */
#define     BTA_FM_REGION_CHN   0x04        /* bit0/bit1/bit2: China: 76.0 ~ 108.0  */
#endif
#define     BTA_FM_RDS_BIT      1<<4        /* bit4: RDS functionality */
#define     BTA_FM_RBDS_BIT     1<<5        /* bit5: RBDS functionality, exclusive with RDS bit */
#define     BTA_FM_AF_BIT       1<<6        /* bit6: AF functionality */
typedef UINT8   tBTA_FM_FUNC_MASK;

/* low 3 bits (bit0, 1, 2)of FUNC mask is region code */
#if BTA_FM_10KHZ_INCLUDED
#define     BTA_FM_REGION_MAX           BTA_FM_REGION_CHN
#define     BTA_FM_REGION_MASK          (BTA_FM_REGION_NA | BTA_FM_REGION_EUR | BTA_FM_REGION_RUS | BTA_FM_REGION_CHN)
#else
#define     BTA_FM_REGION_MAX           BTA_FM_REGION_JP
#define     BTA_FM_REGION_MASK          (BTA_FM_REGION_NA | BTA_FM_REGION_EUR)             /* low 3 bits (bit0, 1)of FUNC mask is region code */
#endif

typedef UINT8  tBTA_FM_REGION_CODE;

#define     BTA_FM_DEEMPHA_50U      0       /* 6th bit in FM_AUDIO_CTRL0 set to 0, Europe default */
#define     BTA_FM_DEEMPHA_75U      1<<6    /* 6th bit in FM_AUDIO_CTRL0 set to 1, US  default */
typedef UINT8 tBTA_FM_DEEMPHA_TIME;

#define BTA_FM_GET_FREQ(x)  ((UINT16)((x * 10) - 64000))

enum
{
    BTA_FM_SCH_RDS_NONE,
    BTA_FM_SCH_RDS_PTY,
    BTA_FM_SCH_RDS_TP
};
typedef UINT8 tBTA_FM_SCH_RDS_TYPE;

typedef struct
{
    tBTA_FM_SCH_RDS_TYPE    cond_type;
    UINT8                   cond_val;
} tBTA_FM_SCH_RDS_COND;

/* FM audio output mode */
enum
{
    BTA_FM_AUTO_MODE,       /* auto blend by default */
    BTA_FM_STEREO_MODE,     /* manual stereo switch */
    BTA_FM_MONO_MODE,       /* manual mono switch */
    BTA_FM_SWITCH_MODE      /* auto stereo, and switch activated */
};
typedef UINT8  tBTA_FM_AUDIO_MODE;

/* FM audio output quality */
#define BTA_FM_STEREO_ACTIVE    0x01     /* audio stereo detected */
#define BTA_FM_MONO_ACTIVE      0x02     /* audio mono */
#define BTA_FM_BLEND_ACTIVE     0x04     /* stereo blend active */

typedef UINT8  tBTA_FM_AUDIO_QUALITY;

/* FM audio routing configuration */
#define BTA_FM_AUDIO_NONE       BTA_DM_ROUTE_NONE       /* No FM audio output */
#define BTA_FM_AUDIO_DAC        BTA_DM_ROUTE_DAC        /* routing FM over analog output */
#define BTA_FM_AUDIO_I2S        BTA_DM_ROUTE_I2S        /* routing FM over digital (I2S) output */
#define BTA_FM_AUDIO_BT_MONO    BTA_DM_ROUTE_BT_MONO    /* routing FM over SCO */
#define BTA_FM_AUDIO_BT_STEREO  BTA_DM_ROUTE_BT_STEREO  /* routing FM over BT Stereo */

typedef UINT8 tBTA_FM_AUDIO_PATH;

/* scan mode */
#define BTA_FM_PRESET_SCAN      I2C_FM_SEARCH_PRESET        /* preset scan : bit0 = 1 */
#define BTA_FM_NORMAL_SCAN      I2C_FM_SEARCH_NORMAL        /* normal scan : bit0 = 0 */
typedef UINT8 tBTA_FM_SCAN_METHOD;

/* frequency scanning direction */
#define BTA_FM_SCAN_DOWN        0x00        /* bit7 = 0 scanning toward lower frequency */
#define BTA_FM_SCAN_UP          0x80        /* bit7 = 1 scanning toward higher frequency */
typedef UINT8 tBTA_FM_SCAN_DIR;

#define BTA_FM_SCAN_FULL        (BTA_FM_SCAN_UP | BTA_FM_NORMAL_SCAN|0x02)       /* full band scan */
#define BTA_FM_FAST_SCAN        (BTA_FM_SCAN_UP | BTA_FM_PRESET_SCAN)       /* use preset scan */
#define BTA_FM_SCAN_NONE        0xff

typedef UINT8 tBTA_FM_SCAN_MODE;

#define  BTA_FM_STEP_100KHZ     0x00
#define  BTA_FM_STEP_50KHZ      0x10
typedef UINT8   tBTA_FM_STEP_TYPE;

/* minimum and maximum value of start/stop SNR for stereo/mono blend */
#define     BTA_FM_START_SNR_MIN        0
#define     BTA_FM_START_SNR_MAX        63

#define     BTA_FM_STOP_SNR_MIN         0
#define     BTA_FM_STOP_SNR_MAX         63
/* minimum and maximum value of start/stop RSSI for stereo/mono blend */
#define     BTA_FM_START_RSSI_MIN       -128
#define     BTA_FM_START_RSSI_MAX       127

#define     BTA_FM_STOP_RSSI_MIN        -128
#define     BTA_FM_STOP_RSSI_MAX        127
/* minimum and maximum value of start SNR for soft mute */
#define     BTA_FM_START_MUTE_MIN       0
#define     BTA_FM_START_MUTE_MAX       63
/* minimum and maximum value of ultimate mute attenuation level */
#define     BTA_FM_STOP_ATTEN_MIN       -128
#define     BTA_FM_STOP_ATTEN_MAX       127
/* minimum and maximum value of mute rate */
#define     BTA_FM_MUTE_RATE_MIN        0
#define     BTA_FM_MUTE_RATE_MAX        63
/* minimum and maximum value of SNR reading adjustment */
#define     BTA_FM_SNR40_MIN            -128
#define     BTA_FM_SNR40_MAX            127

/* maximum FM audio volume */
#define     BTA_FM_VOLUME_MAX       0xff

/* default FM volume*/
#ifndef BTA_FM_VOLUME_DEFAULT
#define		BTA_FM_VOLUME_DEFAULT   0x50
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
}tBTA_FM_BLEND_CFG;

typedef struct
{
    UINT8       start_mute; /* SNR to start muting. Below this SNR the chip will
                               partially mute until the ultimate mute attenuation
                               is reached. valid range 0 - 63, default 19 */
    INT8       stop_atten; /* Ultimate mute attenuation level. valid range -128 ~ 127, default 26 */
    UINT8      mute_rate;  /* Rate at which muting occurs valid range 0 - 63, default 8 */
    INT8       snr40;     /* constant to add to SNR reading to get the actual SNR,
                               i.e. snr40 - snr_read = 40. valid range -128 ~ 127, default -77 */
}tBTA_FM_SOFT_MUTE_CFG;
/* struct of stereo/mono audio blend and soft mute parameters */
typedef struct
{
    tBTA_FM_BLEND_CFG       blend;  /* stereo/mono blend threshold */
    tBTA_FM_SOFT_MUTE_CFG   soft_mute; /* soft mute parameters */

}tBTA_FM_BLEND_MUTE;

/* search criteria type */
#define BTA_FM_CTYPE_SNR        (1)         /* SNR criteria */
#define BTA_FM_CTYPE_QUALITY    (2)         /* preset scan quality (COS slope) threshold criteria */
typedef UINT8 tBTA_FM_CTYPE;

/* maximum SNR value */
#define BTA_FM_SNR_MAX      31

/* maximum COS value */
#define BTA_FM_COS_MAX      127
#define BTA_FM_COS_MIN      -128

typedef union
{
    INT8   snr;            /* valid SNR value range from 0 ~ BTA_FM_SNR_MAX */
    INT8   cos;            /* valid COS slope value range from BTA_FM_COS_MIN ~ BTA_FM_COS_MAX */
}tBTA_FM_CVALUE;


/* RDS mode */
enum
{
    BTA_FM_RDS,
    BTA_FM_RBDS
};
typedef UINT8 tBTA_FM_RDS_B;

enum
{
    BTA_FM_AF_FM = 1,
    BTA_FM_AF_LF,
    BTA_FM_AF_MF
};
typedef UINT8 tBTA_FM_AF_TYPE;

/* AF structure */
typedef struct
{
    UINT16              num_af;     /* number of AF in list */
    tBTA_FM_AF_TYPE     af_type[BTA_FM_AF_MAX_NUM];
    UINT16              af_list[BTA_FM_AF_MAX_NUM];
}tBTA_FM_AF_LIST;

typedef struct
{
    UINT16              pi_code;          /* currently tuned frequency PI code */
    tBTA_FM_AF_LIST     af_list;          /* AF frequency list */
    UINT8               af_thresh;        /* AF jump RSSI threshold*/
} tBTA_FM_AF_PARAM;

enum
{
    BTA_FM_NFL_LOW,
    BTA_FM_NFL_MED,
    BTA_FM_NFL_FINE
};
typedef UINT8 tBTA_FM_NFE_LEVL;

/* channel tunning/scanning call back data */
typedef struct
{
    tBTA_FM_STATUS      status;         /* operation status */
    UINT8               rssi;
    UINT16              freq;           /* tuned frequency */
}tBTA_FM_CHNL_DATA;


/* set FM audio mode callback data */
typedef struct
{
    tBTA_FM_STATUS      status;     /* operation status */
    tBTA_FM_AUDIO_MODE  audio_mode; /* audio mode */
}tBTA_FM_MODE_INFO;

/* audio quality live updating call back data */
typedef struct
{
    tBTA_FM_STATUS          status;     /* operation status */
    UINT8                   rssi;       /* rssi strength    */
    tBTA_FM_AUDIO_QUALITY   audio_mode; /* audio mode       */
}tBTA_FM_AUD_DATA;


typedef struct
{
    tBTA_FM_STATUS          status;     /* operation status         */
    tBTA_FM_DEEMPHA_TIME    time_const; /* deemphasis parameter     */
}tBTA_FM_DEEMPH_DATA;

/* set FM audio mode callback data */
typedef struct
{
    tBTA_FM_STATUS      status;     /* operation status */
    BOOLEAN             rds_on;
    BOOLEAN             af_on;  /* audio mode */
}tBTA_FM_RDS_MODE_INFO;

typedef struct
{
    UINT8               rssi;
    UINT16              freq;
    INT8                cos; /* carrier offset */
    INT8                snr; /* only valid when preset sscan mode 3 is used, otherwise set to 0 */
}tBTA_FM_SCAN_DAT;

typedef struct
{
    tBTA_FM_STATUS      status;
    BOOLEAN             is_mute;
}tBTA_FM_MUTE_STAT;

typedef struct
{
    tBTA_FM_STATUS          status;
    tBTA_FM_REGION_CODE     region;
}tBTA_FM_REGION_INFO;

typedef struct
{
    tBTA_FM_STATUS          status;
    tBTA_FM_RDS_B           type;
}tBTA_FM_RDS_TYPE;

typedef struct
{
    tBTA_FM_STATUS          status;
    UINT8                   rssi;
}tBTA_FM_NFE;

typedef struct
{
    tBTA_FM_STATUS      status;
    UINT16              volume;
}tBTA_FM_VOLUME;

/* Union of all FM callback structures */
typedef union
{
    tBTA_FM_STATUS          status;             /* BTA_FM_DISABLE_EVT/
                                                   BTA_FM_ENABLE_EVT/
                                                   BTA_FM_AUD_PATH_EVT/
                                                   BTA_FM_RDS_MODE_EVT
												   BTA_FM_AUD_PATH_EVT
                                                   BTA_FM_RDS_UPD_EVT
                                                   BTA_FM_CFG_BLEND_MUTE_EVT call back data*/
    tBTA_FM_CHNL_DATA       chnl_info;          /* BTA_FM_TUNE_EVT/BTA_FM_SEARCH_CMPL_EVT */
    tBTA_FM_MODE_INFO       mode_info;          /* BTA_FM_AUD_MODE_EVT */
    tBTA_FM_AUD_DATA        audio_data;         /* BTA_FM_AUD_DATA_EVT call back data */
    tBTA_FM_DEEMPH_DATA     deemphasis;         /* BTA_FM_SET_DEEMPH_EVT call back data */
    tBTA_FM_RDS_MODE_INFO   rds_mode;           /* BTA_FM_RDS_MODE_EVT call back data */
    tBTA_FM_SCAN_DAT        scan_data;          /* BTA_FM_SEARCH_EVT call back data */
    tBTA_FM_MUTE_STAT       mute_stat;          /* BTA_FM_MUTE_AUD_EVT call back data */
    tBTA_FM_STEP_TYPE       scan_step;          /* BTA_FM_SCAN_STEP_EVT callback data */
    tBTA_FM_REGION_INFO     region_info;        /* BTA_FM_SET_REGION_EVT callback data */
    tBTA_FM_RDS_TYPE        rds_type;
    tBTA_FM_NFE             nfloor;
    tBTA_FM_VOLUME          volume;             /* BTA_FM_VOLUME_EVT */
} tBTA_FM;

/* run-time configuration struct */
typedef struct
{
    UINT16                  low_bound;      /* lowest frequency boundary */
    UINT16                  high_bound;     /* highest frequency boundary */
    tBTA_FM_DEEMPHA_TIME    deemphasis;     /* FM de-emphasis time constant */
    UINT8                   scan_step;      /* scanning step */
} tBTA_FM_CFG_ENTY;

typedef struct
{
    tBTA_FM_CFG_ENTY        reg_cfg_tbl[BTA_MAX_REGION_SETTING];
                                            /* region related setting table */
    INT32                   aud_timer_value;/* audio quality live updating timer
                                               value, must be a non-0 positive value */
    INT32                   rds_timer_value;/* RDS data update timer */
    INT32                   enable_delay_val;/* FM enable delay time value, platform dependant */
    UINT8                   rds_read_size;  /* how many RDS tuples per read */
    UINT8                   max_station;    /* maximum number of FM stations for a full band scan
                                                can not exceed BTA_FM_MAX_PRESET_STA */
} tBTA_FM_CFG;

/* BTA FM callback function */
typedef void (tBTA_FM_CBACK)(tBTA_FM_EVT event, tBTA_FM *p_data);

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_FmEnable
**
** Description      Enable FM fuctionality.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmEnable(tBTA_FM_FUNC_MASK func_mask,
                                 tBTA_FM_CBACK *p_cback, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_FmDisable
**
** Description      Disable FM functionality.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmDisable(void);

/*******************************************************************************
**
** Function         BTA_FmReadRDS
**
** Description      Read RDS information. For every BTA_FmReadRDS API call,
**                  will receive one or more application BTA_FM_RDS_UPD_EVT event
**                  call back. For each call back event, 50 or less tuples of RDS
**                  data will be returned.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmReadRDS(void);

/*******************************************************************************
**
** Function         BTA_FmSetSchCriteria
**
** Description      Set FM search criteria if any special SNR condition wants to
**                  be used.
**
** Parameters       criteria_type: criteria type, can be SNR or preset quality.
**                  p_value: pointer to search criteria value structure.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern tBTA_FM_STATUS BTA_FmSetSchCriteria(tBTA_FM_CTYPE criteria_type,
                                                   tBTA_FM_CVALUE *p_value );

/*******************************************************************************
**
** Function         BTA_FmSearchFreq
**
** Description      Scan FM toward higher/lower frequency for next clear channel.
**                  If no clear channel is found, BTA_FM_SEARCH_EVT event with
**                  BTA_FM_SCAN_FAIL status will be returned.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmSearchFreq(tBTA_FM_SCAN_MODE scn_mode, UINT8 rssi_thresh,
                                    tBTA_FM_SCH_RDS_COND *p_rds_cond);

/*******************************************************************************
**
** Function         BTA_FmTuneFreq
**
** Description      Tune to a designated frequency, BTA_FM_TUNE_EVT will be returned
**                  when tuning is finished.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmTuneFreq (UINT16 freq);

/*******************************************************************************
**
** Function         BTA_FmSearchAbort
**
** Description      Abort a frequency scanning operation. BTA_FM_SEARCH_EVT will
**                  be sent to application when operation is aborted, and the
**                  stopping frequency is reported with the event.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmSearchAbort (void);

/*******************************************************************************
**
** Function         BTA_FmSetAudioMode
**
** Description      Configure FM audio mode to be mono/stereo/blend. When operation
**                  finishes, event BTA_FM_AUD_MODE_EVT will be returned.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmSetAudioMode ( tBTA_FM_AUDIO_MODE mode);

/*******************************************************************************
**
** Function         BTA_FmSetRDSMode
**
** Description      Turn on/off RDS feature and AF algorithm.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmSetRDSMode (BOOLEAN rds_on, BOOLEAN af_on, tBTA_FM_AF_PARAM *p_af_struct);


/*******************************************************************************
**
** Function         BTA_FmConfigAudioPath
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
BTA_API extern void BTA_FmConfigAudioPath(tBTA_FM_AUDIO_PATH audio_path);

/*******************************************************************************
**
** Function         BTA_FmReadAudioQuality
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
BTA_API extern void BTA_FmReadAudioQuality(BOOLEAN turn_on);


/*******************************************************************************
**
** Function         BTA_FmConfigDeemphasis
**
** Description      Config deemphasis parameter.
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmConfigDeemphasis(tBTA_FM_DEEMPHA_TIME time_const);

/*******************************************************************************
**
** Function         BTA_FmMute
**
** Description      Mute/Unmute FM audio
**
** Parameters       TRUE:  mute audio
**                  FALSE: unmute audio
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmMute(BOOLEAN mute);

/*******************************************************************************
**
** Function         BTA_FmSetScanStep
**
** Description      Configure FM Scanning Step.
**
** Parameters       step: 50KHz/ 100KHz step
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmSetScanStep(tBTA_FM_STEP_TYPE step);
 /*******************************************************************************
**
** Function         BTA_FmEstNoiseFloor
**
** Description      Estimate noise floor.
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmEstNoiseFloor(tBTA_FM_NFE_LEVL level);

/*******************************************************************************
**
** Function         BTA_FmSetRegion
**
** Description      Set a region for band selection.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmSetRegion (tBTA_FM_REGION_CODE region);

/*******************************************************************************
**
** Function         BTA_FmComboSearch
**
** Description      A search function let user define frequency range
**                  [start_freq, end_freq](inclusive bracket), mode,
**                  search directions and multiple channel requirement.
**
**
** Parameters       start_freq: Starting frequency of search operation range.
**                  end_freq: Ending frequency of search operation, if the same
**                            as start_freq, start_freq will be the only freqeucny in
**                            search. When start_freq and end_freq is different and
**                            back to back, will start from start_freq, hit the
**                            boundary and wrap around search through the ending frequency.
**                  rssi: search RSSI threshold.
**                  direction: the direction to search in, it can only be either
**                              BTA_FM_SCAN_DOWN or BTA_FM_SCAN_UP.
**                  scn_method: search method, it can only be either BTA_FM_PRESET_SCAN
**                            or BTA_FM_NORMAL_SCAN.
**                  multi_chnl: Is multiple channels are required, or only find
**                              next valid channel(seek).
**                  p_rds_cond: RDS search oncdition, can not combine with
**                              BTA_FM_PRESET_SCAN mode.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmComboSearch(UINT16 start_freq, UINT16 end_freq, UINT8 rssi,
                      tBTA_FM_SCAN_DIR direction, tBTA_FM_SCAN_METHOD scn_method,
                      BOOLEAN multi_chnl, tBTA_FM_SCH_RDS_COND * p_rds_cond);

/*******************************************************************************
**
** Function         BTA_FmSetRdsRbds
**
** Description      Choose the RDS mode - RDS/RBDS.
**
** Parameters       rds_type: BTA_FM_RDS or BTA_FM_RBDS.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmSetRdsRbds(tBTA_FM_RDS_B  rds_type);

/*******************************************************************************
**
** Function         BTA_FmSetSignalNotifSetting
**
** Description      Configure RSSI value polling interval.
**
** Parameters       poll_interval: polling interval in milli second unit.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern tBTA_FM_STATUS BTA_FmSetSignalNotifSetting(INT32 poll_interval);

/*******************************************************************************
**
** Function         BTA_FmConfigBlendSoftMuteParams
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
BTA_API extern void BTA_FmConfigBlendSoftMuteParams(tBTA_FM_BLEND_MUTE *p_config);

/*******************************************************************************
**
** Function         BTA_FmVolumeControl
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
BTA_API extern void BTA_FmVolumeControl(UINT16 volume);


#ifdef __cplusplus
}
#endif

#endif /* BTA_FM_API_H */
