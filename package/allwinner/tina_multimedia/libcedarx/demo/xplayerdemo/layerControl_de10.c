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
#include "memoryAdapter.h"
#include "disp.h"
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

    struct ScMemOpsS*    pMemops;

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
	unsigned int nGpuBufWidthAlign32;
	unsigned int nGpuBufHeightAlign64;
    int i = 0;
    char* pVirBuf;
    char* pPhyBuf;

    nGpuBufWidth  = lc->nWidth;  //* restore nGpuBufWidth to mWidth;
    nGpuBufHeight = lc->nHeight;
	nGpuBufWidthAlign32 = ((nGpuBufWidth + 31) & ~31);
	nGpuBufHeightAlign64 = ((nGpuBufHeight + 63) & ~63);

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
        //pVirBuf = (char*)CdcMemPalloc(lc->pMemops, nGpuBufWidth * nGpuBufHeight * 3/2);
        pVirBuf = (char*)CdcMemPalloc(lc->pMemops, nGpuBufWidthAlign32 * nGpuBufHeight + nGpuBufWidth*nGpuBufHeightAlign64/2);

        pPhyBuf = (char*)CdcMemGetPhysicAddressCpu(lc->pMemops, pVirBuf);
        lc->mBufferInfo[i].nUsedFlag    = 0;

        lc->mBufferInfo[i].pPicture.nWidth  = lc->nWidth;
        lc->mBufferInfo[i].pPicture.nHeight = lc->nHeight;
        lc->mBufferInfo[i].pPicture.nLineStride  = nGpuBufWidthAlign32;
        lc->mBufferInfo[i].pPicture.pData0       = pVirBuf;
        lc->mBufferInfo[i].pPicture.pData1       = pVirBuf + (nGpuBufWidthAlign32 * nGpuBufHeight);
        lc->mBufferInfo[i].pPicture.pData2       = NULL;
                //lc->mBufferInfo[i].pPicture.pData1 + (lc->nHeight * lc->nWidth)/4;
        lc->mBufferInfo[i].pPicture.phyYBufAddr  = (uintptr_t)pPhyBuf;
        lc->mBufferInfo[i].pPicture.phyCBufAddr  =
                lc->mBufferInfo[i].pPicture.phyYBufAddr + (nGpuBufWidthAlign32 * nGpuBufHeight);
        lc->mBufferInfo[i].pPicture.nBufId       = i;
        lc->mBufferInfo[i].pPicture.ePixelFormat = lc->eDisplayPixelFormat;
    }

    return 0;
}

static int SetLayerParam(LayerContext* lc, VideoPicture* pPicture)
{
	unsigned int fbAddr[3];
	struct win_info info;
    if(lc->bLayerShowed == 1)
    {
        lc->bLayerShowed = 0;
        //TO DO.
    }

	switch(lc->eDisplayPixelFormat)
	{
		case PIXEL_FORMAT_YUV_MB32_420:
			fbAddr[0] = (unsigned int )
										CdcMemGetPhysicAddressCpu(lc->pMemops, pPicture->pData0);

			fbAddr[1] = (unsigned int )
										CdcMemGetPhysicAddressCpu(lc->pMemops, pPicture->pData1);

			//fbAddr[2] = (unsigned int )
										//CdcMemGetPhysicAddressCpu(lc->pMemops, pPicture->pData2);
			break;
		default:
			loge("%s not support pixFormat:%d\n",__FUNCTION__,pPicture->ePixelFormat);
			return -1;
	}

	//logd("addr[0]:%x addr[1]:%x addr[2]:%x\n",fbAddr[0],fbAddr[1],fbAddr[2]);
	//logd("1:%d 2:%d 3:%d 4:%d 5:%d 6:%d\n",lc->nDisplayWidth,lc->nDisplayHeight,lc->nLeftOff,lc->nTopOff,
		//lc->nScreenWidth,lc->nScreenHeight);
	//logd("linestride:%d\n",pPicture->nLineStride);
	info.width = lc->nDisplayWidth;
	info.height = lc->nDisplayHeight;
	info.src_win.outLeftOffset = 0;
	info.src_win.outTopOffset = 0;
	info.src_win.outRightOffset = lc->nDisplayWidth;
	info.src_win.outBottomOffset = lc->nDisplayHeight;

	info.scn_win.outLeftOffset = lc->nLeftOff;
	info.scn_win.outTopOffset = lc->nTopOff;
	info.scn_win.outRightOffset = lc->nScreenWidth;
	info.scn_win.outBottomOffset = lc->nScreenHeight;
	//logd("set param pid:%d\n",getpid());
	disp_video_chanel_open();
	if(0 != disp_video_layer_param_set(fbAddr[0], DISP_MB32_PLANNAR, info, DISP_UI_ALPHA))
	{
		logd("set video layer param fail!\n");
	}
	//usleep(50000);
	//usleep(1000000);
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
        CdcMemPfree(lc->pMemops, lc->mBufferInfo[i].pPicture.pData0);
    }

    return 0;
}


