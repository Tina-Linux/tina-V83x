/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : layerControl.cpp
* Description : surface display interface -- decoder and de share the buffer
* History :
*/

//#define CONFIG_LOG_LEVEL    OPTION_LOG_LEVEL_DETAIL
//#define LOG_TAG "layerControl_android_newDisplay"
#include "cdx_config.h"
#include "layerControl.h"
#include "cdx_log.h"
#include "memoryAdapter.h"
#include "outputCtrl.h"
#include <iniparserapi.h>
#include "CdcUtil.h"
#include <cutils/properties.h>

#if (((CONF_ANDROID_MAJOR_VER == 4)&&(CONF_ANDROID_SUB_VER == 2)))
#include <gui/ISurfaceTexture.h>
#elif ((CONF_ANDROID_MAJOR_VER >= 4))
#include <gui/Surface.h>
#else
    #error "invalid configuration of os version."
#endif

#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include <hardware/hwcomposer.h>

#if ((CONF_ANDROID_MAJOR_VER >= 4))
#include <hardware/hal_public.h>
#endif

#include <linux/ion.h>
#include <ion/ion.h>
#include <sys/mman.h>

#if (defined(CONF_PTS_TOSF))
#include "VideoFrameSchedulerWrap.h"
#endif

#if (defined(CONF_HIGH_DYNAMIC_RANGE_ENABLE) || defined(CONF_AFBC_ENABLE))
#include <hardware/sunxi_metadata_def.h>
#endif

#if defined(CONF_PRODUCT_STB)
#if ((CONF_ANDROID_MAJOR_VER == 4)&&(CONF_ANDROID_SUB_VER >= 4))
#include <gui/ISurfaceComposer.h>
//#include <private/gui/ComposerService.h> //* another way
#include <binder/IServiceManager.h>
#elif (CONF_ANDROID_MAJOR_VER == 7)
#include "libdispclient.h"
#endif
#endif

using namespace android;

#if defined(CONF_ION_HANDLE_POINTER)
typedef struct ion_handle* ion_handle_abstract_t;
#define ION_NULL_VALUE (NULL)
#elif defined(CONF_ION_HANDLE_INT)
typedef ion_user_handle_t ion_handle_abstract_t;
#define ION_NULL_VALUE (0)
#else
#error: not define ion_handle type.
#endif

#if defined(CONF_HIGH_DYNAMIC_RANGE_ENABLE)
#define CONF_10BIT_ENABLE
#endif

#define GPU_BUFFER_NUM 32

/* +1 allows queue after SetGpuBufferToDecoder */
#define NUM_OF_PICTURES_KEEP_IN_NODE (GetConfigParamterInt("pic_4list_num", 3) + 1)

typedef struct VPictureNode_t VPictureNode;
struct VPictureNode_t
{
    int                  bUsed;
    VideoPicture*        pPicture;
    ANativeWindowBuffer* pNodeWindowBuf;
};

enum BufferStatus
{
    UNRECOGNIZED,            // not a tracked buffer
    OWN_BY_NATIVE_WINDOW,
    OWN_BY_US,
    OWN_BY_DECODER,
};

typedef struct GpuBufferInfoS
{
    ANativeWindowBuffer* pWindowBuf;
    ion_handle_abstract_t handle_ion;
    int   nBufFd;
    char* pBufPhyAddr;
    char* pBufVirAddr;
    enum BufferStatus   mStatus;
    int   nUsedFlag;
    void* pMetaDataVirAddr;
    int   nMetaDataVirAddrSize;
    int   nMetaDataMapFd;
    ion_handle_abstract_t metadata_handle_ion;
    VideoPicture pPicture;
}GpuBufferInfoT;

typedef struct LayerCtrlContext
{
    LayerCtrl            base;
    ANativeWindow*       pNativeWindow;
    enum EPIXELFORMAT    eDisplayPixelFormat;
    enum EVIDEOCODECFORMAT eVideoCodecFormat;
    int                  nWidth;
    int                  nHeight;
    int                  nLeftOff;
    int                  nTopOff;
    int                  nDisplayWidth;
    int                  nDisplayHeight;
    int                  bLayerInitialized;
    int                  bLayerShowed;
    int                  bProtectFlag;

    //* use when render derect to hardware layer.
//    VPictureNode         picNodes[16];

    GpuBufferInfoT       mGpuBufferInfo[GPU_BUFFER_NUM];
    int                  nGpuBufferCount;
    int                  ionFd;
    int                  b4KAlignFlag;
    int                  bHoldLastPictureFlag;
    int                  bVideoWithTwoStreamFlag;
    int                  bIsSoftDecoderFlag;
    unsigned int         nUsage;
#if defined(CONF_PTS_TOSF)
//&& ((CONF_ANDROID_MAJOR_VER >= 5)||((CONF_ANDROID_MAJOR_VER == 4)&&(CONF_ANDROID_SUB_VER >= 4))))
    CdxVideoScheduler   *mVideoScheduler;
#endif
    int bHdrVideoFlag;
    int b10BitVideoFlag;
    int bAfbcModeFlag;
    int b3DDisplayFlag;
    int nVideoWidth;
    int nVideoHeight;

    //* there are some buffers cached in hwc at AndroidO,
    //* we should switch buffers in cache, or the number of buffers will double in case loop playback
    //* only swicth buffer in start of play, no need in  resetSurface.
    int bSwitchBuffer;

    int nSwitchNum;

    //* the buffer used by decoder & display,
    //* we should check whether the buffers are freed avoid memoryleak;
    int                 nUnFreeBufferCount;
}LayerCtrlContext;

static int getVirAddrOfMetadataBuffer(LayerCtrlContext* lc,
                                ANativeWindowBuffer* pWindowBuf,
                                ion_handle_abstract_t* pHandle_ion,
                                int* pMapfd,
                                int* pVirsize,
                                void** pViraddress)
{
    ion_handle_abstract_t handle_ion = ION_NULL_VALUE;
    int nMapfd = -1;
    unsigned char* nViraddress = 0;
    int nVirsize = 0;
    int ret = 0;

#if (defined(CONF_HIGH_DYNAMIC_RANGE_ENABLE) || defined(CONF_AFBC_ENABLE))
#if defined(CONF_GPU_MALI)
    private_handle_t* hnd = (private_handle_t *)(pWindowBuf->handle);
    if(hnd != NULL)
    {
        ret = ion_import(lc->ionFd, hnd->metadata_fd, &handle_ion);
        if(ret < 0)
        {
            loge("ion_import fail, maybe the buffer was free by display!");
            return -1;
        }
        nVirsize = hnd->ion_metadata_size;
    }
    else
    {
        logd("the hnd is wrong : hnd = %p", hnd);
        return -1;
    }

    ret = ion_map(lc->ionFd, handle_ion, nVirsize,
            PROT_READ | PROT_WRITE, MAP_SHARED, 0, &nViraddress, &nMapfd);
    if(ret < 0)
    {
        loge("ion_map fail!");
        if(nMapfd >= 0)
            close(nMapfd);
        ion_free(lc->ionFd, handle_ion);
        *pViraddress = 0;
        *pVirsize = 0;
        *pMapfd = 0;
        *pHandle_ion = 0;
        return -1;
    }
#else
#error invalid GPU type config
#endif

#endif

    *pViraddress = nViraddress;
    *pVirsize = nVirsize;
    *pMapfd = nMapfd;
    *pHandle_ion = handle_ion;
    return 0;
}

static int freeVirAddrOfMetadataBuffer(LayerCtrlContext* lc,
                                ion_handle_abstract_t handle_ion,
                                int mapfd,
                                int virsize,
                                void* viraddress)
{
#if (defined(CONF_HIGH_DYNAMIC_RANGE_ENABLE) || defined(CONF_AFBC_ENABLE))
    if (viraddress != 0) {
        munmap(viraddress, virsize);
    }
    if (mapfd != 0) {
        close(mapfd);
    }
    if (handle_ion != 0) {
        ion_free(lc->ionFd, handle_ion);
    }
#endif
    return 0;
}



#if defined(CONF_SEND_BLACK_FRAME_TO_GPU)
static int sendThreeBlackFrameToGpu(LayerCtrlContext* lc)
{
    logd("sendThreeBlackFrameToGpu()");

    ANativeWindowBuffer* pWindowBuf;
    void*                pDataBuf;
    int                  i;
    int                  err;
    int pic_4list = GetConfigParamterInt("pic_4list_num", 3);
    int ret = -1;
    void*  nViraddress = NULL;
    int nVirsize = 0;
    int nMapfd = -1;
    ion_handle_abstract_t metadata_handle_ion = ION_NULL_VALUE;

    if (lc->pNativeWindow == NULL || lc->bLayerInitialized == 0)
    {
        logv("skip %s", __func__);
         return 0;
    }

    native_window_set_buffers_format(lc->pNativeWindow, HAL_PIXEL_FORMAT_AW_NV12);
#if defined(CONF_HIGH_DYNAMIC_RANGE_ENABLE)
    native_window_set_buffers_data_space(lc->pNativeWindow, HAL_DATASPACE_UNKNOWN);
#endif
    for(i = 0; i < pic_4list; i++)
    {
        err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
        if(err != 0)
        {
            logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.",
                  err);
            return -1;
        }
        lc->pNativeWindow->lockBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

        //* lock the data buffer.
        {
            GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
            Rect bounds(lc->nWidth, lc->nHeight);
            unsigned int nUsage = lc->nUsage|GRALLOC_USAGE_SW_WRITE_OFTEN;
            graphicMapper.lock(pWindowBuf->handle, nUsage, bounds, &pDataBuf);
            logd("graphicMapper %p", pDataBuf);
        }

        if (pDataBuf) {
            memset((char*)pDataBuf,0x10,(pWindowBuf->height * pWindowBuf->stride));
            memset((char*)pDataBuf + pWindowBuf->height * pWindowBuf->stride,
                   0x80,(pWindowBuf->height * pWindowBuf->stride)/2);
        }

        ret = getVirAddrOfMetadataBuffer(lc, pWindowBuf, &metadata_handle_ion,
                &nMapfd, &nVirsize, &nViraddress);
        if(ret == -1)
        {
            loge("getVirAddrOfMetadataBuffer failed");
            return -1;
        }

#if (defined(CONF_HIGH_DYNAMIC_RANGE_ENABLE) || defined(CONFIG_AFBC_ENABLE))
        sunxi_metadata* s_metadata = (sunxi_metadata*)nViraddress;
        s_metadata->flag = 0;
#endif

        //* unlock the buffer.
        GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
        graphicMapper.unlock(pWindowBuf->handle);

        lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

        freeVirAddrOfMetadataBuffer(lc, metadata_handle_ion, nMapfd, nVirsize, nViraddress);
    }

    return 0;
}

