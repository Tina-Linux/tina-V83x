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
    void*        pMetaDataVirAddr;
    void*        pMetaDataPhyAddr;
    int          nMetaDataVirAddrSize;
    int          nMetaDataMapFd;
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

    int                  crop_x,crop_y,crop_w,crop_h;
    //viewer view
    int                  view_x,view_y,view_w,view_h;
    int                  bHdrVideoFlag;
    int                  b10BitVideoFlag;
    int                  bAfbcModeFlag;
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
    int   nBufFd;

    int   nMemSizeY;
    int   nMemSizeC;

    char* pMeteVirBuf;
    char* pMetePhyBuf;
    int   nMeteBufFd;
    int   nTotalPicPhyBufSize = 0;

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
        switch(lc->eDisplayPixelFormat)
        {
            case PIXEL_FORMAT_P010_UV:
            case PIXEL_FORMAT_P010_VU:
            {

                nMemSizeY = nGpuBufWidth*nGpuBufHeight*2;
                nMemSizeC = nMemSizeY>>2;
                nTotalPicPhyBufSize = nMemSizeY + nMemSizeC*2;

                pVirBuf = CdxIonPalloc(nTotalPicPhyBufSize);
                pPhyBuf = CdxIonVir2Phy(pVirBuf);
                nBufFd =  CdxIonVir2Fd(pVirBuf);

                if(pVirBuf == NULL)
                {
                    loge("memory alloc fail, require %d bytes for picture buffer.", \
                        nTotalPicPhyBufSize);
                    return -1;
                }

                lc->mBufferInfo[i].pPicture.nWidth  = nGpuBufWidth;
                lc->mBufferInfo[i].pPicture.nHeight = nGpuBufHeight;
                lc->mBufferInfo[i].pPicture.nLineStride  = nGpuBufWidth;
                lc->mBufferInfo[i].pPicture.nBufFd       = nBufFd;
                lc->mBufferInfo[i].pPicture.pData0       = pVirBuf;
                lc->mBufferInfo[i].pPicture.pData1       = pVirBuf+nMemSizeY;
                lc->mBufferInfo[i].pPicture.pData2       = NULL;
                lc->mBufferInfo[i].pPicture.phyYBufAddr  = (uintptr_t)pPhyBuf;
                lc->mBufferInfo[i].pPicture.phyCBufAddr  =
                        lc->mBufferInfo[i].pPicture.phyYBufAddr + nMemSizeY;
                lc->mBufferInfo[i].pPicture.nBufSize     = nTotalPicPhyBufSize;
                break;
            }
            case PIXEL_FORMAT_YUV_PLANER_420:
            case PIXEL_FORMAT_YUV_PLANER_422:
            case PIXEL_FORMAT_YUV_PLANER_444:
            case PIXEL_FORMAT_YV12:
            case PIXEL_FORMAT_NV21:
            case PIXEL_FORMAT_NV12:
            {
                nMemSizeY = nGpuBufWidth*nGpuBufHeight;

                if(lc->eDisplayPixelFormat == PIXEL_FORMAT_YUV_PLANER_420 ||
                   lc->eDisplayPixelFormat == PIXEL_FORMAT_YV12 ||
                   lc->eDisplayPixelFormat == PIXEL_FORMAT_NV21)
                    nMemSizeC = nMemSizeY>>2;
                else if(lc->eDisplayPixelFormat == PIXEL_FORMAT_YUV_PLANER_422)
                    nMemSizeC = nMemSizeY>>1;
                else
                    nMemSizeC = nMemSizeY;  //* PIXEL_FORMAT_YUV_PLANER_444

                int nLower2BitBufSize = 0;
                int nAfbcBufSize = 0;
                int nNormalYuvBufSize = nMemSizeY + nMemSizeC*2;
                int nTotalPicPhyBufSize = 0;
                int frmbuf_c_size = 0;

                nLower2BitBufSize = ((((nGpuBufWidth+3)>>2) + 31) & 0xffffffe0) * nGpuBufHeight * 3/2;
                //int PriChromaStride = ((nGpuBufWidth/2) + 31)&0xffffffe0;
                //frmbuf_c_size = 2 * (PriChromaStride * (((nGpuBufHeight/2)+15)&0xfffffff0)/4);
                if(lc->b10BitVideoFlag)
                {
                    if(lc->bAfbcModeFlag == 1)
                    {
                        nAfbcBufSize = ((nGpuBufWidth+15)>>4) * ((nGpuBufHeight+4+15)>>4) * (512 + 16) + 32 + 1024;
                        nTotalPicPhyBufSize = nAfbcBufSize + frmbuf_c_size + nLower2BitBufSize;
                        logd("nTotalSize = %d, nAfbcBufSize = %d,\
                              frmbuf_c_size = %d, nLower2BitBufSize = %d",
                               nTotalPicPhyBufSize, nAfbcBufSize,
                               frmbuf_c_size, nLower2BitBufSize);
                    }
                    else
                    {
                        nTotalPicPhyBufSize = nNormalYuvBufSize + nLower2BitBufSize;
                    }
                }
                else
                {
                    if(lc->bAfbcModeFlag == 1)
                    {
                        nAfbcBufSize = ((nGpuBufWidth+15)>>4) * ((nGpuBufHeight+4+15)>>4) * (384 + 16) + 32 + 1024;
                        nTotalPicPhyBufSize = nAfbcBufSize + frmbuf_c_size;
                        logd("the_afbc,nAfbcBufSize=%d",nAfbcBufSize);
                    }
                    else
                    {
                        nTotalPicPhyBufSize = nNormalYuvBufSize;
                    }
                }

                pVirBuf = CdxIonPalloc(nTotalPicPhyBufSize);
                pPhyBuf = CdxIonVir2Phy(pVirBuf);
                nBufFd  = CdxIonVir2Fd(pVirBuf);

                if(pVirBuf == NULL)
                {
                    loge("memory alloc fail, require %d bytes for picture buffer.", \
                        nTotalPicPhyBufSize);
                    return -1;
                }

                logv("lc->bEnableAfbcFlag = %d",lc->bAfbcModeFlag);

            #if 0
                if(lc->bAfbcModeFlag == 1)
                {
                    logd("** set buf to 0, pMem = %p",pVirBuf);
                    memset(pVirBuf, 0, nTotalPicPhyBufSize);
                    CdxIonFlushCache(pVirBuf,nTotalPicPhyBufSize);
                }
            #endif

                if(lc->b10BitVideoFlag)
                {
                    if(lc->bAfbcModeFlag== 1)
                    {
                        lc->mBufferInfo[i].pPicture.pData0 = pVirBuf;
                        lc->mBufferInfo[i].pPicture.nLower2BitBufSize  = nLower2BitBufSize;
                        lc->mBufferInfo[i].pPicture.nLower2BitBufOffset = nAfbcBufSize + frmbuf_c_size;
                        lc->mBufferInfo[i].pPicture.nLower2BitBufStride = ((((nGpuBufWidth+3)>>2) + 31) & 0xffffffe0);
                        lc->mBufferInfo[i].pPicture.nAfbcSize = nAfbcBufSize;
                    }
                    else
                    {
                        lc->mBufferInfo[i].pPicture.pData0 = pVirBuf;
                        lc->mBufferInfo[i].pPicture.pData1 = pVirBuf + nMemSizeY;
                        lc->mBufferInfo[i].pPicture.nLower2BitBufSize  = nLower2BitBufSize;
                        lc->mBufferInfo[i].pPicture.nLower2BitBufOffset = nNormalYuvBufSize;
                        lc->mBufferInfo[i].pPicture.nLower2BitBufStride = ((((nGpuBufWidth+3)>>2) + 31) & 0xffffffe0);
                    }
                }
                else
                {
                    if(lc->bAfbcModeFlag== 1)
                    {
                        lc->mBufferInfo[i].pPicture.pData0 = pVirBuf;
                        lc->mBufferInfo[i].pPicture.nAfbcSize = nAfbcBufSize;
                        logd("the_afbc,pVirBuf=%p",pVirBuf);
                    }
                    else
                    {
                        lc->mBufferInfo[i].pPicture.pData0 = pVirBuf;
                        lc->mBufferInfo[i].pPicture.pData1 = pVirBuf + nMemSizeY;
                        if(lc->eDisplayPixelFormat != PIXEL_FORMAT_NV21)
                            lc->mBufferInfo[i].pPicture.pData2 = pVirBuf + nMemSizeY + nMemSizeC;
                        else
                            lc->mBufferInfo[i].pPicture.pData2 = NULL;
                    }
                }

                lc->mBufferInfo[i].pPicture.pData3 = NULL;
                lc->mBufferInfo[i].pPicture.nBufSize = nTotalPicPhyBufSize;
                lc->mBufferInfo[i].pPicture.phyYBufAddr  = (uintptr_t)pPhyBuf;
                lc->mBufferInfo[i].pPicture.phyCBufAddr  =
                        lc->mBufferInfo[i].pPicture.phyYBufAddr +nMemSizeY;

                break;
            }
            default:
                loge("pixel format not support yet, ePixelFormat=%d", lc->eDisplayPixelFormat);
                return -1;
        }

        pMeteVirBuf = CdxIonPalloc(4096);
        pMetePhyBuf = CdxIonVir2Phy(pMeteVirBuf);
        nMeteBufFd  = CdxIonVir2Fd(pMeteVirBuf);
        logv("pMetePhyBuf %p",pMetePhyBuf);

        if (pMeteVirBuf == NULL)
        {
            loge("error: ion buff allocate fail!");
            return -1;
        }

        lc->mBufferInfo[i].nUsedFlag    = 0;

        lc->mBufferInfo[i].pPicture.nWidth  = lc->nWidth;
        lc->mBufferInfo[i].pPicture.nHeight = lc->nHeight;
        lc->mBufferInfo[i].pPicture.nLineStride  = lc->nWidth;
        lc->mBufferInfo[i].pPicture.nBufFd       = nBufFd;

        lc->mBufferInfo[i].pPicture.nBufId       = i;
        lc->mBufferInfo[i].pPicture.ePixelFormat = lc->eDisplayPixelFormat;
        lc->mBufferInfo[i].pPicture.pMetaData   = pMeteVirBuf;

        lc->mBufferInfo[i].pMetaDataVirAddr = pMeteVirBuf;
        lc->mBufferInfo[i].pMetaDataPhyAddr = pMetePhyBuf;
        lc->mBufferInfo[i].nMetaDataMapFd   = nMeteBufFd;
    }

    return 0;
}

