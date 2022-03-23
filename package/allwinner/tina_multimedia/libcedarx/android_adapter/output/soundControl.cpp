/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : soundControl.cpp
 * Description : soundControl for android
 * History :
 *
 */


#include <utils/Errors.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "soundControl.h"
#include "cdx_log.h"
#include <media/AudioTrack.h>
#include <media/MediaPlayerInterface.h>
#include <inttypes.h>

#include <pthread.h>
#include "outputCtrl.h"

using namespace android;
#define SAVE_PCM (0)
#if SAVE_PCM
const char* binpath = "/data/camera/test.bin";
#endif

enum ESOUNDSTATUS
{
    SOUND_STATUS_STOPPED = 0,
    SOUND_STATUS_STARTED,
    SOUND_STATUS_PAUSED
};

typedef struct AudioDirectOutPutSink
{
    sp<AudioTrack>        pAudioTrackHandle;
    audio_output_flags_t  flag;
    audio_format_t        format;
    audio_channel_mask_t  chanmask;
    unsigned int          samplerate;
}AudioDirectOutPutSink;

typedef struct SoundCtrlContext
{
    SoundCtrl                   base;
    MediaPlayerBase::AudioSink* pAudioSink;

    sp<AudioTrack>              pAudioTrack;//* new an audio track if pAudioSink not set.
    unsigned int                nSampleRate;
    unsigned int                nChannelNum;
    unsigned int                nBitPerSample;
    unsigned int                needDirectOutPut;
    enum AUIDO_RAW_DATA_TYPE    data_type;
    AudioDirectOutPutSink       directSink;
    unsigned int                nRoutine;

#if (CONF_ANDROID_MAJOR_VER > 5)
    AudioPlaybackRate           nPlayRate;
#endif

    int64_t                     nDataSizePlayed;
    int64_t                     nFramePosOffset;
    unsigned int                nFrameSize;
    unsigned int                nLastFramePos;

    enum ESOUNDSTATUS          eStatus;
    pthread_mutex_t             mutex;
    float                        volume;

#if SAVE_PCM
    FILE*                        testpcm;
#endif

#if defined(CONF_SC_WRITE_ASYNC_MODE)
    size_t fillBuffer(void *data, size_t size);
    void *pBufAudioSink;
    size_t nSizeAudioSink;
    int bReachedEOS;
    pthread_cond_t cbCond;
    pthread_mutex_t cbMutex;
#endif
}SoundCtrlContext;

static int SoundDeviceStop_l(SoundCtrlContext* sc);

static void SoundClearDircectOutSink_l(SoundCtrlContext* sc)
{
    if(sc->directSink.pAudioTrackHandle != NULL)
    {
        sc->directSink.pAudioTrackHandle->stop();
        sc->directSink.pAudioTrackHandle.clear();
        sc->directSink.pAudioTrackHandle = NULL;
        sc->directSink.flag       = AUDIO_OUTPUT_FLAG_NONE;
        sc->directSink.format     = AUDIO_FORMAT_DEFAULT;
        sc->directSink.chanmask   = AUDIO_CHANNEL_NONE;
        sc->directSink.samplerate = 0;
    }
}
static void __SdDestroy(SoundCtrl* s)
{
    SoundCtrlContext* sc;

    logd("<SoundCtl>: destroy");
    sc = (SoundCtrlContext*)s;

    pthread_mutex_lock(&sc->mutex);
    if(sc->eStatus != SOUND_STATUS_STOPPED)
        SoundDeviceStop_l(sc);
    SoundClearDircectOutSink_l(sc);
    pthread_mutex_unlock(&sc->mutex);

    pthread_mutex_destroy(&sc->mutex);

#if defined(CONF_SC_WRITE_ASYNC_MODE)
    if(sc->pAudioSink != NULL)
    {
        pthread_mutex_destroy(&sc->cbMutex);
        pthread_cond_destroy(&sc->cbCond);
    }
#endif

#if SAVE_PCM
    fclose(sc->testpcm);
    sc->testpcm = NULL;
#endif


    free(sc);
    return;
}

