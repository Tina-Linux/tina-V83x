
//#define CONFIG_LOG_LEVEL    OPTION_LOG_LEVEL_DETAIL
//#define LOG_TAG "videoDecComponent"
#include "log.h"

#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>

#include "videoDecComponent.h"
#include "messageQueue.h"
#include "memoryAdapter.h"

typedef struct VideoDecCompContext
{
    //* created at initialize time.
    MessageQueue*       mq;
    sem_t               startMsgReplySem;
    sem_t               stopMsgReplySem;
    sem_t               pauseMsgReplySem;
    sem_t               resetMsgReplySem;
    sem_t               eosMsgReplySem;
    sem_t               quitMsgReplySem;
    sem_t               streamDataSem;
    sem_t               frameBufferSem;

    int                 nStartReply;
    int                 nStopReply;
    int                 nPauseReply;
    int                 nResetReply;

    pthread_t           sDecodeThread;

    VideoDecoder*       pDecoder;
    enum EPLAYERSTATUS  eStatus;

    //* objects set by user.
    AvTimer*            pAvTimer;
    PlayerCallback      callback;
    void*               pUserData;
    int                 bEosFlag;

    int                 bConfigDecodeKeyFrameOnly;
    int                 bConfigDropDelayFrames;

    int                 bResolutionChange;
    int                 bCrashFlag;

    VConfig             vconfig;
    VideoStreamInfo     videoStreamInfo;
    int                 nSelectStreamIndex;

	//*for new display
    VideoRenderComp*    pVideoRenderComp;
    VideoRenderCallback videoRenderCallback;
    void*               pVideoRenderUserData;
    FbmBufInfo          mFbmBufInfo;
    int                 bFbmBufInfoValidFlag;

    char*               pSecureBuf;// for wvm video

    int                 nGpuAlignStride;
    int                 bUseNewDisplayFlag;

    pthread_mutex_t     videoRenderCallbackMutex;
    struct ScMemOpsS*   memOps;

}VideoDecCompContext;

static void* VideoDecodeThread(void* arg);
static void PostDecodeMessage(MessageQueue* mq);
static int CallbackProcess(void* pUserData, int eMessageId, void* param);


VideoDecComp* VideoDecCompCreate(void)
{
    VideoDecCompContext* p;
    int                  err;

    p = (VideoDecCompContext*)malloc(sizeof(VideoDecCompContext));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }
    memset(p, 0, sizeof(*p));

    p->mq = MessageQueueCreate(4, "VideoDecodeMq");
    if(p->mq == NULL)
    {
        loge("video decoder component create message queue fail.");
        free(p);
        return NULL;
    }

    p->pDecoder = CreateVideoDecoder();
    if(p->pDecoder == NULL)
    {
        loge("video decoder component create decoder fail.");
        MessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    err = pthread_mutex_init(&p->videoRenderCallbackMutex, NULL);
    if(err != 0)
    {
        loge("pthread_mutex_init failed, err = %d",err);
        free(p);
        return NULL;
    }

    sem_init(&p->startMsgReplySem, 0, 0);
    sem_init(&p->stopMsgReplySem, 0, 0);
    sem_init(&p->pauseMsgReplySem, 0, 0);
    sem_init(&p->quitMsgReplySem, 0, 0);
    sem_init(&p->resetMsgReplySem, 0, 0);
    sem_init(&p->eosMsgReplySem, 0, 0);
    sem_init(&p->streamDataSem, 0, 0);
    sem_init(&p->frameBufferSem, 0, 0);

    p->eStatus = PLAYER_STATUS_STOPPED;

    err = pthread_create(&p->sDecodeThread, NULL, VideoDecodeThread, p);
    if(err != 0)
    {
        loge("video decode component create thread fail.");
        sem_destroy(&p->startMsgReplySem);
        sem_destroy(&p->stopMsgReplySem);
        sem_destroy(&p->pauseMsgReplySem);
        sem_destroy(&p->quitMsgReplySem);
        sem_destroy(&p->resetMsgReplySem);
        sem_destroy(&p->eosMsgReplySem);
        sem_destroy(&p->streamDataSem);
        sem_destroy(&p->frameBufferSem);
        DestroyVideoDecoder(p->pDecoder);
        MessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

	//*for new display
    //* we set gpu align to 16 as defual, maybe we should set it
    //* rely on different chip(as 1673, 1680) if they are not the same
#if(USE_NEW_DISPLAY == 1)
    #if(USE_NEW_DISPLAY_GPU_ALIGN_STRIDE == GPU_ALIGN_STRIDE_32)
        p->nGpuAlignStride = 32;
    #elif(USE_NEW_DISPLAY_GPU_ALIGN_STRIDE == GPU_ALIGN_STRIDE_16)
        p->nGpuAlignStride = 16;
    #endif
#else
    p->nGpuAlignStride = 16;
#endif

#if(USE_NEW_DISPLAY == 1)
    p->bUseNewDisplayFlag = 1;
#else
    p->bUseNewDisplayFlag = 0;
#endif

    return (VideoDecComp*)p;
}



int VideoDecCompDestroy(VideoDecComp* v)
{
    void*                status;
    VideoDecCompContext* p;
    Message              msg;

    p = (VideoDecCompContext*)v;

    msg.messageId = MESSAGE_ID_QUIT;
    msg.params[0] = (uintptr_t)&p->quitMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;
    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video decode component post message fail.");
        abort();
    }

    if(p->memOps != NULL)
    {
        if(p->pSecureBuf != NULL)
        {
            CdcMemPfree(p->memOps, p->pSecureBuf);
        }
        CdcMemClose(p->memOps);
    }

    if(p->videoStreamInfo.pCodecSpecificData)
	free(p->videoStreamInfo.pCodecSpecificData);
    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    SemTimedWait(&p->quitMsgReplySem, -1);
    pthread_join(p->sDecodeThread, &status);

    sem_destroy(&p->startMsgReplySem);
    sem_destroy(&p->stopMsgReplySem);
    sem_destroy(&p->pauseMsgReplySem);
    sem_destroy(&p->quitMsgReplySem);
    sem_destroy(&p->resetMsgReplySem);
    sem_destroy(&p->eosMsgReplySem);
    sem_destroy(&p->streamDataSem);
    sem_destroy(&p->frameBufferSem);
    DestroyVideoDecoder(p->pDecoder);
    MessageQueueDestroy(p->mq);

    pthread_mutex_destroy(&p->videoRenderCallbackMutex);

    free(p);

    return 0;
}


