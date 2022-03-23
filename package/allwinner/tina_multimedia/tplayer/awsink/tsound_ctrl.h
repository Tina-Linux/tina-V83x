
#ifndef TINA_SOUND_CONTROL_H
#define TINA_SOUND_CONTROL_H

#include <alsa/asoundlib.h>
#include "soundControl.h"

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
    SoundCtrl                   base;
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

SoundCtrl* TSoundDeviceCreate(AudioFrameCallback callback,void* pUser);

void TSoundDeviceDestroy(SoundCtrl* s);

void TSoundDeviceSetFormat(SoundCtrl* s,CdxPlaybkCfg* cfg);

int TSoundDeviceStart(SoundCtrl* s);

int TSoundDeviceStop(SoundCtrl* s);

int TSoundDevicePause(SoundCtrl* s);

int TSoundDeviceWrite(SoundCtrl* s, void* pData, int nDataSize);

int TSoundDeviceReset(SoundCtrl* s);

int TSoundDeviceGetCachedTime(SoundCtrl* s);
int TSoundDeviceGetFrameCount(SoundCtrl* s);
int TSoundDeviceSetPlaybackRate(SoundCtrl* s,const XAudioPlaybackRate *rate);

int TSoundDeviceSetVolume(SoundCtrl* s,int volume);

int TSoundDeviceControl(SoundCtrl* s, int cmd, void* para);

static SoundControlOpsT mSoundControlOps =
{
    .destroy          =   TSoundDeviceDestroy,
    .setFormat        =   TSoundDeviceSetFormat,
    .start            =   TSoundDeviceStart,
    .stop             =   TSoundDeviceStop,
    .pause            =   TSoundDevicePause,
    .write            =   TSoundDeviceWrite,
    .reset            =   TSoundDeviceReset,
    .getCachedTime    =   TSoundDeviceGetCachedTime,
    .getFrameCount    =   TSoundDeviceGetFrameCount,
    .setPlaybackRate  =   TSoundDeviceSetPlaybackRate,
    .control          =   TSoundDeviceControl,
};

#ifdef __cplusplus
}
#endif

#endif
