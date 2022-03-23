/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CTC_MediaProcessorImpl.cpp
 * Description : IPTV API implement
 * History :
 *
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "CTC_MediaProcessorImpl"

#include <dlfcn.h>
#include <utils/Log.h>
#include <android_runtime/AndroidRuntime.h>
#include <binder/IPCThreadState.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <ui/DisplayInfo.h>
#include <hardware/hwcomposer.h>
#include <android/native_window.h>

#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include "CTC_MediaProcessorImpl.h"
#include "player.h"
#include "CdxParser.h"
#include "CdxTypes.h"
#include "outputCtrl.h"
#include <cdx_log.h>

using namespace android;
#define SAVE_FILE_TS (0)
#define SAVE_FILE_ES (0)

// if PVIDEO_PARA_T is not always correct, must reconfig
// but the speed maybe slow down when switch channel
#define RECONFIG_VIDEO_WHEN_SWITCH_CHANNEL (1)

#define UNUSED(x) ((void)(x))

#define TS_READ_SIZE 1316
#define DATA_BUF_SIZE 12886272 //188*192*204 *n// 13160*1024  //it should n*188*192*204
// if the left buf in datBuf end, and the size less than ES Size;
// then we should memcpy the buf
#define PACKET_SIZE 256

#if SAVE_FILE_TS
    FILE    *file = NULL;
#endif

#if SAVE_FILE_ES
    FILE    *file1 = NULL;
    FILE    *file2 = NULL;
#endif

//* config
static const unsigned int CTC_MAX_VIDEO_PACKET_SIZE  = 256 * 1024;
static const unsigned int CTC_MAX_AUDIO_PACKET_SIZE  = 8 * 1024;
static const unsigned int CTC_MAX_TSDATA_PACKET_SIZE = 256 * 1024;
static const unsigned int CTC_ES_VIDEO_BUFFER_SIZE     = 10 * 1024;
static const unsigned int CTC_ES_AUDIO_BUFFER_SIZE     = 1 * 1024;

static int gVolume = 50;//0-100

#define PLANE_WIDTH 1280
#define PLANE_HEIGHT 720

static sp<Surface> videoSurface = NULL;
static sp<SurfaceControl> videoSfControl = NULL;

static sp<SurfaceComposerClient> client = NULL;
static DisplayInfo dinfo;

static long long GetNowUs()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000000 + now.tv_usec;
}

static int sw_surface_init()
{
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    client = new SurfaceComposerClient();
    if (client != NULL)
    {
        sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain));
        status_t status = client->getDisplayInfo(display, &dinfo);
        ALOGD("getDisplayInfo,status(%d)", status);
        if (status != 0)
        {
            dinfo.w = PLANE_WIDTH;
            dinfo.h = PLANE_HEIGHT;
        }

    }
    ALOGD("dinfo.w(%d), dinfo.h(%d)", dinfo.w, dinfo.h);

    videoSfControl = client->createSurface(
                String8("Surface.iptv"), dinfo.w, dinfo.h, HAL_PIXEL_FORMAT_YCrCb_420_SP, 0);
    if(videoSfControl == NULL)
        return -1;

    SurfaceComposerClient::openGlobalTransaction();
    videoSfControl->setLayer(21005);
    videoSfControl->show();
    videoSfControl->setSize(dinfo.w, dinfo.h);
    videoSfControl->setPosition(0, 0);
    Rect outCrop;
    outCrop.left = 0;
    outCrop.right = 1280;
    outCrop.top = 0;
    outCrop.bottom = 720;
    videoSfControl->setCrop(outCrop);

    SurfaceComposerClient::closeGlobalTransaction();
    videoSurface = videoSfControl->getSurface();
    logd("videoSfControl: %p ", videoSfControl.get());
    logd("%s surface0 = %p", __func__, videoSurface.get());
    ANativeWindow_Buffer outBuffer;
    videoSurface->lock(&outBuffer, NULL);
    memset((char*)outBuffer.bits, 0x10,(outBuffer.height * outBuffer.stride));
    memset((char*)outBuffer.bits + outBuffer.height * outBuffer.stride,
                0x80,(outBuffer.height * outBuffer.stride)/2);
    videoSurface->unlockAndPost();

    return 0;
}

static void* swget_VideoSurface( )
{
    ALOGD("%s surface = %p", __func__, videoSurface.get());
    return videoSurface.get();
    //return (videoSurface->getSurfaceTexture()).get();
}

#if defined(CONF_SEND_BLACK_FRAME_TO_GPU)
static int SendThreeBlackFrameToGpu(ANativeWindow* pNativeWindow)
{
    logd("SendThreeBlackFrameToGpu()");

    ANativeWindowBuffer* pWindowBuf;
    void*                pDataBuf;
    int                  i;
    int                  err;

    for(i = 0;i < 1;i++)
    {
        err = pNativeWindow->dequeueBuffer_DEPRECATED(pNativeWindow, &pWindowBuf);
        if(err != 0)
        {
            logw("dequeue buffer fail, return value from \
                    dequeueBuffer_DEPRECATED() method is %d.", err);
            return -1;
        }
        pNativeWindow->lockBuffer_DEPRECATED(pNativeWindow, pWindowBuf);

        //* lock the data buffer.
        {
            GraphicBufferMapper& mapper = GraphicBufferMapper::get();
            Rect bounds(pWindowBuf->stride, pWindowBuf->height);
            mapper.lock(pWindowBuf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &pDataBuf);
        }

        memset((char*)pDataBuf,0x10,(pWindowBuf->height * pWindowBuf->stride));
        memset((char*)pDataBuf + pWindowBuf->height * pWindowBuf->stride,
                        0x80,(pWindowBuf->height * pWindowBuf->stride)/2);

        //* unlock the buffer.
        {
            GraphicBufferMapper& mapper = GraphicBufferMapper::get();
            mapper.unlock(pWindowBuf->handle);
        }

        pNativeWindow->queueBuffer_DEPRECATED(pNativeWindow, pWindowBuf);
    }
    return 0;
}
#endif

int GetMediaProcessorVersion()
{
    logd("GetMediaProcessorVersion");
    return 2;
}

CTC_MediaProcessor* GetMediaProcessor()
{
    logd("GetMediaProcessor");
    return new CTC_MediaProcessorImpl;
}


static int CTCPlayerCallback(void* pUserData, int eMessageId, void* param)
{
    CTC_MediaProcessorImpl* c;
    logd("CTCPlayerCallback");
    c = (CTC_MediaProcessorImpl*)pUserData;
    return c->ProcessCallback(eMessageId, param);
}