static void __SdSetFormat(SoundCtrl* s, CdxPlaybkCfg* cfg)
{
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    pthread_mutex_lock(&sc->mutex);

    if(sc->eStatus != SOUND_STATUS_STOPPED)
    {
        loge("Sound device not int stop status, can not set audio params.");
        abort();
    }

    if(cfg == NULL)
    {
        loge("invalid operation in __SdSetFormat");
        pthread_mutex_unlock(&sc->mutex);
        return;
    }
    else
    {
        sc->nBitPerSample    = cfg->nBitpersample;
        sc->nSampleRate      = cfg->nSamplerate;
        sc->nChannelNum      = cfg->nChannels;
        sc->needDirectOutPut = cfg->nNeedDirect;
        sc->data_type        = cfg->nDataType;
        sc->nRoutine         = cfg->nRoutine;
        pthread_mutex_unlock(&sc->mutex);
        return;
    }
}

#if defined(CONF_SC_WRITE_ASYNC_MODE)
size_t SoundCtrlContext::fillBuffer(void *data, size_t size)
{
    size_t doneSize = 0;

    if(bReachedEOS == 1)
    {
        return 0;
    }

    pthread_mutex_lock(&cbMutex);
    pBufAudioSink = data;
    nSizeAudioSink = size;
    while(nSizeAudioSink > 0)
    {
        if(bReachedEOS == 1)
            break;
        pthread_cond_wait(&cbCond, &cbMutex);
    }
    doneSize = size - nSizeAudioSink;
    if(doneSize == size)
    {
        pBufAudioSink = NULL;
    }
    pthread_mutex_unlock(&cbMutex);
    return doneSize;
}

// static
size_t SoundAudioSinkCallback(
        MediaPlayerBase::AudioSink * /* audioSink */,
        void *buffer, size_t size, void *cookie,
        MediaPlayerBase::AudioSink::cb_event_t event)
{
    SoundCtrlContext* sc = (SoundCtrlContext *)cookie;

    switch(event)
    {
    case MediaPlayerBase::AudioSink::CB_EVENT_FILL_BUFFER:
        return sc->fillBuffer(buffer, size);

    case MediaPlayerBase::AudioSink::CB_EVENT_STREAM_END:
        ALOGV("SoundAudioSinkCallback: stream end");
        sc->bReachedEOS = 1;
        //sc->notifyAudioEOS();
        break;

    case MediaPlayerBase::AudioSink::CB_EVENT_TEAR_DOWN:
        ALOGV("SoundAudioSinkCallback: Tear down event");
        //sc->mObserver->postAudioTearDown();
        break;
    }

    return 0;
}
#endif

static int __SdControl(SoundCtrl* s, cdx_int32 cmd, void* para)
{
    SoundCtrlContext* sc;
    int ret = 0;
    sc = (SoundCtrlContext*)s;
    pthread_mutex_lock(&sc->mutex);

    switch(cmd)
    {
#if defined(CONF_SC_WRITE_ASYNC_MODE)
        case SOUND_CONTROLL_SET_CLBK_EOS:
            pthread_mutex_lock(&sc->cbMutex);
            sc->bReachedEOS = 1;
            pthread_cond_signal(&sc->cbCond);
            pthread_mutex_unlock(&sc->cbMutex);
            logd("__SdControll : set clbk eos");
            ret = 0;
            break;
#endif
        case SOUND_CONTROLL_QUERY_IF_GAPLESS_PLAY:
            {
                int answer = 0;
                if(!sc->pAudioSink->needsTrailingPadding())
                {
                    answer = 1;
                }
                ret = answer;
                logd("__SdControll : query if gapless play (%s)", answer?"yes":"no");
                break;
            }
        default:
            logd("__SdControll : unknown command (%d)...", cmd);
            ret = -1;
            break;
    }
    pthread_mutex_unlock(&sc->mutex);
    return ret;
}

