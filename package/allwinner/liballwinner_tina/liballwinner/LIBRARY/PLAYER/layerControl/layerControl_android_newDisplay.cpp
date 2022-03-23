
//#define CONFIG_LOG_LEVEL    OPTION_LOG_LEVEL_DETAIL
//#define LOG_TAG "layerControl_android_newDisplay"
#include "cdx_config.h"
#include "layerControl.h"
#include "log.h"
#include "memoryAdapter.h"

#if (CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_2)
#include <gui/ISurfaceTexture.h>
#elif (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_4_4)
#include <gui/Surface.h>
#else
    #error "invalid configuration of os version."
#endif
#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include <hardware/hwcomposer.h>

#if(CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_4_4)
#include <hardware/hal_public.h>
#endif
#include <linux/ion.h>
#include <ion/ion.h>

#if (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0 && GRALLOC_PRIV == 1)
#include <gralloc_priv.h>
#endif

#if ((LINUX_VERSION == LINUX_VERSION_3_10) || (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0))
    typedef ion_user_handle_t ion_handle_abstract_t;
    #define ION_NULL_VALUE (0)
#else
    typedef struct ion_handle* ion_handle_abstract_t;
    #define ION_NULL_VALUE (NULL)
#endif

#define GPU_BUFFER_NUM 32

/* +1 allows queue after SetGpuBufferToDecoder */
#define NUM_OF_PICTURES_KEEP_IN_NODE (NUM_OF_PICTURES_KEEP_IN_LIST+1)

typedef struct VPictureNode_t VPictureNode;
struct VPictureNode_t
{
    int                  bUsed;
    VideoPicture*        pPicture;
    ANativeWindowBuffer* pNodeWindowBuf;
};

typedef struct GpuBufferInfoS
{
    ANativeWindowBuffer* pWindowBuf;
    ion_handle_abstract_t handle_ion;
    char* pBufPhyAddr;
    char* pBufVirAddr;
    int   nUsedFlag;
    int   nDequeueFlag;
}GpuBufferInfoT;

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
    int                  bLayerInitialized;
    int                  bLayerShowed;
    int                  bProtectFlag;

    //* use when render derect to hardware layer.
    VPictureNode         picNodes[NUM_OF_PICTURES_KEEP_IN_NODE];

    GpuBufferInfoT       mGpuBufferInfo[GPU_BUFFER_NUM];
    int                  nGpuBufferCount;
    int                  ionFd;
    int                  b4KAlignFlag;
    int                  bHoldLastPictureFlag;
    int                  bVideoWithTwoStreamFlag;
    int                  bIsSoftDecoderFlag;
    unsigned int         nUsage;
}LayerCtrlContext;

#if(SEND_3_BLACK_FRAME_TO_GPU == 1)
static int sendThreeBlackFrameToGpu(LayerCtrlContext* lc)
{
    logd("sendThreeBlackFrameToGpu()");

    ANativeWindowBuffer* pWindowBuf;
    void*                pDataBuf;
    int                  i;
    int                  err;

    if (lc->pNativeWindow == NULL || lc->bLayerInitialized == 0)
    {
        logv("skip %s", __func__);
         return 0;
    }

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
			logd("graphicMapper %p", pDataBuf);
        }

        if (pDataBuf) {
            memset((char*)pDataBuf,0x10,(pWindowBuf->height * pWindowBuf->stride));
            memset((char*)pDataBuf + pWindowBuf->height * pWindowBuf->stride,0x80,(pWindowBuf->height * pWindowBuf->stride)/2);
        }

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

