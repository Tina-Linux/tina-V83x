/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : layerControl_de.cpp
* Description : display DE -- for H3
* History :
*/


#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include "cdx_config.h"
#include <cdx_log.h>
#include "layerControl.h"

//#define GLOBAL_LOG_LEVEL    LOG_LEVEL_VERBOSE

#include <CdxIon.h>
//#include "display_H3.h"
#include "sunxi_display2.h"

#include "iniparserapi.h"
#define SAVE_PIC (0)

#define GPU_BUFFER_NUM 32

static VideoPicture* gLastPicture = NULL;

#define BUF_MANAGE (0)

#define NUM_OF_PICTURES_KEEP_IN_NODE (GetConfigParamterInt("pic_4list_num", 3)+1)

int LayerCtrlHideVideo(LayerCtrl* l);

typedef struct VPictureNode_t VPictureNode;
struct VPictureNode_t
{
    VideoPicture* pPicture;
    VPictureNode* pNext;
    int           bUsed;
};

typedef struct BufferInfoS
{
    VideoPicture pPicture;
    int          nUsedFlag;
}BufferInfoT;

typedef struct LayerContext
{
    LayerCtrl            base;
    enum EPIXELFORMAT    eDisplayPixelFormat;
    int                  nWidth;
    int                  nHeight;
    int                  nLeftOff;
    int                  nTopOff;
    int                  nDisplayWidth;
    int                  nDisplayHeight;

    int                  bHoldLastPictureFlag;
    int                  bVideoWithTwoStreamFlag;
    int                  bIsSoftDecoderFlag;

    int                  bLayerInitialized;
    int                  bProtectFlag;

    void*                pUserData;

    //* use when render derect to hardware layer.
    VPictureNode*        pPictureListHeader;
    VPictureNode         picNodes[16];

    int                  nGpuBufferCount;
    BufferInfoT          mBufferInfo[GPU_BUFFER_NUM];
    int                  bLayerShowed;

    int                  fdDisplay;
    int                  nScreenWidth;
    int                  nScreenHeight;

}LayerContext;

//* set usage, scaling_mode, pixelFormat, buffers_geometry, buffers_count, crop
static int setLayerBuffer(LayerContext* lc)
{
    logd("setLayerBuffer: PixelFormat(%d), nW(%d), nH(%d), leftoff(%d), topoff(%d)",
          lc->eDisplayPixelFormat,lc->nWidth,
          lc->nHeight,lc->nLeftOff,lc->nTopOff);
    logd("setLayerBuffer: dispW(%d), dispH(%d), buffercount(%d), bProtectFlag(%d),\
          bIsSoftDecoderFlag(%d)",
          lc->nDisplayWidth,lc->nDisplayHeight,lc->nGpuBufferCount,
          lc->bProtectFlag,lc->bIsSoftDecoderFlag);

    int          pixelFormat;
    unsigned int nGpuBufWidth;
    unsigned int nGpuBufHeight;
    int i = 0;
    char* pVirBuf;
    char* pPhyBuf;

    nGpuBufWidth  = lc->nWidth;  //* restore nGpuBufWidth to mWidth;
    nGpuBufHeight = lc->nHeight;

    //* We should double the height if the video with two stream,
    //* so the nativeWindow will malloc double buffer
    if(lc->bVideoWithTwoStreamFlag == 1)
    {
        nGpuBufHeight = 2*nGpuBufHeight;
    }

    if(lc->nGpuBufferCount <= 0)
    {
        loge("error: the lc->nGpuBufferCount[%d] is invalid!",lc->nGpuBufferCount);
        return -1;
    }

    for(i=0; i<lc->nGpuBufferCount; i++)
    {
        pVirBuf = CdxIonPalloc(nGpuBufWidth * nGpuBufHeight * 3/2);
        pPhyBuf = CdxIonVir2Phy(pVirBuf);
        lc->mBufferInfo[i].nUsedFlag    = 0;

        lc->mBufferInfo[i].pPicture.nWidth  = lc->nWidth;
        lc->mBufferInfo[i].pPicture.nHeight = lc->nHeight;
        lc->mBufferInfo[i].pPicture.nLineStride  = lc->nWidth;
        lc->mBufferInfo[i].pPicture.pData0       = pVirBuf;
        lc->mBufferInfo[i].pPicture.pData1       = pVirBuf + (lc->nHeight * lc->nWidth);
        lc->mBufferInfo[i].pPicture.pData2       =
                lc->mBufferInfo[i].pPicture.pData1 + (lc->nHeight * lc->nWidth)/4;
        lc->mBufferInfo[i].pPicture.phyYBufAddr  = (uintptr_t)pPhyBuf;
        lc->mBufferInfo[i].pPicture.phyCBufAddr  =
                lc->mBufferInfo[i].pPicture.phyYBufAddr + (lc->nHeight * lc->nWidth);
        lc->mBufferInfo[i].pPicture.nBufId       = i;
        lc->mBufferInfo[i].pPicture.ePixelFormat = lc->eDisplayPixelFormat;

        logd("=== init id:%d pVirBuf: %p", i, pVirBuf);
    }

    return 0;
}

