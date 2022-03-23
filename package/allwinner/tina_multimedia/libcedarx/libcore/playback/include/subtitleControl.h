/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : subtitleControl.h
 * Description : subtitleControl
 * History :
 *
 */

#ifndef __SUB_CONTROL_H
#define __SUB_CONTROL_H

#include "cdx_log.h"
#include "sdecoder.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SubCtrl SubCtrl;

typedef struct SubtitleControlOpsS SubControlOpsT;

struct SubtitleControlOpsS
{
    int (*destory)(SubCtrl* p);
    int (*show)(SubCtrl* p, SubtitleItem *pItem, unsigned int id);
    int (*hide)(SubCtrl* p, unsigned int id);
};

struct SubCtrl
{
    struct SubtitleControlOpsS* ops;
};

static inline int CdxSubRenderDestory(SubCtrl* p)
{
    CDX_CHECK(p);
    CDX_CHECK(p->ops);
    CDX_CHECK(p->ops->destory);
    return p->ops->destory(p);
}

static inline int CdxSubRenderShow(SubCtrl* p, SubtitleItem *pItem, unsigned int id)
{
    CDX_CHECK(p);
    CDX_CHECK(p->ops);
    CDX_CHECK(p->ops->show);
    return p->ops->show(p, pItem, id);
}

static inline int CdxSubRenderHide(SubCtrl* p, unsigned int id)
{
    CDX_CHECK(p);
    CDX_CHECK(p->ops);
    CDX_CHECK(p->ops->hide);
    return p->ops->hide(p, id);
}

#ifdef __cplusplus
}
#endif

#endif
