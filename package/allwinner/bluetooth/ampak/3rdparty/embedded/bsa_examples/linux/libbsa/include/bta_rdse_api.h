/*****************************************************************************
**
**  Name:           bta_rdse_api.h
**
**  Description:    This is the public interface file for the RDS decoder.
**
**  Copyright (c) 2006, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_RDSE_API_H
#define BTA_RDSE_API_H

#include "bta_api.h"
/*****************************************************************************
**  Constants and data types
*****************************************************************************/

#ifndef BTA_RDSE_DEBUG
#define BTA_RDSE_DEBUG    TRUE
#endif

enum
{
    BTA_RDSE_FEAT_PI,        /* mandotory feature included in blk 1/2 */
    BTA_RDSE_FEAT_PTY,       /* program type */
    BTA_RDSE_FEAT_TP,        /* traffic program */
    BTA_RDSE_FEAT_PS,        /* mandotory feature group */

    BTA_RDSE_FEAT_AF,        /* optional features not included in 0A/B group */
    BTA_RDSE_FEAT_RT,        /* radio text */
    BTA_RDSE_FEAT_PTYN,      /* program type name */
    BTA_RDSE_FEAT_CT,        /* clock time */
    BTA_RDSE_FEAT_RTP,       /* RT+ */

    BTA_RDSE_FEAT_MS,       /* optional feature included in mandotory group */
    BTA_RDSE_FEAT_DI,
    BTA_RDSE_FEAT_RPTOR
};
typedef UINT8 tBTA_RDSE_FEATURE;

/* bit mask of different RDS features */
#define BTA_RDSE_FEAT_MASK(x)   (0x0001 << x)
#define BTA_RDSE_PI_BIT        BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_PI)
#define BTA_RDSE_PTY_BIT       BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_PTY)
#define BTA_RDSE_TP_BIT        BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_TP)
#define BTA_RDSE_PS_BIT        BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_PS)
#define BTA_RDSE_AF_BIT        BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_AF)
#define BTA_RDSE_RT_BIT        BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_RT)
#define BTA_RDSE_PTYN_BIT      BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_PTYN)
#define BTA_RDSE_CT_BIT        BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_CT)
#define BTA_RDSE_RTP_BIT       BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_RTP)
#define BTA_RDSE_MS_BIT        BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_MS)
#define BTA_RDSE_DI_BIT        BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_DI)
#define BTA_RDSE_RPTOR_BIT     BTA_RDSE_FEAT_MASK(BTA_RDSE_FEAT_RPTOR)

typedef UINT16 tBTA_RDSE_FEAT_MASK;

#define BTA_RDSE_FEAT_MASK_MIN   BTA_RDSE_FEAT_AF
#define BTA_RDSE_FEAT_MASK_MAX   BTA_RDSE_FEAT_RTP

/* maximum PS length RDS encoder can support */
#define     BTA_RDSE_LEN_8       8
#define     BTA_RDSE_RT_LEN      64
#ifndef     BTA_RDSE_PS_LEN_MAX
#define     BTA_RDSE_PS_LEN_MAX     (BTA_RDSE_LEN_8 * 2)
#endif

/* RDS encoder operation status */
enum
{
    BTA_RDSE_OK,                    /* status OK */
    BTA_RDSE_CONT,                  /* continue */
    BTA_RDSE_ERR,                   /* general error */
    BTA_RDSE_ERR_UNSP               /* feature not supported */
};
typedef UINT8   tBTA_RDSE_STATUS;

/* scrolling rate for PS */
enum
{
    BTA_RDSE_SCR_RATE_1_SEC = 1,          /* scroll once per 1 seconds */
    BTA_RDSE_SCR_RATE_2_SEC,              /* scroll once per 2 seconds */
    BTA_RDSE_SCR_RATE_3_SEC,              /* scroll once per 3 seconds */
    BTA_RDSE_SCR_RATE_4_SEC,              /* scroll once per 4 seconds */
    BTA_RDSE_SCR_RATE_5_SEC,              /* scroll once per 5 seconds */
    BTA_RDSE_SCR_RATE_6_SEC,              /* scroll once per 6 seconds */
    BTA_RDSE_SCR_RATE_NON                 /* none-scrolling */
};
typedef UINT8   tBTA_RDSE_SCR_RATE;

