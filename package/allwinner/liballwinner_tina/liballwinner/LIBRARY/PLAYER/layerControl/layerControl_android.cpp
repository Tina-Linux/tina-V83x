
#include "log.h"
#include "cdx_config.h"

#if CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_2
#include <gui/ISurfaceTexture.h>
#elif (CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_4 || CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0)
#include <gui/Surface.h>
#else
    #error "invalid configuration of os version."
#endif

#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include "layerControl.h"
#include "deinterlace.h"
#include "memoryAdapter.h"
#include <hardware/hwcomposer.h>

#if (CONFIG_OS_VERSION > OPTION_OS_VERSION_ANDROID_4_2)
#include <hardware/hal_public.h>
#endif
#include <linux/ion.h>
#include <ion/ion.h>

#if(CONFIG_PRODUCT==OPTION_PRODUCT_TVBOX)
    #define LAYER_SUPPORT_ZERO_COPY 1
#else
    #define LAYER_SUPPORT_ZERO_COPY 0
#endif

#if(GPU_Y_C_ALIGN == GPU_Y16_C16_ALIGN)  //* on 1680, Y-Align is 16, C-Align is 16,
    #define LAYER_GPU_Y_ALIGN (16)
    #define LAYER_GPU_C_ALIGN (16)
#elif(GPU_Y_C_ALIGN == GPU_Y32_C16_ALIGN)//* on 1673, Y-Align is 32, C-Align is 16,
    #define LAYER_GPU_Y_ALIGN (32)
    #define LAYER_GPU_C_ALIGN (16)
#else                               //* on others, Y-Align is 16, C-Align is 8,
    #define LAYER_GPU_Y_ALIGN (16)
    #define LAYER_GPU_C_ALIGN (8)
#endif

#if ((LINUX_VERSION == LINUX_VERSION_3_10) || (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0))
    typedef ion_user_handle_t ion_handle_abstract_t;
#else
    typedef struct ion_handle* ion_handle_abstract_t;
#endif

#define DEBUG_DUMP_PIC 0

static VideoPicture* gLastPicture = NULL;

typedef struct VPictureNode_t VPictureNode;
struct VPictureNode_t
{
    VideoPicture* pPicture;
    VideoPicture* pSecondPictureOf3D;
    buffer_handle_t handle;
};

typedef struct LayerCtrlContext
{
    ANativeWindow*       pNativeWindow;
    enum EPIXELFORMAT    eDisplayPixelFormat;
    int                  nWidth;
    int                  nHeight;
    int                  nLeftOff;
    int                  nTopOff;
    int                  nDisplayWidth;
    int                  nDisplayHeight;
    PlayerCallback       callback;
    void*                pUserData;
    int                  bRenderToHardwareLayer;
    int                  bLayerInitialized;
    int                  bLayerShowed;
    int                  bDeinterlaceFlag;		// 0: do not de-i, 1: hw de-i, 2: sw de-i
    int                  nPreNativeWindowOutputType;
	int				 bFirstFrameIsShowed; // 0->first not showed, 1->is already showed
	int					 nOutputTypeChanged;	//0:no change  1:gpu->0 copy

    //* use when render to gpu.
    VideoPicture         bufferWrappers[32];
    int                  bBufferWrapperUsed[32];
    ANativeWindowBuffer* pWindowBufs[32];

    //* use when render derect to hardware layer.
    VPictureNode         *pKeepPicNode;

	bool				 keepLastFrameFlag;
    int                  nGpuYAlign;
    int                  nGpuCAlign;
    int picDump;
	struct ScMemOpsS *memops;
    unsigned int         nUsage;
    int                  need_get_phy_addr_flag;
}LayerCtrlContext;

int __LayerSetDisplayPixelFormat(LayerCtrl* l, enum EPIXELFORMAT ePixelFormat);

void savePictureData(LayerCtrlContext* lc, VideoPicture* pPicture)
{
    if (lc->picDump < 2)
    {
        FILE* fp1 = NULL;
        char path[512] = {0};

        logd("layer[%d] dump image hw(%d) size(%d * %d) , addr(%p/%p)",
                    lc->picDump, lc->bRenderToHardwareLayer,  pPicture->nWidth, pPicture->nHeight,
                    pPicture->pData0, pPicture->pData1);

        sprintf(path, "/data/camera/layer.%d.dat", lc->picDump);
        fp1 = fopen(path, "wb");
        //MemAdapterFlushCache(pBuf->pData0, (pBuf->nWidth*pBuf->nHeight*3)/2);
        fwrite(pPicture->pData0, 1, (pPicture->nWidth*pPicture->nHeight*3)/2, fp1);
        fclose(fp1);

        lc->picDump++;
    }
}

uintptr_t getPhyAddrOfGpuBuffer(ANativeWindowBuffer* pWindowBuf)
{
    uintptr_t   nPhyaddress = 0;

    if(pWindowBuf->handle == NULL)
    {
        loge("pWindowBuf->handle is null");
	return 0;
    }

    ion_handle_abstract_t handle_ion;

    int fd = ion_open();
	if(fd == -1)
	{
        loge("ion_open fail");
		return 0;
	}

#if (GPU_TYPE_MALI == 1)
    private_handle_t* hnd = (private_handle_t *)(pWindowBuf->handle);
	ion_import(fd, hnd->share_fd, &handle_ion);
#else
	IMG_native_handle_t* hnd = (IMG_native_handle_t*)(pWindowBuf->handle);
    ion_import(fd, hnd->fd[0], &handle_ion);
#endif

	nPhyaddress = ion_getphyadr(fd, handle_ion);
	logv("++++++++phyaddress: %x\n", (unsigned int)nPhyaddress);
	ion_close(fd);

    return nPhyaddress;
}

//* check the NATIVE_WINDOW_GET_OUTPUT_TYPE
//* we should decide to 0-copy or not-0-copy rely on outputType every frame
void checkGpuOutputType(LayerCtrl* l)
{
    LayerCtrlContext* lc = (LayerCtrlContext*)l;

    //* deinterlace must not-0-copy, so we should not change it
    if(lc->bDeinterlaceFlag != DE_INTERLACE_NONE)
    {
        logv("not check output type when video is deinterlace");
        return ;
    }

    int nCurOutputType = lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_GET_OUTPUT_TYPE);
    logv("nCurOutputType = %d",nCurOutputType);
    if(nCurOutputType != lc->nPreNativeWindowOutputType)
    {
        logd("output type have change : %d, %d",nCurOutputType,lc->nPreNativeWindowOutputType);
        if(nCurOutputType == 0)
        {
            lc->bRenderToHardwareLayer = 0;
        }
        else
        {

            lc->bRenderToHardwareLayer   = 1;
		lc->nOutputTypeChanged = 1;
        }

        //* reset the display pixelformat relay on bRenderToHardwareLayer
        __LayerSetDisplayPixelFormat(l, (enum EPIXELFORMAT) lc->eDisplayPixelFormat);

        lc->bLayerInitialized          = 0; //* reset layer param
        lc->nPreNativeWindowOutputType = nCurOutputType;
    }
}

