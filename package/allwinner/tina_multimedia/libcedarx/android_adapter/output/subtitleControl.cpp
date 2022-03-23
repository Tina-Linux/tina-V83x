/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : subtitleControl.c
 * Description : subControl for standard android api
 * History :
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include "subtitleControl.h"
#include <cdx_log.h>
#include "outputCtrl.h"

typedef struct SubCtrlContext
{
    SubCtrl                   base;
    SubCtrlCallback           pCallback;
    void                      *pUserData;
}SubCtrlContext;

static int __SubCtrlDestory(SubCtrl* p)
{
    SubCtrlContext* sc = (SubCtrlContext*)p;

    free(sc);
    return 0;
}

static int __SubCtrlShow(SubCtrl* p, SubtitleItem *pItem, unsigned int nSubtitleId)
{
    SubCtrlContext* sc = (SubCtrlContext*)p;

    logd("subtitle show. %s, id: %d", pItem->pText, nSubtitleId);
    logd("==== callback=%p, sc->pUserData: %p", sc->pCallback, sc->pUserData);

    uintptr_t callbackParam[2];
    callbackParam[0] = (uintptr_t)nSubtitleId;
    callbackParam[1] = (uintptr_t)pItem;
    if(sc->pCallback)
        sc->pCallback(sc->pUserData, SUBCTRL_SUBTITLE_AVAILABLE, callbackParam);

    return 0;
}

static int __SubCtrlHide(SubCtrl* p, unsigned int id)
{
    SubCtrlContext* sc = (SubCtrlContext*)p;

    if(sc->pCallback)
        sc->pCallback(sc->pUserData, SUBCTRL_SUBTITLE_EXPIRED,(void*)&id);

    return 0;
}

static SubControlOpsT mSubCtrl =
{
    .destory = __SubCtrlDestory,
    .show    = __SubCtrlShow,
    .hide    = __SubCtrlHide,
};

SubCtrl* SubtitleCreate(SubCtrlCallback pCallback, void* pUser)
{
    SubCtrlContext* p;

    p = (SubCtrlContext*)malloc(sizeof(SubCtrlContext));
    if(p == NULL)
    {
        loge("malloc failed");
        return NULL;
    }
    memset(p, 0, sizeof(SubCtrlContext));

    p->base.ops = &mSubCtrl;
    p->pCallback = pCallback;
    p->pUserData = pUser;
    logd("==== pCallback: %p, pUser: %p", pCallback, pUser);
    return &p->base;
}
