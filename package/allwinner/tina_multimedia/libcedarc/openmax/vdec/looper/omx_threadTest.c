/*
 * =====================================================================================
 *
 *       Filename:  omx_threadTest.c
 *
 *    Description: this file is a test for omx_thread.c,
 *                        should move the include files here when using the compiler gcc
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

#include <stdio.h>
#include <unistd.h>
#include "omx_thread.h"
#include <string.h>
#include <malloc.h>
#include "log.h"


OmxThreadHandle gThreads_eat;
OmxThreadHandle gThreads_work;
OmxThreadHandle gThreads_sleep;

struct person{
    OMX_S32 id;
    char *name;
    OMX_S32  age;
    OMX_S32 times;
};
struct person father;
struct person mother;
struct person child;

static OMX_BOOL EatThreadEntry(void *priv){
    if (priv == NULL)
        loge("priv is NULL");
    else{
        struct person *p = (struct person *)priv;

        //logd("*****thread eating: times:%d, id=%d, name=%s, age=%d ------", ++p->times, p->id, p->name, p->age);
        usleep(3000);
    }
    return OMX_TRUE;
}

static OMX_BOOL EatThread_ReadyToRun(void *priv){
    struct person *p = (struct person *)priv;

    logd("%s is ready to eat!", p->name);

    return OMX_TRUE;
}

static OMX_BOOL WorkThreadEntry(void *priv){
    struct person *p = (struct person *)priv;

    //logd("thread working: times:%d, id=%d, name=%s, age=%d +++++++", ++p->times, p->id, p->name, p->age);
    usleep(3000);
    return OMX_TRUE;
}

static OMX_BOOL SleepThreadEntry(void *priv){
    struct person *p = (struct person *)priv;

    //logd("thread sleeping: times:%d, id=%d, name=%s, age=%d  ******", ++p->times, p->id, p->name, p->age);
    usleep(5000);
    return OMX_TRUE;
}


int main(){

    father.name = strdup("father");
    father.age  = 40;
    father.id   = 1;
    father.times= 0;

    mother.name = strdup("mother");
    mother.age  = 30;
    mother.id   = 2;
    mother.times= 0;

    child.name = strdup("child");
    child.age  = 10;
    child.id   = 3;
    child.times= 0;

    gThreads_eat = OmxCreateThread("eating", EatThreadEntry, (void *)&father);
    //gThreads_eat->setFunc_readyToRun(gThreads_eat, EatThread_ReadyToRun);
    gThreads_eat->run(gThreads_eat);
   // gThreads_eat->setParm_Priority(gThreads_eat, OMXTHREAD_PRIORITY_HIGH);

    gThreads_work = OmxCreateThread("working", WorkThreadEntry, (void *)&mother);
    gThreads_work->run(gThreads_work);

    gThreads_sleep = OmxCreateThread("sleeping", SleepThreadEntry, (void *)&child);
    gThreads_sleep->run(gThreads_sleep);

    gThreads_eat->suspend(gThreads_eat);
    //sleep(1);
    //gThreads_eat->resume(gThreads_eat);
    sleep(2);

    gThreads_eat->requestExitAndWait(gThreads_eat, -1);
    gThreads_work->requestExitAndWait(gThreads_work, -1);
    gThreads_sleep->requestExitAndWait(gThreads_sleep, -1);


    OmxDestroyThread(&gThreads_eat);
    OmxDestroyThread(&gThreads_work);
    OmxDestroyThread(&gThreads_sleep);
    free(mother.name);
    free(father.name);
    free(child.name);

    return 0;
}