static void updateKeepListBuffer(LayerCtrlContext* lc, buffer_handle_t handle, VideoPicture* pBuf0, VideoPicture* pBuf1)
{
    int i = 0;

    //
    for(i = 0; i < NUM_OF_PICTURES_KEEP_IN_LIST; i++)
    {
        if ((lc->pKeepPicNode[i].handle == handle)
            && (lc->pKeepPicNode[i].pPicture != NULL))
        {
            //* reture the old pic to ve
            lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)lc->pKeepPicNode[i].pPicture);
            if (lc->pKeepPicNode[i].pSecondPictureOf3D != NULL)
            {
                lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)lc->pKeepPicNode[i].pSecondPictureOf3D);
            }

            //* add the ve pic to keep-list
            lc->pKeepPicNode[i].pPicture = pBuf0;
            lc->pKeepPicNode[i].pSecondPictureOf3D = pBuf1;
            break;
        }
    }

    //* pBuf not in the list, add it
    if (i == NUM_OF_PICTURES_KEEP_IN_LIST)
    {
        for(i = 0; i < NUM_OF_PICTURES_KEEP_IN_LIST; i++)
        {
            if ((lc->pKeepPicNode[i].handle == NULL)
                && (lc->pKeepPicNode[i].pPicture == NULL))
            {
                lc->pKeepPicNode[i].handle = handle;
                lc->pKeepPicNode[i].pPicture = pBuf0;
                lc->pKeepPicNode[i].pSecondPictureOf3D = pBuf1;
                break;
            }
        }
        //* add failed
        if (i == NUM_OF_PICTURES_KEEP_IN_LIST)
        {
            logw("add buffer to list failed, continue to add");
            //* return all buffer to ve
            for(i = 0; i < NUM_OF_PICTURES_KEEP_IN_LIST; i++)
            {
                if (lc->pKeepPicNode[i].pPicture != NULL)
                {
                    lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)lc->pKeepPicNode[i].pPicture);
                    if (lc->pKeepPicNode[i].pSecondPictureOf3D != NULL)
                    {
                        lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)lc->pKeepPicNode[i].pSecondPictureOf3D);
                    }
                    lc->pKeepPicNode[i].handle = NULL;
                    lc->pKeepPicNode[i].pPicture = NULL;
                    lc->pKeepPicNode[i].pSecondPictureOf3D = NULL;
                }
            }
            //* add in the head
            lc->pKeepPicNode[0].handle = handle;
            lc->pKeepPicNode[0].pPicture = pBuf0;
            lc->pKeepPicNode[0].pSecondPictureOf3D = pBuf1;
        }
    }
}

static int initLayerParamNormal(LayerCtrlContext* lc)
{
    int          pixelFormat;
    unsigned int nGpuBufWidth;
    unsigned int nGpuBufHeight;
    Rect         crop;

    switch(lc->eDisplayPixelFormat)
    {
        case PIXEL_FORMAT_YV12:             //* why YV12 use this pixel format.
            pixelFormat = HAL_PIXEL_FORMAT_YV12;
            break;
        case PIXEL_FORMAT_NV21:
            pixelFormat = HAL_PIXEL_FORMAT_YCrCb_420_SP;
            break;
        case PIXEL_FORMAT_NV12: //* display system do not support NV12.
        default:
        {
            loge("unsupported pixel format.");
            return -1;
            break;
        }
    }

    lc->nUsage  = GRALLOC_USAGE_SW_READ_NEVER;
    lc->nUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN;
    lc->nUsage |= GRALLOC_USAGE_HW_TEXTURE;
    lc->nUsage |= GRALLOC_USAGE_EXTERNAL_DISP;

    nGpuBufWidth  = (lc->nWidth + 15) & ~15;
    nGpuBufHeight = (lc->nHeight + 15) & ~15;

    //A10's GPU has a bug, we can avoid it
    if(nGpuBufHeight%8 != 0)
    {
        logw("original picture align_width[%d], height[%d] mod8 = %d", nGpuBufWidth, nGpuBufHeight, nGpuBufHeight%8);
        if((nGpuBufWidth*nGpuBufHeight)%256 != 0)
        {
            logw("original picture align_width[%d]*height[%d] mod 1024 = %d",
                   nGpuBufWidth, nGpuBufHeight, (nGpuBufWidth*nGpuBufHeight)%1024);
            nGpuBufHeight = (nGpuBufHeight+7)&~7;
            logw("change picture height to [%d] when render to gpu", nGpuBufHeight);
        }
    }

    nGpuBufWidth = lc->nWidth;  //* restore nGpuBufWidth to mWidth;

    crop.left   = lc->nLeftOff;
    crop.top    = lc->nTopOff;
    crop.right  = lc->nLeftOff + lc->nDisplayWidth;
    crop.bottom = lc->nTopOff + lc->nDisplayHeight;

    native_window_set_usage(lc->pNativeWindow,lc->nUsage);
    native_window_set_scaling_mode(lc->pNativeWindow, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    native_window_set_buffers_geometry(lc->pNativeWindow, nGpuBufWidth, nGpuBufHeight, pixelFormat);
    lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_CROP, &crop);
    /* native_window_set_buffer_count() will free buffers insides, so do not do it again when seek */
    if (lc->bFirstFrameIsShowed == 0)
    {
        /*
         * set 5 frames when Interlaced, for smooth play in high fps,
         * restore to 3 frames when Progressive, because decoder(VE) assumed it.
         * if used 5 frames in Progressive, DE buffer will changed by VE.
         */
        //*if we set buffer count as 5, will cause some problem,
        // we can ask hankewei what is the problem. so we set it to 3.
        if (lc->bDeinterlaceFlag == DE_INTERLACE_HW)
            native_window_set_buffer_count(lc->pNativeWindow, 3);
        else
            native_window_set_buffer_count(lc->pNativeWindow, 3);
    }
    return 0;
}

