#ifndef _AUDIO_IN_PORT_H_
#define _AUDIO_IN_PORT_H_

#include "pcm_multi.h"
#include <stdlib.h>

typedef enum
{
	TR_AUDIO_PCM,
	TR_AUDIO_AAC,
	TR_AUDIO_MP3,
	TR_AUDIO_LPCM,
	TR_AUDIO_END
}audioEncodeFormat;

typedef struct
{
	int (*init)(void *hdl,int enable,int format,int samplerate,int channels,
				int samplebits);
	int (*deinit)(void *hdl);
	int (*readData)(void *hdl,void *data,int size);

	int (*clear)(void *hdl);
	int (*getEnable)(void *hdl);
	int (*getFormat)(void *hdl);
	int (*getAudioSampleRate)(void *hdl);
	int (*getAudioChannels)(void *hdl);
	int (*getAudioSampleBits)(void *hdl);
    long long (*getpts)(void *hdl);

	int (*setEnable)(void *hdl,int enable);
	int (*setFormat)(void *hdl,int format);
	int (*setAudioSampleRate)(void *hdl,int samplerate);
	int (*setAudioChannels)(void *hdl,int channels);
	int (*setAudioSampleBits)(void *hdl,int samplebits);

	int (*setparameter)(void *hdl,int cmd,int param);
    long long (*setPts)(void *hdl, int pts);

	int enable;
	audioEncodeFormat format;
	int samplerate;
	int channels;
	int samplebits;
	long long pts;
	void *dupHdl;
}aInPort;

aInPort *CreateAlsaAport(int index);
int DestroyAlsaAport(aInPort *hdl);

#endif
