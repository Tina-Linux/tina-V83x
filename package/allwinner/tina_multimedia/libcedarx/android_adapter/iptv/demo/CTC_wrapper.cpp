/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CTC_wrapper.cpp
 * Description : CTC_wrapper
 * History :
 *
 */


#include "cdx_log.h"
//#include "memoryAdapter.h"
#include <string.h>

#include <binder/IPCThreadState.h>
#include <media/IAudioFlinger.h>
#include <fcntl.h>

#include <version.h>
#include <cutils/properties.h> // for property_set

#include <CTC_wrapper.h>

#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>

//* player status.
static const int AWPLAYER_STATUS_IDLE        = 0;
static const int AWPLAYER_STATUS_INITIALIZED = 1<<0;
static const int AWPLAYER_STATUS_PREPARING   = 1<<1;
static const int AWPLAYER_STATUS_PREPARED    = 1<<2;
static const int AWPLAYER_STATUS_STARTED     = 1<<3;
static const int AWPLAYER_STATUS_PAUSED      = 1<<4;
static const int AWPLAYER_STATUS_STOPPED     = 1<<5;
static const int AWPLAYER_STATUS_COMPLETE    = 1<<6;
static const int AWPLAYER_STATUS_ERROR       = 1<<7;

//* callback message id.
static const int AWPLAYER_MESSAGE_DEMUX_PREPARED                = 0x101;
static const int AWPLAYER_MESSAGE_DEMUX_EOS                     = 0x102;
static const int AWPLAYER_MESSAGE_DEMUX_IOERROR                 = 0x103;
static const int AWPLAYER_MESSAGE_DEMUX_SEEK_FINISH             = 0x104;
static const int AWPLAYER_MESSAGE_DEMUX_CACHE_REPORT            = 0x105;
static const int AWPLAYER_MESSAGE_DEMUX_BUFFER_START            = 0x106;
static const int AWPLAYER_MESSAGE_DEMUX_BUFFER_END              = 0x107;
static const int AWPLAYER_MESSAGE_DEMUX_PAUSE_PLAYER            = 0x108;
static const int AWPLAYER_MESSAGE_DEMUX_RESUME_PLAYER           = 0x109;
static const int AWPLAYER_MESSAGE_DEMUX_IOREQ_ACCESS            = 0x10a;
static const int AWPLAYER_MESSAGE_DEMUX_IOREQ_OPEN              = 0x10b;
static const int AWPLAYER_MESSAGE_DEMUX_IOREQ_OPENDIR           = 0x10c;
static const int AWPLAYER_MESSAGE_DEMUX_IOREQ_READDIR           = 0x10d;
static const int AWPLAYER_MESSAGE_DEMUX_IOREQ_CLOSEDIR          = 0x10e;
static const int AWPLAYER_MESSAGE_DEMUX_VIDEO_STREAM_CHANGE        = 0x10f;
static const int AWPLAYER_MESSAGE_DEMUX_AUDIO_STREAM_CHANGE        = 0x110;

static const int AWPLAYER_MESSAGE_PLAYER_EOS                    = 0x201;
static const int AWPLAYER_MESSAGE_PLAYER_FIRST_PICTURE          = 0x202;
static const int AWPLAYER_MESSAGE_PLAYER_SUBTITLE_AVAILABLE     = 0x203;
static const int AWPLAYER_MESSAGE_PLAYER_SUBTITLE_EXPIRED       = 0x204;
static const int AWPLAYER_MESSAGE_PLAYER_VIDEO_SIZE             = 0x205;
static const int AWPLAYER_MESSAGE_PLAYER_VIDEO_CROP             = 0x206;
static const int AWPLAYER_MESSAGE_PLAYER_VIDEO_UNSUPPORTED      = 0x207;
static const int AWPLAYER_MESSAGE_PLAYER_AUDIO_UNSUPPORTED      = 0x208;
static const int AWPLAYER_MESSAGE_PLAYER_SUBTITLE_UNSUPPORTED   = 0x209;
static const int AWPLAYER_MESSAGE_PLAYER_AUDIORAWPLAY           = 0x20a;
static const int AWPLAYER_MESSAGE_PLAYER_SET_SECURE_BUFFER_COUNT= 0x20b;
static const int AWPLAYER_MESSAGE_PLAYER_SET_SECURE_BUFFERS     = 0x20c;
static const int AWPLAYER_MESSAGE_PLAYER_SET_AUDIO_INFO         = 0x20d;

//* command id.
static const int AWPLAYER_COMMAND_SET_SOURCE    = 0x101;
static const int AWPLAYER_COMMAND_SET_SURFACE   = 0x102;
static const int AWPLAYER_COMMAND_SET_AUDIOSINK = 0x103;
static const int AWPLAYER_COMMAND_PREPARE       = 0x104;
static const int AWPLAYER_COMMAND_START         = 0x105;
static const int AWPLAYER_COMMAND_STOP          = 0x106;
static const int AWPLAYER_COMMAND_PAUSE         = 0x107;
static const int AWPLAYER_COMMAND_RESET         = 0x108;
static const int AWPLAYER_COMMAND_QUIT          = 0x109;
static const int AWPLAYER_COMMAND_SEEK          = 0x10a;
static const int AWPLAYER_COMMAND_READ            = 0x10b;
static const int AWPLAYER_COMMAND_HIDE            = 0x10c;
static const int AWPLAYER_COMMAND_FAST          = 0x10d;
static const int AWPLAYER_COMMAND_STOPFAST      = 0x10e;
static const int AWPLAYER_COMMAND_RESUME        = 0x10f;

#define SOURCE_TYPE_URL           0x1

static void* CTC_WrapperThread(void* arg);
static vformat_t videoCodecConvert_1(enum EVIDEOCODECFORMAT codec)
{
    ALOGV("videoCodecConvert");
    vformat_t ret = VFORMAT_UNKNOWN;
    switch(codec)
    {
        case VIDEO_CODEC_FORMAT_MPEG2:
            ret = VFORMAT_MPEG12;
            break;
        case VIDEO_CODEC_FORMAT_MPEG4:
            ret = VFORMAT_MPEG4;
            break;
        case VIDEO_CODEC_FORMAT_H264:
            ret = VFORMAT_H264;
            break;
        case VIDEO_CODEC_FORMAT_MJPEG:
            ret = VFORMAT_MJPEG;
            break;
        case VIDEO_CODEC_FORMAT_RX:
            ret = VFORMAT_REAL;
            break;
        case VIDEO_CODEC_FORMAT_WMV3:
            ret = VFORMAT_VC1;
            break;
        case VIDEO_CODEC_FORMAT_H265:
            ret = VFORMAT_H265;
            break;
        case VIDEO_CODEC_FORMAT_DIVX4:
            ret = VFORMAT_MPEG4;  // we should goto mpeg4Normal decoder
            break;
        default:
            ALOGE("unsupported video codec format!");
            break;
    }

    return ret;
}

static aformat_t audioCodecConvert_1(enum EAUDIOCODECFORMAT codec)
{
    ALOGV("audioCodecConvert codec %d", codec);
    aformat_t ret = AFORMAT_UNKNOWN;
    switch(codec)
    {
        case AUDIO_CODEC_FORMAT_MP2:
            ret = AFORMAT_MPEG;
            break;
        case AUDIO_CODEC_FORMAT_PCM:
            ret = AFORMAT_ADPCM;
            break;
        case AUDIO_CODEC_FORMAT_MPEG_AAC_LC:
            ret = AFORMAT_AAC;
            break;
        case AUDIO_CODEC_FORMAT_AC3:
            ret = AFORMAT_AC3;
            break;
        case AUDIO_CODEC_FORMAT_DTS:
            ret = AFORMAT_DTS;
            break;
        case AUDIO_CODEC_FORMAT_FLAC:
            ret = AFORMAT_FLAC;
            break;
        case AUDIO_CODEC_FORMAT_COOK:
            ret = AFORMAT_COOK;
            break;
        case AUDIO_CODEC_FORMAT_AMR:
            ret = AFORMAT_AMR;
            break;
        case AUDIO_CODEC_FORMAT_WMA_STANDARD:
            ret = AFORMAT_WMA;
            break;
        case AUDIO_CODEC_FORMAT_OGG:
            ret = AFORMAT_VORBIS;
            break;
        case AUDIO_CODEC_FORMAT_MP3:
            ret = AFORMAT_MPEG;
            break;
        default:
            ALOGE("unsupported audio codec format(%d)!", codec);
            break;
    }
    return ret;
}