// dequeue all nativewindow buffer from BufferQueue, then cancle all.
// this can make sure all buffer queued by player to render by surfaceflinger.
static int makeSureBlackFrameToShow(LayerCtrlContext* lc)
{
    logd("makeSureBlackFrameToShow()");

    ANativeWindowBuffer* pWindowBuf[32];
    void*                pDataBuf;
    int                  i;
    int                  err;
    int                     bufCnt = lc->nGpuBufferCount;

    for(i = 0;i < bufCnt -1; i++)
    {
        err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf[i]);
        if(err != 0)
        {
            logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.",
                  err);
            break;
        }

        logv("dequeue i = %d, handle: 0x%x", i, pWindowBuf[i]->handle);

    }

    for(i--; i >= 0; i--)
    {
        logv("cancel i = %d, handle: 0x%x", i, pWindowBuf[i]->handle);
        lc->pNativeWindow->cancelBuffer(lc->pNativeWindow, pWindowBuf[i], -1);
    }

    return 0;
}
#endif

//* this function for 3D/2D rotate case.
//* just init alignment buffer to black color.
//* (when the two stream display to 2D, the 32-line buffer will cause "Green Screen" if not init,
//*  as buffer have make 32-align)
//* when the display to 2D,the appearance of "Green edges" in ratation if not init.
//* if init the whole buffer, it would take too much time.
int initPartialGpuBuffer(char* pDataBuf, ANativeWindowBuffer* pWindowBuf, LayerCtrlContext* lc)
{
    logv("initGpuBuffer, stride = %d, height = %d, ",pWindowBuf->stride,pWindowBuf->height);

    if(lc->bVideoWithTwoStreamFlag == 1)
    {
        if(lc->eDisplayPixelFormat == PIXEL_FORMAT_NV21)
        {
            //* Y1
            int nRealHeight = pWindowBuf->height/2;
            int nInitHeight = GetConfigParamterInt("gpu_align_bitwidth", 32);
            int nSkipLen = pWindowBuf->stride*(nRealHeight - nInitHeight);
            int nCpyLenY = pWindowBuf->stride*nInitHeight;
            memset(pDataBuf+nSkipLen, 0x10, nCpyLenY);
            //* Y2
            nSkipLen += pWindowBuf->stride*nRealHeight;
            memset(pDataBuf+nSkipLen, 0x10, nCpyLenY);

            //*UV1
            nSkipLen += nCpyLenY;
            nSkipLen += (pWindowBuf->stride)*(nRealHeight/2 - nInitHeight/2);
            int nCpyLenUV = (pWindowBuf->stride)*(nInitHeight/2);
            memset(pDataBuf+nSkipLen, 0x80, nCpyLenUV);
            //*UV2
            nSkipLen += (pWindowBuf->stride)*(nRealHeight/2);
            memset(pDataBuf+nSkipLen, 0x80, nCpyLenUV);
        }
        else
        {
            loge("the pixelFormat is not support when initPartialGpuBuffer, pixelFormat = %d",
                  lc->eDisplayPixelFormat);
            return -1;
        }
    }
    else
    {
        if(lc->eDisplayPixelFormat == PIXEL_FORMAT_NV21)
        {
            //* Y
            int nRealHeight = pWindowBuf->height;
            int nInitHeight = GetConfigParamterInt("gpu_align_bitwidth", 32);
            int nSkipLen = pWindowBuf->stride*(nRealHeight - nInitHeight);
            int nCpyLenY = pWindowBuf->stride*nInitHeight;
            memset(pDataBuf+nSkipLen, 0x10, nCpyLenY);

            //*UV
            nSkipLen += nCpyLenY;
            nSkipLen += (pWindowBuf->stride)*(nRealHeight/2 - nInitHeight/2);
            int nCpyLenUV = (pWindowBuf->stride)*(nInitHeight/2);
            memset(pDataBuf+nSkipLen, 0x80, nCpyLenUV);
        }
        else if(lc->eDisplayPixelFormat == PIXEL_FORMAT_YV12)
        {
            if(lc->eVideoCodecFormat == VIDEO_CODEC_FORMAT_VP9 && lc->bIsSoftDecoderFlag)
            {
                //* Y
                int nRealHeight = pWindowBuf->height;
                int nSkipLen = pWindowBuf->stride*nRealHeight;
                memset(pDataBuf, 0x10, pWindowBuf->stride*nRealHeight);

                //* UV
                memset(pDataBuf+nSkipLen,0x80,nSkipLen/2);
            }
            else
            {
                //* Y
                int nRealHeight = pWindowBuf->height;
                int nInitHeight = GetConfigParamterInt("gpu_align_bitwidth", 32);
                int nSkipLen = pWindowBuf->stride*(nRealHeight - nInitHeight);
                int nCpyLenY = pWindowBuf->stride*nInitHeight;
                memset(pDataBuf+nSkipLen, 0x10, nCpyLenY);

                //*U
                nSkipLen += nCpyLenY;
                nSkipLen += (pWindowBuf->stride/2)*(nRealHeight/2 - nInitHeight/2);
                int nCpyLenU = (pWindowBuf->stride/2)*(nInitHeight/2);
                memset(pDataBuf+nSkipLen, 0x80, nCpyLenU);

                //*V
                nSkipLen += nCpyLenU;
                nSkipLen += (pWindowBuf->stride/2)*(nRealHeight/2 - nInitHeight/2);
                int nCpyLenV = (pWindowBuf->stride/2)*(nInitHeight/2);
                memset(pDataBuf+nSkipLen, 0x80, nCpyLenV);
            }
        }
    }

    return 0;
}

//* copy from ACodec.cpp
static int pushBlankBuffersToNativeWindow(LayerCtrlContext* lc)
{
    logd("pushBlankBuffersToNativeWindow: pNativeWindow = %p",lc->pNativeWindow);

    if(lc->pNativeWindow == NULL)
    {
        logw(" the nativeWindow is null when call pushBlankBuffersToNativeWindow");
        return 0;
    }
    status_t eErr = NO_ERROR;
    ANativeWindowBuffer* pWindowBuffer = NULL;
    int nNumBufs = 0;
    int nMinUndequeuedBufs = 0;
    ANativeWindowBuffer **pArrBuffer = NULL;

    // We need to reconnect to the ANativeWindow as a CPU client to ensure that
    // no frames get dropped by SurfaceFlinger assuming that these are video
    // frames.
    eErr = native_window_api_disconnect(lc->pNativeWindow,NATIVE_WINDOW_API_MEDIA);
    if (eErr != NO_ERROR) {
        loge("error push blank frames: native_window_api_disconnect failed: %s (%d)",
                strerror(-eErr), -eErr);
        return eErr;
    }

    eErr = native_window_api_connect(lc->pNativeWindow,NATIVE_WINDOW_API_CPU);
    if (eErr != NO_ERROR) {
        loge("error push blank frames: native_window_api_connect failed: %s (%d)",
                strerror(-eErr), -eErr);
        return eErr;
    }

#if (CONF_ANDROID_MAJOR_VER >= 5)
    eErr = lc->pNativeWindow->perform(lc->pNativeWindow,
                            NATIVE_WINDOW_SET_BUFFERS_GEOMETRY,
                            0,
                            0,
                            HAL_PIXEL_FORMAT_RGBX_8888);
#else
    eErr = native_window_set_buffers_geometry(lc->pNativeWindow, 1, 1,
            HAL_PIXEL_FORMAT_RGBX_8888);
#endif
    if (eErr != NO_ERROR) {
        loge("set buffers geometry of nativeWindow failed: %s (%d)",
                strerror(-eErr), -eErr);
        goto error;
    }

    eErr = native_window_set_scaling_mode(lc->pNativeWindow,
                NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    if (eErr != NO_ERROR) {
        loge("error push blank_frames: native_window_set_scaling_mode failed: %s (%d)",
              strerror(-eErr), -eErr);
        goto error;
    }

    eErr = native_window_set_usage(lc->pNativeWindow,
            GRALLOC_USAGE_SW_WRITE_OFTEN);
    if (eErr != NO_ERROR) {
        loge("error push blank frames: native_window_set_usage failed: %s (%d)",
                strerror(-eErr), -eErr);
        goto error;
    }

    eErr = lc->pNativeWindow->query(lc->pNativeWindow,
            NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &nMinUndequeuedBufs);
    if (eErr != NO_ERROR) {
        loge("query  NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS failed: %s (%d)",
              strerror(-eErr), -eErr);
        goto error;
    }

    nNumBufs = nMinUndequeuedBufs + 1;
    if (nNumBufs < 3)
        nNumBufs = 3;
    eErr = native_window_set_buffer_count(lc->pNativeWindow, nNumBufs);
    if (eErr != NO_ERROR) {
        loge("set buffer count of nativeWindow failed: %s (%d)",
                strerror(-eErr), -eErr);
        goto error;
    }

    // We  push nNumBufs + 1 buffers to ensure that we've drawn into the same
    // buffer twice.  This should guarantee that the buffer has been displayed
    // on the screen and then been replaced, so an previous video frames are
    // guaranteed NOT to be currently displayed.

    logd("nNumBufs=%d", nNumBufs);
    //* we just push nNumBufs.If push numBus+1,it will be problem in suspension window
    for (int i = 0; i < nNumBufs; i++) {
        int fenceFd = -1;
        eErr = native_window_dequeue_buffer_and_wait(lc->pNativeWindow, &pWindowBuffer);
        if (eErr != NO_ERROR) {
            loge("error: native_window_dequeue_buffer_and_wait failed: %s (%d)",
                    strerror(-eErr), -eErr);
            goto error;
        }

#if (CONF_ANDROID_MAJOR_VER >= 8)
        sp<GraphicBuffer> mGraphicBuffer(GraphicBuffer::from(pWindowBuffer));
#else
        sp<GraphicBuffer> mGraphicBuffer(new GraphicBuffer(pWindowBuffer, false));
#endif

        // Fill the buffer with the a 1x1 checkerboard pattern ;)
        uint32_t* pImg = NULL;
        eErr = mGraphicBuffer->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)(&pImg));
        if (eErr != NO_ERROR) {
            loge("error push blank frames: lock failed: %s (%d)",
                    strerror(-eErr), -eErr);
            goto error;
        }

        *pImg = 0;

        eErr = mGraphicBuffer->unlock();
        if (eErr != NO_ERROR) {
            loge("error push blank frames: unlock failed: %s (%d)",
                    strerror(-eErr), -eErr);
            goto error;
        }

        eErr = lc->pNativeWindow->queueBuffer(lc->pNativeWindow,
                mGraphicBuffer->getNativeBuffer(), -1);
        if (eErr != NO_ERROR) {
            loge("lc->pNativeWindow->queueBuffer failed: %s (%d)",
                    strerror(-eErr), -eErr);
            goto error;
        }

        pWindowBuffer = NULL;
    }

    pArrBuffer = (ANativeWindowBuffer **)malloc((nNumBufs)*sizeof(ANativeWindowBuffer*));
    for (int i = 0; i < nNumBufs-1; ++i) {
        eErr = native_window_dequeue_buffer_and_wait(lc->pNativeWindow, &pArrBuffer[i]);
        if (eErr != NO_ERROR) {
            loge("native_window_dequeue_buffer_and_wait failed: %s (%d)",
                  strerror(-eErr), -eErr);
            goto error;
        }
    }
    for (int i = 0; i < nNumBufs-1; ++i) {
        lc->pNativeWindow->cancelBuffer(lc->pNativeWindow, pArrBuffer[i], -1);
    }
    free(pArrBuffer);
    pArrBuffer = NULL;

