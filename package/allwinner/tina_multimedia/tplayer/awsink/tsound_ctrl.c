#define LOG_TAG "tsoundcontrol"
#include "tlog.h"
#include "tsound_ctrl.h"
#include <pthread.h>
#include <sys/time.h>
#include <assert.h>
#include "auGaincom.h"

int BLOCK_MODE = 0;
int NON_BLOCK_MODE = 1;
static int openSoundDevice(SoundCtrlContext* sc,int mode);
static int closeSoundDevice(SoundCtrlContext* sc);
static int setSoundDeviceParams(SoundCtrlContext* sc);
static int openSoundDevice(SoundCtrlContext* sc ,int mode)
{
    int ret = 0;
    TLOGD("openSoundDevice() in default style");
    TP_CHECK(sc);
    if(!sc->alsa_handler){
	//if((ret = snd_pcm_open(&sc->alsa_handler, "default",SND_PCM_STREAM_PLAYBACK ,mode))<0){
		//if use dimix func,use the below parms open the sound card
        if((ret = snd_pcm_open(&sc->alsa_handler, "default",SND_PCM_STREAM_PLAYBACK ,mode))<0)
	{
            TLOGE("open audio device failed:%s, errno = %d",strerror(errno),errno);
            /*
            if(errno == 16){//the device is busy,sleep 2 second and try again.notice:this will not happen when open in dmix mode
		sleep(2);
		if((ret = snd_pcm_open(&sc->alsa_handler, "default",SND_PCM_STREAM_PLAYBACK ,mode))<0){
			TLOGE("second open audio device failed:%s, errno = %d",strerror(errno),errno);
		}
            }
            */
        }
    }else{
	TLOGD("the audio device has been opened");
    }
    return ret;
}

static int closeSoundDevice(SoundCtrlContext* sc)
{
    int ret = 0;
    TLOGD("closeSoundDevice()");
    TP_CHECK(sc);
    if (sc->alsa_handler){
        if ((ret = snd_pcm_close(sc->alsa_handler)) < 0)
        {
            TLOGE("snd_pcm_close failed:%s",strerror(errno));
        }
        else
        {
            sc->alsa_handler = NULL;
            TLOGE("alsa-uninit: pcm closed");
        }
    }
    return ret;
}