static int initLayerParamZeroCpy(LayerCtrlContext* lc)
{
    logv("initLayerParamZeroCpy");
    int                   pixelFormat;
    android_native_rect_t crop;

    //* close the layer first, otherwise, in case when last frame is kept showing,
    //* the picture showed will not valid because parameters changed.
    logv("temporally close the HWC layer when parameter changed.");
	lc->bLayerShowed = 0;
    lc->pNativeWindow->perform(lc->pNativeWindow,
                                   NATIVE_WINDOW_SETPARAMETER,
                                   HWC_LAYER_SHOW,
                                   0);

    pixelFormat = HAL_PIXEL_FORMAT_YCrCb_420_SP;

#if(VIDEO_DIRECT_ACCESS_DE == 1)      //* a20 display system do not support MB32.
    switch(lc->eDisplayPixelFormat)
    {
        case PIXEL_FORMAT_YUV_PLANER_420:
        case PIXEL_FORMAT_YV12:             //* why YV12 use this pixel format.
            pixelFormat = HWC_FORMAT_YUV420PLANAR;
            break;
        case PIXEL_FORMAT_YUV_MB32_420:
            pixelFormat = HWC_FORMAT_MBYUV420;
            break;
        case PIXEL_FORMAT_YUV_MB32_422:
            pixelFormat = HWC_FORMAT_MBYUV422;
            break;
        case PIXEL_FORMAT_YUV_PLANER_422:
            pixelFormat = HWC_FORMAT_MBYUV422;
            break;
        default:
        {
            loge("unsupported pixel format.");
            return -1;
            break;
        }
    }
#elif(VIDEO_DIRECT_ACCESS_DE == 2)
    switch(lc->eDisplayPixelFormat)
    {
        case PIXEL_FORMAT_YV12:             //* why YV12 use this pixel format.
            pixelFormat = HAL_PIXEL_FORMAT_YV12;
            break;
        case PIXEL_FORMAT_NV21:
            pixelFormat = HAL_PIXEL_FORMAT_YCrCb_420_SP;
            break;
        case PIXEL_FORMAT_NV12: //* display system do not support NV12.
        default:
        {
            loge("unsupported pixel format.");
            return -1;
            break;
        }
    }
#endif

    lc->nUsage  = GRALLOC_USAGE_SW_READ_NEVER;
    lc->nUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN;
    lc->nUsage |= GRALLOC_USAGE_HW_TEXTURE;
    lc->nUsage |= GRALLOC_USAGE_EXTERNAL_DISP;

    crop.left   = lc->nLeftOff;
    crop.right  = lc->nLeftOff + lc->nDisplayWidth;
    crop.top    = lc->nTopOff;
    crop.bottom = lc->nTopOff + lc->nDisplayHeight;

    if(pixelFormat != HWC_FORMAT_YUV420PLANAR)
    {
        native_window_set_scaling_mode(lc->pNativeWindow,
                                       NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    }
    native_window_set_usage(lc->pNativeWindow,lc->nUsage);
    native_window_set_buffers_geometry(lc->pNativeWindow,lc->nWidth,lc->nHeight,pixelFormat);
    native_window_set_buffer_count(lc->pNativeWindow, 3);
    lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_CROP, (uintptr_t)&crop);

    lc->bLayerShowed = 0;
    return 0;
}

static int setLayerParam(LayerCtrlContext* lc)
{
    if(lc->bRenderToHardwareLayer)
    {
        return initLayerParamZeroCpy(lc);
    }
    else
    {
        return initLayerParamNormal(lc);
    }
}

int queueBufferNormal(LayerCtrlContext* lc, VideoPicture* pBuf, int bValid)
{
    /*
     * queue to GPU here
     */
    int                  i   = 0;
    int                  err = -1;
    ANativeWindowBuffer* pWindowBuf = NULL;

    pWindowBuf = NULL;
    for(i=0; i<32; i++)
    {
        if(pBuf == &lc->bufferWrappers[i])
        {
            pWindowBuf = lc->pWindowBufs[i];
            lc->bBufferWrapperUsed[i] = 0;
            lc->pWindowBufs[i]        = NULL;
            break;
        }
    }
    if(i == 32 || pWindowBuf == NULL)
    {
        loge("enqueue an invalid buffer.");
        abort();
    }
    //* unlock the buffer.
    {
        GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
        graphicMapper.unlock(pWindowBuf->handle);
    }

     //* On A80-box, we must set comman NATIVE_WINDOW_SET_VIDEO_BUFFERS_INFO
     //* to notify GPU that this is 'cpy to gpu', not 'o-cpy'
#if(LAYER_SUPPORT_ZERO_COPY && VIDEO_DIRECT_ACCESS_DE != 1)
     int nBufParam[7] = {0};
     lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_VIDEO_BUFFERS_INFO,nBufParam[0], nBufParam[1],
     nBufParam[2], nBufParam[3], nBufParam[4], nBufParam[5], nBufParam[6]);
#endif

    if(bValid)
        lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
    else
        lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

    return 0;
}

#if(SEND_3_BLACK_FRAME_TO_GPU == 1)
static int sendThreeBlackFrameToGpu(LayerCtrlContext* lc)
{
    logd("sendThreeBlackFrameToGpu()");

    ANativeWindowBuffer* pWindowBuf;
    void*                pDataBuf;
    int                  i;
    int                  err;

    for(i = 0;i < NUM_OF_PICTURES_KEEP_IN_LIST;i++)
    {
        err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
        if(err != 0)
        {
            logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.", err);
            return -1;
        }
        lc->pNativeWindow->lockBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

        //* lock the data buffer.
        {
            GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
            Rect bounds(lc->nWidth, lc->nHeight);
            graphicMapper.lock(pWindowBuf->handle, lc->nUsage, bounds, &pDataBuf);
        }

        memset((char*)pDataBuf,0x10,(pWindowBuf->height * pWindowBuf->stride));
        memset((char*)pDataBuf + pWindowBuf->height * pWindowBuf->stride,0x80,(pWindowBuf->height * pWindowBuf->stride)/2);

        int nBufAddr[7] = {0};
        //nBufAddr[6] = HAL_PIXEL_FORMAT_AW_NV12;
        nBufAddr[6] = HAL_PIXEL_FORMAT_AW_FORCE_GPU;
        lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_VIDEO_BUFFERS_INFO,	nBufAddr[0], nBufAddr[1],
        nBufAddr[2], nBufAddr[3], nBufAddr[4], nBufAddr[5], nBufAddr[6]);

        //* unlock the buffer.
        {
            GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
            graphicMapper.unlock(pWindowBuf->handle);
        }

        lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
    }

    return 0;
}
#endif

#if(LAYER_SUPPORT_ZERO_COPY && VIDEO_DIRECT_ACCESS_DE == 1)