static int __SdStart(SoundCtrl* s)
{
    SoundCtrlContext* sc;
    status_t          err;
    unsigned int      nFramePos;
    audio_output_flags_t flag = AUDIO_OUTPUT_FLAG_NONE;
    audio_format_t       format = AUDIO_FORMAT_PCM_16_BIT;
    audio_channel_mask_t chanmask = AUDIO_CHANNEL_OUT_STEREO;
    int reqBufferCount = 0;

    sc = (SoundCtrlContext*)s;
    logd("<SoundCtl>: start");
    pthread_mutex_lock(&sc->mutex);

    if(sc->eStatus == SOUND_STATUS_STARTED)
    {
        logw("Sound device already started.");
        pthread_mutex_unlock(&sc->mutex);
        return -1;
    }

    if(sc->eStatus == SOUND_STATUS_STOPPED)
    {
        if(sc->needDirectOutPut)
        {
            flag = AUDIO_OUTPUT_FLAG_DIRECT;
            switch(sc->data_type)
            {
                case AUDIO_RAW_DATA_AC3:
                    logd("cedarx passthrough data ac3");
                    format = AUDIO_FORMAT_AC3;
                    break;
                case AUDIO_RAW_DATA_DTS:
                    logd("cedarx passthrough data dts");
                    format = AUDIO_FORMAT_DTS;
                    break;
                case AUDIO_RAW_DATA_DOLBY_DIGITAL_PLUS:
                    logd("cedarx passthrough data ddp");
                    format = AUDIO_FORMAT_E_AC3;
                    break;
/*
* For latest android sdk, AUDIO_FORMAT_DOLBY_TRUEHD has not been defined in audio.h,
* and none in lower android version. So U should add AUDIO_FORMAT_DOLBY_TRUEHD in current
* android audio system util it has been defined.
* MORE details in:
*             'PassThrough Porting Manuals On Android Audio System' --- Coming soon...
* and then set
*                     HAS_ANDROID_SELF_CUSTOMIZATION := 0
*                                 --->
*                     HAS_ANDROID_SELF_CUSTOMIZATION := 1
* in libcedarx/config.mk
*/
#if (ANDROID_SELF_CUSTOMIZATION == 1)
                case AUDIO_RAW_DATA_MAT:
                    logd("cedarx passthrough data mat/truehd");
                    format = AUDIO_FORMAT_DOLBY_TRUEHD;
                    chanmask = AUDIO_CHANNEL_OUT_7POINT1;
                    break;
#endif
                default:
                    break;
            }

            if(sc->data_type == AUDIO_RAW_DATA_PCM)
            {
                format = AUDIO_FORMAT_PCM_16_BIT;
                switch(sc->nBitPerSample)
                {
                    case 32:
                        format = AUDIO_FORMAT_PCM_32_BIT;
                        break;
                    default:
                        break;
                }
                switch(sc->nChannelNum)
                {
                    case 6:
                        chanmask = AUDIO_CHANNEL_OUT_5POINT1;
                        break;
                    case 8:
                        chanmask = AUDIO_CHANNEL_OUT_7POINT1;
                        break;
                    default:
                        break;
                }
            }
        }

        if(sc->pAudioSink != NULL && !sc->needDirectOutPut)
        {
#if defined(CONF_SC_WRITE_ASYNC_MODE)
            sc->bReachedEOS = 0;
#endif
            /*
                For gapless playing:
                    Because of too much audioflinger frameconut in hal buffer leads to agressive
                delay in the very begining, whitch give a sillence buffer when mixing, we should set
                the buffer count to 0 for default nums.
            */
            if(!sc->pAudioSink->needsTrailingPadding())
            {
                reqBufferCount = 0;
            }

            if(!sc->pAudioSink->ready()){
                logd("Cedarx use AudioSink open spr : %d, ch : %d, fmt : %p",
                     sc->nSampleRate, sc->nChannelNum, format);
#if defined(CONF_SC_WRITE_ASYNC_MODE)
                err = sc->pAudioSink->open(sc->nSampleRate,
                                           sc->nChannelNum,
                                           CHANNEL_MASK_USE_CHANNEL_ORDER,
                                           format,
                                           reqBufferCount,
                                           &SoundAudioSinkCallback,
                                           sc,
                                           flag);
#else //sync mode
                err = sc->pAudioSink->open(sc->nSampleRate,
                                           sc->nChannelNum,
                                           CHANNEL_MASK_USE_CHANNEL_ORDER,
                                           format,
                                           reqBufferCount,
                                           NULL,    //* no callback mode.
                                           NULL,
                                           flag);
#endif
                if(err != OK)
                {
                    pthread_mutex_unlock(&sc->mutex);
                    return -1;
                }
            }
            else{
                logd("Audiosink has opened");
            }

            unsigned int tmp = 0;
            sc->pAudioSink->getPosition(&tmp);
            if(tmp != 0)
            {
                sc->pAudioSink->pause();
                sc->pAudioSink->flush();
            }

            sc->nFrameSize = sc->pAudioSink->frameSize();
            SoundClearDircectOutSink_l(sc);
        }
        else if(sc->needDirectOutPut)
        {
            do{
                logd("CedarX request Direct output to transfer iec61937 stream(or muti ch pcm)");
                if(sc->pAudioTrack == NULL)
                {
                    if(sc->directSink.pAudioTrackHandle != NULL)
                    {
                        if(sc->directSink.chanmask == chanmask &&
                            sc->directSink.flag == flag &&
                            sc->directSink.format == format &&
                            sc->directSink.samplerate == sc->nSampleRate)
                        {
                            logd("Direct out put reuse audiotrack from recycle");
                            sc->pAudioTrack = sc->directSink.pAudioTrackHandle;
                            break;
                        }
                        else
                        {
                            SoundClearDircectOutSink_l(sc);
                        }
                    }

                    logd("Direct out put new a Audiotrack");
                    sc->pAudioTrack = new AudioTrack();

                    if(sc->pAudioTrack == NULL)
                    {
                        loge("create audio track fail.");
                        pthread_mutex_unlock(&sc->mutex);
                        return -1;
                    }

                    sc->pAudioTrack->set(AUDIO_STREAM_MUSIC,
                                         sc->nSampleRate,
                                         format,
                                         chanmask,
                                         reqBufferCount,
                                         flag);

                    if(sc->directSink.pAudioTrackHandle != NULL)
                        loge("damn it! maybe memory leak!!!");
                    sc->directSink.flag     = flag;
                    sc->directSink.format   = format;
                    sc->directSink.chanmask = chanmask;
                    sc->directSink.samplerate = sc->nSampleRate;
                    sc->directSink.pAudioTrackHandle = sc->pAudioTrack;
                }
                else
                {
                    logd("Direct out put reuse audiotrack from self");
                }
            }
            while(0);

            if(sc->pAudioTrack->initCheck() != OK)
            {
                loge("audio track initCheck() return fail.");
                sc->pAudioTrack.clear();
                sc->pAudioTrack = NULL;
                pthread_mutex_unlock(&sc->mutex);
                return -1;
            }

            sc->nFrameSize = sc->pAudioTrack->frameSize();
            logd("framesize infomation : nFrameSize : %d", sc->nFrameSize);

#if (CONF_ANDROID_MAJOR_VER > 5)
            if (sc->nPlayRate.mSpeed != 1.0f)
            sc->pAudioTrack->setPlaybackRate(sc->nPlayRate);
#endif
        }
        else
        {
            sc->pAudioTrack = new AudioTrack();

            if(sc->pAudioTrack == NULL)
            {
                loge("create audio track fail.");
                pthread_mutex_unlock(&sc->mutex);
                return -1;
            }

            sc->pAudioTrack->set(AUDIO_STREAM_DEFAULT,
                                 sc->nSampleRate,
                                 format,
                                 (sc->nChannelNum == 2) ? AUDIO_CHANNEL_OUT_STEREO :
                                                          AUDIO_CHANNEL_OUT_MONO,20*1024);

            if(sc->pAudioTrack->initCheck() != OK)
            {
                loge("audio track initCheck() return fail.");
                sc->pAudioTrack.clear();
                sc->pAudioTrack = NULL;
                pthread_mutex_unlock(&sc->mutex);
                return -1;
            }

            sc->nFrameSize = sc->pAudioTrack->frameSize();

#if (CONF_ANDROID_MAJOR_VER > 5)
            if (sc->nPlayRate.mSpeed != 1.0f)
                sc->pAudioTrack->setPlaybackRate(sc->nPlayRate);
#endif
        }

        sc->nDataSizePlayed = 0;
        sc->nFramePosOffset = 0;
        sc->nLastFramePos   = 0;
    }

    if(sc->pAudioSink != NULL && !sc->needDirectOutPut)
        sc->pAudioSink->start();
    else
        sc->pAudioTrack->start();

    sc->eStatus = SOUND_STATUS_STARTED;
    pthread_mutex_unlock(&sc->mutex);

    return 0;
}

