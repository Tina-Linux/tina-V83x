/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxWriteDirect.c
 * Description : Allwinner Muxer Writer without Cache Operation
 * History :
 *   Author  : Q Wang <eric_wang@allwinnertech.com>
 *   Date    : 2014/12/17
 *   Comment : 创建初始版本，用于V3 CedarX_1.0
 *
 *   Author  : GJ Wu <wuguanjian@allwinnertech.com>
 *   Date    : 2016/04/18
 *   Comment_: 修改移植用于CedarX_2.0
 */

#define LOG_TAG "FsWriteDirect"
#include <errno.h>
#include <cdx_log.h>
#include "CdxFsWriter.h"


typedef struct tag_FsDirectContext
{
    CdxWriterT *mp_writer;
} FsDirectContext;

static cdx_int32 FsDirectWrite(CdxFsWriter *thiz, const cdx_uint8 *buf, cdx_int32 size)
{
    FsDirectContext *p_ctx = (FsDirectContext*)thiz->m_priv;
    int ret = FileWriter(p_ctx->mp_writer, buf, size);
    if(ret < 0)
    {
        loge("FileWriter fail");
    }
    return ret;
}

static cdx_int32 FsDirectSeek(CdxFsWriter *thiz, cdx_int64 noffset, cdx_int32 nwhere)
{
    FsDirectContext *p_ctx = (FsDirectContext*)thiz->m_priv;
    cdx_int32 ret;
    thiz->fsFlush(thiz);
    ret = CdxWriterSeek(p_ctx->mp_writer, noffset, nwhere);
    return ret;
}

static cdx_int64 FsDirectTell(CdxFsWriter *thiz)
{
    FsDirectContext *p_ctx = (FsDirectContext*)thiz->m_priv;
    cdx_int64 n_file_pos;
    thiz->fsFlush(thiz);
    n_file_pos = CdxWriterTell(p_ctx->mp_writer);
    return n_file_pos;
}

static cdx_int32 FsDirectFlush(CdxFsWriter *thiz)
{
    return 0;
}

CdxFsWriter *InitFsDirectWrite(CdxWriterT *p_writer)
{
    CdxFsWriter *p_fs_writer = (CdxFsWriter*)malloc(sizeof(CdxFsWriter));
    if (NULL == p_fs_writer)
    {
        loge("(f:%s, l:%d) Failed to alloc CdxFsWriter(%s)",
            __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }
    memset(p_fs_writer, 0, sizeof(CdxFsWriter));

    FsDirectContext *p_context = (FsDirectContext*)malloc(sizeof(FsDirectContext));
    if (NULL == p_context)
    {
        loge("(f:%s, l:%d) Failed to alloc FsDirectContext(%s)",
            __FUNCTION__, __LINE__, strerror(errno));
        free(p_fs_writer);
        return NULL;
    }
    memset(p_context, 0, sizeof(FsDirectContext));

    p_context->mp_writer = p_writer;
    p_fs_writer->fsWrite = FsDirectWrite;
    p_fs_writer->fsSeek = FsDirectSeek;
    p_fs_writer->fsTell = FsDirectTell;
    p_fs_writer->fsTruncate = NULL;
    p_fs_writer->fsFlush = FsDirectFlush;
    p_fs_writer->m_mode = FSWRITEMODE_DIRECT;
    p_fs_writer->m_priv = (void*)p_context;
    return p_fs_writer;
}

cdx_int32 DeinitFsDirectWrite(CdxFsWriter *p_fs_writer)
{
    if (NULL == p_fs_writer)
    {
        loge("(f:%s, l:%d) p_fs_writer is NULL!!", __FUNCTION__, __LINE__);
        return -1;
    }
    FsDirectContext *p_context = (FsDirectContext*)p_fs_writer->m_priv;
    free(p_context);
    p_fs_writer->m_priv = NULL;
    free(p_fs_writer);
    return 0;
}
