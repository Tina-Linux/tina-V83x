
#include <utils/Errors.h>

#include "soundControl.h"
#include "log.h"
#include <media/AudioTrack.h>
#include <pthread.h>

typedef struct SoundCtrlContext
{
    MediaPlayerBase::AudioSink* pAudioSink;

    sp<AudioTrack>              pAudioTrack;//* new an audio track if pAudioSink not set.
    unsigned int                nSampleRate;
    unsigned int                nChannelNum;

    int64_t                     nDataSizePlayed;
    int64_t                     nFramePosOffset;
    unsigned int                nFrameSize;
    unsigned int                nLastFramePos;

    enum EPLAYERSTATUS          eStatus;
    pthread_mutex_t             mutex;
	float						volume;

}SoundCtrlContext;

static int SoundDeviceStop_l(SoundCtrlContext* sc);

SoundCtrl* SoundDeviceInit(void* pAudioSink)
{
    SoundCtrlContext* s;

    logd("<SoundCtl>: init");
    s = (SoundCtrlContext*)malloc(sizeof(SoundCtrlContext));
    if(s == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(s, 0, sizeof(SoundCtrlContext));
    s->volume = -1.0;
    s->pAudioSink = (MediaPlayerBase::AudioSink*)pAudioSink;
    s->eStatus    = PLAYER_STATUS_STOPPED;
    pthread_mutex_init(&s->mutex, NULL);

    return (SoundCtrl*)s;
}


void SoundDeviceRelease(SoundCtrl* s)
{
    SoundCtrlContext* sc;

    logd("<SoundCtl>: release");
    sc = (SoundCtrlContext*)s;

    pthread_mutex_lock(&sc->mutex);
    if(sc->eStatus != PLAYER_STATUS_STOPPED)
        SoundDeviceStop_l(sc);
    pthread_mutex_unlock(&sc->mutex);

    pthread_mutex_destroy(&sc->mutex);

    free(sc);
    return;
}

int SoundDeviceSetCallback(SoundCtrl* s, SndCallback callback, void* pUserData)
{
	SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    return 0;
}


void SoundDeviceSetFormat(SoundCtrl* s, unsigned int nSampleRate, unsigned int nChannelNum)
{
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    pthread_mutex_lock(&sc->mutex);

    if(sc->eStatus != PLAYER_STATUS_STOPPED)
    {
        loge("Sound device not int stop status, can not set audio params.");
        abort();
    }

    sc->nSampleRate = nSampleRate;
    sc->nChannelNum = nChannelNum;

    pthread_mutex_unlock(&sc->mutex);

    return;
}


int SoundDeviceStart(SoundCtrl* s)
{
    SoundCtrlContext* sc;
    status_t          err;
    unsigned int      nFramePos;

    sc = (SoundCtrlContext*)s;
    logd("<SoundCtl>: start");
    pthread_mutex_lock(&sc->mutex);

    if(sc->eStatus == PLAYER_STATUS_STARTED)
    {
        logw("Sound device already started.");
        pthread_mutex_unlock(&sc->mutex);
        return -1;
    }

    if(sc->eStatus == PLAYER_STATUS_STOPPED)
    {
        if(sc->pAudioSink != NULL)
        {
            err = sc->pAudioSink->open(sc->nSampleRate,
                                       sc->nChannelNum,
                                       CHANNEL_MASK_USE_CHANNEL_ORDER,
                                       AUDIO_FORMAT_PCM_16_BIT,
                                       DEFAULT_AUDIOSINK_BUFFERCOUNT,
                                       NULL,    //* no callback mode.
                                       NULL,
                                       AUDIO_OUTPUT_FLAG_NONE);

            if(err != OK)
            {
                pthread_mutex_unlock(&sc->mutex);
                return -1;
            }

			unsigned int tmp = 0;
			sc->pAudioSink->getPosition(&tmp);
			if(tmp != 0)
			{
				sc->pAudioSink->pause();
				sc->pAudioSink->flush();
			}


            sc->nFrameSize = sc->pAudioSink->frameSize();
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
                                 AUDIO_FORMAT_PCM_16_BIT,
                                 (sc->nChannelNum == 2) ? AUDIO_CHANNEL_OUT_STEREO : AUDIO_CHANNEL_OUT_MONO);

            if(sc->pAudioTrack->initCheck() != OK)
            {
                loge("audio track initCheck() return fail.");
                sc->pAudioTrack.clear();
                sc->pAudioTrack = NULL;
                pthread_mutex_unlock(&sc->mutex);
                return -1;
            }

            sc->nFrameSize = sc->pAudioTrack->frameSize();
#if (CONFIG_OS_VERSION != OPTION_OS_VERSION_ANDROID_4_2) //* for compile
			if(sc->volume != -1.0)
			{
				sc->pAudioTrack->setVolume(sc->volume);
			}
#endif
        }

	    sc->nDataSizePlayed = 0;
        sc->nFramePosOffset = 0;
        sc->nLastFramePos   = 0;
    }

    if(sc->pAudioSink != NULL)
        sc->pAudioSink->start();
    else
        sc->pAudioTrack->start();

    sc->eStatus = PLAYER_STATUS_STARTED;
    pthread_mutex_unlock(&sc->mutex);

    return 0;
}