/* PS configuration parameter */
typedef struct
{
    BOOLEAN                 rotate;     /* if to rotate PS */
    tBTA_RDSE_SCR_RATE      scr_rate;   /* PS scrolling rate */
    UINT8                   scr_step;   /* PS scrolling step */
    UINT8                   ps_len;     /* length of PS */
    UINT8                   ps_text[BTA_RDSE_PS_LEN_MAX];  /* PS text */
}tBTA_RDSE_PS;

/* RT+ tag type */
enum
{
    BTA_RDSE_RTP_ITEM_DUMMY,        /* 0 */
    BTA_RDSE_RTP_ITEM_TITLE      ,  /* 1 */
    BTA_RDSE_RTP_ITEM_ALBUM      ,  /* 2 */
    BTA_RDSE_RTP_ITEM_TRCKNUM    ,  /* 3 */
    BTA_RDSE_RTP_ITEM_ARTIST    ,   /* 4 */
    BTA_RDSE_RTP_ITEM_COMP    ,     /* 5 */
    BTA_RDSE_RTP_ITEM_MOVE    ,     /* 6 */
    BTA_RDSE_RTP_ITEM_CNDTR    ,    /* 7 conductor */
    BTA_RDSE_RTP_ITEM_CPOR    ,     /* 8 composor */
    BTA_RDSE_RTP_ITEM_BAND    ,     /* 9 */
    BTA_RDSE_RTP_ITEM_COMM    ,     /* 10 */
    BTA_RDSE_RTP_ITEM_GENRE    ,    /* 11 */

    BTA_RDSE_RTP_INFO_NEWS    ,
    BTA_RDSE_RTP_INFO_NEWSL    ,
    BTA_RDSE_RTP_INFO_STOCK    ,
    BTA_RDSE_RTP_INFO_SPORT    ,
    BTA_RDSE_RTP_INFO_LOTRY    ,
    BTA_RDSE_RTP_INFO_HORO    ,
    BTA_RDSE_RTP_INFO_DAILY    ,
    BTA_RDSE_RTP_INFO_HEALTH    ,
    BTA_RDSE_RTP_INFO_EVENT    ,    /* 20 */
    BTA_RDSE_RTP_INFO_SZENE    ,
    BTA_RDSE_RTP_INFO_CINEMA    ,
    BTA_RDSE_RTP_INFO_TV    ,
    BTA_RDSE_RTP_INFO_DATE    ,
    BTA_RDSE_RTP_INFO_WEATH    ,
    BTA_RDSE_RTP_INFO_TRAFFIC   ,   /* 26 */
    BTA_RDSE_RTP_INFO_ALARM    ,
    BTA_RDSE_RTP_INFO_ADS ,
    BTA_RDSE_RTP_INFO_URL ,
    BTA_RDSE_RTP_INFO_OTHR
};
#define BTA_RDSE_RTP_NONE       0xff            /* none-tagged RT type */
typedef UINT8   tBTA_RDSE_RTP_TYPE;

/* RT+ element */
typedef struct
{
    UINT8                   *p_elem_text;   /* RT+ content information */
    UINT16                  elem_len;       /* RT+ content information length */
    tBTA_RDSE_RTP_TYPE      rtp_type;       /* RT+ content type */

}tBTA_RDSE_RT_ELEM;

/* maximum number of RTplus elements we can support */
#ifndef BTA_RDSE_RTE_MAX
#define BTA_RDSE_RTE_MAX    4
#endif

/* RT configuration parameters */
typedef struct
{
    tBTA_RDSE_RT_ELEM       elem[BTA_RDSE_RTE_MAX];/* RT elements */
    UINT16                  num_elem;              /* number of RT element */

}tBTA_RDSE_RT;

/* Alternative frequency configuration parameter */
#define BTA_RDSE_AF_MAX         25
typedef struct
{
    UINT16      af_list[BTA_RDSE_AF_MAX];   /* AF freqneucy list */
    UINT16      af_num;                     /* Number of AF */
}tBTA_RDSE_AF;

/* CT data */
typedef struct
{
    UINT32  day;
    UINT8   hour;
    UINT8   minute;
    UINT8   offset;
} tBTA_RDSE_CT;

enum
{
    BTA_RDSE_RPTOR_SI,
    BTA_RDSE_RPTOR_SO,
    BTA_RDSE_RPTOR_LS2,
    BTA_RDSE_RPTOR_UNKNOW
};
typedef UINT8 tBTA_RDSE_RPTOR_SET;

