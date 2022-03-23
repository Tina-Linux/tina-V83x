/*****************************************************************************
**
**  Name:           bta_fmtx_api.h
**
**  Description:    This is the public interface file for the FMTX subsystem of
**                  BTA, Broadcom's Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2006, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_FMTX_API_H
#define BTA_FMTX_API_H

#include "bta_api.h"
#include "bta_rdse_api.h"
#include "bta_fm_hwhdr.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
#ifndef BTA_FMTX_INCLUDED
#define BTA_FMTX_INCLUDED       TRUE
#endif

/* Extra Debug Code */
#ifndef BTA_FMTX_DEBUG
#define BTA_FMTX_DEBUG           FALSE
#endif

#ifndef BTA_FMTX_CHNLN_PER_SCAN
#define BTA_FMTX_CHNLN_PER_SCAN     14
#endif

/* FMTX callback events */
enum
{
    BTA_FMTX_ENABLE_EVT,          /* 1 BTA FMTX is enabled */
    BTA_FMTX_DISABLE_EVT,         /* 2 BTA FMTX is disabled */
    BTA_FMTX_POWER_EVT,           /* 3 BTA FMTX is powered on/off */
    BTA_FMTX_SET_FREQ_EVT,        /* 4 New FMTX frequency is set */
    BTA_FMTX_CFG_EVT,             /* 5 set band region */
    BTA_FMTX_MUTE_EVT,            /* 6 mute/unmute audio */
    BTA_FMTX_INTF_EVT,            /* 7 interference level notification */
    BTA_FMTX_BCHNL_EVT,           /* 8 best channel found event */
    BTA_FMTX_RDS_INIT_EVT,        /* 9 RDS transmission enabled/disbaled */
    BTA_FMTX_RDS_UPD_EVT,         /* 10 RDS update event */
    BTA_FMTX_RDS_CMPL_EVT,        /* 11 RDS raw data complete event */
    BTA_FMTX_CHIRP_EVT            /* 12 audio chirp set event */
};
typedef UINT8  tBTA_FMTX_EVT;
#define BTA_FMTX_MAX_EVT        BTA_FMTX_RDS_CMPL_EVT
#define BTA_FMTX_KICK_OFF_EVT		0x80	/* BTA FMTX internal event complete bit */
#define BTA_FMTX_EVT_MASK           0x0f    /* callback event mask */

/* FMTX operation status code */
enum
{
    BTA_FMTX_OK,                /* OK, succeed */
    BTA_FMTX_ERR,               /* general error */
    BTA_FMTX_ERR_VCMD,          /* vendor specific command error */
    BTA_FMTX_ERR_FREQ,          /* invalid frequency error */
    BTA_FMTX_ERR_FLAG_TOUT,     /* interrupt event timeout error */
    BTA_FMTX_ERR_BUSY           /* Operation fails for FMTX state machine busy */
};
typedef UINT8   tBTA_FMTX_STATUS;

/* FMTX output power level: fine tune type */
#define BTA_FMTX_PWR_LVL_MAX    24      /* Max power level 24dB */
typedef UINT8   tBTA_FMTX_PWR_LVL;

/* FMTX output audio mode */
#define     BTA_FMTX_AUD_MONO       I2C_FMTX_MONO_ON
#define     BTA_FMTX_AUD_STEREO     I2C_FMTX_STEREO_ON
typedef UINT8   tBTA_FMTX_AUD_MODE;

/* FMTX output volumn level */
enum
{
    BTA_FMTX_VOL_HI,        /*  2dB */
    BTA_FMTX_VOL_MED,       /* 0 dB */
    BTA_FMTX_VOL_LOW,       /* -2 dB */
    BTA_FMTX_VOL_WEAK       /* -4 dB */
};
typedef UINT8 tBTA_FMTX_VOL_LVL;

/* band region selection */
enum
{
    BTA_FMTX_REG_US,         /* US*/
    BTA_FMTX_REG_JAPAN,      /* Japan */
    BTA_FMTX_REG_EUR         /*  Europe */
};
typedef UINT8  tBTA_FMTX_REGION;

#define BTA_FMTX_REG_MAX    BTA_FMTX_REG_EUR

