#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "player_i.h"
#include "soundControl.h"
#include "log.h"

typedef struct SoundCtrlContext
{
    pthread_mutex_t             mutex;
    SndCallback              callback;
    void*                       pUserData;
}SoundCtrlContext;

SoundCtrl* SoundDeviceInit(void* pAudioSink)
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

    pthread_mutex_init(&s->mutex, NULL);
    return (SoundCtrl*)s;
}


void SoundDeviceRelease(SoundCtrl* s)
{
    int               ret;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    pthread_mutex_destroy(&sc->mutex);

    free(sc);
    sc = NULL;

    return;
}

int SoundDeviceSetCallback(SoundCtrl* s, SndCallback callback, void* pUserData)
{
	SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

	sc->callback = callback;
    sc->pUserData = pUserData;

    return 0;
}



void SoundDeviceSetFormat(SoundCtrl* s, unsigned int nSampleRate, unsigned int nChannelNum)
{
    int               ret;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;



    return;
}


int SoundDeviceStart(SoundCtrl* s)
{
    int               ret;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    return 0;
}


int SoundDeviceStop(SoundCtrl* s)
{
    int               ret = 0;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    return ret;
}


int SoundDevicePause(SoundCtrl* s)
{
    int               ret = 0;
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;

    return 0;
}


int SoundDeviceWrite(SoundCtrl* s, void* pData, int nDataSize)
{
    int               ret;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

	long callbackParam[2];
	callbackParam[0] = (long)pData;
	callbackParam[1] = nDataSize;
    if(sc->callback)
	sc->callback(sc->pUserData, MESSAGE_ID_SOUND_NOTIFY_BUFFER, (void*)callbackParam);

	return nDataSize;
}


//* called at player seek operation.
int SoundDeviceReset(SoundCtrl* s)
{
    return SoundDeviceStop(s);
}


int SoundDeviceGetCachedTime(SoundCtrl* s)
{
    int               ret;
    SoundCtrlContext* sc;

    return 0;
}

// stub for linux
int SoundDevicePause_raw(SoundCtrl* s)
{
	return -1;
}

SoundCtrl* SoundDeviceInit_raw(void* raw,void* hdeccomp,RawCallback callback)
{
	return NULL;
}

int SoundDeviceStop_raw(SoundCtrl* s)
{
	return -1;
}

void SoundDeviceRelease_raw(SoundCtrl* s)
{
	return;
}

int SoundDeviceStart_raw(SoundCtrl* s)
{
	return -1;
}

int SoundDeviceWrite_raw(SoundCtrl* s, void* pData, int nDataSize)
{
	return -1;
}

int SoundDeviceSetVolume(SoundCtrl* s, float volume)
{
    return 0;
}
int SoundDeviceGetVolume(SoundCtrl* s, float *volume)
{
    return 0;
}


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
	SoundDeviceInit_raw:			SoundDeviceInit_raw,
	SoundDeviceRelease_raw:			SoundDeviceRelease_raw,
	SoundDeviceSetFormat_raw:		SoundDeviceSetFormat,
	SoundDeviceStart_raw:			SoundDeviceStart_raw,
	SoundDeviceStop_raw:			SoundDeviceStop_raw,
	SoundDevicePause_raw:			SoundDevicePause_raw,
	SoundDeviceWrite_raw:			SoundDeviceWrite_raw,
	SoundDeviceReset_raw:			SoundDeviceReset,
	SoundDeviceGetCachedTime_raw:	SoundDeviceGetCachedTime,
	SoundDeviceSetVolume:			SoundDeviceSetVolume,
	SoundDeviceGetVolume:		SoundDeviceGetVolume,
	SoundDeviceSetCallback:		SoundDeviceSetCallback
};
