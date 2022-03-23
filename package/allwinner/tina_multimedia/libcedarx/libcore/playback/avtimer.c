/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : avtimer.cpp
 * Description : audio-video synchronization timer
 * History :
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "avtimer.h"
#include "cdx_log.h"
#include <CdxTime.h>

typedef struct videoTimerInfo {
    int64_t nLastPts;
    int64_t nFrameDura;
} videoTimerInfo;

//************************************************************//
//**************** define the avtimer context ****************//
//************************************************************//
typedef struct AvTimerContext
{
    AvTimer         sAvTimer;         //* the avtimer interface.

    int             nSpeed;           //* counting at this speed.

    float           fPlayRate;

    int             eStatus;          //* status of this timer.

    int64_t         nStartTimeMedia;   //* the media file time when timer starts
    int64_t         nStartTimeOs;      //* the system time when timer starts.

    pthread_mutex_t mutex;           //* mutex to lock the timer.

    videoTimerInfo videoInfo;
}AvTimerContext;

//************************************************************//
//****************** declare static methods ******************//
//************************************************************//
static int AvTimerSetSpeed(AvTimer* t, int nSpeed);

static int AvTimerGetSpeed(AvTimer* t);

static int AvTimerSetPlayRate(AvTimer* t, float rate);

static float AvTimerGetPlayRate(AvTimer* t);

static int AvTimerSetTime(AvTimer* t, int64_t nTime);

static int64_t AvTimerGetTime(AvTimer* t);

static int64_t AvTimerPtsToSystemTime(AvTimer* t, int64_t pts);

static int AvTimerStart(AvTimer* t);

static void AvTimerStop(AvTimer* t);

static int AvTimerGetStatus(AvTimer* t);

//************************************************************//
//********************** implementation **********************//
//************************************************************//

AvTimer* AvTimerCreate(void)
{
    AvTimerContext* pAvTimerCtx;
    AvTimer*        pAvTimer;

    pAvTimerCtx = (AvTimerContext*) malloc(sizeof(AvTimerContext));
    if(pAvTimerCtx == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }

    memset(pAvTimerCtx, 0, sizeof(AvTimerContext));

    pAvTimer = (AvTimer*)pAvTimerCtx;

    pAvTimer->SetSpeed  = AvTimerSetSpeed;
    pAvTimer->GetSpeed  = AvTimerGetSpeed;
    pAvTimer->SetPlayRate = AvTimerSetPlayRate;
    pAvTimer->SetTime   = AvTimerSetTime;
    pAvTimer->GetTime   = AvTimerGetTime;
    pAvTimer->PtsToSystemTime = AvTimerPtsToSystemTime;
    pAvTimer->Start     = AvTimerStart;
    pAvTimer->Stop      = AvTimerStop;
    pAvTimer->GetStatus = AvTimerGetStatus;

    pAvTimerCtx->nSpeed     = 1000;
    pAvTimerCtx->eStatus    = TIMER_STATUS_STOP;

    pthread_mutex_init(&pAvTimerCtx->mutex, NULL);

    pAvTimerCtx->videoInfo.nFrameDura = 40 * 1000;

    pAvTimerCtx->fPlayRate = 1.0;

    return pAvTimer;
}

void AvTimerDestroy(AvTimer* t)
{
    AvTimerContext* pAvTimerCtx;

    pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_destroy(&pAvTimerCtx->mutex);

    free(pAvTimerCtx);
}

static int AvTimerStart(AvTimer* t)
{
    AvTimerContext* pAvTimerCtx;

    pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);

    pAvTimerCtx->eStatus = TIMER_STATUS_START;

    pAvTimerCtx->nStartTimeOs = CdxMonoTimeUs();

    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return 0;
}

static inline void AvTimerReset(AvTimerContext *p)
{
    int64_t nCurrentTime = CdxMonoTimeUs();
    int64_t nPassedTime = nCurrentTime - p->nStartTimeOs;

    p->nStartTimeMedia += nPassedTime * p->nSpeed / 1000;
    p->nStartTimeOs = nCurrentTime;
}

static void AvTimerStop(AvTimer* t)
{
    AvTimerContext* pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);

    AvTimerReset(pAvTimerCtx);
    pAvTimerCtx->eStatus = TIMER_STATUS_STOP;

    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return;
}

static int AvTimerSetSpeed(AvTimer* t, int nSpeed)
{
    AvTimerContext* pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);

    AvTimerReset(pAvTimerCtx);
    pAvTimerCtx->nSpeed = nSpeed;

    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return 0;
}

