/*****************************************************************************
**
**  Name:           bta_bi_api.h
**
**  Description:    This is the public interface file for the Basic Imaging
**                  (BI) subsystem of BTA, Widcomm's
**                  Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2004-2004, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_BI_API_H
#define BTA_BI_API_H

#include "bta_api.h"
#include "bip_api.h"

/* Extra Debug Code */
#ifndef BTA_BI_DEBUG
#define BTA_BI_DEBUG            FALSE
#endif

#define BTA_BIC_MAX_REG         3   /* FT, PR, AC */
#define BTA_BIC_INVALID_HANDLE  (BTA_BIC_MAX_REG)
#define BTA_BI_IS_HANDLE        (BTA_BIC_MAX_REG)
#define BTA_BI_RP_HANDLE        (BTA_BIC_MAX_REG + 1)

typedef struct
{
    tBIP_IMAGING_CAPS   *p_is_imaging_caps;
    tBIP_IMAGING_CAPS   *p_rp_imaging_caps;
    UINT8               *p_print_service_id;
    UINT8               *p_archive_service_id;
} tBTA_BI_CFG;

/* Client callback function event */
#define BTA_BIC_REGISTER_EVT         0   /* registered. */
#define BTA_BIC_OPEN_EVT             1   /* Connection to peer is open. */
#define BTA_BIC_AUTH_EVT             2   /* Challenged by the server. */
#define BTA_BIC_CLOSE_EVT            3   /* Connection to peer closed. */
#define BTA_BIC_PROGRESS_EVT         4   /* Push/pull in progres */
#define BTA_BIC_PUT_RSP_EVT          5   /* Push complete */
#define BTA_BIC_SDP_RESULT_EVT       6   /* SDP record of the responder */
#define BTA_BIC_THUMBNAIL_EVT        7   /* Responder/server requests for thumbnail of this image handle. */
#define BTA_BIC_CAPABILITIES_EVT     8   /* Imaging capabilities of the responder */
#define BTA_BIC_IMG_LIST_EVT         9   /* Images listing from the responder */
#define BTA_BIC_PROPERTIES_EVT      10   /* Get Image properties */
#define BTA_BIC_GET_IMAGE_EVT       11   /* Get Image  */
#define BTA_BIC_GET_THUMB_EVT       12   /* Get Image thumbnail */
#define BTA_BIC_GET_MONITOR_EVT     13   /* Get Monitoring Image  */
#define BTA_BIC_GET_STATUS_EVT      14   /* Get status */
#define BTA_BIC_ABORT_EVT           15   /* abort */
typedef UINT8 tBTA_BIC_EVT;


/* Server callback function events (also used for initiator secondary channel) */
#define BTA_BIS_ENABLE_EVT          0x10 /* server is enabled. */
#define BTA_BIS_OPEN_EVT            0x11 /* Connection to peer is open. */
#define BTA_BIS_CLOSE_EVT           0x12 /* Connection to peer closed. */
#define BTA_BIS_PROGRESS_EVT        0x13 /* Number of bytes read or written so far */
#define BTA_BIS_GET_PART_EVT        0x14 /* responder requests for partial image (for printing /w DPOF only) */
#define BTA_BIS_GET_LIST_EVT        0x15 /* client requests for images listing */
#define BTA_BIS_GET_PROP_EVT        0x16 /* client requests for image properties */
#define BTA_BIS_GET_IMAGE_EVT       0x17 /* client requests for an image  */
#define BTA_BIS_GET_THUMB_EVT       0x18 /* client requests for the image thumbnail */
#define BTA_BIS_GET_ATTACH_EVT      0x19 /* client requests for the image attachment */
#define BTA_BIS_GET_MONITOR_EVT     0x1A /* client requests for the monitoring image (for remote camera only) */
#define BTA_BIS_DEL_IMAGE_EVT       0x1B /* client requests to delete an image */
#define BTA_BIS_PUT_IMAGE_EVT       0x1C /* client requests to put an image */
#define BTA_BIS_PUT_THUMB_EVT       0x1D /* client requests to put an image thumbnail */
#define BTA_BIS_PUT_ATTACH_EVT      0x1E /* client requests to put an image attachment */

