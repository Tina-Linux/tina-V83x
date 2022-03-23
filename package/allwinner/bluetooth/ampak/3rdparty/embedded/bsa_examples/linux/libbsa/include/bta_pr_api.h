/*****************************************************************************
**
**  Name:           bta_pr_api.h
**
**  Description:    This is the public interface file for the printer
**                  (PR) client subsystem of BTA, Widcomm's Bluetooth
**                  application layer for mobile phones.
**
**  Copyright (c) 2004-2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_PR_API_H
#define BTA_PR_API_H

#include "bta_api.h"
#include "bta_op_api.h"
#include "bta_bi_api.h"
#include "bpp_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/
/* Extra Debug Code */
#ifndef BTA_PR_DEBUG
#define BTA_PR_DEBUG           TRUE
#endif

#if ((BT_USE_TRACES == FALSE) && (BTA_PR_DEBUG == TRUE))
#undef BTA_PR_DEBUG
#define BTA_PR_DEBUG           FALSE
#endif

#ifndef BTA_BP_DEBUG
#define BTA_BP_DEBUG           TRUE
#endif

#if ((BT_USE_TRACES == FALSE) && (BTA_BP_DEBUG == TRUE))
#undef BTA_BP_DEBUG
#define BTA_BP_DEBUG           FALSE
#endif

/* maximum number of 'other' formats supported by BPP which are not covered in BTA */
#ifndef BTA_PR_OTH_SPT_FMT_MAX
#define BTA_PR_OTH_SPT_FMT_MAX          5
#endif

#ifndef BTA_PR_JOB_ATTR_TIME
#define BTA_PR_JOB_ATTR_TIME            2000
#endif

/* max number of times BPP authentication could be tried, by default for only one time */
#ifndef BTA_PR_MAX_BPP_AUTH_ATMP
#define BTA_PR_MAX_BPP_AUTH_ATMP        1
#endif

/* Document formats for BTA_PrPrint 'format' parameter */
#define BTA_PR_XHTML_PRINT_FMT      0       /* xhtml-print */
#define BTA_PR_VCARD_FMT            1       /* vCard 2.1 */
#define BTA_PR_VCAL_FMT             2       /* vCal 1.0 */
#define BTA_PR_ICAL_FMT             3       /* iCal 2.0 */
#define BTA_PR_VNOTE_FMT            4       /* vNote */
#define BTA_PR_VMSG_FMT             5       /* vMessage */
#define BTA_PR_PLAIN_TEXT_FMT       6       /* text/plain */
#define BTA_PR_PDF_FMT              7       /* Adobe PDF file */
#define BTA_PR_PCL_FMT              8       /* HP PCL */
#define BTA_PR_XHTML_MULTIPLEXED_FMT 9       /* A compound or multiplexed document. */
#define BTA_PR_POSTSCRIPT_FMT       10       /* PostScript */

#define BTA_PR_OTHER_FMT            11      /* other format */

#define BTA_PR_IMAGE_FMT           24       /* Image */
#define BTA_PR_DPOF_FMT            23       /* DPOF format file for image printing */


typedef UINT8 tBTA_PR_FMT;


/* Document formats masks for tBTA_PR_CAPS structure */
#define BTA_PR_FMT_MASK(format)     ((tBTA_PR_FMT_MASK)(1 << (format)))     /* For converting from document format to mask */
#define BTA_PR_XHTML_PRINT_MASK     (BTA_PR_FMT_MASK(BTA_PR_XHTML_PRINT_FMT))   /* 0001 xhtml-print */
#define BTA_PR_VCARD_MASK           (BTA_PR_FMT_MASK(BTA_PR_VCARD_FMT))         /* 0002 vCard 2.1 */
#define BTA_PR_VCAL_MASK            (BTA_PR_FMT_MASK(BTA_PR_VCAL_FMT))          /* 0004 vCal 1.0 */
#define BTA_PR_ICAL_MASK            (BTA_PR_FMT_MASK(BTA_PR_ICAL_FMT))          /* 0008 iCal 2.0 */
#define BTA_PR_VNOTE_MASK           (BTA_PR_FMT_MASK(BTA_PR_VNOTE_FMT))         /* 0010 vNote */
#define BTA_PR_VMSG_MASK            (BTA_PR_FMT_MASK(BTA_PR_VMSG_FMT))          /* 0020 vMessage */
#define BTA_PR_PLAIN_TEXT_MASK      (BTA_PR_FMT_MASK(BTA_PR_PLAIN_TEXT_FMT))    /* 0040 text/plain */
#define BTA_PR_PDF_MASK             (BTA_PR_FMT_MASK(BTA_PR_PDF_FMT))           /* 0080 application/PDF */
#define BTA_PR_PCL_MASK             (BTA_PR_FMT_MASK(BTA_PR_PCL_FMT))           /* 0100 application/vnd.hp-PCL */
#define BTA_PR_XHTML_MULTIPLEXED_MASK   (BTA_PR_FMT_MASK(BTA_PR_XHTML_MULTIPLEXED_FMT))  /* 0200 application/vnd.pwg-multiplexed */
#define BTA_PR_POSTSCRIPT_MASK      (BTA_PR_FMT_MASK(BTA_PR_POSTSCRIPT_FMT))    /* 0400 application/PostScript */
#define BTA_PR_OTHER_MASK           (BTA_PR_FMT_MASK(BTA_PR_OTHER_FMT))         /* 0800 other format */