int VideoDecCompStart(VideoDecComp* v)
{
    VideoDecCompContext* p;
    Message              msg;

    p = (VideoDecCompContext*)v;

    logv("video decode component starting");

    msg.messageId = MESSAGE_ID_START;
    msg.params[0] = (uintptr_t)&p->startMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStartReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->startMsgReplySem, -1) < 0)
    {
        loge("video decode component wait for start finish timeout.");
        return -1;
    }

    return p->nStartReply;
}


int VideoDecCompStop(VideoDecComp* v)
{
    VideoDecCompContext* p;
    Message              msg;

    p = (VideoDecCompContext*)v;

    logv("video decode component stopping");

    msg.messageId = MESSAGE_ID_STOP;
    msg.params[0] = (uintptr_t)&p->stopMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStopReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->stopMsgReplySem, -1) < 0)
    {
        loge("video decode component wait for stop finish timeout.");
        return -1;
    }

    return p->nStopReply;
}


int VideoDecCompPause(VideoDecComp* v)
{
    VideoDecCompContext* p;
    Message              msg;

    p = (VideoDecCompContext*)v;

    logv("video decode component pausing");

    msg.messageId = MESSAGE_ID_PAUSE;
    msg.params[0] = (uintptr_t)&p->pauseMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nPauseReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->pauseMsgReplySem, -1) < 0)
    {
        loge("video decode component wait for pause finish timeout.");
        return -1;
    }

    return p->nPauseReply;
}


enum EPLAYERSTATUS VideoDecCompGetStatus(VideoDecComp* v)
{
    VideoDecCompContext* p;
    p = (VideoDecCompContext*)v;
    return p->eStatus;
}


int VideoDecCompReset(VideoDecComp* v)
{
    VideoDecCompContext* p;
    Message              msg;

    p = (VideoDecCompContext*)v;

    logv("video decode component reseting");

    msg.messageId = MESSAGE_ID_RESET;
    msg.params[0] = (uintptr_t)&p->resetMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nResetReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->resetMsgReplySem, -1) < 0)
    {
        loge("video decode component wait for reset finish timeout.");
        return -1;
    }

    return p->nResetReply;
}


int VideoDecCompSetEOS(VideoDecComp* v)
{
    VideoDecCompContext* p;
    Message              msg;

    p = (VideoDecCompContext*)v;

    logv("video decode component setting EOS.");

    msg.messageId = MESSAGE_ID_EOS;
    msg.params[0] = (uintptr_t)&p->eosMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->eosMsgReplySem, -1) < 0)
    {
        loge("video decode component wait for setting eos finish timeout.");
        return -1;
    }

    return 0;
}


int VideoDecCompSetCallback(VideoDecComp* v, PlayerCallback callback, void* pUserData)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    p->callback  = callback;
    p->pUserData = pUserData;

    return 0;
}


int VideoDecCompSetDecodeKeyFrameOnly(VideoDecComp* v, int bDecodeKeyFrameOnly)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    p->bConfigDecodeKeyFrameOnly  = bDecodeKeyFrameOnly;

    return 0;
}


