/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : subtitleControl_null.c
 * Description : subControl for default none output
 * History :
 *
 */
#define LOG_TAG "tsubtitlectrl"
#include "tlog.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <cdx_log.h>
#include "subtitleControl.h"
#include "tdisp_ctrl.h"

typedef struct SubCtrlContext
{
    SubCtrl                   base;
    SubCtrlCallback           pCallback;
    void*                     pUserData;
}SubCtrlContext;

int __SubCtrlDestory(SubCtrl* p)
{
    SubCtrlContext* sc = (SubCtrlContext*)p;
    TP_CHECK(sc);
    free(sc);
    return 0;
}

int __SubCtrlShow(SubCtrl* p, SubtitleItem *pItem, unsigned int nId)
{
    SubCtrlContext* sc = (SubCtrlContext*)p;
    TP_CHECK(sc);
    //TLOGD("subText: %s ", pItem->pText);
    unsigned int callbackParam[2];
    callbackParam[0] = nId;
    callbackParam[1] = pItem;
    if(sc->pCallback)
        sc->pCallback(sc->pUserData, SUBCTRL_SUBTITLE_AVAILABLE, callbackParam);
    return 0;
}

int __SubCtrlHide(SubCtrl* p, unsigned int nId)
{
    SubCtrlContext* sc = (SubCtrlContext*)p;
    TP_CHECK(sc);
    if(sc->pCallback)
        sc->pCallback(sc->pUserData, SUBCTRL_SUBTITLE_EXPIRED,(void*)&nId);
    return 0;
}

static SubControlOpsT mSubCtrl =
{
    .destory = __SubCtrlDestory,
    .show = __SubCtrlShow,
    .hide = __SubCtrlHide,
};

SubCtrl* SubtitleCreate(SubCtrlCallback pCallback, void* pUser)
{
    SubCtrlContext* p;

    p = (SubCtrlContext*)malloc(sizeof(SubCtrlContext));
    if(p == NULL)
    {
        TLOGE("malloc failed");
        return NULL;
    }
    memset(p, 0, sizeof(SubCtrlContext));

    p->base.ops = &mSubCtrl;

    p->pCallback = pCallback;
    p->pUserData = pUser;
    TLOGD("==== pCallback: %p, pUser: %p", pCallback, pUser);
    return &p->base;
}

int SubtitleDisplayOnoff(SubCtrl* p, int onoff)
{
    SubCtrlContext* sc = (SubCtrlContext*)p;
    TP_CHECK(sc);
    return 0;
}
