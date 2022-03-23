/*****************************************************************************
**
**  Name:           bta_ac_api.h
**
**  Description:    This is the public interface file for the Advanced Camera
**                  (AC) subsystem of BTA, Widcomm's
**                  Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2004-2004, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AC_API_H
#define BTA_AC_API_H

#include "bta_bi_api.h"

/*****************************************************************************
** response code definition
*****************************************************************************/
#define BTA_AC_RSP_OK                   OBX_RSP_OK                 /* 0x20 OK, Success */
#define BTA_AC_RSP_BAD_REQUEST          OBX_RSP_BAD_REQUEST        /* 0x40 Bad Request - server couldn't understand request */
#define BTA_AC_RSP_UNAUTHORIZED         OBX_RSP_UNAUTHORIZED       /* 0x41 Unauthorized */
#define BTA_AC_RSP_FORBIDDEN            OBX_RSP_FORBIDDEN          /* 0x43 Forbidden - operation is understood but refused */
#define BTA_AC_RSP_NOT_FOUND            OBX_RSP_NOT_FOUND          /* 0x44 Not Found */
#define BTA_AC_RSP_NOT_ACCEPTABLE       OBX_RSP_NOT_ACCEPTABLE     /* 0x46 Not Acceptable */
#define BTA_AC_RSP_PRECONDTN_FAILED     OBX_RSP_PRECONDTN_FAILED   /* 0x4C Precondition failed */
#define BTA_AC_RSP_NOT_IMPLEMENTED      OBX_RSP_NOT_IMPLEMENTED    /* 0x51 Not Implemented */
#define BTA_AC_RSP_SERVICE_UNAVL        OBX_RSP_SERVICE_UNAVL      /* 0x53 Service Unavailable */
typedef tOBX_RSP_CODE   tBTA_AC_RSP;

/*****************************************************************************
** definition from BIP
*****************************************************************************/
#define BTA_AC_DSP_NEXT_IMG     BIP_DSP_NEXT_IMG /* 0x01 NextImage */
#define BTA_AC_DSP_PREV_IMG     BIP_DSP_PREV_IMG /* 0x02 PreviousImage */
#define BTA_AC_DSP_SELT_IMG     BIP_DSP_SELT_IMG /* 0x03 SelectImage */
#define BTA_AC_DSP_CURR_IMG     BIP_DSP_CURR_IMG /* 0x04 CurrentImage */
typedef tBIP_DSP_OP             tBTA_AC_DSP_OP;
typedef tBIP_IMAGE_DESC         tBTA_AC_IMAGE_DESC;
typedef tBIP_IMG_HDL_STR        tBTA_AC_IMG_HDL;
typedef tBIP_IMAGE_HDL_DESC     tBTA_AC_IMG_HDL_DESC;
typedef tBIP_IMAGE_LIST_ELM     tBTA_AC_IMG_LIST_ELM;
typedef tBIP_IMAGING_CAPS       tBTA_AC_IMG_CAPS;
typedef tBIP_IMAGE_PROPERTIES   tBTA_AC_IMG_PROPERTIES;
typedef tBIP_SDP_PARAMS         tBTA_AC_SDP_PARAMS;
typedef tBIP_PUT_RSP_EVT        tBTA_AC_PUT_RSP_EVT;
typedef tBIP_LIST_REQ_EVT       tBTA_ACS_LIST_REQ_EVT;
typedef tBIP_GET_IMG_REQ_EVT    tBTA_ACS_IMG_REQ_EVT;
typedef tBIP_GET_ATTACH_EVT     tBTA_ACS_ATTACH_EVT;
typedef tBIP_PUT_IMAGE_EVT      tBTA_ACS_PUT_IMAGE_EVT;
typedef tBIP_PUT_ATTACH_EVT     tBTA_ACS_PUT_ATTACH_EVT;
typedef tBIP_RMT_DISP_REQ       tBTA_ACS_RMT_DISP_REQ;
typedef tBIP_GET_PART_REQ_EVT   tBTA_ACS_PART_REQ_EVT;

/*****************************************************************************
** definition from BTA BI
*****************************************************************************/
typedef tBTA_BI_STATUS      tBTA_AC_STATUS;
typedef tBTA_BI_PROGRESS    tBTA_AC_PROGRESS;
typedef tBTA_BI_XML         tBTA_AC_XML;
typedef tBTA_BIS_OPEN       tBTA_ACS_OPEN;

