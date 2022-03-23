
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdint.h>
#include "avtimer.h"
#include "log.h"

//************************************************************//
//**************** define the avtimer context ****************//
//************************************************************//
typedef struct AvTimerContext
{
    AvTimer         sAvTimer;         //* the avtimer interface.

    int             nSpeed;           //* counting at this speed.

    int64_t         nStartTime;       //* start counting from this time.

    int             eStatus;          //* status of this timer.

    struct timeval  startOsTime;   //* the system's time when this timer starts.

    struct timeval  lastOsTime;    //* the system's time at last operation.

    pthread_mutex_t mutex;           //* mutex to lock the timer.

    int64_t         nStartPts;			//* pts of the first frame
    int64_t         nStartSystemTime;	//* render systemtime of the first frame
}AvTimerContext;


static int64_t __systemTime()
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec*1000000000LL + t.tv_nsec;
}

//************************************************************//
//****************** declare static methods ******************//
//************************************************************//
static int AvTimerSetSpeed(AvTimer* t, int nSpeed);

static int AvTimerGetSpeed(AvTimer* t);

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
    pAvTimer->SetTime   = AvTimerSetTime;
    pAvTimer->GetTime   = AvTimerGetTime;
    pAvTimer->PtsToSystemTime = AvTimerPtsToSystemTime;
    pAvTimer->Start     = AvTimerStart;
    pAvTimer->Stop      = AvTimerStop;
    pAvTimer->GetStatus = AvTimerGetStatus;

    pAvTimerCtx->nSpeed     = 1000;
    pAvTimerCtx->nStartTime = 0;
    pAvTimerCtx->eStatus    = TIMER_STATUS_STOP;

    pthread_mutex_init(&pAvTimerCtx->mutex, NULL);

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

    //* record the system's time when timer starts.
    gettimeofday(&pAvTimerCtx->startOsTime, NULL);

    pAvTimerCtx->lastOsTime.tv_sec  = pAvTimerCtx->startOsTime.tv_sec;
    pAvTimerCtx->lastOsTime.tv_usec = pAvTimerCtx->startOsTime.tv_usec;

    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return 0;
}


static void AvTimerStop(AvTimer* t)
{
    AvTimerContext* pAvTimerCtx;
    int64_t         nPassedTime;

    pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);

    //* record the system's time when timer stops.
    gettimeofday(&pAvTimerCtx->lastOsTime, NULL);

    //* change the start counting point.
    nPassedTime  = (int64_t)(pAvTimerCtx->lastOsTime.tv_sec  - pAvTimerCtx->startOsTime.tv_sec) * 1000000;
    nPassedTime += (pAvTimerCtx->lastOsTime.tv_usec - pAvTimerCtx->startOsTime.tv_usec);

    pAvTimerCtx->nStartTime += pAvTimerCtx->nSpeed * nPassedTime / 1000;

    pAvTimerCtx->startOsTime.tv_sec  = pAvTimerCtx->lastOsTime.tv_sec;
    pAvTimerCtx->startOsTime.tv_usec = pAvTimerCtx->lastOsTime.tv_usec;

    pAvTimerCtx->eStatus = TIMER_STATUS_STOP;

    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return;
}


static int AvTimerSetSpeed(AvTimer* t, int nSpeed)
{
    AvTimerContext* pAvTimerCtx;
    int64_t         nPassedTime;

    pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);

    //* record the system's time.
    gettimeofday(&pAvTimerCtx->lastOsTime, NULL);

    //* change the start counting point.
    nPassedTime  = (int64_t)(pAvTimerCtx->lastOsTime.tv_sec  - pAvTimerCtx->startOsTime.tv_sec) * 1000000;
    nPassedTime += (pAvTimerCtx->lastOsTime.tv_usec - pAvTimerCtx->startOsTime.tv_usec);

    pAvTimerCtx->nStartTime += pAvTimerCtx->nSpeed * nPassedTime / 1000;

    pAvTimerCtx->startOsTime.tv_sec  = pAvTimerCtx->lastOsTime.tv_sec;
    pAvTimerCtx->startOsTime.tv_usec = pAvTimerCtx->lastOsTime.tv_usec;

    //* change the counting speed.
    pAvTimerCtx->nSpeed = nSpeed;

    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return 0;
}


static int AvTimerGetSpeed(AvTimer* t)
{
    AvTimerContext* pAvTimerCtx;

    pAvTimerCtx = (AvTimerContext*)t;

    return pAvTimerCtx->nSpeed;
}


static int AvTimerGetStatus(AvTimer* t)
{
    AvTimerContext* pAvTimerCtx;

    pAvTimerCtx = (AvTimerContext*)t;

    return pAvTimerCtx->eStatus;
}


static int64_t AvTimerGetTime(AvTimer* t)
{
    AvTimerContext* pAvTimerCtx;
    int64_t         nPassedTime;
    int64_t         c;

    pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);

    if(pAvTimerCtx->eStatus == TIMER_STATUS_START)
    {
        //* record the system's time.
        gettimeofday(&pAvTimerCtx->lastOsTime, NULL);

        //* change the start counting point.
        nPassedTime  = (int64_t)(pAvTimerCtx->lastOsTime.tv_sec  - pAvTimerCtx->startOsTime.tv_sec) * 1000000;
        nPassedTime += (pAvTimerCtx->lastOsTime.tv_usec - pAvTimerCtx->startOsTime.tv_usec);

        nPassedTime = pAvTimerCtx->nStartTime + (pAvTimerCtx->nSpeed * nPassedTime / 1000);
    }
    else    //* in stop status.
    {
        //* return the last record time.
        nPassedTime = pAvTimerCtx->nStartTime;
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

    if(pAvTimerCtx->eStatus == TIMER_STATUS_START)
    {
        //* record the system's time.
        nPtsAbs = pAvTimerCtx->nStartSystemTime + (pts - pAvTimerCtx->nStartPts) * 1000LL;
    }
    else    //* in stop status.
    {
        //* return the last record time.
        nPtsAbs = pAvTimerCtx->nStartSystemTime;
    }

	pthread_mutex_unlock(&pAvTimerCtx->mutex);

	return nPtsAbs;
}


static int AvTimerSetTime(AvTimer* t, int64_t nTime)
{
    AvTimerContext* pAvTimerCtx;
    int64_t         c;

    pAvTimerCtx = (AvTimerContext*)t;

    pthread_mutex_lock(&pAvTimerCtx->mutex);

    gettimeofday(&pAvTimerCtx->startOsTime, NULL);

    pAvTimerCtx->lastOsTime.tv_sec  = pAvTimerCtx->startOsTime.tv_sec;
    pAvTimerCtx->lastOsTime.tv_usec = pAvTimerCtx->startOsTime.tv_usec;

    pAvTimerCtx->nStartTime = nTime;

	pAvTimerCtx->nStartPts = nTime;
	pAvTimerCtx->nStartSystemTime = __systemTime() + 30000000LL;

    pthread_mutex_unlock(&pAvTimerCtx->mutex);

    return 0;
}