CTC_MediaProcessorImpl::CTC_MediaProcessorImpl()
    : m_callBackFunc(NULL),
      m_eventHandler(NULL),
      //mSurface(NULL),
      mNativeWindow(NULL),
      mPlayer(NULL),
      mIsInited(false),
      mIsEos(false),
      mHoldLastPicture(false),
      mAudioTrackCount(0),
      mCurrentAudioTrack(0),
      mLeftBytes(0),
      mIsQuitedRequest(false),
      mIsQuited(false),
      mVideoX(0),
      mVideoY(0),
      mVideoWidth(0),
      mVideoHeight(0),
      mIsSetVideoPosition(false),
      mSeekRequest(false),
      mSeekReply(0),
      mEnv(NULL)
{
    logd("media processor construct!");

#if DEBUG_WRITE_DATA_SPEED
    mFirst = 0;
    mTotalSize = 0;
    mNowtime = 0;
#endif

    mVideoStreamBufSize = CTC_MAX_VIDEO_PACKET_SIZE;
    mVideoStreamBuf = malloc(mVideoStreamBufSize);
    if(mVideoStreamBuf == NULL)
        mVideoStreamBufSize = 0;

    mAudioStreamBufSize = CTC_MAX_AUDIO_PACKET_SIZE;
    mAudioStreamBuf = malloc(mVideoStreamBufSize);
    if(mAudioStreamBuf == NULL)
        mAudioStreamBufSize = 0;

    mTsStreamBufSize = 0;
    mTsStreamBuf = malloc(PACKET_SIZE);

    mPlayer = PlayerCreate();
    if(mPlayer == NULL)
    {
        loge("==== PlayerCreate faied");
    }

    mVideoPids = 0;
    memset(mAudioPids, 0, sizeof(mAudioPids));
    mDemux = ts_demux_open();

    memset(&mVideoPara, 0, sizeof(mVideoPara));
    memset(&mAudioPara, 0, sizeof(mAudioPara));

    pthread_cond_init(&mCond, NULL);
    pthread_mutex_init(&mSeekMutex, NULL);

    mDataBuffer = (unsigned char*)malloc(DATA_BUF_SIZE);
    mDataBufferSize = DATA_BUF_SIZE;
    mDataWritePtr = mDataBuffer;
    mDataReadPtr = mDataBuffer;
    mValidDataSize = 0;
    pthread_mutex_init(&mBufMutex, NULL);

    sw_surface_init();
    mSurface = (Surface *)swget_VideoSurface();
    logd("-----------------------0-----------");
    sp<ANativeWindow> tmp = videoSurface.get();
    tmp->perform(tmp.get(), NATIVE_WINDOW_SET_BUFFERS_GEOMETRY,0, 0, HAL_PIXEL_FORMAT_YCrCb_420_SP);

    logd("-----------------------1-----------");
    mNativeWindow = mSurface.get();
    logd("-----------------------2-----------");
    mNativeWindow->perform(mNativeWindow.get(),
                        NATIVE_WINDOW_SET_BUFFERS_GEOMETRY,
                        0,
                        0,
                        HAL_PIXEL_FORMAT_YCrCb_420_SP);

    if (mPlayer != NULL)
    {
        PlayerSetCallback(mPlayer, CTCPlayerCallback, (void*)this);

        LayerCtrl* lc;
        lc = LayerCreate();
        LayerControl(lc, CDX_LAYER_CMD_SET_NATIVE_WINDOW, mNativeWindow.get());
        PlayerSetWindow(mPlayer, lc);

        SoundCtrl* sc = SoundDeviceCreate(NULL);
        PlayerSetAudioSink(mPlayer, sc);

        Deinterlace* di = DeinterlaceCreate();
        PlayerSetDeinterlace(mPlayer, di);
    }

    if(pthread_create(&mWriteDataThreadId, NULL, WriteDataThread, this))
    {
        loge("create thread failed!\n");
    }

#if SAVE_FILE_TS
    file = fopen("/data/camera/save.ts", "wb+");
    if (!file)
    {
        CDX_LOGE("open file failure errno(%d)", errno);
    }
#endif

#if SAVE_FILE_ES
    file1 = fopen("/data/camera/video.dat", "wb+");
    if (!file1)
    {
        CDX_LOGE("open file failure errno(%d)", errno);
    }

    file2 = fopen("/data/camera/audio.dat", "wb+");
    if (!file2)
    {
        CDX_LOGE("open file failure errno(%d)", errno);
    }
#endif
}

CTC_MediaProcessorImpl::~CTC_MediaProcessorImpl()
{
    logd("media processor destroy!");
    mIsQuitedRequest = true;
    mIsQuited = true;
    pthread_join(mWriteDataThreadId, NULL);

    if(mVideoStreamBuf != NULL)
    {
        free(mVideoStreamBuf);
        mVideoStreamBuf = NULL;
    }
    if(mAudioStreamBuf != NULL)
    {
        free(mAudioStreamBuf);
        mAudioStreamBuf = NULL;
    }
    if(mTsStreamBuf != NULL)
    {
        free(mTsStreamBuf);
        mTsStreamBuf = NULL;
    }
    if(mDemux != NULL)
    {
        ts_demux_close(mDemux);
        mDemux = NULL;
    }
    if(mPlayer != NULL)
    {
        PlayerDestroy(mPlayer);
        mPlayer = NULL;
    }
    if (mNativeWindow != NULL)
    {
        mNativeWindow.clear();
        mNativeWindow = NULL;
    }

    pthread_cond_destroy(&mCond);
    pthread_mutex_destroy(&mSeekMutex);

    if(mDataBuffer)
    {
        free(mDataBuffer);
    }
    pthread_mutex_destroy(&mBufMutex);

#if SAVE_FILE_TS
    if (file)
    {
        fclose(file);
        file = NULL;
    }
#endif

#if SAVE_FILE_ES
    if (file1)
    {
        fclose(file1);
        file1 = NULL;
    }

    if (file2)
    {
        fclose(file2);
        file2 = NULL;
    }
#endif
}

extern "C" int RequestVideoBufferCallback(void* param, void* cookie)
{
    md_buf_t*               mdBuf;
    CTC_MediaProcessorImpl* self;

    mdBuf = (md_buf_t*)param;
    self  = (CTC_MediaProcessorImpl*)cookie;
    return self->RequestBuffer(&mdBuf->buf, &mdBuf->bufSize, (int)MEDIA_TYPE_VIDEO);
}

extern "C" int UpdateVideoDataCallback(void* param, void* cookie)
{
    int                     bIsFirst;
    int                     bIsLast;
    int64_t                 nPts;
    md_data_info_t*         mdDataInfo;
    CTC_MediaProcessorImpl* self;

    mdDataInfo = (md_data_info_t*)param;
    self  = (CTC_MediaProcessorImpl*)cookie;

    bIsFirst = (mdDataInfo->ctrlBits & FIRST_PART_BIT) ? 1 : 0;
    bIsLast  = (mdDataInfo->ctrlBits & LAST_PART_BIT) ? 1 : 0;
    nPts     = (mdDataInfo->ctrlBits & PTS_VALID_BIT) ? mdDataInfo->pts : -1;
    return self->UpdateData(nPts, mdDataInfo->dataLen, bIsFirst, bIsLast, (int)MEDIA_TYPE_VIDEO);
}

extern "C" int RequestAudioBufferCallback(void* param, void* cookie)
{
    md_buf_t*               mdBuf;
    CTC_MediaProcessorImpl* self;

    mdBuf = (md_buf_t*)param;
    self  = (CTC_MediaProcessorImpl*)cookie;
    return self->RequestBuffer(&mdBuf->buf, &mdBuf->bufSize, (int)MEDIA_TYPE_AUDIO);
}

extern "C" int UpdateAudioDataCallback(void* param, void* cookie)
{
    int                     bIsFirst;
    int                     bIsLast;
    int64_t                 nPts;
    md_data_info_t*         mdDataInfo;
    CTC_MediaProcessorImpl* self;

    mdDataInfo = (md_data_info_t*)param;
    self        = (CTC_MediaProcessorImpl*)cookie;
    bIsFirst   = (mdDataInfo->ctrlBits & FIRST_PART_BIT) ? 1 : 0;
    bIsLast    = (mdDataInfo->ctrlBits & LAST_PART_BIT) ? 1 : 0;
    nPts       = (mdDataInfo->ctrlBits & PTS_VALID_BIT) ? mdDataInfo->pts : -1;
    return self->UpdateData(nPts, mdDataInfo->dataLen, bIsFirst, bIsLast, (int)MEDIA_TYPE_AUDIO);
}

