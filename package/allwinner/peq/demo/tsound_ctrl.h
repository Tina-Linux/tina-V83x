
#ifndef TINA_SOUND_CONTROL_H
#define TINA_SOUND_CONTROL_H

#include <alsa/asoundlib.h>
//#include "soundControl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*AudioFrameCallback)(void* pUser, void* para);

typedef struct SoundPcmData
{
    unsigned char* pData;
    int   nSize;
    unsigned int samplerate;
    unsigned int channels;
    int accuracy;
} SoundPcmData;

typedef enum SoundStatus_t
{
    STATUS_START = 0,
    STATUS_PAUSE ,
    STATUS_STOP
}SoundStatus;

typedef struct SoundCtrlContext_t
{
    void                        *base;
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
    AudioFrameCallback mAudioframeCallback;
    void*                pUserData;
}SoundCtrlContext;

void* TSoundDeviceCreate(AudioFrameCallback callback,void* pUser, int rat, int ch);

void TSoundDeviceDestroy(void* s);

//void TSoundDeviceSetFormat(void* s,CdxPlaybkCfg* cfg);

int TSoundDeviceStart(void* s);

int TSoundDeviceStop(void* s);

int TSoundDevicePause(void* s);

int TSoundDeviceWrite(void* s, void* pData, int nDataSize);

int TSoundDeviceReset(void* s);

int TSoundDeviceGetCachedTime(void* s);
int TSoundDeviceGetFrameCount(void* s);
//int TSoundDeviceSetPlaybackRate(void* s,const XAudioPlaybackRate *rate);

int TSoundDeviceSetVolume(void* s,int volume);
#ifdef __cplusplus
}
#endif

#endif