// dequeue all nativewindow buffer from BufferQueue, then cancle all.
// this can make sure all buffer queued by player to render by surfaceflinger.
static int makeSureBlackFrameToShow(LayerCtrlContext* lc)
{
	logd("makeSureBlackFrameToShow()");

    ANativeWindowBuffer* pWindowBuf[32];
    void*                pDataBuf;
    int                  i;
    int                  err;
	int					 bufCnt = lc->nGpuBufferCount;

    for(i = 0;i < bufCnt -1; i++)
    {
        err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf[i]);
        if(err != 0)
        {
            logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.", err);
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

//* this function just for 3D case.
//* just init 32-line buffer to black color.
//* (when the two stream display to 2D, the 32-line buffer will cause "Green Screen" if not init,
//*  as buffer have make 32-align)
//* if init the whole buffer, it would take too much time.
int initPartialGpuBuffer(char* pDataBuf, ANativeWindowBuffer* pWindowBuf, LayerCtrlContext* lc)
{
    logv("initGpuBuffer, stride = %d, height = %d, ",pWindowBuf->stride,pWindowBuf->height);

    if(lc->eDisplayPixelFormat == PIXEL_FORMAT_NV21)
    {
	//* Y1
	int nRealHeight = pWindowBuf->height/2;
	int nInitHeight = 32;
	int nSkipLen = pWindowBuf->stride*(nRealHeight - nInitHeight);
	int nCpyLenY = pWindowBuf->stride*nInitHeight;
	memset(pDataBuf+nSkipLen, 0x10, nCpyLenY);
	//* Y2
	nSkipLen += pWindowBuf->stride*nRealHeight;
	memset(pDataBuf+nSkipLen, 0x10, nCpyLenY);

	//*UV1
	nSkipLen += nCpyLenY;
	nSkipLen += (pWindowBuf->stride/2)*(nRealHeight/2 - nInitHeight/2);
	int nCpyLenUV = (pWindowBuf->stride/2)*(nInitHeight/2);
	memset(pDataBuf+nSkipLen, 0x80, nCpyLenUV);
	//*UV2
	nSkipLen += (pWindowBuf->stride/2)*(nRealHeight/2);
	memset(pDataBuf+nSkipLen, 0x80, nCpyLenUV);
    }
    else
    {
        loge("the pixelFormat is not support when initPartialGpuBuffer, pixelFormat = %d",
              lc->eDisplayPixelFormat);
        return -1;
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

    eErr = native_window_set_buffers_geometry(lc->pNativeWindow, 1, 1,
            HAL_PIXEL_FORMAT_RGBX_8888);
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

        sp<GraphicBuffer> mGraphicBuffer(new GraphicBuffer(pWindowBuffer, false));

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

//* set usage, scaling_mode, pixelFormat, buffers_geometry, buffers_count, crop
static int setLayerParam(LayerCtrlContext* lc)
{
    logd("setLayerParam: PixelFormat(%d), nW(%d), nH(%d), leftoff(%d), topoff(%d)",
          lc->eDisplayPixelFormat,lc->nWidth,
          lc->nHeight,lc->nLeftOff,lc->nTopOff);
    logd("setLayerParam: dispW(%d), dispH(%d), buffercount(%d), bProtectFlag(%d), bIsSoftDecoderFlag(%d)",
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
            lc->nUsage |= GRALLOC_USAGE_SW_WRITE_RARELY;
        }
    }

    //* add other usage
    lc->nUsage |= GRALLOC_USAGE_SW_READ_NEVER     |
                  GRALLOC_USAGE_HW_TEXTURE        |
                  GRALLOC_USAGE_EXTERNAL_DISP;

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

    if(lc->nGpuBufferCount <= 0)
    {
        loge("error: the lc->nGpuBufferCount[%d] is invalid!",lc->nGpuBufferCount);
        return -1;
    }

    native_window_set_usage(lc->pNativeWindow,lc->nUsage);
    native_window_set_scaling_mode(lc->pNativeWindow, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    native_window_set_buffers_geometry(lc->pNativeWindow, nGpuBufWidth, nGpuBufHeight, pixelFormat);
    native_window_set_buffer_count(lc->pNativeWindow,lc->nGpuBufferCount);
    lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_CROP, &crop);
    return 0;
}

ANativeWindowBuffer* dequeueTheInitGpuBuffer(LayerCtrlContext* lc)
{
    int     err = -1;
    int     i   = 0;
    int     nCancelNum      = 0;
    int     nNeedCancelFlag = 0;
    int     nCancelIndex[GPU_BUFFER_NUM] = {-1};
    ANativeWindowBuffer*      pWindowBuf = NULL;

dequeue_buffer:

    //* dequeue a buffer from the nativeWindow object.
    err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
    if(err != 0)
    {
        logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.", err);
        return NULL;
    }

    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        if(lc->mGpuBufferInfo[i].nUsedFlag == 1
           && lc->mGpuBufferInfo[i].pWindowBuf == pWindowBuf)
        {
            nNeedCancelFlag = 1;
            nCancelIndex[nCancelNum] = i;
            nCancelNum++;
            logv("the buffer have not return ,dequeue agagin! : %p",pWindowBuf);
            goto dequeue_buffer;
        }
    }

    if(nNeedCancelFlag == 1)
    {
        for(i = 0;i<nCancelNum;i++)
        {
            int nIndex = nCancelIndex[i];
            ANativeWindowBuffer* pTmpWindowBuf = lc->mGpuBufferInfo[nIndex].pWindowBuf;
            lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pTmpWindowBuf);
        }
        nCancelNum = 0;
        nNeedCancelFlag = 0;
    }
    return pWindowBuf;
}

uintptr_t getPhyAddrOfGpuBuffer(LayerCtrlContext* lc,
                                ANativeWindowBuffer* pWindowBuf,
                                ion_handle_abstract_t* pHandle_ion,
                                uintptr_t*  pPhyaddress)
{
    ion_handle_abstract_t handle_ion = ION_NULL_VALUE;
    uintptr_t  nPhyaddress = 0;

#if(GPU_TYPE_MALI == 1)
	private_handle_t* hnd = (private_handle_t *)(pWindowBuf->handle);
    if(hnd != NULL)
    {
        ion_import(lc->ionFd, hnd->share_fd, &handle_ion);
    }
    else
    {
        logd("the hnd is wrong : hnd = %p",hnd);
        return -1;
    }
#else
    IMG_native_handle_t* hnd = (IMG_native_handle_t*)(pWindowBuf->handle);
    if(hnd != NULL)
    {
        ion_import(lc->ionFd, hnd->fd[0], &handle_ion);
    }
    else
    {
        logd("the hnd is wrong : hnd = %p",hnd);
        return -1;
    }
#endif

    //* we should not get the phyaddr if it is software decoder
    if(lc->bIsSoftDecoderFlag == 0)
    {
        if(lc->ionFd > 0)
            nPhyaddress = ion_getphyadr(lc->ionFd, handle_ion);
        else
        {
            logd("the ion fd is wrong : fd = %d",lc->ionFd);
            return -1;
        }

        nPhyaddress -= VE_PHY_OFFSET;
    }
    *pPhyaddress = nPhyaddress;
    *pHandle_ion = handle_ion;
    return 0;
}

int matchWindowBufferAndPicture(LayerCtrlContext* lc,
                                ANativeWindowBuffer* pWindowBuf,
                                               char* pGpuVirBuf,
                                  VideoPicture** ppVideoPicture,
                                                  int bInitFlag)
{
    logv("matchWindowBufferAndPicture");
    int i   = 0;
    int ret = -1;
    uintptr_t  nPhyaddress = 0;
    ion_handle_abstract_t handle_ion = ION_NULL_VALUE;
    VideoPicture* pPicture = NULL;

    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        if(lc->mGpuBufferInfo[i].pWindowBuf == NULL)
        {
            ret = getPhyAddrOfGpuBuffer(lc, pWindowBuf, &handle_ion, &nPhyaddress);
            if(ret == -1)
            {
                loge("getPhyAddrOfGpuBuffer failed");
                return -1;
            }
            if(lc->bVideoWithTwoStreamFlag == 1)
            {
                initPartialGpuBuffer(pGpuVirBuf,pWindowBuf,lc);
            }

            lc->mGpuBufferInfo[i].pWindowBuf   = pWindowBuf;
            lc->mGpuBufferInfo[i].handle_ion   = handle_ion;
            lc->mGpuBufferInfo[i].pBufVirAddr  = pGpuVirBuf;
            lc->mGpuBufferInfo[i].pBufPhyAddr  = (char*)nPhyaddress;
            lc->mGpuBufferInfo[i].nUsedFlag    = 1;
            lc->mGpuBufferInfo[i].nDequeueFlag = 1;
            break;
        }
        else if(lc->mGpuBufferInfo[i].pWindowBuf == pWindowBuf)
        {
            lc->mGpuBufferInfo[i].nUsedFlag    = 1;
            lc->mGpuBufferInfo[i].nDequeueFlag = 1;
            break;
        }
    }

    if(i == lc->nGpuBufferCount)
    {
        loge("not enouth gpu buffer , should not run here");
        abort();
    }

    //* dequeue buffer for the first time, we should not dequeue from picNode
    if(bInitFlag == 1)
    {
	pPicture = *ppVideoPicture;
	//* set the buffer address
        pPicture->pData0       = lc->mGpuBufferInfo[i].pBufVirAddr;
        pPicture->pData1       = pPicture->pData0 + (pWindowBuf->height * pWindowBuf->stride);
        pPicture->pData2       = pPicture->pData1 + (pWindowBuf->height * pWindowBuf->stride)/4;
        pPicture->phyYBufAddr  = (uintptr_t)lc->mGpuBufferInfo[i].pBufPhyAddr;
        pPicture->phyCBufAddr  = pPicture->phyYBufAddr + (pWindowBuf->height * pWindowBuf->stride);
        pPicture->nBufId	   = i;
        pPicture->pPrivate     = (void*)(uintptr_t)lc->mGpuBufferInfo[i].handle_ion;
        pPicture->ePixelFormat = lc->eDisplayPixelFormat;
        pPicture->nWidth       = pWindowBuf->stride;
        pPicture->nHeight      = pWindowBuf->height;
        pPicture->nLineStride  = pWindowBuf->stride;

        if(lc->b4KAlignFlag == 1)
	{
	    uintptr_t tmpAddr = (uintptr_t)pPicture->pData1;
	    tmpAddr     = (tmpAddr + 4095) & ~4095;
	    pPicture->pData1      = (char *)tmpAddr;
	    pPicture->phyCBufAddr = (pPicture->phyCBufAddr + 4095) & ~4095;
	}
    }
    else
    {
	for(i = 0; i<NUM_OF_PICTURES_KEEP_IN_NODE; i++)
	{
		logv("** dequeue , i(%d), used(%d), pPicture(%p), pNodeWindowBuf(%p),pWindowBuf(%p)",
				i,lc->picNodes[i].bUsed,lc->picNodes[i].pPicture,
				lc->picNodes[i].pNodeWindowBuf, pWindowBuf);
		if(lc->picNodes[i].bUsed == 1
		   && lc->picNodes[i].pPicture != NULL
		   && lc->picNodes[i].pNodeWindowBuf == pWindowBuf)
		{
			pPicture = lc->picNodes[i].pPicture ;
			lc->picNodes[i].bUsed = 0;
			break;
		}
	}
	if(i == NUM_OF_PICTURES_KEEP_IN_NODE)
	{
		loge("hava no unused picture in the picNode, pDataBuf = %p",pGpuVirBuf);
		return -1;
	}
    }

    *ppVideoPicture = pPicture;
    return 0;
}