error:

    if (eErr != NO_ERROR) {
        // Clean up after an error.
        if (pWindowBuffer != NULL) {
            lc->pNativeWindow->cancelBuffer(lc->pNativeWindow, pWindowBuffer, -1);
        }

        if (pArrBuffer) {
            free(pArrBuffer);
        }

        native_window_api_disconnect(lc->pNativeWindow,
                NATIVE_WINDOW_API_CPU);
        native_window_api_connect(lc->pNativeWindow,
                NATIVE_WINDOW_API_MEDIA);

        return eErr;
    } else {
        // Clean up after success.
        eErr = native_window_api_disconnect(lc->pNativeWindow,
                NATIVE_WINDOW_API_CPU);
        if (eErr != NO_ERROR) {
            loge("native_window_api_disconnect failed: %s (%d)",
                    strerror(-eErr), -eErr);
            return eErr;
        }

        eErr = native_window_api_connect(lc->pNativeWindow,
                NATIVE_WINDOW_API_MEDIA);
        if (eErr != NO_ERROR) {
            loge("native_window_api_connect failed: %s (%d)",
                    strerror(-eErr), -eErr);
            return eErr;
        }

        return 0;
    }
}

static cdx_int64 GetNowUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_sec * 1000000ll + tv.tv_usec;
}

//* set usage, scaling_mode, pixelFormat, buffers_geometry, buffers_count, crop
static int setLayerParam(LayerCtrlContext* lc)
{
    logd("setLayerParam: PixelFormat(%d), nW(%d), nH(%d), leftoff(%d), topoff(%d)",
          lc->eDisplayPixelFormat,lc->nWidth,
          lc->nHeight,lc->nLeftOff,lc->nTopOff);
    logd("setLayerParam: dispW(%d), dispH(%d), buffercount(%d), bProtectFlag(%d),\
          bIsSoftDecoderFlag(%d)",
          lc->nDisplayWidth,lc->nDisplayHeight,lc->nGpuBufferCount,
          lc->bProtectFlag,lc->bIsSoftDecoderFlag);

    int          pixelFormat;
    unsigned int nGpuBufWidth;
    unsigned int nGpuBufHeight;
    Rect         crop;
    lc->nUsage   = 0;

    //* add the protected usage when the video is secure
    if(lc->bProtectFlag == 1)
    {
        // Verify that the ANativeWindow sends images directly to
        // SurfaceFlinger.
        int nErr = -1;
        int nQueuesToNativeWindow = 0;
        nErr = lc->pNativeWindow->query(
                lc->pNativeWindow, NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER,
                &nQueuesToNativeWindow);
        if (nErr != 0) {
            loge("error authenticating native window: %d", nErr);
            return nErr;
        }
        if (nQueuesToNativeWindow != 1) {
            loge("nQueuesToNativeWindow is not 1, PERMISSION_DENIED");
            return PERMISSION_DENIED;
        }
        logd("set usage to GRALLOC_USAGE_PROTECTED");
        lc->nUsage |= GRALLOC_USAGE_PROTECTED;
    }

    if(lc->bIsSoftDecoderFlag == 1)
    {
        //* gpu use this usage to malloc buffer with cache.
        lc->nUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN;
    }
    else
    {
        //* gpu use this usage to malloc continuous physical buffer.
        lc->nUsage |= GRALLOC_USAGE_HW_2D;
        //* cpu cannot WRITE and READ the buffer if the video is protect.
        //* when it is not protect, cpu will W the buffer, but not often.
        if(lc->bProtectFlag == 0)
        {
            //lc->nUsage |= GRALLOC_USAGE_SW_WRITE_RARELY;
        }
    }

    //if(lc->bVideoWithTwoStreamFlag == 1)
    lc->nUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN; // we will memset bottom green screen

    //* add other usage
    lc->nUsage |= GRALLOC_USAGE_SW_READ_NEVER     |
                  GRALLOC_USAGE_HW_TEXTURE        |
                  GRALLOC_USAGE_EXTERNAL_DISP;

#if (defined(CONF_AFBC_ENABLE))
    // do not need set the flag GRALLOC_USAGE_METADATA_BUF;
    // gralloc will alloc metadata buffer in all mode
    if (lc->bAfbcModeFlag)
        lc->nUsage |= GRALLOC_USAGE_AFBC_MODE;
#endif

    switch(lc->eDisplayPixelFormat)
    {
        case PIXEL_FORMAT_YV12:             //* why YV12 use this pixel format.
        {
            logd("++++++++ yv12");
            pixelFormat = HAL_PIXEL_FORMAT_YV12;

#if (defined(CONF_10BIT_ENABLE))
            if(lc->b10BitVideoFlag)
                pixelFormat = HAL_PIXEL_FORMAT_AW_YV12_10bit;
#endif

            break;
        }
        case PIXEL_FORMAT_NV21:
        {
            logd("++++++++++++++ nv21");
            pixelFormat = HAL_PIXEL_FORMAT_YCrCb_420_SP;

#if (defined(CONF_10BIT_ENABLE))
            if(lc->b10BitVideoFlag)
                pixelFormat = HAL_PIXEL_FORMAT_AW_NV21_10bit;
#endif
            break;
        }
        case PIXEL_FORMAT_NV12: //* display system do not support NV12.
        {
            loge("=+++++ it is nv12, just for test");
            pixelFormat = HAL_PIXEL_FORMAT_AW_NV12; //HAL_PIXEL_FORMAT_AW_NV12;

#if (defined(CONF_10BIT_ENABLE))
            if(lc->b10BitVideoFlag)
                pixelFormat = HAL_PIXEL_FORMAT_AW_NV12_10bit;
#endif
            break;
        }
#if (defined(CONF_10BIT_ENABLE))
        case PIXEL_FORMAT_P010_UV:
        {
            pixelFormat = HAL_PIXEL_FORMAT_AW_P010_UV;
            break;
        }
        case PIXEL_FORMAT_P010_VU:
        {
            pixelFormat = HAL_PIXEL_FORMAT_AW_P010_VU;
            break;
        }
#endif

        default:
        {
            loge("unsupported pixel format.");
            return -1;
        }
    }

    nGpuBufWidth  = lc->nWidth;  //* restore nGpuBufWidth to mWidth;
    nGpuBufHeight = lc->nHeight;

    //* We should double the height if the video with two stream,
    //* so the nativeWindow will malloc double buffer
    if(lc->bVideoWithTwoStreamFlag == 1)
    {
        nGpuBufHeight = 2*nGpuBufHeight;
    }

    crop.left   = lc->nLeftOff;
    crop.top    = lc->nTopOff;
    crop.right  = lc->nLeftOff + lc->nDisplayWidth;
    crop.bottom = lc->nTopOff + lc->nDisplayHeight;
    logd("crop (l,t,r,b); %d %d %d %d", crop.left, crop.top, crop.right, crop.bottom);

    if(lc->nGpuBufferCount <= 0)
    {
        loge("error: the lc->nGpuBufferCount[%d] is invalid!",lc->nGpuBufferCount);
        return -1;
    }

    native_window_set_usage(lc->pNativeWindow,lc->nUsage);
    native_window_set_scaling_mode(lc->pNativeWindow, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

#if defined(CONF_PRODUCT_STB)
    lc->pNativeWindow->perform(lc->pNativeWindow,
                            NATIVE_WINDOW_SET_BUFFERS_GEOMETRY,
                            nGpuBufWidth,
                            nGpuBufHeight,
                            pixelFormat);

#endif

    native_window_set_buffers_dimensions(lc->pNativeWindow, nGpuBufWidth, nGpuBufHeight);
    native_window_set_buffers_format(lc->pNativeWindow, pixelFormat);
    native_window_set_buffer_count(lc->pNativeWindow,lc->nGpuBufferCount);
    lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_CROP, &crop);
    return 0;
}