/* Image formats */
#define BTA_PR_JPEG_MASK            (BTA_PR_FMT_MASK(BTA_PR_IMAGE_FMT + 0))     /* 01000000 image/jpeg */
#define BTA_PR_GIF_MASK             (BTA_PR_FMT_MASK(BTA_PR_IMAGE_FMT + 1))     /* 02000000 image/gif */

typedef UINT32 tBTA_PR_FMT_MASK;


/* Status */
#define BTA_PR_OK                   0   /* Object push succeeded. */
#define BTA_PR_ERR                  1   /* Object push failed. */
#define BTA_PR_ERR_FILE             2   /* Object not found. */
#define BTA_PR_ERR_PERMISSION       3   /* Operation not authorized. */
#define BTA_PR_ERR_MEM              4   /* Unable to allocate buffer for xhtml conversion. */
#define BTA_PR_ERR_SDP              5   /* SDP error (printer did not respond or does not support object format */
#define BTA_PR_ERR_FORBIDDEN        6   /* Printer cannot print requested file */
#define BTA_PR_OBJ_ERR              7   /* Printer referenced object channel error */
#define BTA_PR_JOB_FAILED           8   /* Printer job failed */
#define BTA_PR_ERR_DOC_FORMAT       9   /* unsupported document format */
#define BTA_PR_UNAUTH               10

typedef UINT8 tBTA_PR_STATUS;

/* Client callback function event */
#define BTA_PR_ENABLE_EVT           0   /* Printer client is enabled. */
#define BTA_PR_OPEN_EVT             1   /* Connection to printer server is open. */
#define BTA_PR_PROGRESS_EVT         2   /* Push to printer in progress */
#define BTA_PR_CLOSE_EVT            3   /* Connection to printer server closed. */
#define BTA_PR_GETCAPS_EVT          4   /* Response to BTA_PrGetCaps */
#define BTA_PR_AUTH_EVT             5   /* Request for Authentication key and realm */
#define BTA_PR_THUMBNAIL_EVT        6   /* Printer requests for thumbnail version of the image */
#define BTA_PR_PARTIAL_IMAGE_EVT    7   /* Printer requests for a portion of the image (for DPOF print) */
#define BTA_PR_GET_OBJ_EVT          8   /* printer requests for referenced objects in xhtml content sent via BPP */
#define BTA_PR_BP_DOC_CMPL_EVT      9   /* (BPP) - Sent after document has been transmitted to the printer. */
#define BTA_PR_JOB_STATUS_EVT       10  /* (BPP) - Printer status changed event.
                                           BTA_PrCancelBpStatus() is called when status is no longer desired.
                                           Typically called after BTA_PR_BP_DOC_CMPL_EVT (Job is transferred to
                                           printer), or this event is received with job state of completed, aborted,
                                           cancelled, etc. */

typedef UINT8 tBTA_PR_EVT;

/* Additional print paramters for BTA_PrPrint using BIP servers */
typedef tBIP_IMAGE_DESC tBTA_PR_BI_DESC;


/****************************************************************
** Additional print paramters for BTA_PrPrint using BPP servers *
*****************************************************************/

/* The following definition is used for unspecified integer parameters */
#define BTA_BP_PARAM_UNSPECIFIED    0

/* maximum length of type header to be passed from application */
#ifndef BTA_PA_MIME_TYPE_LEN_MAX
#define BTA_PA_MIME_TYPE_LEN_MAX    59
#endif

typedef char tBTA_MIME_HDR[BTA_PA_MIME_TYPE_LEN_MAX+1]; /* contains the MIME type header */