void IPTV_allbackProcess(IPTV_PLAYER_EVT_e evt, void *handler,
                            unsigned long param1, unsigned long param2)
{
    CTC_Wrapper *me = (CTC_Wrapper *)handler;
    logd("IPTV_allbackProcess evt(%d)", evt);
}
static void clearDataSourceFields(CdxDataSourceT* source)
{
    CdxHttpHeaderFieldsT* pHttpHeaders;
    int                   i;
    int                   nHeaderSize;

    if(source->uri != NULL)
    {
        free(source->uri);
        source->uri = NULL;
    }

    if(source->extraDataType == EXTRA_DATA_HTTP_HEADER &&
       source->extraData != NULL)
    {
        pHttpHeaders = (CdxHttpHeaderFieldsT*)source->extraData;
        nHeaderSize  = pHttpHeaders->num;

        for(i=0; i<nHeaderSize; i++)
        {
            if(pHttpHeaders->pHttpHeader[i].key != NULL)
                free((void*)pHttpHeaders->pHttpHeader[i].key);
            if(pHttpHeaders->pHttpHeader[i].val != NULL)
                free((void*)pHttpHeaders->pHttpHeader[i].val);
        }

        free(pHttpHeaders->pHttpHeader);
        free(pHttpHeaders);
        source->extraData = NULL;
        source->extraDataType = EXTRA_DATA_UNKNOWN;
    }

    return;
}
static void clearMediaInfo(MediaInfo* pMediaInfo)
{
    int                 i;
    VideoStreamInfo*    pVideoStreamInfo;
    AudioStreamInfo*    pAudioStreamInfo;

    if(pMediaInfo->nVideoStreamNum > 0)
    {
        for(i=0; i<pMediaInfo->nVideoStreamNum; i++)
        {
            pVideoStreamInfo = &pMediaInfo->pVideoStreamInfo[i];
            if(pVideoStreamInfo->pCodecSpecificData != NULL &&
               pVideoStreamInfo->nCodecSpecificDataLen > 0)
            {
                free(pVideoStreamInfo->pCodecSpecificData);
                pVideoStreamInfo->pCodecSpecificData = NULL;
                pVideoStreamInfo->nCodecSpecificDataLen = 0;
            }
        }

        free(pMediaInfo->pVideoStreamInfo);
        pMediaInfo->pVideoStreamInfo = NULL;
        pMediaInfo->nVideoStreamNum = 0;
    }

    if(pMediaInfo->nAudioStreamNum > 0)
    {
        for(i=0; i<pMediaInfo->nAudioStreamNum; i++)
        {
            pAudioStreamInfo = &pMediaInfo->pAudioStreamInfo[i];
            if(pAudioStreamInfo->pCodecSpecificData != NULL &&
               pAudioStreamInfo->nCodecSpecificDataLen > 0)
            {
                free(pAudioStreamInfo->pCodecSpecificData);
                pAudioStreamInfo->pCodecSpecificData = NULL;
                pAudioStreamInfo->nCodecSpecificDataLen = 0;
            }
        }

        free(pMediaInfo->pAudioStreamInfo);
        pMediaInfo->pAudioStreamInfo = NULL;
        pMediaInfo->nAudioStreamNum = 0;
    }

    if(pMediaInfo->nSubtitleStreamNum > 0)
    {
        free(pMediaInfo->pSubtitleStreamInfo);
        pMediaInfo->pSubtitleStreamInfo = NULL;
        pMediaInfo->nSubtitleStreamNum = 0;
    }

    pMediaInfo->nFileSize      = 0;
    pMediaInfo->nDurationMs    = 0;
    pMediaInfo->eContainerType = CONTAINER_TYPE_UNKNOWN;
    pMediaInfo->bSeekable      = 0;

    return;
}
static int setMediaInfo(MediaInfo* pMediaInfo, CdxMediaInfoT* pInfoFromParser)
{
    int                 i;
    int                 nStreamCount;
    VideoStreamInfo*    pVideoStreamInfo = NULL;
    AudioStreamInfo*    pAudioStreamInfo = NULL;
    SubtitleStreamInfo* pSubtitleStreamInfo = NULL;
    int                 nCodecSpecificDataLen;
    char*               pCodecSpecificData = NULL;

    clearMediaInfo(pMediaInfo);

    pMediaInfo->nDurationMs = pInfoFromParser->program[0].duration;
    pMediaInfo->nFileSize   = pInfoFromParser->fileSize;
    pMediaInfo->nFirstPts   = pInfoFromParser->program[0].firstPts;
    if(pInfoFromParser->bitrate > 0)
        pMediaInfo->nBitrate = pInfoFromParser->bitrate;
    else if(pInfoFromParser->fileSize > 0 && pInfoFromParser->program[0].duration > 0)
        pMediaInfo->nBitrate =
            (int)((pInfoFromParser->fileSize*8*1000)/pInfoFromParser->program[0].duration);
    else
        pMediaInfo->nBitrate = 0;
    pMediaInfo->bSeekable   = pInfoFromParser->bSeekable;

    memcpy(pMediaInfo->cRotation,pInfoFromParser->rotate,4*sizeof(unsigned char));

    nStreamCount = pInfoFromParser->program[0].videoNum;
    logv("video stream count = %d", nStreamCount);
    if(nStreamCount > 0)
    {
        pVideoStreamInfo = (VideoStreamInfo*)malloc(sizeof(VideoStreamInfo)*nStreamCount);
        if(pVideoStreamInfo == NULL)
        {
            loge("can not alloc memory for media info.");
            return -1;
        }
        memset(pVideoStreamInfo, 0, sizeof(VideoStreamInfo)*nStreamCount);
        pMediaInfo->pVideoStreamInfo = pVideoStreamInfo;

        for(i=0; i<nStreamCount; i++)
        {
            pVideoStreamInfo = &pMediaInfo->pVideoStreamInfo[i];
            memcpy(pVideoStreamInfo, &pInfoFromParser->program[0].video[i],
                        sizeof(VideoStreamInfo));

            pCodecSpecificData    = pVideoStreamInfo->pCodecSpecificData;
            nCodecSpecificDataLen = pVideoStreamInfo->nCodecSpecificDataLen;
            pVideoStreamInfo->pCodecSpecificData = NULL;
            pVideoStreamInfo->nCodecSpecificDataLen = 0;

            if(pCodecSpecificData != NULL && nCodecSpecificDataLen > 0)
            {
                pVideoStreamInfo->pCodecSpecificData = (char*)malloc(nCodecSpecificDataLen);
                if(pVideoStreamInfo->pCodecSpecificData == NULL)
                {
                    loge("can not alloc memory for media info.");
                    clearMediaInfo(pMediaInfo);
                    return -1;
                }

                memcpy(pVideoStreamInfo->pCodecSpecificData, pCodecSpecificData,
                            nCodecSpecificDataLen);
                pVideoStreamInfo->nCodecSpecificDataLen = nCodecSpecificDataLen;
            }

            logv("the %dth video stream info.", i);
            logv("    codec: %d.", pVideoStreamInfo->eCodecFormat);
            logv("    width: %d.", pVideoStreamInfo->nWidth);
            logv("    height: %d.", pVideoStreamInfo->nHeight);
            logv("    frame rate: %d.", pVideoStreamInfo->nFrameRate);
            logv("    aspect ratio: %d.", pVideoStreamInfo->nAspectRatio);
            logv("    is 3D: %s.", pVideoStreamInfo->bIs3DStream ? "true" : "false");
            logv("    codec specific data size: %d.", pVideoStreamInfo->nCodecSpecificDataLen);
            logv("    bSecureStreamFlag : %d",pVideoStreamInfo->bSecureStreamFlag);
        }

        pMediaInfo->nVideoStreamNum = nStreamCount;
    }

    //* copy audio stream info.
    nStreamCount = pInfoFromParser->program[0].audioNum;
    if(nStreamCount > 0)
    {
        pAudioStreamInfo = (AudioStreamInfo*)malloc(sizeof(AudioStreamInfo)*nStreamCount);
        if(pAudioStreamInfo == NULL)
        {
            clearMediaInfo(pMediaInfo);
            loge("can not alloc memory for media info.");
            return -1;
        }
        memset(pAudioStreamInfo, 0, sizeof(AudioStreamInfo)*nStreamCount);
        pMediaInfo->pAudioStreamInfo = pAudioStreamInfo;

        for(i=0; i<nStreamCount; i++)
        {
            pAudioStreamInfo = &pMediaInfo->pAudioStreamInfo[i];
            memcpy(pAudioStreamInfo, &pInfoFromParser->program[0].audio[i],
                        sizeof(AudioStreamInfo));

            pCodecSpecificData    = pAudioStreamInfo->pCodecSpecificData;
            nCodecSpecificDataLen = pAudioStreamInfo->nCodecSpecificDataLen;
            pAudioStreamInfo->pCodecSpecificData = NULL;
            pAudioStreamInfo->nCodecSpecificDataLen = 0;

            if(pCodecSpecificData != NULL && nCodecSpecificDataLen > 0)
            {
                pAudioStreamInfo->pCodecSpecificData = (char*)malloc(nCodecSpecificDataLen);
                if(pAudioStreamInfo->pCodecSpecificData == NULL)
                {
                    loge("can not alloc memory for media info.");
                    clearMediaInfo(pMediaInfo);
                    return -1;
                }

                memcpy(pAudioStreamInfo->pCodecSpecificData, pCodecSpecificData,
                                nCodecSpecificDataLen);
                pAudioStreamInfo->nCodecSpecificDataLen = nCodecSpecificDataLen;
            }
        }

        pMediaInfo->nAudioStreamNum = nStreamCount;
    }

    //* copy subtitle stream info.
    nStreamCount = pInfoFromParser->program[0].subtitleNum;
    if(nStreamCount > 0)
    {
        pSubtitleStreamInfo = (SubtitleStreamInfo*)malloc(sizeof(SubtitleStreamInfo)*nStreamCount);
        if(pSubtitleStreamInfo == NULL)
        {
            clearMediaInfo(pMediaInfo);
            loge("can not alloc memory for media info.");
            return -1;
        }
        memset(pSubtitleStreamInfo, 0, sizeof(SubtitleStreamInfo)*nStreamCount);
        pMediaInfo->pSubtitleStreamInfo = pSubtitleStreamInfo;

        for(i=0; i<nStreamCount; i++)
        {
            pSubtitleStreamInfo = &pMediaInfo->pSubtitleStreamInfo[i];
            memcpy(pSubtitleStreamInfo, &pInfoFromParser->program[0].subtitle[i],
                            sizeof(SubtitleStreamInfo));
            pSubtitleStreamInfo->bExternal = 0;
            pSubtitleStreamInfo->pUrl      = NULL;
            pSubtitleStreamInfo->fd        = -1;
            pSubtitleStreamInfo->fdSub     = -1;
        }

        pMediaInfo->nSubtitleStreamNum = nStreamCount;
    }

    return 0;
}