typedef tBTA_BI_GET_IMAGE_EVT   tBTA_ACC_GET_IMAGE_EVT;
typedef tBTA_BI_GET_THUMB_EVT   tBTA_ACC_GET_THUMB_EVT;
typedef tBTA_BI_ABORT_EVT       tBTA_ACC_ABORT_EVT;

/* callback function result */
#define BTA_AC_OK                   BTA_BI_OK               /* Request succeeded. */
#define BTA_AC_FAIL                 BTA_BI_FAIL             /* Request failed. */
#define BTA_AC_NOT_FOUND            BTA_BI_RESOURCES        /* Object not found. */
#define BTA_AC_NO_PERMISSION        BTA_BI_NO_PERMISSION    /* Operation not authorized. */
#define BTA_AC_NOT_SUPPORTED        BTA_BI_NOT_SUPPORTED    /* Image Format/encoding not supported. */

/*****************************************************************************
** Client definition
*****************************************************************************/
/* Client callback function event */
#define BTA_ACC_ENABLE_EVT          BTA_BIC_REGISTER_EVT    /* Advanced Camera client is enabled. */
#define BTA_ACC_OPEN_EVT            BTA_BIC_OPEN_EVT        /* Connection to peer is open. */
#define BTA_ACC_AUTH_EVT            BTA_BIC_AUTH_EVT        /* Challenged by the server. */
#define BTA_ACC_CLOSE_EVT           BTA_BIC_CLOSE_EVT       /* Connection to peer closed. */
#define BTA_ACC_PROGRESS_EVT        BTA_BIC_PROGRESS_EVT    /* Push/pull in progres */
#define BTA_ACC_PUT_RSP_EVT         BTA_BIC_PUT_RSP_EVT     /* Push complete */
#define BTA_ACC_SDP_RESULT_EVT      BTA_BIC_SDP_RESULT_EVT  /* SDP record of the responder */
#define BTA_ACC_THUMBNAIL_EVT       BTA_BIC_THUMBNAIL_EVT   /* Responder/server requests for thumbnail of this image handle. */
#define BTA_ACC_CAPABILITIES_EVT    BTA_BIC_CAPABILITIES_EVT/* Imaging capabilities of the responder */
#define BTA_ACC_IMG_LIST_EVT        BTA_BIC_IMG_LIST_EVT    /* Images listing from the responder */
#define BTA_ACC_PROPERTIES_EVT      BTA_BIC_PROPERTIES_EVT  /* Image properties */
#define BTA_ACC_GET_IMAGE_EVT       BTA_BIC_GET_IMAGE_EVT   /* Get Image  */
#define BTA_ACC_GET_THUMB_EVT       BTA_BIC_GET_THUMB_EVT   /* Get Image thumbnail */
#define BTA_ACC_GET_MONITOR_EVT     BTA_BIC_GET_MONITOR_EVT /* Get Monitoring Image  */
#define BTA_ACC_GET_STATUS_EVT      BTA_BIC_GET_STATUS_EVT  /* Get status */
#define BTA_ACC_ABORT_EVT           BTA_BIC_ABORT_EVT       /* Abort response */

typedef UINT8 tBTA_ACC_EVT;

/* Union of all client callback structures */
typedef tBTA_BIC tBTA_ACC;

/* callback function */
typedef void (tBTA_ACC_CBACK)(tBTA_ACC_EVT event, tBTA_ACC *p_data);