int VideoDecCompSetDropDelayFrames(VideoDecComp* v, int bDropDelayFrames)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    p->bConfigDropDelayFrames  = bDropDelayFrames;

    return 0;
}


int VideoDecCompSetVideoStreamInfo(VideoDecComp* v, VideoStreamInfo* pStreamInfo, VConfig* pVconfig)
{
    VideoDecCompContext* p;
    int ret;
    p = (VideoDecCompContext*)v;
	pVconfig->nAlignStride = p->nGpuAlignStride;

    if(p->bUseNewDisplayFlag == 1)
        pVconfig->bGpuBufValid = 1;  //* in awplayer, we set bGpuBufValid to 1 as defual
    else
        pVconfig->bGpuBufValid = 0;

    logd("++++++++ pVconfig->bGpuBufValid = %d,nGpuAlignStride = %d ",
		 pVconfig->bGpuBufValid,p->nGpuAlignStride);
	//* save the config and video stream info
	if(p->videoStreamInfo.pCodecSpecificData)
    {
        free(p->videoStreamInfo.pCodecSpecificData);
        p->videoStreamInfo.pCodecSpecificData = NULL;
        p->videoStreamInfo.nCodecSpecificDataLen = 0;
    }

    memcpy(&p->vconfig, pVconfig, sizeof(VConfig));
    memcpy(&p->videoStreamInfo, pStreamInfo, sizeof(VideoStreamInfo));

    if(pStreamInfo->nCodecSpecificDataLen > 0)
    {
        p->videoStreamInfo.pCodecSpecificData = (char*)malloc(pStreamInfo->nCodecSpecificDataLen);
        if(p->videoStreamInfo.pCodecSpecificData == NULL)
        {
            loge("malloc video codec specific data fail!");
            return -1;
        }
        memcpy(p->videoStreamInfo.pCodecSpecificData,
               pStreamInfo->pCodecSpecificData,
               pStreamInfo->nCodecSpecificDataLen);
        p->videoStreamInfo.nCodecSpecificDataLen = pStreamInfo->nCodecSpecificDataLen;
    }

    logv("pStreamInfo->bSecureStreamFlagLevel1 = %d",pStreamInfo->bSecureStreamFlagLevel1);
    if(pStreamInfo->bSecureStreamFlagLevel1 == 1)
    {
        pVconfig->memops = SecureMemAdapterGetOpsS();
    }
    else
    {
        pVconfig->memops = MemAdapterGetOpsS();
    }

    ret = InitializeVideoDecoder(p->pDecoder, pStreamInfo, pVconfig);

	//* set secure buffer to wvm parser
    if(pStreamInfo->bSecureStreamFlagLevel1 == 1)
    {
        char* pBuf;
        int nBufferCount = 1;
        int nBufferSize  = 256*1024;
        uintptr_t param[2];

        //* set buffer count
        int nMessageId = PLAYER_VIDEO_DECODER_NOTIFY_SET_SECURE_BUFFER_COUNT;
        p->callback(p->pUserData,nMessageId,(void*)&nBufferCount);

        if(p->memOps != NULL)
        {
            if(p->pSecureBuf != NULL)
            {
                CdcMemPfree(p->memOps, p->pSecureBuf);
            }

            CdcMemClose(p->memOps);
        }

        p->memOps = SecureMemAdapterGetOpsS();
        CdcMemOpen(p->memOps);

        pBuf = (char*)CdcMemPalloc(p->memOps, nBufferSize);
        if(pBuf == NULL)
        {
            loge("video request secure buffer failed!");
            ret = -1;
			goto _exit;
        }

        //* set buffer address
        param[0]   = nBufferSize;
        param[1]   = (uintptr_t)pBuf;
        nMessageId = PLAYER_VIDEO_DECODER_NOTIFY_SET_SECURE_BUFFERS;

        p->callback(p->pUserData,nMessageId,(void*)param);

        p->pSecureBuf = pBuf;
    }
    else
    {
        p->memOps = MemAdapterGetOpsS();
        CdcMemOpen(p->memOps);
    }
	_exit:
	if(ret < 0)
	{
		p->bCrashFlag = 1;
		loge("VideoDecCompSetVideoStreamInfo fail");
	}

    return ret;
}

int VideoDecCompGetVideoStreamInfo(VideoDecComp* v, VideoStreamInfo* pStreamInfo)
{
    VideoDecCompContext* p;
    p = (VideoDecCompContext*)v;
    return GetVideoStreamInfo(p->pDecoder, pStreamInfo);
}


int VideoDecCompSetTimer(VideoDecComp* v, AvTimer* timer)
{
    VideoDecCompContext* p;
    p = (VideoDecCompContext*)v;
    p->pAvTimer  = timer;
    return 0;
}