struct AwMessage {
    AWMESSAGE_COMMON_MEMBERS
    uintptr_t params[5];
};

CTC_Wrapper::CTC_Wrapper()
{
    logv("CTC_Wrapper construct.");

    mSourceUrl      = NULL;

    mGraphicBufferProducer = NULL;
    mNativeWindow   = NULL;
    surface = NULL;

    mStatus         = AWPLAYER_STATUS_IDLE;
    mSeeking        = 0;
    mSeekSync       = 0;

    pParser   =  NULL;
    pStream = NULL;
    mKeepLastFrame  = 0;
    memset(&parserMediaInfo, 0, sizeof(CdxMediaInfoT));
    mMediaInfo      = NULL;
    mMessageQueue   = NULL;
    mVideoSizeWidth = 0;
    mVideoSizeHeight= 0;
    mediaProcessor = GetMediaProcessor();

    mCurrentSelectAudioTrack = -1;
    mCurrentSelectSubtitleTrack = -1;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mMutexMediaInfo, NULL);
    pthread_mutex_init(&mMutexStatus, NULL);
    sem_init(&mSemSetDataSource, 0, 0);
    sem_init(&mSemPrepare, 0, 0);
    sem_init(&mSemStart, 0, 0);
    sem_init(&mSemStop, 0, 0);
    sem_init(&mSemPause, 0, 0);
    sem_init(&mSemReset, 0, 0);
    sem_init(&mSemQuit, 0, 0);
    sem_init(&mSemSeek, 0, 0);
    sem_init(&mSemSetSurface, 0, 0);
    sem_init(&mSemSetAudioSink, 0, 0);

    sem_init(&mSemPrepareFinish, 0, 0); //* for signal prepare finish, used in prepare().

    mMessageQueue = AwMessageQueueCreate(64, "CTC_Wrap");
    memset(&mSource, 0x00, sizeof(CdxDataSourceT));
    mediaProcessor->playerback_register_evt_cb(IPTV_allbackProcess, this);

    if(pthread_create(&mThreadId, NULL, CTC_WrapperThread, this) == 0)
        mThreadCreated = 1;
    else
        mThreadCreated = 0;

}

CTC_Wrapper::~CTC_Wrapper()
{
    AwMessage msg;
    logw("~CTC_Wrapper");

    if(mThreadCreated)
    {
        void* status;

        reset();    //* stop demux and player.

        //* send a quit message to quit the main thread.
        memset(&msg, 0, sizeof(AwMessage));
        msg.messageId = AWPLAYER_COMMAND_QUIT;
        msg.params[0] = (unsigned int)&mSemQuit;

        AwMessageQueuePostMessage(mMessageQueue, &msg);
        SemTimedWait(&mSemQuit, -1);
        pthread_join(mThreadId, &status);
    }

    if(mMessageQueue != NULL)
        AwMessageQueueDestroy(mMessageQueue);

    pthread_mutex_destroy(&mMutexMediaInfo);
    pthread_mutex_destroy(&mMutexStatus);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&mSemSetDataSource);
    sem_destroy(&mSemPrepare);
    sem_destroy(&mSemStart);
    sem_destroy(&mSemStop);
    sem_destroy(&mSemPause);
    sem_destroy(&mSemReset);
    sem_destroy(&mSemQuit);
    sem_destroy(&mSemSeek);
    sem_destroy(&mSemSetSurface);
    sem_destroy(&mSemSetAudioSink);
    sem_destroy(&mSemPrepareFinish);
#if ClearMediaInfo

    if(mMediaInfo != NULL)
        clearMediaInfo();
#else
    clearMediaInfo(&mediaInfo);
#endif

    if(mSourceUrl != NULL)
        free(mSourceUrl);

}
CdxMediaInfoT* CTC_Wrapper::GetMediaInfo()
{
    return &parserMediaInfo;
}
int CTC_Wrapper::SwitchAudio(int nStreamIndex)
{
    if(nStreamIndex < 0 || !mMediaInfo || nStreamIndex >= mMediaInfo->nAudioStreamNum)
    {
        loge("nStreamIndex invalid (%d)", nStreamIndex);
        return -1;
    }
    mediaProcessor->SwitchAudioTrack(parserMediaInfo.program[0].id);
    return 0;
}
int CTC_Wrapper::SetVolume(int volume)
{
    return mediaProcessor->SetVolume(volume);
}
int CTC_Wrapper::GetVolume()
{
    return mediaProcessor->GetVolume();
}