/*****************************************************************************
** Server definition
*****************************************************************************/
/* Server callback function event */
#define BTA_ACS_ENABLE_EVT          BTA_BIS_ENABLE_EVT      /* Advanced Camera client is enabled. */
#define BTA_ACS_OPEN_EVT            BTA_BIS_OPEN_EVT        /* Connection to peer is open. */
#define BTA_ACS_CLOSE_EVT           BTA_BIS_CLOSE_EVT       /* Connection to peer closed. */
#define BTA_ACS_PROGRESS_EVT        BTA_BIS_PROGRESS_EVT    /* Push/pull in progres */
#define BTA_ACS_PUT_FINAL_EVT       BTA_BIS_GET_PART_EVT    /* final packet of PUT transaction */
#define BTA_ACS_GET_LIST_EVT        BTA_BIS_GET_LIST_EVT    /* client requests for images listing */
#define BTA_ACS_GET_PROP_EVT        BTA_BIS_GET_PROP_EVT    /* client requests for image properties */
#define BTA_ACS_GET_IMAGE_EVT       BTA_BIS_GET_IMAGE_EVT   /* client requests for an image  */
#define BTA_ACS_GET_THUMB_EVT       BTA_BIS_GET_THUMB_EVT   /* client requests for the image thumbnail */
#define BTA_ACS_GET_ATTACH_EVT      BTA_BIS_GET_ATTACH_EVT  /* client requests for the image attachment */
#define BTA_ACS_GET_MONITOR_EVT     BTA_BIS_GET_MONITOR_EVT /* client requests for the monitoring image (for remote camera only) */
#define BTA_ACS_DEL_IMAGE_EVT       BTA_BIS_DEL_IMAGE_EVT   /* client requests to delete an image */
#define BTA_ACS_PUT_IMAGE_EVT       BTA_BIS_PUT_IMAGE_EVT   /* client requests to put an image */
#define BTA_ACS_PUT_THUMB_EVT       BTA_BIS_PUT_THUMB_EVT   /* client requests to put an image thumbnail */
#define BTA_ACS_PUT_ATTACH_EVT      BTA_BIS_PUT_ATTACH_EVT  /* client requests to put an image attachment */

typedef UINT8 tBTA_ACS_EVT;

typedef tBTA_BIS tBTA_ACS;

typedef void (tBTA_ACS_CBACK)(tBTA_ACS_EVT event, tBTA_ACS *p_data);

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/**************************
**  Client Functions
***************************/

/*******************************************************************************
**
** Function         BTA_AccEnable
**
** Description      Enable the Advanced Camera client.  This function must be
**                  called before any other functions in the ACC API are called.
**                  When the enable operation is complete the callback function
**                  will be called with a BTA_ACC_ENABLE_EVT.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccEnable(tBTA_ACC_CBACK *p_cback, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_AccDisable
**
** Description      Disable the advanced camera client.  If the client is currently
**                  connected to a peer device the connection will be closed.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccDisable(void);

/*******************************************************************************
**
** Function         BTA_AccClose
**
** Description      Close the current connection.  This function is called if
**                  the phone wishes to close the connection before the current
**                  operation is completed.  In a typical connection this function
**                  does not need to be called; the connection will be closed
**                  automatically when the current operation is complete.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccClose(void);

/*******************************************************************************
**
** Function         BTA_AccPutImage
**
** Description      Push an image to a peer device.
**                  p_name must point to a fully qualified path and file name.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccPutImage(UINT8 *p_name, tBTA_AC_IMAGE_DESC *p_param);

/*******************************************************************************
**
** Function         BTA_AccPutThumbnail
**
** Description      Push an image thumbnail to a peer device.
**                  p_name must point to a fully qualified path and file name.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccPutThumbnail(UINT8 *p_name, tBTA_AC_IMG_HDL img_hdl);


/*******************************************************************************
**
** Function         BTA_AccAutomaticArchive
**
** Description      Open the BIP connection to the archiving storage device.
**                  The connection stays open until BTA_AccDisable() or
**                  BTA_AccClose() or when the secondary channel is disconnected
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccAutomaticArchive(tBTA_SEC sec_mask,
                        BD_ADDR bd_addr, tBTA_ACS_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_AccGetStatus
**
** Description      Check the status of the archive activity.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccGetStatus(void);

/*******************************************************************************
**
** Function         BTA_AccRemoteCamera
**
** Description      Open the BIP connection to the Camera.
**                  The connection stays open until BTA_AccDisable() or BTA_AccClose()
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccRemoteCamera(tBTA_SEC sec_mask,
                        BD_ADDR bd_addr);

/*******************************************************************************
**
** Function         BTA_AccGetMonitorImage
**
** Description      Get the monitoring image.
**                  If the store is TRUE, the shutter on the responder is
**                  triggerred and an image handle is received in BTA_ACC_GET_MONITOR_EVT.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccGetMonitorImage(BOOLEAN store, UINT8 * p_name);

/*******************************************************************************
**
** Function         BTA_AccGetImageProperties
**
** Description      With the image handle from BTA_ACC_GET_MONITOR_EVT, learn the
**                  image properties of tne newly captured image
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccGetImageProperties(tBTA_AC_IMG_HDL img_hdl);

/*******************************************************************************
**
** Function         BTA_AccGetImage
**
** Description      With the image handle from BTA_ACC_GET_MONITOR_EVT, retrieve the
**                  newly captured image
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccGetImage(tBTA_AC_IMG_HDL img_hdl, UINT8 * p_name, tBIP_IMAGE_DESC *p_param);

/*******************************************************************************
**
** Function         BTA_AccGetThumbnail
**
** Description      With the image handle from BTA_ACC_GET_MONITOR_EVT, retrieve the
**                  associated image thumbnail of newly captured image
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccGetThumbnail(tBTA_AC_IMG_HDL img_hdl, UINT8 * p_name);

/*******************************************************************************
**
** Function         BTA_AccRemoteDisplay
**
** Description      Open the BIP connection to the remote display device.
**                  The connection stays open until BTA_AccDisable() or BTA_AccClose()
**
**                  This feature can use BTA_AccPutImage() to put the local
**                  images to the responder and collect the associated image
**                  handles.
**                  This feature can also use the images on the responder
**                  device and learn the image handles with
**                  BTA_AccGetImagesListing() function.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccRemoteDisplay(tBTA_SEC sec_mask, BD_ADDR bd_addr);

/*******************************************************************************
**
** Function         BTA_AccDisplayImage
**
** Description      Display the requested image
**                  The function is usually called with BTA_AC_DSP_SELT_IMG and an
**                  image handle first.
**                  After the first image is selected, we can use
**                  BTA_AC_DSP_NEXT_IMG or BTA_AC_DSP_PREV_IMG.
**                  param can also be BTA_AC_DSP_CURR_IMG to retrieve the current
**                  image handle.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccDisplayImage(UINT8 * p_img_hdl, tBTA_AC_DSP_OP param);

/*******************************************************************************
**
** Function         BTA_AccGetImagesListing
**
** Description      Get the images Listing (for the remote display feature)
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccGetImagesListing(UINT16 nb_ret_hdl,
                    UINT16 start_off, BOOLEAN last_cap, tBTA_AC_IMG_HDL_DESC *p_hdl_desc);

/*******************************************************************************
**
** Function         BTA_AccImagingCapabilities
**
** Description      Get the Imaging Capabilities of the connected responder
**                  The result is reported with BTA_ACC_CAPABILITIES_EVT
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccImagingCapabilities(void);

/*******************************************************************************
**
** Function         BTA_AccAuthRsp
**
** Description      Sends a response to an OBEX authentication challenge to the
**                  connected OBEX server. Called in response to an BTA_ACC_AUTH_EVT
**                  event.
**
**                  Note: If the "userid_required" is TRUE in the BTA_ACC_AUTH_EVT
**                        event, then p_userid is required, otherwise it is optional.
**
**                  p_password  must be less than GOEP_MAX_AUTH_KEY_SIZE (16 bytes)
**                  p_userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccAuthRsp (char *p_password, char *p_userid);

/*******************************************************************************
**
** Function         BTA_AccCheckImageFormat
**
** Description      Given the imaging capabilities, check if the image descriptor
**                  is supported.
**
** Returns          TRUE, if the image descriptor is supported.
**
 *******************************************************************************/
