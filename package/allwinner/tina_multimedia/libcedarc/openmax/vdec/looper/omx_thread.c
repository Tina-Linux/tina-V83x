/*
 * =====================================================================================
 *
 *       Filename:  omx_thread.c
 *
 *    Description: the thread class can supend, resume, quit and run
 *
 *        Version:  1.0
 *        Created:  08/02/2018 04:18:11 PM
 *       Revision:  none
 *
 *         Author:  Gan Qiuye
 *        Company:
 *
 * =====================================================================================
 */

#include "omx_thread.h"
#include <sys/prctl.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#define LOG_TAG "omx_thread"
#include "log.h"

#define USE_DETACHED 0

static int loopEntryWrapper(void* userData)
{
    OmxThread_t *user = (OmxThread_t*)userData;
    OMX_BOOL ret = OMX_TRUE;
    OMX_BOOL first = OMX_TRUE;
    do{
        if(user->bSuspendRequest)
        {
            logd("post stop run sem %s", user->mName);
            OmxTryPostSem(user->mSemStopRun);
            OmxTimedWaitSem(user->mSemSuspend, -1);
        }
        if(first || user->bRepeatToReady)
        {
            if(user->mfnReadyToRun)
            {
                ret = user->mfnReadyToRun(user->mPriv);
                if(ret)
                {
                    first = OMX_FALSE;
                    user->bRepeatToReady = OMX_FALSE;
                }
            }
            else
            {
                first = OMX_FALSE;
                user->bRepeatToReady = OMX_FALSE;
            }
        }
        else
        {
            ret = user->mfnThreadLoop(user->mPriv);
        }
        if(/*ret != OMX_TRUE || */user->bExitPending)
        {
            user->bExitPending = OMX_TRUE;
            user->bRunning = OMX_FALSE;
            OmxTryPostSem(user->mSemExit);
            logv("exit thread %s!", user->mName);
            break;
        }
    }while(1);
    logd("exit thread %s truely!!!", user->mName);
    return 0;
}

static void *loopEntry(void* priv)
{
    OmxThread_t *userData;
    if(priv != NULL)
    {
        userData = (OmxThread_t*)priv;
        prctl(PR_SET_NAME, (unsigned long)userData->mName, 0, 0, 0);
        loopEntryWrapper(priv);
    }
    return priv;
}

static int createThread(OmxThread_t *self)
{
    int ret;
    self->bExitPending = OMX_FALSE;
#if USE_DETACHED
    pthread_attr_t attr = NULL;
    struct sched_param param;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(self->mStackSize)
        pthread_attr_setstacksize(&attr, self->mStackSize);
    if(self->mPriority == OMXTHREAD_PRIORITY_HIGH)
    {
        logd("set the thread %p as the SCHED_FIFO@50", self);
        param.sched_priority = 50;
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        pthread_attr_setschedparam(&attr, &param);
#ifndef __ANDROID__
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
#endif
    }
#endif

    ret = pthread_create(&self->mThread, NULL /*&attr*/, loopEntry, (void*)self);

#if USE_DETACHED
    pthread_attr_destroy(&attr);
#endif

    if(ret != 0)
    {
        loge("createThread failed (ret = %d, threadPriority=%ld)", ret, self->mPriority);
        return -1;
    }
    logd("+++++ self->mThread: %lu", (unsigned long)self->mThread);

    return 0;
}

static int OmxThread_Run(OmxThread_t *self)
{
    int ret;
    OmxAcquireMutex(self->lock);
    if(self->bRunning)
    {
        if(self->bSuspendRequest)
        {
            self->bRepeatToReady = OMX_TRUE;
            logd("the thread %s is repeat to ready!", self->mName);
            self->bSuspendRequest = OMX_FALSE;
            OmxTryPostSem(self->mSemSuspend);
        }
        else
        {
            logw("the thread %s is running. ignore the run() command!", self->mName);
        }
        OmxReleaseMutex(self->lock);
        return -1;
    }
    ret = createThread(self);
    if(ret == 0)
    {
        logd("thread %s start to run!", self->mName);
        self->bRunning = OMX_TRUE;
    }
    OmxReleaseMutex(self->lock);
    return ret;
}

static int OmxThread_readyToRun(OmxThread_t *self, fnReadyToRun fn)
{
    int ret;

    if(self)
    {
        OmxAcquireMutex(self->lock);
        self->mfnReadyToRun = fn;
        ret = 0;
        OmxReleaseMutex(self->lock);
    }
    else
    {
        ret = -1;
    }
    return ret;
}

static int OmxThread_setParm_Priority(OmxThread_t *self, OmxThread_Priority_t priority)
{
    int ret;
    if(self)
    {
        OmxAcquireMutex(self->lock);
        self->mPriority = priority;
        ret = 0;
        OmxReleaseMutex(self->lock);
    }
    else
    {
        ret = -1;
    }
    return ret;
}

