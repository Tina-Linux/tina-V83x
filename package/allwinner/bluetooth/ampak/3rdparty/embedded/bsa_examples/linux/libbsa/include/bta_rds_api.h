/*****************************************************************************
**
**  Name:           bta_rds_api.h
**
**  Description:    This is the public interface file for the RDS decoder.
**
**  Copyright (c) 2006, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_RDS_API_H
#define BTA_RDS_API_H

#include "bta_api.h"
#include "bta_fm_api.h"
/*****************************************************************************
**  Constants and data types
*****************************************************************************/

#ifndef BTA_RDS_DEBUG
#define BTA_RDS_DEBUG    TRUE
#endif

/* Maxinum number of different RDS streams supported by the decoder */
#ifndef BTA_RDS_MAX_STREAM
#define BTA_RDS_MAX_STREAM      1
#endif

/* RDS Decoder structure */
#define BTA_RDS_PI_BIT           (1<<0)
#define BTA_RDS_PTY_BIT          (1<<1)
#define BTA_RDS_TP_BIT           (1<<2)
#define BTA_RDS_DI_BIT           (1<<3)
#define BTA_RDS_MS_BIT           (1<<4)
#define BTA_RDS_AF_BIT           (1<<5)
#define BTA_RDS_PS_BIT           (1<<6)
#define BTA_RDS_PTYN_BIT         (1<<7)
#define BTA_RDS_RT_BIT           (1<<8)
#define BTA_RDS_PIN_BIT          (1<<9)
#define BTA_RDS_CT_BIT           (1<<10)
#define BTA_RDS_ODA_BIT          (1 <<11)
#define BTA_RDS_TDC_BIT          (1 <<12)
#define BTA_RDS_EWS_BIT          (1 << 13)
#define BTA_RDS_SLC_BIT          (1 << 14)
#define BTA_RDS_EON_BIT          (1 << 15)
#define BTA_RDS_TMC_BIT          (1 << 16)
#define BTA_RDS_RTP_BIT          (1 << 17)

typedef UINT32  tBTA_RDS_FEATURE;



/* RDS decoder event */
enum
{
    BTA_RDS_REG_EVT,        /* RDS decoder user register event*/
    BTA_RDS_PI_EVT,         /* PI code */
    BTA_RDS_PTY_EVT,        /* program type */
    BTA_RDS_TP_EVT,         /* traffic program identification */
    BTA_RDS_DI_EVT,         /* Decoder Identification */
    BTA_RDS_MS_EVT,         /* music speech content */
    BTA_RDS_AF_EVT,         /* Alternative Frequency */
    BTA_RDS_PS_EVT,         /* Program service name */
    BTA_RDS_PTYN_EVT,       /* program type name */
    BTA_RDS_RT_EVT,         /* Radio Text */
    BTA_RDS_PIN_EVT,        /* program Item number */
    BTA_RDS_CT_EVT,         /* clock time */
    BTA_RDS_ODA_EVT,        /* open data application */
    BTA_RDS_TDC_EVT,        /* transparent data channel */
    BTA_RDS_EWS_EVT,        /* emergency warning system */
    BTA_RDS_SLC_EVT,         /* slow labelling code */
    BTA_RDS_RPTOR_EVT,      /* charcter repertoire set */
    BTA_RDS_EON_EVT,        /* EON information, should be evalute with sub code */
    BTA_RDS_TMC_EVT,        /* Traffic Message Channel */
    BTA_RDS_RTP_EVT,        /* RT plus event */
    BTA_RDS_STATS_EVT
};
typedef UINT32   tBTA_RDS_EVT;


/* RDS status code */
enum
{
    BTA_RDS_OK,
    BTA_RDS_ERR,            /* general error */
    BTA_RDS_ERR_DAT,        /* error RDS data */
    BTA_RDS_ERR_INTL,       /* internal error */
    BTA_RDS_ERR_BAD_AID     /* bad application ID error */
};
typedef UINT8 tBTA_RDS_STATUS;