static void setHwcLayerPictureInfo(LayerCtrlContext* lc,
                                   libhwclayerpara_t* pHwcLayerPictureInfo,
                                   VideoPicture*      pPicture,
                                   VideoPicture*      pSecondPictureOf3D)
{
    memset(pHwcLayerPictureInfo, 0, sizeof(libhwclayerpara_t));

    if(pSecondPictureOf3D == NULL)
    {
        pHwcLayerPictureInfo->number          = pPicture->nID;
        pHwcLayerPictureInfo->bProgressiveSrc = pPicture->bIsProgressive;
        pHwcLayerPictureInfo->bTopFieldFirst  = pPicture->bTopFieldFirst;
        pHwcLayerPictureInfo->flag_addr       = (unsigned long)CdcMemGetPhysicAddressCpu(lc->memops, pPicture->pMafData);
        pHwcLayerPictureInfo->flag_stride     = pPicture->nMafFlagStride;
        pHwcLayerPictureInfo->maf_valid       = pPicture->bMafValid;
        pHwcLayerPictureInfo->pre_frame_valid = pPicture->bPreFrmValid;
        pHwcLayerPictureInfo->top_y           = (unsigned long)CdcMemGetPhysicAddressCpu(lc->memops, pPicture->pData0);
        pHwcLayerPictureInfo->top_c           = (unsigned long)CdcMemGetPhysicAddressCpu(lc->memops, pPicture->pData1);
        pHwcLayerPictureInfo->bottom_y        = (unsigned long)CdcMemGetPhysicAddressCpu(lc->memops, pPicture->pData2);
        pHwcLayerPictureInfo->bottom_c        = (unsigned long)CdcMemGetPhysicAddressCpu(lc->memops, pPicture->pData3);
    }
    else
    {
        pHwcLayerPictureInfo->number          = pPicture->nID;
        pHwcLayerPictureInfo->bProgressiveSrc = pPicture->bIsProgressive;
        pHwcLayerPictureInfo->bTopFieldFirst  = pPicture->bTopFieldFirst;
        pHwcLayerPictureInfo->flag_addr       = (unsigned long)CdcMemGetPhysicAddressCpu(lc->memops, pPicture->pMafData);
        pHwcLayerPictureInfo->flag_stride     = pPicture->nMafFlagStride;
        pHwcLayerPictureInfo->maf_valid       = pPicture->bMafValid;
        pHwcLayerPictureInfo->pre_frame_valid = pPicture->bPreFrmValid;
        pHwcLayerPictureInfo->top_y           = (unsigned long)CdcMemGetPhysicAddressCpu(lc->memops, pPicture->pData0);
        pHwcLayerPictureInfo->top_c           = (unsigned long)CdcMemGetPhysicAddressCpu(lc->memops, pPicture->pData1);
        pHwcLayerPictureInfo->bottom_y        = (unsigned long)CdcMemGetPhysicAddressCpu(lc->memops, pSecondPictureOf3D->pData0);
        pHwcLayerPictureInfo->bottom_c        = (unsigned long)CdcMemGetPhysicAddressCpu(lc->memops, pSecondPictureOf3D->pData1);
    }

    return;
}

