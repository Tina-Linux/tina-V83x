#include"audioInPort.h"

static int alsaAportReadData(void *hdl,void *data,int size)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	return MultiPCMRead((MultiPCM *)hdlAudio->dupHdl,data,size,0);
}

static int alsaAportClear(void *hdl)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	return MultiPCMClear((MultiPCM *)hdlAudio->dupHdl,0);
}

static int alsaAportClear2(void *hdl)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	return MultiPCMClear((MultiPCM *)hdlAudio->dupHdl,1);
}


static int alsaAportReadData2(void *hdl,void *data,int size)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	return MultiPCMRead((MultiPCM *)hdlAudio->dupHdl,data,size,1);
}

static int alsaAportInit(void *hdl,int enable,int format,int samplerate,
						int channels,int samplebits)
{
	aInPort *hdlAudio = (aInPort *)hdl;

	struct pcm_config *config = calloc(1, sizeof(struct pcm_config));

	if(hdlAudio == NULL)
	{
		free(config);
		return -1;
	}
	config->channels = channels;
	config->rate = samplerate;
	config->period_size = 1024;
	config->period_count = 4;
	if(format == TR_AUDIO_PCM)
		config->format = PCM_FORMAT_S16_LE;
	else
	{
		free(config);
		return -1;
	}
	config->start_threshold = 0;
	config->stop_threshold = 0;
	config->silence_threshold = 0;

	hdlAudio->samplebits = samplebits;
	hdlAudio->channels = channels;
	hdlAudio->samplerate = samplerate;
	hdlAudio->enable = enable;
	hdlAudio->format = format;
	hdlAudio->pts = 0;
	hdlAudio->dupHdl = (void *)MultiPCMInit(config);
	if(hdlAudio->dupHdl == NULL)
	{
		free(config);
		return -1;
	}
	else
		return 0;

	return 0;
}

static int alsaAportGetEnable(void *hdl)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	return hdlAudio->enable;
}

static int alsaAportGetFormat(void *hdl)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	return hdlAudio->format;
}

static int alsaAportGetSamplerate(void *hdl)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	return hdlAudio->samplerate;
}

static int alsaAportGetAudioChannels(void *hdl)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	return hdlAudio->channels;
}

static int alsaAportGetAudioSamplebits(void *hdl)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	return hdlAudio->samplebits;
}

static long long alsaAportgetPts(void *hdl)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	hdlAudio->pts+=1024*1000/hdlAudio->samplerate;
	return hdlAudio->pts;
}


static int alsaAportSetEnable(void *hdl,int enable)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	hdlAudio->enable = enable;
	return 0;
}

static int alsaAportSetFormat(void *hdl,int format)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	hdlAudio->format = format;
	return 0;
}

static int alsaAportSetSamplerate(void *hdl,int samplerate)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	hdlAudio->samplerate = samplerate;
	return 0;
}

static int alsaAportSetAudioChannels(void *hdl,int channels)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	hdlAudio->channels = channels;
	return 0;
}

static int alsaAportSetAudioSamplebits(void *hdl,int samplebits)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	hdlAudio->samplebits = samplebits;
	return 0;
}

static long long alsaAportSetPts(void *hdl,int pts)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	hdlAudio->pts = pts;
	return 0;
}


static int alsaAportDeinit(void *hdl)
{
	aInPort *hdlAudio = (aInPort *)hdl;
	if(hdlAudio == NULL)
		return -1;
	return MultiPCMDeInit((MultiPCM *)hdlAudio->dupHdl);
}

aInPort *CreateAlsaAport(int index)
{
	aInPort *aport = (aInPort *)malloc(sizeof(aInPort));

	if(aport == NULL)
		return NULL;
	if(index == 0)
	{
		aport->readData = alsaAportReadData;
		aport->clear = alsaAportClear;
	}
	else
	{
		aport->readData = alsaAportReadData2;
		aport->clear = alsaAportClear2;
	}

	aport->getEnable = alsaAportGetEnable;
	aport->getFormat = alsaAportGetFormat;
	aport->getAudioSampleRate = alsaAportGetSamplerate;
	aport->getAudioChannels = alsaAportGetAudioChannels;
	aport->getAudioSampleBits = alsaAportGetAudioSamplebits;
	aport->init = alsaAportInit;
	aport->getpts = alsaAportgetPts;
	aport->deinit = alsaAportDeinit;

	aport->setEnable = alsaAportSetEnable;
	aport->setFormat = alsaAportSetFormat;
	aport->setAudioSampleRate = alsaAportSetSamplerate;
	aport->setAudioChannels = alsaAportSetAudioChannels;
	aport->setAudioSampleBits = alsaAportSetAudioSamplebits;
	aport->setPts = alsaAportSetPts;
	//aport.setEnable(&aport,1);
	//aport.setFormat(&aport,TR_AUDIO_PCM);
	//aport.setAudioSampleRate(&aport,44100);
	//aport.setAudioChannels(&aport,2);
	//aport.setAudioBitRate(&aport,8000);
	return aport;
}

int DestroyAlsaAport(aInPort *hdl)
{
	if(hdl != NULL)
		free(hdl);

    return 0;
}
