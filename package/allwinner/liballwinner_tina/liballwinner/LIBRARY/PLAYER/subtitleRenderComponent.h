
#ifndef SUBTITLE_RENDER_H
#define SUBTITLE_RENDER_H

#include "player_i.h"
#include "subtitleDecComponent.h"

typedef void* SubtitleRenderComp;

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

#if( CONFIG_ALI_YUNOS == OPTION_ALI_YUNOS_YES)
int SubtitleRenderCompSetSubtitleStreamInfo(SubtitleRenderComp* s,SubtitleStreamInfo* pStreamInfo,int nStreamCount,int nDefaultStreamIndex);

int SubtitleRenderCompSwitchStream(SubtitleRenderComp* s, int nStreamIndex);
#endif

#endif