/* Bandwidth type */
enum
{
    BTA_FMTX_BW_50,     /* 50kHz */
    BTA_FMTX_BW_100,    /* 100 kHz */
    BTA_FMTX_BW_200     /* 200 kHz */
};
typedef UINT8 tBTA_FMTX_BW_TYPE;

/* de-emphasis type */
#define     BTA_FMTX_PREEMP_OFF     I2C_FMTX_PREEMPH_OFF
#define     BTA_FMTX_PREEMP_50      I2C_FMTX_PREEMPH_50
#define     BTA_FMTX_PREEMP_75      I2C_FMTX_PREEMPH_75
typedef UINT8 tBTA_FMTX_PREEMP;

/* audio path type */
enum
{
    BTA_FMTX_AUD_I2S,    /*  I2S */
    BTA_FMTX_AUD_ADC,    /*  ADC */
    BTA_FMTX_AUD_LIGHT_STACK     /*  audio path will be set by light stack */
};
typedef UINT8 tBTA_FMTX_AUD_PATH;

/* FMTX configuration types */
enum
{
    BTA_FMTX_CFG_TP_PWR,        /* FMTX output power */
    BTA_FMTX_CFG_TP_AUD_MODE,   /* FMTX output audio mode */
    BTA_FMTX_CFG_TP_VOL,        /* FMTX output volumn */
    BTA_FMTX_CFG_TP_REG,        /* FMTX band region */
    BTA_FMTX_CFG_TP_BW,         /* FMTX bandwidth */
    BTA_FMTX_CFG_TP_PREEMP,       /* FMTX pre-emphasis */
    BTA_FMTX_CFG_TP_AUD_PATH,   /* set FMTX audio path */

    BTA_FMTX_CFG_TP_MAX
};
typedef UINT8   tBTA_FMTX_CFG_TYPE;

/* FMTX configuration values */
typedef union
{
    tBTA_FMTX_PWR_LVL       pwr_level;      /* FMTX output power, valid range 0 ~ 24dB */
    tBTA_FMTX_AUD_MODE      aud_mode;       /* FMTX output audio mode */
    tBTA_FMTX_VOL_LVL       vol_level;      /* FMTX output volumn */
    tBTA_FMTX_REGION        region;         /* FMTX band region */
    tBTA_FMTX_BW_TYPE       bw;             /* FMTX bandwidth */
    tBTA_FMTX_PREEMP        preemph;        /* FMTX pre-emphasis */
    tBTA_FMTX_AUD_PATH      aud_path;       /* FMTX audio path: I2S, ADC, or set by light stack */
}tBTA_FMTX_CFG_DATA;

/* FMTX configuration values */
#define BTA_FMTX_MUTE       I2C_FMTX_MUTE_BOTH
#define BTA_FMTX_UNMUTE     I2C_FMTX_UNMUTE
#define BTA_FMTX_MUTE_L     I2C_FMTX_MUTE_L
#define BTA_FMTX_MUTE_R     I2C_FMTX_MUTE_R
typedef UINT8 tBTA_FMTX_MUTE_TYPE;

#define BTA_FMTX_NOTIF_NONE_BIT     0x00    /* none */
#define BTA_FMTX_NOTIF_RSSI_BIT     0x01    /* bit0 */
#define BTA_FMTX_NOTIF_AUD_LVL_BIT  0x02    /* bit1 */
typedef UINT8 tBTA_FMTX_NOTIF_MASK;

/* audio channel status */
#define BTA_FMTX_CHNL_BOTH_LO        0x00
#define BTA_FMTX_CHNL_RITE_HI        0x01
#define BTA_FMTX_CHNL_LEFT_HI        0x02
#define BTA_FMTX_CHNL_BOTH_HI        0x03
#define BTA_FMTX_CHNL_INVALID        0xff
typedef UINT8 tBTA_FMTX_CHNL_STATUS;

/* RDS group type version */
enum
{
    BTA_FMTX_RDS_GTYPE_A,
    BTA_FMTX_RDS_GTYPE_B
};
typedef UINT8   tBTA_FMTX_RDS_GTYPE;

