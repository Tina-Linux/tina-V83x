/*****************************************************************************
**
**  Name:           bsa_rds_api.h
**
**  Description:    This is the public interface file for the RDS decoder.
**
**  Copyright (c) 2006, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BSA_RDS_API_H
#define BSA_RDS_API_H

#include "bsa_api.h"
#include "bsa_fm_api.h"
/*****************************************************************************
**  Constants and data types
*****************************************************************************/

#ifndef BSA_RDS_DEBUG
#define BSA_RDS_DEBUG    TRUE
#endif

/* Maxinum number of different RDS streams supported by the decoder */
#ifndef BSA_RDS_MAX_STREAM
#define BSA_RDS_MAX_STREAM      1
#endif

/* RDS Decoder structure */
#define BSA_RDS_PI_BIT           (1<<0)
#define BSA_RDS_PTY_BIT          (1<<1)
#define BSA_RDS_TP_BIT           (1<<2)
#define BSA_RDS_DI_BIT           (1<<3)
#define BSA_RDS_MS_BIT           (1<<4)
#define BSA_RDS_AF_BIT           (1<<5)
#define BSA_RDS_PS_BIT           (1<<6)
#define BSA_RDS_PTYN_BIT         (1<<7)
#define BSA_RDS_RT_BIT           (1<<8)
#define BSA_RDS_PIN_BIT          (1<<9)
#define BSA_RDS_CT_BIT           (1<<10)
#define BSA_RDS_ODA_BIT          (1 <<11)
#define BSA_RDS_TDC_BIT          (1 <<12)
#define BSA_RDS_EWS_BIT          (1 << 13)
#define BSA_RDS_SLC_BIT          (1 << 14)
#define BSA_RDS_EON_BIT          (1 << 15)
#define BSA_RDS_TMC_BIT          (1 << 16)
#define BSA_RDS_RTP_BIT          (1 << 17)

typedef UINT32  tBSA_RDS_FEATURE;


/* RDS decoder event */
enum
{
    BSA_RDS_REG_EVT,        /* RDS decoder user register event*/
    BSA_RDS_PI_EVT,         /* PI code */
    BSA_RDS_PTY_EVT,        /* program type */
    BSA_RDS_TP_EVT,         /* traffic program identification */
    BSA_RDS_DI_EVT,         /* Decoder Identification */
    BSA_RDS_MS_EVT,         /* music speech content */
    BSA_RDS_AF_EVT,         /* Alternative Frequency */
    BSA_RDS_PS_EVT,         /* Program service name */
    BSA_RDS_PTYN_EVT,       /* program type name */
    BSA_RDS_RT_EVT,         /* Radio Text */
    BSA_RDS_PIN_EVT,        /* program Item number */
    BSA_RDS_CT_EVT,         /* clock time */
    BSA_RDS_ODA_EVT,        /* open data application */
    BSA_RDS_TDC_EVT,        /* transparent data channel */
    BSA_RDS_EWS_EVT,        /* emergency warning system */
    BSA_RDS_SLC_EVT,         /* slow labelling code */
    BSA_RDS_RPTOR_EVT,      /* charcter repertoire set */
    BSA_RDS_EON_EVT,        /* EON information, should be evalute with sub code */
    BSA_RDS_TMC_EVT,        /* Traffic Message Channel */
    BSA_RDS_RTP_EVT,        /* RT plus event */
    BSA_RDS_STATS_EVT
};
typedef UINT32   tBSA_RDS_EVT;


/* RDS status code */
enum
{
    BSA_RDS_OK,
    BSA_RDS_ERR,            /* general error */
    BSA_RDS_ERR_DAT,        /* error RDS data */
    BSA_RDS_ERR_INTL,       /* internal error */
    BSA_RDS_ERR_BAD_AID     /* bad application ID error */
};
typedef UINT8 tBSA_RDS_STATUS;

/* TP/TA flag */
#define BSA_RDS_TP_NONE         0x00    /* TP, TA all 0 */
#define BSA_RDS_TP_EON          0x10    /* TA 1; TP 0, no traffic serrvice, available on EON*/
#define BSA_RDS_TP_AVAIL        0x01    /* TA 0; TP 1: traffic service available */
#define BSA_RDS_TP_ACTIVE       0x11    /* TA: 1 TP 1: traffic service active now */
#define BSA_RDS_TP_INVAL        0xff    /* invalid TP/TA code */

typedef UINT8  tBSA_FM_RDS_TP_TYPE;