static int SetLayerParam(LayerContext* lc, VideoPicture* pPicture)
{
    disp_layer_config config;
    unsigned int     args[4];
    int ret = 0;
    //* close the layer first, otherwise, in case when last frame is kept showing,
    //* the picture showed will not valid because parameters changed.
    memset(&config.info, 0, sizeof(disp_layer_info));
    if(lc->bLayerShowed == 1)
    {
        lc->bLayerShowed = 0;
        //TO DO.
    }

    //* transform pixel format.
    switch(lc->eDisplayPixelFormat)
    {
        case PIXEL_FORMAT_YUV_PLANER_420:
            config.info.fb.format = DISP_FORMAT_YUV420_P;
            config.info.fb.addr[0] = (unsigned long )
                                        CdxIonVir2Phy(pPicture->pData0);
            config.info.fb.addr[1] = (unsigned long )
                                        CdxIonVir2Phy(pPicture->pData1);
            config.info.fb.addr[2] = (unsigned long )
                                        CdxIonVir2Phy(pPicture->pData2);
            config.info.fb.size[0].width     = lc->nWidth;
            config.info.fb.size[0].height    = lc->nHeight;
            config.info.fb.size[1].width     = lc->nWidth/2;
            config.info.fb.size[1].height    = lc->nHeight/2;
            config.info.fb.size[2].width     = lc->nWidth/2;
            config.info.fb.size[2].height    = lc->nHeight/2;
        break;

        case PIXEL_FORMAT_YV12:
            config.info.fb.format = DISP_FORMAT_YUV420_P;
            config.info.fb.addr[0]  = (unsigned long )
                                        CdxIonVir2Phy(pPicture->pData0);
            config.info.fb.addr[1]  = (unsigned long )
                                        CdxIonVir2Phy(pPicture->pData1);
            config.info.fb.addr[2]  = (unsigned long )
                                       CdxIonVir2Phy(pPicture->pData2);
            config.info.fb.size[0].width     = lc->nWidth;
            config.info.fb.size[0].height    = lc->nHeight;
            config.info.fb.size[1].width     = lc->nWidth/2;
            config.info.fb.size[1].height    = lc->nHeight/2;
            config.info.fb.size[2].width     = lc->nWidth/2;
            config.info.fb.size[2].height    = lc->nHeight/2;
        break;

        case PIXEL_FORMAT_NV12:
            config.info.fb.format = DISP_FORMAT_YUV420_SP_UVUV;
            config.info.fb.addr[0] = (unsigned long )
                                        CdxIonVir2Phy(pPicture->pData0);
            config.info.fb.addr[1] = (unsigned long )
                                        CdxIonVir2Phy(pPicture->pData1);
            config.info.fb.size[0].width     = lc->nWidth;
            config.info.fb.size[0].height    = lc->nHeight;

            config.info.fb.size[1].width     = lc->nWidth/2;
            config.info.fb.size[1].height    = lc->nHeight/2;
        break;

        case PIXEL_FORMAT_NV21:
            config.info.fb.format = DISP_FORMAT_YUV420_SP_VUVU;
            config.info.fb.addr[0] = (unsigned long )
                                        CdxIonVir2Phy(pPicture->pData0);
            config.info.fb.addr[1] = (unsigned long )
                                        CdxIonVir2Phy(pPicture->pData1);
            config.info.fb.size[0].width     = lc->nWidth;
            config.info.fb.size[0].height    = lc->nHeight;
            config.info.fb.size[1].width     = lc->nWidth/2;
            config.info.fb.size[1].height    = lc->nHeight/2;
        break;

        default:
        {
            loge("unsupported pixel format.");
            return -1;
        }
    }

    //* initialize the layerInfo.
    config.info.mode            = LAYER_MODE_BUFFER;
    config.info.zorder          = 1;
    config.info.alpha_mode      = 1;
    config.info.alpha_mode      = 0xff;

    //* image size.
    config.info.fb.crop.x = (unsigned long long)lc->nLeftOff << 32;
    config.info.fb.crop.y = (unsigned long long)lc->nTopOff << 32;
    config.info.fb.crop.width   = (unsigned long long)lc->nDisplayWidth << 32;
    config.info.fb.crop.height  = (unsigned long long)lc->nDisplayHeight << 32;
    config.info.fb.color_space  = (lc->nHeight < 720) ? DISP_BT601 : DISP_BT709;

    //* source window.
    /*
    config.info.screen_win.x        = 0;
    config.info.screen_win.y        = 0;
    config.info.screen_win.width    = lc->nScreenWidth;
    config.info.screen_win.height   = lc->nScreenHeight;
    */
    if(lc->nScreenWidth * lc->nDisplayHeight < lc->nScreenHeight * lc->nDisplayWidth)
    {
        config.info.screen_win.x        = 0;
        config.info.screen_win.y        = (lc->nScreenHeight - lc->nScreenWidth * lc->nDisplayHeight / lc->nDisplayWidth) / 2;
        config.info.screen_win.width    = lc->nScreenWidth;
        config.info.screen_win.height   = lc->nScreenWidth * lc->nDisplayHeight / lc->nDisplayWidth;
    }
    else if(lc->nScreenWidth * lc->nDisplayHeight > lc->nScreenHeight * lc->nDisplayWidth)
    {
        config.info.screen_win.x        = (lc->nScreenWidth - lc->nScreenHeight * lc->nDisplayWidth / lc->nDisplayHeight) / 2;
        config.info.screen_win.y        = 0;
        config.info.screen_win.width    = lc->nScreenHeight * lc->nDisplayWidth / lc->nDisplayHeight;
        config.info.screen_win.height   = lc->nScreenHeight;
    }
    else
    {
        config.info.screen_win.x        = 0;
        config.info.screen_win.y        = 0;
        config.info.screen_win.width    = lc->nScreenWidth;
        config.info.screen_win.height   = lc->nScreenHeight;
    }

    config.info.alpha_mode          = 1;
    config.info.alpha_value         = 0xff;

    config.channel = 0;
    config.enable = 1;
    config.layer_id = 0;
    //* set layerInfo to the driver.
    args[0] = 0;
    args[1] = (unsigned int)(&config);
    args[2] = 1;

    ret = ioctl(lc->fdDisplay, DISP_LAYER_SET_CONFIG, (void*)args);
    if(0 != ret)
    loge("fail to set layer info!");

    return 0;
}