int VideoDecCompRequestStreamBuffer(VideoDecComp* v,
                                    int           nRequireSize,
                                    char**        ppBuf,
                                    int*          pBufSize,
                                    char**        ppRingBuf,
                                    int*          pRingBufSize,
                                    int           nStreamIndex)
{
    int ret;
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    //* we can use the same sbm interface to process secure video

    ret = RequestVideoStreamBuffer(p->pDecoder,
                                    nRequireSize,
                                    ppBuf,
                                    pBufSize,
                                    ppRingBuf,
                                    pRingBufSize,
                                    nStreamIndex);

    if(p->bCrashFlag && (ret < 0 || ((*pBufSize + *pRingBufSize) < nRequireSize)))
    {
        //* decoder crashed.
        ResetVideoDecoder(p->pDecoder); //* flush streams.
    }

    return ret;
}


int VideoDecCompSubmitStreamData(VideoDecComp*        v,
                                 VideoStreamDataInfo* pDataInfo,
                                 int                  nStreamIndex)
{
    int                  ret;
    int                  nSemCnt;
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    //* don't receive input stream when decoder crashed.
    //* so the stream buffer always has empty buffer for the demux.
    //* otherwise the demux thread will blocked when video stream buffer is full.
    if(p->bCrashFlag)
    {
        return 0;
    }

    //* we can use the same sbm interface to process secure video

    ret = SubmitVideoStreamData(p->pDecoder, pDataInfo, nStreamIndex);
    if(sem_getvalue(&p->streamDataSem, &nSemCnt) == 0)
    {
        if(nSemCnt == 0)
            sem_post(&p->streamDataSem);
    }
    p->nSelectStreamIndex = nStreamIndex;

    return ret;
}


int VideoDecCompStreamBufferSize(VideoDecComp* v, int nStreamIndex)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;
    if(p == NULL)
    {
	return 0;
    }
    return VideoStreamBufferSize(p->pDecoder, nStreamIndex);
}


int VideoDecCompStreamDataSize(VideoDecComp* v, int nStreamIndex)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    if(p == NULL)
    {
	return 0;
    }
    return VideoStreamDataSize(p->pDecoder, nStreamIndex);
}


int VideoDecCompStreamFrameNum(VideoDecComp* v, int nStreamIndex)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;
    if(p == NULL)
    {
	return 0;
    }
    return VideoStreamFrameNum(p->pDecoder, nStreamIndex);
}


VideoPicture* VideoDecCompRequestPicture(VideoDecComp* v, int nStreamIndex, int* bResolutionChanged)
{
    VideoDecCompContext* p;
    VideoPicture*        pPicture;

    p = (VideoDecCompContext*)v;

    pPicture = RequestPicture(p->pDecoder, nStreamIndex);
    if(bResolutionChanged != NULL)
    {
        if(pPicture != NULL || p->bResolutionChange == 0)
            *bResolutionChanged = 0;
        else
        {
            logd("set resolution changed.");
            *bResolutionChanged = 1;
        }
    }
    return pPicture;
}


int VideoDecCompReturnPicture(VideoDecComp* v, VideoPicture* pPicture)
{
    int                  ret;
    int                  nSemCnt;
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    ret = ReturnPicture(p->pDecoder, pPicture);
    if(sem_getvalue(&p->frameBufferSem, &nSemCnt) == 0)
    {
        if(nSemCnt == 0)
            sem_post(&p->frameBufferSem);
    }

    return ret;
}


VideoPicture* VideoDecCompNextPictureInfo(VideoDecComp* v, int nStreamIndex, int* bResolutionChanged)
{
    VideoDecCompContext* p;
    VideoPicture*        pPicture;

    p = (VideoDecCompContext*)v;

    pPicture = NextPictureInfo(p->pDecoder, nStreamIndex);
    if(bResolutionChanged != NULL)
    {
        if(pPicture != NULL || p->bResolutionChange == 0)
            *bResolutionChanged = 0;
        else
        {
            logd("set resolution changed.");
            *bResolutionChanged = 1;
        }
    }
    return pPicture;
}


int VideoDecCompTotalPictureBufferNum(VideoDecComp* v, int nStreamIndex)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return TotalPictureBufferNum(p->pDecoder, nStreamIndex);
}


int VideoDecCompEmptyPictureBufferNum(VideoDecComp* v, int nStreamIndex)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return EmptyPictureBufferNum(p->pDecoder, nStreamIndex);
}


int VideoDecCompValidPictureNum(VideoDecComp* v, int nStreamIndex)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return ValidPictureNum(p->pDecoder, nStreamIndex);
}


int VideoDecCompConfigHorizonScaleDownRatio(VideoDecComp* v, int nScaleDownRatio)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return ConfigHorizonScaleDownRatio(p->pDecoder, nScaleDownRatio);
}


int VideoDecCompConfigVerticalScaleDownRatio(VideoDecComp* v, int nScaleDownRatio)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return ConfigVerticalScaleDownRatio(p->pDecoder, nScaleDownRatio);
}


