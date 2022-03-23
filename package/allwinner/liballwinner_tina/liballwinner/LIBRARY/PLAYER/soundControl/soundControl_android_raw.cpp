#include <utils/Errors.h>

#include "soundControl.h"
#include "asoundlib.h"

#include "log.h"
#include <pthread.h>

//#define CDX_SUPPROT_IEC61937

#ifdef CDX_SUPPROT_IEC61937
#include "IEC61937.h"
#endif

enum SND_OUTPUT_MODE
{
	SND_PCM = 0,
	SND_HDMI_RAW,
	SND_SPDIF_RAW
};

enum SND_AUIDO_RAW_DATA_TYPE
{
	SND_AUDIO_RAW_DATA_UNKOWN = 0,
	SND_AUDIO_RAW_DATA_PCM = 1,
	SND_AUDIO_RAW_DATA_AC3 = 2,
	SND_AUDIO_RAW_DATA_MPEG1 = 3,
	SND_AUDIO_RAW_DATA_MP3 = 4,
	SND_AUDIO_RAW_DATA_MPEG2 = 5,
	SND_AUDIO_RAW_DATA_AAC = 6,
	SND_AUDIO_RAW_DATA_DTS = 7,
	SND_AUDIO_RAW_DATA_ATRAC = 8,
	SND_AUDIO_RAW_DATA_ONE_BIT_AUDIO = 9,
	SND_AUDIO_RAW_DATA_DOLBY_DIGITAL_PLUS = 10,
	SND_AUDIO_RAW_DATA_DTS_HD = 11,
	SND_AUDIO_RAW_DATA_MAT = 12,
	SND_AUDIO_RAW_DATA_DST = 13,
	SND_AUDIO_RAW_DATA_WMAPRO = 14
};

typedef struct Snd_raw_data
{
	int nRawDataFlag;//UI set 0:pcm;1:hdmi raw data;2:spdif raw data;
	int RawInitFlag;//if init flag 0:no init raw;1:init raw
	int channels;
	int samplerate;
	int bitpersample;
	int raw_flag;

}snd_raw_data;

typedef struct SoundCtrlContext_raw
{
    snd_raw_data				raw_data;
    int64_t                     nDataSizePlayed;
    int64_t                     nFramePosOffset;
    unsigned int                nFrameSize;
    unsigned int                nLastFramePos;
	enum EPLAYERSTATUS          eStatus;
    pthread_mutex_t             mutex_raw;
    /*for comunication with outer*/
	RawCallback                 RawplayCallback;
	void*                       hcomp;
	/*pcm API use*/
	struct pcm_config config;
	unsigned int device;
	struct pcm *pcm;

#ifdef CDX_SUPPROT_IEC61937
	unsigned char* aout_buf61937;
#endif
}SoundCtrlContext_raw;


static int SoundDeviceStop_raw_l(SoundCtrlContext_raw* sc);

static int SoundDeviceSetRawFlag(int card, int raw_flag)
{
    logd("SoundDeviceSetRawFlag(card=%d, raw_flag=%d)", card, raw_flag);
    struct mixer *mixer = mixer_open(card);
    if (!mixer) {
        ALOGE("Unable to open the mixer, aborting.");
        return -1;
    }
    const char *control_name = (card == 1) ? "hdmi audio format Function" : "spdif audio format Function";
    const char *control_value = (raw_flag==SND_AUDIO_RAW_DATA_AC3) ? "AC3" : (raw_flag==SND_AUDIO_RAW_DATA_DTS) ? "DTS" : "pcm";
    struct mixer_ctl *audio_format = mixer_get_ctl_by_name(mixer, control_name);
    if (audio_format)
        mixer_ctl_set_enum_by_string(audio_format, control_value);
    mixer_close(mixer);
    return 0;
}