static int __LayerReset(LayerCtrl* l)
{
    LayerContext* lc;
    int i;

    logd("LayerInit.");

    lc = (LayerContext*)l;

    for(i=0; i<lc->nGpuBufferCount; i++)
    {
        CdxIonPfree(lc->mBufferInfo[i].pPicture.pData0);
    }

    return 0;
}


static void __LayerRelease(LayerCtrl* l)
{
    LayerContext* lc;
    int i;

    lc = (LayerContext*)l;

    logv("Layer release");
    for(i=0; i<lc->nGpuBufferCount; i++)
    {
        CdxIonPfree(lc->mBufferInfo[i].pPicture.pData0);
    }

    disp_layer_config config;
    unsigned int args[4];
    memset(&config.info, 0, sizeof(disp_layer_info));
    config.channel = 0;
    config.enable = 0;
    config.layer_id = 0;
    //* set layerInfo to the driver.
    args[0] = 0;
    args[1] = (unsigned int)(&config);
    args[2] = 1;

    ioctl(lc->fdDisplay, DISP_LAYER_SET_CONFIG, (void*)args);
}

static void __LayerDestroy(LayerCtrl* l)
{
    LayerContext* lc;
    int i;

    lc = (LayerContext*)l;

    if(lc->fdDisplay>=0)
        close(lc->fdDisplay);
    lc->fdDisplay=-1;
    CdxIonClose();
    free(lc);
}


