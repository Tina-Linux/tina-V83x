/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : subtitleRenderComponent.h
 * Description : subtitle render component
 * History :
 *
 */

#ifndef SUBTITLE_RENDER_H
#define SUBTITLE_RENDER_H

#include "player_i.h"
#include "subtitleDecComponent.h"
#include "subtitleControl.h"

typedef struct SubtitleRenderComp SubtitleRenderComp;

SubtitleRenderComp* SubtitleRenderCompCreate(void);

int SubtitleRenderCompDestroy(SubtitleRenderComp* s);

int SubtitleRenderCompStart(SubtitleRenderComp* s);

int SubtitleRenderCompStop(SubtitleRenderComp* s);

int SubtitleRenderCompPause(SubtitleRenderComp* s);

enum EPLAYERSTATUS SubtitleRenderCompGetStatus(SubtitleRenderComp* s);

int SubtitleRenderCompReset(SubtitleRenderComp* s);

int SubtitleRenderCompSetEOS(SubtitleRenderComp* s);

int SubtitleRenderCompSetCallback(SubtitleRenderComp* s, PlayerCallback callback, void* pUserData);

int SubtitleRenderCompSetTimer(SubtitleRenderComp* s, AvTimer* timer);

int SubtitleRenderCompSetDecodeComp(SubtitleRenderComp* s, SubtitleDecComp* d);

int SubtitleRenderSetShowTimeAdjustment(SubtitleRenderComp* s, int nTimeMs);

int SubtitleRenderGetShowTimeAdjustment(SubtitleRenderComp* s);

int SubtitleRenderCompSetVideoOrAudioFirstPts(SubtitleRenderComp* s,int64_t nFirstPts);

#if defined(CONF_YUNOS)
int SubtitleRenderCompSetSubtitleStreamInfo(SubtitleRenderComp* s,
        SubtitleStreamInfo* pStreamInfo, int nStreamCount, int nDefaultStreamIndex);

int SubtitleRenderCompSwitchStream(SubtitleRenderComp* s, int nStreamIndex);
#endif

int SubtitleRenderCompSetSubControl(SubtitleRenderComp* s, SubCtrl* pSubCtrl);

#endif