static int AvTimerGetSpeed(AvTimer* t)
{
    int nSpeed;
    AvTimerContext* pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);
    nSpeed = pAvTimerCtx->nSpeed;
    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return nSpeed;
}

static int AvTimerSetPlayRate(AvTimer* t, float rate)
{
    logv("set playrate to %f",rate);
    AvTimerContext* pAvTimerCtx = (AvTimerContext*)t;
    int64_t         nPassedTime;
    int64_t         nCurrentTime;

    pthread_mutex_lock(&pAvTimerCtx->mutex);

    pAvTimerCtx->fPlayRate = rate;

    if(pAvTimerCtx->eStatus == TIMER_STATUS_START)
    {
        nCurrentTime = CdxMonoTimeUs();
        nPassedTime = (nCurrentTime - pAvTimerCtx->nStartTimeOs) *
            pAvTimerCtx->nSpeed * pAvTimerCtx->fPlayRate / 1000;
        nPassedTime += pAvTimerCtx->nStartTimeMedia;
        pAvTimerCtx->nStartTimeMedia = nPassedTime;
        pAvTimerCtx->nStartTimeOs = nCurrentTime;
    }
    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return 0;
}

static int AvTimerGetStatus(AvTimer* t)
{
    int eStatus;
    AvTimerContext* pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);
    eStatus = pAvTimerCtx->eStatus;
    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return eStatus;
}

static int64_t AvTimerGetTime(AvTimer* t)
{
    AvTimerContext* pAvTimerCtx;
    int64_t         nPassedTime;
    int64_t         nCurrentTime;

    pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);

    if(pAvTimerCtx->eStatus == TIMER_STATUS_START)
    {
        nCurrentTime = CdxMonoTimeUs();
        nPassedTime = (nCurrentTime - pAvTimerCtx->nStartTimeOs) *
                        pAvTimerCtx->nSpeed * pAvTimerCtx->fPlayRate / 1000;

        nPassedTime += pAvTimerCtx->nStartTimeMedia;
        pAvTimerCtx->nStartTimeMedia = nPassedTime;
        pAvTimerCtx->nStartTimeOs = nCurrentTime;
    }
    else    //* in stop status.
    {
        //* return the last record time.
        nPassedTime = pAvTimerCtx->nStartTimeMedia;
    }

    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return nPassedTime;
}

static int64_t AvTimerPtsToSystemTime(AvTimer* t, int64_t pts)
{
    AvTimerContext* pAvTimerCtx;
    int64_t         nPtsAbs;

    pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);

    if (pAvTimerCtx->eStatus != TIMER_STATUS_START)
    {
        nPtsAbs = pAvTimerCtx->nStartTimeOs;
        pthread_mutex_unlock(&pAvTimerCtx->mutex);
        return nPtsAbs * 1000;
    }

    videoTimerInfo *vi = &pAvTimerCtx->videoInfo;
    int64_t nFrameDura = pts - vi->nLastPts;

    if (nFrameDura < 0)
    {
        logv("pts %lld, nFrameDura %lld, vi->nFrameDura %lld",
                pts, nFrameDura, vi->nFrameDura);
        pts = vi->nLastPts + vi->nFrameDura;
        vi->nFrameDura = vi->nFrameDura * 0.95;
    }
    else if (nFrameDura <= (2 * vi->nFrameDura))
    {
        vi->nFrameDura = vi->nFrameDura * 0.95 + nFrameDura * 0.05;
    }
    else
    {
        logv("pts %lld, nFrameDura %lld, vi->nFrameDura %lld",
                pts, nFrameDura, vi->nFrameDura);
        pts = vi->nLastPts + 2 * vi->nFrameDura;
        vi->nFrameDura = vi->nFrameDura * 1.05;
    }

    nPtsAbs = (pts - pAvTimerCtx->nStartTimeMedia) + pAvTimerCtx->nStartTimeOs;
                // + 30 * 1000; // Does this value be tested and verified?

    if (vi->nFrameDura < 5000) // higher than 200 fps, almost impossible
        vi->nFrameDura = 5000;
    else if (vi->nFrameDura > 100 * 1000) // less than 10 fps, not quite possible
        vi->nFrameDura = 100 * 1000;
    vi->nLastPts = pts;

    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    // This function is expected to return nanoseconds
    return nPtsAbs * 1000;
}

static int AvTimerSetTime(AvTimer* t, int64_t nTime)
{
    AvTimerContext* pAvTimerCtx;
    int64_t         c;

    pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);

    pAvTimerCtx->nStartTimeMedia = nTime;
    pAvTimerCtx->nStartTimeOs = CdxMonoTimeUs();

    pAvTimerCtx->videoInfo.nLastPts = nTime;

    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return 0;
}