/* PTY definition, RDS set */
enum
{
    BSA_RDS_PTY_NONE,   /* 0    */
    BSA_RDS_PTY_NEWS,   /* 1    */
    BSA_RDS_PTY_AFIRS,  /* 2    */
    BSA_RDS_PTY_INFO,   /* 3    */
    BSA_RDS_PTY_SPRT,   /* 4    */
    BSA_RDS_PTY_EDCT,   /* 5    */
    BSA_RDS_PTY_DRMA,   /* 6    */
    BSA_RDS_PTY_CLTR,   /* 7    */
    BSA_RDS_PTY_SCI,    /* 8    */
    BSA_RDS_PTY_VARI,   /* 9    */
    BSA_RDS_PTY_POP,    /* 10   */
    BSA_RDS_PTY_ROCK,   /* 11   */
    BSA_RDS_PTY_EASY,   /* 12   */
    BSA_RDS_PTY_LITE,   /* 13   */
    BSA_RDS_PTY_CLSC,   /* 14   */
    BSA_RDS_PTY_OTHR,   /* 15   */
    BSA_RDS_PTY_WTHR,   /* 16   */
    BSA_RDS_PTY_FINC,   /* 17   */
    BSA_RDS_PTY_CHLD,   /* 18   */
    BSA_RDS_PTY_SOCL,   /* 19   */
    BSA_RDS_PTY_RELG,   /* 20   */
    BSA_RDS_PTY_FONE,   /* 21   */
    BSA_RDS_PTY_TRVL,   /* 22   */
    BSA_RDS_PTY_LESR,   /* 23   */
    BSA_RDS_PTY_JAZZ,   /* 24   */
    BSA_RDS_PTY_CNTY,   /* 25   */
    BSA_RDS_PTY_NATI,   /* 26   */
    BSA_RDS_PTY_OLDI,   /* 27   */
    BSA_RDS_PTY_FOLK,   /* 28   */
    BSA_RDS_PTY_DOC,    /* 29   */
    BSA_RDS_PTY_TEST,   /* 30   */
    BSA_RDS_PTY_ALRM    /* 31   */
};
/* PTY definition, RBDS set */
enum
{
    BSA_RBDS_PTY_NONE,   /* 0    */
    BSA_RBDS_PTY_NEWS,   /* 1    */
    BSA_RBDS_PTY_INFM,   /* 2    */
    BSA_RBDS_PTY_SPRT,   /* 3    */
    BSA_RBDS_PTY_TALK,   /* 4    */
    BSA_RBDS_PTY_ROCK,   /* 5    */
    BSA_RBDS_PTY_CROCK,  /* 6    */
    BSA_RBDS_PTY_ADLT,   /* 7    */
    BSA_RBDS_PTY_SROCK,  /* 8    */
    BSA_RBDS_PTY_TOP,    /* 9    */
    BSA_RBDS_PTY_CNTY,   /* 10   */
    BSA_RBDS_PTY_OLDI,   /* 11   */
    BSA_RBDS_PTY_SOFT,   /* 12   */
    BSA_RBDS_PTY_NSTG,   /* 13   */
    BSA_RBDS_PTY_JAZZ,   /* 14   */
    BSA_RBDS_PTY_CLCS,   /* 15   */
    BSA_RBDS_PTY_RNB,    /* 16   */
    BSA_RBDS_PTY_SRNB,   /* 17   */
    BSA_RBDS_PTY_LAGU,   /* 18   */
    BSA_RBDS_PTY_RELM,   /* 19   */
    BSA_RBDS_PTY_RELT,   /* 20   */
    BSA_RBDS_PTY_PERS,   /* 21   */
    BSA_RBDS_PTY_PBLK,   /* 22   */
    BSA_RBDS_PTY_COLL,   /* 23   */
    BSA_RBDS_PTY_UN24,   /* 24   */
    BSA_RBDS_PTY_UN25,   /* 25   */
    BSA_RBDS_PTY_UN26,   /* 26   */
    BSA_RBDS_PTY_UN27,   /* 27   */
    BSA_RBDS_PTY_UN28,   /* 28   */
    BSA_RBDS_PTY_WTHR,   /* 29   */
    BSA_RBDS_PTY_TEST,   /* 30   */
    BSA_RBDS_PTY_ALER,   /* 31   */
    BSA_RDS_PTY_UNDEF    /* 32   */
};
typedef UINT8   tBSA_RDS_PTY_VAL;