static int __LayerSetDisplayBufferSize(LayerCtrl* l, int nWidth, int nHeight)
{
    LayerContext* lc;

    lc = (LayerContext*)l;

    lc->nWidth         = nWidth;
    lc->nHeight        = nHeight;
    lc->nDisplayWidth  = nWidth;
    lc->nDisplayHeight = nHeight;
    lc->nLeftOff       = 0;
    lc->nTopOff        = 0;
    lc->bLayerInitialized = 0;

    if(lc->bVideoWithTwoStreamFlag == 1)
    {
        //* display the whole buffer region when 3D
        //* as we had init align-edge-region to black. so it will be look ok.
        int nScaler = 2;
        lc->nDisplayHeight = lc->nDisplayHeight*nScaler;
    }

    return 0;
}


//* Description: set initial param -- display region
static int __LayerSetDisplayRegion(LayerCtrl* l, int nLeftOff, int nTopOff,
                                        int nDisplayWidth, int nDisplayHeight)
{
    LayerContext* lc;

    lc = (LayerContext*)l;
    logv("Layer set display region, leftOffset = %d, topOffset = %d, displayWidth = %d, \
              displayHeight = %d",
            nLeftOff, nTopOff, nDisplayWidth, nDisplayHeight);

    if(nDisplayWidth == 0 && nDisplayHeight == 0)
    {
        return -1;
    }

    lc->nDisplayWidth     = nDisplayWidth;
    lc->nDisplayHeight    = nDisplayHeight;
    lc->nLeftOff          = nLeftOff;
    lc->nTopOff           = nTopOff;

    if(lc->bVideoWithTwoStreamFlag == 1)
    {
        //* display the whole buffer region when 3D
        //* as we had init align-edge-region to black. so it will be look ok.
        int nScaler = 2;
        lc->nDisplayHeight = lc->nHeight*nScaler;
    }

    return 0;
}

//* Description: set initial param -- display pixelFormat
static int __LayerSetDisplayPixelFormat(LayerCtrl* l, enum EPIXELFORMAT ePixelFormat)
{
    LayerContext* lc;

    lc = (LayerContext*)l;
    logd("Layer set expected pixel format, format = %d", (int)ePixelFormat);

    if(ePixelFormat == PIXEL_FORMAT_NV12 ||
       ePixelFormat == PIXEL_FORMAT_NV21 ||
       ePixelFormat == PIXEL_FORMAT_YV12)           //* add new pixel formats supported by gpu here.
    {
        lc->eDisplayPixelFormat = ePixelFormat;
    }
    else
    {
        logv("receive pixel format is %d, not match.", lc->eDisplayPixelFormat);
        return -1;
    }

    return 0;
}

//* Description: set initial param -- deinterlace flag
static int __LayerSetDeinterlaceFlag(LayerCtrl* l,int bFlag)
{
    LayerContext* lc;
    (void)bFlag;
    lc = (LayerContext*)l;

    return 0;
}