static int SetLayerParam(LayerContext* lc, VideoPicture* pPicture)
{
    disp_layer_config2 config;
    unsigned long     args[4];
    int ret = 0;
    //* close the layer first, otherwise, in case when last frame is kept showing,
    //* the picture showed will not valid because parameters changed.
    memset(&config.info, 0, sizeof(disp_layer_info2));
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
            config.info.fb.size[0].width     = pPicture->nWidth;
            config.info.fb.size[0].height    = pPicture->nHeight;
            config.info.fb.size[1].width     = pPicture->nWidth/2;
            config.info.fb.size[1].height    = pPicture->nHeight/2;
            config.info.fb.size[2].width     = pPicture->nWidth/2;
            config.info.fb.size[2].height    = pPicture->nHeight/2;
        break;
        case PIXEL_FORMAT_YV12:
            config.info.fb.format = DISP_FORMAT_YUV420_P;
            config.info.fb.size[0].width     = pPicture->nWidth;
            config.info.fb.size[0].height    = pPicture->nHeight;
            config.info.fb.size[1].width     = pPicture->nWidth/2;
            config.info.fb.size[1].height    = pPicture->nHeight/2;
            config.info.fb.size[2].width     = pPicture->nWidth/2;
            config.info.fb.size[2].height    = pPicture->nHeight/2;
        break;

        case PIXEL_FORMAT_NV12:
            config.info.fb.format = DISP_FORMAT_YUV420_SP_UVUV;
            config.info.fb.size[0].width     = pPicture->nWidth;
            config.info.fb.size[0].height    = pPicture->nHeight;

            config.info.fb.size[1].width     = pPicture->nWidth/2;
            config.info.fb.size[1].height    = pPicture->nHeight/2;
        break;

        case PIXEL_FORMAT_NV21:
            config.info.fb.format = DISP_FORMAT_YUV420_SP_VUVU;
            config.info.fb.size[0].width     = pPicture->nWidth;
            config.info.fb.size[0].height    = pPicture->nHeight;
            config.info.fb.size[1].width     = pPicture->nWidth/2;
            config.info.fb.size[1].height    = pPicture->nHeight/2;
        break;
#if (defined(CONF_10BIT_ENABLE))
        case PIXEL_FORMAT_P010_UV:
        {
            config.info.fb.format = DISP_FORMAT_YUV420_SP_UVUV_10BIT;
            config.info.fb.align[0] = YV12_ALIGN;//fixme
            config.info.fb.align[1] = YV12_ALIGN;//fixme
            config.info.fb.size[0].width     = pPicture->nWidth;
            config.info.fb.size[0].height    = pPicture->nHeight;
            config.info.fb.size[1].width = pPicture->nWidth / 2;
            config.info.fb.size[1].height = pPicture->nHeight / 2;

            break;
        }
        case PIXEL_FORMAT_P010_VU:
        {
            config.info.fb.format = DISP_FORMAT_YUV420_SP_VUVU_10BIT;
            config.info.fb.align[0] = YV12_ALIGN;//fixme
            config.info.fb.align[1] = YV12_ALIGN;//fixme
            config.info.fb.size[0].width     = pPicture->nWidth;
            config.info.fb.size[0].height    = pPicture->nHeight;
            config.info.fb.size[1].width = pPicture->nWidth / 2;
            config.info.fb.size[1].height = pPicture->nHeight / 2;
            break;
        }
#endif
        default:
        {
            loge("unsupported pixel format(%d).",lc->eDisplayPixelFormat);
            return -1;
        }
    }

    //* initialize the layerInfo.
    config.info.mode            = LAYER_MODE_BUFFER;
    config.info.zorder          = 1;
    config.info.alpha_mode      = 1;
    config.info.alpha_mode      = 0xff;

    //* image size.
    config.info.fb.crop.x = 0;
    config.info.fb.crop.y = 0;
    config.info.fb.crop.width   = (unsigned long long)lc->nDisplayWidth << 32;
    config.info.fb.crop.height  = (unsigned long long)lc->nDisplayHeight << 32;
    config.info.fb.color_space  = (lc->nHeight < 720) ? DISP_BT601 : DISP_BT709;

    //* source window.
    if((lc->view_w == 0)||(lc->view_h == 0))
    {
        config.info.screen_win.x        = lc->nLeftOff;
        config.info.screen_win.y        = lc->nTopOff;
        config.info.screen_win.width    = lc->nScreenWidth;
        config.info.screen_win.height   = lc->nScreenHeight;

    }else{
        config.info.screen_win.x        = lc->view_x;//lc->nLeftOff;
        config.info.screen_win.y        = lc->view_y;//lc->nTopOff;
        config.info.screen_win.width    = lc->view_w;//lc->nScreenWidth;
        config.info.screen_win.height   = lc->view_h;//lc->nScreenHeight;
    }


    int i;
    for(i = 0; i < GPU_BUFFER_NUM; i++)
    {
        if(lc->mBufferInfo[i].pMetaDataVirAddr == pPicture->pMetaData)
        {
            config.info.fb.metadata_fd = lc->mBufferInfo[i].nMetaDataMapFd;
            config.info.fb.metadata_size = 4096;
            if (lc->bAfbcModeFlag)
            {
                config.info.fb.metadata_flag = 1<<4;
                config.info.fb.fbd_en = 1;
            }
            if (lc->bHdrVideoFlag)
            {
                config.info.fb.metadata_flag = config.info.fb.metadata_flag|(1<<1);
            }
            break;
        }
    }

    config.info.fb.fd = pPicture->nBufFd;

    config.info.alpha_mode          = 1;
    config.info.alpha_value         = 0xff;

    config.channel = 0;
    config.enable = 1;
    config.layer_id = 0;
    //* set layerInfo to the driver.
    args[0] = (unsigned long)0;
    args[1] = (unsigned long)(&config);
    args[2] = (unsigned long)1;

    ret = ioctl(lc->fdDisplay, DISP_LAYER_SET_CONFIG2, (void*)args);
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
        CdxIonPfree(lc->mBufferInfo[i].pMetaDataVirAddr);
    }

    return 0;
}