BTA_API extern BOOLEAN BTA_AccCheckImageFormat(tBTA_AC_IMG_CAPS *p_caps, tBTA_AC_IMAGE_DESC *p_desc);

/**************************
**  Server Functions
***************************/

/*******************************************************************************
**
** Function         BTA_AcsEnable
**
** Description      Enable the advanced camera server.  This function must be
**                  called before any other functions in the ACS API are called.
**                  When the enable operation is complete the callback function
**                  will be called with a BTA_ACS_ENABLE_EVT.
**                  p_cback and p_sdp must not be NULL.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsEnable(tBTA_SEC sec_mask, tBTA_AC_SDP_PARAMS *p_sdp,
                                  tBTA_ACS_CBACK *p_cback, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_AcsDisable
**
** Description      Disable the Advanced Camera server.  If the server is currently
**                  connected to a peer device the connection will be closed.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsDisable(void);

/*******************************************************************************
**
** Function         BTA_AcsClose
**
** Description      Close the current connection.  This function is called if
**                  the phone wishes to close the connection before the client
**                  disconnects.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsClose(void);

/*******************************************************************************
**
** Function         BTA_AcsImagesListingRsp
**
** Description      When the application receives BTA_ACS_GET_LIST_EVT, this
**                  function is used to send the response to the connected client.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsImagesListingRsp(UINT8 handle,
                        UINT16 nb_ret_hdl, /* The number of image handles returned in the list */
                        tBTA_AC_IMG_HDL_DESC *p_hdl_desc,   /* info to compose image handles descriptor XML object. */
                        tBTA_AC_IMG_LIST_ELM *p_list);