static int __SdStop(SoundCtrl* s)
{
    int               ret;
    SoundCtrlContext* sc;

    logd("<SoundCtl>: stop");
    sc = (SoundCtrlContext*)s;

    pthread_mutex_lock(&sc->mutex);
    ret = SoundDeviceStop_l(sc);
    pthread_mutex_unlock(&sc->mutex);

    return ret;
}

static int SoundDeviceStop_l(SoundCtrlContext* sc)
{
    if(sc->eStatus == SOUND_STATUS_STOPPED)
    {
        logw("Sound device already stopped.");
        return 0;
    }

    if(sc->pAudioSink != NULL && !sc->needDirectOutPut)
    {
#if defined(CONF_SC_WRITE_ASYNC_MODE)
        pthread_mutex_lock(&sc->cbMutex);
        sc->bReachedEOS = 1;
        pthread_cond_signal(&sc->cbCond);
        pthread_mutex_unlock(&sc->cbMutex);
#endif
        sc->pAudioSink->pause();
        sc->pAudioSink->flush();
        sc->pAudioSink->stop();
        sc->pAudioSink->close();
    }
    else
    {
        if (sc->pAudioTrack.get() != NULL)
        {
            sc->pAudioTrack->pause();
            sc->pAudioTrack->flush();
            sc->pAudioTrack->stop();
            sc->pAudioTrack.clear();
            sc->pAudioTrack = NULL;
        }
    }

    sc->nDataSizePlayed = 0;
    sc->nFramePosOffset = 0;
    sc->nLastFramePos   = 0;
    sc->nFrameSize      = 0;
    sc->eStatus         = SOUND_STATUS_STOPPED;
    return 0;
}