status_t CTC_Wrapper::setDataSource(const char* pUrl, const KeyedVector<String8, String8>* pHeaders)
{
    AwMessage msg;
    status_t ret;

    if(pUrl == NULL)
    {
        loge("setDataSource(url), url=NULL");
        return BAD_TYPE;
    }

    logd("setDataSource(url), url='%s'", pUrl);

    //* send a set data source message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = AWPLAYER_COMMAND_SET_SOURCE;
    msg.params[0] = (unsigned int)&mSemSetDataSource;    //* params[0] = &mSemSetDataSource.
    msg.params[1] = (unsigned int)&mSetDataSourceReply;  //* params[1] = &mSetDataSourceReply.
    msg.params[2] = SOURCE_TYPE_URL;       //* params[2] = SOURCE_TYPE_URL.
    msg.params[3] = (unsigned int)pUrl;    //* params[3] = pUrl.
    msg.params[4] = (unsigned int)pHeaders;//* params[4] = KeyedVector<String8, String8>*pHeaders;

    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemSetDataSource, -1);

    return mSetDataSourceReply != (int)OK ? mSetDataSourceReply : (int)OK;
}

status_t CTC_Wrapper::setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer)
{
    AwMessage msg;

    logv("setVideoSurfaceTexture, surface = %p", bufferProducer.get());

    //* send a set surface message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = AWPLAYER_COMMAND_SET_SURFACE;
    msg.params[0] = (uintptr_t)&mSemSetSurface;    //* params[0] = &mSemSetDataSource.
    msg.params[1] = (uintptr_t)&mSetSurfaceReply;  //* params[1] = &mSetDataSourceReply.
    msg.params[2] = (uintptr_t)bufferProducer.get();

    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemSetSurface, -1);

    return (status_t)mSetSurfaceReply;
}

status_t CTC_Wrapper::prepareAsync()
{
    AwMessage msg;

    logd("prepareAsync");

    //* send a prepare.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = AWPLAYER_COMMAND_PREPARE;

    AwMessageQueuePostMessage(mMessageQueue, &msg);
    return 0;
}
status_t CTC_Wrapper::prepare()
{
    AwMessage msg;

    logd("prepare");

    return 0;
}

int CTC_Wrapper::setNotifyCallback(NotifyCallback notifier, void* pUserData)
{
    mNotifier = notifier;
    mUserData = pUserData;
    return 0;
}
int CTC_Wrapper::initCheck()
{
    logv("initCheck");

    if(mediaProcessor == NULL || mThreadCreated == 0 || mNotifier == NULL)
    {
        loge("initCheck() fail, mediaProcessor = %p, mNotifier = %p", mediaProcessor, mNotifier);
        return -1;
    }
    else
        return 0;
}

status_t CTC_Wrapper::start()
{
    AwMessage msg;

    logd("start");

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = AWPLAYER_COMMAND_START;
    msg.params[0] = (uintptr_t)&mSemStart;    //* params[0] = &mSemSetDataSource.
    msg.params[1] = (uintptr_t)&mStartReply;  //* params[1] = &mSetDataSourceReply.

    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemStart, -1);
    return (status_t)mStartReply;
}

status_t CTC_Wrapper::resume()
{
    AwMessage msg;

    logd("start");

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = AWPLAYER_COMMAND_RESUME;
    msg.params[0] = (uintptr_t)&mSemStart;
    msg.params[1] = (uintptr_t)&mStartReply;

    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemStart, -1);
    return (status_t)mStartReply;
}

status_t CTC_Wrapper::stop()
{
    AwMessage msg;

    logw("stop");

    //* send a stop message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = AWPLAYER_COMMAND_STOP;
    msg.params[0] = (uintptr_t)&mSemStop;
    msg.params[1] = (uintptr_t)&mStopReply;

    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemStop, -1);
    return (status_t)mStopReply;
}

status_t CTC_Wrapper::fast()
{
    AwMessage msg;

    logw("stop");

    //* send a stop message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = AWPLAYER_COMMAND_FAST;
    msg.params[0] = (uintptr_t)&mSemStop;
    msg.params[1] = (uintptr_t)&mStopReply;

    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemStop, -1);
    return (status_t)mStopReply;
}

status_t CTC_Wrapper::stopFast()
{
    AwMessage msg;

    logw("stop");

    //* send a stop message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = AWPLAYER_COMMAND_STOPFAST;
    msg.params[0] = (uintptr_t)&mSemStop;
    msg.params[1] = (uintptr_t)&mStopReply;

    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemStop, -1);
    return (status_t)mStopReply;
}

status_t CTC_Wrapper::pause()
{
    AwMessage msg;

    logd("pause");

    //* send a pause message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = AWPLAYER_COMMAND_PAUSE;
    msg.params[0] = (uintptr_t)&mSemPause;
    msg.params[1] = (uintptr_t)&mPauseReply;

    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemPause, -1);
    return (status_t)mPauseReply;
}

status_t CTC_Wrapper::seekTo(int nSeekTimeMs)
{
    AwMessage msg;

    logd("seekTo [%dms]", nSeekTimeMs);
    pthread_mutex_lock(&mMutexStatus);
    if(mSeeking)
    {
        CdxParserForceStop(pParser);
    }
    pthread_mutex_unlock(&mMutexStatus);
    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = AWPLAYER_COMMAND_SEEK;
    msg.params[0] = (uintptr_t)&mSemSeek;
    msg.params[1] = (uintptr_t)&mSeekReply;
    msg.params[2] = nSeekTimeMs;
    msg.params[3] = 0;

    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemSeek, -1);
    return (status_t)mSeekReply;
}

status_t CTC_Wrapper::reset()
{
    AwMessage msg;

    logw("reset...");

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = AWPLAYER_COMMAND_RESET;
    msg.params[0] = (uintptr_t)&mSemReset;
    msg.params[1] = (uintptr_t)&mResetReply;

    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemReset, -1);
    return (status_t)mResetReply;
}

bool CTC_Wrapper::isPlaying()
{
    logv("isPlaying");
    if(mStatus == AWPLAYER_STATUS_STARTED ||
        (mStatus == AWPLAYER_STATUS_COMPLETE && mLoop != 0))
        return true;
    else
        return false;
}

status_t CTC_Wrapper::getCurrentPosition(int* msec)
{
    int64_t nPositionUs;

    logv("getCurrentPosition");

    if(mStatus == AWPLAYER_STATUS_PREPARED ||
       mStatus == AWPLAYER_STATUS_STARTED  ||
       mStatus == AWPLAYER_STATUS_PAUSED   ||
       mStatus == AWPLAYER_STATUS_COMPLETE)
    {
        if(mSeeking != 0)
        {
            *msec = mSeekTime;
            return OK;
        }

        pthread_mutex_lock(&mMutexMediaInfo);
        if(mMediaInfo != NULL)
        {
            nPositionUs = mediaProcessor->GetCurrentPlayTime();
            *msec = (nPositionUs + 500)/1000;
            pthread_mutex_unlock(&mMutexMediaInfo);
            return OK;
        }
        else
        {
            loge("getCurrentPosition() fail, mMediaInfo==NULL.");
            *msec = 0;
            pthread_mutex_unlock(&mMutexMediaInfo);
            return OK;
        }
    }
    else
    {
        *msec = 0;
        if(mStatus == AWPLAYER_STATUS_ERROR)
            return INVALID_OPERATION;
        else
            return OK;
    }
}

