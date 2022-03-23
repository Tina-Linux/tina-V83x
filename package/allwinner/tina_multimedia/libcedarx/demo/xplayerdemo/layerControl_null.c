/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : layerControl_null.c
* Description : default display interface -- for linux
* History :
*/

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "cdx_config.h"
#include <cdx_log.h>
#include "layerControl.h"
#include <CdxIon.h>

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
    }

    return 0;
}

int __LayerReset(LayerCtrl* l)
{
    LayerContext* lc;
    int i;

    logd("LayerReset.");

    lc = (LayerContext*)l;

    for(i=0; i<lc->nGpuBufferCount; i++)
    {
        CdxIonPfree(lc->mBufferInfo[i].pPicture.pData0);
    }

    return 0;
}


void __LayerRelease(LayerCtrl* l)
{
    LayerContext* lc;
    int i;

    lc = (LayerContext*)l;

    logv("Layer release");
    for(i=0; i<lc->nGpuBufferCount; i++)
    {
        CdxIonPfree(lc->mBufferInfo[i].pPicture.pData0);
    }
}

void __LayerDestroy(LayerCtrl* l)
{
    LayerContext* lc;

    lc = (LayerContext*)l;

    CdxIonClose();

    free(lc);
}


int __LayerSetDisplayBufferSize(LayerCtrl* l, int nWidth, int nHeight)
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
int __LayerSetDisplayRegion(LayerCtrl* l, int nLeftOff, int nTopOff,
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
int __LayerSetDisplayPixelFormat(LayerCtrl* l, enum EPIXELFORMAT ePixelFormat)
{
    LayerContext* lc;

    lc = (LayerContext*)l;
    logv("Layer set expected pixel format, format = %d", (int)ePixelFormat);

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
int __LayerSetDeinterlaceFlag(LayerCtrl* l,int bFlag)
{
    LayerContext* lc;

    lc = (LayerContext*)l;

    return 0;
}

//* Description: set buffer timestamp -- set this param every frame
int __LayerSetBufferTimeStamp(LayerCtrl* l, int64_t nPtsAbs)
{
    LayerContext* lc;

    lc = (LayerContext*)l;

    return 0;
}

int __LayerGetRotationAngle(LayerCtrl* l)
{
    LayerContext* lc;
    int nRotationAngle = 0;

    lc = (LayerContext*)l;

    return 0;
}

int __LayerCtrlShowVideo(LayerCtrl* l)
{
    LayerContext* lc;
    int               i;

    lc = (LayerContext*)l;

    lc->bLayerShowed = 1;

    return 0;
}

int __LayerCtrlHideVideo(LayerCtrl* l)
{
    LayerContext* lc;
    int               i;

    lc = (LayerContext*)l;

    lc->bLayerShowed = 0;

    return 0;
}

int __LayerCtrlIsVideoShow(LayerCtrl* l)
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

            if(i > GetConfigParamterInt("pic_4list_num", 3))
            {
                nodePtr = lc->pPictureListHeader;
                lc->pPictureListHeader = lc->pPictureListHeader->pNext;
                pPicture = nodePtr->pPicture;
                nodePtr->bUsed = 0;
            }
        }
    }

    logv("** dequeue  pPicture(%p)",pPicture);
    *ppVideoPicture = pPicture;
    return 0;
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
    logv("** queue , pPicture(%p)", pBuf);

    if(bValid == 0)
    {
        return 0;
    }

    if(lc->bLayerInitialized == 0)
    {
        if(setLayerBuffer(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }

        lc->bLayerInitialized = 1;
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

int __LayerSetDisplayBufferCount(LayerCtrl* l, int nBufferCount)
{
    LayerContext* lc;

    lc = (LayerContext*)l;

    logv("LayerSetBufferCount: count = %d",nBufferCount);

    lc->nGpuBufferCount = nBufferCount;

    if(lc->nGpuBufferCount > GPU_BUFFER_NUM)
        lc->nGpuBufferCount = GPU_BUFFER_NUM;

    return lc->nGpuBufferCount;
}

int __LayerGetBufferNumHoldByGpu(LayerCtrl* l)
{
    return GetConfigParamterInt("pic_4list_num", 3);
}

int __LayerGetDisplayFPS(LayerCtrl* l)
{
    return 60;
}

void __LayerResetNativeWindow(LayerCtrl* l,void* pNativeWindow)
{
    logd("LayerResetNativeWindow : %p ",pNativeWindow);

    LayerContext* lc;
    VideoPicture mPicBufInfo;

    lc = (LayerContext*)l;
    lc->bLayerInitialized = 0;

    return ;
}

VideoPicture* __LayerGetBufferOwnedByGpu(LayerCtrl* l)
{
    LayerContext* lc;
    BufferInfoT bufInfo;

    lc = (LayerContext*)l;
    int i;
    for(i = 0; i<lc->nGpuBufferCount; i++)
    {
        bufInfo = lc->mBufferInfo[i];
        if(bufInfo.nUsedFlag == 1)
        {
            bufInfo.nUsedFlag = 0;
            break;
        }
    }
    if(i >= lc->nGpuBufferCount)
        return NULL;
    else
        return &lc->mBufferInfo[i].pPicture;
}

int __LayerSetVideoWithTwoStreamFlag(LayerCtrl* l, int bVideoWithTwoStreamFlag)
{
    LayerContext* lc;

    lc = (LayerContext*)l;

    logv("LayerSetIsTwoVideoStreamFlag, flag = %d",bVideoWithTwoStreamFlag);
    lc->bVideoWithTwoStreamFlag = bVideoWithTwoStreamFlag;

    return 0;
}

int __LayerSetIsSoftDecoderFlag(LayerCtrl* l, int bIsSoftDecoderFlag)
{
    LayerContext* lc;

    lc = (LayerContext*)l;

    logv("LayerSetIsSoftDecoderFlag, flag = %d",bIsSoftDecoderFlag);
    lc->bIsSoftDecoderFlag = bIsSoftDecoderFlag;

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

static int __LayerSetSecureFlag(LayerCtrl* l, int flag)
{
    logv("***LayerReleaseBuffer");
    LayerContext* lc;

    lc = (LayerContext*)l;

    return 0;
}


static LayerControlOpsT mLayerControlOps =
{
    release:                    __LayerRelease                   ,
    setSecureFlag:              __LayerSetSecureFlag             ,

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
    destroy:                    __LayerDestroy
};

LayerCtrl* LayerCreate()
{
    LayerContext* lc;

    logd("LayerCreate.");

    lc = (LayerContext*)malloc(sizeof(LayerContext));
    if(lc == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(lc, 0, sizeof(LayerContext));

    lc->base.ops = &mLayerControlOps;

    CdxIonOpen();

    return &lc->base;
}