static int switchBuffer(LayerCtrlContext* lc)
{
    int64_t start=GetNowUs();
    ANativeWindowBuffer*      pWindowBuf = NULL;
    int i=0, j=0;
    struct checkBuffer
    {
        ANativeWindowBuffer*      pWindowBuf;
        int bDeque;
    };
    void*   pDataBuf    = NULL;
    int YSize = lc->nWidth*lc->nHeight;
    struct checkBuffer* bufferV = (struct checkBuffer*)malloc(lc->nGpuBufferCount*sizeof(struct checkBuffer));
    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        do
        {
            lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
            for(j=0; j<i; j++)
            {
                if(bufferV[j].pWindowBuf == pWindowBuf)
                {
                    bufferV[j].bDeque = 1;
                    break;
                }
            }

        } while(j != i);

        bufferV[i].pWindowBuf = pWindowBuf;
        bufferV[i].bDeque = 1;


        lc->pNativeWindow->lockBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
        GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
        Rect bounds(lc->nWidth, lc->nHeight);
        graphicMapper.lock(pWindowBuf->handle,lc->nUsage, bounds, &pDataBuf);
        memset(pDataBuf, 0x10, YSize);
        memset((char*)pDataBuf+YSize, 0x80, YSize/2);
        graphicMapper.unlock(pWindowBuf->handle);

        lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

        bufferV[i].bDeque = 0;
    }

    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        if(bufferV[i].bDeque)
        {
            lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, bufferV[i].pWindowBuf);
            bufferV[i].bDeque = 0;
        }
    }
    free(bufferV);

    logd("=== diff: %lld", GetNowUs()-start);
    return 0;
}

int getPhyAddrOfGpuBuffer(LayerCtrlContext* lc,
                                ANativeWindowBuffer* pWindowBuf,
                                ion_handle_abstract_t* pHandle_ion,
                                uintptr_t*  pPhyaddress,
                                int* pBufFd)
{
    ion_handle_abstract_t handle_ion = ION_NULL_VALUE;
    uintptr_t  nPhyaddress = 0;
    int ret;

#if defined(CONF_GPU_MALI)
    private_handle_t* hnd = (private_handle_t *)(pWindowBuf->handle);
    if(hnd != NULL)
    {
        ret = ion_import(lc->ionFd, hnd->share_fd, &handle_ion);
        if(ret < 0)
        {
            loge("ion_import fail, maybe the buffer was free by display!");
            return -1;
        }
    }
    else
    {
        logd("the hnd is wrong : hnd = %p",hnd);
        return -1;
    }
#elif defined(CONF_GPU_IMG)
    IMG_native_handle_t* hnd = (IMG_native_handle_t*)(pWindowBuf->handle);
    if(hnd != NULL)
    {
        ret = ion_import(lc->ionFd, hnd->fd[0], &handle_ion);
        if(ret < 0)
        {
            loge("ion_import fail, maybe the buffer was free by display!");
            return -1;
        }
    }
    else
    {
        logd("the hnd is wrong : hnd = %p",hnd);
        return -1;
    }
#else
#error invalid GPU type config
#endif

    //* we should get the phyaddr, because queueBuffer and queueBuffer need
    //* get the buffer id according the phyaddr
    //if(lc->bIsSoftDecoderFlag == 0)
    {
        if(lc->ionFd >= 0)
        {
            nPhyaddress = CdcIonGetPhyAdr(lc->ionFd, (uintptr_t)handle_ion);
            *pBufFd = CdcIonGetFd(lc->ionFd, (uintptr_t)handle_ion);
        }
        else
        {
            logd("the ion fd is wrong : fd = %d",lc->ionFd);
            return -1;
        }

        nPhyaddress -= CONF_VE_PHY_OFFSET;
    }
    *pPhyaddress = nPhyaddress;
    *pHandle_ion = handle_ion;
    return 0;
}

VideoPicture* dequeueTheInitGpuBuffer(LayerCtrlContext* lc)
{
    int     err = -1;
    int     i   = 0;
    int     nCancelNum      = 0;
    int     nNeedCancelFlag = 0;
    int     nCancelIndex[GPU_BUFFER_NUM] = {-1};

    ANativeWindowBuffer*      pWindowBuf = NULL;
    int ret = -1;
    uintptr_t  nPhyaddress = 0;
    int nBufFd = -1;
    ion_handle_abstract_t handle_ion = ION_NULL_VALUE;
    void*  nViraddress = NULL;
    int nVirsize = 0;
    int nMapfd = -1;
    ion_handle_abstract_t metadata_handle_ion = ION_NULL_VALUE;

dequeue_buffer:

    //* queueBuffer in switchBuffer, so there is a buffer in surfaceflinger cannot be dequeue.
    //* we must queue a buffer in order to dequeu the last buffer
    if(lc->nSwitchNum == lc->nGpuBufferCount-1 && lc->bSwitchBuffer)
    {
        lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow,
            lc->mGpuBufferInfo[lc->nGpuBufferCount-3].pWindowBuf);
        lc->mGpuBufferInfo[lc->nGpuBufferCount-3].mStatus = OWN_BY_NATIVE_WINDOW;
        lc->nSwitchNum --;
    }

    //* dequeue a buffer from the nativeWindow object.
    err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
    if(err != 0)
    {
        logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.",
              err);
        return NULL;
    }

    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        if(lc->mGpuBufferInfo[i].nUsedFlag == 1 &&
            pWindowBuf == lc->mGpuBufferInfo[i].pWindowBuf)
        {
            nNeedCancelFlag = 1;
            nCancelIndex[nCancelNum] = i;
            nCancelNum++;
            logv("the buffer[%p] not return, dequeue again!", pWindowBuf);
            goto dequeue_buffer;
        }
    }

    int j=0;
    if(nNeedCancelFlag == 1)
    {
        for(j = 0; j<nCancelNum; j++)
        {
            int nIndex = nCancelIndex[j];
            ANativeWindowBuffer* pTmpWindowBuf = lc->mGpuBufferInfo[nIndex].pWindowBuf;
            lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pTmpWindowBuf);
        }
        nCancelNum = 0;
        nNeedCancelFlag = 0;
    }

    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        if(lc->mGpuBufferInfo[i].pWindowBuf == NULL)
        {
            lc->mGpuBufferInfo[i].mStatus = OWN_BY_US;
            break;
        }
    }

    if(i == lc->nGpuBufferCount)
    {
        loge("cannot find the buffer");
        return NULL;
    }

    lc->nSwitchNum ++;

    ret = getPhyAddrOfGpuBuffer(lc, pWindowBuf, &handle_ion, &nPhyaddress, &nBufFd);
    if(ret == -1)
    {
        loge("getPhyAddrOfGpuBuffer failed");
        return NULL;
    }

    //* get the virtual address
    lc->pNativeWindow->lockBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
    GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
    Rect bounds(lc->nWidth, lc->nHeight);
    graphicMapper.lock(pWindowBuf->handle,lc->nUsage, bounds, &nViraddress);

    // if witch buffer( the pixels have init black), we donot need init partial buffer
    initPartialGpuBuffer((char*)nViraddress, pWindowBuf, lc);
    lc->mGpuBufferInfo[i].pWindowBuf   = pWindowBuf;
    lc->mGpuBufferInfo[i].handle_ion   = handle_ion;
    lc->mGpuBufferInfo[i].pBufVirAddr  = (char*)nViraddress;
    lc->mGpuBufferInfo[i].pBufPhyAddr  = (char*)nPhyaddress;
    lc->mGpuBufferInfo[i].nBufFd       = nBufFd;
    lc->mGpuBufferInfo[i].mStatus = OWN_BY_DECODER;
    lc->mGpuBufferInfo[i].nUsedFlag = 1;

    lc->nUnFreeBufferCount++;

    private_handle_t *hnd = (private_handle_t*)pWindowBuf->handle;
    logd("==== init: buf(fd: %d), nUnFreeBufferCount: %d, aw_buf_id: %lld, private_handle_t: %p", nBufFd, lc->nUnFreeBufferCount, hnd->aw_buf_id, hnd);

    ret = getVirAddrOfMetadataBuffer(lc, pWindowBuf, &metadata_handle_ion,
           &nMapfd, &nVirsize, &nViraddress);
    if(ret == -1)
    {
       loge("getVirAddrOfMetadataBuffer failed");
       return NULL;
    }
    lc->mGpuBufferInfo[i].pMetaDataVirAddr = nViraddress;
    lc->mGpuBufferInfo[i].nMetaDataVirAddrSize = nVirsize;
    lc->mGpuBufferInfo[i].nMetaDataMapFd = nMapfd;
    lc->mGpuBufferInfo[i].metadata_handle_ion = metadata_handle_ion;

    VideoPicture* pPicture = &lc->mGpuBufferInfo[i].pPicture;
    //* set the buffer address
    pPicture->pData0       = lc->mGpuBufferInfo[i].pBufVirAddr;

#if (defined(CONF_10BIT_ENABLE))
    if((lc->eDisplayPixelFormat == PIXEL_FORMAT_P010_UV)
        ||(lc->eDisplayPixelFormat == PIXEL_FORMAT_P010_VU))
    {
        pPicture->pData1       = pPicture->pData0 + (pWindowBuf->height * pWindowBuf->stride*2);
        pPicture->pData2       = pPicture->pData1 + (pWindowBuf->height * pWindowBuf->stride)/2;
        pPicture->phyYBufAddr  = (uintptr_t)lc->mGpuBufferInfo[i].pBufPhyAddr;
        pPicture->phyCBufAddr  = pPicture->phyYBufAddr + (pWindowBuf->height * pWindowBuf->stride*2);
    }
    else
