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

#include "soundControl.h"
#include "outputCtrl.h"

typedef struct SoundCtrlContext
{
    SoundCtrl                   base;
    pthread_mutex_t             mutex;
    void*                       pUserData;
}SoundCtrlContext;

static void __SdNullRelease(SoundCtrl* s)
{
    int               ret;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    pthread_mutex_destroy(&sc->mutex);

    free(sc);
    sc = NULL;

    return;
}

static void __SdNullSetFormat(SoundCtrl* s, CdxPlaybkCfg *cfg)
{
    int               ret;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;
(void)cfg;
    return;
}


static int __SdNullStart(SoundCtrl* s)
{
    int               ret;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    return 0;
}


static int __SdNullStop(SoundCtrl* s)
{
    int               ret = 0;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    return ret;
}


static int __SdNullPause(SoundCtrl* s)
{
    int               ret = 0;
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;

    return 0;
}


static int __SdNullWrite(SoundCtrl* s, void* pData, int nDataSize)
{
    int               ret;
    SoundCtrlContext* sc;

    sc = (SoundCtrlContext*)s;

    CDX_UNUSE(pData);
    return nDataSize;
}


//* called at player seek operation.
static int __SdNullReset(SoundCtrl* s)
{
    return SoundDeviceStop(s);
}


static int __SdNullGetCachedTime(SoundCtrl* s)
{
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;

    return 0;
}

static int __SdNullGetFrameCount(SoundCtrl* s)
{
    SoundCtrlContext* sc;
    sc = (SoundCtrlContext*)s;

    return 0;
}

int _SdNullSetPlaybackRate(SoundCtrl* , const XAudioPlaybackRate *)
{
    return 0;
}


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
    .setPlaybackRate = _SdNullSetPlaybackRate,
};


SoundCtrl* DefaultSoundDeviceCreate()
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

    pthread_mutex_init(&s->mutex, NULL);
    return (SoundCtrl*)s;
}
