/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : layerControl.h
 * Description : layer control
 * History :
 *
 */

#ifndef LAYER_CONTROL
#define LAYER_CONTROL

#include "cdx_log.h"
#include <CdxTypes.h>

#include "vdecoder.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LayerCtrl LayerCtrl;

typedef struct LayerControlOpsS
{
    void (*release)(LayerCtrl*);

    int (*setSecureFlag)(LayerCtrl*, int);

    int (*setDisplayBufferSize)(LayerCtrl* , int , int );

    int (*setDisplayBufferCount)(LayerCtrl* , int );

    int (*setDisplayRegion)(LayerCtrl* , int , int , int , int );

    int (*setDisplayPixelFormat)(LayerCtrl* , enum EPIXELFORMAT );

    int (*setVideoWithTwoStreamFlag)(LayerCtrl* , int );

    int (*setIsSoftDecoderFlag)(LayerCtrl* , int);

    int (*setBufferTimeStamp)(LayerCtrl* , int64_t );

    void (*resetNativeWindow)(LayerCtrl* ,void*);

    VideoPicture* (*getBufferOwnedByGpu)(LayerCtrl* );

    int (*getDisplayFPS)(LayerCtrl* );

    int (*getBufferNumHoldByGpu)(LayerCtrl* );

    int (*ctrlShowVideo)(LayerCtrl* );

    int (*ctrlHideVideo)(LayerCtrl*);

    int (*ctrlIsVideoShow)(LayerCtrl* );

    int (*ctrlHoldLastPicture)(LayerCtrl* , int );

    int (*dequeueBuffer)(LayerCtrl* , VideoPicture** , int);

    int (*queueBuffer)(LayerCtrl* , VideoPicture* , int);

    int (*releaseBuffer)(LayerCtrl* ,VideoPicture*);

    int (*reset)(LayerCtrl*);

    void (*destroy)(LayerCtrl*);

    int (*control)(LayerCtrl*, cdx_int32 , void *);
}LayerControlOpsT;

struct LayerCtrl
{
    struct LayerControlOpsS* ops;
    void*  pNativeWindow;  // only for android
};

enum CdxLayerCommandE
{
    CDX_LAYER_CMD_RESTART_SCHEDULER = 0x100,
    CDX_LAYER_CMD_SET_HDR_INFO      = 0x101,
    CDX_LAYER_CMD_SET_NATIVE_WINDOW = 0x102,
    CDX_LAYER_CMD_SET_VIDEOCODEC_FORMAT = 0x103,
};

static void LayerRelease(LayerCtrl* l)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->release);
    return l->ops->release(l);
}

static int LayerSetSecureFlag(LayerCtrl* l, int f)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->setSecureFlag);
    return l->ops->setSecureFlag(l, f);
}

static int LayerSetDisplayBufferSize(LayerCtrl* l, int w, int h)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->setDisplayBufferSize);
    return l->ops->setDisplayBufferSize(l, w, h);
}

static int LayerSetDisplayBufferCount(LayerCtrl* l, int n)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->setDisplayBufferCount);
    return l->ops->setDisplayBufferCount(l, n);
}

static int LayerSetDisplayRegion(LayerCtrl* l, int left, int top,
                                    int w, int h)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->setDisplayRegion);
    return l->ops->setDisplayRegion(l, left, top, w, h);
}

static int LayerSetDisplayPixelFormat(LayerCtrl* l, enum EPIXELFORMAT p)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->setDisplayPixelFormat);
    return l->ops->setDisplayPixelFormat(l, p);
}

static int LayerSetVideoWithTwoStreamFlag(LayerCtrl* l, int f)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->setVideoWithTwoStreamFlag);
    return l->ops->setVideoWithTwoStreamFlag(l, f);
}

static int LayerSetIsSoftDecoderFlag(LayerCtrl* l, int flag)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->setIsSoftDecoderFlag);
    return l->ops->setIsSoftDecoderFlag(l, flag);
}

static int LayerSetBufferTimeStamp(LayerCtrl* l, int64_t pts)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->setBufferTimeStamp);
    return l->ops->setBufferTimeStamp(l, pts);
}

static void LayerResetNativeWindow(LayerCtrl* l,void* p)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->resetNativeWindow);
    return l->ops->resetNativeWindow(l, p);
}

static VideoPicture* LayerGetBufferOwnedByGpu(LayerCtrl* l)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->getBufferOwnedByGpu);
    return l->ops->getBufferOwnedByGpu(l);
}

static int LayerGetDisplayFPS(LayerCtrl* l)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->getDisplayFPS);
    return l->ops->getDisplayFPS(l);
}

static int LayerGetBufferNumHoldByGpu(LayerCtrl* l)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->getBufferNumHoldByGpu);
    return l->ops->getBufferNumHoldByGpu(l);
}

static int LayerCtrlShowVideo(LayerCtrl* l)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->ctrlShowVideo);
    return l->ops->ctrlShowVideo(l);
}

static int LayerCtrlHideVideo(LayerCtrl* l)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->ctrlHideVideo);
    return l->ops->ctrlHideVideo(l);
}

static int LayerCtrlIsVideoShow(LayerCtrl* l)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->ctrlIsVideoShow);
    return l->ops->ctrlIsVideoShow(l);
}

static int LayerCtrlHoldLastPicture(LayerCtrl* l, int h)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->ctrlHoldLastPicture);
    return l->ops->ctrlHoldLastPicture(l, h);
}

static int LayerDequeueBuffer(LayerCtrl* l, VideoPicture** p, int f)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->dequeueBuffer);
    return l->ops->dequeueBuffer(l, p, f);
}

static int LayerQueueBuffer(LayerCtrl* l, VideoPicture* p, int f)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->queueBuffer);
    return l->ops->queueBuffer(l, p, f);
}

static int LayerReleaseBuffer(LayerCtrl* l,VideoPicture* p)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->releaseBuffer);
    return l->ops->releaseBuffer(l, p);
}

static int LayerReset(LayerCtrl* l)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->reset);
    return l->ops->reset(l);
}

static void LayerDestroy(LayerCtrl* l)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->destroy);
    return l->ops->destroy(l);
}

static int LayerControl(LayerCtrl* l, int cmd, void *para)
{
    CDX_CHECK(l);
    CDX_CHECK(l->ops);
    CDX_CHECK(l->ops->control);
    return l->ops->control(l, cmd, para);
}

#ifdef __cplusplus
}
#endif

#endif