#endif
    {
        pPicture->pData1       = pPicture->pData0 + (pWindowBuf->height * pWindowBuf->stride);
        pPicture->pData2       = pPicture->pData1 + (pWindowBuf->height * pWindowBuf->stride)/4;
        pPicture->phyYBufAddr  = (uintptr_t)lc->mGpuBufferInfo[i].pBufPhyAddr;
        pPicture->phyCBufAddr  = pPicture->phyYBufAddr + (pWindowBuf->height * pWindowBuf->stride);
    }
    pPicture->nBufId       = i;
    pPicture->pPrivate     = (void*)(uintptr_t)lc->mGpuBufferInfo[i].handle_ion;
    pPicture->nBufFd       = nBufFd;
    pPicture->ePixelFormat = lc->eDisplayPixelFormat;
    pPicture->nWidth       = pWindowBuf->stride;
    pPicture->nHeight      = pWindowBuf->height;
    pPicture->nLineStride  = pWindowBuf->stride;

    pPicture->pMetaData    = lc->mGpuBufferInfo[i].pMetaDataVirAddr;

    if(lc->b4KAlignFlag == 1)
    {
        uintptr_t tmpAddr = (uintptr_t)pPicture->pData1;
        tmpAddr     = (tmpAddr + 4095) & ~4095;
        pPicture->pData1      = (char *)tmpAddr;
        pPicture->phyCBufAddr = (pPicture->phyCBufAddr + 4095) & ~4095;
    }

    return pPicture;
}

#if (defined(CONF_HIGH_DYNAMIC_RANGE_ENABLE))
static android_dataspace_t getDataspaceFromVideoPicture(const VideoPicture *pic)
{
    uint32_t space = HAL_DATASPACE_UNKNOWN;

    switch (pic->transfer_characteristics)
    {
        case VIDEO_TRANSFER_RESERVED_0:
        case VIDEO_TRANSFER_BT1361:
        case VIDEO_TRANSFER_UNSPECIFIED:
        case VIDEO_TRANSFER_RESERVED_1:
            space |= HAL_DATASPACE_TRANSFER_UNSPECIFIED;
            break;
        case VIDEO_TRANSFER_GAMMA2_2:
            space |= HAL_DATASPACE_TRANSFER_GAMMA2_2;
            break;
        case VIDEO_TRANSFER_GAMMA2_8:
            space |= HAL_DATASPACE_TRANSFER_GAMMA2_8;
            break;
        case VIDEO_TRANSFER_SMPTE_170M:
            space |= HAL_DATASPACE_TRANSFER_SMPTE_170M;
            break;
        case VIDEO_TRANSFER_SMPTE_240M:
            space |= HAL_DATASPACE_TRANSFER_UNSPECIFIED;
            break;
        case VIDEO_TRANSFER_LINEAR:
            space |= HAL_DATASPACE_TRANSFER_LINEAR;
            break;
        case VIDEO_TRANSFER_LOGARITHMIC_0:
        case VIDEO_TRANSFER_LOGARITHMIC_1:
        case VIDEO_TRANSFER_IEC61966:
        case VIDEO_TRANSFER_BT1361_EXTENDED:
            space |= HAL_DATASPACE_TRANSFER_UNSPECIFIED;
            break;
        case VIDEO_TRANSFER_SRGB:
            space |= HAL_DATASPACE_TRANSFER_SRGB;
            break;
        case VIDEO_TRANSFER_BT2020_0:
        case VIDEO_TRANSFER_BT2020_1:
            space |= HAL_DATASPACE_TRANSFER_UNSPECIFIED;
            break;
        case VIDEO_TRANSFER_ST2084:
            space |= HAL_DATASPACE_TRANSFER_ST2084;
            break;
        case VIDEO_TRANSFER_ST428_1:
            space |= HAL_DATASPACE_TRANSFER_UNSPECIFIED;
            break;
        case VIDEO_TRANSFER_HLG:
            space |= HAL_DATASPACE_TRANSFER_HLG;
            break;
        default:
            space |= HAL_DATASPACE_TRANSFER_UNSPECIFIED;
            break;
    }

    switch (pic->matrix_coeffs)
    {
        case VIDEO_MATRIX_COEFFS_IDENTITY:
            space |= HAL_DATASPACE_STANDARD_UNSPECIFIED;
            break;
        case VIDEO_MATRIX_COEFFS_BT709:
            space |= HAL_DATASPACE_STANDARD_BT709;
            break;
        case VIDEO_MATRIX_COEFFS_UNSPECIFIED_0:
        case VIDEO_MATRIX_COEFFS_RESERVED_0:
            space |= HAL_DATASPACE_STANDARD_UNSPECIFIED;
            break;
        case VIDEO_MATRIX_COEFFS_BT470M:
            space |= HAL_DATASPACE_STANDARD_BT470M;
            break;
        case VIDEO_MATRIX_COEFFS_BT601_625_0:
            space |= HAL_DATASPACE_BT601_625;
            break;
        case VIDEO_MATRIX_COEFFS_BT601_625_1:
            space |= HAL_DATASPACE_BT601_525;
            break;
        case VIDEO_MATRIX_COEFFS_SMPTE_240M:
        case VIDEO_MATRIX_COEFFS_YCGCO:
            space |= HAL_DATASPACE_STANDARD_UNSPECIFIED;
            break;
        case VIDEO_MATRIX_COEFFS_BT2020:
            space |= HAL_DATASPACE_STANDARD_BT2020;
            break;
        case VIDEO_MATRIX_COEFFS_BT2020_CONSTANT_LUMINANCE:
            space |= HAL_DATASPACE_STANDARD_BT2020_CONSTANT_LUMINANCE;
            break;
        case VIDEO_MATRIX_COEFFS_SOMPATE:
        case VIDEO_MATRIX_COEFFS_CD_NON_CONSTANT_LUMINANCE:
        case VIDEO_MATRIX_COEFFS_CD_CONSTANT_LUMINANCE:
        case VIDEO_MATRIX_COEFFS_BTICC:
            space |= HAL_DATASPACE_STANDARD_UNSPECIFIED;
            break;
        default:
            space |= HAL_DATASPACE_STANDARD_UNSPECIFIED;
            break;
    }

    switch (pic->video_full_range_flag)
    {
        case VIDEO_FULL_RANGE_LIMITED:
            space |= HAL_DATASPACE_RANGE_LIMITED;
            break;
        case VIDEO_FULL_RANGE_FULL:
            space |= HAL_DATASPACE_RANGE_FULL;
            break;
        default:
        {
            loge("should not be here");
            abort();
        }
    }

    return (android_dataspace_t)space;
}
#endif

#if defined(CONF_PRODUCT_STB)
static void setDisplay3DMode(LayerCtrlContext* lc)
{
    if (lc->bVideoWithTwoStreamFlag == 0)
        return;
    if (lc->b3DDisplayFlag == 1)
        return;
    if (lc->nVideoHeight == 0) // set 3d display after height got
        return;

    if(lc->bVideoWithTwoStreamFlag == 1)
        property_set("mediasw.doublestream", "1");
    else
        property_set("mediasw.doublestream", "0");

#if ((CONF_ANDROID_MAJOR_VER == 4)&&(CONF_ANDROID_SUB_VER >= 4))
    //sp<ISurfaceComposer> composer(ComposerService::getComposerService());
    logd("abao lc->nDisplayHeight=%d, nHeight=%d", lc->nDisplayHeight, lc->nHeight);
    String16 name("SurfaceFlinger");
    sp<IServiceManager> sm = defaultServiceManager();
    sp<ISurfaceComposer> composer = interface_cast<ISurfaceComposer>(sm->checkService(name));
    if(composer->getDisplayParameter(0, DISPLAY_CMD_GETSUPPORT3DMODE, 0, 0))
    {
        composer->setDisplayParameter(0, DISPLAY_CMD_SET3DMODE,
                    DISPLAY_3D_TOP_BOTTOM_HDMI, lc->nVideoHeight*2, 0);
    }
#elif (CONF_ANDROID_MAJOR_VER == 7)
    displaydClient client;
    if(client.getSupport3DMode(1) == 1)
    {
        client.set3DLayerMode(1, DISPLAY_3D_TOP_BOTTOM_HDMI);
    }
#endif
    lc->b3DDisplayFlag == 1;
}
#endif