int SoundDeviceStop(SoundCtrl* s)
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
    if(sc->eStatus == PLAYER_STATUS_STOPPED)
    {
        logw("Sound device already stopped.");
        return 0;
    }

    if(sc->pAudioSink != NULL)
    {
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
    sc->eStatus         = PLAYER_STATUS_STOPPED;
    return 0;
}


int SoundDevicePause(SoundCtrl* s)
{
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;
    logd("<SoundCtl>: pause");
    pthread_mutex_lock(&sc->mutex);

    if(sc->eStatus != PLAYER_STATUS_STARTED)
    {
        logw("Invalid pause operation, sound device not in start status.");
        pthread_mutex_unlock(&sc->mutex);
        return -1;
    }

    if(sc->pAudioSink != NULL)
        sc->pAudioSink->pause();
    else
        sc->pAudioTrack->pause();

    sc->eStatus = PLAYER_STATUS_PAUSED;
    pthread_mutex_unlock(&sc->mutex);
    return 0;
}


int SoundDeviceWrite(SoundCtrl* s, void* pData, int nDataSize)
{
    int               nWritten;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    pthread_mutex_lock(&sc->mutex);

    if(sc->eStatus == PLAYER_STATUS_STOPPED || sc->eStatus == PLAYER_STATUS_PAUSED)
    {
        pthread_mutex_unlock(&sc->mutex);
        return 0;
    }

    if(sc->pAudioSink != NULL)
        nWritten = sc->pAudioSink->write(pData, nDataSize);
    else
        nWritten = sc->pAudioTrack->write(pData, nDataSize);

    if(nWritten < 0)
        nWritten = 0;
    else
        sc->nDataSizePlayed += nWritten;

    pthread_mutex_unlock(&sc->mutex);

    return nWritten;
}


//* called at player seek operation.
int SoundDeviceReset(SoundCtrl* s)
{
    logd("<SoundCtl>: reset");
    return SoundDeviceStop(s);
}


