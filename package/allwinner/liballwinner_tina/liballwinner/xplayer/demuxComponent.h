/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : demuxComponent.h
 * Description : demuxComponent
 * History :
 *
 */


#ifndef DEMUX_COMPONENT_H
#define DEMUX_COMPONENT_H

#include <sys/types.h>
#include "player.h"      //* player library in "android/hardware/aw/"
#include "mediaInfo.h"

#if CONFIG_OS == OPTION_OS_ANDROID
#include <utils/KeyedVector.h>
#include <utils/String8.h>
using namespace android;
#else
#include <string>
#include <map>
using namespace std;
#endif

typedef void* DemuxComp;

#define SOURCE_TYPE_URL           0x1
#define SOURCE_TYPE_FD            0x2

enum EDEMUXNOTIFY  //* player internal notify.
{
    DEMUX_NOTIFY_PREPARED       = 512,
    DEMUX_NOTIFY_EOS,
    DEMUX_NOTIFY_IOERROR,
    DEMUX_NOTIFY_SEEK_FINISH,
    DEMUX_NOTIFY_CACHE_STAT,
    DEMUX_NOTIFY_BUFFER_START,
    DEMUX_NOTIFY_BUFFER_END,
	DEMUX_NOTIFY_PAUSE_PLAYER,
	DEMUX_NOTIFY_RESUME_PLAYER,

    DEMUX_NOTIFY_DATA_PACKET,
};

enum EDEMUXERROR
{
    DEMUX_ERROR_NONE        = 0,
    DEMUX_ERROR_UNKNOWN     = -1,
    DEMUX_ERROR_IO          = -2,
    DEMUX_ERROR_USER_CANCEL = -3,
};

typedef int (*DemuxCallback)(void* pUserData, int eMessageId, void* param);

DemuxComp* DemuxCompCreate(void);

void DemuxCompDestroy(DemuxComp* d);

void DemuxCompClear(DemuxComp* d);  //* clear the data source, like just created.

int DemuxCompSetUrlSource(DemuxComp* d, const char* pUrl, const map<string, string>* pHeaders);

int DemuxCompSetFdSource(DemuxComp* d, int fd, int64_t nOffset, int64_t nLength);

int DemuxCompSetPlayer(DemuxComp* d, Player* player);

int DemuxCompSetCallback(DemuxComp* d, DemuxCallback callback, void* pUserData);

int DemuxCompPrepareAsync(DemuxComp* d);

int DemuxCompCancelPrepare(DemuxComp* d);   //* should call back DEMUX_PREPARE_FINISH message.

MediaInfo* DemuxCompGetMediaInfo(DemuxComp* d);

int DemuxCompStart(DemuxComp* d);

int DemuxCompStop(DemuxComp* d);    //* close the data source, must call prepare again to restart.

int DemuxCompPause(DemuxComp* d);   //* no pause status in demux component, return OK immediately.

int DemuxCompGetStatus(DemuxComp* d);

int DemuxCompSeekTo(DemuxComp* d, int mSec);

int DemuxCompCancelSeek(DemuxComp* d);  //* should not call back DEMUX_SEEK_FINISH message.

int DemuxCompSetCacheStatReportInterval(DemuxComp* d, int ms);

int DemuxCompSetCacheSize(DemuxComp* d, int nStartPlaySize, int nMaxBufferSize);

#endif