int VideoDecCompConfigRotation(VideoDecComp* v, int nRotateDegree)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return ConfigRotation(p->pDecoder, nRotateDegree);
}


int VideoDecCompConfigDeinterlace(VideoDecComp* v, int bDeinterlace)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return ConfigDeinterlace(p->pDecoder, bDeinterlace);
}

int VideoDecCompConfigThumbnailMode(VideoDecComp* v, int bOpenThumbnailMode)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return ConfigThumbnailMode(p->pDecoder, bOpenThumbnailMode);
}


int VideoDecCompConfigOutputPicturePixelFormat(VideoDecComp* v, int ePixelFormat)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return ConfigOutputPicturePixelFormat(p->pDecoder, ePixelFormat);
}


int VideoDecCompConfigNoBFrame(VideoDecComp* v, int bNoBFrames)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return ConfigNoBFrames(p->pDecoder, bNoBFrames);
}


int VideoDecCompConfigDisable3D(VideoDecComp* v, int bDisable3D)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return ConfigDisable3D(p->pDecoder, bDisable3D);
}

int VideoDecCompSetMemortThresh(VideoDecComp* v, int nMemoryThresh)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return ConfigVeMemoryThresh(p->pDecoder, nMemoryThresh);
}

int VideoDecCompReopenVideoEngine(VideoDecComp* v)
{
    VideoDecCompContext* p;
    int                  ret;

    p = (VideoDecCompContext*)v;

    VideoStreamInfo* pVideoInfo =
        (VideoStreamInfo*)VideoStreamDataInfoPointer(p->pDecoder,p->nSelectStreamIndex);
    if(pVideoInfo != NULL)
    {
        if(p->videoStreamInfo.pCodecSpecificData)
            free(p->videoStreamInfo.pCodecSpecificData);
        memcpy(&p->videoStreamInfo,pVideoInfo,sizeof(VideoStreamInfo));
        if(pVideoInfo->pCodecSpecificData)
        {
            p->videoStreamInfo.pCodecSpecificData =
                (char*)malloc(pVideoInfo->nCodecSpecificDataLen);
            if(p->videoStreamInfo.pCodecSpecificData == NULL)
            {
                loge("malloc video codec specific data failed!");
                return -1;
            }
            memcpy(p->videoStreamInfo.pCodecSpecificData,
                   pVideoInfo->pCodecSpecificData,
                   pVideoInfo->nCodecSpecificDataLen);
            p->videoStreamInfo.nCodecSpecificDataLen = pVideoInfo->nCodecSpecificDataLen;

            free(pVideoInfo->pCodecSpecificData);
        }
        else
        {
            p->videoStreamInfo.pCodecSpecificData    = NULL;
            p->videoStreamInfo.nCodecSpecificDataLen = 0;
        }

		free(pVideoInfo);
    }
	else
	{
		//*if resolustionChange was detected by decoder, we should not send the
		//* specific data to decoder. or decoder will appear error.
		if(p->videoStreamInfo.pCodecSpecificData)
            free(p->videoStreamInfo.pCodecSpecificData);

		p->videoStreamInfo.pCodecSpecificData = NULL;
		p->videoStreamInfo.nCodecSpecificDataLen = 0;
	}


    ret = ReopenVideoEngine(p->pDecoder,&p->vconfig,&p->videoStreamInfo);

    p->bResolutionChange = 0;
    PostDecodeMessage(p->mq);

    return ret;
}


int VideoDecCompRotatePicture(VideoDecComp* v,
                              VideoPicture* pPictureIn,
                              VideoPicture* pPictureOut,
                              int           nRotateDegree,
                              int           nGpuYAlign,
                              int           nGpuCAlign)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    //* on chip-1673,we use hardware to do video rotation
#if(ROTATE_PIC_HW == 1)
    CDX_PLAYER_UNUSE(nGpuYAlign);
    CDX_PLAYER_UNUSE(nGpuCAlign);
    return RotatePictureHw(p->pDecoder,pPictureIn, pPictureOut, nRotateDegree);
#else
    //* rotatePicture() should known the gpuAlign
    return RotatePicture(p->memOps, pPictureIn, pPictureOut, nRotateDegree,nGpuYAlign,nGpuCAlign);
#endif

}

/*
VideoPicture* VideoDecCompAllocatePictureBuffer(int nWidth, int nHeight, int nLineStride, int ePixelFormat)
{
    return AllocatePictureBuffer(nWidth, nHeight, nLineStride, ePixelFormat);
}

int VideoDecCompFreePictureBuffer(VideoPicture* pPicture)
{
    return FreePictureBuffer(pPicture);
}
*/