/* PS scroll rate used for RDS AUTO mode */
enum
{
    BTA_FMTX_SCRR_0_5,     /* 500 ms per scroll    */
    BTA_FMTX_SCRR_1_0,     /* 1 second per scroll  */
    BTA_FMTX_SCRR_1_5,     /* 1.5 seconds per scroll*/
    BTA_FMTX_SCRR_2_0,     /* 2 seconds per scroll*/
    BTA_FMTX_SCRR_2_5,     /* 2.5 seconds per scroll*/
    BTA_FMTX_SCRR_3_0,     /* 3 seconds per scroll*/
    BTA_FMTX_SCRR_3_5,     /* 3.5 seconds per scroll*/
    BTA_FMTX_SCRR_4_0,     /* 4 seconds per scroll*/
    BTA_FMTX_SCRR_4_5,     /* 4.5 seconds per scroll*/
    BTA_FMTX_SCRR_5_0,    /* 5 seconds per scroll*/
    BTA_FMTX_SCRR_5_5,    /* 5.5 seconds per scroll*/
    BTA_FMTX_SCRR_6_0,    /* 6 seconds per scroll*/
    BTA_FMTX_SCRR_6_5,    /* 6.5 seconds per scroll*/
    BTA_FMTX_SCRR_7_0,    /* 7 seconds per scroll*/
    BTA_FMTX_SCRR_7_5     /* 7.5 seconds per scroll */
};
typedef UINT8 tBTA_FMTX_SCR_RATE;

#define BTA_FMTX_RDS_PS_MAX         16

/* program service configuration parameter in RDS auto mode */
typedef struct
{
    UINT8               ps_str[BTA_FMTX_RDS_PS_MAX];
    UINT8               ps_len;     /* program service length */
    tBTA_FMTX_RDS_GTYPE grp_type;   /* prefered PS encoding group type */
    tBTA_FMTX_SCR_RATE  scr_rate;
    UINT8               scr_step;
    UINT8               disp_size;

}tBTA_FMTX_RDS_PS;

/* RDS auto mode initialization data */
typedef struct
{
    tBTA_FMTX_RDS_PS    ps;         /* program service */
    UINT32              pty;        /* program type */
    UINT16              pi_code;    /* PI code */
}tBTA_FMTX_RDS_INIT;

enum
{
    BTA_FMTX_RDS_RPTOR_SI,
    BTA_FMTX_RDS_RPTOR_SO,
    BTA_FMTX_RDS_RPTOR_LS2,
    BTA_FMTX_RDS_RPTOR_UNKNOW
};
typedef UINT8 tBTA_FMTX_RDS_RPTOR;

/* RDS mode type supported in BRCM FMTX mode */
enum
{
    BTA_FMTX_RDS_MODE_OFF,          /* RDS mode off */
    BTA_FMTX_RDS_MODE_AUTO,         /* RDS auto mode */
    BTA_FMTX_RDS_MODE_MANU          /* RDS manual mode */
};
typedef UINT8   tBTA_FMTX_RDS_MODE;

#define BTA_FMTX_RDS_RT_MAX         64

/* radio text updating data structure in RDS auto mode */
typedef struct
{
    UINT8                   rt_str[BTA_FMTX_RDS_RT_MAX];   /* RT string */
    UINT8                   rt_len;     /* length of Radio Text */
    tBTA_FMTX_RDS_GTYPE     grp_type;   /* prefered RT encoding type */
}tBTA_FMTX_RDS_RT;

/* RDS data updating data structure used in RDS auto mode */
typedef union
{
    tBTA_FMTX_RDS_RT    rt;     /* radio text */
    tBTA_FMTX_RDS_PS    ps;     /* program service name */
    UINT16              af;     /* alternative frequency */
    UINT16              pi_code;/* PI code */
    tBTA_FMTX_RDS_RPTOR rptor;  /* character repertoire set */
    UINT8               pty;    /* program type */

}tBTA_FMTX_RDS_DATA;

/* RDS auto mode updating, RDS data type */
enum
{
    BTA_FMTX_RDST_RT,           /* radio text */
    BTA_FMTX_RDST_PS,           /* program service name */
    BTA_FMTX_RDST_AF,           /* alternative frequency */
    BTA_FMTX_RDST_PI,           /* PI code      */
    BTA_FMTX_RDST_RPTOR,        /* character repertoire set */
    BTA_FMTX_RDST_PTY,           /* program type */
    BTA_FMTX_RDST_MAX
};
typedef UINT8 tBTA_FMTX_RDS_TYPE;