static enum EVIDEOCODECFORMAT videoCodecConvert(vformat_t codec)
{
    logd("videoCodecConvert");
    enum EVIDEOCODECFORMAT ret = VIDEO_CODEC_FORMAT_UNKNOWN;
    switch(codec)
    {
        case VFORMAT_MPEG12:
            ret = VIDEO_CODEC_FORMAT_MPEG2;
            break;
        case VFORMAT_MPEG4:
            logd(" mpeg4 , we should goto mpeg4Normal decoder");
            ret = VIDEO_CODEC_FORMAT_DIVX4;  // we should goto mpeg4Normal decoder
            break;
        case VFORMAT_H264:
            ret = VIDEO_CODEC_FORMAT_H264;
            break;
        case VFORMAT_MJPEG:
            ret = VIDEO_CODEC_FORMAT_MJPEG;
            break;
        case VFORMAT_REAL:
            ret = VIDEO_CODEC_FORMAT_RX;
            break;
        case VFORMAT_VC1:
            ret = VIDEO_CODEC_FORMAT_WMV3;
            break;
        case VFORMAT_H265:
            ret = VIDEO_CODEC_FORMAT_H265;
            break;
        case VFORMAT_AVS:
            ret = VIDEO_CODEC_FORMAT_AVS;
            break;
        default:
            loge("unsupported video codec format(%d)!", codec);
            break;
    }

    return ret;
}

static enum EAUDIOCODECFORMAT audioCodecConvert(aformat_t codec)
{
    logd("audioCodecConvert codec %d", codec);
    enum EAUDIOCODECFORMAT ret = AUDIO_CODEC_FORMAT_UNKNOWN;
    switch(codec)
    {
        case AFORMAT_MPEG:
            ret = AUDIO_CODEC_FORMAT_MP2;
            break;
        case AFORMAT_PCM_U8:
        case AFORMAT_ADPCM:
        case AFORMAT_PCM_S16BE:
        case AFORMAT_ALAW:
        case AFORMAT_MULAW:
        case AFORMAT_PCM_S16LE:
        case AFORMAT_PCM_BLURAY:
            ret = AUDIO_CODEC_FORMAT_PCM;
            break;
        case AFORMAT_RAAC:
        case AFORMAT_AAC:
            ret = AUDIO_CODEC_FORMAT_MPEG_AAC_LC;
            break;
        //case AFORMAT_DDPlus:
        case AFORMAT_AC3:
            ret = AUDIO_CODEC_FORMAT_AC3;
            break;
        case AFORMAT_DTS:
            ret = AUDIO_CODEC_FORMAT_DTS;
            break;
        case AFORMAT_FLAC:
            ret = AUDIO_CODEC_FORMAT_FLAC;
            break;
        case AFORMAT_COOK:
            ret = AUDIO_CODEC_FORMAT_COOK;
            break;
        case AFORMAT_AMR:
            ret = AUDIO_CODEC_FORMAT_AMR;
            break;
        case AFORMAT_WMA:
        case AFORMAT_WMAPRO:
            ret = AUDIO_CODEC_FORMAT_WMA_STANDARD;
            break;
        case AFORMAT_VORBIS:
            ret = AUDIO_CODEC_FORMAT_OGG;
            break;
        default:
            loge("unsupported video codec format!");
            break;
    }
    return ret;
}

int CTC_MediaProcessorImpl::ProcessCallback(int eMessageId, void* param)
{
    UNUSED(param);
    logd("ProcessCallback %d", eMessageId);
    switch(eMessageId)
    {
        case PLAYBACK_NOTIFY_EOS:
            logd("Player Notify EOS");
            mIsEos = true;
            break;
        case PLAYBACK_NOTIFY_FIRST_PICTURE:
            if(m_callBackFunc != NULL)
                m_callBackFunc(IPTV_PLAYER_EVT_FIRST_PTS, m_eventHandler, 0, 0);
            break;

        case PLAYBACK_NOTIFY_VIDEO_UNSUPPORTED:
            loge("do not support this video format");
            if(m_callBackFunc != NULL)
                m_callBackFunc(IPTV_PLAYER_EVT_ABEND, m_eventHandler, 0, 0);
            break;
        default:
            break;
    }

    return 0;
}

int CTC_MediaProcessorImpl::QueryBuffer(Player* pl, int nRequireSize,  int MediaType)
{
    UNUSED(pl);
    int             ret = 0;
    void*           pBuf0 = NULL;
    int             nBufSize0 = 0;
    void*           pBuf1 = NULL;
    int             nBufSize1 = 0;
    int             nStreamIndex = 0;
    enum EMEDIATYPE eMediaType;

    eMediaType = (enum EMEDIATYPE)MediaType;
    ret = PlayerRequestStreamBuffer(mPlayer, nRequireSize,
                &pBuf0, &nBufSize0, &pBuf1, &nBufSize1, eMediaType, nStreamIndex);
    if(nRequireSize > nBufSize0 + nBufSize1)
    {
        logd("not enough stream buffer, nBufSize0 %d, nBufSize1 %d", nBufSize0, nBufSize1);
        ret = -1;
    }
    return ret;
}

int CTC_MediaProcessorImpl::RequestBuffer(unsigned char** ppBuf, unsigned int* pSize, int MediaType)
{
    int             ret = 0;
    int             nRequireSize = 0;
    void*           pBuf0 = NULL;
    int             nBufSize0 = 0;
    void*           pBuf1 = NULL;
    int             nBufSize1 = 0;
    enum EMEDIATYPE eMediaType;
    int             nStreamIndex = 0;

    eMediaType = (enum EMEDIATYPE) MediaType;
    nRequireSize = (eMediaType == MEDIA_TYPE_VIDEO) ?
                    CTC_ES_VIDEO_BUFFER_SIZE : CTC_ES_AUDIO_BUFFER_SIZE;

    while(1)
    {
        ret = PlayerRequestStreamBuffer(mPlayer, nRequireSize,
                &pBuf0, &nBufSize0, &pBuf1, &nBufSize1, eMediaType, nStreamIndex);
        if(ret == 0)
        {
                if (eMediaType == MEDIA_TYPE_VIDEO)
                mVideoBufForTsDemux = (char*)pBuf0;
            else
                mAudioBufForTsDemux = (char*)pBuf0;
            *ppBuf                  = (unsigned char*)pBuf0;
            *pSize                  = nBufSize0;
            break;
        }
        else
        {
            logd("waiting for v/a %d stream buffer size %d", eMediaType, nRequireSize);

            if(mIsQuitedRequest)
                return 0;

            usleep(10*1000);
        }
    }

    return 0;
}

int CTC_MediaProcessorImpl::UpdateData(int64_t pts, int nDataSize,
                            int bIsFirst, int bIsLast, int MediaType)
{
    MediaStreamDataInfo dataInfo;
    memset(&dataInfo, 0, sizeof(MediaStreamDataInfo));
    enum EMEDIATYPE     eMediaType;
    int                 nStreamIndex = 0;

    eMediaType              = (enum EMEDIATYPE)MediaType;
    dataInfo.pData  = (eMediaType == MEDIA_TYPE_VIDEO)?
                (char*)mVideoBufForTsDemux : (char*)mAudioBufForTsDemux;
    dataInfo.nLength      = nDataSize;
    dataInfo.nPts         = pts;  //* input pts is in 90KHz.
    dataInfo.bIsFirstPart = bIsFirst;
    dataInfo.bIsLastPart  = bIsLast;

#if SAVE_FILE_ES
    FILE *tmp = NULL;
    if(eMediaType == MEDIA_TYPE_VIDEO)
    {
        tmp = file1;
    }
    else
    {
        tmp = file2;
    }
    if (tmp)
    {
        fwrite(dataInfo.pData, 1, dataInfo.nLength, tmp);
        sync();
    }
    else
    {
        CDX_LOGW("save file = NULL");
    }
#endif
    return PlayerSubmitStreamData(mPlayer, &dataInfo, eMediaType, nStreamIndex);
}