#if 0
//* for new display
int VideoDecCompSetVideoRenderCallback(VideoDecComp* v, VideoRenderCallback callback, void* pUserData)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    pthread_mutex_lock(&p->videoRenderCallbackMutex);
    p->videoRenderCallback  = callback;
    pthread_mutex_unlock(&p->videoRenderCallbackMutex);

    p->pVideoRenderUserData = pUserData;
    return 0;
}
#endif

//*******************************  START  **********************************//
//** for new display structure interface.
//**
FbmBufInfo* VideoDecCompGetVideoFbmBufInfo(VideoDecComp* v)
{
	VideoDecCompContext* p;

	p = (VideoDecCompContext*)v;

	if(p->pDecoder != NULL)
	{
		return GetVideoFbmBufInfo(p->pDecoder);
	}
	else
	{
		loge("the pDecoder is null when call VideoDecCompGetVideoFbmBufInfo()");
		return NULL;
	}
}

VideoPicture* VideoDecCompSetVideoFbmBufAddress(VideoDecComp* v, VideoPicture* pVideoPicture, int bForbidUseFlag)
{
	VideoDecCompContext* p;

	p = (VideoDecCompContext*)v;

	if(p->pDecoder != NULL)
	{
		return SetVideoFbmBufAddress(p->pDecoder,pVideoPicture,bForbidUseFlag);
	}
	else
	{
		loge("the pDecoder is null when call VideoDecCompSetVideoFbmBufAddress()");
		return NULL;
	}
}

int VideoDecCompSetVideoFbmBufRelease(VideoDecComp* v)
{
	VideoDecCompContext* p;

	p = (VideoDecCompContext*)v;

	if(p->pDecoder != NULL)
	{
		return SetVideoFbmBufRelease(p->pDecoder);
	}
	else
	{
		loge("the pDecoder is null when call VideoDecCompSetVideoFbmBufRelease()");
		return -1;
	}
}

VideoPicture* VideoDecCompRequestReleasePicture(VideoDecComp* v)
{
	VideoDecCompContext* p;

	p = (VideoDecCompContext*)v;

	if(p->pDecoder != NULL)
	{
		return RequestReleasePicture(p->pDecoder);
	}
	else
	{
		loge("the pDecoder is null when call VideoDecCompRequestReleasePicture()");
		return NULL;
	}
}

VideoPicture*  VideoDecCompReturnRelasePicture(VideoDecComp* v, VideoPicture* pVpicture, int bForbidUseFlag)
{
	VideoDecCompContext* p;

	p = (VideoDecCompContext*)v;

	if(p->pDecoder != NULL)
	{
		return ReturnRelasePicture(p->pDecoder,pVpicture, bForbidUseFlag);
	}
	else
	{
		loge("the pDecoder is null when call VideoDecCompReturnRelasePicture()");
		return NULL;
	}
}


//***************************************************************************//
//*********************added by xyliu at 2015-12-23*****************************************************//
int VideoDecCompSetExtraScaleInfo(VideoDecComp* v, int nWidthTh, int nHeightTh,
		                          int nHorizontalScaleRatio, int nVerticalScaleRatio)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

    return ConfigExtraScaleInfo(p->pDecoder, nWidthTh, nHeightTh, nHorizontalScaleRatio, nVerticalScaleRatio);

}
//****************************************************************************//
//********************************  END  ***********************************//

#if 0
void VideoDecCompFreePictureBuffer(VideoDecComp* v, VideoPicture* pPicture)
{
    VideoDecCompContext* p;

    p = (VideoDecCompContext*)v;

	FreePictureBuffer(v, pPicture);
}
#endif