/* maximum length of media size string to be passed from application */
#ifndef BTA_MAX_MEDIA_SIZE_STRING
#define BTA_MAX_MEDIA_SIZE_STRING   31
#endif

/* maximum number of media trays loaded */
#ifndef BTA_MAX_MEDIA_LOADED
#define BTA_MAX_MEDIA_LOADED        BPP_MAX_MEDIA_LOADED
#endif

/* Sides parameter values */
#define BTA_BP_SIDES_IGNORED        BPP_SIDES_UNDEF
#define BTA_BP_ONE_SIDED            BPP_BIT_ONE_SIDED
#define BTA_BP_TWO_SIDED_LONG       BPP_BIT_TWO_SIDED_LONG_EDGE
#define BTA_BP_TWO_SIDED_SHORT      BPP_BIT_TWO_SIDED_SHORT_EDGE

typedef tBPP_SIDES_B tBTA_PR_SIDES;


/* Orientation parameter values */
#define BTA_BP_ORIENTATION_IGNORED  BPP_PR_QUALITY_UNDEF
#define BTA_BP_PORTRAIT             BPP_BIT_PORTRAIT
#define BTA_BP_LANDSCAPE            BPP_BIT_LANDSCAPE
#define BTA_BP_REVERSE_LANDSCAPE    BPP_REVERSE_LANDSCAPE
#define BTA_BP_REVERSE_PORTRAIT     BPP_BIT_REVERSE_PORTRAIT

typedef tBPP_ORIENTATION_B tBTA_PR_ORIENT;


/* Print quality parameter values */
#define BTA_BP_QUALITY_IGNORED      BPP_PR_QUALITY_UNDEF
#define BTA_BP_QUALITY_NORMAL       BPP_BIT_NORMAL
#define BTA_BP_QUALITY_DRAFT        BPP_BIT_DRAFT
#define BTA_BP_QUALITY_HIGH         BPP_BIT_HIGH

typedef tBPP_PR_QUALITY_B tBTA_PR_QUALITY;


/* MediaType */
/* Determine the number of UINT32's necessary for media types */
#define BTA_MTYPE_ARRAY_BITS    BPP_MTYPE_ARRAY_BITS  /* Number of bits in each array element */
#define BTA_MTYPE_ARRAY_INDEX   BPP_MTYPE_ARRAY_INDEX /* Number of elements in array */

/* MediaType */
#define  BTA_MEDIA_TYPE_IGNORED         BPP_MEDIA_TYPE_UNDEF        /* 0 */
#define  BTA_STATIONERY                 BPP_STATIONERY              /* 1 */
#define  BTA_STATIONERY_COATED          BPP_STATIONERY_COATED       /* 2 */
#define  BTA_STATIONERY_INKJET          BPP_STATIONERY_INKJET       /* 3 */
#define  BTA_STATIONERY_PREPRINTED      BPP_STATIONERY_PREPRINTED   /* 4 */
#define  BTA_STATIONERY_LETTERHEAD      BPP_STATIONERY_LETTERHEAD   /* 5 */
#define  BTA_STATIONERY_PREPUNCHED      BPP_STATIONERY_PREPUNCHED   /* 6 */
#define  BTA_STATIONERY_FINE            BPP_STATIONERY_FINE         /* 7 */
#define  BTA_STATIONERY_HEAVYWEIGHT     BPP_STATIONERY_HEAVYWEIGHT  /* 8 */
#define  BTA_STATIONERY_LIGHTWEIGHT     BPP_STATIONERY_LIGHTWEIGHT  /* 9 */
#define  BTA_TRANSPARENCY               BPP_TRANSPARENCY            /* 10 */
#define  BTA_ENVELOPE                   BPP_ENVELOPE                /* 11 */
#define  BTA_ENVELOPE_PLAIN             BPP_ENVELOPE_PLAIN          /* 12 */
#define  BTA_ENVELOPE_WINDOW            BPP_ENVELOPE_WINDOW         /* 13 */
#define  BTA_CONTINUOUS                 BPP_CONTINUOUS              /* 14 */
#define  BTA_CONTINUOUS_LONG            BPP_CONTINUOUS_LONG         /* 15 */
#define  BTA_CONTINUOUS_SHORT           BPP_CONTINUOUS_SHORT        /* 16 */
#define  BTA_TAB_STOCK                  BPP_TAB_STOCK               /* 17 */
#define  BTA_PRE_CUT_TABS               BPP_PRE_CUT_TABS            /* 18 */
#define  BTA_FULL_CUT_TABS              BPP_FULL_CUT_TABS           /* 19 */
#define  BTA_MULTI_PART_FORM            BPP_MULTI_PART_FORM         /* 20 */
#define  BTA_LABELS                     BPP_LABELS                  /* 21 */
#define  BTA_MULTI_LAYER                BPP_MULTI_LAYER             /* 22 */
#define  BTA_SCREEN                     BPP_SCREEN                  /* 23 */
#define  BTA_SCREEN_PAGED               BPP_SCREEN_PAGED            /* 24 */
#define  BTA_PHOTOGRAPHIC               BPP_PHOTOGRAPHIC            /* 25 */
#define  BTA_PHOTOGRAPHIC_GLOSSY        BPP_PHOTOGRAPHIC_GLOSSY     /* 26 */
#define  BTA_PHOTOGRAPHIC_HIGH_GLOSS    BPP_PHOTOGRAPHIC_HIGH_GLOSS /* 27 */
#define  BTA_PHOTOGRAPHIC_SEMI_GLOSS    BPP_PHOTOGRAPHIC_SEMI_GLOSS /* 28 */
#define  BTA_PHOTOGRAPHIC_SATIN         BPP_PHOTOGRAPHIC_SATIN      /* 29 */
#define  BTA_PHOTOGRAPHIC_MATTE         BPP_PHOTOGRAPHIC_MATTE      /* 30 */
#define  BTA_PHOTOGRAPHIC_FILM          BPP_PHOTOGRAPHIC_FILM       /* 31 */
#define  BTA_BACK_PRINT_FILM            BPP_BACK_PRINT_FILM         /* 32 */
#define  BTA_CARDSTOCK                  BPP_CARDSTOCK               /* 33 */
#define  BTA_ROLL                       BPP_ROLL                    /* 34 */
#define  BTA_MAX_MEDIA_TYPES            BPP_MAX_MEDIA_TYPE