LayerCtrl* __NewLayerInit(void* pNativeWindow, int bProtectedFlag)
{
    logv("LayerInit, pNativeWindow = %p",pNativeWindow);

    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)malloc(sizeof(LayerCtrlContext));
    if(lc == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(lc, 0, sizeof(LayerCtrlContext));

    lc->ionFd = -1;
    lc->ionFd = ion_open();

    logd("ion open fd = %d",lc->ionFd);
    if(lc->ionFd < -1)
    {
        loge("ion open fail ! ");
        return NULL;
    }

    lc->bProtectFlag  = bProtectedFlag;
    lc->pNativeWindow = (ANativeWindow*)pNativeWindow;
    return (LayerCtrl*)lc;
}


void __NewLayerRelease(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    VPictureNode*     nodePtr;
    int i;
    int ret;
    VideoPicture mPicBufInfo;

    lc = (LayerCtrlContext*)l;

    memset(&mPicBufInfo, 0, sizeof(VideoPicture));

    logv("LayerRelease, ionFd = %d",lc->ionFd);

    #if(SEND_3_BLACK_FRAME_TO_GPU == 1)
    if(lc->bProtectFlag == 0)
	{
		if(lc->bHoldLastPictureFlag == false/* && lc->bFirstFrameIsShowed == 1*/)
		{
			sendThreeBlackFrameToGpu(lc);
            makeSureBlackFrameToShow(lc);
		}
    }
    #endif

    for(i = 0; i < lc->nGpuBufferCount; i++)
    {
        //* we should queue buffer which had dequeued to gpu
        if(lc->mGpuBufferInfo[i].nDequeueFlag == 1)
        {
            //* unlock the buffer.
			ANativeWindowBuffer* pWindowBuf = lc->mGpuBufferInfo[i].pWindowBuf;
	        GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
	        graphicMapper.unlock(pWindowBuf->handle);

	        lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
			lc->mGpuBufferInfo[i].nDequeueFlag = 0;
        }

        if(lc->mGpuBufferInfo[i].handle_ion != ION_NULL_VALUE)
        {
            logd("ion_free: handle_ion[%d] = %p",i,lc->mGpuBufferInfo[i].handle_ion);
            ion_free(lc->ionFd,lc->mGpuBufferInfo[i].handle_ion);
        }
    }

    if(lc->ionFd > 0)
    {
        ion_close(lc->ionFd);
    }

    if(lc->bProtectFlag == 1 /*|| lc->bHoldLastPictureFlag == 0*/)
    {
        ret = pushBlankBuffersToNativeWindow(lc);
        if(ret != 0)
        {
            loge("pushBlankBuffersToNativeWindow appear error!: ret = %d",ret);
        }
    }

    free(lc);
}

