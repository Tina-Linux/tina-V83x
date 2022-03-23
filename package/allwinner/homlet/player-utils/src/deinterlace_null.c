/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : deinterlace_null.c
* Description : default di
* History :
*/

#include <string.h>
#include "deinterlace.h"

typedef struct DiContext
{
    Deinterlace               base;
}DiContext;

void __DiDestory(Deinterlace* di)
{
    DiContext *impl = (DiContext*)di;
    free(impl);
}

int __DiInit(Deinterlace* di)
{
    DiContext *impl = (DiContext*)di;
    return -1;
}

int __DiReset(Deinterlace* di)
{
    DiContext *impl = (DiContext*)di;
    return 0;
}

/*
enum EPIXELFORMAT
{
    PIXEL_FORMAT_DEFAULT            = 0,
    PIXEL_FORMAT_YUV_PLANER_420     = 1,
    PIXEL_FORMAT_YUV_PLANER_422     = 2,
    PIXEL_FORMAT_YUV_PLANER_444     = 3,

    PIXEL_FORMAT_YV12               = 4,
    PIXEL_FORMAT_NV21               = 5,
    PIXEL_FORMAT_NV12               = 6,
    PIXEL_FORMAT_YUV_MB32_420       = 7,
    PIXEL_FORMAT_YUV_MB32_422       = 8,
    PIXEL_FORMAT_YUV_MB32_444       = 9,

    PIXEL_FORMAT_RGBA                = 10,
    PIXEL_FORMAT_ARGB                = 11,
    PIXEL_FORMAT_ABGR                = 12,
    PIXEL_FORMAT_BGRA                = 13,

    PIXEL_FORMAT_YUYV                = 14,
    PIXEL_FORMAT_YVYU                = 15,
    PIXEL_FORMAT_UYVY                = 16,
    PIXEL_FORMAT_VYUY                = 17,

    PIXEL_FORMAT_PLANARUV_422        = 18,
    PIXEL_FORMAT_PLANARVU_422        = 19,
    PIXEL_FORMAT_PLANARUV_444        = 20,
    PIXEL_FORMAT_PLANARVU_444        = 21,

    PIXEL_FORMAT_MIN = PIXEL_FORMAT_DEFAULT,
    PIXEL_FORMAT_MAX = PIXEL_FORMAT_PLANARVU_444,
}
*/
enum EPIXELFORMAT __DiExpectPixelFormat(Deinterlace* di)
{
    DiContext *impl = (DiContext*)di;
    return 0;
}

int __Diflag(Deinterlace* di)
{
    DiContext *impl = (DiContext*)di;
    return DE_INTERLACE_NONE;
}

int __DiProcess(Deinterlace* di, VideoPicture * pPrePicture,
                  VideoPicture * pCurPicture, VideoPicture * pOutPicture,
                   int nField)
{
    DiContext *impl = (DiContext*)di;

    (void)pPrePicture;
    (void)pCurPicture;
    (void)pOutPicture;
    (void)nField;
    logd("=== process");
    return 0;
}

static struct DeinterlaceOps di =
{
    .destroy           = __DiDestory,
    .init              = __DiInit,
    .reset             = __DiReset,
    .expectPixelFormat = __DiExpectPixelFormat,
    .flag              = __Diflag,
    .process           = __DiProcess,
};


Deinterlace* DeinterlaceCreate()
{
    DiContext *impl;
    impl = (DiContext*)malloc(sizeof(DiContext));
    if(impl == NULL)
    {
        logd("malloc failed");
        return NULL;
    }
    memset(impl, 0, sizeof(DiContext));

    impl->base.ops = &di;
    return &impl->base;
}