typedef tBPP_MEDIA_TYPE tBTA_PR_MTYPE;

/************************************************************************************************
** Media Type MACROS handle array of UINT32 bits for more than 32 media types
*************************************************************************************************/
/* MACRO to set the security service bit mask in a bit stream */
#define BTA_SET_MTYPE(p, mtype)  (((UINT32 *)(p))[(((UINT32)((mtype)-1)) / BTA_MTYPE_ARRAY_BITS)] |=  \
                                    (1 << (((UINT32)((mtype)-1)) % BTA_MTYPE_ARRAY_BITS)))


/* MACRO to clear the media type bit mask in a bit stream */
#define BTA_CLR_MTYPE(p, mtype)  (((UINT32 *)(p))[(((UINT32)((mtype)-1)) / BTA_MTYPE_ARRAY_BITS)] &=  \
                                      ~(1 << (((UINT32)((mtype)-1)) % BTA_MTYPE_ARRAY_BITS)))

/* MACRO to check the media type bit mask in a bit stream (Returns TRUE or FALSE) */
#define BTA_IS_MTYPE_SUPPORTED(p, mtype)    (((((UINT32 *)(p))[(((UINT32)((mtype)-1)) / BTA_MTYPE_ARRAY_BITS)]) &   \
                                            (UINT32)((1 << (((UINT32)((mtype)-1)) % BTA_MTYPE_ARRAY_BITS)))) ? TRUE : FALSE)

/* MACRO to copy two media type bitmasks */
#define BTA_COPY_MTYPE(p_src, p_dst)   {int mdty; for (mdty = 0; mdty < BTA_MTYPE_ARRAY_INDEX; mdty++) \
                                                      ((UINT32 *)(p_dst))[mdty] = ((UINT32 *)(p_src))[mdty];}


/************************************************************************
** Parameter types used by BTA BP
**      Note: 0/FALSE is used for unspecified integer/boolean parameters.
**            NULL String ("") is used for unspecified string parameters.
*************************************************************************/
typedef struct
{
    UINT32          copies;          /* number of copies to print */
    UINT32          number_up;       /* number of pages printed per side of the medium   */

    char            media_size[BTA_MAX_MEDIA_SIZE_STRING+1];

    tBTA_PR_MTYPE   media_type;      /* Type of media to use */
    tBTA_PR_ORIENT  orient;          /* Lanscape, Portrait, etc. */
    tBTA_PR_SIDES   sides;           /* how pages are imposed upon the sides of medium   */
    tBTA_PR_QUALITY quality;         /* normal, draft, or high - see definitions (above) */
    BOOLEAN         use_ref_channel; /* BPP referenced object channel requested/or not   */
    BOOLEAN         use_precise_job; /* TRUE if precise job desired. Check printer
                                        capabilities before using. Error is returned if
                                        set and printer does not support the feature.    */
} tBTA_PR_BP_PARAMS;