static int __SdPause(SoundCtrl* s)
{
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;
    logd("<SoundCtl>: pause");
    pthread_mutex_lock(&sc->mutex);

    if(sc->eStatus != SOUND_STATUS_STARTED)
    {
        logw("Invalid pause operation, sound device not in start status.");
        pthread_mutex_unlock(&sc->mutex);
        return -1;
    }

    if(sc->pAudioSink != NULL && !sc->needDirectOutPut)
        sc->pAudioSink->pause();
    else
    {
        if (sc->pAudioTrack.get() != NULL)
        {
            sc->pAudioTrack->pause();
        }
    }
    sc->eStatus = SOUND_STATUS_PAUSED;
    pthread_mutex_unlock(&sc->mutex);
    return 0;
}

static int __SdWrite(SoundCtrl* s, void* pData, int nDataSize)
{
    int               nWritten = 0;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    pthread_mutex_lock(&sc->mutex);

    if(sc->eStatus == SOUND_STATUS_STOPPED || sc->eStatus == SOUND_STATUS_PAUSED)
    {
        pthread_mutex_unlock(&sc->mutex);
        return 0;
    }

#if SAVE_PCM
    fwrite(pData, 1, nDataSize,sc->testpcm);
#endif

#if defined(CONF_SC_WRITE_ASYNC_MODE)
    if(sc->pAudioSink != NULL && !sc->needDirectOutPut)
    {
        //memcpy to the buf from audioSink
        pthread_mutex_lock(&sc->cbMutex);
        if(sc->nSizeAudioSink > 0 && sc->pBufAudioSink != NULL)
        {
            nWritten = sc->nSizeAudioSink < (size_t)nDataSize ? sc->nSizeAudioSink : nDataSize;
            memcpy(sc->pBufAudioSink, pData, nWritten);
            sc->nSizeAudioSink -= (size_t)nWritten;
            sc->pBufAudioSink = (char *)sc->pBufAudioSink + nWritten;
        }
        else
        {
            nWritten = -1;//buf is not ready
        }
        pthread_cond_signal(&sc->cbCond);
        pthread_mutex_unlock(&sc->cbMutex);
    }
#else
    if(sc->pAudioSink != NULL && !sc->needDirectOutPut)
        nWritten = sc->pAudioSink->write(pData, nDataSize);
#endif
    else
    {
        if (sc->pAudioTrack.get() != NULL)
        {
            nWritten = sc->pAudioTrack->write(pData, nDataSize);
        }
    }
    if(nWritten < 0)
        nWritten = 0;
    else
        sc->nDataSizePlayed += nWritten;

    pthread_mutex_unlock(&sc->mutex);

    return nWritten;
}