static int queueBufferZeroCpyAccessDe(LayerCtrlContext* lc,
                                      VideoPicture* pBuf0,
                                      VideoPicture* pBuf1,
                                      int bValid)
{
    CDX_PLAYER_UNUSE(bValid);
    libhwclayerpara_t hwcLayerPictureInfo;
    setHwcLayerPictureInfo(lc, &hwcLayerPictureInfo, pBuf0, pBuf1);
    lc->pNativeWindow->perform(lc->pNativeWindow,
                               NATIVE_WINDOW_SETPARAMETER,
                               HWC_LAYER_SETFRAMEPARA,
                               (uintptr_t)(&hwcLayerPictureInfo));
    //* wait for new frame showed.
    if(lc->bLayerShowed == 1)
    {
	int nCurFrameId;
        int nWaitTime;

        nWaitTime = 50000;  //* max frame interval is 1000/24fps = 41.67ms, here we wait 50ms for max.
        do
        {
		nCurFrameId = lc->pNativeWindow->perform(lc->pNativeWindow,
                                                          NATIVE_WINDOW_SETPARAMETER,
                                                          HWC_LAYER_GETCURFRAMEPARA,
                                                          0);
            if(nCurFrameId == pBuf0->nID)
		break;
            else
            {
		if(nWaitTime <= 0)
		{
			logv("check frame id fail, maybe something error with the HWC layer.");
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

    //* free last picture of last video stream in case gConfigHoldLastPicture is set.
    if(gLastPicture != NULL)
    {
        logv("xxxx free gLastPicture, pData0 = %p", gLastPicture->pData0);
        FreePictureBuffer(lc->memops, gLastPicture);
        gLastPicture = NULL;
    }
    return 0;
}

static int queueLastBufferZeroCpyAccessDe(LayerCtrlContext* lc, VideoPicture* pPicture)
{
    if(gLastPicture != NULL)
    {
        FreePictureBuffer(lc->memops, gLastPicture);
    }
    gLastPicture = AllocatePictureBuffer(pPicture->nWidth,pPicture->nHeight,
                                         pPicture->nLineStride,pPicture->ePixelFormat);
    if(gLastPicture == NULL)
    {
        loge("pmalloc for lastPicture failed");
        return -1;
    }

	//* the 0 copy on A20 is different
    libhwclayerpara_t hwcLayerPictureInfo;
    gLastPicture->nID = 0xa5a5a5a5;
    RotatePicture(pPicture, gLastPicture, 0, lc->nGpuYAlign, lc->nGpuCAlign);
    //* set picture to display hardware.
    setHwcLayerPictureInfo(&hwcLayerPictureInfo, gLastPicture, NULL);
    lc->pNativeWindow->perform(lc->pNativeWindow,
                               NATIVE_WINDOW_SETPARAMETER,
                               HWC_LAYER_SETFRAMEPARA,
                               (unsigned int)(&hwcLayerPictureInfo));
    //* wait for new frame showed.
    if(lc->bLayerShowed == 1)
    {
        int nCurFrameId;
        int nWaitTime;
        nWaitTime = 50000;  //* max frame interval is 1000/24fps = 41.67ms, here we wait 50ms for max.
        do
        {
            nCurFrameId = lc->pNativeWindow->perform(lc->pNativeWindow,
                                                     NATIVE_WINDOW_SETPARAMETER,
                                                     HWC_LAYER_GETCURFRAMEPARA,
                                                     0);
            if(nCurFrameId == gLastPicture->nID)
                break;
            else
            {
                if(nWaitTime <= 0)
                {
                    loge("check frame id fail, maybe something error with the HWC layer.");
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
    return 0;
}

#elif(LAYER_SUPPORT_ZERO_COPY)

static void getVideoBufferInfo(int *nBufAddr, VideoPicture* pBuf0, VideoPicture* pBuf1, struct ScMemOpsS *memops)
{
    //HAL_PIXEL_FORMAT_AW_NV12		     = 0x101,
    //HAL_PIXEL_FORMAT_AW_MB420			 = 0x102,
    //HAL_PIXEL_FORMAT_AW_MB411			 = 0x103,
    //HAL_PIXEL_FORMAT_AW_MB422			 = 0x104,
    //HAL_PIXEL_FORMAT_AW_YUV_PLANNER420 = 0x105,
	//HAL_PIXEL_FORMAT_AW_YVU_PLANNER420 = 0x106,
    int temp;

    nBufAddr[0] = (int)CdcMemGetPhysicAddress(memops, pBuf0->pData0);
    nBufAddr[1] = (int)CdcMemGetPhysicAddress(memops, pBuf0->pData1);
    nBufAddr[2] = (int)CdcMemGetPhysicAddress(memops, pBuf0->pData2);

    if(pBuf1 != NULL)
    {
	 nBufAddr[3] = (int)CdcMemGetPhysicAddress(memops, pBuf1->pData0);
	 nBufAddr[4] = (int)CdcMemGetPhysicAddress(memops, pBuf1->pData1);
	 nBufAddr[5] = (int)CdcMemGetPhysicAddress(memops, pBuf1->pData2);
    }

    switch(pBuf0->ePixelFormat)
    {
	case PIXEL_FORMAT_YUV_MB32_420:
	{
	    nBufAddr[2] = 0;
	    nBufAddr[5] = 0;
		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_MB420;
		break;
	}
	case PIXEL_FORMAT_YUV_MB32_422:
	{
		nBufAddr[2] = 0;
		nBufAddr[5] = 0;
		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_MB422;
		break;
	}
	case PIXEL_FORMAT_NV12:
	{
		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_NV12;
		break;
	}
	case PIXEL_FORMAT_NV21:
	{
		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_NV21;
		break;
	}
	case PIXEL_FORMAT_YV12:
	{
#if(ZEROCOPY_HAL_PIXEL_FORMAT_AW == ZEROCOPY_HAL_PIXEL_FORMAT_AW_YUV_PLANNER420)
            nBufAddr[6] = HAL_PIXEL_FORMAT_AW_YUV_PLANNER420;
		temp = nBufAddr[1];
		nBufAddr[1] = nBufAddr[2];
		nBufAddr[2] = temp;

		temp = nBufAddr[4];
	    nBufAddr[4] = nBufAddr[5];
	    nBufAddr[5] = temp;
#elif(ZEROCOPY_HAL_PIXEL_FORMAT_AW == ZEROCOPY_HAL_PIXEL_FORMAT_AW_YVU_PLANNER420)
		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_YVU_PLANNER420;
#endif
		break;
	}
	case PIXEL_FORMAT_YUV_PLANER_420:
	{
		nBufAddr[6] = HAL_PIXEL_FORMAT_AW_YUV_PLANNER420;
		break;
	}
	default:
	{
		logd("error pixel format\n");
	}
    }
    return;
}

static int queueBufferZeroCpyNormal(LayerCtrlContext* lc,
                                    VideoPicture* pBuf0,
                                    VideoPicture* pBuf1,
                                    int bValid)
{
    logv("queueBufferZeroCpyNormal");
    int                  err;
    ANativeWindowBuffer* pWindowBuf;
    int                 nBufAddr[7]={0};
    VideoPicture*       pPicture;
    void*               pDataBuf;
    buffer_handle_t     handle = NULL;

    //* dequeue a buffer from the nativeWindow object.
    err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
    if(err != 0)
    {
	logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.", err);
        return -1;
    }
    lc->pNativeWindow->lockBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

    //* lock the data buffer.
    {
	GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
        Rect bounds(lc->nWidth, lc->nHeight);
        graphicMapper.lock(pWindowBuf->handle, lc->nUsage, bounds, &pDataBuf);
	handle = pWindowBuf->handle;
    }

    if(lc->nOutputTypeChanged > 0 && lc->nOutputTypeChanged <= NUM_OF_PICTURES_KEEP_IN_LIST)
    {
	//logd("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx GPU->0 COPY, send black frame to GPU!!! %d", lc->nOutputTypeChanged);
	memset((char*)pDataBuf,0x10,(pWindowBuf->height * pWindowBuf->stride));
	memset((char*)pDataBuf + pWindowBuf->height * pWindowBuf->stride,0x80,(pWindowBuf->height * pWindowBuf->stride)/2);
	lc->nOutputTypeChanged ++;
    }
    else if(lc->nOutputTypeChanged > NUM_OF_PICTURES_KEEP_IN_LIST)
    {
	//logd("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx send over!!");
	lc->nOutputTypeChanged = 0;
    }

    getVideoBufferInfo(nBufAddr, pBuf0, pBuf1, lc->memops);
    lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_VIDEO_BUFFERS_INFO,	nBufAddr[0], nBufAddr[1],
    nBufAddr[2], nBufAddr[3], nBufAddr[4], nBufAddr[5], nBufAddr[6]);
    lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_DIT_INFO, !pBuf0->bIsProgressive, pBuf0->bTopFieldFirst);

    //* unlock the buffer.
    {
	GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
        graphicMapper.unlock(pWindowBuf->handle);
    }

    if(bValid)
	lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
    else
        lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

    updateKeepListBuffer(lc, handle, pBuf0, pBuf1);

    return 0;
}

static int queueLastBufferZeroCpyNormal(LayerCtrlContext* lc, VideoPicture* pPicture)
{
    logd("*******************queueLastBufferZeroCpyNormal");
    int                  err;
    ANativeWindowBuffer* pWindowBuf = NULL;
    int                  nBufAddr[7]={0};
    void*                pDataBuf   = NULL;

	for(int k = 0;k < NUM_OF_PICTURES_KEEP_IN_LIST;k++)
	{
        //* dequeue a buffer from the nativeWindow object.
        err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
        if(err != 0)
        {
		logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.", err);
            return -1;
        }
        lc->pNativeWindow->lockBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
        //* lock the data buffer.
        {
		GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
            Rect bounds(lc->nWidth, lc->nHeight);
            graphicMapper.lock(pWindowBuf->handle, lc->nUsage, bounds, &pDataBuf);
        }

        //* copy data to gpu buffer
        VideoPicture mQueuePicture;
        memset(&mQueuePicture, 0 , sizeof(VideoPicture));
        mQueuePicture.ePixelFormat = lc->eDisplayPixelFormat;
        mQueuePicture.nWidth	   = pWindowBuf->width;
        mQueuePicture.nLineStride  = pWindowBuf->stride;
        mQueuePicture.nHeight	   = pWindowBuf->height;
        mQueuePicture.pData0	   = (char*)pDataBuf;
        mQueuePicture.pData1	   = mQueuePicture.pData0 + (pWindowBuf->height * pWindowBuf->stride);
        mQueuePicture.pData2	   = mQueuePicture.pData1 + (pWindowBuf->height * pWindowBuf->stride)/4;
        RotatePicture(lc->memops, pPicture, &mQueuePicture, 0, lc->nGpuYAlign, lc->nGpuCAlign);

        //* notify hwc use gpu to compose surface
        int nBufAddr[7] = {0};
        nBufAddr[6] = HAL_PIXEL_FORMAT_AW_FORCE_GPU;
        lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_VIDEO_BUFFERS_INFO,	nBufAddr[0], nBufAddr[1],
        nBufAddr[2], nBufAddr[3], nBufAddr[4], nBufAddr[5], nBufAddr[6]);
        //* unlock the buffer.
        {
		GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
            graphicMapper.unlock(pWindowBuf->handle);
        }

	lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
	}

    return 0;
}

#endif

LayerCtrl* __LayerInit(void* pNativeWindow)
{
    LayerCtrlContext* lc;

    logv("LayerInit.");

    lc = (LayerCtrlContext*)malloc(sizeof(LayerCtrlContext));
    if(lc == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(lc, 0, sizeof(LayerCtrlContext));
    lc->picDump = 0;

    lc->pNativeWindow = (ANativeWindow*)pNativeWindow;

#if(ROTATE_PIC_HW == 1)
    lc->need_get_phy_addr_flag = 1;
#endif

#if(LAYER_SUPPORT_ZERO_COPY && ZEROCOPY_DYNAMIC_CHECK == 1)
    int nOutputType = lc->pNativeWindow->perform(lc->pNativeWindow,
                                       NATIVE_WINDOW_GETPARAMETER,
                                       NATIVE_WINDOW_CMD_GET_SURFACE_TEXTURE_TYPE,
                                       0);
    if(nOutputType != 0)
        lc->bRenderToHardwareLayer = 1;
    else
        lc->bRenderToHardwareLayer = 0;
    lc->nPreNativeWindowOutputType = nOutputType;
#elif(LAYER_SUPPORT_ZERO_COPY)
    lc->bRenderToHardwareLayer = 1;
#endif

    lc->nGpuYAlign = LAYER_GPU_Y_ALIGN;
    lc->nGpuCAlign = LAYER_GPU_C_ALIGN;

    //* open the memory module, we need get physical address of a picture buffer
    //* by MemAdapterGetPhysicAddress().
    lc->memops = MemAdapterGetOpsS();
    CdcMemOpen(lc->memops);

	lc->keepLastFrameFlag = false;

	lc->pKeepPicNode = (VPictureNode*)malloc(NUM_OF_PICTURES_KEEP_IN_LIST*sizeof(VPictureNode));
	if(lc->pKeepPicNode == NULL)
	{
		loge("malloc failed!");
		free(lc);
		return NULL;
	}
	memset(lc->pKeepPicNode,0,NUM_OF_PICTURES_KEEP_IN_LIST*sizeof(VPictureNode));

    return (LayerCtrl*)lc;
}


void __LayerRelease(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer release");

#if(SEND_3_BLACK_FRAME_TO_GPU == 1)
    //* if it is 0-copy, we should send 3 black frame to gpu when exit for
    //* avoiding a bug of GPU(Green screen when switch video).
    //* When gpu fix this bug, we should remove the codec

    //* We should ensure that gpu had displayed at least one picture when
    //* send-3-black-frame, or some param in gpu will not right,such as w, h
    if(lc->keepLastFrameFlag == false && lc->bFirstFrameIsShowed == 1)
    {
		sendThreeBlackFrameToGpu(lc);
    }
#endif

    //* return pictures.
	for(int i = 0; i < NUM_OF_PICTURES_KEEP_IN_LIST; i++)
    {
	if (lc->pKeepPicNode[i].pPicture != NULL)
	{
		lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)lc->pKeepPicNode[i].pPicture);
	}
    }

	if(lc->pKeepPicNode)
	{
		free(lc->pKeepPicNode);
		lc->pKeepPicNode = NULL;
	}

    //* free the memory module.
    CdcMemClose(lc->memops);

    free(lc);
}

int __LayerSetCallback(LayerCtrl* l, PlayerCallback callback, void* pUserData)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    lc->callback  = callback;
    lc->pUserData = pUserData;
    return 0;
}

//* Description: set initial param -- display buffer size
int __LayerSetDisplayBufferSize(LayerCtrl* l, int nWidth, int nHeight)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer set picture size, width = %d, height = %d", nWidth, nHeight);

    lc->nWidth         = nWidth;
    lc->nHeight        = nHeight;
    lc->nDisplayWidth  = nWidth;
    lc->nDisplayHeight = nHeight;
    lc->nLeftOff       = 0;
    lc->nTopOff        = 0;
    lc->bLayerInitialized = 0;

    return 0;
}

//* Description: set initial param -- display region
int __LayerSetDisplayRegion(LayerCtrl* l, int nLeftOff, int nTopOff, int nDisplayWidth, int nDisplayHeight)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer set display region, leftOffset = %d, topOffset = %d, displayWidth = %d, displayHeight = %d",
        nLeftOff, nTopOff, nDisplayWidth, nDisplayHeight);

    if(nDisplayWidth != 0 && nDisplayHeight != 0)
    {
        lc->nDisplayWidth  = nDisplayWidth;
        lc->nDisplayHeight = nDisplayHeight;
        lc->nLeftOff       = nLeftOff;
        lc->nTopOff        = nTopOff;
        return 0;
    }
    else
        return -1;
}