static int setSoundDeviceParams(SoundCtrlContext* sc)
{
    int ret = 0;
    TLOGD("setSoundDeviceParams()");
    TP_CHECK(sc);
    sc->bytes_per_sample = sc->nChannelNum*snd_pcm_format_physical_width(sc->alsa_format) / 8;
    sc->alsa_fragcount = 8;//cache count
    sc->chunk_size = 1024;//each cache size,unit : sample
    if ((ret = snd_pcm_hw_params_malloc(&sc->alsa_hwparams)) < 0)
    {
	TLOGE("snd_pcm_hw_params_malloc failed:%s",strerror(errno));
	return ret;
    }

    if ((ret = snd_pcm_hw_params_any(sc->alsa_handler, sc->alsa_hwparams)) < 0)
    {
	TLOGE("snd_pcm_hw_params_any failed:%s",strerror(errno));
	goto SET_PAR_ERR;
    }

    if ((ret = snd_pcm_hw_params_set_access(sc->alsa_handler, sc->alsa_hwparams,
                SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
	TLOGE("snd_pcm_hw_params_set_access failed:%s",strerror(errno));
	goto SET_PAR_ERR;
    }

    if ((ret = snd_pcm_hw_params_test_format(sc->alsa_handler, sc->alsa_hwparams,
            sc->alsa_format)) < 0)
    {
        TLOGE("snd_pcm_hw_params_test_format fail , MSGTR_AO_ALSA_FormatNotSupportedByHardware");
        sc->alsa_format = SND_PCM_FORMAT_S16_LE;
    }

    if ((ret = snd_pcm_hw_params_set_format(sc->alsa_handler, sc->alsa_hwparams,
                sc->alsa_format)) < 0)
    {
        TLOGE("snd_pcm_hw_params_set_format failed:%s",strerror(errno));
        goto SET_PAR_ERR;
    }

    if ((ret = snd_pcm_hw_params_set_channels_near(sc->alsa_handler,
            sc->alsa_hwparams, &sc->nChannelNum)) < 0) {
        TLOGE("snd_pcm_hw_params_set_channels_near failed:%s",strerror(errno));
        goto SET_PAR_ERR;
    }

    if ((ret = snd_pcm_hw_params_set_rate_near(sc->alsa_handler, sc->alsa_hwparams,
            &sc->nSampleRate, NULL)) < 0) {
        TLOGE("snd_pcm_hw_params_set_rate_near failed:%s",strerror(errno));
        goto SET_PAR_ERR;
    }

    if ((ret = snd_pcm_hw_params_set_period_size_near(sc->alsa_handler,
		sc->alsa_hwparams, &sc->chunk_size, NULL)) < 0) {
	TLOGE("snd_pcm_hw_params_set_period_size_near fail , MSGTR_AO_ALSA_UnableToSetPeriodSize");
	goto SET_PAR_ERR;
    } else {
	TLOGD("alsa-init: chunksize set to %ld", sc->chunk_size);
    }

    if ((ret = snd_pcm_hw_params_set_periods_near(sc->alsa_handler,
            sc->alsa_hwparams, (unsigned int*)&sc->alsa_fragcount, NULL)) < 0)
    {
        TLOGE("snd_pcm_hw_params_set_periods_near fail , MSGTR_AO_ALSA_UnableToSetPeriods");
        goto SET_PAR_ERR;
    } else {
        TLOGD("alsa-init: fragcount=%d", sc->alsa_fragcount);
    }

    if ((ret = snd_pcm_hw_params(sc->alsa_handler, sc->alsa_hwparams)) < 0) {
        TLOGE("snd_pcm_hw_params failed:%s",strerror(errno));
        goto SET_PAR_ERR;
    }

    sc->alsa_can_pause = snd_pcm_hw_params_can_pause(sc->alsa_hwparams);

    TLOGD("setSoundDeviceParams():sc->alsa_can_pause = %d",sc->alsa_can_pause);
SET_PAR_ERR:
    snd_pcm_hw_params_free(sc->alsa_hwparams);

    return ret;

}

SoundCtrl* TSoundDeviceCreate(AudioFrameCallback callback,void* pUser){
    SoundCtrlContext* s;
    s = (SoundCtrlContext*)malloc(sizeof(SoundCtrlContext));
    TLOGD("TinaSoundDeviceInit()");
    if(s == NULL)
    {
        TLOGE("malloc SoundCtrlContext fail.");
        return NULL;
    }
    memset(s, 0, sizeof(SoundCtrlContext));
    s->base.ops = &mSoundControlOps;
    s->alsa_access_type = SND_PCM_ACCESS_RW_INTERLEAVED;
    s->nSampleRate = 48000;
    s->nChannelNum = 2;
    s->alsa_format = SND_PCM_FORMAT_S16_LE;
    s->alsa_can_pause = 0;
    s->sound_status = STATUS_STOP;
    s->mVolume = 0;
    s->mAudioframeCallback = callback;
    s->pUserData = pUser;
    pthread_mutex_init(&s->mutex, NULL);
    int ret = openSoundDevice(s, BLOCK_MODE);
    if(ret != 0){
        TLOGD("open sound device fail");
        free(s);
        s = NULL;
        return NULL;
    }
    return (SoundCtrl*)&s->base;
}

void TSoundDeviceDestroy(SoundCtrl* s){
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;
    TP_CHECK(sc);
    pthread_mutex_lock(&sc->mutex);
    TLOGD("TinaSoundDeviceRelease(),close sound device");
    closeSoundDevice(sc);
    pthread_mutex_unlock(&sc->mutex);
    pthread_mutex_destroy(&sc->mutex);
    free(sc);
    sc = NULL;
}

void TSoundDeviceSetFormat(SoundCtrl* s, CdxPlaybkCfg* cfg){
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;
    TP_CHECK(sc);
    pthread_mutex_lock(&sc->mutex);
    TLOGD("TinaSoundDeviceSetFormat(),sc->sound_status == %d",sc->sound_status);
    if(sc->sound_status == STATUS_STOP){
	TLOGD("TinaSoundDeviceSetFormat()");
	sc->nSampleRate = cfg->nSamplerate;
	sc->nChannelNum = cfg->nChannels;
	sc->alsa_format = SND_PCM_FORMAT_S16_LE;
	sc->bytes_per_sample = sc->nChannelNum*snd_pcm_format_physical_width(sc->alsa_format) / 8;
	TLOGD("TinaSoundDeviceSetFormat()>>>sample_rate:%d,channel_num:%d,sc->bytes_per_sample:%d",
		cfg->nSamplerate,cfg->nChannels,sc->bytes_per_sample);
    }
    pthread_mutex_unlock(&sc->mutex);
}

int TSoundDeviceStart(SoundCtrl* s){
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;
    TP_CHECK(sc);
    pthread_mutex_lock(&sc->mutex);
    int ret = 0;
    TLOGD("TinaSoundDeviceStart(): sc->sound_status = %d",sc->sound_status);
    if(sc->sound_status == STATUS_START){
	TLOGD("Sound device already start.");
	pthread_mutex_unlock(&sc->mutex);
	return ret;
    }else if(sc->sound_status == STATUS_PAUSE){
	if(snd_pcm_state(sc->alsa_handler) == SND_PCM_STATE_SUSPENDED){
		TLOGD("MSGTR_AO_ALSA_PcmInSuspendModeTryingResume");
		while((ret = snd_pcm_resume(sc->alsa_handler)) == -EAGAIN){
			sleep(1);
		}
	}
	if(sc->alsa_can_pause){
            if((ret = snd_pcm_pause(sc->alsa_handler, 0))<0){
		TLOGE("snd_pcm_pause failed:%s",strerror(errno));
		pthread_mutex_unlock(&sc->mutex);
		return ret;
            }
        }else{
            if ((ret = snd_pcm_prepare(sc->alsa_handler)) < 0)
            {
		TLOGE("snd_pcm_prepare failed:%s",strerror(errno));
		pthread_mutex_unlock(&sc->mutex);
		return ret;
            }
        }
	sc->sound_status = STATUS_START;
    }
    else if(sc->sound_status == STATUS_STOP){
        ret = setSoundDeviceParams(sc);
        if(ret < 0){
            TLOGE("setSoundDeviceParams fail , ret = %d",ret);
            pthread_mutex_unlock(&sc->mutex);
            return ret;
        }
        sc->sound_status = STATUS_START;
    }
    pthread_mutex_unlock(&sc->mutex);
    return ret;
}

int TSoundDeviceStop(SoundCtrl* s){
    int ret = 0;
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;
    TP_CHECK(sc);
    pthread_mutex_lock(&sc->mutex);
    TLOGD("TinaSoundDeviceStop():sc->sound_status = %d",sc->sound_status);
    if(sc->sound_status == STATUS_STOP)
    {
        TLOGD("Sound device already stopped.");
	pthread_mutex_unlock(&sc->mutex);
	return ret;
    }else{
    if ((ret = snd_pcm_drop(sc->alsa_handler)) < 0)
        {
            TLOGE("snd_pcm_drop():MSGTR_AO_ALSA_PcmPrepareError");
            pthread_mutex_unlock(&sc->mutex);
            return ret;
	}
	if ((ret = snd_pcm_prepare(sc->alsa_handler)) < 0)
        {
            TLOGE("snd_pcm_prepare():MSGTR_AO_ALSA_PcmPrepareError");
            pthread_mutex_unlock(&sc->mutex);
            return ret;
	}
	sc->sound_status = STATUS_STOP;
    }
    pthread_mutex_unlock(&sc->mutex);
    return ret;
}

int TSoundDevicePause(SoundCtrl* s){
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;
    TP_CHECK(sc);
    pthread_mutex_lock(&sc->mutex);
    int ret = 0;
    TLOGD("TinaSoundDevicePause(): sc->sound_status = %d",sc->sound_status);
    if(sc->sound_status == STATUS_START){
	if(sc->alsa_can_pause){
            TLOGD("alsa can pause,use snd_pcm_pause");
            ret = snd_pcm_pause(sc->alsa_handler, 1);
            if(ret<0){
		TLOGE("snd_pcm_pause failed:%s",strerror(errno));
		pthread_mutex_unlock(&sc->mutex);
                return ret;
            }
        }else{
            TLOGD("alsa can not pause,use snd_pcm_drop");
            if ((ret = snd_pcm_drop(sc->alsa_handler)) < 0)
            {
		TLOGE("snd_pcm_drop failed:%s",strerror(errno));
		pthread_mutex_unlock(&sc->mutex);
		return ret;
            }
	}
	sc->sound_status = STATUS_PAUSE;
    }else{
	TLOGD("TinaSoundDevicePause(): pause in an invalid status,status = %d",sc->sound_status);
    }
    pthread_mutex_unlock(&sc->mutex);
    return ret;
}

int TSoundDeviceWrite(SoundCtrl* s, void* pData, int nDataSize){
    int ret = 0;
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;
    TP_CHECK(sc);
    //TLOGD("TinaSoundDeviceWrite:sc->bytes_per_sample = %d\n",sc->bytes_per_sample);
    if(sc->bytes_per_sample == 0){
	sc->bytes_per_sample = 4;
    }
    if(sc->sound_status == STATUS_STOP || sc->sound_status == STATUS_PAUSE)
    {
        return ret;
    }
    //TLOGD("TinaSoundDeviceWrite>>> pData = %p , nDataSize = %d\n",pData,nDataSize);
    int num_frames = nDataSize / sc->bytes_per_sample;
    snd_pcm_sframes_t res = 0;

    if (!sc->alsa_handler)
    {
        TLOGE("MSGTR_AO_ALSA_DeviceConfigurationError");
	return ret;
    }

    if (num_frames == 0){
	TLOGE("num_frames == 0");
	return ret;
    }

    //store the pcm data and callback to tplayer
    SoundPcmData pcmData;
    memset(&pcmData, 0x00, sizeof(SoundPcmData));
    pcmData.pData = pData;
    pcmData.nSize = nDataSize;
    pcmData.samplerate = sc->nSampleRate;
    pcmData.channels = sc->nChannelNum;
    pcmData.accuracy = 16;
    sc->mAudioframeCallback(sc->pUserData,&pcmData);
#if !defined(c600)
    //adjust the pcm data
    AudioGain audioGain;
    audioGain.preamp = sc->mVolume;
    audioGain.InputChan = (int)sc->nChannelNum;
    audioGain.OutputChan = (int)sc->nChannelNum;
    audioGain.InputLen = nDataSize;
    audioGain.InputPtr = (short*)pData;
    audioGain.OutputPtr = (short*)pData;
    int gainRet = tina_do_AudioGain(&audioGain);
    if(gainRet == 0){
	TLOGE("tina_do_AudioGain fail");
    }
#endif

    do {
	//TLOGD("snd_pcm_writei begin,nDataSize = %d",nDataSize);
	res = snd_pcm_writei(sc->alsa_handler, pData, num_frames);
	//TLOGD("snd_pcm_writei finish,res = %ld",res);
	if (res == -EINTR)
        {
		/* nothing to do */
		res = 0;
	} else if (res == -ESTRPIPE)
	{ /* suspend */
            TLOGD("MSGTR_AO_ALSA_PcmInSuspendModeTryingResume");
            while ((res = snd_pcm_resume(sc->alsa_handler)) == -EAGAIN)
		sleep(1);
	}
	if (res < 0)
        {
            TLOGE("MSGTR_AO_ALSA_WriteError,res = %ld",res);
            if ((res = snd_pcm_prepare(sc->alsa_handler)) < 0)
            {
                TLOGE("MSGTR_AO_ALSA_PcmPrepareError");
		return res;
	    }
	}
    } while (res == 0);
    return res < 0 ? res : res * sc->bytes_per_sample;
}

int TSoundDeviceReset(SoundCtrl* s){
	TLOGD("TinaSoundDeviceReset()");
        TP_CHECK(s);
	return TSoundDeviceStop(s);
}

int TSoundDeviceGetCachedTime(SoundCtrl* s){
    int ret = 0;
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;
    TP_CHECK(sc);
    //TLOGD("TinaSoundDeviceGetCachedTime()");
    if (sc->alsa_handler)
    {
        snd_pcm_sframes_t delay = 0;
        //notify:snd_pcm_delay means the cache has how much data(the cache has been filled with pcm data),
        //snd_pcm_avail_update means the free cache,
        if ((ret = snd_pcm_delay(sc->alsa_handler, &delay)) < 0){
		TLOGE("TinaSoundDeviceGetCachedTime(),ret = %d , delay = %ld",ret,delay);
		return 0;
        }
        //TLOGD("TinaSoundDeviceGetCachedTime(),snd_pcm_delay>>> delay = %d",delay);
        //delay = snd_pcm_avail_update(sc->alsa_handler);
        //TLOGD("TinaSoundDeviceGetCachedTime(), snd_pcm_avail_update >>> delay = %d\n",delay);
        if (delay < 0) {
			/* underrun - move the application pointer forward to catch up */
#if SND_LIB_VERSION >= 0x000901 /* snd_pcm_forward() exists since 0.9.0rc8 */
	    snd_pcm_forward(sc->alsa_handler, -delay);
#endif
	    delay = 0;
        }
		//TLOGD("TinaSoundDeviceGetCachedTime(),ret = %d , delay = %ld",ret,delay);
        ret = ((int)((float) delay * 1000000 / (float) sc->nSampleRate));
    }
    return ret;
}

int TSoundDeviceGetFrameCount(SoundCtrl* s){
    //to do
    TP_CHECK(s);
    return 0;
}

int TSoundDeviceSetPlaybackRate(SoundCtrl* s,const XAudioPlaybackRate *rate){
    //to do
    TP_CHECK(s);
    return 0;
}
int TSoundDeviceSetVolume(SoundCtrl* s,int volume){
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;
    if(sc){
        sc->mVolume = volume;
        return 0;
    }else{
        return -1;
    }
}

int TSoundDeviceControl(SoundCtrl* s, int cmd, void* para)
{
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;
    if(sc){
        return 0;
    }else{
        return -1;
    }
}