static void* VideoDecodeThread(void* arg)
{
    VideoDecCompContext* p;
    Message              msg;
    int                  ret;
    sem_t*               pReplySem;
    int*                 pReplyValue;
    int64_t              nCurTime;
    int                  bFirstFrameDecoded;

    p = (VideoDecCompContext*)arg;
    bFirstFrameDecoded = 0;

    while(1)
    {
        if(MessageQueueGetMessage(p->mq, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }

        pReplySem   = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == MESSAGE_ID_START)
        {
            logi("process MESSAGE_ID_START message");
            //* check status.
            if(p->eStatus == PLAYER_STATUS_STARTED)
            {
                loge("invalid start operation, already in started status.");
		        if(pReplyValue != NULL)
			        *pReplyValue = -1;
		        sem_post(pReplySem);
		        continue;
            }

            if(p->eStatus == PLAYER_STATUS_PAUSED)
            {
                //* send a decode message to start decoing.
                if(p->bCrashFlag == 0)
                    PostDecodeMessage(p->mq);
                p->eStatus = PLAYER_STATUS_STARTED;
		        if(pReplyValue != NULL)
			        *pReplyValue = 0;
		        sem_post(pReplySem);
		        continue;
            }

            //* send a decode message to start decode processing, change status to start.
            PostDecodeMessage(p->mq);
            ResetVideoDecoder(p->pDecoder);
            p->bEosFlag = 0;
            p->eStatus = PLAYER_STATUS_STARTED;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_STOP)
        {
            logd("process MESSAGE_ID_STOP message");
            //* check status.
            if(p->eStatus == PLAYER_STATUS_STOPPED)
            {
                loge("invalid stop operation, already in stopped status.");
		        if(pReplyValue != NULL)
			        *pReplyValue = -1;
		        sem_post(pReplySem);
		        continue;
            }

            bFirstFrameDecoded = 0;
            ResetVideoDecoder(p->pDecoder);
            p->eStatus = PLAYER_STATUS_STOPPED;
            //p->bCrashFlag = 0;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_PAUSE)
        {
            logi("process MESSAGE_ID_PAUSE message");
            //* check status.
            if(p->eStatus != PLAYER_STATUS_STARTED  &&
               !(p->eStatus == PLAYER_STATUS_PAUSED && bFirstFrameDecoded != 1))
            {
                loge("invalid pause operation, component not in started status.");
		        if(pReplyValue != NULL)
			        *pReplyValue = -1;
		        sem_post(pReplySem);
		        continue;
            }

            p->eStatus = PLAYER_STATUS_PAUSED;
            if(bFirstFrameDecoded != 1)
                PostDecodeMessage(p->mq);   //* post a decode message to decode the first picture.

		    if(pReplyValue != NULL)
			    *pReplyValue = 0;
		    sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_QUIT)
        {
            logi("process MESSAGE_ID_QUIT message");
            ResetVideoDecoder(p->pDecoder);
            p->eStatus = PLAYER_STATUS_STOPPED;
		    if(pReplyValue != NULL)
			    *pReplyValue = 0;
		    sem_post(pReplySem);
		    break;
        }
        else if(msg.messageId == MESSAGE_ID_RESET)
        {
            logi("process MESSAGE_ID_RESET message");
            p->bEosFlag = 0;
            bFirstFrameDecoded = 0;
            ResetVideoDecoder(p->pDecoder);
            //p->bCrashFlag = 0;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus != PLAYER_STATUS_STOPPED)
                PostDecodeMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_EOS)
        {
            logi("process MESSAGE_ID_EOS message");
            p->bEosFlag = 1;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);

            if(p->bCrashFlag == 0)
            {
                //* send a message to continue the thread.
                if(p->eStatus == PLAYER_STATUS_STARTED ||
                   (p->eStatus == PLAYER_STATUS_PAUSED && bFirstFrameDecoded != 1))
                    PostDecodeMessage(p->mq);
            }
			else
			{
				logv("video decoder notify eos.");
				p->callback(p->pUserData, PLAYER_VIDEO_DECODER_NOTIFY_EOS, NULL);
			}
        }
        else if(msg.messageId == MESSAGE_ID_DECODE)
        {
            logi("process MESSAGE_ID_DECODE message");
            if(p->eStatus != PLAYER_STATUS_STARTED &&
               !(p->eStatus == PLAYER_STATUS_PAUSED && bFirstFrameDecoded != 1))
            {
                loge("not in started status, ignore decode message.");
                continue;
            }
			if(p->bCrashFlag)
			{
                logw("video decoder has already crashed,  MESSAGE_ID_DECODE will not be processe.");
                continue;
			}

            nCurTime = p->pAvTimer->GetTime(p->pAvTimer);

            ret = DecodeVideoStream(p->pDecoder,
                                    p->bEosFlag,
                                    p->bConfigDecodeKeyFrameOnly,
                                    p->bConfigDropDelayFrames,
                                    nCurTime);

            logv("DecodeVideoStream return = %d, p->bCrashFlag(%d)", ret, p->bCrashFlag);

            if(ret == VDECODE_RESULT_NO_BITSTREAM)
            {
                if(p->bEosFlag)
                {
                    logv("video decoder notify eos.");
                    p->callback(p->pUserData, PLAYER_VIDEO_DECODER_NOTIFY_EOS, NULL);
                    continue;
                }
                else
                {
                    SemTimedWait(&p->streamDataSem, 20);    //* wait for stream data.
                    PostDecodeMessage(p->mq);
                    continue;
                }
            }
            else if(ret == VDECODE_RESULT_NO_FRAME_BUFFER)
            {
                SemTimedWait(&p->frameBufferSem, 20);    //* wait for frame buffer.
                PostDecodeMessage(p->mq);
                continue;
            }
            else if(ret == VDECODE_RESULT_RESOLUTION_CHANGE)
            {
                logv("decode thread detect resolution change.");
                p->bResolutionChange = 1;
				//*for new display, we should callback the message to videoRender
				if(p->bUseNewDisplayFlag == 1 && p->videoRenderCallback != NULL)
				{
                    int msg = VIDEO_RENDER_RESOLUTION_CHANGE;
                    p->videoRenderCallback(p->pVideoRenderUserData,msg,(void*)(&p->bResolutionChange));
                }
                continue;
            }
            else if(ret < 0)    //* ret == VDECODE_RESULT_UNSUPPORTED
            {
                logw("video decoder notify crash.");
                p->bCrashFlag = 1;
                p->callback(p->pUserData, PLAYER_VIDEO_DECODER_NOTIFY_CRASH, NULL);
                continue;
            }
            else
            {
                if(bFirstFrameDecoded != 1)
                {
                    if(ret == VDECODE_RESULT_FRAME_DECODED || ret == VDECODE_RESULT_KEYFRAME_DECODED)
                    {
                        VideoStreamInfo videoStreamInfo;
                        GetVideoStreamInfo(p->pDecoder, &videoStreamInfo);
                        if(videoStreamInfo.bIs3DStream)
                        {
                            if(bFirstFrameDecoded == 2)
                                bFirstFrameDecoded = 1;
                            else
                                bFirstFrameDecoded = 2; //*  bFirstFrameDecoded == 2 means decode one picture of a 3d frame(with two picture).
                        }
                        else
                            bFirstFrameDecoded = 1;
                    }
                }

                if(p->eStatus == PLAYER_STATUS_STARTED || bFirstFrameDecoded != 1)
                {
                    PostDecodeMessage(p->mq);
                    continue;
                }
                else
                {
                    //* p->eStatus == PLAYER_STATUS_PAUSED && bFirstFrameDecoded != 1
                    //* need to decode and show the first picture as soon as we can after seek.
                    logi("first picture decoded at paused status.");
                }
            }
        }
        else
        {
            loge("unknown message in video decode thread.");
            abort();
        }
    }

    ret = 0;
    pthread_exit(&ret);
    return NULL;
}