static void __LayerRelease(LayerCtrl* l)
{
    logv("__LayerRelease");
    LayerCtrlContext* lc;
    VPictureNode*     nodePtr;
    int i;
    int ret;
    VideoPicture mPicBufInfo;
    ANativeWindowBuffer* pWindowBuf;

    lc = (LayerCtrlContext*)l;

    memset(&mPicBufInfo, 0, sizeof(VideoPicture));

    logd("LayerRelease, ionFd = %d",lc->ionFd);

#if defined(CONF_SEND_BLACK_FRAME_TO_GPU)
    if (GetConfigParamterInt("black_pic_4_SP", 0) == 1)
    {
        if(lc->bProtectFlag == 0)
        {
            if(lc->bHoldLastPictureFlag == false && lc->pNativeWindow != NULL)
            {
                sendThreeBlackFrameToGpu(lc);
                makeSureBlackFrameToShow(lc);
            }
        }
    }
#endif

    //for funshion apk,  when reopen the apk to continue playing
    if(lc->pNativeWindow != NULL)
        native_window_api_connect(lc->pNativeWindow, NATIVE_WINDOW_API_MEDIA);

    for(i = 0; i < GPU_BUFFER_NUM; i++)
    {
        pWindowBuf = lc->mGpuBufferInfo[i].pWindowBuf;
        if(lc->mGpuBufferInfo[i].mStatus != UNRECOGNIZED)
        {
            //* unlock the buffer.
            GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
            graphicMapper.unlock(pWindowBuf->handle);
        }

        //* we should queue buffer which had dequeued to gpu
        if(lc->mGpuBufferInfo[i].mStatus == OWN_BY_DECODER
            || lc->mGpuBufferInfo[i].mStatus == OWN_BY_US)
        {
            lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
            lc->mGpuBufferInfo[i].mStatus = OWN_BY_NATIVE_WINDOW;
        }

        if(lc->mGpuBufferInfo[i].nBufFd >= 0)
        {

            logd("close: ion_buf_fd[%d] = %d", i, lc->mGpuBufferInfo[i].nBufFd);
            close(lc->mGpuBufferInfo[i].nBufFd);
        }

        if(lc->mGpuBufferInfo[i].handle_ion != ION_NULL_VALUE)
        {
            lc->nUnFreeBufferCount--;
            logd("ion_free: handle_ion[%d] = %p, nUnFreeBufferCount: %d",
                i, lc->mGpuBufferInfo[i].handle_ion, lc->nUnFreeBufferCount);
            ion_free(lc->ionFd,lc->mGpuBufferInfo[i].handle_ion);
        }

        if(lc->mGpuBufferInfo[i].pMetaDataVirAddr != NULL)
        {
            munmap(lc->mGpuBufferInfo[i].pMetaDataVirAddr,lc->mGpuBufferInfo[i].nMetaDataVirAddrSize);
        }
        if(lc->mGpuBufferInfo[i].nMetaDataMapFd >= 0)
        {
            close(lc->mGpuBufferInfo[i].nMetaDataMapFd);
        }
        if(lc->mGpuBufferInfo[i].metadata_handle_ion != ION_NULL_VALUE)
        {
            ion_free(lc->ionFd,lc->mGpuBufferInfo[i].metadata_handle_ion);
        }

        lc->mGpuBufferInfo[i].pWindowBuf = NULL;
        lc->mGpuBufferInfo[i].nUsedFlag = 0;
        lc->mGpuBufferInfo[i].mStatus = UNRECOGNIZED;
    }

    if(lc->bProtectFlag == 1 /*|| lc->bHoldLastPictureFlag == 0*/)
    {
        ret = pushBlankBuffersToNativeWindow(lc);
        if(ret != 0)
        {
            loge("pushBlankBuffersToNativeWindow appear error!: ret = %d",ret);
        }
    }

    if(lc->nUnFreeBufferCount > 0)
    {
        logw("========= memory leak : %d", lc->nUnFreeBufferCount);
    }

#if defined(CONF_PRODUCT_STB)
#if ((CONF_ANDROID_MAJOR_VER == 4)&&(CONF_ANDROID_SUB_VER >= 4))
    if(lc->bVideoWithTwoStreamFlag == 1)
    {
        //sp<ISurfaceComposer> composer(ComposerService::getComposerService());
        String16 name("SurfaceFlinger");
        sp<IServiceManager> sm = defaultServiceManager();
        sp<ISurfaceComposer> composer = interface_cast<ISurfaceComposer>(sm->checkService(name));
        if(composer->getDisplayParameter(0, DISPLAY_CMD_GETSUPPORT3DMODE, 0, 0))
        {
            composer->setDisplayParameter(0, DISPLAY_CMD_SET3DMODE, DISPLAY_2D_ORIGINAL, 0, 0);
        }
    }
#elif (CONF_ANDROID_MAJOR_VER == 7)
    if(lc->bVideoWithTwoStreamFlag == 1)
    {
        displaydClient client;
        if(client.getSupport3DMode(1) == 1)
            client.set3DLayerMode(1, DISPLAY_2D_ORIGINAL);
    }
#endif
#endif
}

static void __LayerDestroy(LayerCtrl* l)
{
    logd("__LayerDestroy");
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    if(lc->nUnFreeBufferCount > 0)
    {
        logw("========= memory leak : %d", lc->nUnFreeBufferCount);
    }

    if(lc->ionFd >= 0)
    {
        ion_close(lc->ionFd);
    }

#if (defined(CONF_PTS_TOSF))
    CdxVideoSchedulerDestroy(lc->mVideoScheduler);
#endif

    free(lc);
}

//* Description: set initial param -- size of display
static int __LayerSetDisplayBufferSize(LayerCtrl* l, int nWidth, int nHeight)
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

    if(lc->bVideoWithTwoStreamFlag == 1)
    {
        //* display the whole buffer region when 3D
        //* as we had init align-edge-region to black. so it will be look ok.
        int nScaler = 2;
        lc->nDisplayHeight = lc->nDisplayHeight*nScaler;
    }

    return 0;
}

//* Description: set initial param -- buffer count
static int __LayerSetDisplayBufferCount(LayerCtrl* l, int nBufferCount)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerSetBufferCount: count = %d",nBufferCount);

    lc->nGpuBufferCount = nBufferCount;

    if(lc->nGpuBufferCount > GPU_BUFFER_NUM)
        lc->nGpuBufferCount = GPU_BUFFER_NUM;

    return lc->nGpuBufferCount;
}

//* Description: set display region -- can set when video is playing
static int __LayerSetDisplayRegion(LayerCtrl* l, int nLeftOff, int nTopOff,
                               int nDisplayWidth, int nDisplayHeight)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer set display region, leftOffset = %d, topOffset = %d, displayWidth = %d, \
          displayHeight = %d",
        nLeftOff, nTopOff, nDisplayWidth, nDisplayHeight);

    lc->nVideoWidth = nDisplayWidth;
    lc->nVideoHeight = nDisplayHeight;

#if defined(CONF_PRODUCT_STB)
    setDisplay3DMode(lc);
#endif

    if(nDisplayWidth != 0 && nDisplayHeight != 0)
    {
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
        if(lc->bLayerInitialized == 1)
        {
            Rect         crop;
            crop.left   = lc->nLeftOff;
            crop.top    = lc->nTopOff;
            crop.right  = lc->nLeftOff + lc->nDisplayWidth;
            crop.bottom = lc->nTopOff + lc->nDisplayHeight;
            lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_CROP, &crop);
        }
        return 0;
    }
    else
        return -1;
}

//* Description: set initial param -- displayer pixel format
static int __LayerSetDisplayPixelFormat(LayerCtrl* l, enum EPIXELFORMAT ePixelFormat)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer set expected pixel format, format = %d", (int)ePixelFormat);
    //* add new pixel formats supported by gpu here.
    if(ePixelFormat == PIXEL_FORMAT_NV12 ||
       ePixelFormat == PIXEL_FORMAT_NV21 ||
       ePixelFormat == PIXEL_FORMAT_YV12)
    {
        lc->eDisplayPixelFormat = ePixelFormat;
    }
#if (defined(CONF_10BIT_ENABLE))
    else if(ePixelFormat == PIXEL_FORMAT_P010_UV ||
       ePixelFormat == PIXEL_FORMAT_P010_VU)
    {
        lc->eDisplayPixelFormat = ePixelFormat;
    }
#endif
    else
    {
        logv("receive pixel format is %d, not match.", lc->eDisplayPixelFormat);
        return -1;
    }

/*
#if defined(CONF_GPU_IMG)
    //* on A83-pad and A83-box , the address should 4k align when format is NV21.
    //* it is a GPU bug, we had make sure that it is no need 4K align for IMG
    if((lc->eDisplayPixelFormat == PIXEL_FORMAT_NV21) ||
        (lc->eDisplayPixelFormat == PIXEL_FORMAT_NV12))
        lc->b4KAlignFlag = 1;
#endif
*/
    return 0;
}

//* Description: set initial param -- video whether have hdr info or not
static int LayerSetHdrInfo(LayerCtrl *l, const FbmBufInfo *fbmInfo)
{
    if (!fbmInfo)
    {
        loge("fbmInfo is null");
        return -1;
    }

    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    lc->bHdrVideoFlag = fbmInfo->bHdrVideoFlag;
    lc->b10BitVideoFlag = fbmInfo->b10bitVideoFlag;
    lc->bAfbcModeFlag = fbmInfo->bAfbcModeFlag;

    return 0;
}

//* Description: set initial param -- video whether have two stream or not
static int __LayerSetVideoWithTwoStreamFlag(LayerCtrl* l, int bVideoWithTwoStreamFlag)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerSetIsTwoVideoStreamFlag, flag = %d",bVideoWithTwoStreamFlag);

    lc->bVideoWithTwoStreamFlag = bVideoWithTwoStreamFlag;

    return 0;
}

//* Description: set initial param -- whether picture-bitstream decoded by software decoder
static int __LayerSetIsSoftDecoderFlag(LayerCtrl* l, int bIsSoftDecoderFlag)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerSetIsSoftDecoderFlag, flag = %d",bIsSoftDecoderFlag);

    lc->bIsSoftDecoderFlag = bIsSoftDecoderFlag;

    return 0;
}

//* Description: set buffer timestamp -- set this param every frame
static int __LayerSetBufferTimeStamp(LayerCtrl* l, int64_t nPtsAbs)
{
    LayerCtrlContext* lc;
    int64_t renderTime;

    lc = (LayerCtrlContext*)l;

#if (defined(CONF_PTS_TOSF))
    static int nVsync = -1;
    renderTime = CdxVideoSchedulerSchedule(lc->mVideoScheduler, nPtsAbs);

#if 0 //for debug
    static cdx_int64 prePts = -1;
    static cdx_int64 prePicPts = -1;
    if (prePts > 0)
    {
        logd("====[diff=%lld us, preRenderTime=%lld us, curRenderTime=%lld us],"
            "[diff=%lld, prePicPts=%lld us, nPtsAbs=%lld us]",
                  (renderTime - prePts)/1000, prePts/1000, renderTime/1000,
                  (nPtsAbs - prePicPts)/1000, prePicPts/1000, nPtsAbs/1000);
    }
    prePts = renderTime;
    prePicPts = nPtsAbs;
#endif

    //attention! 4 vsync period is set after measured in h5(video before audio),
    //different platform may has different value.
    if (nVsync < 0)
        nVsync = GetConfigParamterInt("compensate_vsync", 4);
    renderTime += nVsync * CdxVideoSchedulerGetVsyncPeriod(lc->mVideoScheduler);
#else
    renderTime = nPtsAbs;
#endif

    native_window_set_buffers_timestamp(lc->pNativeWindow, renderTime);

    return 0;
}