/* Data for BTA_PR_PROGRESS_EVT callback event */
typedef struct
{
    UINT32      obj_size;   /* Total size of object */
    UINT16      bytes;      /* Number of bytes read or written since last progress event */
} tBTA_PR_PROGRESS;

typedef tBPP_OPER_STATUS tBTA_PR_BPP_OPSTAT;

/************************************************************************
** Printer Capability parameter types used by BTA BP
**      Note: 0/FALSE is used for unspecified integer/boolean values.
**            NULL String ("") is used for unspecified string values.
*************************************************************************/

/* Printer state values */
#define BTA_BP_STATE_UNKNOWN    BPP_PR_STATE_UNDEF
#define BTA_BP_STATE_IDLE       BPP_PR_IDLE         /* idle */
#define BTA_BP_STATE_PROC       BPP_PR_PROCESSING   /* processing */
#define BTA_BP_STATE_STOPPED    BPP_PR_STOPPED      /* stopped - intervention required */

typedef tBPP_PR_STATE   tBTA_PR_STATE;

/* Character Repertoires (from SDP) 128-bit mask */
#define BTA_CHAR_REPERTOIRES_SIZE       BPP_CHAR_REPERTOIRES_SIZE

/* 1284 device ID string from SDP */
#ifndef BTA_PR_1284_NAME_LEN
#define BTA_PR_1284_NAME_LEN 300
#endif

typedef struct
{
    UINT16          len;
    UINT8           id[BTA_PR_1284_NAME_LEN + 1];
} tBTA_1284_INFO;


/* Printer state reason values */
#define BTA_PR_REASON_NONE              BPP_ST_RS_NONE      /* No reason given */
#define BTA_PR_REASON_ATTNREQ           BPP_ST_RS_ATT_REQ   /* Attention required - not listed */
#define BTA_PR_REASON_JAM               BPP_ST_RS_MED_JAM   /* Media Jam */
#define BTA_PR_REASON_PAUSED            BPP_ST_RS_PAUSED    /* Paused because state is stopped */
#define BTA_PR_REASON_DOOROPEN          BPP_ST_RS_DOOR_OPEN /* One or more covers on device are open */
#define BTA_PR_REASON_LOW               BPP_ST_RS_MED_LOW   /* At least one media tray is low */
#define BTA_PR_REASON_EMPTY             BPP_ST_RS_MED_EMP   /* At least one media tray is empty */
#define BTA_PR_REASON_OUTAREAALMOSTFULL BPP_ST_RS_OUT_AFUL  /* Output area almost full */
#define BTA_PR_REASON_OUTAREAFULL       BPP_ST_RS_OUT_FUL   /* Output area full */
#define BTA_PR_REASON_MRKRSUPLOW        BPP_ST_RS_MARKER_L  /* Device low on ink or toner */
#define BTA_PR_REASON_MRKRSUPEMPTY      BPP_ST_RS_MARKER_E  /* Device out of ink or toner */
#define BTA_PR_REASON_MRKRFAILURE       BPP_ST_RS_MARKER_F  /* Device ink cartridge or toner ribbon error */

typedef tBPP_PR_ST_REASON  tBTA_PR_REASON;

/* Media Loaded */
typedef tBPP_MEDIA_LOADED  tBTA_PR_MLOADED;

typedef tBPP_ATTR_STR_LIST tBTA_ATTR_STR_LIST;