/* audio chirp mode */
enum
{
    BTA_FMTX_CHIRP_OFF,
    BTA_FMTX_CHIRP_MANU,
    BTA_FMTX_CHIRP_AUTO
};
typedef UINT8 tBTA_FMTX_CHIRP_MODE;

/* audio chirp configuration */
typedef struct
{
    UINT16   l_magntd;  /* left channel chirp maganitude */
    UINT16   r_magntd;  /* right channel chirp magnitude */
    UINT16   l_fc;      /* left channel chirp frequency */
    UINT16   r_fc;      /* right channel chirp frequency */
    UINT16   l_mrk;     /* left channel mark time in ms for chirp */
    UINT16   r_mrk;     /* mark time in ms for the right channel */
    UINT16   l_spc;     /* space time in ms for the left channel */
    UINT16   r_spc;     /* space time in ms for the right channel */
}tBTA_FMTX_CHIRP_CFG;

/* possible operation subcode */
typedef union
{
    tBTA_FMTX_CFG_TYPE  cfg_type;       /* configuration type */
    tBTA_FMTX_RDS_TYPE  upd_type;       /* RDS updating type */
    tBTA_FMTX_RDS_MODE  mode;           /* RDS mode */
} tBTA_FMTX_SUB_OP;


typedef struct
{
    UINT16              freq;
    UINT8               rssi;
} tBTA_FMTX_BCHNL_INFO;

typedef struct
{
    UINT16                  num_chnl;
    tBTA_FMTX_BCHNL_INFO    *chnl_list;
}tBTA_FMTX_BCHNL;

typedef struct
{
    UINT8                   rssi;
    tBTA_FMTX_CHNL_STATUS   chnl_status;
}tBTA_FMTX_NOTIF;

typedef union
{
    tBTA_FMTX_BCHNL         bchnl;          /* BTA_FMTX_BCHNL_EVT */
    UINT16                  chnl_freq;      /* BTA_FMTX_SET_FREQ_EVT */
    BOOLEAN                 power_on;       /* BTA_FMTX_POWER_EVT */
    tBTA_FMTX_SUB_OP        op_code;        /* BTA_FMTX_CFG_EVT,
                                                BTA_FMTX_RDS_UPD_EVT */
    tBTA_FMTX_NOTIF         notif;           /* BTA_FMTX_INTF_EVT */
} tBTA_FMTX_CBDATA;

/* Union of all FM callback structures */
typedef struct
{
    tBTA_FMTX_STATUS         status;             /* operation status*/
    tBTA_FMTX_CBDATA         data;               /* operation callback data */
} tBTA_FMTX;

/* struct holding audio configuration for audio level detection algorithm */
typedef struct
{
    UINT8       aud_thresh_hi;
    UINT8       aud_thresh_lo;
    UINT8       aud_count_hi;
    UINT8       aud_count_lo;
    UINT8       aud_count_meas;
} tBTA_FMTX_AUD_CFG;

typedef struct
{
    INT32               enable_delay_val; /* FMTX enable delay timer */
    INT32               intf_timer_val;   /* Interference notification timer value */
    INT32               scan_timer_val;   /* RSSI scan timer value */
    tBTA_FMTX_REGION    region;     /* band region */
    tBTA_FMTX_AUD_CFG   *p_audio_cfg;    /* audio configuration for audio level detection algo */

    UINT16              pi_code;    /* PI code */
    UINT8               pty;        /* program type */
}tBTA_FMTX_CFG;

