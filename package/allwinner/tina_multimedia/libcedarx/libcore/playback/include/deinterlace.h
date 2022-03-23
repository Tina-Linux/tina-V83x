/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : deinterlace.h
 * Description : deinterlace
 * History :
 *
 */

#ifndef DEINTERLACE_H
#define DEINTERLACE_H

#include <cdx_config.h>
#include <vdecoder.h>
#include "cdx_log.h"

#ifdef __cplusplus
extern "C" {
#endif

enum DE_INTERLACE_FLAG
{
    DE_INTERLACE_NONE,
    DE_INTERLACE_HW,
    DE_INTERLACE_SW
};

typedef struct CdxDeinterlaceS Deinterlace;

typedef struct DeinterlaceOps DeinterlaceOpsT;
struct DeinterlaceOps
{
    void (*destroy)(Deinterlace* di);

    int (*init)(Deinterlace* di);

    int (*reset)(Deinterlace* di);

    enum EPIXELFORMAT (*expectPixelFormat)(Deinterlace* di);

    int (*flag)(Deinterlace* di);

    int (*process)(Deinterlace* di,
                        VideoPicture *pPrePicture,
                        VideoPicture *pCurPicture,
                        VideoPicture *pOutPicture,
                        int nField);

    int (*process2)(Deinterlace* di,
                        VideoPicture *pPrePicture,
                        VideoPicture *pCurPicture,
                        VideoPicture *pNextPicture,
                        VideoPicture *pOutPicture,
                        int nField);
};

struct CdxDeinterlaceS
{
    struct DeinterlaceOps *ops;
};

static inline void CdxDiDestroy(Deinterlace* di)
{
    CDX_CHECK(di);
    CDX_CHECK(di->ops);
    CDX_CHECK(di->ops->destroy);
    return di->ops->destroy(di);
}

static inline int CdxDiInit(Deinterlace* di)
{
    CDX_CHECK(di);
    CDX_CHECK(di->ops);
    CDX_CHECK(di->ops->init);
    return di->ops->init(di);
}

static inline int CdxDiReset(Deinterlace* di)
{
    CDX_CHECK(di);
    CDX_CHECK(di->ops);
    CDX_CHECK(di->ops->reset);
    return di->ops->reset(di);
}

static inline enum EPIXELFORMAT CdxDiExpectPixelFormat(Deinterlace* di)
{
    CDX_CHECK(di);
    CDX_CHECK(di->ops);
    CDX_CHECK(di->ops->expectPixelFormat);
    return di->ops->expectPixelFormat(di);
}

static inline int CdxDiFlag(Deinterlace* di)
{
    CDX_CHECK(di);
    CDX_CHECK(di->ops);
    CDX_CHECK(di->ops->flag);
    return di->ops->flag(di);
}

static inline int CdxDiProcess(Deinterlace* di,
                        VideoPicture *pPrePicture,
                        VideoPicture *pCurPicture,
                        VideoPicture *pOutPicture,
                        int nField)
{
    CDX_CHECK(di);
    CDX_CHECK(di->ops);
    CDX_CHECK(di->ops->process);
    return di->ops->process(di, pPrePicture, pCurPicture,
                    pOutPicture, nField);
}

static inline int CdxDiProcess2(Deinterlace* di,
                        VideoPicture *pPrePicture,
                        VideoPicture *pCurPicture,
                        VideoPicture *pNextPicture,
                        VideoPicture *pOutPicture,
                        int nField)
{
    CDX_CHECK(di);
    CDX_CHECK(di->ops);
    CDX_CHECK(di->ops->process2);
    return di->ops->process2(di, pPrePicture, pCurPicture,pNextPicture,
                    pOutPicture, nField);
}

#ifdef __cplusplus
}
#endif

#endif
