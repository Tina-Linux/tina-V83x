/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : VideoFrameSchedulerWrap.cpp
* Description :  wrap for VideoFrameScheduler
* History :
*   Author  : zhaozhili
*   Date    : 2016/09/05
*   Comment : first version
*
*/

#include "VideoFrameSchedulerWrap.h"

#if ((CONF_ANDROID_MAJOR_VER >= 5)||((CONF_ANDROID_MAJOR_VER == 4)&&(CONF_ANDROID_SUB_VER >= 4)))

#include "VideoFrameScheduler.h"

using namespace android;

struct CdxVideoScheduler : public VideoFrameScheduler {};

CdxVideoScheduler *CdxVideoSchedulerCreate()
{
    CdxVideoScheduler *p = new (std::nothrow) CdxVideoScheduler();
    if (p == NULL)
        return NULL;

    p->init();
    return p;
}

void CdxVideoSchedulerDestroy(CdxVideoScheduler *p)
{
    if (p == NULL)
        return;
    delete p;
}

int64_t CdxVideoSchedulerSchedule(CdxVideoScheduler *p, int64_t pts)
{
    if (p != NULL)
        return p->schedule(pts);
    else
        return pts;
}

void CdxVideoSchedulerRestart(CdxVideoScheduler *p)
{
    if (p != NULL)
        p->restart();
}

int64_t CdxVideoSchedulerGetVsyncPeriod(CdxVideoScheduler *p)
{
    if (p != NULL)
        return p->getVsyncPeriod();
    return 1000000000 / 60; //default ns.
}
#endif