/* TP/TA flag */
#define BTA_RDS_TP_NONE         0x00    /* TP, TA all 0 */
#define BTA_RDS_TP_EON          0x10    /* TA 1; TP 0, no traffic serrvice, available on EON*/
#define BTA_RDS_TP_AVAIL        0x01    /* TA 0; TP 1: traffic service available */
#define BTA_RDS_TP_ACTIVE       0x11    /* TA: 1 TP 1: traffic service active now */
#define BTA_RDS_TP_INVAL        0xff    /* invalid TP/TA code */

typedef UINT8  tBTA_FM_RDS_TP_TYPE;

/* PTY definition, RDS set */
enum
{
    BTA_RDS_PTY_NONE,   /* 0    */
    BTA_RDS_PTY_NEWS,   /* 1    */
    BTA_RDS_PTY_AFIRS,  /* 2    */
    BTA_RDS_PTY_INFO,   /* 3    */
    BTA_RDS_PTY_SPRT,   /* 4    */
    BTA_RDS_PTY_EDCT,   /* 5    */
    BTA_RDS_PTY_DRMA,   /* 6    */
    BTA_RDS_PTY_CLTR,   /* 7    */
    BTA_RDS_PTY_SCI,    /* 8    */
    BTA_RDS_PTY_VARI,   /* 9    */
    BTA_RDS_PTY_POP,    /* 10   */
    BTA_RDS_PTY_ROCK,   /* 11   */
    BTA_RDS_PTY_EASY,   /* 12   */
    BTA_RDS_PTY_LITE,   /* 13   */
    BRA_RDS_PTY_CLSC,   /* 14   */
    BTA_RDS_PTY_OTHR,   /* 15   */
    BTA_RDS_PTY_WTHR,   /* 16   */
    BTA_RDS_PTY_FINC,   /* 17   */
    BTA_RDS_PTY_CHLD,   /* 18   */
    BTA_RDS_PTY_SOCL,   /* 19   */
    BTA_RDS_PTY_RELG,   /* 20   */
    BTA_RDS_PTY_FONE,   /* 21   */
    BTA_RDS_PTY_TRVL,   /* 22   */
    BTA_RDS_PTY_LESR,   /* 23   */
    BTA_RDS_PTY_JAZZ,   /* 24   */
    BTA_RDS_PTY_CNTY,   /* 25   */
    BTA_RDS_PTY_NATI,   /* 26   */
    BTA_RDS_PTY_OLDI,   /* 27   */
    BTA_RDS_PTY_FOLK,   /* 28   */
    BTA_RDS_PTY_DOC,    /* 29   */
    BTA_RDS_PTY_TEST,   /* 30   */
    BTA_RDS_PTY_ALRM    /* 31   */
};
/* PTY definition, RBDS set */
enum
{
    BTA_RBDS_PTY_NONE,   /* 0    */
    BTA_RBDS_PTY_NEWS,   /* 1    */
    BTA_RBDS_PTY_INFM,   /* 2    */
    BTA_RBDS_PTY_SPRT,   /* 3    */
    BTA_RBDS_PTY_TALK,   /* 4    */
    BTA_RBDS_PTY_ROCK,   /* 5    */
    BTA_RBDS_PTY_CROCK,  /* 6    */
    BTA_RBDS_PTY_ADLT,   /* 7    */
    BTA_RBDS_PTY_SROCK,  /* 8    */
    BTA_RBDS_PTY_TOP,    /* 9    */
    BTA_RBDS_PTY_CNTY,   /* 10   */
    BTA_RBDS_PTY_OLDI,   /* 11   */
    BTA_RBDS_PTY_SOFT,   /* 12   */
    BTA_RBDS_PTY_NSTG,   /* 13   */
    BTA_RBDS_PTY_JAZZ,   /* 14   */
    BTA_RBDS_PTY_CLCS,   /* 15   */
    BTA_RBDS_PTY_RNB,    /* 16   */
    BTA_RBDS_PTY_SRNB,   /* 17   */
    BTA_RBDS_PTY_LAGU,   /* 18   */
    BTA_RBDS_PTY_RELM,   /* 19   */
    BTA_RBDS_PTY_RELT,   /* 20   */
    BTA_RBDS_PTY_PERS,   /* 21   */
    BTA_RBDS_PTY_PBLK,   /* 22   */
    BTA_RBDS_PTY_COLL,   /* 23   */
    BTA_RBDS_PTY_UN24,   /* 24   */
    BTA_RBDS_PTY_UN25,   /* 25   */
    BTA_RBDS_PTY_UN26,   /* 26   */
    BTA_RBDS_PTY_UN27,   /* 27   */
    BTA_RBDS_PTY_UN28,   /* 28   */
    BTA_RBDS_PTY_WTHR,   /* 29   */
    BTA_RBDS_PTY_TEST,   /* 30   */
    BTA_RBDS_PTY_ALER,   /* 31   */
    BTA_RDS_PTY_UNDEF    /* 32   */
};
typedef UINT8   tBTA_RDS_PTY_VAL;