//* Description: reset nativewindow -- need release old resource and init newer
static void __LayerResetNativeWindow(LayerCtrl* l,void* pNativeWindow)
{
    logd("LayerResetNativeWindow : %p ",pNativeWindow);

    LayerCtrlContext* lc;
    VideoPicture mPicBufInfo;

    lc = (LayerCtrlContext*)l;
    ANativeWindowBuffer* pWindowBuf;

    memset(&mPicBufInfo, 0, sizeof(VideoPicture));

    //* we should queue buffer which had dequeued to gpu
    int i;
    for(i = 0; i < GPU_BUFFER_NUM; i++)
    {
        pWindowBuf = lc->mGpuBufferInfo[i].pWindowBuf;
        if(lc->mGpuBufferInfo[i].mStatus != UNRECOGNIZED)
        {
            //* unlock the buffer.
            GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
            graphicMapper.unlock(pWindowBuf->handle);
        }

        if(lc->mGpuBufferInfo[i].mStatus == OWN_BY_DECODER
            || lc->mGpuBufferInfo[i].mStatus == OWN_BY_US)
        {
            lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
            lc->mGpuBufferInfo[i].mStatus = UNRECOGNIZED;
        }
    }

    //memset(&lc->mGpuBufferInfo,0,sizeof(GpuBufferInfoT)*GPU_BUFFER_NUM);

    lc->pNativeWindow = (ANativeWindow*)lc->base.pNativeWindow   ;//(ANativeWindow*)pNativeWindow;

    if(lc->pNativeWindow != NULL)
        lc->bLayerInitialized = 0;

    return ;
}

static int PrintBufferStatus(LayerCtrlContext* lc)
{
    int i;
    logd("========== print start =============");
    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        logd("buffer[%d], status: %d, nBufFd: %d, phyAddr: %x, pWindowBuf: %p",
            i, lc->mGpuBufferInfo[i].mStatus, lc->mGpuBufferInfo[i].nBufFd,
            lc->mGpuBufferInfo[i].pPicture.phyYBufAddr, lc->mGpuBufferInfo[i].pWindowBuf);
    }
    logd("====== print end ===========");
    return 0;
}

//* Description: get the buffer which owned by nativewindow
//* use in setWindow
static VideoPicture* __LayerGetBufferOwnedByGpu(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    lc = (LayerCtrlContext*)l;

    for(int i = 0; i< lc->nGpuBufferCount; i++)
    {
        if(lc->mGpuBufferInfo[i].mStatus == OWN_BY_NATIVE_WINDOW)
        {
            // in reset native window, all of the buffers should be cancel to nativeWindow,
            // so we set the status of buffers to UNRECOGNIZED, which already in native window;
            // the other buffers in decoder will be set UNRECOGNIZED in LayerResetNativeWindow
            lc->mGpuBufferInfo[i].mStatus = UNRECOGNIZED;
            return &lc->mGpuBufferInfo[i].pPicture;
        }
    }
    return NULL;
}

//* Description: get FPS(frames per second) of GPU
//*  Limitation: private implement for tvbox-platform
static int __LayerGetDisplayFPS(LayerCtrl* l)
{
    enum {
        DISPLAY_CMD_GETDISPFPS = 0x29,
    };

    int dispFPS = 0;

    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

#if defined(CONF_PRODUCT_STB)
    if(lc->pNativeWindow != NULL)
        dispFPS = lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_GETPARAMETER,
                                             DISPLAY_CMD_GETDISPFPS);
#endif
    if (dispFPS <= 0) /* DISPLAY_CMD_GETDISPFPS not support, assume a nice fps */
        dispFPS = 60;

    return dispFPS;
}

static int __LayerGetBufferNumHoldByGpu(LayerCtrl* l)
{
    CDX_UNUSE(l);
    return GetConfigParamterInt("pic_4list_num", 3);
}

//* Description: make the video to show -- control the status of display
static int __LayerCtrlShowVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;

    logv("LayerCtrlShowVideo, current show flag = %d", lc->bLayerShowed);

#if defined(CONF_PRODUCT_STB)
    if(lc->bLayerShowed == 0)
    {
        if(lc->pNativeWindow != NULL)
        {
            lc->bLayerShowed = 1;
            lc->pNativeWindow->perform(lc->pNativeWindow,
                                       NATIVE_WINDOW_SETPARAMETER,
                                       HWC_LAYER_SHOW,
                                       1);
        }
        else
        {
            logw("the nativeWindow is null when call LayerCtrlShowVideo()");
            return -1;
        }
    }
#endif
    return 0;
}

//* Description: make the video to hide -- control the status of display
static int __LayerCtrlHideVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;

    logv("LayerCtrlHideVideo, current show flag = %d", lc->bLayerShowed);
#if defined(CONF_PRODUCT_STB)
    if(lc->bLayerShowed == 1)
    {
        if(lc->pNativeWindow != NULL)
        {
        lc->bLayerShowed = 0;
        lc->pNativeWindow->perform(lc->pNativeWindow,
                                       NATIVE_WINDOW_SETPARAMETER,
                                       HWC_LAYER_SHOW,
                                       0);
        }
        else
        {
            logw("the nativeWindow is null when call LayerCtrlHideVideo()");
            return -1;
        }
    }
#endif

    return 0;
}

//* Description: query whether the video is showing -- query the status of display
static int __LayerCtrlIsVideoShow(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerCtrlIsVideoShow : bLayerShowed = %d",lc->bLayerShowed);

    return lc->bLayerShowed;
}

//* Description: make the last pic to show always when quit playback
static int __LayerCtrlHoldLastPicture(LayerCtrl* l, int bHold)
{
    logv("LayerCtrlHoldLastPicture, bHold = %d", bHold);
    //*TODO
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    lc->bHoldLastPictureFlag = bHold;

    return 0;
}

//* Description: the picture buf is secure
static int __LayerSetSecure(LayerCtrl* l, int bSecure)
{
    logv("__LayerSetSecure, bSecure = %d", bSecure);
    //*TODO
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    lc->bProtectFlag = bSecure;

    return 0;
}


//* Description: dequeue buffer from nativewindow
static int __LayerDequeueBuffer(LayerCtrl* l,VideoPicture** ppVideoPicture, int bInitFlag)
{
    logv("__LayerDequeueBuffer, *ppVideoPicture(%p),bInitFlag(%d)",
        *ppVideoPicture,bInitFlag);

    LayerCtrlContext* lc = (LayerCtrlContext*)l;
    ANativeWindowBuffer* pWindowBuf = NULL;
    void*   pDataBuf    = NULL;
    int     err = -1;
    int     i   = 0;

    if(lc->pNativeWindow == NULL)
    {
        logw("pNativeWindow is null when dequeue buffer");
        return -1;
    }
    if(lc->bLayerInitialized == 0)
    {
        if(setLayerParam(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }

#if defined(CONF_PRODUCT_STB)
        setDisplay3DMode(lc);
#endif

#if (CONF_ANDROID_MAJOR_VER >= 8)
        if(lc->nWidth * lc->nHeight > 8000000 && lc->nSwitchNum==0)
        {
            switchBuffer(lc);
            lc->bSwitchBuffer = 1;
            lc->nSwitchNum = 0;
        }
#endif

        lc->bLayerInitialized = 1;
    }
    //* we should make sure that the dequeue buffer is new when init
    if(bInitFlag == 1)
    {
        *ppVideoPicture = dequeueTheInitGpuBuffer(lc);
        if(*ppVideoPicture == NULL)
        {
            loge("*** dequeueTheInitGpuBuffer failed");
            return -1;
        }
        //PrintBufferStatus(lc);
    }
    else
    {
        //* dequeue a buffer from the nativeWindow object.
        err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
        if(err != 0)
        {
            logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.",
                  err);
            return -1;
        }

        for(i = 0; i < lc->nGpuBufferCount; i++)
        {
            if(lc->mGpuBufferInfo[i].pWindowBuf == pWindowBuf)
            {
                if(lc->mGpuBufferInfo[i].mStatus == OWN_BY_DECODER)
                {
                    logw("=== status error, i: %d", i);
                }
                lc->mGpuBufferInfo[i].mStatus = OWN_BY_US;
                break;
            }
        }

        for(i = 0; i < lc->nGpuBufferCount; i++)
        {
            if(lc->mGpuBufferInfo[i].mStatus == OWN_BY_US)
            {
                lc->mGpuBufferInfo[i].mStatus = OWN_BY_DECODER;
                *ppVideoPicture = &lc->mGpuBufferInfo[i].pPicture;
                break;
            }
        }
    }

    //PrintBufferStatus(lc);
    return 0;
}