int SoundDeviceGetCachedTime(SoundCtrl* s)
{
	unsigned int      nFramePos;
	int64_t           nCachedFrames;
	int64_t           nCachedTimeUs;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    pthread_mutex_lock(&sc->mutex);

    if(sc->eStatus == PLAYER_STATUS_STOPPED)
    {
        pthread_mutex_unlock(&sc->mutex);
        return 0;
    }

	if(sc->pAudioSink != NULL)
	    sc->pAudioSink->getPosition(&nFramePos);
	else
	    sc->pAudioTrack->getPosition(&nFramePos);

	if(sc->nFrameSize == 0)
	{
	    loge("nFrameSize == 0.");
	    abort();
	}

    if(nFramePos < sc->nLastFramePos)
        sc->nFramePosOffset += 0x100000000;
	nCachedFrames = sc->nDataSizePlayed/sc->nFrameSize - nFramePos - sc->nFramePosOffset;
	nCachedTimeUs = nCachedFrames*1000000/sc->nSampleRate;

	logi("nDataSizePlayed = %lld, nFrameSize = %d, nFramePos = %u, nLastFramePos = %u, nFramePosOffset = %lld",
	    sc->nDataSizePlayed, sc->nFrameSize, nFramePos, sc->nLastFramePos, sc->nFramePosOffset);

	logi("nCachedFrames = %lld, nCachedTimeUs = %lld, nSampleRate = %d",
	    nCachedFrames, nCachedTimeUs, sc->nSampleRate);

    sc->nLastFramePos = nFramePos;
    pthread_mutex_unlock(&sc->mutex);

    return (int)nCachedTimeUs;
}
#if (CONFIG_OS_VERSION != OPTION_OS_VERSION_ANDROID_4_2) //* for compile
int SoundDeviceSetVolume(SoundCtrl* s, float volume)
{

	logd("SoundDeviceSetVolume, volume=%f", volume);
    SoundCtrlContext* sc = (SoundCtrlContext*)s;
	if(volume == -1.0)
	{
		logw("volume == -1.0");
		return 0;
	}
    pthread_mutex_lock(&sc->mutex);
	sc->volume = volume;
	if(sc->pAudioTrack == NULL)
	{
		logw("sc->pAudioTrack == NULL");
		pthread_mutex_unlock(&sc->mutex);
		return -1;
	}
	int ret = (int)sc->pAudioTrack->setVolume(sc->volume);
    pthread_mutex_unlock(&sc->mutex);
	return ret;
}
int SoundDeviceGetVolume(SoundCtrl* s, float *volume)
{
    SoundCtrlContext* sc = (SoundCtrlContext*)s;
	*volume = sc->volume;
	/*
    pthread_mutex_lock(&sc->mutex);
	if(sc->pAudioTrack == NULL)
	{
		logw("sc->pAudioTrack == NULL");
		pthread_mutex_unlock(&sc->mutex);
		return -1;
	}
	int ret = (int)sc->pAudioTrack->getVolume(0, volume);
    pthread_mutex_unlock(&sc->mutex);
    */
	return 0;
}
#else
int SoundDeviceSetVolume(SoundCtrl* s, float volume)
{
    return 0;
}
int SoundDeviceGetVolume(SoundCtrl* s, float *volume)
{
    return 0;
}
#endif


SoundControlOpsT mSoundControlOps =
{
	SoundDeviceInit:			SoundDeviceInit,
	SoundDeviceRelease:			SoundDeviceRelease,
	SoundDeviceSetFormat:			SoundDeviceSetFormat,
	SoundDeviceStart:			SoundDeviceStart,
	SoundDeviceStop:			SoundDeviceStop,
	SoundDevicePause:			SoundDevicePause,
	SoundDeviceWrite:			SoundDeviceWrite,
	SoundDeviceReset:			SoundDeviceReset,
	SoundDeviceGetCachedTime:		SoundDeviceGetCachedTime,
	SoundDeviceInit_raw:			NULL,
	SoundDeviceRelease_raw:			NULL,
	SoundDeviceSetFormat_raw:		NULL,
	SoundDeviceStart_raw:			NULL,
	SoundDeviceStop_raw:			NULL,
	SoundDevicePause_raw:			NULL,
	SoundDeviceWrite_raw:			NULL,
	SoundDeviceReset_raw:			NULL,
	SoundDeviceGetCachedTime_raw:	NULL,
	SoundDeviceSetVolume:			SoundDeviceSetVolume,
	SoundDeviceGetVolume:		SoundDeviceGetVolume,
	SoundDeviceSetCallback:		SoundDeviceSetCallback
};