status_t CTC_Wrapper::getDuration(int *msec)
{
    logv("getDuration");

    if(mStatus == AWPLAYER_STATUS_PREPARED ||
       mStatus == AWPLAYER_STATUS_STARTED  ||
       mStatus == AWPLAYER_STATUS_PAUSED   ||
       mStatus == AWPLAYER_STATUS_STOPPED  ||
       mStatus == AWPLAYER_STATUS_COMPLETE)
    {
        pthread_mutex_lock(&mMutexMediaInfo);
        if(mMediaInfo != NULL)
            *msec = mMediaInfo->nDurationMs;
        else
        {
            loge("getCurrentPosition() fail, mMediaInfo==NULL.");
            *msec = 0;
        }
        pthread_mutex_unlock(&mMutexMediaInfo);
        return OK;
    }
    else
    {
        loge("invalid getDuration() call, player not in valid status.");
        return INVALID_OPERATION;
    }
}

status_t CTC_Wrapper::setVideoWindow(int x, int y, int width, int height){
    if(mediaProcessor == NULL){
        return -1;
    }
    mediaProcessor->SetVideoWindow(x, y, width, height);
    return OK;
}

status_t CTC_Wrapper::show(){
    if(mediaProcessor == NULL){
        return -1;
    }
    mediaProcessor->VideoShow();
    return OK;
}

status_t CTC_Wrapper::hide(){
    if(mediaProcessor == NULL){
        return -1;
    }
    mediaProcessor->VideoHide();
    return OK;
}

status_t CTC_Wrapper::getVideoPixel(int& width, int& height){
    if(mediaProcessor == NULL){
        return -1;
    }
    mediaProcessor->GetVideoPixels(width, height);
    return OK;
}

status_t CTC_Wrapper::setVideoRadio(int radio){
    if(mediaProcessor == NULL){
        return -1;
    }
    mediaProcessor->SetRatio(radio);
    return OK;
}

#if ClearMediaInfo
void CTC_Wrapper::clearMediaInfo()
{
    int                 i;
    VideoStreamInfo*    v;
    AudioStreamInfo*    a;
    SubtitleStreamInfo* s;

    if(mMediaInfo != NULL)
    {
        //* free video stream info.
        if(mMediaInfo->pVideoStreamInfo != NULL)
        {
            for(i=0; i<mMediaInfo->nVideoStreamNum; i++)
            {
                v = &mMediaInfo->pVideoStreamInfo[i];
                if(v->pCodecSpecificData != NULL && v->nCodecSpecificDataLen > 0)
                    free(v->pCodecSpecificData);
            }
            free(mMediaInfo->pVideoStreamInfo);
            mMediaInfo->pVideoStreamInfo = NULL;
        }

        //* free audio stream info.
        if(mMediaInfo->pAudioStreamInfo != NULL)
        {
            for(i=0; i<mMediaInfo->nAudioStreamNum; i++)
            {
                a = &mMediaInfo->pAudioStreamInfo[i];
                if(a->pCodecSpecificData != NULL && a->nCodecSpecificDataLen > 0)
                    free(a->pCodecSpecificData);
            }
            free(mMediaInfo->pAudioStreamInfo);
            mMediaInfo->pAudioStreamInfo = NULL;
        }

        //* free subtitle stream info.
        if(mMediaInfo->pSubtitleStreamInfo != NULL)
        {
            for(i=0; i<mMediaInfo->nSubtitleStreamNum; i++)
            {
                s = &mMediaInfo->pSubtitleStreamInfo[i];
                if(s->pUrl != NULL)
                {
                    free(s->pUrl);
                    s->pUrl = NULL;
                }
                if(s->fd >= 0)
                {
                    ::close(s->fd);
                    s->fd = -1;
                }
                if(s->fdSub >= 0)
                {
                    ::close(s->fdSub);
                    s->fdSub = -1;
                }
            }
            free(mMediaInfo->pSubtitleStreamInfo);
            mMediaInfo->pSubtitleStreamInfo = NULL;
        }

        //* free the media info.
        free(mMediaInfo);
        mMediaInfo = NULL;
    }

    return;
}
#endif

static int setDataSourceFields(CdxDataSourceT* source, char* uri,
                                KeyedVector  <String8,String8>* pHeaders)
{
    CdxHttpHeaderFieldsT* pHttpHeaders;
    int                   i;
    int                   nHeaderSize;

    clearDataSourceFields(source);

    if(uri != NULL)
    {
        //* check whether ths uri has a scheme.
        if(strstr(uri, "://") != NULL)
        {
            source->uri = strdup(uri);
            if(source->uri == NULL)
            {
                loge("can not dump string of uri.");
                return -1;
            }
        }
        else
        {
            source->uri  = (char*)malloc(strlen(uri)+8);
            if(source->uri == NULL)
            {
                loge("can not dump string of uri.");
                return -1;
            }
            sprintf(source->uri, "file://%s", uri);
        }

        if(pHeaders != NULL && (!strncasecmp("http://", uri, 7) ||
                    !strncasecmp("https://", uri, 8)))
        {
            String8 key;
            String8 value;
            char*   str;

            i = pHeaders->indexOfKey(String8("x-hide-urls-from-log"));
            if(i >= 0)
                pHeaders->removeItemsAt(i);

            nHeaderSize = pHeaders->size();
            if(nHeaderSize > 0)
            {
                pHttpHeaders = (CdxHttpHeaderFieldsT*)malloc(sizeof(CdxHttpHeaderFieldsT));
                if(pHttpHeaders == NULL)
                {
                    loge("can not malloc memory for http header.");
                    clearDataSourceFields(source);
                    return -1;
                }
                memset(pHttpHeaders, 0, sizeof(CdxHttpHeaderFieldsT));
                pHttpHeaders->num = nHeaderSize;

                pHttpHeaders->pHttpHeader = (CdxHttpHeaderFieldT*)malloc(
                            sizeof(CdxHttpHeaderFieldT)*nHeaderSize);
                if(pHttpHeaders->pHttpHeader == NULL)
                {
                    loge("can not malloc memory for http header.");
                    free(pHttpHeaders);
                    clearDataSourceFields(source);
                    return -1;
                }

                source->extraData = (void*)pHttpHeaders;
                source->extraDataType = EXTRA_DATA_HTTP_HEADER;

                for(i=0; i<nHeaderSize; i++)
                {
                    key   = pHeaders->keyAt(i);
                    value = pHeaders->valueAt(i);
                    str = (char*)key.string();
                    if(str != NULL)
                    {
                        pHttpHeaders->pHttpHeader[i].key = (const char*)strdup(str);
                        if(pHttpHeaders->pHttpHeader[i].key == NULL)
                        {
                            loge("can not dump string of http header.");
                            clearDataSourceFields(source);
                            return -1;
                        }
                    }
                    else
                        pHttpHeaders->pHttpHeader[i].key = NULL;

                    str = (char*)value.string();
                    if(str != NULL)
                    {
                        pHttpHeaders->pHttpHeader[i].val = (const char*)strdup(str);
                        if(pHttpHeaders->pHttpHeader[i].val == NULL)
                        {
                            loge("can not dump string of http header.");
                            clearDataSourceFields(source);
                            return -1;
                        }
                    }
                    else
                        pHttpHeaders->pHttpHeader[i].val = NULL;
                }
            }
        }
    }

    return 0;
}
static void PostReadMessage(AwMessageQueue* mq)
{
    if(1/*AwMessageQueueGetCount(mq)<=0*/)
    {
        AwMessage msg;

        memset(&msg, 0, sizeof(AwMessage));
        msg.messageId = AWPLAYER_COMMAND_READ;
        AwMessageQueuePostMessage(mq, &msg);
    }

    return;
}

