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
#include "subtitleControl.h"
#include "deinterlace.h"
#include <CdxEnumCommon.h>

enum SubCtrlCallbackId {
    SUBCTRL_SUBTITLE_AVAILABLE = SUBTITLE_CALLBACK_VALID_RANGE_MIN,
    SUBCTRL_SUBTITLE_EXPIRED,

    SUBCTRL_SUBTITLE_MAX,
};

typedef enum {
	LAYER_CMD_SET_BRIGHTNESS = 0,
	LAYER_CMD_SET_CONTRAST,
	LAYER_CMD_SET_HUE,
	LAYER_CMD_SET_SATURATION,
	LAYER_CMD_SET_VEDIO_ENHANCE_DEFAULT,
}LAYER_CMD_TYPE;


CHECK_SUBTITLE_CALLBACK_MAX_VALID(SUBCTRL_SUBTITLE_MAX)

typedef int (*SubCtrlCallback)(void* pUser, int msg, void* para);

typedef int (*VideoFrameCallback)(void* pUser, void* para);


LayerCtrl* LayerCreate(VideoFrameCallback callback,void* pUser);

void LayerSetControl(LayerCtrl* l, LAYER_CMD_TYPE cmd, int grade);

int LayerSetDisplayRect(LayerCtrl* l, int x, int y, unsigned int width, unsigned int height);

int LayerDisplayOnoff(LayerCtrl* l, int onoff);

SubCtrl* SubtitleCreate(SubCtrlCallback pCallback, void* pUser);

int SubtitleDisplayOnoff(SubCtrl* p, int onoff);

Deinterlace* DeinterlaceCreate(void);

#ifdef __cplusplus
}
#endif

#endif
