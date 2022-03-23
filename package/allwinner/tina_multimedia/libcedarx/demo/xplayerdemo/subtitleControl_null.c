/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : subtitleControl_null.c
 * Description : subControl for default none output
 * History :
 *
 */

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

typedef struct SubCtrlContext
{
    SubCtrl                   base;
    void*                     pUserData;
}SubCtrlContext;

int __SubCtrlDestory(SubCtrl* p)
{
    SubCtrlContext* sc = (SubCtrlContext*)p;

    return 0;
}

int __SubCtrlShow(SubCtrl* p, SubtitleItem *pItem, unsigned int nId)
{
    SubCtrlContext* sc = (SubCtrlContext*)p;
    logd("subText: %s ", pItem->pText);
    return 0;
}

int __SubCtrlHide(SubCtrl* p, unsigned int nId)
{
    SubCtrlContext* sc = (SubCtrlContext*)p;
    return 0;
}

static SubControlOpsT mSubCtrl =
{
    .destory = __SubCtrlDestory,
    .show = __SubCtrlShow,
    .hide = __SubCtrlHide,
};

SubCtrl* SubtitleCreate()
{
    SubCtrlContext* p;

    p = (SubCtrlContext*)malloc(sizeof(SubCtrl));
    if(p == NULL)
    {
        loge("malloc failed");
        return NULL;
    }
    memset(p, 0, sizeof(SubCtrl));

    p->base.ops = &mSubCtrl;

    return &p->base;
}