/* BTA FMTX callback function */
typedef void (tBTA_FMTX_CBACK)(tBTA_FMTX_EVT event, tBTA_FMTX *p_data);

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_FmtxEnable
**
** Description      Enable FMTX fuctionality.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxEnable(tBTA_FMTX_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_FmtxDisable
**
** Description      Disable FMTX functionality.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxDisable(void);

/*******************************************************************************
**
** Function         BTA_FmtxPower
**
** Description      Turn on/off FMTX power.
**
** Parameters       BOOLEAN: turn on or off:
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxPower(BOOLEAN pwr_on);

/*******************************************************************************
**
** Function         BTA_FmtxSetFreq
**
** Description      Tune FMTX to a designated frequency.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxSetFreq(UINT16 freq);

/*******************************************************************************
**
** Function         BTA_FmtxConfig
**
** Description      Configure FMTX region related settings including:
**                  power level, audio output mode, region, bandwidth, preemphasis,
**                  volume settings.
**
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxConfig(tBTA_FMTX_CFG_TYPE cfg_type, tBTA_FMTX_CFG_DATA  cfg_data);

/*******************************************************************************
**
** Function         BTA_FmtxMute
**
** Description      Mute/Unmute FMTX audio.
**
**
** Parameters       mute: unmute, mute
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxMute(tBTA_FMTX_MUTE_TYPE mute);
/*******************************************************************************
**
** Function         BTA_FmtxGetIntfNotif
**
** Description      Get a constant notification on FMTX channel interference level.
**
**
** Parameters       notif: It's bit mask indicating which notification is requested.
**                  BTA_FMTX_NOTIF_RSSI_BIT: request channel RSSI notification.
**                  BTA_FMTX_NOTIF_AUD_LVL_BIT: request audio level status notification.
**                  BTA_FMTX_NOTIF_NONE_BIT: disable notification events.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxGetIntfNotif(tBTA_FMTX_NOTIF_MASK  notif);

/*******************************************************************************
**
** Function         BTA_FmtxBestChnls
**
** Description      Find the least interfered channels for FMTX usage.
**
**
** Parameters       num_chnl: number of best channels requested.
**                  optimize: if FASLE, scan full FM band at one time;
**                            if TRUE, scan BTA_FMTX_CHNLN_PER_SCAN channels at
**                                  one time, and full band will be covered
**                                  with multiple scans with intervals in between.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxBestChnls(UINT8 num_chnl, BOOLEAN optimize);
/*******************************************************************************
**
** Function         BTA_FmtxAbortChnlScan
**
** Description      Abort the best channel scan.
**
**
** Parameters       None.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxAbortChnlScan(void) ;
/*******************************************************************************
**
** Function         BTA_FmtxRdsInit
**
** Description      Enable/Disable the RDS transmission at auto or manual mode
**                  with initialized RDS data.
**
**
** Parameters       mode:  off, auto or manual mode.
**                  rds_init : inital RDS data including PI code, PS and PTY.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxRdsInit(tBTA_FMTX_RDS_MODE  mode,
                                    tBTA_FMTX_RDS_INIT *p_rds_init);

/*******************************************************************************
**
** Function         BTA_FmtxRdsAutoUpdate
**
** Description      Update RDS data in RDS auto mode. Updating data can be: RT,
**                  PS, AF, PI code and character repertoire settings.
**
**
** Parameters       rds_type    : RT,PS, AF, PI code and character repertoire.
**                  data        : rds data to be updated.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxRdsAutoUpdate(tBTA_FMTX_RDS_TYPE rds_type,
                                          tBTA_FMTX_RDS_DATA * p_data);


/*******************************************************************************
**
** Function         BTA_FmtxRdsRawData
**
** Description      Feed FMTX with RDS data in RDS manual mode.
**
**
** Parameters       len     : length of data
**                  *p_data  : pointer to the encoded RDS data stream
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxRdsRawData(UINT16 len, UINT8 *p_data);

/*******************************************************************************
**
** Function         BTA_FmtxAudioChirp
**
** Description      This function enable/disable audio chirp in different modes.
**                  Feature avaialble for 4329B0/2049B0 or newer chip.
**
**
** Parameters       mode: audio chirp operating mode, can be auto, manual or off mode.
**                        if mode is
**                        BTA_FMTX_CHIRP_AUTO: chirp will start whenever FMTX audio
**                                      level is low; and stop when audio level
**                                      goes back high.
**                        BTA_FMTX_CHIRP_MANU: manually command start chirping now.
**                        BTA_FMTX_CHIRP_OFF:  stop audio chirp.
**
**                  p_cfg: pointer to the chirp configuration, if NULL is used,
**                         default chirp configuration will be used.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmtxAudioChirp(tBTA_FMTX_CHIRP_MODE mode, tBTA_FMTX_CHIRP_CFG *p_cfg);


#ifdef __cplusplus
}
#endif

#endif /* BTA_FMTX_API_H */