//* Description: set initial param -- size of display
int __NewLayerSetDisplayBufferSize(LayerCtrl* l, int nWidth, int nHeight)
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

//* Description: set initial param -- buffer count
int __NewLayerSetDisplayBufferCount(LayerCtrl* l, int nBufferCount)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerSetBufferCount: count = %d",nBufferCount);

//    lc->nGpuBufferCount = nBufferCount + NUM_OF_PICTURES_KEEP_IN_LIST;
    lc->nGpuBufferCount = nBufferCount;

    if(lc->nGpuBufferCount > GPU_BUFFER_NUM)
        lc->nGpuBufferCount = GPU_BUFFER_NUM;

	return lc->nGpuBufferCount;
}

//* Description: set display region -- can set when video is playing
int __NewLayerSetDisplayRegion(LayerCtrl* l, int nLeftOff, int nTopOff, int nDisplayWidth, int nDisplayHeight)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer set display region, leftOffset = %d, topOffset = %d, displayWidth = %d, displayHeight = %d",
        nLeftOff, nTopOff, nDisplayWidth, nDisplayHeight);
    int scaler = (lc->bVideoWithTwoStreamFlag == 1) ? 2 : 1;

    if(nDisplayWidth != 0 && nDisplayHeight != 0)
    {
        lc->nDisplayWidth     = nDisplayWidth;
        lc->nDisplayHeight    = nDisplayHeight;
        lc->nLeftOff          = nLeftOff;
        lc->nTopOff           = nTopOff;

        int displayHeight = (lc->bVideoWithTwoStreamFlag == 1) ? lc->nDisplayHeight : nDisplayHeight;

        if(lc->bLayerInitialized == 1)
        {
            Rect         crop;
            crop.left   = lc->nLeftOff;
            crop.top    = lc->nTopOff;
            crop.right  = lc->nLeftOff + lc->nDisplayWidth;
            crop.bottom = lc->nTopOff + displayHeight * scaler;
            lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_SET_CROP, &crop);
        }

        return 0;
    }
    else
        return -1;
}