int CTC_MediaProcessorImpl::OpenTsDemux(void *mDemux, int nPid, int MediaType, int Format)
{
    int ret = 0;
    demux_filter_param_t filterParam;
    enum EMEDIATYPE    eMediaType;
    vformat_t vFormat;

    eMediaType = (enum EMEDIATYPE)MediaType;
    vFormat    = (vformat_t)Format;

    if (mDemux != NULL)
    {
        if (eMediaType == MEDIA_TYPE_VIDEO)
        {
            filterParam.request_buffer_cb = RequestVideoBufferCallback;
            filterParam.update_data_cb    = UpdateVideoDataCallback;
            if(vFormat == VFORMAT_H264)
                filterParam.codec_type          =  DMX_CODEC_H264;
            else if(vFormat == VFORMAT_H265)
                filterParam.codec_type          =  DMX_CODEC_H265;
            else
                filterParam.codec_type          =  DMX_CODEC_UNKOWN;
        }
        else if (eMediaType == MEDIA_TYPE_AUDIO)
        {
            filterParam.request_buffer_cb = RequestAudioBufferCallback;
            filterParam.update_data_cb      = UpdateAudioDataCallback;
        }
        else
            ret = -1;

        filterParam.cookie = (void*)this;
        ret = ts_demux_open_filter(mDemux, nPid, &filterParam);
    }
    else
    {
        ALOGW("OpenTsDemux Failed");
        ret = -1;
    }

    return ret;
}

int CTC_MediaProcessorImpl::CloseTsDemux(void *mDemux, int nPid)
{
    logd("CloseTsDemux pid %d", nPid);
    int ret = 0;
    if (mDemux != NULL)
    {
        ret = ts_demux_close_filter(mDemux, nPid);
    }else
    {
        ALOGW("CloseTsDemux Failed");
        ret = -1;
    }
    return ret;
}

int CTC_MediaProcessorImpl::SetAudioInfo(PAUDIO_PARA_T pAudioPara)
{
    logd("SetAudioInfo, pid: %d", pAudioPara->pid);
    int    ret = 0;
    AudioStreamInfo streamInfo;

    //* set audio stream info.
    memset(&streamInfo, 0, sizeof(streamInfo));
    streamInfo.eCodecFormat          = (enum EAUDIOCODECFORMAT)audioCodecConvert(pAudioPara->aFmt);
    streamInfo.nChannelNum           = pAudioPara->nChannels;
    streamInfo.nSampleRate           = pAudioPara->nSampleRate;
#if !NewInterface
    streamInfo.nBitsPerSample        = pAudioPara->bit_per_sample;
    streamInfo.nBlockAlign           = pAudioPara->block_align;
#endif
    streamInfo.nCodecSpecificDataLen = pAudioPara->nExtraSize;
    streamInfo.pCodecSpecificData    = (char*)pAudioPara->pExtraData;
    streamInfo.nFlags = (streamInfo.eCodecFormat == AUDIO_CODEC_FORMAT_MPEG_AAC_LC) ? 1 : 0;

    logd("******* audio info *******************");
    logd("audio streamInfo.eCodecFormat(%d)", streamInfo.eCodecFormat);
    logd("streamInfo.nChannelNum(%d)",        streamInfo.nChannelNum);
    logd("streamInfo.nSampleRate(%d)",        streamInfo.nSampleRate);
    logd("streamInfo.nBitsPerSample(%d)",     streamInfo.nBitsPerSample);
    logd("streamInfo.nCodecSpecificDataLen(%d)", streamInfo.nCodecSpecificDataLen);
    logd("************* end *******************");

    memcpy(&mAudioPara, pAudioPara, sizeof(AUDIO_PARA_T));

    ret = PlayerSetAudioStreamInfo(mPlayer, &streamInfo, 1, 0);

    return ret;
}

int CTC_MediaProcessorImpl::SetVideoInfo(PVIDEO_PARA_T pVideoPara)
{
    logd("SetVideoInfo");
    int    ret = 0;
    VideoStreamInfo streamInfo;

    //* set video stream info.
    memset(&streamInfo,0,sizeof(streamInfo));
    streamInfo.eCodecFormat = videoCodecConvert(pVideoPara->vFmt);
    streamInfo.nWidth        = pVideoPara->nVideoWidth;
    streamInfo.nHeight        = pVideoPara->nVideoHeight;
    streamInfo.nFrameRate    = pVideoPara->nFrameRate;

    memcpy(&mVideoPara, pVideoPara, sizeof(VIDEO_PARA_T));

    ret = PlayerSetVideoStreamInfo(mPlayer, &streamInfo);

    logd("*********** video info **********************");
    logd("vFmt:%d",pVideoPara->vFmt);
    logd("nVideoWidth:%d",pVideoPara->nVideoWidth);
    logd("nVideoHeight:%d",pVideoPara->nVideoHeight);
    logd("nFrameRate:%d",pVideoPara->nFrameRate);
    logd("**********************************************");

    return ret;
}

void CTC_MediaProcessorImpl::InitVideo(PVIDEO_PARA_T pVideoPara)
{
    logd("InitVideo, pid: %d", pVideoPara->pid);
    int ret = 0;

    Mutex::Autolock _l(mLock);

    //* ES pid=0xffff, do not open ts demux
    if (pVideoPara->pid != 0xffff)
        ret = OpenTsDemux(mDemux, pVideoPara->pid, (int)MEDIA_TYPE_VIDEO, (int)pVideoPara->vFmt);
    else
        logd("ES Video Stream");

    if (pVideoPara != NULL)
    {
#if (RECONFIG_VIDEO_WHEN_SWITCH_CHANNEL == 0)
        if(pVideoPara->vFmt != mVideoPara.vFmt ||
                pVideoPara->nVideoWidth != mVideoPara.nVideoWidth ||
                pVideoPara->nVideoHeight != mVideoPara.nVideoHeight ||
                pVideoPara->nFrameRate != mVideoPara.nFrameRate)
#endif
            ret = SetVideoInfo(pVideoPara);
    }
    else
    {
        ALOGW("InitVideo Failed");
    }
    mVideoPids = pVideoPara->pid;

    if(pVideoPara->vFmt == VFORMAT_H265)
    {
        isH265 = 1;
        mFirstStartPlay = 0;
        mFirstCache = 0;
    }
    else
    {
        isH265 = 0;
        mFirstStartPlay = 1;
        mFirstCache = 1;
    }

#if DEBUG_WRITE_DATA_SPEED
    mFirst = 1;
#endif

    PlayerConfigDispErrorFrame(mPlayer, 0);

    mHoldLastPicture = true;

    logd("+++ mHoldLastPicture: %d", mHoldLastPicture);

    if(mHoldLastPicture == true)
    {
        int ret = PlayerSetHoldLastPicture(mPlayer, 1);
        if(ret == -1)
        {
            loge("set hold last picture failed!");
        }
    }
    else
    {
        int ret = PlayerSetHoldLastPicture(mPlayer, 0);
        if(ret == -1)
        {
            loge("set hold last picture failed!");
        }
    }

    return;
}

void CTC_MediaProcessorImpl::InitAudio(PAUDIO_PARA_T pAudioPara)
{
    logd("InitAudio, pid: %d", pAudioPara->pid);
    int    ret = 0;

    Mutex::Autolock _l(mLock);

    //* ES pid=0xffff, do not open ts demux
    if (pAudioPara->pid != 0xffff)
        ret = OpenTsDemux(mDemux, pAudioPara->pid, (int)MEDIA_TYPE_AUDIO, (int)0);
    else
        logd("ES Video Stream");

    mAudioTrackCount               = 1;
    mCurrentAudioTrack             = 0;
    mAudioPids[mCurrentAudioTrack] = pAudioPara->pid;

    if (pAudioPara != NULL)
    {
        ret = SetAudioInfo(pAudioPara);
        PlayerStopAudio(mPlayer);
    }
    else
    {
        ALOGW("InitAudio Failed");
    }

    return;
}