/*******************************************************************************
**
** Function         BTA_AcsImagePropertiesRsp
**
** Description      When the application receives BTA_ACS_GET_PROP_EVT, this
**                  function is used to send the response to the connected client.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsImagePropertiesRsp(UINT8 handle,
                        tBTA_AC_RSP rsp,        /* the OBEX response code */
                        tBTA_AC_IMG_PROPERTIES *p_prop);

/*******************************************************************************
**
** Function         BTA_AcsGetImageRsp
**
** Description      When the application receives BTA_ACS_GET_IMAGE_EVT, this
**                  function is used to send the response to the connected client.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsGetImageRsp(UINT8 handle,
                        tBTA_AC_RSP rsp_code, UINT8 *p_name);

/*******************************************************************************
**
** Function         BTA_AcsGetThumbRsp
**
** Description      When the application receives BTA_ACS_GET_THUMB_EVT, this
**                  function is used to send the response to the connected client.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsGetThumbRsp(UINT8 handle,
                        tBTA_AC_RSP rsp_code, UINT8 *p_name);

/*******************************************************************************
**
** Function         BTA_AcsGetAttachRsp
**
** Description      When the application receives BTA_ACS_GET_ATTACH_EVT, this
**                  function is used to send the response to the connected client.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsGetAttachRsp(UINT8 handle,
                        tBTA_AC_RSP rsp_code, UINT8 *p_name);

/*******************************************************************************
**
** Function         BTA_AcsGetMonitorImageRsp
**
** Description      When the application receives BTA_ACS_GET_MONITOR_EVT, this
**                  function is used to send the response to the connected client.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsGetMonitorImageRsp(UINT8 *p_img_hdl, UINT8 *p_name);

/*******************************************************************************
**
** Function         BTA_AcsDelImageRsp
**
** Description      When the application receives BTA_ACS_DEL_IMAGE_EVT, this
**                  function is used to send the response to the connected client.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsDelImageRsp(UINT8 handle, tBTA_AC_RSP rsp_code);

/*******************************************************************************
**
** Function         BTA_AcsPutImageRsp
**
** Description      When the application receives BTA_ACS_PUT_IMAGE_EVT, this
**                  function is used to send the response to the connected client.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsPutImageRsp(tBTA_AC_RSP rsp_code,
                                       UINT8 *p_img_hdl);

/*******************************************************************************
**
** Function         BTA_AcsPutThumbRsp
**
** Description      When the application receives BTA_ACS_PUT_THUMB_EVT, this
**                  function is used to send the response to the connected client.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsPutThumbRsp(tBTA_AC_RSP rsp_code);

/*******************************************************************************
**
** Function         BTA_AcsPutAttachRsp
**
** Description      When the application receives BTA_ACS_PUT_ATTACH_EVT, this
**                  function is used to send the response to the connected client.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsPutAttachRsp(tBTA_AC_RSP rsp_code);

/*******************************************************************************
**
** Function         BTA_AcsPutContinue
**
** Description      When the application receives BTA_ACS_PUT_*_EVT, this
**                  function is used to let BIP learn the file name to hold
**                  the received object from client.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AcsPutContinue(UINT8 handle, UINT8 *p_name);

/*******************************************************************************
**
** Function         BTA_AccImagePull
**
** Description      Open the BIP connection to the responder that supports the
**                  Image Pull feature.
**                  The connection stays open until BTA_AccDisable() or BTA_AccClose()
**
**                  This feature can use
**                  BTA_AccGetImagesListing() to rettrieve the image handle listing
**                  BTA_AccGetImageProperties() to rettrieve the image properties
**                      on each image handle reported in BTA_ACC_IMG_LIST_EVT
**                  BTA_AccGetImage() to rettrieve the image
**                      on each image handle reported in BTA_ACC_IMG_LIST_EVT
**                  BTA_AccGetThumbnail() to rettrieve the image thumbnail
**                      on each image handle reported in BTA_ACC_IMG_LIST_EVT
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccImagePull(tBTA_SEC sec_mask, BD_ADDR bd_addr);

/*******************************************************************************
**
** Function         BTA_AccAbort
**
** Description      Abort the current transaction.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_AccAbort(void);

#ifdef __cplusplus
}
#endif

#endif /* BTA_AI_API_H */