static void __LayerRelease(LayerCtrl* l)
{
    LayerContext* lc;
    int i;

    lc = (LayerContext*)l;

    logd("Layer release");
    for(i=0; i<lc->nGpuBufferCount; i++)
    {
        CdcMemPfree(lc->pMemops, lc->mBufferInfo[i].pPicture.pData0);
    }
}

static void __LayerDestroy(LayerCtrl* l)
{
    LayerContext* lc;
    int i;

    lc = (LayerContext*)l;

    //close(lc->fdDisplay);
    //disp_video_chanel_close();
    disp_uninit();
    CdcMemClose(lc->pMemops);

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
    logd("Layer set display region, leftOffset = %d, topOffset = %d, displayWidth = %d, \
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
       ePixelFormat == PIXEL_FORMAT_YV12 ||
       ePixelFormat == PIXEL_FORMAT_YUV_MB32_420)           //* add new pixel formats supported by gpu here.
    {
        lc->eDisplayPixelFormat = ePixelFormat;
    }
    else
    {
        logd("receive pixel format is %d, not match.", lc->eDisplayPixelFormat);
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
#if 0
           unsigned int     args[3];
#endif

                    //* return one picture.
            if(i > GetConfigParamterInt("pic_4list_num", 3))
            {
                nodePtr = lc->pPictureListHeader;
                lc->pPictureListHeader = lc->pPictureListHeader->pNext;
#if 0
                //---------------------------
                {
                    int nCurFrameId;
                    int nWaitTime;
                    int disp0 = 0;
                    int disp1 = 0;

                    nWaitTime = 50000;  //* max frame interval is 1000/24fps = 41.67ms,
                                        //  we wait 50ms at most.
                    args[1] = 0; //chan(0 for video)
                    args[2] = 0; //layer_id
                    do
                    {
                        args[0] = 0; //disp
                        nCurFrameId = ioctl(lc->fdDisplay, DISP_LAYER_GET_FRAME_ID, args);
                        logv("nCurFrameId hdmi = %d, pPicture id = %d",
                             nCurFrameId, nodePtr->pPicture->nID);
                        if(nCurFrameId != nodePtr->pPicture->nID)
                        {
                            break;
                        }
                        else
                        {
                            if(nWaitTime <= 0)
                            {
                                loge("check frame id fail, maybe something error happen.");
                                break;
                            }
                            else
                            {
                                usleep(5000);
                                nWaitTime -= 5000;
                            }
                        }
                    }while(1);
                }
#endif
                pPicture = nodePtr->pPicture;
                nodePtr->bUsed = 0;
                i--;

            }
			else
			{
				logd("return -1 because no queue!\n");
				return -1;
			}
        }
		else
		{
			logd("return -1 because list header null!\n");
			return -1;
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

    if(lc->bLayerInitialized == 0)
    {
        if(setLayerBuffer(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }

        lc->bLayerInitialized = 1;
    }
	if(bValid)
	{
	    if(SetLayerParam(lc, pBuf) != 0)
	    {
	        loge("can not initialize layer.");
	        return -1;
	    }
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

    logd("LayerSetBufferCount: count = %d",nBufferCount);

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
    for(i = 0; i<lc->nGpuBufferCount; i++)
    {
        bufInfo = lc->mBufferInfo[i];
        if(bufInfo.nUsedFlag == 1)
        {
            bufInfo.nUsedFlag = 0;
            pPicture = &bufInfo.pPicture;
            break;
        }
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
    logd("***LayerReleaseBuffer");
    LayerContext* lc;

    lc = (LayerContext*)l;

    CdcMemPfree(lc->pMemops, pPicture->pData0);
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
    destroy:                    __LayerDestroy
};

LayerCtrl* LayerCreate_DE()
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

	disp_init();
	lc->fdDisplay = disp_get_cur_fd();
    if(lc->fdDisplay < 1)
    {
        loge("open disp failed");
        free(lc);
        return NULL;
    }
	disp_lcd_on(lc->fdDisplay);
	disp_open_backlight();

    lc->base.ops = &mLayerControlOps;
    lc->eDisplayPixelFormat = PIXEL_FORMAT_YUV_MB32_420;

	lc->nScreenWidth = disp_screenwidth_get();
	lc->nScreenHeight = disp_screenheight_get();
    logd("screen:w %d, screen:h %d", lc->nScreenWidth, lc->nScreenHeight);

    lc->pMemops = MemAdapterGetOpsS();
    CdcMemOpen(lc->pMemops);

    return &lc->base;
}