bool CTC_MediaProcessorImpl::StartPlay()
{
    int ret;
    logd("StartPlay");

    mIsEos = false;
    mIsQuitedRequest = false;

    Mutex::Autolock _l(mLock);

    ret = PlayerStart(mPlayer,0,0);
    if(ret == 0)
        return true;
    else
        return false;
}

bool CTC_MediaProcessorImpl::Pause()
{
    int ret;

    logd("Pause");

    //mIsQuitedRequest = true;
    Mutex::Autolock _l(mLock);

    ret = PlayerPause(mPlayer);
    if(ret == 0)
        return true;
    else
        return false;
}


bool CTC_MediaProcessorImpl::Resume()
{
    int ret;
    logd("Resume");

    //mIsQuitedRequest = false;
    Mutex::Autolock _l(mLock);

    ret = PlayerStart(mPlayer,0,0);
    if(ret == 0)
        return true;
    else
        return false;
}

void CTC_MediaProcessorImpl::SetSurface(Surface* pSurface)
{
    // this interface is not used now
    logd("SetSurface %p", pSurface);
    UNUSED(pSurface);
    return;
}

bool CTC_MediaProcessorImpl::Fast()
{
    int ret;
    logd("Fast");

    pthread_mutex_lock(&mSeekMutex);
    mSeekRequest = true;
    while(!mSeekReply)
    {
        pthread_cond_wait(&mCond, &mSeekMutex);
    }
    pthread_mutex_unlock(&mSeekMutex);

    Mutex::Autolock _l(mLock);

    if (mPlayer)
        ret = PlayerReset(mPlayer, 0);

    mLeftBytes = 0; //* discard kept ts data.

    pthread_mutex_lock(&mBufMutex);
    mDataWritePtr = mDataBuffer;
    mDataReadPtr = mDataBuffer;
    mValidDataSize = 0;
    pthread_mutex_unlock(&mBufMutex);

    mSeekRequest = false;

    ret = PlayerFast(mPlayer, 1);

    PlayerSetDiscardAudio(mPlayer, 1);
    mLeftBytes = 0; //* discard kept ts data.
    if(ret == 0)
        return true;
    else
        return false;
}

bool CTC_MediaProcessorImpl::StopFast()
{
    int ret;
    logd("StopFast");
    Mutex::Autolock _l(mLock);

    ret = PlayerStopFast(mPlayer);
    PlayerSetDiscardAudio(mPlayer, 0);
    mLeftBytes = 0; //* discard kept ts data.
    if(ret == 0)
        return true;
    else
        return false;
}

bool CTC_MediaProcessorImpl::Stop()
{
    int ret;
    logd("Stop");

    mIsQuitedRequest = true;

    pthread_mutex_lock(&mSeekMutex);
    mSeekRequest = true;
    while(!mSeekReply)
    {
        pthread_cond_wait(&mCond, &mSeekMutex);
    }
    pthread_mutex_unlock(&mSeekMutex);

    Mutex::Autolock _l(mLock);

    //mHoldLastPicture = false;
    //PlayerSetHoldLastPicture(mPlayer, 0);

    ret = PlayerStop(mPlayer);
    ret = PlayerClear(mPlayer);

    memset(&mVideoPara, 0, sizeof(mVideoPara));
    memset(&mAudioPara, 0, sizeof(mAudioPara));

    ret = CloseTsDemux(mDemux, mAudioPids[mCurrentAudioTrack]);
    ret = CloseTsDemux(mDemux, mVideoPids);
    mVideoBufForTsDemux = NULL;
    mAudioBufForTsDemux = NULL;
    logd("Stop end");
    mLeftBytes = 0;

    pthread_mutex_lock(&mBufMutex);
    mDataWritePtr = mDataBuffer;
    mDataReadPtr = mDataBuffer;
    mValidDataSize = 0;
    pthread_mutex_unlock(&mBufMutex);

    mSeekRequest = false;
    mIsQuitedRequest = false;

    if(ret == 0)
        return true;
    else
        return false;
}

bool CTC_MediaProcessorImpl::Seek()
{
    int ret;
    logd("Seek");

    pthread_mutex_lock(&mSeekMutex);
    mSeekRequest = true;
    while(!mSeekReply)
    {
        pthread_cond_wait(&mCond, &mSeekMutex);
    }
    pthread_mutex_unlock(&mSeekMutex);

    Mutex::Autolock _l(mLock);

    if (mPlayer)
    {
        ret = PlayerPause(mPlayer);
        ret = PlayerReset(mPlayer, 0);
        PlayerStopAudio(mPlayer);
    }
    else
        ret = -1;

    mLeftBytes = 0; //* discard kept ts data.

    pthread_mutex_lock(&mBufMutex);
    mDataWritePtr = mDataBuffer;
    mDataReadPtr = mDataBuffer;
    mValidDataSize = 0;
    pthread_mutex_unlock(&mBufMutex);

    ret = PlayerStart(mPlayer,0,0);

    mSeekRequest = false;

    if(ret == 0)
        return true;
    else
        return false;
}


static int PlayerBufferOverflow(Player* p)
{
    int bVideoOverflow;
    int bAudioOverflow;

    int     nPictureNum;
    int     nFrameDuration;
    int     nPcmDataSize;
    int     nSampleRate;
    int     nChannelCount;
    int     nBitsPerSample;
    int     nStreamDataSize;
    int     nBitrate;
    int     nStreamBufferSize;
    int64_t nVideoCacheTime;
    int64_t nAudioCacheTime;

    bVideoOverflow = 1;
    bAudioOverflow = 1;

    if(PlayerHasVideo(p))
    {
        nPictureNum       = PlayerGetValidPictureNum(p);
        nFrameDuration    = PlayerGetVideoFrameDuration(p);
        nStreamDataSize   = PlayerGetVideoStreamDataSize(p);
        nBitrate          = PlayerGetVideoBitrate(p);
        nStreamBufferSize = PlayerGetVideoStreamBufferSize(p);

        nVideoCacheTime = nPictureNum*nFrameDuration;

        if(nBitrate > 0)
            nVideoCacheTime += ((int64_t)nStreamDataSize)*8*1000*1000/nBitrate;

        if(nVideoCacheTime <= 4000000 || nStreamDataSize<nStreamBufferSize/2)
            bVideoOverflow = 0;

        //logi("picNum = %d, frameDuration = %d, dataSize = %d, \
        //            bitrate = %d, bVideoOverflow = %d",
        //    nPictureNum, nFrameDuration, nStreamDataSize, nBitrate, bVideoOverflow);
    }

    if(PlayerHasAudio(p))
    {
        nPcmDataSize    = PlayerGetAudioPcmDataSize(p);
        nStreamDataSize = PlayerGetAudioStreamDataSize(p);
        nBitrate        = PlayerGetAudioBitrate(p);
        PlayerGetAudioParam(p, &nSampleRate, &nChannelCount, &nBitsPerSample);

        nAudioCacheTime = 0;

        if(nSampleRate != 0 && nChannelCount != 0 && nBitsPerSample != 0)
        {
            nAudioCacheTime +=
                ((int64_t)nPcmDataSize)*8*1000*1000/(nSampleRate*nChannelCount*nBitsPerSample);
        }

        if(nBitrate > 0)
            nAudioCacheTime += ((int64_t)nStreamDataSize)*8*1000*1000/nBitrate;

        if(nAudioCacheTime <= 2000000)   //* cache more than 2 seconds of data.
            bAudioOverflow = 0;

        //logi("nPcmDataSize = %d, nStreamDataSize = %d, \
        //        nBitrate = %d, nAudioCacheTime = %lld, bAudioOverflow = %d",
        //    nPcmDataSize, nStreamDataSize, nBitrate, nAudioCacheTime, bAudioOverflow);
    }

    return bVideoOverflow && bAudioOverflow;
}