static void PostDecodeMessage(MessageQueue* mq)
{
    if(MessageQueueGetCount(mq)<=0)
    {
        Message msg;
        msg.messageId = MESSAGE_ID_DECODE;
        msg.params[0] = msg.params[1] = msg.params[2] = msg.params[3] = 0;
        if(MessageQueuePostMessage(mq, &msg) != 0)
        {
            loge("fatal error, audio decode component post message fail.");
            abort();
        }

        return;
    }
}
#if 0
//* for new display callback form decoder
static int CallbackProcess(void* pUserData, int eMessageId, void* param)
{
    logv("DecoderCallback, msg = %d",eMessageId);

    VideoDecCompContext* p;
    int msg;
    int ret;

    p = (VideoDecCompContext*)pUserData;

    //* The p->videoRenderCallback maybe is null when decoder callback
    //* message VIDEO_DEC_BUFFER_INFO.
    //* If it happen, we should save it and send it to VideoRender next time.
    pthread_mutex_lock(&p->videoRenderCallbackMutex);

    if(p->videoRenderCallback == NULL)
    {
        if(eMessageId == VIDEO_DEC_BUFFER_INFO)
        {
            FbmBufInfo* pFbmBufInfo = (FbmBufInfo*)param;
            memcpy(&p->mFbmBufInfo, pFbmBufInfo, sizeof(FbmBufInfo));
            p->bFbmBufInfoValidFlag = 1;
            pthread_mutex_unlock(&p->videoRenderCallbackMutex);
            return 0;
        }
        else
        {
            logw("the p->videoRenderCallback is null");
            pthread_mutex_unlock(&p->videoRenderCallbackMutex);
            return -1;
        }
    }

    if(p->bFbmBufInfoValidFlag == 1)
    {
        int msg;
        msg = VIDEO_RENDER_VIDEO_INFO;
        p->videoRenderCallback(p->pVideoRenderUserData,msg,(void*)(&p->mFbmBufInfo));
        p->bFbmBufInfoValidFlag = 0;
    }

    switch(eMessageId)
    {
        case VIDEO_DEC_BUFFER_INFO:
             msg = VIDEO_RENDER_VIDEO_INFO;
             break;
        case VIDEO_DEC_REQUEST_BUFFER:
             msg = VIDEO_RENDER_REQUEST_BUFFER;
             break;
        case VIDEO_DEC_DISPLAYER_BUFFER:
             msg = VIDEO_RENDER_DISPLAYER_BUFFER;
             break;
        case VIDEO_DEC_RETURN_BUFFER:
             msg = VIDEO_RENDER_RETURN_BUFFER;
             break;
        default:
             loge("the callback message is not support! msg = %d",eMessageId);
             pthread_mutex_unlock(&p->videoRenderCallbackMutex);
             return -1;
    }

    ret = p->videoRenderCallback(p->pVideoRenderUserData,msg,param);

    pthread_mutex_unlock(&p->videoRenderCallbackMutex);

    return ret;
}

#endif