//* Description: set buffer timestamp -- set this param every frame
static int __LayerSetBufferTimeStamp(LayerCtrl* l, int64_t nPtsAbs)
{
    LayerContext* lc;
    (void)nPtsAbs;

    lc = (LayerContext*)l;

    return 0;
}

static int __LayerGetRotationAngle(LayerCtrl* l)
{
    LayerContext* lc;
    int nRotationAngle = 0;

    lc = (LayerContext*)l;

    return 0;
}

static int __LayerCtrlShowVideo(LayerCtrl* l)
{
    LayerContext* lc;
    int               i;

    lc = (LayerContext*)l;

    lc->bLayerShowed = 1;

    return 0;
}

static int __LayerCtrlHideVideo(LayerCtrl* l)
{
    LayerContext* lc;
    int               i;

    lc = (LayerContext*)l;

    lc->bLayerShowed = 0;

    return 0;
}

static int __LayerCtrlIsVideoShow(LayerCtrl* l)
{
    LayerContext* lc;

    lc = (LayerContext*)l;

     return lc->bLayerShowed;
}

static int  __LayerCtrlHoldLastPicture(LayerCtrl* l, int bHold)
{
    logd("LayerCtrlHoldLastPicture, bHold = %d", bHold);

    LayerContext* lc;
    lc = (LayerContext*)l;

    return 0;
}

