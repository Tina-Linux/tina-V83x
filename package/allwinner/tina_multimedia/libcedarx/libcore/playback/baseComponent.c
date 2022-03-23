/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : baseComponent.c
 * Description : base component
 * History :
 *     Date     : 2016/08/03
 *     Comment  : first version
 */

#include "baseComponent.h"
#include <cdx_log.h>
#include <CdxTypes.h>

#include <stdlib.h>

int BaseCompInit(BaseCompCtx *p, const char *name,
                AwMessageQueue *mq, BaseMsgHandler *handler)
{
    if (!p || !name || !mq || !handler)
    {
        loge("p %p, name %p, mq %p, handler %p", p, name, mq, handler);
        return -1;
    }

    p->name = name;
    p->mq = mq;

    size_t i;
    for (i = 0; i < ARRAY_SIZE(p->replySem); i++)
        sem_init(&p->replySem[i], 0, 0);

    p->handler = *handler;

    return 0;
}

void BaseCompDestroy(BaseCompCtx *p)
{
    if (p == NULL)
        return;

    size_t i;
    for (i = 0; i < ARRAY_SIZE(p->replySem); i++)
        sem_destroy(&p->replySem[i]);
}

static int BaseCompPostAndWait(BaseCompCtx *p, AwMessage *msg,
        task_t afterPostBeforeWait, void *arg)
{
    static const char *const msgName[] = {
        [MESSAGE_ID_START] = "start",
        [MESSAGE_ID_STOP] = "stop",
        [MESSAGE_ID_PAUSE] = "pause",
        [MESSAGE_ID_QUIT] = "quit",
        [MESSAGE_ID_RESET] = "reset",
        [MESSAGE_ID_EOS] = "eos",
    };

    logd("%s receive cmd: %s", p->name, msgName[msg->messageId]);

    if (AwMessageQueuePostMessage(p->mq, msg) != 0)
    {
        loge("fatal error, %s post message fail", p->name);
        abort();
    }

    if (afterPostBeforeWait != NULL)
        afterPostBeforeWait(arg);

    if (SemTimedWait(msg->replySem, -1) < 0)
    {
        loge("%s wait for %s finish failed", p->name, msgName[msg->messageId]);
        abort();
    }

    return 0;
}

int BaseCompStart(BaseCompCtx *p, task_t afterPostBeforeWait, void *arg)
{
    int reply = 0;
    AwMessage msg = {
        .messageId = MESSAGE_ID_START,
        .execute = p->handler.start,
        .replySem = &p->replySem[MESSAGE_ID_START],
        .result = &reply,
    };

    BaseCompPostAndWait(p, &msg, afterPostBeforeWait, arg);

    return reply;
}

int BaseCompStop(BaseCompCtx *p, task_t afterPostBeforeWait, void *arg)
{
    int reply = 0;
    AwMessage msg = {
        .messageId = MESSAGE_ID_STOP,
        .execute = p->handler.stop,
        .replySem = &p->replySem[MESSAGE_ID_STOP],
        .result = &reply,
    };

    BaseCompPostAndWait(p, &msg, afterPostBeforeWait, arg);

    return reply;
}

int BaseCompPause(BaseCompCtx *p, task_t afterPostBeforeWait, void *arg)
{
    int reply = 0;
    AwMessage msg = {
        .messageId = MESSAGE_ID_PAUSE,
        .execute = p->handler.pause,
        .replySem = &p->replySem[MESSAGE_ID_PAUSE],
        .result = &reply,
    };

    BaseCompPostAndWait(p, &msg, afterPostBeforeWait, arg);

    return reply;
}

int BaseCompReset(BaseCompCtx *p, int64_t nSeekTime,
        task_t afterPostBeforeWait, void *arg)
{
    int reply = 0;
    AwMessage msg = {
        .messageId = MESSAGE_ID_RESET,
        .execute = p->handler.reset,
        .replySem = &p->replySem[MESSAGE_ID_RESET],
        .result = &reply,
        .seekTime = nSeekTime,
    };

    BaseCompPostAndWait(p, &msg, afterPostBeforeWait, arg);

    return reply;
}

int BaseCompSetEos(BaseCompCtx *p, task_t afterPostBeforeWait, void *arg)
{
    AwMessage msg = {
        .messageId = MESSAGE_ID_EOS,
        .execute = p->handler.setEos,
        .replySem = &p->replySem[MESSAGE_ID_EOS],
    };

    BaseCompPostAndWait(p, &msg, afterPostBeforeWait, arg);

    return 0;
}

int BaseCompQuit(BaseCompCtx *p, task_t afterPostBeforeWait, void *arg)
{
    AwMessage msg = {
        .messageId = MESSAGE_ID_QUIT,
        .execute = p->handler.quit,
        .replySem = &p->replySem[MESSAGE_ID_QUIT],
    };

    BaseCompPostAndWait(p, &msg, afterPostBeforeWait, arg);

    return 0;
}

int BaseCompContinue(BaseCompCtx *p)
{
    if (AwMessageQueueGetCount(p->mq) > 0)
        return 0;

    AwMessage msg = {
        .messageId = MESSAGE_ID_CONTINUE,
        .execute = p->handler.continueWork,
    };

    if (AwMessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, %s post message fail", p->name);
        abort();
    }

    return 0;
}
