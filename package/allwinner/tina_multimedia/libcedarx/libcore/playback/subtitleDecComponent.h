/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : subtitleDecComponent.h
 * Description : subtitle decoder component
 * History :
 *
 */

#ifndef SUBTITLE_DECODE_COMPONENT
#define SUBTITLE_DECODE_COMPONENT

#include "player_i.h"
#include "avtimer.h"
#include "sdecoder.h"

typedef struct SubtitleDecComp SubtitleDecComp;

SubtitleDecComp* SubtitleDecCompCreate(void);

void SubtitleDecCompDestroy(SubtitleDecComp* s);

int SubtitleDecCompStart(SubtitleDecComp* s);

int SubtitleDecCompStop(SubtitleDecComp* s);

int SubtitleDecCompPause(SubtitleDecComp* s);

enum EPLAYERSTATUS SubtitleDecCompGetStatus(SubtitleDecComp* s);

int SubtitleDecCompReset(SubtitleDecComp* s, int64_t nSeekTime);

int SubtitleDecCompSetEOS(SubtitleDecComp* s);

int SubtitleDecCompSetCallback(SubtitleDecComp* s, PlayerCallback callback, void* pUserData);

int SubtitleDecCompSetSubtitleStreamInfo(SubtitleDecComp*    s,
                                         SubtitleStreamInfo* pStreamInfo,
                                         int                 nStreamCount,
                                         int                 nDefaultStreamIndex);

int SubtitleDecCompAddSubtitleStream(SubtitleDecComp* s, SubtitleStreamInfo* pStreamInfo);

int SubtitleDecCompGetSubtitleStreamCnt(SubtitleDecComp* s);

int SubtitleDecCompCurrentStreamIndex(SubtitleDecComp* s);

int SubtitleDecCompGetSubtitleStreamInfo(SubtitleDecComp* a, int* pStreamNum,
        SubtitleStreamInfo** ppStreamInfo);

int SubtitleDecCompSetTimer(SubtitleDecComp* s, AvTimer* timer);

int SubtitleDecCompRequestStreamBuffer(SubtitleDecComp* s,
                                       int              nRequireSize,
                                       char**           ppBuf,
                                       int*             pBufSize,
                                       int              nStreamIndex);

int SubtitleDecCompSubmitStreamData(SubtitleDecComp* s,
        SubtitleStreamDataInfo* pDataInfo, int nStreamIndex);

SubtitleItem* SubtitleDecCompRequestSubtitleItem(SubtitleDecComp* s);

void SubtitleDecCompFlushSubtitleItem(SubtitleDecComp* s, SubtitleItem* pItem);

SubtitleItem* SubtitleDecCompNextSubtitleItem(SubtitleDecComp* s);

int SubtitleDecCompSwitchStream(SubtitleDecComp* s, int nStreamIndex);

int SubtitleDecCompStreamBufferSize(SubtitleDecComp* s, int nStreamIndex);

int SubtitleDecCompStreamDataSize(SubtitleDecComp* s, int nStreamIndex);

int SubtitleDecCompIsExternalSubtitle(SubtitleDecComp* s, int nStreamIndex);

int SubtitleDecCompGetExternalFlag(SubtitleDecComp* s);

#endif