/* PTY */
typedef struct
{
    tBSA_RDS_PTY_VAL    pty_val;    /* PTY  value */
    UINT8               *p_str;  /* PTY display string */
} tBSA_RDS_PTY;

/* CT data */
typedef struct
{
    UINT32  day;
    UINT8   hour;
    UINT8   minute;
    UINT8   sense;
    UINT8   offset;
} tBSA_RDS_CT;

/* PIN data */
typedef struct
{
    UINT8   day;
    UINT8   hour;
    UINT8   minute;
} tBSA_RDS_PIN;

/* music speech content type */
enum
{
    BSA_RDS_SPEECH = 1,
    BSA_RDS_MUSIC
};
typedef UINT8 tBSA_RDS_M_S;

/* transprant data channell structure */
typedef struct
{
    UINT8       chnl_num;   /* channel number */
    UINT8       len;        /* data length */
    UINT8       tdc_seg[4]; /* TDC data, max to 4 bytes */

}tBSA_RDS_TDC_DATA;

/* TMC coding Protocol types corresponding Application ID */
#define BSA_RDS_TMC_ALERT         0xCD46
#define BSA_RDS_TMC_ALERT_PLUS    0x4B02

#define BSA_RDS_RT_PLUS_AID       0x4BD7

/* segment data type, used for EWS, and ODA */
typedef struct
{
    UINT8               data_5b;        /* the 5 bits data carried in 2nd block */
    UINT16              data_16b_1;     /* 16 bits data carried in block 3 if group type A */
    UINT16              data_16b_2;     /* 16 bits data carried in block 4 */
} tBSA_RDS_SEG_DATA;

/* ODA */
typedef struct
{
    UINT16              aid;        /* ODA application ID */
    UINT8               grp_type;   /* group type which is carrying this AID */
    union
    {
        tBSA_RDS_SEG_DATA       msg_body;    /* ODA message information */
        UINT16                  sys_info;    /* ODA system information carried in group 3A block C */
    }                           data_info;
}tBSA_RDS_ODA_DATA;

/* slow labelling code type */
enum
{
    BSA_RDS_SLC_ECC,            /* enhanced country code */
    BSA_RDS_SLC_TMC,
    BSA_RDS_SLC_PID,
    BSA_RDS_SLC_LAG,
    BSA_RDS_SLC_UNASSIGNED,
    BSA_RDS_SLC_BRCST,
    BSA_RDS_SLC_EWS
};
typedef UINT8   tBSA_RDS_SLC_TYPE;

typedef struct
{
    UINT8           paging;
    UINT8           ecc_code;
}tBSA_RDS_ECC;  /* ECC code */
/* Slow labelling Code data */
typedef union
{
    tBSA_RDS_ECC        ecc;            /* ECC code */
    UINT16              language;       /* language code */
    UINT16              tmc_id;         /* TMC ID */
    UINT16              paging_id;      /* paging ID */
    UINT16              ews_channel;    /* EWS channel ID */
    UINT16              other;           /* broadcaster data */
}  tBSA_RDS_SLC ;

/* Slow labelling Code call back data*/
typedef struct
{
    tBSA_RDS_SLC_TYPE   slc_type;
    tBSA_RDS_SLC        data;

}tBSA_RDS_SLC_DATA;


enum
{
    BSA_RDS_RPTOR_SI,
    BSA_RDS_RPTOR_SO,
    BSA_RDS_RPTOR_LS2,
    BSA_RDS_RPTOR_UNKNOW
};
typedef UINT8 tBSA_RDS_RPTOR_SET;

enum
{
    BSA_RDS_EON_PS,         /* EON PS */
    BSA_RDS_EON_AF,         /* EON AF method-A AF list */
    BSA_RDS_EON_MAP_FREQ,   /* EON mapped AF frequency */
    BSA_RDS_EON_PIN,        /* EON PIN info */
    BSA_RDS_EON_PTY,        /* EON PTY informtion */
    BSA_RDS_EON_TA,         /* TA information from 14A group, information only */
    BSA_RDS_EON_TA_B,       /* TA/TP information from group 14B */
    BSA_RDS_EON_LINK        /* EON linkage information */
};
typedef UINT8 tBSA_RDS_EON_CODE;

typedef struct
{
    UINT16  tn_freq;
    UINT16  on_freq;
}tBSA_RDS_MAP_FREQ;