//* Description: queue buffer to nativewindow
static int __LayerQueueBuffer(LayerCtrl* l,VideoPicture* pInPicture, int bValid)
{
    logv("LayerQueueBuffer: pInPicture = %p, pData0 = %p",
            pInPicture,pInPicture->pData0);

    ANativeWindowBuffer* pWindowBuf = NULL;
    int               i      = 0;
    LayerCtrlContext*    lc  = (LayerCtrlContext*)l;

    if(lc->bLayerInitialized == 0)
    {
        if(setLayerParam(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }

#if defined(CONF_PRODUCT_STB)
        setDisplay3DMode(lc);
#endif

        lc->bLayerInitialized = 1;
    }

    if(lc->bSwitchBuffer)
    {
        for(i = 0; i < lc->nGpuBufferCount; i++)
        {
            if(lc->mGpuBufferInfo[i].pPicture.phyYBufAddr == pInPicture->phyYBufAddr)
            {
                if(i != lc->nGpuBufferCount-3)
                {
                    logw("=== buffer error, i: %d", i);
                }
                lc->mGpuBufferInfo[i].mStatus = OWN_BY_NATIVE_WINDOW;
                break;
            }
        }

        lc->bSwitchBuffer = 0;
        lc->nSwitchNum = 0;

        //PrintBufferStatus(lc);
        return 0;
    }

    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        if(lc->mGpuBufferInfo[i].pPicture.phyYBufAddr == pInPicture->phyYBufAddr)
        {
            if(lc->mGpuBufferInfo[i].mStatus != OWN_BY_DECODER)
            {
                logd("=== status error %d, buf id: %d", lc->mGpuBufferInfo[i].mStatus, i);
                //PrintBufferStatus(lc);
                return -1;
            }

            break;
        }
    }
    //loge("*** LayerQueueBuffer pInPicture = %p, bValid = %d",pInPicture,bValid);
    pWindowBuf = lc->mGpuBufferInfo[i].pWindowBuf;

    if(bValid == 1)
    {
#if (defined(CONF_HIGH_DYNAMIC_RANGE_ENABLE))
        android_dataspace_t space = getDataspaceFromVideoPicture(pInPicture);
        native_window_set_buffers_data_space(lc->pNativeWindow, space);
#endif
        lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
    }
    else
    {
        lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
    }

    lc->mGpuBufferInfo[i].mStatus = OWN_BY_NATIVE_WINDOW;

    logv("******LayerQueueBuffer finish!");
    return 0;
}

//* Description: release the buffer by ion
static int __LayerReleaseBuffer(LayerCtrl* l,VideoPicture* pPicture)
{
    logv("***LayerReleaseBuffer");
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    ion_handle_abstract_t handle_ion = ION_NULL_VALUE;
    handle_ion = (ion_handle_abstract_t)(uintptr_t)pPicture->pPrivate;

    int i;
    for(i = 0; i < GPU_BUFFER_NUM; i++)
    {
        if((size_addr)lc->mGpuBufferInfo[i].pBufPhyAddr == pPicture->phyYBufAddr)
        {
            if(pPicture->nBufFd >= 0)
                close(pPicture->nBufFd);

            if(handle_ion != ION_NULL_VALUE)
                ion_free(lc->ionFd,handle_ion);

            if(lc->mGpuBufferInfo[i].pMetaDataVirAddr != NULL)
            {
                munmap(lc->mGpuBufferInfo[i].pMetaDataVirAddr,lc->mGpuBufferInfo[i].nMetaDataVirAddrSize);
            }
            if(lc->mGpuBufferInfo[i].nMetaDataMapFd >= 0)
            {
                close(lc->mGpuBufferInfo[i].nMetaDataMapFd);
            }
            if(lc->mGpuBufferInfo[i].metadata_handle_ion != ION_NULL_VALUE)
            {
                ion_free(lc->ionFd,lc->mGpuBufferInfo[i].metadata_handle_ion);
            }

            lc->mGpuBufferInfo[i].nBufFd = -1;
            lc->mGpuBufferInfo[i].nMetaDataMapFd = -1;
            lc->mGpuBufferInfo[i].handle_ion = ION_NULL_VALUE;
            lc->mGpuBufferInfo[i].metadata_handle_ion = ION_NULL_VALUE;
            lc->mGpuBufferInfo[i].pWindowBuf = NULL;
            lc->mGpuBufferInfo[i].mStatus = UNRECOGNIZED;
            lc->nUnFreeBufferCount--;
            logd("release this buffer(fd: %d), nUnFreeBufferCount: %d", pPicture->nBufFd, lc->nUnFreeBufferCount);
            break;
        }
    }

    if(i == GPU_BUFFER_NUM)
    {
        logw("warning: the buffer (fd:%d) cannot found in list", pPicture->nBufFd);
    }

    return 0;
}

static int __LayerReset(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int i;
    lc = (LayerCtrlContext*)l;
    ANativeWindowBuffer* pWindowBuf;

    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        pWindowBuf = lc->mGpuBufferInfo[i].pWindowBuf;
        if(lc->mGpuBufferInfo[i].mStatus != UNRECOGNIZED)
        {
            //* unlock the buffer.
            GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
            graphicMapper.unlock(pWindowBuf->handle);
        }

        //* we should queue buffer which had dequeued to gpu
        if(lc->mGpuBufferInfo[i].mStatus == OWN_BY_DECODER
            || lc->mGpuBufferInfo[i].mStatus == OWN_BY_US)
        {
            lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
        }

        if(lc->mGpuBufferInfo[i].pMetaDataVirAddr != NULL)
        {
            munmap(lc->mGpuBufferInfo[i].pMetaDataVirAddr,lc->mGpuBufferInfo[i].nMetaDataVirAddrSize);
        }
        if(lc->mGpuBufferInfo[i].nMetaDataMapFd >= 0)
        {
            close(lc->mGpuBufferInfo[i].nMetaDataMapFd);
        }
        if(lc->mGpuBufferInfo[i].metadata_handle_ion != ION_NULL_VALUE)
        {
            ion_free(lc->ionFd,lc->mGpuBufferInfo[i].metadata_handle_ion);
        }
        if(lc->mGpuBufferInfo[i].nBufFd >= 0)
        {
            lc->nUnFreeBufferCount--;
            logd("close: ion_buf_fd[%d] = %d",i,lc->mGpuBufferInfo[i].nBufFd);
            close(lc->mGpuBufferInfo[i].nBufFd);
        }
        if(lc->mGpuBufferInfo[i].handle_ion != ION_NULL_VALUE)
        {
            logd("ion_free: handle_ion[%d] = %p",i,lc->mGpuBufferInfo[i].handle_ion);
            ion_free(lc->ionFd,lc->mGpuBufferInfo[i].handle_ion);
        }

        lc->mGpuBufferInfo[i].pWindowBuf = NULL;
        lc->mGpuBufferInfo[i].nUsedFlag = 0;
        lc->mGpuBufferInfo[i].mStatus = UNRECOGNIZED;
    }

    return 0;
}

static int __LayerControl(LayerCtrl* l, int cmd, void *para)
{
    LayerCtrlContext *lc = (LayerCtrlContext*)l;

    CDX_UNUSE(para);

    switch(cmd)
    {
#if (defined(CONF_PTS_TOSF))
        case CDX_LAYER_CMD_RESTART_SCHEDULER:
        {
            CdxVideoSchedulerRestart(lc->mVideoScheduler);
            break;
        }
#endif
        case CDX_LAYER_CMD_SET_HDR_INFO:
        {
            LayerSetHdrInfo(l, (const FbmBufInfo *)para);
            break;
        }

        case CDX_LAYER_CMD_SET_NATIVE_WINDOW:
        {
            lc->base.pNativeWindow = (void*)para;
            break;
        }
        case CDX_LAYER_CMD_SET_VIDEOCODEC_FORMAT:
        {
            lc->eVideoCodecFormat = *((enum EVIDEOCODECFORMAT *)para);
            break;
        }
        default:
            break;
    }

    return 0;
}


static LayerControlOpsT mLayerControlOps =
{
    .release                   =   __LayerRelease                   ,
    .setSecureFlag             =   __LayerSetSecure                 ,
    .setDisplayBufferSize      =   __LayerSetDisplayBufferSize      ,
    .setDisplayBufferCount     =   __LayerSetDisplayBufferCount     ,
    .setDisplayRegion          =   __LayerSetDisplayRegion          ,
    .setDisplayPixelFormat     =   __LayerSetDisplayPixelFormat     ,
    .setVideoWithTwoStreamFlag =   __LayerSetVideoWithTwoStreamFlag ,
    .setIsSoftDecoderFlag      =   __LayerSetIsSoftDecoderFlag      ,
    .setBufferTimeStamp        =   __LayerSetBufferTimeStamp        ,
    .resetNativeWindow         =   __LayerResetNativeWindow         ,
    .getBufferOwnedByGpu       =   __LayerGetBufferOwnedByGpu       ,
    .getDisplayFPS             =   __LayerGetDisplayFPS             ,
    .getBufferNumHoldByGpu     =   __LayerGetBufferNumHoldByGpu     ,
    .ctrlShowVideo             =   __LayerCtrlShowVideo             ,
    .ctrlHideVideo             =   __LayerCtrlHideVideo             ,
    .ctrlIsVideoShow           =   __LayerCtrlIsVideoShow           ,
    .ctrlHoldLastPicture       =   __LayerCtrlHoldLastPicture       ,

    .dequeueBuffer             =   __LayerDequeueBuffer             ,
    .queueBuffer               =   __LayerQueueBuffer               ,
    .releaseBuffer             =   __LayerReleaseBuffer             ,
    .reset                     =   __LayerReset                     ,
    .destroy                   =   __LayerDestroy                   ,
    .control                   =   __LayerControl
};

LayerCtrl* LayerCreate()
{
    logv("LayerInit.");

    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)malloc(sizeof(LayerCtrlContext));
    if(lc == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(lc, 0, sizeof(LayerCtrlContext));

    int i;
    for(i = 0; i < GPU_BUFFER_NUM; i++)
    {
        lc->mGpuBufferInfo[i].nBufFd = -1;
        lc->mGpuBufferInfo[i].nMetaDataMapFd = -1;
        lc->mGpuBufferInfo[i].handle_ion = ION_NULL_VALUE;
        lc->mGpuBufferInfo[i].metadata_handle_ion = ION_NULL_VALUE;
        lc->mGpuBufferInfo[i].pWindowBuf = NULL;
    }

    lc->ionFd = -1;
    lc->ionFd = ion_open();

    logd("ion open fd = %d",lc->ionFd);
    if(lc->ionFd < -1)
    {
        loge("ion open fail ! ");
        return NULL;
    }

    lc->base.ops = &mLayerControlOps;

#if (defined(CONF_PTS_TOSF))
    lc->mVideoScheduler = CdxVideoSchedulerCreate();
    if (lc->mVideoScheduler == NULL)
        logw("CdxVideoSchedulerCreate failed");
#endif

    return (LayerCtrl*)&lc->base;
}