/* PTY */
typedef struct
{
    tBTA_RDS_PTY_VAL    pty_val;    /* PTY  value */
    UINT8               *p_str;  /* PTY display string */
} tBTA_RDS_PTY;

/* CT data */
typedef struct
{
    UINT32  day;
    UINT8   hour;
    UINT8   minute;
    UINT8   sense;
    UINT8   offset;
} tBTA_RDS_CT;

/* PIN data */
typedef struct
{
    UINT8   day;
    UINT8   hour;
    UINT8   minute;
} tBTA_RDS_PIN;

/* music speech content type */
enum
{
    BTA_RDS_SPEECH = 1,
    BTA_RDS_MUSIC
};
typedef UINT8 tBTA_RDS_M_S;

/* transprant data channell structure */
typedef struct
{
    UINT8       chnl_num;   /* channel number */
    UINT8       len;        /* data length */
    UINT8       tdc_seg[4]; /* TDC data, max to 4 bytes */

}tBTA_RDS_TDC_DATA;

/* TMC coding Protocol types corresponding Application ID */
#define BTA_RDS_TMC_ALERT         0xCD46
#define BTA_RDS_TMC_ALERT_PLUS    0x4B02

#define BTA_RDS_RT_PLUS_AID       0x4BD7

/* segment data type, used for EWS, and ODA */
typedef struct
{
    UINT8               data_5b;        /* the 5 bits data carried in 2nd block */
    UINT16              data_16b_1;     /* 16 bits data carried in block 3 if group type A */
    UINT16              data_16b_2;     /* 16 bits data carried in block 4 */
} tBTA_RDS_SEG_DATA;

/* ODA */
typedef struct
{
    UINT16              aid;        /* ODA application ID */
    UINT8               grp_type;   /* group type which is carrying this AID */
    union
    {
        tBTA_RDS_SEG_DATA       msg_body;    /* ODA message information */
        UINT16                  sys_info;    /* ODA system information carried in group 3A block C */
    }                           data_info;
}tBTA_RDS_ODA_DATA;

/* slow labelling code type */
enum
{
    BTA_RDS_SLC_ECC,            /* enhanced country code */
    BTA_RDS_SLC_TMC,
    BTA_RDS_SLC_PID,
    BTA_RDS_SLC_LAG,
    BTA_RDS_SLC_UNASSIGNED,
    BTA_RDS_SLC_BRCST,
    BTA_RDS_SLC_EWS
};
typedef UINT8   tBTA_RDS_SLC_TYPE;