//* Description: set initial param -- display pixelFormat
int __LayerSetDisplayPixelFormat(LayerCtrl* l, enum EPIXELFORMAT ePixelFormat)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer set expected pixel format, format = %d", (int)ePixelFormat);

    if(0 == lc->bRenderToHardwareLayer)
    {
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
    }
    else
    {
        //* render directly to hardware layer.
        if(ePixelFormat == PIXEL_FORMAT_YUV_MB32_420   ||
           ePixelFormat == PIXEL_FORMAT_YUV_MB32_422   ||
           ePixelFormat == PIXEL_FORMAT_YUV_MB32_444   ||
           ePixelFormat == PIXEL_FORMAT_YUV_PLANER_420 ||
           ePixelFormat == PIXEL_FORMAT_YV12           ||
           ePixelFormat == PIXEL_FORMAT_NV12           ||
           ePixelFormat == PIXEL_FORMAT_NV21)   //* add new pixel formats supported by hardware layer here.
        {
            lc->eDisplayPixelFormat = ePixelFormat;
        }
        else
        {
            logv("receive pixel format is %d, not match.", lc->eDisplayPixelFormat);
            return -1;
        }
    }

    return 0;
}

//* Description: set initial param -- deinterlace flag
int __LayerSetDeinterlaceFlag(LayerCtrl* l,int bFlag)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerSetDeinterlaceFlag : %d",bFlag);

    if(lc == NULL)
    {
        loge("error: the lc is null!");
        return -1;
    }
    lc->bDeinterlaceFlag = bFlag;

    if(lc->bDeinterlaceFlag == DE_INTERLACE_HW)
    {
        lc->need_get_phy_addr_flag = 1;
    }

    if(lc->bDeinterlaceFlag != DE_INTERLACE_NONE)
    {
        lc->bRenderToHardwareLayer = 0;
    }

    return 0;
}