SoundCtrl* SoundDeviceInit_raw(void* raw,void* hdeccomp,RawCallback callback)
{
    SoundCtrlContext_raw* s;
	snd_raw_data* raw_data = (snd_raw_data*)raw;
	if(!raw_data)
	{
		loge("raw_data is null!!");
		return NULL;
	}
	if(raw_data->RawInitFlag != 1)
	{
        logv("not raw mode");
		return NULL;
	}
    s = (SoundCtrlContext_raw*)malloc(sizeof(SoundCtrlContext_raw));
	if(s == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
	memset(s, 0, sizeof(SoundCtrlContext_raw));
	memcpy(&s->raw_data ,raw_data,sizeof(snd_raw_data));



	s->config.channels = s->raw_data.channels;
	s->config.rate = s->raw_data.samplerate;
	s->config.raw_flag = s->raw_data.raw_flag;
    switch(s->raw_data.bitpersample)
    {
	case 32:
			s->config.format = PCM_FORMAT_S32_LE;
			break;
		case 24:
			s->config.format = PCM_FORMAT_S24_LE;
			break;
		default:
			s->config.format = PCM_FORMAT_S16_LE;
			break;
	}
	if(s->raw_data.samplerate == 192000)
	{
		s->config.period_size = 2048;
		s->config.period_count = 8;
	}
	else
	{
		s->config.period_size = 1024;
		s->config.period_count = 4;
	}
	s->config.start_threshold = 0;
	s->config.stop_threshold = 0;
	s->config.silence_threshold = 0;

	s->hcomp = hdeccomp;
	s->RawplayCallback = callback;
    s->eStatus    = PLAYER_STATUS_STOPPED;
    pthread_mutex_init(&s->mutex_raw, NULL);
    return (SoundCtrl*)s;
}


void SoundDeviceRelease_raw(SoundCtrl* s)
{
    SoundCtrlContext_raw* sc;
    sc = (SoundCtrlContext_raw*)s;

    pthread_mutex_lock(&sc->mutex_raw);
    if(sc->eStatus != PLAYER_STATUS_STOPPED)
        SoundDeviceStop_raw_l(sc);
    pthread_mutex_unlock(&sc->mutex_raw);

    pthread_mutex_destroy(&sc->mutex_raw);

    free(sc);
    return;
}


void SoundDeviceSetFormat_raw(SoundCtrl* s, unsigned int nSampleRate, unsigned int nChannelNum)
{
	SoundCtrlContext_raw* sc;
	sc = (SoundCtrlContext_raw*)s;

	CEDARX_UNUSE(nSampleRate);
	CEDARX_UNUSE(nChannelNum);

    pthread_mutex_lock(&sc->mutex_raw);

	if(sc->eStatus != PLAYER_STATUS_STOPPED)
	{
	    loge("Sound device not int stop status, can not set audio params.");
	    abort();
	}
	/*do no thing*/
	pthread_mutex_unlock(&sc->mutex_raw);
	return;
}


int SoundDeviceStart_raw(SoundCtrl* s)
{
    SoundCtrlContext_raw* sc;
    status_t          err;
    unsigned int      nFramePos;
    int				  param_occupy[3]={1,0,0};
	int			  param_release[3]={1,0,0};
    sc = (SoundCtrlContext_raw*)s;

    pthread_mutex_lock(&sc->mutex_raw);

    if(sc->eStatus == PLAYER_STATUS_STARTED)
    {
        logw("raw pcm device has started!!");
		pthread_mutex_unlock(&sc->mutex_raw);
        return -1;
    }

    if(sc->pcm == NULL && sc->raw_data.RawInitFlag == 1)
    {
        sc->RawplayCallback(sc->hcomp,(void*)param_occupy);
#ifdef CDX_SUPPROT_IEC61937
		if(sc->raw_data.nRawDataFlag == SND_HDMI_RAW)
		{
			if(sc->aout_buf61937 == NULL)
			{
				sc->aout_buf61937 = (unsigned char*)malloc(128*1024);
				memset(sc->aout_buf61937,0,sizeof(sc->aout_buf61937));
			}
			//PROP_RAWDATA_MODE_SPDIF_RAW
		}
		else if (sc->raw_data.nRawDataFlag == SND_SPDIF_RAW)
		{
		//TODO
		}
#endif
#if (SOUND_DEVICE_SET_RAW_FLAG == 1)
		SoundDeviceSetRawFlag(sc->raw_data.nRawDataFlag, sc->config.raw_flag);
#endif
		sc->pcm = pcm_open(sc->raw_data.nRawDataFlag, sc->device, PCM_OUT, &(sc->config));
		if (!sc->pcm || !pcm_is_ready(sc->pcm)) {
			logw("Unable to open PCM device %u (%s)\n",sc->device, pcm_get_error(sc->pcm));
			sc->RawplayCallback(sc->hcomp,(void*)param_release);
			pthread_mutex_unlock(&sc->mutex_raw);
            return -1;
		}
	}
	sc->nFrameSize = (sc->raw_data.bitpersample/8)*sc->raw_data.channels;

	sc->nDataSizePlayed = 0;
	sc->nFramePosOffset = 0;
	sc->nLastFramePos   = 0;
	sc->eStatus = PLAYER_STATUS_STARTED;
	pthread_mutex_unlock(&sc->mutex_raw);

	return 0;
}


int SoundDeviceStop_raw(SoundCtrl* s)
{
	int               ret;
	SoundCtrlContext_raw* sc;
	sc = (SoundCtrlContext_raw*)s;

    pthread_mutex_lock(&sc->mutex_raw);
    ret = SoundDeviceStop_raw_l(sc);
    pthread_mutex_unlock(&sc->mutex_raw);

    return ret;
}


static int SoundDeviceStop_raw_l(SoundCtrlContext_raw* sc)
{
    int    param_occupy[3]={0,0,0};
	int    param_release[3]={0,0,0};
	if(sc->eStatus == PLAYER_STATUS_STOPPED)
    {
        logw("Sound device already stopped.");
        return -1;
    }

    if(sc->pcm != NULL)
    {
		logv("release the device");
#ifdef CDX_SUPPROT_IEC61937
		if(sc->aout_buf61937)
		{
			free(sc->aout_buf61937);
			sc->aout_buf61937 = NULL;
		}
#endif
		pcm_close(sc->pcm);
		sc->pcm = 0;
		sc->RawplayCallback(sc->hcomp,(void*)param_release);
    }
    sc->nDataSizePlayed = 0;
    sc->nFramePosOffset = 0;
    sc->nLastFramePos   = 0;
    sc->nFrameSize      = 0;
    sc->eStatus         = PLAYER_STATUS_STOPPED;
    return 0;
}


int SoundDevicePause_raw(SoundCtrl* s)
{
	SoundCtrlContext_raw* sc;
	sc = (SoundCtrlContext_raw*)s;

    pthread_mutex_lock(&sc->mutex_raw);
	if(sc->eStatus != PLAYER_STATUS_STARTED)
    {
        logw("Invalid pause operation, sound device not in start status.");
        pthread_mutex_unlock(&sc->mutex_raw);
        return -1;
    }
	if(sc->pcm != NULL)
    {
        loge("player pause  we need pcm stop!");
		pcm_stop(sc->pcm);
	}
    sc->eStatus = PLAYER_STATUS_PAUSED;
    pthread_mutex_unlock(&sc->mutex_raw);
    return 0;
}


int SoundDeviceWrite_raw(SoundCtrl* s, void* pData, int nDataSize)
{
    int               nWritten=0;
	int framesize = 0;
	int * tempint = NULL;
	short * tempshort = NULL;
    SoundCtrlContext_raw* sc;
	sc = (SoundCtrlContext_raw*)s;

    pthread_mutex_lock(&sc->mutex_raw);
	if(sc->eStatus == PLAYER_STATUS_STOPPED || sc->eStatus == PLAYER_STATUS_PAUSED)
    {
        pthread_mutex_unlock(&sc->mutex_raw);
        return 0;
    }

    if(sc->raw_data.RawInitFlag)
	{
		//PROP_RAWDATA_MODE_HDMI_RAW
		//loge("raw mode   write size:%d",*pBufSize);
		switch(sc->raw_data.bitpersample)
		{
			case 24:
				framesize = sc->config.channels*3;
				break;
			case 32:
				framesize = sc->config.channels*4;
				break;
			default:
				framesize = sc->config.channels*2;
				break;
		}
        if((nDataSize)%framesize!=0)
        {
            nDataSize = nDataSize / framesize;
			nDataSize = nDataSize * framesize;
		}
#ifdef CDX_SUPPROT_IEC61937
		if((sc->raw_data.nRawDataFlag == SND_HDMI_RAW)&&(sc->config.raw_flag == SND_AUDIO_RAW_DATA_AC3||sc->config.raw_flag == SND_AUDIO_RAW_DATA_DTS))
		{
			add61937Head((void*)sc->aout_buf61937,(void*)pData, nDataSize);
			logv("!!!!!!!!!raw write:%d",nDataSize);
			if (pcm_write(sc->pcm, sc->aout_buf61937, 2*(nDataSize)))
			{
				logw("Error playing sample\n");
				nWritten = 0;
			}
		}
		else
#endif
		{
		    //PROP_RAWDATA_MODE_SPDIF_RAW
			if (pcm_write(sc->pcm, pData, nDataSize))
		    {
				logw("Error playing sample\n");
				nWritten = 0;
			}
		}
		nWritten = nDataSize;
	}
	else
	{
        //loge("normal mode   write size:%d",*wirtesize);
	}

    if(nWritten < 0)
        nWritten = 0;
    else
        sc->nDataSizePlayed += nWritten;

    pthread_mutex_unlock(&sc->mutex_raw);
	return nWritten;
}


//* called at player seek operation.
int SoundDeviceReset_raw(SoundCtrl* s)
{
    return SoundDeviceStop_raw(s);
}


int SoundDeviceGetCachedTime_raw(SoundCtrl* s)
{
	unsigned int      nFramePos;
	int64_t           nCachedFrames;
	int64_t           nCachedTimeUs;
    SoundCtrlContext_raw* sc;

    sc = (SoundCtrlContext_raw*)s;

    pthread_mutex_lock(&sc->mutex_raw);

    if(sc->eStatus == PLAYER_STATUS_STOPPED)
    {
        pthread_mutex_unlock(&sc->mutex_raw);
        return 0;
    }
	/*do nothing*/
	pthread_mutex_unlock(&sc->mutex_raw);
	return 0;
}


SoundControlOpsT mRawSoundControlOps =
{
	SoundDeviceInit:			NULL,
	SoundDeviceRelease:			NULL,
	SoundDeviceSetFormat:			NULL,
	SoundDeviceStart:			NULL,
	SoundDeviceStop:			NULL,
	SoundDevicePause:			NULL,
	SoundDeviceWrite:			NULL,
	SoundDeviceReset:			NULL,
	SoundDeviceGetCachedTime:		NULL,
	SoundDeviceInit_raw:			SoundDeviceInit_raw,
	SoundDeviceRelease_raw:			SoundDeviceRelease_raw,
	SoundDeviceSetFormat_raw:		SoundDeviceSetFormat_raw,
	SoundDeviceStart_raw:			SoundDeviceStart_raw,
	SoundDeviceStop_raw:			SoundDeviceStop_raw,
	SoundDevicePause_raw:			SoundDevicePause_raw,
	SoundDeviceWrite_raw:			SoundDeviceWrite_raw,
	SoundDeviceReset_raw:			SoundDeviceReset_raw,
	SoundDeviceGetCachedTime_raw:	SoundDeviceGetCachedTime_raw,
	SoundDeviceSetVolume:			SoundDeviceSetVolume,
	SoundDeviceGetVolume:		SoundDeviceGetVolume,
	SoundDeviceSetCallback:		SoundDeviceSetCallback
};