static int OmxThread_setParm_StackSize(OmxThread_t *self, int stackSize)
{
    int ret;

    if(self)
    {
        OmxAcquireMutex(self->lock);
        self->mStackSize = stackSize;
        ret = 0;
        OmxReleaseMutex(self->lock);
    }
    else
    {
        ret = -1;
    }
    return ret;
}

static int OmxThread_suspend(OmxThread_t *self)
{
    int ret = 0;

    if(self)
    {
        OmxAcquireMutex(self->lock);
        if(!self->bSuspendRequest && self->bRunning)
        {
            logd("++++++++++suspend, %s", self->mName);
            self->bSuspendRequest = OMX_TRUE;
            OmxTimedWaitSem(self->mSemStopRun, -1);
            logd("++++++++wait stop-run sem done: %s", self->mName);
        }
        OmxReleaseMutex(self->lock);
    }
    else
    {
        ret = -1;
    }
    return ret;
}

static int OmxThread_resume(OmxThread_t *self)
{
    int ret = 0;

    if(self)
    {
        OmxAcquireMutex(self->lock);
        if(self->bSuspendRequest)
        {
            self->bSuspendRequest = OMX_FALSE;
            OmxTryPostSem(self->mSemSuspend);

            logd("++++++++++resume, %s", self->mName);
        }
        OmxReleaseMutex(self->lock);
    }
    else
    {
        ret = -1;
    }
    return ret;
}

static int OmxThread_requestExit(OmxThread_t *self)
{
    int ret = 0;

    if(self->bRunning)
    {
        OmxAcquireMutex(self->lock);
        if(self->bSuspendRequest)
        {
            self->bSuspendRequest = OMX_FALSE;
            OmxTryPostSem(self->mSemSuspend);
        }
        self->bExitPending = OMX_TRUE;
        OmxReleaseMutex(self->lock);
    }
#if !USE_DETACHED
    pthread_join(self->mThread, NULL);
#endif
    return ret;
}

static int OmxThread_requestExitAndWait(OmxThread_t *self, OMX_S32 timeout)
{
    int ret = 0;
    if(self->bRunning)
    {
        OmxAcquireMutex(self->lock);
        self->bExitPending = OMX_TRUE;
        if(self->bSuspendRequest)
        {
            self->bSuspendRequest = OMX_FALSE;
            OmxTryPostSem(self->mSemSuspend);
        }
        OmxTimedWaitSem(self->mSemExit, timeout);
        OmxReleaseMutex(self->lock);
    }
#if !USE_DETACHED
    pthread_join(self->mThread, NULL);
#endif
    return ret;
}

OmxThreadHandle OmxCreateThread(const char* name, fnThreadLoop fn, void* priv)
{
    OmxThread_t *thread;
    thread = (OmxThread_t*)malloc(sizeof(OmxThread_t));
    if(thread != NULL)
    {
        memset(thread, 0, sizeof(OmxThread_t));
        thread->mfnThreadLoop = fn;
        thread->mPriv = priv;
        thread->mPriority = OMXTHREAD_PRIORITY_NORMAL;
        thread->mName = strdup(name);
        thread->bRunning = OMX_FALSE;
        thread->bRepeatToReady = OMX_FALSE;
        OmxCreateMutex(&thread->lock, OMX_TRUE);
        thread->mSemStopRun = OmxCreateSem("stopRunSem",0,0, OMX_FALSE);
        thread->mSemSuspend = OmxCreateSem("suspendSem",0,0, OMX_FALSE);
        thread->mSemExit    = OmxCreateSem("exitSem",0,0, OMX_FALSE);

        thread->run                  = OmxThread_Run;
        thread->setFunc_readyToRun   = OmxThread_readyToRun;
        thread->setParm_Priority     = OmxThread_setParm_Priority;
        thread->setParm_StackSize    = OmxThread_setParm_StackSize;
        thread->suspend              = OmxThread_suspend;
        thread->resume               = OmxThread_resume;
        thread->requestExit          = OmxThread_requestExit;
        thread->requestExitAndWait   = OmxThread_requestExitAndWait;
    }
    return (OmxThreadHandle)thread;
}

void OmxDestroyThread(OmxThreadHandle *pSelf)
{
    OmxThreadHandle self = *pSelf;
    if(self->bRunning)
    {
        logd("running destroy");
        self->requestExitAndWait(self, -1);
    }

    OmxDestroyMutex(&self->lock);
    OmxDestroySem(&self->mSemExit);
    OmxDestroySem(&self->mSemSuspend);
    OmxDestroySem(&self->mSemStopRun);
    //destroy event
    free(self->mName);
    free(self);
    self = NULL;
}