//* Description: set buffer timestamp -- set this param every frame
int __LayerSetBufferTimeStamp(LayerCtrl* l, int64_t nPtsAbs)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

	native_window_set_buffers_timestamp(lc->pNativeWindow, nPtsAbs);

	return 0;
}

int __LayerGetRotationAngle(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int nRotationAngle = 0;

    lc = (LayerCtrlContext*)l;

    if(lc == NULL)
    {
        loge("**the lc is null when get rotation angle!");
        return 0;
    }

    if(lc->pNativeWindow != NULL)
    {
        int nTransform     = 0;
        lc->pNativeWindow->query(lc->pNativeWindow, NATIVE_WINDOW_TRANSFORM_HINT,&nTransform);
        logv("################### nTransform = %d",nTransform);
        if(nTransform == 0)
        {
            lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_BUFFERS_TRANSFORM, 0);
            nRotationAngle = 0;
        }
        else if(nTransform == HAL_TRANSFORM_ROT_90)
        {
            lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_BUFFERS_TRANSFORM, 7);
            nRotationAngle = 90;
        }
        else if(nTransform == HAL_TRANSFORM_ROT_180)
        {
            lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_BUFFERS_TRANSFORM, 3);
            nRotationAngle = 180;
        }
        else if(nTransform == HAL_TRANSFORM_ROT_270)
        {
            lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_BUFFERS_TRANSFORM, 4);
            nRotationAngle = 270;
        }
        else
            nRotationAngle = 0;
    }
    else
        nRotationAngle = 0;

    logv("Layer get rotation angle , nRotationAngle = %d", nRotationAngle);

    return nRotationAngle;

}

int __LayerCtrlShowVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;
    logd("xxxx show video, current show flag = %d", lc->bLayerShowed);
    if(lc->bLayerShowed == 0)
    {
	lc->bLayerShowed = 1;
        lc->pNativeWindow->perform(lc->pNativeWindow,
                                   NATIVE_WINDOW_SETPARAMETER,
                                   HWC_LAYER_SHOW,
                                   1);
    }
    return 0;
}


int __LayerCtrlHideVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;
    logd("xxxx hide video, current show flag = %d", lc->bLayerShowed);
    if(lc->bLayerShowed == 1)
    {
	lc->bLayerShowed = 0;
        lc->pNativeWindow->perform(lc->pNativeWindow,
                                       NATIVE_WINDOW_SETPARAMETER,
                                       HWC_LAYER_SHOW,
                                       0);
    }
    return 0;
}


int __LayerCtrlIsVideoShow(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    return lc->bLayerShowed;
}


int  __LayerCtrlHoldLastPicture(LayerCtrl* l, int bHold)
{
    logd("LayerCtrlHoldLastPicture, bHold = %d", bHold);

    LayerCtrlContext* lc;
    lc = (LayerCtrlContext*)l;

    if(bHold == 0)
    {
        logw("*** the bHold is 0, not need to Hold last picture ***");
        return 0;
    }

    //* just on tvbox have the 0-copy
    if(lc->bRenderToHardwareLayer)
    {
        VideoPicture* lastPicture = NULL;
		//* find the latest pic which the pts is laragest
        for (int i = 0; i < NUM_OF_PICTURES_KEEP_IN_LIST; i++)
        {
		if (lc->pKeepPicNode[i].pPicture != NULL)
		{
                if(lastPicture == NULL
                   || (lastPicture->nPts < lc->pKeepPicNode[i].pPicture->nPts))
                {
			    lastPicture = lc->pKeepPicNode[i].pPicture;
                }
		}
        }
        if(lastPicture == NULL)
        {
            loge("find lastPicture failed");
            return -1;
        }

#if(LAYER_SUPPORT_ZERO_COPY && VIDEO_DIRECT_ACCESS_DE == 1)
        queueLastBufferZeroCpyAccessDe(lc, lastPicture);
#elif(LAYER_SUPPORT_ZERO_COPY)
        queueLastBufferZeroCpyNormal(lc, lastPicture);
#else
        logw("*** unsurpport __LayerCtrlHoldLastPicture");
#endif

        lc->keepLastFrameFlag = true;
    }
    return 0;
}

int __LayerDequeueBuffer(LayerCtrl* l, VideoPicture** ppBuf)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    *ppBuf = NULL;

#if(ZEROCOPY_DYNAMIC_CHECK == 1)
    checkGpuOutputType(l);
#endif

    if(lc->bRenderToHardwareLayer)
    {
        return LAYER_RESULT_USE_OUTSIDE_BUFFER;
    }
    else
    {
        //* dequeue a buffer from the native window object, set it to a picture buffer wrapper.
        int               i   = 0;
        int               err = -1;
        ANativeWindowBuffer* pWindowBuf = NULL;
        VideoPicture*          pPicture = NULL;
        void*                  pDataBuf = NULL;

        if(lc->bLayerInitialized == 0)
        {
            if(setLayerParam(lc) != 0)
            {
                loge("can not initialize layer.");
                return -1;
            }
            lc->bLayerInitialized = 1;
        }
        //* dequeue a buffer from the nativeWindow object.
        err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
        if(err != 0)
        {
            logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.", err);
            return -1;
        }
        lc->pNativeWindow->lockBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
        //* lock the data buffer.
        {
            GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
            Rect bounds(lc->nWidth, lc->nHeight);
            graphicMapper.lock(pWindowBuf->handle, lc->nUsage, bounds, &pDataBuf);
        }
        //* get a picture buffer wrapper.
        pPicture = NULL;
        for(i=0; i<32; i++)
        {
            if(lc->bBufferWrapperUsed[i] == 0)
            {
                lc->bBufferWrapperUsed[i] = 1;
                pPicture = &lc->bufferWrappers[i];
                break;
            }
        }
        if(i == 32)
        {
            loge("not enough picture buffer wrapper, shouldn't run here.");
            abort();
        }
        //* transform buffer into to VideoPicture struct
        memset(pPicture, 0, sizeof(VideoPicture));
        if(lc->need_get_phy_addr_flag)
        {
            uintptr_t nPhyaddress = getPhyAddrOfGpuBuffer(pWindowBuf);
            if(nPhyaddress == 0)
            {
                loge("get phy addr of gpu buffer failed");
                return -1;
            }
            pPicture->pData0       = (char*)nPhyaddress;
            pPicture->pData1       = (char*)(nPhyaddress + (pWindowBuf->height * pWindowBuf->stride));
            pPicture->pData2       = (char*)(nPhyaddress + (pWindowBuf->height * pWindowBuf->stride)*5/4);
            pPicture->phyYBufAddr  = nPhyaddress - VE_PHY_OFFSET;
            pPicture->phyCBufAddr  = nPhyaddress + (pWindowBuf->height * pWindowBuf->stride) - VE_PHY_OFFSET;
        }
        else
        {
            pPicture->pData0       = (char*)pDataBuf;
            pPicture->pData1       = pPicture->pData0 + (pWindowBuf->height * pWindowBuf->stride);
            pPicture->pData2       = pPicture->pData1 + (pWindowBuf->height * pWindowBuf->stride)/4;
        }
        pPicture->ePixelFormat = lc->eDisplayPixelFormat;
        pPicture->nWidth       = pWindowBuf->width;
        pPicture->nLineStride  = pWindowBuf->stride;
        pPicture->nHeight      = pWindowBuf->height;

        *ppBuf = pPicture;
        lc->pWindowBufs[i] = pWindowBuf;
        return 0;
    }
}