static int __LayerDequeueBuffer(LayerCtrl* l, VideoPicture** ppVideoPicture, int bInitFlag)
{
    LayerContext* lc;
    int i = 0;
    VPictureNode*     nodePtr;
    BufferInfoT bufInfo;
    VideoPicture* pPicture = NULL;

    lc = (LayerContext*)l;

    if(lc->bLayerInitialized == 0)
    {
        if(setLayerBuffer(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }

        lc->bLayerInitialized = 1;
    }

    if(bInitFlag == 1)
    {
        for(i = 0; i < lc->nGpuBufferCount; i++)
        {
            if(lc->mBufferInfo[i].nUsedFlag == 0)
            {
                //* set the buffer address
                pPicture = *ppVideoPicture;
                pPicture = &lc->mBufferInfo[i].pPicture;

                lc->mBufferInfo[i].nUsedFlag = 1;
                break;
            }
        }
    }
    else
    {
        if(lc->pPictureListHeader != NULL)
        {
            nodePtr = lc->pPictureListHeader;
            i=1;
            while(nodePtr->pNext != NULL)
            {
                i++;
                nodePtr = nodePtr->pNext;
            }

           unsigned int     args[3];
                    //* return one picture.
            if(i > GetConfigParamterInt("pic_4list_num", 3))
            {
                nodePtr = lc->pPictureListHeader;
                lc->pPictureListHeader = lc->pPictureListHeader->pNext;
                pPicture = nodePtr->pPicture;
                nodePtr->bUsed = 0;
                i--;

            }
        }
    }

    *ppVideoPicture = pPicture;
    if(pPicture)
    {
        logv("** dequeue  pPicture(%p), id(%d)", pPicture, pPicture->nBufId);
        return 0;
    }
    else
    {
        logv("** dequeue  pPicture(%p)", pPicture);
        return -1;
    }
}

// this method should block here,
static int __LayerQueueBuffer(LayerCtrl* l, VideoPicture* pBuf, int bValid)
{
    LayerContext* lc  = NULL;

    int               i;
    VPictureNode*     newNode;
    VPictureNode*     nodePtr;
    BufferInfoT    bufInfo;

    lc = (LayerContext*)l;

    if(pBuf)
        logv("** queue , pPicture(%p), id(%d)", pBuf, pBuf->nBufId);
    else
        logv("** queue , pPicture(%p)", pBuf);

    if(lc->bLayerInitialized == 0)
    {
        if(setLayerBuffer(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }

        lc->bLayerInitialized = 1;
    }

    if(bValid != 0 && SetLayerParam(lc, pBuf) != 0)
    {
        loge("can not initialize layer.");
        return -1;
    }

    // *****************************************
    // *****************************************
    //  TODO: Display this video picture here () blocking
    //
    // *****************************************

    newNode = NULL;
    for(i = 0; i<NUM_OF_PICTURES_KEEP_IN_NODE; i++)
    {
        if(lc->picNodes[i].bUsed == 0)
        {
            newNode = &lc->picNodes[i];
            newNode->pNext = NULL;
            newNode->bUsed = 1;
            newNode->pPicture = pBuf;
            break;
        }
    }
    if(i == NUM_OF_PICTURES_KEEP_IN_NODE)
    {
        loge("*** picNode is full when queue buffer");
        return -1;
    }

    if(lc->pPictureListHeader != NULL)
    {
        nodePtr = lc->pPictureListHeader;
        while(nodePtr->pNext != NULL)
        {
            nodePtr = nodePtr->pNext;
        }
        nodePtr->pNext = newNode;
    }
    else
    {
        lc->pPictureListHeader = newNode;
    }

    return 0;
}

static int __LayerSetDisplayBufferCount(LayerCtrl* l, int nBufferCount)
{
    LayerContext* lc;

    lc = (LayerContext*)l;

    logv("LayerSetBufferCount: count = %d",nBufferCount);

    lc->nGpuBufferCount = nBufferCount;

    if(lc->nGpuBufferCount > GPU_BUFFER_NUM)
        lc->nGpuBufferCount = GPU_BUFFER_NUM;

    return lc->nGpuBufferCount;
}

static int __LayerGetBufferNumHoldByGpu(LayerCtrl* l)
{
    (void)l;
    return GetConfigParamterInt("pic_4list_num", 3);
}

static int __LayerGetDisplayFPS(LayerCtrl* l)
{
    (void)l;
    return 60;
}

static void __LayerResetNativeWindow(LayerCtrl* l,void* pNativeWindow)
{
    logd("LayerResetNativeWindow : %p ",pNativeWindow);

    LayerContext* lc;
    VideoPicture mPicBufInfo;

    lc = (LayerContext*)l;
    lc->bLayerInitialized = 0;

    return ;
}

static VideoPicture* __LayerGetBufferOwnedByGpu(LayerCtrl* l)
{
    LayerContext* lc;
    VideoPicture* pPicture = NULL;
    BufferInfoT bufInfo;

    lc = (LayerContext*)l;
    int i;
    VPictureNode* nodePtr;
    if(lc->pPictureListHeader != NULL)
    {
        nodePtr = lc->pPictureListHeader;
        lc->pPictureListHeader = lc->pPictureListHeader->pNext;
        pPicture = nodePtr->pPicture;
        nodePtr->bUsed = 0;
    }
    return pPicture;
}

static int __LayerSetVideoWithTwoStreamFlag(LayerCtrl* l, int bVideoWithTwoStreamFlag)
{
    LayerContext* lc;

    lc = (LayerContext*)l;

    logv("LayerSetIsTwoVideoStreamFlag, flag = %d",bVideoWithTwoStreamFlag);
    lc->bVideoWithTwoStreamFlag = bVideoWithTwoStreamFlag;

    return 0;
}

static int __LayerSetIsSoftDecoderFlag(LayerCtrl* l, int bIsSoftDecoderFlag)
{
    LayerContext* lc;

    lc = (LayerContext*)l;

    logv("LayerSetIsSoftDecoderFlag, flag = %d",bIsSoftDecoderFlag);
    lc->bIsSoftDecoderFlag = bIsSoftDecoderFlag;

    return 0;
}

//* Description: the picture buf is secure
static int __LayerSetSecure(LayerCtrl* l, int bSecure)
{
    logv("__LayerSetSecure, bSecure = %d", bSecure);
    //*TODO
    LayerContext* lc;

    lc = (LayerContext*)l;

    lc->bProtectFlag = bSecure;

    return 0;
}

static int __LayerReleaseBuffer(LayerCtrl* l, VideoPicture* pPicture)
{
    logv("***LayerReleaseBuffer");
    LayerContext* lc;

    lc = (LayerContext*)l;

    CdxIonPfree(pPicture->pData0);
    return 0;
}

static int __LayerControl(LayerCtrl* l, int cmd, void *para)
{
    LayerContext *lc = (LayerContext*)l;

    CDX_UNUSE(para);

    switch(cmd)
    {
        default:
            break;
    }
    return 0;
}

static LayerControlOpsT mLayerControlOps =
{
    release:                    __LayerRelease                   ,

    setSecureFlag:              __LayerSetSecure                 ,
    setDisplayBufferSize:       __LayerSetDisplayBufferSize      ,
    setDisplayBufferCount:      __LayerSetDisplayBufferCount     ,
    setDisplayRegion:           __LayerSetDisplayRegion          ,
    setDisplayPixelFormat:      __LayerSetDisplayPixelFormat     ,
    setVideoWithTwoStreamFlag:  __LayerSetVideoWithTwoStreamFlag ,
    setIsSoftDecoderFlag:       __LayerSetIsSoftDecoderFlag      ,
    setBufferTimeStamp:         __LayerSetBufferTimeStamp        ,

    resetNativeWindow :         __LayerResetNativeWindow         ,
    getBufferOwnedByGpu:        __LayerGetBufferOwnedByGpu       ,
    getDisplayFPS:              __LayerGetDisplayFPS             ,
    getBufferNumHoldByGpu:      __LayerGetBufferNumHoldByGpu     ,

    ctrlShowVideo :             __LayerCtrlShowVideo             ,
    ctrlHideVideo:              __LayerCtrlHideVideo             ,
    ctrlIsVideoShow:            __LayerCtrlIsVideoShow           ,
    ctrlHoldLastPicture :       __LayerCtrlHoldLastPicture       ,

    dequeueBuffer:              __LayerDequeueBuffer             ,
    queueBuffer:                __LayerQueueBuffer               ,
    releaseBuffer:              __LayerReleaseBuffer             ,
    reset:                      __LayerReset                     ,
    destroy:                    __LayerDestroy                   ,
    control:                    __LayerControl
};

LayerCtrl* LayerCreate_DE()
{
    LayerContext* lc;
    unsigned int args[4];
    disp_layer_info layerInfo;

    int screen_id;
    disp_output_type output_type;

    logd("LayerCreate.");

    lc = (LayerContext*)malloc(sizeof(LayerContext));
    if(lc == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(lc, 0, sizeof(LayerContext));

    lc->fdDisplay = open("/dev/disp", O_RDWR);
    if(lc->fdDisplay < 1)
    {
        loge("open disp failed");
        free(lc);
        return NULL;
    }

    lc->base.ops = &mLayerControlOps;
    lc->eDisplayPixelFormat = PIXEL_FORMAT_YV12;

    for(screen_id=0;screen_id<3;screen_id++)
    {
        args[0] = screen_id;
        output_type = (disp_output_type)ioctl(lc->fdDisplay, DISP_GET_OUTPUT_TYPE, (void*)args);
        logi("screen_id = %d,output_type = %d",screen_id ,output_type);
        if((output_type != DISP_OUTPUT_TYPE_NONE) && (output_type != -1))
        {
            logi("the output type: %d",screen_id);
            break;
        }
    }

    memset(&layerInfo, 0, sizeof(disp_layer_info));

    //  get screen size.
    args[0] = 0;
    lc->nScreenWidth  = ioctl(lc->fdDisplay, DISP_GET_SCN_WIDTH, (void *)args);
    lc->nScreenHeight = ioctl(lc->fdDisplay, DISP_GET_SCN_HEIGHT,(void *)args);
    logd("screen:w %d, screen:h %d", lc->nScreenWidth, lc->nScreenHeight);

    CdxIonOpen();

    return &lc->base;
}