static int SoundDeviceReset_l(SoundCtrlContext* sc)
{
    if(sc->eStatus == SOUND_STATUS_STOPPED)
    {
        logd("Sound device already in Idle");
        return -1;
    }
    if(sc->pAudioSink != NULL && !sc->needDirectOutPut)
    {
#if defined(CONF_SC_WRITE_ASYNC_MODE)
        pthread_mutex_lock(&sc->cbMutex);
        sc->bReachedEOS = 1;
        pthread_cond_signal(&sc->cbCond);
        pthread_mutex_unlock(&sc->cbMutex);
#endif
        sc->pAudioSink->pause();
        sc->pAudioSink->flush();
        sc->pAudioSink->stop();
    }
    else
    {
        if (sc->pAudioTrack.get() != NULL)
        {
            sc->pAudioTrack->pause();
            sc->pAudioTrack->flush();
            sc->pAudioTrack->stop();
        }
    }

    sc->nDataSizePlayed = 0;
    sc->nFramePosOffset = 0;
    sc->nLastFramePos   = 0;
    sc->eStatus         = SOUND_STATUS_STOPPED;
    return 0;
}

//* called at player seek operation.
static int __SdReset(SoundCtrl* s)
{
    SoundCtrlContext* sc;
    int ret = 0;
    logd("<SoundCtl>: reset");
    sc = (SoundCtrlContext*)s;
    pthread_mutex_lock(&sc->mutex);
    ret = SoundDeviceReset_l(sc);
    pthread_mutex_unlock(&sc->mutex);
    return ret;
}

static int __SdGetCachedTime(SoundCtrl* s)
{
    unsigned int      nFramePos;
    int64_t           nCachedFrames;
    int64_t           nCachedTimeUs;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    pthread_mutex_lock(&sc->mutex);

    if(sc->eStatus == SOUND_STATUS_STOPPED)
    {
        pthread_mutex_unlock(&sc->mutex);
        return 0;
    }

    if(sc->pAudioSink != NULL && !sc->needDirectOutPut)
        sc->pAudioSink->getPosition(&nFramePos);
    else
    {
        if (sc->pAudioTrack.get() != NULL)
        {
            sc->pAudioTrack->getPosition(&nFramePos);
        }
    }

    if(sc->nFrameSize == 0)
    {
        loge("nFrameSize == 0.");
        abort();
    }

    if(nFramePos < sc->nLastFramePos)
        sc->nFramePosOffset += 0x100000000;
    nCachedFrames = sc->nDataSizePlayed/sc->nFrameSize - nFramePos - sc->nFramePosOffset;
    nCachedTimeUs = nCachedFrames*1000000/sc->nSampleRate;

    logv("nDataSizePlayed = %lld, nFrameSize = %d, nFramePos = %u, \
          nLastFramePos = %u, nFramePosOffset = %lld",
        sc->nDataSizePlayed, sc->nFrameSize, nFramePos, sc->nLastFramePos, sc->nFramePosOffset);

    logv("nCachedFrames = %lld, nCachedTimeUs = %lld, nSampleRate = %d",
        nCachedFrames, nCachedTimeUs, sc->nSampleRate);

    sc->nLastFramePos = nFramePos;
    pthread_mutex_unlock(&sc->mutex);

    return (int)(nCachedTimeUs);
}