//* Description: set initial param -- displayer pixel format
int __NewLayerSetDisplayPixelFormat(LayerCtrl* l, enum EPIXELFORMAT ePixelFormat)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

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

    //* on A83-pad and A83-box , the address should 4k align when format is NV21
#if(IMG_NV21_4K_ALIGN == 1)
    if(lc->eDisplayPixelFormat == PIXEL_FORMAT_NV21)
        lc->b4KAlignFlag = 1;
#endif

    return 0;
}

//* Description: set initial param -- video whether have two stream or not
int __NewLayerSetVideoWithTwoStreamFlag(LayerCtrl* l, int bVideoWithTwoStreamFlag)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerSetIsTwoVideoStreamFlag, flag = %d",bVideoWithTwoStreamFlag);

    lc->bVideoWithTwoStreamFlag = bVideoWithTwoStreamFlag;

    return 0;
}

//* Description: set initial param -- whether picture-bitstream decoded by software decoder
int __NewLayerSetIsSoftDecoderFlag(LayerCtrl* l, int bIsSoftDecoderFlag)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerSetIsSoftDecoderFlag, flag = %d",bIsSoftDecoderFlag);

    lc->bIsSoftDecoderFlag = bIsSoftDecoderFlag;

    return 0;
}

//* Description: set buffer timestamp -- set this param every frame
int __NewLayerSetBufferTimeStamp(LayerCtrl* l, int64_t nPtsAbs)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    native_window_set_buffers_timestamp(lc->pNativeWindow, nPtsAbs);

    return 0;
}