#define BTA_BI_INVALID_EVT          0x7F

typedef UINT8 tBTA_BIS_EVT;

/* callback function result */
#define BTA_BIC_OK                  0   /* Request succeeded. */
#define BTA_BIC_FAIL                1   /* Request failed. */
#define BTA_BIC_RESOURCES           2   /* Resources not available */
#define BTA_BIC_NO_PERMISSION       3   /* Operation not authorized. */
#define BTA_BIC_NOT_SUPPORTED       4   /* Image Format/encoding not supported. */

typedef UINT8 tBTA_BIC_STATUS;

#define BTA_BIS_OK                  BTA_BIC_OK
#define BTA_BIS_FAIL                BTA_BIC_FAIL
#define BTA_BIS_RESOURCES           BTA_BIC_RESOURCES
#define BTA_BIS_NO_PERMISSION       BTA_BIC_NO_PERMISSION
#define BTA_BIS_NOT_SUPPORTED       BTA_BIC_NOT_SUPPORTED

typedef UINT8 tBTA_BIS_STATUS;

typedef UINT8 tBTA_BI_STATUS;

typedef struct
{
    UINT32          obj_size;   /* Total size of object (BTA_FS_LEN_UNKNOWN if unknown) */
    UINT16          bytes;      /* Number of bytes read or written since last progress event */
} tBTA_BI_PROGRESS;

typedef struct
{
    BD_ADDR         addr;       /* Bluetooth address of the connected device */
    tBIP_SERVICE    service;    /* The BIP service for this connection */
} tBTA_BIS_OPEN;

typedef union
{
    tBIP_IMAGING_CAPS       caps;/* BTA_BIC_CAPABILITIES_EVT */
    tBIP_LIST_RSP_EVT       list;/* BTA_BIC_IMG_LIST_EVT */
    tBIP_IMAGE_PROPERTIES   prop;/* BTA_BIC_PROPERTIES_EVT */
} tBTA_BI_XML;

typedef struct
{
    UINT8           *p_file;    /* NThe file name associated with the aborte operation */
    tBTA_BI_STATUS  status;
} tBTA_BI_ABORT_EVT;

typedef struct
{
    UINT32              size;    /* The size of the image */
    tOBX_RSP_CODE       obx_rsp;
} tBTA_BI_GET_IMAGE_EVT;

typedef struct
{
    tBIP_IMG_HDL_STR    img_hdl;    /* The image for the thumbnail */
    tOBX_RSP_CODE       obx_rsp;
} tBTA_BI_GET_THUMB_EVT;

/* Union of all client callback structures */
typedef union
{
    UINT8                   bi_hdl; /* BTA_BIC_REGISTER_EVT */
    tBTA_BI_PROGRESS        prog;   /* BTA_BIC_PROGRESS_EVT */
    tBTA_BI_STATUS          status; /* BTA_BIC_CLOSE_EVT */
    tBIP_IMG_HDL_STR        img_hdl;/* BTA_BIC_GET_MONITOR_EVT */
    tBIP_PUT_RSP_EVT        put_rsp;/* BTA_BIC_PUT_RSP_EVT */
    tBTA_BI_XML             *p_xml; /* BTA_BIC_CAPABILITIES_EVT, BTA_BIC_IMG_LIST_EVT, BTA_BIC_PROPERTIES_EVT */
    tBTA_BI_GET_IMAGE_EVT   get_img;/* BTA_BIC_GET_IMAGE_EVT */
    tBTA_BI_GET_THUMB_EVT   get_thumb; /* BTA_BIC_THUMBNAIL_EVT */
    tOBX_RSP_CODE           obx_rsp;/* BTA_BIC_GET_STATUS_EVT */
    tBIP_AUTH_EVT           auth;   /* BTA_BIC_AUTH_EVT */
    tBTA_BI_ABORT_EVT       abort;  /* BTA_BIC_ABORT_EVT */
} tBTA_BIC;