static int PlayerBufferUnderflow(Player* p)
{
    int bVideoUnderflow;
    int bAudioUnderFlow;

    bVideoUnderflow = 0;
    bAudioUnderFlow = 0;

    if(PlayerHasVideo(p))
    {
        int nPictureNum;
        int nStreamFrameNum;

        nPictureNum = PlayerGetValidPictureNum(p);
        nStreamFrameNum = PlayerGetVideoStreamFrameNum(p);
        if(nPictureNum + nStreamFrameNum < 15)
            bVideoUnderflow = 1;

        logd("nPictureNum = %d, nStreamFrameNum = %d, bVideoUnderflow = %d",
            nPictureNum, nStreamFrameNum, bVideoUnderflow);
    }

    if(PlayerHasAudio(p))
    {
        int nStreamDataSize;
        int nPcmDataSize;
        int nCacheTime;

        nStreamDataSize = PlayerGetAudioStreamDataSize(p);
        nPcmDataSize    = PlayerGetAudioPcmDataSize(p);
        nCacheTime      = 0;
        if(nCacheTime == 0 && (nPcmDataSize + nStreamDataSize < 8000))
            bAudioUnderFlow = 1;

        logd("nStreamDataSize = %d, nPcmDataSize = %d, nCacheTime = %d, bAudioUnderFlow = %d",
            nStreamDataSize, nPcmDataSize, nCacheTime, bAudioUnderFlow);
    }

    return bVideoUnderflow | bAudioUnderFlow;
}


void* CTC_MediaProcessorImpl::WriteDataThread(void* pArg)
{
    unsigned char* pBuffer;
    unsigned int   nSize;
    unsigned int   tsReadSize;
    unsigned int   leftSize;
    int            bufEndFlag = 0;
    int64_t        nowTime = 0;

    VideoStreamInfo videoStreamInfo;
    int ret;
    int findWH = 0;
    CTC_MediaProcessorImpl *impl = (CTC_MediaProcessorImpl *)pArg;
    while(1)
    {
        if(impl->mIsQuited)
            return NULL;

        if(impl->mSeekRequest)
        {
            pthread_mutex_lock(&impl->mSeekMutex);
            impl->mSeekReply = 1;
            pthread_cond_signal(&impl->mCond);
            pthread_mutex_unlock(&impl->mSeekMutex);

            usleep(10*1000); // wait playerReset and FlushCacheStream
            continue;
        }
        impl->mSeekReply = 0;

        if(impl->mFirstCache == 0)
        {
            impl->mStartCacheTime = GetNowUs();
            findWH = 0;
            impl->mFirstCache = 1;
        }

        if(PlayerBufferOverflow(impl->mPlayer) || impl->mValidDataSize<= TS_READ_SIZE)
        {
            usleep(10*1000);
            continue;
        }

        if(impl->mFirstStartPlay == 0 && impl->isH265)
        {
            if(findWH == 0)
            {
                ret = ProbeVideoSpecificData(&videoStreamInfo, impl->mDataBuffer,
                            impl->mValidDataSize, VIDEO_CODEC_FORMAT_H265, CDX_PARSER_TS);
                if(ret == PROBE_SPECIFIC_DATA_SUCCESS)
                {
                    findWH = 1;
                    if(videoStreamInfo.nWidth < 3840 || videoStreamInfo.nHeight < 2160)
                    {
                        impl->mFirstStartPlay = 1;
                    }
                    logd("++++ find width(%d), height(%d)",
                            videoStreamInfo.nWidth, videoStreamInfo.nHeight);
                }
            }

            nowTime = GetNowUs();
            if((impl->mValidDataSize <= 4*1024*1024) &&
                    (nowTime - impl->mStartCacheTime < 1500*1000))
            {
                usleep(10*1000);
                continue;
            }
            else
            {
                impl->mFirstStartPlay = 1;
                logd("startplay, impl->mValidDataSize: %d, diff: %lld",
                        impl->mValidDataSize, nowTime - impl->mStartCacheTime);
            }
        }

        if(impl->mDataReadPtr <= impl->mDataWritePtr)
        {
            leftSize = impl->mDataWritePtr - impl->mDataReadPtr;
            //nSize    = (leftSize > TS_READ_SIZE)? TS_READ_SIZE : leftSize;
            nSize = leftSize;
            tsReadSize = nSize;
        }
        else
        {
            leftSize = impl->mDataBuffer + impl->mDataBufferSize - impl->mDataReadPtr;
            //nSize    = (leftSize > TS_READ_SIZE)? TS_READ_SIZE : leftSize;
            nSize = leftSize;
            tsReadSize = nSize;
            bufEndFlag = 1;
        }

        if(impl->mTsStreamBufSize)
        {
            pBuffer = (unsigned char*)impl->mTsStreamBuf;
            tsReadSize = impl->mTsStreamBufSize;
            nSize = tsReadSize - impl->mLeftBytes;
            logd("++++++ impl->mTsStreamBufSize: %d, nSize: %d", impl->mTsStreamBufSize, nSize);
            //impl->mTsStreamBufSize = 0;
        }
        else
        {
            pBuffer   = impl->mDataReadPtr;
        }

        impl->mLeftBytes = ts_demux_handle_packets(impl->mDemux,
                        (unsigned char*)pBuffer, tsReadSize);
        if(impl->mLeftBytes < 0)
        {
            loge("mLeftBytes(%d)", impl->mLeftBytes);
            impl->mLeftBytes = 0;
            //return NULL;
        }
        else if(impl->mLeftBytes > 0)
        {
            logw("mLeftBytes(%d), impl->mDataReadPtr: %p, impl->mDataBUffer: %p",
                impl->mLeftBytes, impl->mDataReadPtr, impl->mDataBuffer);

            if(impl->mTsStreamBufSize)
            {
                nSize -= impl->mLeftBytes;
                impl->mTsStreamBufSize = 0;
                logd("=== nSize: %d", nSize);
            }

            if(impl->mLeftBytes < 205 && bufEndFlag)
            {
                memcpy(impl->mTsStreamBuf, impl->mDataReadPtr+nSize-impl->mLeftBytes,
                                impl->mLeftBytes);
                memcpy((unsigned char*)impl->mTsStreamBuf+impl->mLeftBytes,
                        impl->mDataBuffer, PACKET_SIZE-impl->mLeftBytes);

                impl->mTsStreamBufSize = PACKET_SIZE;

                bufEndFlag = 0;
            }
        }

        pthread_mutex_lock(&impl->mBufMutex);
        impl->mValidDataSize -= nSize;
        impl->mDataReadPtr += nSize;
        if(impl->mDataReadPtr >= (impl->mDataBuffer+impl->mDataBufferSize))
        {
            impl->mDataReadPtr -= impl->mDataBufferSize;
        }
        pthread_mutex_unlock(&impl->mBufMutex);
    }

    return NULL;
}