//* Description: reset nativewindow -- need release old resource and init newer
void __NewLayerResetNativeWindow(LayerCtrl* l,void* pNativeWindow)
{
    logd("LayerResetNativeWindow : %p ",pNativeWindow);

    LayerCtrlContext* lc;
    VideoPicture mPicBufInfo;

    lc = (LayerCtrlContext*)l;

    memset(&mPicBufInfo, 0, sizeof(VideoPicture));

    //* we should queue buffer which had dequeued to gpu
    int i;
    for(i = 0; i < GPU_BUFFER_NUM; i++)
    {
        if(lc->mGpuBufferInfo[i].nDequeueFlag == 1)
        {
		//* unlock the buffer.
			ANativeWindowBuffer* pWindowBuf = lc->mGpuBufferInfo[i].pWindowBuf;
	        GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
	        graphicMapper.unlock(pWindowBuf->handle);

	        lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
			lc->mGpuBufferInfo[i].nDequeueFlag = 0;
        }
    }

    memset(&lc->mGpuBufferInfo,0,sizeof(GpuBufferInfoT)*GPU_BUFFER_NUM);

    lc->pNativeWindow = (ANativeWindow*)pNativeWindow;

    if(lc->pNativeWindow != NULL)
        lc->bLayerInitialized = 0;

    return ;
}

//* Description: get the buffer which owned by nativewindow
VideoPicture* __NewLayerGetBufferOwnedByGpu(LayerCtrl* l)
{
	LayerCtrlContext* lc;
	VideoPicture* pPicture = NULL;
	lc = (LayerCtrlContext*)l;
	for(int i = 0; i<NUM_OF_PICTURES_KEEP_IN_NODE; i++)
	{
		if(lc->picNodes[i].bUsed == 1)
		{
			lc->picNodes[i].bUsed = 0;
			pPicture = lc->picNodes[i].pPicture;
			break;
		}
	}
	return pPicture;
}