void ShowMediaInfo(CdxMediaInfoT *mediaInfo)
{
    printf("*********PrintMediaInfo begin*********\n");
    struct CdxProgramS *program = &mediaInfo->program[0];

    printf("fileSize = %lld, "
            "bSeekable = %d, "
            "duration = %d, "
            "audioNum = %d, "
            "videoNum = %d, "
            "subtitleNum = %d. \n",
            mediaInfo->fileSize,
            mediaInfo->bSeekable,
            program->duration,
            program->audioNum,
            program->videoNum,
            program->subtitleNum);
    int i;
    for (i = 0; i < VIDEO_STREAM_LIMIT && i < program->videoNum; i++)
    {
        VideoStreamInfo *video = program->video + i;
        printf("***Video[%d]*** "
                "eCodecFormat = 0x%x, "
                "nWidth = %d, "
                "nHeight = %d, "
                "nFrameRate = %d, "
                "nFrameDuration = %d, "
                "bIs3DStream = %d. \n ",
                i,
                video->eCodecFormat,
                video->nWidth,
                video->nHeight,
                video->nFrameRate,
                video->nFrameDuration,
                video->bIs3DStream);

    }

    for (i = 0; i < AUDIO_STREAM_LIMIT && i < program->audioNum; i++)
    {
        AudioStreamInfo *audio = program->audio + i;
        printf("***Audio[%d]*** "
                "eCodecFormat = 0x%x, "
                "eSubCodecFormat = 0x%x, "
                "nChannelNum = %d, "
                "nBitsPerSample = %d, "
                "nSampleRate = %d. \n ",
                i,
                audio->eCodecFormat,
                audio->eSubCodecFormat,
                audio->nChannelNum,
                audio->nBitsPerSample,
                audio->nSampleRate);

    }

    for (i = 0; i < SUBTITLE_STREAM_LIMIT && i < program->subtitleNum; i++)
    {
        SubtitleStreamInfo *subtitle = program->subtitle + i;

        printf("***Subtitle[%d]*** "
                "eCodecFormat = 0x%x, "
                "strLang = (%s). \n ",
                i,
                subtitle->eCodecFormat,
                subtitle->strLang);
    }

    printf("*********PrintMediaInfo end*********\n");
}