static void __LayerRelease(LayerCtrl* l)
{
    LayerContext* lc;
    int i;

    lc = (LayerContext*)l;

    logd("Layer release");
    //sendBlackFrameToDisplay(lc);
    for(i=0; i<lc->nGpuBufferCount; i++)
    {
        CdxIonPfree(lc->mBufferInfo[i].pPicture.pData0);
        CdxIonPfree(lc->mBufferInfo[i].pMetaDataVirAddr);
    }

    //*to clear unnecessary members of layercontext object.
    for(i = 0; i < 16; i++)
    {
        memset(&lc->picNodes[i],0x00,sizeof(lc->picNodes));
    }
    lc->pPictureListHeader = NULL;
    lc->bLayerInitialized = 0;
    lc->nGpuBufferCount = 0;
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
                pPicture = nodePtr->pPicture;
                nodePtr->bUsed = 0;
                i--;

            }
        }
    }

    logv("** dequeue  pPicture(%p), id(%d)", pPicture, pPicture->nBufId);
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
    logv("** queue , pPicture(%p), id(%d)", pBuf, pBuf->nBufId);

    if(lc->bLayerInitialized == 0)
    {
        if(setLayerBuffer(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }

        lc->bLayerInitialized = 1;
    }

    if(SetLayerParam(lc, pBuf) != 0)
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

static int LayerSetHdrInfo(LayerCtrl *l, const FbmBufInfo *fbmInfo)
{
    logv("LayerSetHdrInfo");
    if (!fbmInfo)
    {
        loge("fbmInfo is null");
        return -1;
    }

    LayerContext *lc = (LayerContext*)l;

    lc->bHdrVideoFlag = fbmInfo->bHdrVideoFlag;
    lc->b10BitVideoFlag = fbmInfo->b10bitVideoFlag;
    lc->bAfbcModeFlag = fbmInfo->bAfbcModeFlag;

    return 0;
}

static int __LayerControl(LayerCtrl* l, int cmd, void *para)
{
    LayerContext *lc = (LayerContext*)l;

    switch(cmd)
    {
        case CDX_LAYER_CMD_SET_HDR_INFO:
        {
            LayerSetHdrInfo(l, (const FbmBufInfo *)para);
            break;
        }
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