//* Description: get FPS(frames per second) of GPU
//*  Limitation: private implement for tvbox-platform
int __NewLayerGetDisplayFPS(LayerCtrl* l)
{
	enum{
		DISPLAY_CMD_GETDISPFPS = 0x29,
	};

	int dispFPS = 0;

	LayerCtrlContext* lc;

	lc = (LayerCtrlContext*)l;

	if(lc->pNativeWindow != NULL)
		dispFPS = lc->pNativeWindow->perform(lc->pNativeWindow, NATIVE_WINDOW_GETPARAMETER, DISPLAY_CMD_GETDISPFPS);

	if (dispFPS <= 0) /* DISPLAY_CMD_GETDISPFPS not support, assume a nice fps */
		dispFPS = 60;

	return dispFPS;
}

int __NewLayerGetBufferNumHoldByGpu(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    return NUM_OF_PICTURES_KEEP_IN_LIST;
}

//* Description: make the video to show -- control the status of display
int __NewLayerCtrlShowVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;

    logv("LayerCtrlShowVideo, current show flag = %d", lc->bLayerShowed);

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
    return 0;
}

//* Description: make the video to hide -- control the status of display
int __NewLayerCtrlHideVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;

    logv("LayerCtrlHideVideo, current show flag = %d", lc->bLayerShowed);

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
    return 0;
}

//* Description: query whether the video is showing -- query the status of display
int __NewLayerCtrlIsVideoShow(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("LayerCtrlIsVideoShow : bLayerShowed = %d",lc->bLayerShowed);

    return lc->bLayerShowed;
}

//* Description: make the last pic to show always when quit playback
int __NewLayerCtrlHoldLastPicture(LayerCtrl* l, int bHold)
{
    logv("LayerCtrlHoldLastPicture, bHold = %d", bHold);
    //*TODO
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    lc->bHoldLastPictureFlag = bHold;

    return 0;
}

//* Description: dequeue buffer from nativewindow
int __NewLayerDequeueBuffer(LayerCtrl* l,VideoPicture** ppVideoPicture, int bInitFlag)
{
	logv("__NewLayerDequeueBuffer, *ppVideoPicture(%p),bInitFlag(%d)",
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
        lc->bLayerInitialized = 1;
    }
    //* we should make sure that the dequeue buffer is new when init
	if(bInitFlag == 1)
	{
		pWindowBuf = dequeueTheInitGpuBuffer(lc);
        if(pWindowBuf == NULL)
        {
            loge("*** dequeueTheInitGpuBuffer failed");
            return -1;
        }
	}
    else
    {
        //* dequeue a buffer from the nativeWindow object.
        err = lc->pNativeWindow->dequeueBuffer_DEPRECATED(lc->pNativeWindow, &pWindowBuf);
        if(err != 0)
        {
            logw("dequeue buffer fail, return value from dequeueBuffer_DEPRECATED() method is %d.", err);
            return -1;
        }
    }
    //* lock the data buffer.
    lc->pNativeWindow->lockBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
    {
        GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
        Rect bounds(lc->nWidth, lc->nHeight);
        graphicMapper.lock(pWindowBuf->handle,lc->nUsage, bounds, &pDataBuf);
    }

    err = matchWindowBufferAndPicture(lc, pWindowBuf, (char*)pDataBuf, ppVideoPicture, bInitFlag);
    if(err == -1)
    {
        loge("matchWindowBufferAndPicture failed");
        return -1;
    }
    return 0;
}