typedef struct
{
    UINT8           paging;
    UINT8           ecc_code;
}tBTA_RDS_ECC;  /* ECC code */
/* Slow labelling Code data */
typedef union
{
    tBTA_RDS_ECC        ecc;            /* ECC code */
    UINT16              language;       /* language code */
    UINT16              tmc_id;         /* TMC ID */
    UINT16              paging_id;      /* paging ID */
    UINT16              ews_channel;    /* EWS channel ID */
    UINT16              other;           /* broadcaster data */
}  tBTA_RDS_SLC ;

/* Slow labelling Code call back data*/
typedef struct
{
    tBTA_RDS_SLC_TYPE   slc_type;
    tBTA_RDS_SLC        data;

}tBTA_RDS_SLC_DATA;


enum
{
    BTA_RDS_RPTOR_SI,
    BTA_RDS_RPTOR_SO,
    BTA_RDS_RPTOR_LS2,
    BTA_RDS_RPTOR_UNKNOW
};
typedef UINT8 tBTA_RDS_RPTOR_SET;

enum
{
    BTA_RDS_EON_PS,         /* EON PS */
    BTA_RDS_EON_AF,         /* EON AF method-A AF list */
    BTA_RDS_EON_MAP_FREQ,   /* EON mapped AF frequency */
    BTA_RDS_EON_PIN,        /* EON PIN info */
    BTA_RDS_EON_PTY,        /* EON PTY informtion */
    BTA_RDS_EON_TA,         /* TA information from 14A group, information only */
    BTA_RDS_EON_TA_B,       /* TA/TP information from group 14B */
    BTA_RDS_EON_LINK        /* EON linkage information */
};
typedef UINT8 tBTA_RDS_EON_CODE;

typedef struct
{
    UINT16  tn_freq;
    UINT16  on_freq;
}tBTA_RDS_MAP_FREQ;

typedef struct
{
    UINT8   la;     /* Linkage Actuator */
    UINT8   eg;     /* Extended Generic indicator */
    UINT8   ils;    /* Internaltional Linkage Set Indicator */
    UINT8   lsn_ci;    /* Linkage Set Number : contry identifier */
    UINT8   lsn_li;    /* Linkage Set Number: linkage identifier */

}tBTA_RDS_LINK_DATA;

/* AF encoding method */
enum
{
    BTA_RDS_AF_M_U, /* unknown */
    BTA_RDS_AF_M_A, /* method - A */
    BTA_RDS_AF_M_B  /* method - B */
};
typedef UINT8 tBTA_RDS_AF_M_TYPE;

typedef struct
{
    tBTA_FM_AF_LIST         list;
    tBTA_RDS_AF_M_TYPE      method; /* method-A or method-B */
                                    /* when method-B is used, the tuning frequency, for which
                                  the associated Af list is valid, will be thelist.af_list[0] */

}tBTA_RDS_AF_LIST;

typedef struct
{
    UINT16                  pi_eon;
    tBTA_RDS_EON_CODE       sub_code;
    union
    {
        UINT8               *p_data;    /* PS, PTY */
        tBTA_FM_RDS_TP_TYPE ta_tp;
        UINT8               ta;         /* TA flag */
        tBTA_RDS_AF_LIST    af_data;    /* AF */
        tBTA_RDS_PIN        pin;        /* PIN */
        tBTA_RDS_MAP_FREQ   map_freq;
        tBTA_RDS_LINK_DATA  linkage;    /* Linkage information */
    }data;
}tBTA_RDS_EON_DATA;

#ifndef BTA_RDS_RTP_TAG_MAX
#define BTA_RDS_RTP_TAG_MAX     6
#endif

typedef struct
{
    UINT8               content_type;
    UINT8               start;
    UINT8               len;
} tBTA_RDS_RTP_TAG;

typedef struct
{
    BOOLEAN             running;
    UINT8               tag_toggle;
    UINT8               tag_num;
    tBTA_RDS_RTP_TAG    tag[BTA_RDS_RTP_TAG_MAX];
} tBTA_RDS_RTP;