static int __SdGetFrameCount(SoundCtrl* s)
{
    SoundCtrlContext* sc = (SoundCtrlContext*)s;
    int framecount = 0;
    if (sc->pAudioSink != NULL && !sc->needDirectOutPut)
    {
        framecount = sc->pAudioSink->frameCount();
    }
    else
    {
        if (sc->pAudioTrack.get() != NULL)
        {
            framecount = sc->pAudioTrack->frameCount();
        }
    }
    logv("framecount = %d",framecount);

    return framecount;
}

#if (CONF_ANDROID_MAJOR_VER > 5)
int _SetPlaybackRate(SoundCtrl* s,const XAudioPlaybackRate *rate)
{
    SoundCtrlContext* sc = (SoundCtrlContext*)s;

    int ret =0;

    sc->nPlayRate.mSpeed = rate->mSpeed;
    sc->nPlayRate.mPitch = rate->mPitch;
    sc->nPlayRate.mStretchMode = (AudioTimestretchStretchMode)rate->mStretchMode;
    sc->nPlayRate.mFallbackMode = (AudioTimestretchFallbackMode)rate->mFallbackMode;

    ALOGD("rate.mSpeed          %f",sc->nPlayRate.mSpeed);
    ALOGD("rate.mPitch          %f",sc->nPlayRate.mPitch);
    ALOGD("rate.mFallbackMode   %d",sc->nPlayRate.mFallbackMode);
    ALOGD("rate.mStretchMode    %d",sc->nPlayRate.mStretchMode);

    if (sc->pAudioSink != NULL && !sc->needDirectOutPut)
    {
        ret = sc->pAudioSink->setPlaybackRate(sc->nPlayRate);
    }
    else
    {
        if (sc->pAudioTrack == NULL)
            __SdStart(s);
        ret = sc->pAudioTrack->setPlaybackRate(sc->nPlayRate);
    }
    return ret;
}
#else
int _SetPlaybackRate(SoundCtrl* s,const XAudioPlaybackRate *rate)
{
    CDX_UNUSE(s);
    CDX_UNUSE(rate);
    return 0;
}
#endif

static SoundControlOpsT mSoundControlOps =
{
    .destroy       =   __SdDestroy,
    .setFormat     =   __SdSetFormat,
    .start         =   __SdStart,
    .stop          =   __SdStop,
    .pause         =   __SdPause,
    .write         =   __SdWrite,
    .reset         =   __SdReset,
    .getCachedTime =   __SdGetCachedTime,
    .getFrameCount =   __SdGetFrameCount,
    .setPlaybackRate = _SetPlaybackRate,
    .control       =   __SdControl,
};

SoundCtrl* SoundDeviceCreate(void* pAudioSink)
{
    SoundCtrlContext* s;

    logd("<SoundDeviceCreate>: init");
    s = (SoundCtrlContext*)malloc(sizeof(SoundCtrlContext));
    if(s == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(s, 0, sizeof(SoundCtrlContext));

    s->base.ops = &mSoundControlOps;
    s->pAudioSink = (MediaPlayerBase::AudioSink*)pAudioSink;
    s->pAudioTrack = NULL;

#if (CONF_ANDROID_MAJOR_VER > 5)
    s->nPlayRate.mSpeed = 1.0f;
#endif

    s->eStatus    = SOUND_STATUS_STOPPED;
#if SAVE_PCM
    s->testpcm = fopen(binpath,"wb");
    if(!s->testpcm)
    {
       loge("open file failed, %s", strerror(errno));
    }
#endif

    pthread_mutex_init(&s->mutex, NULL);

#if defined(CONF_SC_WRITE_ASYNC_MODE)
    if(s->pAudioSink != NULL)
    {
        pthread_mutex_init(&s->cbMutex, NULL);
        pthread_cond_init(&s->cbCond, NULL);
    }
#endif

    return (SoundCtrl*)&s->base;
}