//* Description: queue buffer to nativewindow
int __NewLayerQueueBuffer(LayerCtrl* l,VideoPicture* pInPicture, int bValid)
{
	logv("LayerQueueBuffer: pInPicture = %p, pData0 = %p",
			pInPicture,pInPicture->pData0);
    ANativeWindowBuffer* pWindowBuf = NULL;
    int               i      = 0;
    char*             pBuf   = NULL;
    int               nBufId = -1;
    LayerCtrlContext*    lc  = (LayerCtrlContext*)l;

    if(lc->bLayerInitialized == 0)
    {
        if(setLayerParam(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }

        lc->bLayerInitialized = 1;

    }
	for(i = 0; i<NUM_OF_PICTURES_KEEP_IN_NODE; i++)
	{
		if(lc->picNodes[i].bUsed == 0)
		{
			lc->picNodes[i].bUsed = 1;
			lc->picNodes[i].pPicture = pInPicture;
			break;
		}
	}
	if(i == NUM_OF_PICTURES_KEEP_IN_NODE)
	{
		loge("*** picNode is full when queue buffer");
		return -1;
	}
	//loge("*** LayerQueueBuffer pInPicture = %p, bValid = %d",pInPicture,bValid);
    pBuf       = (char*)pInPicture->phyYBufAddr;
    nBufId     = pInPicture->nBufId;
    pWindowBuf = lc->mGpuBufferInfo[nBufId].pWindowBuf;
    lc->picNodes[i].pNodeWindowBuf = pWindowBuf;

    //* unlock the buffer.
    {
        GraphicBufferMapper& graphicMapper = GraphicBufferMapper::get();
        graphicMapper.unlock(pWindowBuf->handle);
    }

    if(bValid == 1)
        lc->pNativeWindow->queueBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);
    else
        lc->pNativeWindow->cancelBuffer_DEPRECATED(lc->pNativeWindow, pWindowBuf);

    lc->mGpuBufferInfo[nBufId].nDequeueFlag = 0;

    logv("******LayerQueueBuffer finish!");
    return 0;
}

//* Description: release the buffer by ion
int __NewLayerReleaseBuffer(LayerCtrl* l,VideoPicture* pPicture)
{
	logv("***LayerReleaseBuffer");
	LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
	ion_handle_abstract_t handle_ion = ION_NULL_VALUE;
	handle_ion = (ion_handle_abstract_t)(uintptr_t)pPicture->pPrivate;
	ion_free(lc->ionFd,handle_ion);

	return 0;
}

NewLayerControlOpsT mNewLayerControlOps =
{
    init:                       __NewLayerInit                      ,
    release:                    __NewLayerRelease                   ,

    setDisplayBufferSize:       __NewLayerSetDisplayBufferSize      ,
    setDisplayBufferCount:      __NewLayerSetDisplayBufferCount     ,
    setDisplayRegion:           __NewLayerSetDisplayRegion          ,
    setDisplayPixelFormat:      __NewLayerSetDisplayPixelFormat     ,
    setVideoWithTwoStreamFlag:  __NewLayerSetVideoWithTwoStreamFlag ,
    setIsSoftDecoderFlag:       __NewLayerSetIsSoftDecoderFlag      ,
    setBufferTimeStamp:         __NewLayerSetBufferTimeStamp        ,

    resetNativeWindow :         __NewLayerResetNativeWindow         ,

    getBufferOwnedByGpu:        __NewLayerGetBufferOwnedByGpu       ,
    getDisplayFPS:		        __NewLayerGetDisplayFPS             ,
    getBufferNumHoldByGpu:      __NewLayerGetBufferNumHoldByGpu     ,

    ctrlShowVideo :             __NewLayerCtrlShowVideo             ,
    ctrlHideVideo:              __NewLayerCtrlHideVideo             ,
    ctrlIsVideoShow:            __NewLayerCtrlIsVideoShow           ,
    ctrlHoldLastPicture :       __NewLayerCtrlHoldLastPicture       ,

    dequeueBuffer:              __NewLayerDequeueBuffer             ,
    queueBuffer:                __NewLayerQueueBuffer               ,
    releaseBuffer:              __NewLayerReleaseBuffer
};

NewLayerControlOpsT* __GetNewLayerControlOps()
{
    return &mNewLayerControlOps;
}
