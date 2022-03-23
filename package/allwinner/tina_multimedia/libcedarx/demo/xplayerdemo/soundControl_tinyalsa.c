/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : soundControl_null.c
 * Description : soundControl for default none output
 * History :
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <string.h>
#include <cdx_log.h>
#include "asoundlib.h"

#include "soundControl.h"
#include <sound/asound.h>


extern struct pcm *pcm_p;
extern int pcm_delay(struct pcm *pcm, snd_pcm_sframes_t *delayp);
typedef struct SoundCtrlContext
{
    SoundCtrl                   base;
    pthread_mutex_t             mutex;
    void*                       pUserData;
	int nBitPerSample;
	int dataType;
	int nChannels;
	int sampleRate;
	int initialized;
	int needDirect;
	int lastDelay;
}SoundCtrlContext;

static void __SdNullRelease(SoundCtrl* s)
{
    int               ret;
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;
    pthread_mutex_destroy(&sc->mutex);
	if(sc->initialized)
	{
		uninit_pcm(PCM_OUT);
	}
    free(sc);
    sc = NULL;

    return;
}

static void __SdNullSetFormat(SoundCtrl* s, CdxPlaybkCfg *cfg)
{
    int               ret;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;
	sc->nBitPerSample= cfg->nBitpersample;
	sc->dataType = cfg->nDataType;
	sc->nChannels = cfg->nChannels;
	sc->sampleRate = cfg->nSamplerate;
	sc->needDirect = cfg->nNeedDirect;
    return;
}


static int __SdNullStart(SoundCtrl* s)
{
    int               ret;
    SoundCtrlContext* sc;
	struct pcm_config config;
	enum pcm_format format;
    sc = (SoundCtrlContext*)s;

	memset(&config,0,sizeof(struct pcm_config));
	printf("channels:%d\n",sc->nChannels);
	config.channels = sc->nChannels;
	config.format = PCM_FORMAT_S16_LE;
	config.period_count = 4;
	config.period_size = 2048;
	config.rate = sc->sampleRate;

	if(sc->initialized == 0)
	{
		init_pcm(PCM_OUT,&config);
		audio_set_play_volume(63);
		sc->initialized = 1;
	}

    return 0;
}


static int __SdNullStop(SoundCtrl* s)
{
    int               ret = 0;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;
	logd("Enter %s:%d\n",__FUNCTION__,__LINE__);
    return ret;
}


static int __SdNullPause(SoundCtrl* s)
{
    int               ret = 0;
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;
	logd("Enter %s:%d\n",__FUNCTION__,__LINE__);
    return 0;
}


static int __SdNullWrite(SoundCtrl* s, void* pData, int nDataSize)
{
    int               ret;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;
	if(sc->initialized)
	{
		pthread_mutex_lock(&sc->mutex);
	pcm_write_api(pData,nDataSize);
		pthread_mutex_unlock(&sc->mutex);
	return nDataSize;
	}
	else
		return 0;
}


//* called at player seek operation.
static int __SdNullReset(SoundCtrl* s)
{
	logd("Enter %s:%d\n",__FUNCTION__,__LINE__);
    return SoundDeviceStop(s);
}


static int __SdNullGetCachedTime(SoundCtrl* s)
{
    SoundCtrlContext* sc;
	snd_pcm_sframes_t delay;
    sc = (SoundCtrlContext*)s;
	int cached_time = 0;

	if(sc->initialized)
	{
		pthread_mutex_lock(&sc->mutex);
		if (pcm_delay(pcm_p,&delay) < 0)
		{
			delay = 0;
		}
		pthread_mutex_unlock(&sc->mutex);
		sc->lastDelay = delay;
		cached_time = ((int)((float) delay * 1000000 / (float) sc->sampleRate));
		return cached_time;
	}
	else
	{
	return 0;
	}
}

static int __SdNullGetFrameCount(SoundCtrl* s)
{
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;

	logd("Enter %s:%d\n",__FUNCTION__,__LINE__);
    return 0;
}

#if defined(CONF_PLAY_RATE)
int _SdNullSetPlaybackRate(SoundCtrl* , const XAudioPlaybackRate *)
{
    return 0;
}
#endif

static SoundControlOpsT mSoundControlOps =
{
    .destroy       =  __SdNullRelease,
    .setFormat     =  __SdNullSetFormat,
    .start         =  __SdNullStart,
    .stop          =  __SdNullStop,
    .pause         =  __SdNullPause,
    .write         =  __SdNullWrite,
    .reset         =  __SdNullReset,
    .getCachedTime =  __SdNullGetCachedTime,
    .getFrameCount =  __SdNullGetFrameCount,
#if defined(CONF_PLAY_RATE)
    .setPlaybackRate = _SdNullSetPlaybackRate,
#endif
};


SoundCtrl* SoundDeviceCreate()
{
    SoundCtrlContext* s;
    logd("==   SoundDeviceInit");
    s = (SoundCtrlContext*)malloc(sizeof(SoundCtrlContext));
    if(s == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(s, 0, sizeof(SoundCtrlContext));

    s->base.ops = &mSoundControlOps;
	s->initialized = 0;

    pthread_mutex_init(&s->mutex, NULL);
    return (SoundCtrl*)s;
}