int __LayerDequeue3DBuffer(LayerCtrl* l, VideoPicture** ppBuf0, VideoPicture** ppBuf1)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    *ppBuf0 = NULL;
    *ppBuf1 = NULL;

    if(lc->bRenderToHardwareLayer)
    {
        return LAYER_RESULT_USE_OUTSIDE_BUFFER;
    }
    else
    {
        if(lc->bLayerInitialized == 0)
        {
            if(setLayerParam(lc) != 0)
            {
                loge("can not initialize layer.");
                return -1;
            }
            lc->bLayerInitialized = 1;
        }

        logw("can not render 3D picture(two seperated pictures) when not \
                rendering to hardware layer.");
        return __LayerDequeueBuffer(l, ppBuf0);
    }
}

int __LayerQueueBuffer(LayerCtrl* l, VideoPicture* pBuf, int bValid)
{
    LayerCtrlContext* lc  = NULL;
    int               i   = 0;
    int               ret = -1;

    lc = (LayerCtrlContext*)l;

#if (DEBUG_DUMP_PIC)
    savePictureData(lc, pBuf);
#endif

    if(lc->bRenderToHardwareLayer)
    {
        if(bValid == 0)
        {
            if(pBuf != NULL)
                lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf);
            return 0;
        }

        if(lc->bLayerInitialized == 0)
        {
            if(setLayerParam(lc) != 0)
            {
                loge("can not initialize layer.");
                return -1;
            }
            lc->bLayerInitialized = 1;
        }

        //* set picture to display hardware.
#if(LAYER_SUPPORT_ZERO_COPY && VIDEO_DIRECT_ACCESS_DE == 1)
        ret = queueBufferZeroCpyAccessDe(lc, pBuf, NULL, bValid);
#elif(LAYER_SUPPORT_ZERO_COPY)
        ret = queueBufferZeroCpyNormal(lc, pBuf, NULL, bValid);
#endif
        if(ret != 0)
        {
            loge("*** queueBuffer failed");
            return -1;
        }

    }
    else
    {
        queueBufferNormal(lc, pBuf, bValid);
    }

	if(lc->bFirstFrameIsShowed != 1)
	{
		lc->bFirstFrameIsShowed = 1;
	}
	return 0;
}

int __LayerQueue3DBuffer(LayerCtrl* l, VideoPicture* pBuf0, VideoPicture* pBuf1, int bValid)
{
    LayerCtrlContext* lc  = NULL;
    int               i   = 0;
    int               ret = -1;

    lc = (LayerCtrlContext*)l;

    if(lc->bRenderToHardwareLayer)
    {
        if(bValid == 0)
        {
            if(pBuf0 != NULL)
                lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf0);
            if(pBuf1 != NULL)
                lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf1);
            return 0;
        }

        if(lc->bLayerInitialized == 0)
        {
            if(setLayerParam(lc) != 0)
            {
                loge("can not initialize layer.");
                return -1;
            }

            lc->bLayerInitialized = 1;
        }

		//* the 0 copy on A20 is different
#if(LAYER_SUPPORT_ZERO_COPY && VIDEO_DIRECT_ACCESS_DE == 1)
        queueBufferZeroCpyAccessDe(lc, pBuf0, pBuf1, bValid);
#elif(LAYER_SUPPORT_ZERO_COPY)
        queueBufferZeroCpyNormal(lc, pBuf0, pBuf1, bValid);
#endif
        if(ret != 0)
        {
            loge("*** queueBuffer failed");
            return -1;
        }

		if(lc->bFirstFrameIsShowed != 1)
		{
			lc->bFirstFrameIsShowed = 1;
		}

        return 0;
    }
    else
    {
        logw("can not render 3D picture(two seperated pictures) when not \
                rendering to hardware layer.");
        if(bValid)
            return __LayerQueueBuffer(l, pBuf0, 1);
        else
            return __LayerQueueBuffer(l, pBuf0, 0);
    }
}

LayerControlOpsT mLayerControlOps =
{
    init:                       __LayerInit                      ,
    release:                    __LayerRelease                   ,

    setCallback:                __LayerSetCallback               ,
    setDisplayBufferSize:       __LayerSetDisplayBufferSize      ,
    setDisplayRegion:           __LayerSetDisplayRegion          ,
    setDisplayPixelFormat:      __LayerSetDisplayPixelFormat     ,
    setDeinterlaceFlag:         __LayerSetDeinterlaceFlag        ,
    setBufferTimeStamp:         __LayerSetBufferTimeStamp        ,

    getRotationAngle:           __LayerGetRotationAngle          ,

    ctrlShowVideo:              __LayerCtrlShowVideo             ,
    ctrlHideVideo:              __LayerCtrlHideVideo             ,
    ctrlIsVideoShow:		    __LayerCtrlIsVideoShow           ,
    ctrlHoldLastPicture:        __LayerCtrlHoldLastPicture       ,

    dequeueBuffer:              __LayerDequeueBuffer             ,
    dequeue3DBuffer:            __LayerDequeue3DBuffer           ,
    queueBuffer:                __LayerQueueBuffer               ,
    queue3DBuffer:              __LayerQueue3DBuffer
};

LayerControlOpsT* __GetLayerControlOps()
{
    return &mLayerControlOps;
}