static int64_t totalTime=0;
int CTC_MediaProcessorImpl::WriteData(unsigned char* pBufferIn, unsigned int nSizeIn)
{
    //logd("WriteData, pBuffer(%p), nSize(%d)", pBufferIn, nSizeIn);
    //SetVideoWindow(mVideoX, mVideoY, mVideoWidth, mVideoHeight);
    Mutex::Autolock _l(mLock);
    unsigned int size1 = nSizeIn;
    unsigned int leftBufSize;

#if    DEBUG_WRITE_DATA_SPEED
    int64_t rate = 0;
    int64_t diff = 0;

    if(mFirst)
    {
        mTotalSize = 0;
        mNowtime = GetNowUs();
        mPrintTime = 0;
        mFirst = 0;
    }
    else
    {
        mTotalSize += nSizeIn;
        diff = (GetNowUs() - mNowtime);
        rate = mTotalSize *1000000*8 / diff;
    }

    if(diff > mPrintTime)
    {
        mPrintTime += 1000000;
        logd("++++ writeData bitrate: %lld, totalSize: %lld, diff: %lld", rate, mTotalSize, diff);
    }
#endif

    //logd("++++ mValidDataSize: %d, totalSize: %d", mValidDataSize, mDataBufferSize);
    if(mValidDataSize+nSizeIn >= mDataBufferSize)
    {
        return -1;
    }

    if(mDataReadPtr > mDataWritePtr)
    {
        memcpy(mDataWritePtr, pBufferIn, nSizeIn);
    }
    else
    {
        leftBufSize = mDataBuffer+mDataBufferSize - mDataWritePtr;
        if(leftBufSize >= nSizeIn)
        {
            memcpy(mDataWritePtr, pBufferIn, nSizeIn);
        }
        else
        {
            memcpy(mDataWritePtr, pBufferIn, leftBufSize);
            memcpy(mDataBuffer, pBufferIn+leftBufSize, nSizeIn-leftBufSize);
        }
    }

    pthread_mutex_lock(&mBufMutex);
    mValidDataSize += nSizeIn;
    mDataWritePtr += nSizeIn;
    if(mDataWritePtr >= (mDataBuffer+mDataBufferSize))
    {
        mDataWritePtr -= mDataBufferSize;
    }
    pthread_mutex_unlock(&mBufMutex);

#if SAVE_FILE_TS
    if(file)
    {
        fwrite(pBufferIn, 1, nSizeIn, file);
        sync();
    }
#endif

    //logd("+++ total time: %lld, diff: %lld", totalTime, end-start);
    return size1;
}


#if NewInterface
void CTC_MediaProcessorImpl::SwitchAudioTrack(int pid)
{
    logd("SwitchAudioTrack, pid %d", pid);
    int ret = 0;
    Mutex::Autolock _l(mLock);
    if (mPlayer != NULL)
    {
        if(mAudioPids[mCurrentAudioTrack] == pid)
        {
            logd("mAudioPids[mCurrentAudioTrack] == pid");
            return;
        }
        logd("SwitchAudio, pid:(%d)->(%d)", mAudioPids[mCurrentAudioTrack], pid);
#if SwitchAudioSeamless
        PlayerStopAudio(mPlayer);
#endif
        mIsQuitedRequest  = true;
        ret = CloseTsDemux(mDemux, mAudioPids[mCurrentAudioTrack]);
        mIsQuitedRequest  = false;
        mAudioPids[mCurrentAudioTrack] = pid;
        ret = OpenTsDemux(mDemux, pid, (int)MEDIA_TYPE_AUDIO, (int)0);

#if SwitchAudioSeamless
        PlayerStartAudio(mPlayer);
#endif
    }
    else
    {
        ALOGW("mPlayer Not Initialized");
    }
    return ;
}
#else
void CTC_MediaProcessorImpl::SwitchAudioTrack(int pid, PAUDIO_PARA_T pAudioPara)
{
    logd("SwitchAudioTrack");
    int ret = 0;
    Mutex::Autolock _l(mLock);

    if (mPlayer != NULL)
    {
        if (mStreamType == IPTV_PLAYER_STREAMTYPE_TS) //* TS Stream
        {
            mIsQuitedRequest  = true;
            ret = CloseTsDemux(mDemux, mAudioPids[mCurrentAudioTrack]);
            mIsQuitedRequest  = false;
            mAudioPids[mCurrentAudioTrack] = pAudioPara->pid = pid;
            ret = OpenTsDemux(mDemux, pAudioPara->pid, (int)MEDIA_TYPE_AUDIO, (int)0);
        }
        else
        {
            //* for temp
            ret = PlayerPause(mPlayer);
            if (ret != 0)
                goto EXIT;
            SetAudioInfo(pAudioPara);
            ret = PlayerStart(mPlayer,0,0);
            if (ret != 0)
                goto EXIT;

        }
    }else
    {
        ALOGW("mPlayer Not Initialized");
    }
EXIT:
    return ;
}
#endif

void CTC_MediaProcessorImpl::SwitchSubtitle(int pid)
{
    logd("SwitchSubtitle");
    UNUSED(pid);
    return ;
}

#if !NewInterface
bool CTC_MediaProcessorImpl::GetIsEos()
{
    logd("GetIsEos");
    Mutex::Autolock _l(mLock);
    //TO DO
    return mIsEos;
}
int CTC_MediaProcessorImpl::GetBufferStatus(long *totalsize, long *datasize)
{
    Mutex::Autolock _l(mLock);
    return 0;
}

void CTC_MediaProcessorImpl::SetStopMode(bool bHoldLastPic)
{
    Mutex::Autolock _l(mLock);

    logd("SetStopMode, bHoldLastPic=%d", bHoldLastPic);
    mHoldLastPicture = bHoldLastPic;
    PlayerSetHoldLastPicture(mPlayer, mHoldLastPicture);
    return;
}
#endif

int    CTC_MediaProcessorImpl::GetPlayMode()
{
    logd("GetPlayMode");
#if NewInterface
    return 1;
#else
    enum EPLAYERSTATUS status = PlayerGetStatus(mPlayer);
    IPTV_PLAYER_STATE_e ret = IPTV_PLAYER_STATE_OTHER;

    Mutex::Autolock _l(mLock);

    switch(status)
    {
        case PLAYER_STATUS_STOPPED:
            ret = IPTV_PLAYER_STATE_STOP;
            break;
        case PLAYER_STATUS_STARTED:
            ret = IPTV_PLAYER_STATE_PLAY;
            break;
        case PLAYER_STATUS_PAUSED:
            ret = IPTV_PLAYER_STATE_PAUSE;
            break;
        default:
            loge("unkonw player status!");
            break;
    }
    return ret;
#endif
}

long CTC_MediaProcessorImpl::GetCurrentPlayTime()
{
    int64_t nCurTime;
    Mutex::Autolock _l(mLock);

    nCurTime = PlayerGetPosition(mPlayer)/1000;       //* current presentation time stamp.
    logd("GetCurrentPlayTime, nCurTime(%lld)", nCurTime);
    return (long)nCurTime;
}

void CTC_MediaProcessorImpl::GetVideoPixels(int& width, int& height)
{
    Mutex::Autolock _l(mLock);
    sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain));
    DisplayInfo info;
    SurfaceComposerClient::getDisplayInfo(display, &info);
    width = info.w;
    height = info.h;
    logd("GetVideoPixels, width = %d, height = %d", width, height);
    return ;
}

int CTC_MediaProcessorImpl::SetVideoWindow(int x,int y,int width,int height)
{
    Mutex::Autolock _l(mLock);
    logd("SetVideoWindow, x(%d),y(%d),width(%d), height(%d)", x,y,width, height);

    if(videoSfControl.get() != NULL)
    {
        SurfaceComposerClient::openGlobalTransaction();
        videoSfControl->setSize(width, height);
        videoSfControl->setPosition(x, y);
        SurfaceComposerClient::closeGlobalTransaction();
    }

    return 0;
}

