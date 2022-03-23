/*
 * Copyright (c) 2008-2017 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : NotifyHalDirect.c
 * Description : Hold the alsa pcm device
 * History :
 *
 */

#include <binder/IPCThreadState.h>
#include <media/IAudioFlinger.h>
#include <media/AudioSystem.h>

#include <utils/String8.h>

#define DIRECT_ON  1
#define DIRECT_OFF 0
extern "C" {
using namespace android;

void NotifyHalDirect(int on_off)
{
    if(on_off == DIRECT_ON)
    {
        String8 direct_mode_on = String8("direct-mode=1");
        int64_t token = 0;
        const sp<IAudioFlinger>& af = AudioSystem::get_audio_flinger();
        token = IPCThreadState::self()->clearCallingIdentity();
        af->setParameters(0, direct_mode_on);
        IPCThreadState::self()->restoreCallingIdentity(token);
    }
    else
    {
        String8 direct_mode_off = String8("direct-mode=0");
        int64_t token = 0;
        const sp<IAudioFlinger>& af = AudioSystem::get_audio_flinger();
        token = IPCThreadState::self()->clearCallingIdentity();
        af->setParameters(0, direct_mode_off);
        IPCThreadState::self()->restoreCallingIdentity(token);
    }
}
}