/* music speech content type */
enum
{
    BTA_RDSE_SPEECH = 1,
    BTA_RDSE_MUSIC
};
typedef UINT8 tBTA_RDSE_M_S;

typedef UINT8 tBTA_RDSE_TP_TYPE;

/* RDS encoder initialization data */
typedef struct
{
    tBTA_RDSE_TP_TYPE    tp_ta;     /* traffic program/announce flag */
    UINT16              pi_code;    /* PI code */
    UINT32              pty;        /* program type */
    tBTA_RDSE_PS        ps;         /* program service name */

}tBTA_RDSE_ENC_DATA;

/* data structure for configuring RDS Encoder */
typedef union
{
    tBTA_RDSE_RPTOR_SET  reptor;
    tBTA_RDSE_PS        ps;         /* program service */
    tBTA_RDSE_RT        rt;         /* radio text */
    tBTA_RDSE_AF        af;         /* alternative frequency */
    UINT8               ptyn[8];    /* program type name */
    UINT32              pty;        /* program type */
    UINT16              pi_code;    /* program identifier */
    UINT8               di;         /* device identifier */
    tBTA_RDSE_M_S        ms;        /* music speech */
    tBTA_RDSE_TP_TYPE    tp_ta;     /* traffic program/annoucement flag */
    tBTA_RDSE_CT         ct;        /* clock time */

}tBTA_RDSE_CFG_DATA;

/* size of each RDS block in bytes */
#define     BTA_RDSE_BLK_SIZE_INBYTE    2
/* block number per group */
#define     BTA_RDSE_BLK_PER_GRP    4
/* size of each RDS group in bytes , 1 group = 4 blk * 2 bytes = 8 */
#define     BTA_RDSE_GRP_SIZE_INBYTE    (BTA_RDSE_BLK_SIZE_INBYTE * BTA_RDSE_BLK_PER_GRP)
typedef UINT8  tBTA_RDSE_GRP_DATA[BTA_RDSE_GRP_SIZE_INBYTE];

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/*******************************************************************************
**
** Function         BTA_RdsEncEnable
**
** Description      This funtion initialize the RDS group decoder by resetting
**                  default values to the RDS encoder control block.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_RdsEncEnable(tBTA_RDSE_ENC_DATA *p_data);

/*******************************************************************************
**
** Function         BTA_RdsEncDisable
**
** Description      This funtion disable the RDS encoder.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_RdsEncDisable(void);

/*******************************************************************************
**
** Function         BTA_RdsEncSetFeatMask
**
** Description      Configure new RDS encoding features.When a RDS feature is added
**                  or removed, the RDS data scheduler need to change the data
**                  scheduling to balance the work load of new RDS data groups.
**
** Parameter
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_RdsEncSetFeatMask(tBTA_RDSE_FEAT_MASK mask);

/*******************************************************************************
**
** Function         BTA_RdsEncSetValue
**
** Description      Configure the RDS encoder RDS feature value.
**
** Parameter
**
** Returns          BTA_RDSE_OK:        configuration succeed
**                  BTA_RDSE_ERR_UNSP : unsupported RDS features
**
*******************************************************************************/
BTA_API extern tBTA_RDSE_STATUS BTA_RdsEncSetValue(tBTA_RDSE_FEAT_MASK rds_type,
                                                 tBTA_RDSE_CFG_DATA *p_cfg_data);

/*******************************************************************************
**
** Function         BTA_FmRDSEncode
**
** Description      Encode the RDS data
**
** Parameter        grp_length: input as the maximum length of RDS data to be encoded.
**                              output as the actual length of the encoded RDS data.
**                  p_data  : pre-allocated data buffer to store the encoded RDS
**                            data.
**                  one_set:  Request one set of complete RDS feature encoding or not.
**
** Returns          BTA_RDSE_OK:   encoding finished correctly
**                  BTA_RDSE_CONT: One set encoded RDS data can not be fit into
**                                  the buffer provided.
**                  BTA_RDSE_ERR:  encoding error
**
*******************************************************************************/
BTA_API extern tBTA_RDSE_STATUS BTA_RdsEncode(UINT8 *grp_len, tBTA_RDSE_GRP_DATA *p_data,
                                              BOOLEAN one_set);


#ifdef __cplusplus
}
#endif

#endif /* BTA_RDSE_API_H */
