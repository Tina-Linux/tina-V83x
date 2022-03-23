/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : outputCtrl.h
* Description : output, include layer/soundCtrl/subCtrl/di
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : first version
*
*/

#ifndef __OUTPUT_CTRL_H__
#define __OUTPUT_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "layerControl.h"
#include "soundControl.h"
#include "subtitleControl.h"
#include "deinterlace.h"
#include <CdxEnumCommon.h>

enum SubCtrlCallbackId {
    SUBCTRL_SUBTITLE_AVAILABLE = SUBTITLE_CALLBACK_VALID_RANGE_MIN,
    SUBCTRL_SUBTITLE_EXPIRED,

    SUBCTRL_SUBTITLE_MAX,
};
CHECK_SUBTITLE_CALLBACK_MAX_VALID(SUBCTRL_SUBTITLE_MAX)

typedef int (*SubCtrlCallback)(void* pUser, int msg, void* para);

LayerCtrl* LayerCreate();

SoundCtrl* SoundDeviceCreate(void* pAudioSink);

SoundCtrl* DefaultSoundDeviceCreate();

SubCtrl* SubtitleCreate(SubCtrlCallback pCallback, void* pUser);

SubCtrl* SubNativeRenderCreate();

Deinterlace* DeinterlaceCreate();

#ifdef __cplusplus
}
#endif

#endif