/* Client callback function */
typedef void (tBTA_BIC_CBACK)(tBTA_BIC_EVT event, tBTA_BIC *p_data);

/* Union of all server callback structures */
typedef union
{
    UINT8                   bi_hdl; /* BTA_BIS_ENABLE_EVT */
    tBTA_BI_PROGRESS        prog;   /* BTA_BIS_PROGRESS_EVT */
    tBIP_IMG_HDL_STR        img_hdl;/* BTA_BIS_PUT_THUMB_EVT, BTA_BIS_DEL_IMAGE_EVT
                                     * BTA_BIS_GET_PROP_EVT, BTA_BIS_GET_THUMB_EVT  */
    tBTA_BIS_OPEN           open;   /* BTA_BIS_OPEN_EVT */
    tBIP_GET_PART_REQ_EVT   part;   /* BTA_BIS_GET_PART_EVT */
    tBIP_LIST_REQ_EVT       list;   /* BTA_BIS_GET_LIST_EVT */
    tBIP_GET_IMG_REQ_EVT    get_img;/* BTA_BIS_GET_IMAGE_EVT */
    tBIP_GET_ATTACH_EVT     get_att;/* BTA_BIS_GET_ATTACH_EVT */
    BOOLEAN                 store;  /* BTA_BIS_GET_MONITOR_EVT */
    tBIP_PUT_IMAGE_EVT      put_img;/* BTA_BIS_PUT_IMAGE_EVT */
    tBIP_PUT_ATTACH_EVT     put_att;/* BTA_BIS_PUT_ATTACH_EVT */
} tBTA_BIS;

/* Server callback function */
typedef void (tBTA_BIS_CBACK)(tBTA_BIS_EVT event, tBTA_BIS *p_data);


/**************************
**  Client Functions
***************************/