#if NewInterface
bool CTC_MediaProcessorImpl::SetRatio(int nRatio)
{
    logd("SetRadioMode %d, ignore", nRatio);
    return 0;
    Mutex::Autolock _l(mLock);
#if (!((CONF_ANDROID_MAJOR_VER == 4)&&(CONF_ANDROID_SUB_VER == 2)))
    mNativeWindow->perform(mNativeWindow.get(),
                        NATIVE_WINDOW_SETPARAMETER,
                        DISPLAY_CMD_SETSCREENRADIO,
                        nRatio);
#endif
    return 0;
}
#else
void CTC_MediaProcessorImpl::SetContentMode(IPTV_PLAYER_CONTENTMODE_e contentMode)
{
    logd("SetContentMode");
    enum EDISPLAYRATIO eDisplayRatio;

    Mutex::Autolock _l(mLock);

    switch(contentMode)
    {
        case IPTV_PLAYER_CONTENTMODE_LETTERBOX:
            eDisplayRatio = DISPLAY_RATIO_LETTERBOX;
            break;
        default:
            eDisplayRatio = DISPLAY_RATIO_FULL_SCREEN;
            break;
    }

    PlayerSetDisplayRatio(mPlayer, eDisplayRatio);
}
#endif

int CTC_MediaProcessorImpl::VideoShow()
{
    logd("VideoShow");

    //Mutex::Autolock _l(mLock);

    if(videoSfControl.get() != NULL)
    {
        SurfaceComposerClient::openGlobalTransaction();
        videoSfControl->show();
        SurfaceComposerClient::closeGlobalTransaction();
    }

    return 0;
}

int CTC_MediaProcessorImpl::VideoHide()
{
    logd("VideoHide");
    //Mutex::Autolock _l(mLock);

    if(videoSfControl.get() != NULL)
    {
        SurfaceComposerClient::openGlobalTransaction();
        videoSfControl->hide();
        SurfaceComposerClient::closeGlobalTransaction();
    }

    if(mNativeWindow != NULL && mNativeWindow.get() != NULL)
    {
#if defined(CONF_SEND_BLACK_FRAME_TO_GPU)
        SendThreeBlackFrameToGpu(mNativeWindow.get());
#endif
    }

    return 0;
}

int CTC_MediaProcessorImpl::GetAudioBalance()
{
    logd("GetAudioBalance");
    Mutex::Autolock _l(mLock);

    return PlayerGetAudioBalance(mPlayer);
}

bool CTC_MediaProcessorImpl::SetAudioBalance(int nAudioBalance)
{
    logd("SetAudioBalance");
    Mutex::Autolock _l(mLock);

    if(PlayerSetAudioBalance(mPlayer, nAudioBalance) == 0)
        return true;
    else
        return false;
}

void CTC_MediaProcessorImpl::playerback_register_evt_cb(IPTV_PLAYER_EVT_CB pfunc, void *handler)
{
    logd("playback_register_evt_cb");
    m_callBackFunc = pfunc;
    m_eventHandler = handler;
    return;
}

void CTC_MediaProcessorImpl::SetProperty(int nType, int nSub, int nValue)
{
    UNUSED(nType);
    UNUSED(nSub);
    UNUSED(nValue);
    return;
}

void CTC_MediaProcessorImpl::leaveChannel()
{
    logd("leaveChannel");

    mIsQuitedRequest = true;

    pthread_mutex_lock(&mSeekMutex);
    mSeekRequest = true;
    while(!mSeekReply)
    {
        pthread_cond_wait(&mCond, &mSeekMutex);
    }
    pthread_mutex_unlock(&mSeekMutex);

    Mutex::Autolock _l(mLock);

    PlayerPause(mPlayer);
    PlayerReset(mPlayer, 0);
    PlayerStopAudio(mPlayer);

    CloseTsDemux(mDemux, mAudioPids[mCurrentAudioTrack]);
    CloseTsDemux(mDemux, mVideoPids);
    mVideoBufForTsDemux = NULL;
    mAudioBufForTsDemux = NULL;

    mLeftBytes = 0;

#if USE_LIST_CACHE
    if(mCache)
        IptvStreamCacheFlushAll(mCache);
#endif

    pthread_mutex_lock(&mBufMutex);
    mDataWritePtr = mDataBuffer;
    mDataReadPtr = mDataBuffer;
    mValidDataSize = 0;
    pthread_mutex_unlock(&mBufMutex);

    mSeekRequest = false;
    mIsQuitedRequest = false;

    return;
}

int  CTC_MediaProcessorImpl::GetAbendNum()
{
    return 0;
}
bool CTC_MediaProcessorImpl::IsSoftFit()
{
    return true;
}
void CTC_MediaProcessorImpl::SetEPGSize(int w, int h)
{
    UNUSED(w);
    UNUSED(h);
    return;
}

/* index: 0-100 */
int CTC_MediaProcessorImpl::setVolumeJNI(int index)
{
    JNIEnv* env = NULL;
    JavaVM* vm = AndroidRuntime::getJavaVM();
    int bAttach = false;

    if(vm)
    {
        int status = vm->GetEnv((void**)&env, JNI_VERSION_1_4);
        if(status < 0)
        {
            logd("---- vm GetEnv fail.");
            status = vm->AttachCurrentThread(&env, NULL);
            if(status < 0)
            {
                logd("---- vm AttachCurrentThread fail, env: %p", env);
                env = NULL;
            }
            else
                bAttach = true;
        }
    }

    logd("+++++ env(%p), vm(%p)", env, vm);
    if (env) {
        jclass clazz = env->FindClass("android/media/AudioManager");
        jmethodID setVolume = env->GetStaticMethodID(clazz, "setIPTVVolume", "(III)V");
        logd("clazz=%p, method=%p", clazz, setVolume);
        if (clazz && setVolume)
            env->CallStaticVoidMethod(clazz, setVolume, AUDIO_STREAM_MUSIC, index, 0);
        if(bAttach)
            vm->DetachCurrentThread();
        return 0;
    }
    return -1;
}

bool CTC_MediaProcessorImpl::SetVolume(int volume)
{
    logd("SetVolume, gVolume = %d, volume=%d", gVolume, volume);
    gVolume = volume;

    int index = volume;
    int ret = setVolumeJNI(index);

    logd("SetVolume, ret = %d", ret);
    return ret == 0? true : false;
}

int CTC_MediaProcessorImpl::getVolumeJNI()
{
    JNIEnv* env = NULL;
    JavaVM* vm = AndroidRuntime::getJavaVM();
    int bAttach = false;

    if(vm)
    {
        int status = vm->GetEnv((void**)&env, JNI_VERSION_1_4);
        if(status < 0)
        {
            logd("---- vm GetEnv fail.");
            status = vm->AttachCurrentThread(&env, NULL);
            if(status < 0)
            {
                logd("---- vm AttachCurrentThread fail, env: %p", env);
                env = NULL;
            }
            else
                bAttach = true;
        }
    }

    logd("+++++ env(%p), vm(%p)", env, vm);
    int index = -1;
    if (env) {
        jclass clazz = env->FindClass("android/media/AudioManager");
        jmethodID getVolume = env->GetStaticMethodID(clazz, "getIPTVVolume", "()I");
        logd("clazz=%p, method=%p", clazz, getVolume);
        if (clazz && getVolume)
            index = env->CallStaticIntMethod(clazz, getVolume);
        if(bAttach)
            vm->DetachCurrentThread();
    }
    return index;
}

int CTC_MediaProcessorImpl::GetVolume()
{
    logd("GetVolume, gVolume = %d", gVolume);
    int volume = gVolume;
    int index = getVolumeJNI();
    if(index >= 0)
        volume = index;
    logd("GetVolume, volume = %d", volume);
    return volume;
}
