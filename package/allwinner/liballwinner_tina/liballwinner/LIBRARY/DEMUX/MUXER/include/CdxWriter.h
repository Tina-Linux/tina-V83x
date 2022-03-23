/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxWriter.h
 * Description : Allwinner Muxer Writer Definition
 * History :
 *
 */

#ifndef __CDX_WRITER_H__
#define __CDX_WRITER_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "log.h"

typedef struct CdxWriter CdxWriterT;
struct CdxWriter{
    int (*cdxRead)(CdxWriterT *thiz, void *buf, int size);
    int (*cdxWrite)(CdxWriterT *thiz, void *buf, int size);
    long (*cdxSeek)(CdxWriterT *thiz, long moffset, int mwhere);
    long (*cdxTell)(CdxWriterT *thiz);
};

CdxWriterT *CdxWriterCreat();
void CdxWriterDestroy(CdxWriterT *writer);

#endif