status_t CTC_Wrapper::mainThread()
{
    AwMessage            msg;
    int                  ret;
    sem_t*               pReplySem;
    int*                 pReplyValue;

    while(1)
    {
        if(AwMessageQueueGetMessage(mMessageQueue, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }
process_message:
        pReplySem   = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == AWPLAYER_COMMAND_SET_SOURCE)
        {
            logd("process message AWPLAYER_COMMAND_SET_SOURCE.");
            //* check status.
            if(mStatus != AWPLAYER_STATUS_IDLE && mStatus != AWPLAYER_STATUS_INITIALIZED)
            {
                loge("invalid setDataSource() operation, player not in IDLE or INITIALIZED status");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)INVALID_OPERATION;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            if((int)msg.params[2] == SOURCE_TYPE_URL)
            {
                KeyedVector<String8, String8>* pHeaders;
                //* data source is a url string.
                if(mSourceUrl != NULL)
                    free(mSourceUrl);
                mSourceUrl = strdup((char*)msg.params[3]);
                pHeaders   = (KeyedVector<String8, String8>*) msg.params[4];

                if(setDataSourceFields(&mSource, mSourceUrl, pHeaders) == 0)
                {
                    mStatus = AWPLAYER_STATUS_INITIALIZED;
                    if(pReplyValue != NULL)
                        *pReplyValue = (int)OK;
                }
                else
                {
                    loge("DemuxCompSetUrlSource() return fail.");
                    mStatus = AWPLAYER_STATUS_IDLE;
                    free(mSourceUrl);
                    mSourceUrl = NULL;
                    if(pReplyValue != NULL)
                        *pReplyValue = (int)UNKNOWN_ERROR;
                }
            }
            else
            {
                CDX_LOGE("should not be here. (%d)", (int)msg.params[2]);
            }

            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end AWPLAYER_COMMAND_SET_SOURCE.
        else if(msg.messageId == AWPLAYER_COMMAND_SET_SURFACE)
        {
            sp<IGraphicBufferProducer> graphicBufferProducer;
            sp<ANativeWindow>   anw;

            logd("process message AWPLAYER_COMMAND_SET_SURFACE.");

            //* set native window before delete the old one.
            //* because the player's render thread may use the old surface
            //* before it receive the new surface.

            /*
            graphicBufferProducer = (IGraphicBufferProducer *)msg.params[2];
            if(graphicBufferProducer.get() != NULL)
                anw = new Surface(graphicBufferProducer);
            else
                anw = NULL;
            mediaProcessor->SetSurface(anw);

            // save the new surface.
            mGraphicBufferProducer = graphicBufferProducer;
            if(mNativeWindow != NULL)
                native_window_api_disconnect(mNativeWindow.get(), NATIVE_WINDOW_API_MEDIA);
            mNativeWindow   = anw;

            if(ret == 0)
            {
                if(pReplyValue != NULL)
                    *pReplyValue = (int)OK;
            }
            else
            {
                loge("PlayerSetWindow() return fail.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)UNKNOWN_ERROR;
            }

            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
            */
        } //* end AWPLAYER_COMMAND_SET_SURFACE.
        else if(msg.messageId == AWPLAYER_COMMAND_PREPARE)
        {
            logd("process message AWPLAYER_COMMAND_PREPARE.");

            if(mStatus != AWPLAYER_STATUS_INITIALIZED && mStatus != AWPLAYER_STATUS_STOPPED)
            {
                logd("invalid prepareAsync() call, player not in initialized or stopped status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)INVALID_OPERATION;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            mStatus = AWPLAYER_STATUS_PREPARING;
            mPrepareSync = msg.params[2];

            int flags = 0, ret = 0;
            if(pParser)
            {
                //* should not run here, pParser should be NULL under INITIALIZED or STOPPED status.
                logw("demux->pParser != NULL when DEMUX_COMMAND_PREPARE message received.");
                CdxParserClose(pParser);
                pParser = NULL;
                pStream = NULL;
            }
            else if(pStream)
            {
                CdxStreamClose(pStream);
                pStream = NULL;
            }

#if AWPLAYER_CONFIG_DISABLE_SUBTITLE
            flags |= DISABLE_SUBTITLE;
#endif
#if AWPLAYER_CONFIG_DISABLE_AUDIO
            flags |= DISABLE_AUDIO;
#endif
#if AWPLAYER_CONFIG_DISABLE_VIDEO
            flags |= DISABLE_VIDEO;
#endif
#if !AWPLAYER_CONFIG_DISALBE_MULTI_AUDIO
            flags |= MUTIL_AUDIO;
#endif
            bCancelPrepare = 0;
            ret = CdxParserPrepare(&mSource, flags, &mutex, &bCancelPrepare,
                &pParser, &pStream, NULL, NULL);
            if(ret < 0)
            {
                goto _endTask;
            }

            //CdxMediaInfoT parserMediaInfo;
            memset(&parserMediaInfo,0,sizeof(CdxMediaInfoT));
            memset(&mediaInfo,0,sizeof(MediaInfo));

            CdxParserGetMediaInfo(pParser, &parserMediaInfo);
            setMediaInfo(&mediaInfo, &parserMediaInfo);
            mediaInfo.eContainerType = (enum ECONTAINER)pParser->type;
            if(!mediaInfo.nVideoStreamNum && !mediaInfo.nAudioStreamNum)
            {
                loge("nVideoStreamNum == 0 && nAudioStreamNum == 0");
                goto _endTask;
            }
            mMediaInfo = &mediaInfo;
            if(mediaInfo.nVideoStreamNum > 0)
            {
                VIDEO_PARA_T VideoPara;
                VideoPara.pid = parserMediaInfo.program[0].video[0].pid;
                //VideoPara.nVideoWidth = mediaInfo.pVideoStreamInfo[0].nWidth;
                //VideoPara.nVideoHeight = mediaInfo.pVideoStreamInfo[0].nHeight;
                VideoPara.nVideoWidth = 0;
                VideoPara.nVideoHeight = 0; //ipanel do not set video width and height
                VideoPara.vFmt = videoCodecConvert_1(
                        (enum EVIDEOCODECFORMAT)mediaInfo.pVideoStreamInfo[0].eCodecFormat);
                VideoPara.nFrameRate = mediaInfo.pVideoStreamInfo[0].nFrameRate/1000;
                mediaProcessor->InitVideo(&VideoPara);
            }
            if(mediaInfo.nAudioStreamNum > 0)
            {
                AUDIO_PARA_T AudioPara;
                AudioPara.pid = parserMediaInfo.program[0].audio[0].pid;
                AudioPara.aFmt = audioCodecConvert_1(mediaInfo.pAudioStreamInfo[0].eCodecFormat);
#if !NewInterface
                AudioPara.bit_per_sample = mediaInfo.pAudioStreamInfo[0].nBitsPerSample;
                AudioPara.block_align = mediaInfo.pAudioStreamInfo[0].nBlockAlign;
#endif
                AudioPara.nChannels = mediaInfo.pAudioStreamInfo[0].nChannelNum;
                AudioPara.nSampleRate = mediaInfo.pAudioStreamInfo[0].nSampleRate;
                AudioPara.nExtraSize = mediaInfo.pAudioStreamInfo[0].nCodecSpecificDataLen;
                AudioPara.pExtraData =
                    (unsigned char *)mediaInfo.pAudioStreamInfo[0].pCodecSpecificData;
                mediaProcessor->InitAudio(&AudioPara);
            }
            //sw_surface_init();
            //surface = (Surface *)swget_VideoSurface();
            //logd("surface %p", surface.get());
            //mediaProcessor->SetSurface(surface.get());

            //CDX_LOGD("CdxStreamTell(pStream)(%lld)", CdxStreamTell(pStream));
            CdxStreamSeek(pStream, 0, STREAM_SEEK_SET);

_endTask:
            if(ret != 0)
            {
                loge("DemuxCompPrepareAsync return fail immediately.");
                mStatus = AWPLAYER_STATUS_IDLE;
                if(pReplyValue != NULL)
                    *pReplyValue = (int)INVALID_OPERATION;
                mNotifier(mUserData, NOTIFY_ERROR, NOTIFY_ERROR_TYPE_IO, NULL);
            }
            else
            {
                mStatus = AWPLAYER_STATUS_PREPARED;
                if(pReplyValue != NULL)
                    *pReplyValue = (int)OK;
                mNotifier(mUserData, NOTIFY_PREPARED, 0, NULL);
            }

            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end AWPLAYER_COMMAND_PREPARE.
        else if(msg.messageId == AWPLAYER_COMMAND_START)
        {
            logd("process message AWPLAYER_COMMAND_START.");
            if(mStatus != AWPLAYER_STATUS_PREPARED &&
               mStatus != AWPLAYER_STATUS_STARTED  &&
               mStatus != AWPLAYER_STATUS_PAUSED   &&
               mStatus != AWPLAYER_STATUS_COMPLETE)
            {
                logd("invalid start() call, player not in prepared, \
                        started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)INVALID_OPERATION;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            bool ret = mediaProcessor->StartPlay();
            if(ret != true)
            {
                loge("mediaProcessor->StartPlay fail.");
            }

            mStatus = AWPLAYER_STATUS_STARTED;

            PostReadMessage(mMessageQueue);
            if(pReplyValue != NULL)
                *pReplyValue = (int)OK;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;

        } //* end AWPLAYER_COMMAND_START.
        else if(msg.messageId == AWPLAYER_COMMAND_RESUME)
        {
            logd("process message AWPLAYER_COMMAND_START.");
            if(mStatus != AWPLAYER_STATUS_PREPARED &&
               mStatus != AWPLAYER_STATUS_STARTED  &&
               mStatus != AWPLAYER_STATUS_PAUSED   &&
               mStatus != AWPLAYER_STATUS_COMPLETE)
            {
                logd("invalid start() call, player not in prepared, \
                        started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)INVALID_OPERATION;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            bool ret = mediaProcessor->Resume();
            if(ret != true)
            {
                loge("mediaProcessor->StartPlay fail.");
            }

            mStatus = AWPLAYER_STATUS_STARTED;

            PostReadMessage(mMessageQueue);
            if(pReplyValue != NULL)
                *pReplyValue = (int)OK;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;

        }
        else if(msg.messageId == AWPLAYER_COMMAND_FAST)
        {
            logd("process message AWPLAYER_COMMAND_FAST.");
            if(mStatus != AWPLAYER_STATUS_PREPARED &&
               mStatus != AWPLAYER_STATUS_STARTED  &&
               mStatus != AWPLAYER_STATUS_PAUSED   &&
               mStatus != AWPLAYER_STATUS_COMPLETE)
            {
                logd("invalid start() call, player not in prepared, \
                        started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)INVALID_OPERATION;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            bool ret = mediaProcessor->Fast();
            if(ret != true)
            {
                loge("mediaProcessor->StartPlay fail.");
            }

            mStatus = AWPLAYER_STATUS_STARTED;

            PostReadMessage(mMessageQueue);
            if(pReplyValue != NULL)
                *pReplyValue = (int)OK;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;

        } //* end AWPLAYER_COMMAND_FAST.
        else if(msg.messageId == AWPLAYER_COMMAND_STOPFAST)
        {
            logd("process message AWPLAYER_COMMAND_STOPFAST.");
            if(mStatus != AWPLAYER_STATUS_PREPARED &&
               mStatus != AWPLAYER_STATUS_STARTED  &&
               mStatus != AWPLAYER_STATUS_PAUSED   &&
               mStatus != AWPLAYER_STATUS_COMPLETE &&
               mStatus != AWPLAYER_STATUS_STOPPED)
            {
                logd("invalid stop() call, player not in prepared, \
                        paused, started, stopped or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)INVALID_OPERATION;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            if(mStatus == AWPLAYER_STATUS_STOPPED)
            {
                logv("player already in stopped status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)OK;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            bool ret = mediaProcessor->StopFast();
            if(ret != true)
            {
                loge("mediaProcessor->StartPlay fail.");
            }

            mStatus  = AWPLAYER_STATUS_STARTED;
            if(pReplyValue != NULL)
                *pReplyValue = (int)OK;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        }// * end AWPLAYER_COMMAND_STOPFAST.
        else if(msg.messageId == AWPLAYER_COMMAND_STOP)
        {
            logd("process message AWPLAYER_COMMAND_STOP.");
            if(mStatus != AWPLAYER_STATUS_PREPARED &&
               mStatus != AWPLAYER_STATUS_STARTED  &&
               mStatus != AWPLAYER_STATUS_PAUSED   &&
               mStatus != AWPLAYER_STATUS_COMPLETE &&
               mStatus != AWPLAYER_STATUS_STOPPED)
            {
                logd("invalid stop() call, player not in prepared, \
                        paused, started, stopped or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)INVALID_OPERATION;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            if(mStatus == AWPLAYER_STATUS_STOPPED)
            {
                logv("player already in stopped status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)OK;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            bool ret = mediaProcessor->Stop();
            if(ret != true)
            {
                loge("mediaProcessor->StartPlay fail.");
            }

            mStatus  = AWPLAYER_STATUS_STOPPED;
            if(pReplyValue != NULL)
                *pReplyValue = (int)OK;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end AWPLAYER_COMMAND_STOP.
        else if(msg.messageId == AWPLAYER_COMMAND_PAUSE)
        {
            logd("process message AWPLAYER_COMMAND_PAUSE.");
            if(mStatus != AWPLAYER_STATUS_STARTED  &&
               mStatus != AWPLAYER_STATUS_PAUSED   &&
               mStatus != AWPLAYER_STATUS_COMPLETE)
            {
                logd("invalid pause() call, player not in started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)INVALID_OPERATION;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            if(mStatus == AWPLAYER_STATUS_PAUSED)
            {
                logd("player already in paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)OK;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            pthread_mutex_lock(&mMutexStatus);

            if(mSeeking)
            {
                mStatus = AWPLAYER_STATUS_PAUSED;
                pthread_mutex_unlock(&mMutexStatus);

                if(pReplyValue != NULL)
                    *pReplyValue = (int)OK;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            bool ret = mediaProcessor->Pause();
            if(ret != true)
            {
                loge("mediaProcessor->StartPlay fail.");
            }
            mStatus = AWPLAYER_STATUS_PAUSED;

            //* sync with the seek, complete or pause_player/resume_player call back.
            pthread_mutex_unlock(&mMutexStatus);

            if(pReplyValue != NULL)
                *pReplyValue = (int)OK;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end AWPLAYER_COMMAND_PAUSE.
        else if(msg.messageId == AWPLAYER_COMMAND_SEEK)
        {
            logd("process message AWPLAYER_COMMAND_SEEK.");
            if(mStatus != AWPLAYER_STATUS_PREPARED &&
               mStatus != AWPLAYER_STATUS_STARTED  &&
               mStatus != AWPLAYER_STATUS_PAUSED   &&
               mStatus != AWPLAYER_STATUS_COMPLETE)
            {
                logd("invalid seekTo() call, player not in prepared,\
                            started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)INVALID_OPERATION;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            //* the application will call seekTo() when player is in complete status.
            //* after seekTo(), the player should still stay on complete status until
            //* application call start().
            //* cts test requires this implement.
            if(mStatus == AWPLAYER_STATUS_COMPLETE)
            {
                pthread_mutex_lock(&mMutexStatus);
                mSeekTime = msg.params[2];
                pthread_mutex_unlock(&mMutexStatus);
                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            bIsStreamEos = 0;
            bIOError = 0;

            if(mMediaInfo == NULL || mMediaInfo->bSeekable == 0)
            {
                if(mMediaInfo == NULL)
                {
                    loge("seekTo fail because mMediaInfo == NULL.");
                    if(pReplyValue != NULL)
                        *pReplyValue = (int)NO_INIT;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;
                }
                else
                {
                    loge("media not seekable.");
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;
                }
            }

            pthread_mutex_lock(&mMutexStatus);
            mSeeking  = 1;
            mSeekTime = msg.params[2];
            mSeekSync = msg.params[3];
            logv("seekTo %.2f secs", mSeekTime / 1E3);
            pthread_mutex_unlock(&mMutexStatus);

            if(mediaProcessor->GetPlayMode() == PLAYER_STATUS_STOPPED)
            {
                //* if in prepared status, the player is in stopped status,
                //* this will make the player not record the nSeekTime at PlayerReset() operation
                //* called at seek finish callback.
                mediaProcessor->StartPlay();
            }
            //mediaProcessor->Pause();
            CdxParserClrForceStop(pParser);
            ret = CdxParserSeekTo(pParser, ((int64_t)mSeekTime)*1000);
            mediaProcessor->Seek();
            if(ret < 0)
            {
                loge("CdxParserSeekTo fail.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }
            else
            {
                if(mStatus == AWPLAYER_STATUS_STARTED)
                {
                    mediaProcessor->StartPlay();
                }

                logd("+++++ mNotifier seek_complete");
                mNotifier(mUserData, NOTIFY_SEEK_COMPLETE, 0, NULL);
                logd("+++++ mNotifier seek_complete end");
                mSeeking  = 0;
                PostReadMessage(mMessageQueue);

                if(pReplyValue != NULL)
                    *pReplyValue = (int)OK;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }
        } //* end AWPLAYER_COMMAND_SEEK.
        else if(msg.messageId == AWPLAYER_COMMAND_QUIT)
        {
            if(pReplyValue != NULL)
                *pReplyValue = (int)OK;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            break;  //* break the thread.
        } //* end AWPLAYER_COMMAND_QUIT.
        else if(msg.messageId == AWPLAYER_COMMAND_RESET)
        {
            logd("process message AWPLAYER_COMMAND_RESET.");

            //* clear suface.
            if(mKeepLastFrame == 0)
            {
                if(mNativeWindow != NULL)
                    native_window_api_disconnect(mNativeWindow.get(), NATIVE_WINDOW_API_MEDIA);
                mNativeWindow.clear();
                mGraphicBufferProducer.clear();
            }

            //* clear data source.
            if(mSourceUrl != NULL)
            {
                free(mSourceUrl);
                mSourceUrl = NULL;
            }

            //* clear media info.
#if ClearMediaInfo

                clearMediaInfo();
#else
                clearMediaInfo(&mediaInfo);
#endif

            clearDataSourceFields(&mSource);

            //* set status to IDLE.
            mStatus = AWPLAYER_STATUS_IDLE;

            if(pReplyValue != NULL)
                *pReplyValue = (int)OK;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        }
        else if(msg.messageId == AWPLAYER_COMMAND_READ)
        {
//            logd("AWPLAYER_COMMAND_READ");
            if(bIsStreamEos || bIOError)
            {
                logd("CdxStreamEos");
                continue;
            }

            unsigned char* pBuffer = NULL;
            int nSize = 188 * 2 * 1024;

//            logd("AWPLAYER_COMMAND_READ1");
#if NewInterface
            pBuffer = (unsigned char*)malloc(nSize);
            CDX_CHECK(pBuffer);
            ret = CdxStreamRead(pStream, pBuffer, nSize);
            if(ret < 0)
            {
                logd("read error: %d", ret);
                if(mStopping == 0 && mSeeking == 0)
                {
                    int err = CdxStreamGetIoState(pStream);

                    if(err == CDX_IO_STATE_ERROR)
                    {
                        loge("stream io err");
                        bIOError = 1;
                    }
                    else
                    {
                        loge("stream err(%d)", err);
                    }
                }
                free(pBuffer);
                continue;
            }
            else if(ret == 0)
            {
                bIsStreamEos = 1;

                free(pBuffer);
                continue;
            }

  //          logd("AWPLAYER_COMMAND_READ2");

            int ret1;

            /*
            if(mediaProcessor->WriteData(pBuffer, ret) == -1)
            {
                loge("WriteData fail");
            }*/

            while(mediaProcessor->WriteData(pBuffer, ret) == -1)
            {
                //loge("WriteData fail");

                ret1 = AwMessageQueueTryGetMessage(mMessageQueue, &msg, 10);
                if(ret1 == 0)    // new message come, quit loop to process.
                {
                    CdxStreamSeek(pStream, -nSize, SEEK_CUR);
                    free(pBuffer);
                    goto process_message;
                }
            }

            free(pBuffer);
#else
            if(mediaProcessor->GetWriteBuffer(IPTV_PLAYER_STREAMTYPE_TS, &pBuffer, &nSize) == -1)
            {
                loge("GetWriteBuffer fail");
                usleep(10000);
                PostReadMessage(mMessageQueue);
                continue;
            }
            ret = CdxStreamRead(pStream, pBuffer, nSize);
            if(ret < 0)
            {
                if(mStopping == 0 && mSeeking == 0)
                {
                    int err = CdxStreamGetIoState(pStream);

                    if(err == CDX_IO_STATE_ERROR)
                    {
                        loge("stream io err");
                        bIOError = 1;
                    }
                    else
                    {
                        loge("stream err(%d)", err);
                    }
                }

                continue;
            }
            else if(ret == 0)
            {
                bIsStreamEos = 1;
                continue;
            }
            logd("AWPLAYER_COMMAND_READ2, ret(%d)", ret);
            if(mediaProcessor->WriteData(IPTV_PLAYER_STREAMTYPE_TS, pBuffer, ret, 0) == -1)
            {
                loge("WriteData fail");

                continue;
            }
#endif
            //usleep(200*1000);
            PostReadMessage(mMessageQueue);
            continue;

        }
        else if(msg.messageId == AWPLAYER_COMMAND_HIDE)
        {
            logd("AWPLAYER_COMMAND_HIDE");
            mediaProcessor->VideoHide();
            if(pReplyValue != NULL)
                *pReplyValue = (int)OK;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;

        }
        else
        {
            logw("unknow message with id %d, ignore.", msg.messageId);
        }
    }

    return OK;
}

static void* CTC_WrapperThread(void* arg)
{
    CTC_Wrapper* me = (CTC_Wrapper*)arg;
    me->mainThread();
    return NULL;
}
