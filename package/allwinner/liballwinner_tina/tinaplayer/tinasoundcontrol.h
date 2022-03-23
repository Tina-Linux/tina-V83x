
#ifndef TINA_SOUND_CONTROL_H
#define TINA_SOUND_CONTROL_H

#include <alsa/asoundlib.h>
#include <allwinner/soundControl.h>

namespace aw
{
	typedef void* SoundCtrl;

	typedef enum SoundStatus_t
	{
		STATUS_START = 0,
		STATUS_PAUSE ,
		STATUS_STOP
	}SoundStatus;

	typedef struct SoundCtrlContext_t
	{
		snd_pcm_uframes_t           chunk_size;
	    snd_pcm_format_t            alsa_format;
	    snd_pcm_hw_params_t         *alsa_hwparams;
	    snd_pcm_t                   *alsa_handler;
		snd_pcm_access_t            alsa_access_type;
		snd_pcm_stream_t            alsa_open_mode;
	    unsigned int                nSampleRate;
	    unsigned int                nChannelNum;
		int                         alsa_fragcount;
	    int                         alsa_can_pause;
		size_t                      bytes_per_sample;
		SoundStatus                 sound_status;
		int                         mVolume;
	    pthread_mutex_t             mutex;
	}SoundCtrlContext;

	typedef struct pcm_hw_params_t
	{
		unsigned int samplerate;
		unsigned int channels;
		int format;
	} pcm_hw_params;

	SoundCtrl* TinaSoundDeviceInit(void* pAudioSink);

	void TinaSoundDeviceRelease(SoundCtrl* s);

	void TinaSoundDeviceSetFormat(SoundCtrl* s, unsigned int nSampleRate, unsigned int nChannelNum);

	int TinaSoundDeviceStart(SoundCtrl* s);

	int TinaSoundDeviceStop(SoundCtrl* s);

	int TinaSoundDevicePause(SoundCtrl* s);

	int TinaSoundDeviceWrite(SoundCtrl* s, void* pData, int nDataSize);

	int TinaSoundDeviceReset(SoundCtrl* s);

	int TinaSoundDeviceGetCachedTime(SoundCtrl* s);

	SoundCtrl* TinaSoundDeviceInit_raw(void* raw_data,void* hdeccomp,RawCallback callback);

	void TinaSoundDeviceRelease_raw(SoundCtrl* s);

	void TinaSoundDeviceSetFormat_raw(SoundCtrl* s, unsigned int nSampleRate, unsigned int nChannelNum);

	int TinaSoundDeviceStart_raw(SoundCtrl* s);

	int TinaSoundDeviceStop_raw(SoundCtrl* s);

	int TinaSoundDevicePause_raw(SoundCtrl* s);

	int TinaSoundDeviceWrite_raw(SoundCtrl* s, void* pData, int nDataSize);

	int TinaSoundDeviceReset_raw(SoundCtrl* s);

	int TinaSoundDeviceGetCachedTime_raw(SoundCtrl* s);

	int TinaSoundDeviceSetVolume(SoundCtrl* s, float volume);

	int TinaSoundDeviceGetVolume(SoundCtrl* s, float *volume);

	int TinaSoundDeviceSetCallback(SoundCtrl* s, SndCallback callback, void* pUserData);

}

#endif