/* BPP data returned in printer capabilities and SDP response */
typedef struct
{
    tBTA_PR_STATUS      bta_prstat;
    tBTA_PR_BPP_OPSTAT  bpp_prstat;          /* Operation status returned in BPP protocol */
    UINT32          max_copies;              /* Maximum number of copies */
    UINT32          max_number_up;           /* Maximum number of pages on a side */
    UINT32          queued_jobs;             /* Number of jobs in printer queue */
    UINT32          btp_width;               /* Basic text page width */
    UINT32          btp_height;              /* Basic text page height */
    UINT32          num_mloaded;             /* number of media loaded entries */
    UINT32          mtypes_mask[BTA_MTYPE_ARRAY_INDEX]; /* List of media types supported (bitmask). */

    tBTA_UINT128    char_rep;                /* Char repertoires supported. See BPP spec. sect 12.2.3 */
    tBTA_1284_INFO ID1284_info;     /* 1284 device ID string parsed information */
    char           *p_name;                  /* Name - may be truncated */
    char           *p_loc;                   /* Location - may be truncated */
    tBTA_PR_MLOADED mloaded[BTA_MAX_MEDIA_LOADED];  /* Array of loaded media */
    tBTA_PR_STATE   state;                   /* State - idle, processing, or stopped */
    tBTA_PR_REASON  state_reason;            /* Reason why the printer is in current state */
    tBTA_PR_SIDES   sides_mask;              /* BTA_BP_ONE_SIDED, BTA_BP_TWO_SIDED_LONG,
                                                and/or BTA_BP_TWO_SIDED_SHORT */

    tBTA_PR_QUALITY quality_mask;            /* BTA_BP_QUALITY_NORMAL, BTA_BP_QUALITY_DRAFT,
                                                and/or BTA_BP_QUALITY_HIGH */

    tBTA_PR_ORIENT  orient_mask;             /* BTA_BP_PORTRAIT, BTA_BP_LANDSCAPE,
                                                BTA_BP_REVERSE_LANDSCAPE, and/or
                                                BTA_BP_REVERSE_PORTRAIT */

    BOOLEAN         color_supported;         /* Printer supports color? */
    BOOLEAN         enhanced_supported;      /* Printer supports enhanced layout? */
    tBTA_ATTR_STR_LIST  *p_doc_fmt_list;     /* List of document formats supported */
    tBTA_ATTR_STR_LIST  *p_media_sizes_list; /* List of media sizes supported. NULL if none */
    tBTA_ATTR_STR_LIST  *p_img_fmt_list;     /* List of image formats supported. NULL if none */
} tBTA_BP_PR_CAPS;

/* Data for BTA_PR_GETCAPS_EVT callback event */
typedef struct
{
    tBTA_SERVICE_MASK   services;           /* Services supported */
    tBTA_PR_FMT_MASK    bp_caps_mask;       /* BPP documents supported */
    tBTA_PR_FMT_MASK    op_caps_mask;       /* OPP documents supported */
    tBIP_FEATURE_FLAGS  bi_features;        /* BIP supported features */
    tBIP_IMAGING_CAPS  *p_bi_caps;         /* BIP imaging capabilities */
    tBTA_BP_PR_CAPS    *p_bp_pr_attrs;     /* BPP Printer attributes (NULL if no service) */
} tBTA_PR_CAPS;


/* Data for BTA_PR_AUTH_EVT callback event */
typedef struct
{
    UINT8  *p_realm;
    UINT8   realm_len;
    UINT8   realm_charset;
    BOOLEAN userid_required;    /* If TRUE, a user ID must be sent */
} tBTA_PR_AUTH;


#ifndef BTA_PR_ROBJ_NAME_SIZE
#define BTA_PR_ROBJ_NAME_SIZE           200
#endif

/* Job State values */
#define BTA_PR_JOBSTATE_PRINTING        BPP_PRINTING
#define BTA_PR_JOBSTATE_WAITING         BPP_WAITING
#define BTA_PR_JOBSTATE_STOPPED         BPP_STOPPED
#define BTA_PR_JOBSTATE_COMPLETED       BPP_COMPLETED
#define BTA_PR_JOBSTATE_ABORTED         BPP_ABORTED
#define BTA_PR_JOBSTATE_CANCELLED       BPP_CANCELLED
#define BTA_PR_JOBSTATE_UNKNOWN         BPP_JS_UNKNOWN
typedef tBPP_JOB_STATE  tBTA_JOB_STATE;

typedef struct
{
    tBTA_JOB_STATE  job_state;            /* waiting, stopped, cancelled, unknown, etc. */
    tBTA_PR_STATE   printer_state;        /* State - idle, processing, or stopped */
    tBTA_PR_REASON  printer_state_reason; /* Reason in current state */
} tBTA_PR_JOB_STATUS;

/* Union of all client callback structures */
typedef union
{
    tBTA_PR_PROGRESS        prog;
    tBTA_PR_STATUS          status;
    tBTA_PR_CAPS            caps;
    tBTA_SERVICE_ID         service;        /* BTA_PR_OPEN_EVT */
    tBIP_GET_PART_REQ_EVT   part;           /* BTA_PR_PART_IMAGE_EVT */
    tBTA_PR_AUTH            auth;
    tBTA_PR_JOB_STATUS      bp_job_status;  /* BTA_PR_JOB_STATUS_EVT */
    char                    *p_name;        /* receive GetRefObj request in BPP
                                               referenced object channel */
} tBTA_PR;


