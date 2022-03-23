/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : VideoFrameSchedulerWrap.h
* Description : wrap for VideoFrameScheduler
* History :
*   Author  : zhaozhili
*   Date    : 2016/09/05
*   Comment : first version
*
*/

#ifndef VIDEO_FRAME_SCHEDULER_WRAP_H_
#define VIDEO_FRAME_SCHEDULER_WRAP_H_

#include <CdxTypes.h>
#include <stdint.h>

typedef struct CdxVideoScheduler CdxVideoScheduler;

CDX_API CdxVideoScheduler *CdxVideoSchedulerCreate(void);

CDX_API void CdxVideoSchedulerDestroy(CdxVideoScheduler *p);

CDX_API int64_t CdxVideoSchedulerSchedule(CdxVideoScheduler *p, int64_t pts);

// use in case of video render-time discontinuity, e.g. seek
CDX_API void CdxVideoSchedulerRestart(CdxVideoScheduler *p);

CDX_API int64_t CdxVideoSchedulerGetVsyncPeriod(CdxVideoScheduler *p);
#endif