typedef struct
{
    UINT8   la;     /* Linkage Actuator */
    UINT8   eg;     /* Extended Generic indicator */
    UINT8   ils;    /* Internaltional Linkage Set Indicator */
    UINT8   lsn_ci;    /* Linkage Set Number : contry identifier */
    UINT8   lsn_li;    /* Linkage Set Number: linkage identifier */

}tBSA_RDS_LINK_DATA;

/* AF encoding method */
enum
{
    BSA_RDS_AF_M_U, /* unknown */
    BSA_RDS_AF_M_A, /* method - A */
    BSA_RDS_AF_M_B  /* method - B */
};
typedef UINT8 tBSA_RDS_AF_M_TYPE;

typedef struct
{
    tBSA_FM_AF_LIST         list;
    tBSA_RDS_AF_M_TYPE      method; /* method-A or method-B */
                                    /* when method-B is used, the tuning frequency, for which
                                  the associated Af list is valid, will be thelist.af_list[0] */

}tBSA_RDS_AF_LIST;

typedef struct
{
    UINT16                  pi_eon;
    tBSA_RDS_EON_CODE       sub_code;
    union
    {
        UINT8               *p_data;    /* PS, PTY */
        tBSA_FM_RDS_TP_TYPE ta_tp;
        UINT8               ta;         /* TA flag */
        tBSA_RDS_AF_LIST    af_data;    /* AF */
        tBSA_RDS_PIN        pin;        /* PIN */
        tBSA_RDS_MAP_FREQ   map_freq;
        tBSA_RDS_LINK_DATA  linkage;    /* Linkage information */
    }data;
}tBSA_RDS_EON_DATA;

#ifndef BSA_RDS_RTP_TAG_MAX
#define BSA_RDS_RTP_TAG_MAX     6
#endif

typedef struct
{
    UINT8               content_type;
    UINT8               start;
    UINT8               len;
} tBSA_RDS_RTP_TAG;

typedef struct
{
    BOOLEAN             running;
    UINT8               tag_toggle;
    UINT8               tag_num;
    tBSA_RDS_RTP_TAG    tag[BSA_RDS_RTP_TAG_MAX];
} tBSA_RDS_RTP;

typedef struct
{
    UINT8           *p_data;
    UINT8           g_type;
    BOOLEAN         complete;
} tBSA_RDS_RT;

/* RDS callback data */
typedef union
{
    tBSA_RDS_CT         ct;             /* Clock time */
    tBSA_RDS_PIN        pin;            /* Program Item Number */
    tBSA_RDS_STATUS     status;         /* status */
    tBSA_RDS_AF_LIST    af_data;        /* AF */
    tBSA_RDS_PTY        pty;            /* PTY callback data */
    UINT16              pi_code;       /* PI code */
    UINT8               data8;         /* UINT8 data, used for DI callback */
    tBSA_FM_RDS_TP_TYPE ta_tp;          /* TP/TA status */
    UINT8               *p_data;        /* data pointer to UINT8, used for PS, PTYN */
    tBSA_RDS_RPTOR_SET  rptor_set;
    tBSA_RDS_M_S        m_s;            /* music speech feature */
    tBSA_RDS_ODA_DATA   oda;            /* Open data application data, TMC */
    tBSA_RDS_TDC_DATA   tdc_data;       /* transparent data channel data */
    tBSA_RDS_SEG_DATA   seg_data;       /* EWS data */
    tBSA_RDS_SLC_DATA   slc;            /* slow labelling codes */
    tBSA_RDS_EON_DATA   eon;            /* EON data */
    tBSA_RDS_RTP        rtplus;
    UINT32              stats[4];       /* statistics data */
    tBSA_RDS_RT         rt;             /* used for RT */
} tBSA_FM_RDS;

typedef struct
{
    tBSA_RDS_EVT event;
    tBSA_FM_RDS data;
} tBSA_FM_RDS_DATA;

typedef void (tBSA_RDS_CBACK)(tBSA_RDS_EVT event, tBSA_FM_RDS *p_data, UINT8 app_id);
/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BSA_FmRDSResetDecoder
**
** Description      Reset RDS decoder decoding control variable for a new parsing.
**                  Usually this function is called when jumping to a new frequency,
**                  and all the old RDS information should be erased.
**
** Returns          app_id: application ID associate with a RDS stream, it must be a
**                           NONE-ZERO positive number.
**
*******************************************************************************/
tBSA_RDS_STATUS BSA_FmRDSResetDecoder(UINT8 app_id);


#ifdef __cplusplus
}
#endif

#endif /* BSA_RDS_API_H */