/* Client callback function */
typedef void (tBTA_PR_CBACK)(tBTA_PR_EVT event, tBTA_PR *p_data);

/* BPP object channel idle timer timeout duration */
#ifndef BTA_PR_OBJ_IDLE_TOUT
#define BTA_PR_OBJ_IDLE_TOUT           60000
#endif


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_PrEnable
**
** Description      Enable the printer client.  This function must be
**                  called before any other functions in the PR API are called.
**                  When the enable operation is complete the callback function
**                  will be called with a BTA_PR_ENABLE_EVT.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PrEnable(tBTA_SEC sec_mask, tBTA_PR_CBACK *p_cback,
                                  UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_PrDisable
**
** Description      Disable the printer client.  If the client is currently
**                  connected to a peer device the connection will be closed.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PrDisable(void);

/*******************************************************************************
**
** Function         BTA_PrGetCaps
**
** Description      Get the print capabilites of a printer. When capabilities
**                  have been retrieved, the tBTA_PR_CBACK will be called with a
**                  BTA_PR_GETCAPS_EVT.
**
** Parameters
**      bd_addr     Address of print server
**      services    Mask of services for which to get capabilities
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PrGetCaps(BD_ADDR bd_addr, tBTA_SERVICE_MASK services);

/*******************************************************************************
**
** Function         BTA_PrAuthRsp
**
** Description      Sends a response to an OBEX authentication challenge to the
**                  connected OBEX server. Called in response to an BTA_PR_AUTH_EVT
**                  event.
**
**                  Note: If the "userid_required" is TRUE in the BTA_PR_AUTH_EVT
**                        event, then p_userid is required, otherwise it is optional.
**
**                  p_password  must be less than BTA_PR_MAX_AUTH_KEY_SIZE (16 bytes)
**                  p_userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PrAuthRsp (char *p_password, char *p_userid);

/*******************************************************************************
**
** Function         BTA_PrPrint
**
** Description      Push object(s) to a peer device for printing. Supports
**                  printing a list of objects on the same page. If specifying
**                  printing profile as BPP, and the format is BTA_PR_OTHER_FMT,
**                  specific type header is required. The application (BTA user)
**                  assumes the responsibility to make sure that the document format
**                  is supported by the printer when printer claim it may support
**                  BTA_PR_OTHER_FMT. When printing xhtml file via BPP, referenced
**                  object channel can be requested/not via the use_ref_channel field in
**                  p_bp_parms. BPP referenced object channel is turned off by default.
**
**
** Parameters
**      bd_addr     Address of print server
**      services    Mask of services for which to use to print
**      format      Format of items to be printed
**      p_header    MIME type header. Must be specified if format is BTA_PR_OTHER_FMT.
**      p_name      Fully qualified path and file name
**      p_bi_desc   Additional print parameters for BIP image printing.
**      p_bp_parms  Additional print parameters for BPP printing.
**                  If p_bp_parms is NULL, a default print job is initiated. Either
**                  the p_header must be specified and the format must be set to
**                  BTA_PR_OTHER_FMT, or the format parameter must NOT be set to
**                  BTA_PR_OTHER_FMT and the p_header can be NULL.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PrPrint(BD_ADDR bd_addr, tBTA_SERVICE_MASK services,
                                tBTA_PR_FMT format, char *p_header,
                                char *p_name,
                                tBTA_PR_BI_DESC *p_bi_desc,
                                tBTA_PR_BP_PARAMS *p_bp_parms);

/*******************************************************************************
**
** Function         BTA_PrPartialImageRsp
**
** Description      When BTA_PrPrint() is called with format as BTA_PR_DPOF_FMT
**                  and received BTA_PR_PARTIAL_IMAGE_EVT event, this function
**                  is called to supply the fully qualified file name and path.
**
** Parameters
**      p_name      Fully qualified path and file name
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PrPartialImageRsp(char *p_name);

/*******************************************************************************
**
** Function         BTA_PrRefObjRsp
**
** Description      When BTA_PrRefObjRsp() is called with use_ref_channel TRUE
**                  and received BTA_PR_GET_OBJ_EVT event, this function
**                  is called to supply the fully qualified file name with path.
**
** Parameters
**      p_name      Fully qualified path and file name
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PrRefObjRsp(char *p_name);

/*******************************************************************************
**
** Function         BTA_PrAbort
**
** Description      Close the current connection.  This function is called if
**                  the phone wishes to close the connection before the printing
**                  is completed.  In a typical connection this function does
**                  not need to be called; the connection will be closed
**                  automatically when the printing is complete.
**
** Parameters
**      none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PrAbort(void);


/*******************************************************************************
**
** Function         BTA_PrCancelBpStatus
**
** Description      (BPP Only) - Called to disable print job update events.
**                  Events are reenabled for every BPP print (BTA_PrPrint).
**
** Parameters
**      none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_PrCancelBpStatus(void);

/*******************************************************************************
**
** Function         BTA_PrCard2Xhtml
**
** Description      Convert array of vCard properties into xhtml buffer
**
** Parameters
**      Input:      p_xhtml_buf:    pointer to buffer for holding converted vcard
**                                      (if NULL, will return size of buffer
**                                      req'd to do the conversion in *p_len)
**                  p_len:          size of xhtml_buf
**                  p_props:        array of vCard properties
**                  num_props:      number of properties in p_prop array
**
**      Output:     p_len:          Number of bytes copied to xhtml_buf
**
** Returns          BTA_PR_OK       if operation successful.
**                  BTA_PR_ERR_MEM  if not enough for holding xhtml data
**
*******************************************************************************/
BTA_API extern tBTA_PR_STATUS BTA_PrCard2Xhtml(char *p_xhtml_buf, UINT32 *p_len,
                                               tBTA_OP_PROP *p_props, UINT16 num_props);

/*******************************************************************************
**
** Function         BTA_PrCal2Xhtml
**
** Description      Convert array of vCal properties into xhtml buffer
**
** Parameters
**      Input:      p_xhtml_buf:    pointer to buffer for holding converted vcard
**                                      (if NULL, will return size of buffer
**                                      req'd to do the conversion in *p_len)
**                  p_len:          size of xhtml_buf
**                  p_props:        array of vCal properties
**                  num_props:      number of properties in p_prop array
**
**      Output:     p_len:          Number of bytes copied to xhtml_buf
**
** Returns          BTA_PR_OK       if operation successful.
**                  BTA_PR_ERR_MEM  if not enough for holding xhtml data
**
*******************************************************************************/
BTA_API extern tBTA_PR_STATUS BTA_PrCal2Xhtml(char *p_xhtml_buf, UINT32 *p_len,
                                              tBTA_OP_PROP *p_props, UINT16 num_props);

/*******************************************************************************
**
** Function         BTA_PrNote2Xhtml
**
** Description      Convert array of vNote properties into xhtml buffer
**
** Parameters
**      Input:      p_xhtml_buf:    pointer to buffer for holding converted vcard
**                                      (if NULL, will return size of buffer
**                                      req'd to do the conversion in *p_len)
**                  p_len:          size of xhtml_buf
**                  p_props:        array of vCal properties
**                  num_props:      number of properties in p_prop array
**
**      Output:     p_len:          Number of bytes copied to xhtml_buf
**
** Returns          BTA_PR_OK       if operation successful.
**                  BTA_PR_ERR_MEM  if not enough for holding xhtml data
**
*******************************************************************************/
BTA_API extern tBTA_PR_STATUS BTA_PrNote2Xhtml(char *p_xhtml_buf, UINT32 *p_len,
                                               tBTA_OP_PROP *p_props, UINT16 num_props);

/*******************************************************************************
**
** Function         BTA_PrText2Xhtml
**
** Description      Convert a text buffer into xhtml buffer
**
** Parameters
**      Input:      p_xhtml_buf:    pointer to buffer for holding converted vcard
**                                      (if NULL, will return size of buffer
**                                      req'd to do the conversion in *p_len)
**                  p_len:          size of xhtml_buf
**                  p_text_buf:     array of vCal properties
**                  text_buflen:    size of p_text_buf
**                  show_frame:     if TRUE, a frame will be drawn around the text
**
**      Output:     p_len:          Number of bytes copied to xhtml_buf
**
** Returns          BTA_PR_OK       if operation successful.
**                  BTA_PR_ERR_MEM  if not enough for holding xhtml data
**
*******************************************************************************/
BTA_API extern tBTA_PR_STATUS BTA_PrText2Xhtml(char *p_xhtml_buf, UINT32 *p_len,
                                               UINT8 *p_text_buf, UINT32 text_buflen);

#ifdef __cplusplus
}
#endif

#endif /* BTA_PR_API_H */