typedef struct
{
    UINT8           *p_data;
    UINT8           g_type;
    BOOLEAN         complete;
} tBTA_RDS_RT;

/* RDS callback data */
typedef union
{
    tBTA_RDS_CT         ct;             /* Clock time */
    tBTA_RDS_PIN        pin;            /* Program Item Number */
    tBTA_RDS_STATUS     status;         /* status */
    tBTA_RDS_AF_LIST    af_data;        /* AF */
    tBTA_RDS_PTY        pty;            /* PTY callback data */
    UINT16              pi_code;       /* PI code */
    UINT8               data8;         /* UINT8 data, used for DI callback */
    tBTA_FM_RDS_TP_TYPE ta_tp;          /* TP/TA status */
    UINT8               *p_data;        /* data pointer to UINT8, used for PS, PTYN */
    tBTA_RDS_RPTOR_SET  rptor_set;
    tBTA_RDS_M_S        m_s;            /* music speech feature */
    tBTA_RDS_ODA_DATA   oda;            /* Open data application data, TMC */
    tBTA_RDS_TDC_DATA   tdc_data;       /* transparent data channel data */
    tBTA_RDS_SEG_DATA   seg_data;       /* EWS data */
    tBTA_RDS_SLC_DATA   slc;            /* slow labelling codes */
    tBTA_RDS_EON_DATA   eon;            /* EON data */
    tBTA_RDS_RTP        rtplus;
    UINT32              stats[4];       /* statistics data */
    tBTA_RDS_RT         rt;             /* used for RT */
} tBTA_FM_RDS;

typedef void (tBTA_RDS_CBACK)(tBTA_RDS_EVT event, tBTA_FM_RDS *p_data, UINT8 app_id);
/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/*******************************************************************************
**
** Function         BTA_FmRDSInitDecoder
**
** Description      This funtion initialize the RDS group decoder by resetting
**                  default values to the RDS parser control block.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_FmRDSInitDecoder(void);
/*******************************************************************************
**
** Function         BTA_FmRDSRegister
**
** Description      This funtion register the user callback and app ID with RDS
**                  decoder, together with the RDS feature mask they are interested
**                  in. Use the function, a client:
**
**                  I.   can be registered at RDS decoder,
**                  II.  deregister at decoder by sending the same app_id with a NULL
**                       call back function; or
**                  III. re-register the call back with different RDS feature mask.
**
**
** Parameter        rds_mask: RDS feature this client interested in
**                  p_cback: call back function of the client
**                  app_id: application ID associate with this client, it must be a
**                           NONE-ZERO number.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern tBTA_RDS_STATUS BTA_FmRDSRegister(tBTA_FM_RDS_B mode, tBTA_RDS_FEATURE rds_mask,
                                      tBTA_RDS_CBACK *p_cback, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_FmRDSResetDecoder
**
** Description      Reset RDS decoder decoding control variable for a new parsing.
**                  Usually this function is called when jumping to a new frequency,
**                  and all the old RDS information should be erased.
**
** Returns          app_id: application ID associate with a RDS stream, it must be a
**                           NONE-ZERO positive number.
**
*******************************************************************************/
BTA_API extern tBTA_RDS_STATUS BTA_FmRDSResetDecoder(UINT8 app_id);
/*******************************************************************************
**
** Function         BTA_FmRDSDecode
**
** Description      Parse the rds group
**
** Parameter        p_data : input rds data in multiple tuples, which means it's
**                           always 3n bytes.
**                  len    : the total length of p_data.
**s
**                  app_id: application ID associate with a RDS stream, it must be a
**                           NONE-ZERO positive number.
** Returns          void
**
*******************************************************************************/
BTA_API extern tBTA_RDS_STATUS BTA_FmRDSDecode(UINT8 app_id, UINT8 *p_data, UINT16 length);


#ifdef __cplusplus
}
#endif

#endif /* BTA_RDS_API_H */