/*******************************************************************************
**
** Function         BTA_BicInit
**
** Description      Initialized the basic imaging client control block
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicInit(void);

/*******************************************************************************
**
** Function         BTA_BicRegister
**
** Description      Register to the basic imaging client.  This function must be
**                  called before any other functions in the BIC API are called.
**                  When the enable operation is complete the callback function
**                  will be called with a BTA_BIC_REGISTER_EVT.
**                  p_cback must not be NULL.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicRegister(tBTA_BIC_CBACK *p_cback, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_BicDeRegister
**
** Description      Deregister from the basic imaging client.  If the client is
**                  currently connected to a peer device the connection will be
**                  closed.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicDeRegister(UINT8 handle);

/*******************************************************************************
**
** Function         BTA_BicFreeXmlData
**
** Description      Free the XML data received in the following BIC events:
**                  BTA_BIC_CAPABILITIES_EVT, BTA_BIC_IMG_LIST_EVT, or
**                  BTA_BIC_PROPERTIES_EVT
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicFreeXmlData(tBTA_BIC_EVT event, tBTA_BI_XML **pp_xml);

/*******************************************************************************
**
** Function         BTA_BicImagePush
**
** Description      Open the BIP connection with the Image Push feature.
**                  The connection stays open until BTA_BicClose() or
**                  BTA_BicDeRegister()
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicImagePush(UINT8 handle, tBTA_SEC sec_mask,
                                    BD_ADDR bd_addr, UINT8 scn);

/*******************************************************************************
**
** Function         BTA_BicPutImage
**
** Description      Push an image to a peer device.
**                  p_name must point to a fully qualified path and file name.
**                  p_param must not be NULL.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicPutImage(UINT8 handle,
                                    UINT8 *p_name, tBIP_IMAGE_DESC *p_param);

/*******************************************************************************
**
** Function         BTA_BicPutThumbnail
**
** Description      Push an image thumbnail to a peer device.
**                  p_name must point to a fully qualified path and file name.
**                  img_hdl must be the image handle given in the
**                      BTA_BIC_THUMBNAIL_EVT event
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicPutThumbnail(UINT8 handle, UINT8 *p_name, tBIP_IMG_HDL_STR img_hdl);

/*******************************************************************************
**
** Function         BTA_BicAdvancedPrint
**
** Description      Open the BIP connection with the Advanced Image Print feature.
**                  The connection stays open until BTA_BicClose() or
**                  BTA_BicDeRegister()
**
**                  The BIP secondary channel server will listen for the connect
**                  request from the printer as a result of this function.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicAdvancedPrint(UINT8 handle, tBTA_SEC sec_mask,
                         BD_ADDR bd_addr, UINT8 scn, tBTA_BIS_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_BicPrintDpof
**
** Description      Starts Advanced Image Printing by putting a DPOF file
**                  to the responder already connected with the Advanced Image
**                  Print feature
**
**                  p_name must point to a fully qualified path and file name.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicPrintDpof(UINT8 handle, UINT8 *p_name, tBTA_BIS_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_BicGetStatus
**
** Description      Check the status of the archive or advanced print activity.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicGetStatus(UINT8 handle);

/*******************************************************************************
**
** Function         BTA_BicImagingCapabilities
**
** Description      Get the Imaging Capabilities from the responder
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicImagingCapabilities(UINT8 handle);

/*******************************************************************************
**
** Function         BTA_BicAuthRsp
**
** Description      Sends a response to an OBEX authentication challenge to the
**                  connected OBEX server. Called in response to an BTA_BIC_AUTH_EVT
**                  event.
**
**                  Note: If the "userid_required" is TRUE in the BTA_BIC_AUTH_EVT
**                        event, then p_userid is required, otherwise it is optional.
**
**                  p_password  must be less than GOEP_MAX_AUTH_KEY_SIZE (16 bytes)
**                  p_userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicAuthRsp (UINT8 handle, char *p_password, char *p_userid);

/*******************************************************************************
**
** Function         BTA_BicClose
**
** Description      Close the current connection.  This function is called if
**                  the phone wishes to close the connection before the operation
**                  is completed.  In a typical connection this function
**                  does not need to be called; the connection will be closed
**                  automatically when the operation is complete.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicClose(UINT8 handle);

/*******************************************************************************
**
** Function         BTA_BicAbort
**
** Description      Abort the current transaction.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BicAbort(UINT8 handle);

/*******************************************************************************
**
** Function         BTA_BicCheckImageFormat
**
** Description      Given the imaging capabilities, check if the image descriptor
**                  is supported.
**
** Returns          TRUE, if the image descriptor is supported.
**
 *******************************************************************************/
BTA_API extern BOOLEAN BTA_BicCheckImageFormat(tBIP_IMAGING_CAPS *p_caps,
                                               tBIP_IMAGE_DESC *p_desc);

/**************************
**  Server Functions
***************************/
/*******************************************************************************
**
** Function         BTA_BisGetObjRsp
**
** Description      A common function for
**                  BTA_BisPartialImageRsp, BTA_AcsGetImageRsp,
**                  BTA_AcsGetThumbRsp, and BTA_AcsGetAttachRsp
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BisGetObjRsp(UINT8 handle, UINT8 bip_op,
                                     tOBX_RSP_CODE rsp_code, UINT8 *p_name);

/*******************************************************************************
**
** Function         BTA_BisPartialImageRsp
**
** Description      When printer (responder) receives a DPOF file, it opens a
**                  secondary channel and requests portions of the images in the
**                  DPOF file.
**                  When the application receives BTA_BI_GET_PART_SEVT, this
**                  function is used to send the requested partial image to responder.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_BisPartialImageRsp(UINT8 handle,
                        tOBX_RSP_CODE rsp_code, UINT8 *p_name);

#endif /* BTA_BI_API_H */
