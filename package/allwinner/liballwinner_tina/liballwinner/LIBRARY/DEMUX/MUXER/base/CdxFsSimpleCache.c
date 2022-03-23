/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxFsSimpleCache.c
 * Description : Allwinner Muxer Writer with Simple Cache Operation
 * History :
 *   Author  : Q Wang <eric_wang@allwinnertech.com>
 *   Date    : 2014/12/17
 *   Comment : 创建初始版本，用于V3 CedarX_1.0
 *
 *   Author  : GJ Wu <wuguanjian@allwinnertech.com>
 *   Date    : 2016/04/18
 *   Comment_: 修改移植用于CedarX_2.0
 */

#define LOG_TAG "FsSimpleCache"
#include <log.h>
#include <errno.h>
#include <stdlib.h>

#include <CdxTsemaphore.h>
#include <CdxFsWriter.h>

//#define SHOW_FWRITE_TIME

typedef struct tag_FsSimpleCacheContext
{
    CdxWriterT *mp_writer;
    cdx_int8      *mp_cache;
    cdx_int32     m_cache_size;
    cdx_int32     m_valid_len;
}FsSimpleCacheContext;

static cdx_int32 FsSimpleCacheWrite(CdxFsWriter *thiz, const cdx_int8 *buf, cdx_int32 size)
{
    FsSimpleCacheContext *p_ctx = (FsSimpleCacheContext*)thiz->m_priv;
    cdx_int32 n_left_size, ret;
    if(0 == size || NULL == buf)
    {
        loge("(f:%s, l:%d) Invalid input paramter!", __FUNCTION__, __LINE__);
        return -1;
    }
    if(p_ctx->m_valid_len >= p_ctx->m_cache_size)
    {
        loge("(f:%s, l:%d) fatal error! [%d]>=[%d], check code!",
            __FUNCTION__, __LINE__, p_ctx->m_valid_len, p_ctx->m_cache_size);
        return -1;
    }
    if(p_ctx->m_valid_len > 0)
    {
        if(p_ctx->m_valid_len + size <= p_ctx->m_cache_size)
        {
            memcpy(p_ctx->mp_cache + p_ctx->m_valid_len, buf, size);
            p_ctx->m_valid_len+=size;
            if(p_ctx->m_valid_len == p_ctx->m_cache_size)
            {
                ret = FileWriter(p_ctx->mp_writer, p_ctx->mp_cache, p_ctx->m_cache_size);
                if (ret < 0)
                {
                    loge("(f:%s, l:%d) FileWriter() failed", __FUNCTION__, __LINE__);
                    return ret;
                }
                p_ctx->m_valid_len = 0;
            }
            return size;
        }
        else
        {
            cdx_int32 n_size0 = p_ctx->m_cache_size - p_ctx->m_valid_len;
            n_left_size = size - n_size0;
            memcpy(p_ctx->mp_cache + p_ctx->m_valid_len, buf, n_size0);
            ret = FileWriter(p_ctx->mp_writer, p_ctx->mp_cache, p_ctx->m_cache_size);
            if (ret < 0)
            {
                loge("(f:%s, l:%d) FileWriter() failed", __FUNCTION__, __LINE__);
                return ret;
            }
            p_ctx->m_valid_len = 0;
        }
    }
    else
    {
        n_left_size = size;
    }

    if(n_left_size >= p_ctx->m_cache_size)
    {
        size_t n_write_size = (n_left_size / p_ctx->m_cache_size) * p_ctx->m_cache_size;
        logv("(f:%s, l:%d) [%d], direct fwrite[%d]kB!",
            __FUNCTION__, __LINE__, size-n_left_size, n_left_size / 1024);
        ret = FileWriter(p_ctx->mp_writer, buf+(size-n_left_size), n_write_size);
        if (ret < 0)
        {
            loge("(f:%s, l:%d) FileWriter() failed", __FUNCTION__, __LINE__);
            return ret;
        }
        n_left_size -= n_write_size;
    }
    if(n_left_size > 0)
    {
        memcpy(p_ctx->mp_cache, buf + (size - n_left_size), n_left_size);
        p_ctx->m_valid_len = n_left_size;
    }
    return size;
}

static cdx_int32 FsSimpleCacheSeek(CdxFsWriter *thiz, cdx_int64 noffset, cdx_int32 nwhere)
{
    FsSimpleCacheContext *p_ctx = (FsSimpleCacheContext*)thiz->m_priv;
    cdx_int32 ret;
    thiz->fsFlush(thiz);
    ret = p_ctx->mp_writer->cdxSeek(p_ctx->mp_writer, noffset, nwhere);
    return ret;
}

static cdx_int64 FsSimpleCacheTell(CdxFsWriter *thiz)
{
    FsSimpleCacheContext *p_ctx = (FsSimpleCacheContext*)thiz->m_priv;
    cdx_int64 n_file_pos;
    thiz->fsFlush(thiz);
    n_file_pos = p_ctx->mp_writer->cdxTell(p_ctx->mp_writer);
    return n_file_pos;
}

static cdx_int32 FsSimpleCacheFlush(CdxFsWriter *thiz)
{
    FsSimpleCacheContext *p_ctx = (FsSimpleCacheContext*)thiz->m_priv;
    if(p_ctx->m_valid_len > 0)
    {
        cdx_int32 ret = FileWriter(p_ctx->mp_writer, p_ctx->mp_cache, p_ctx->m_valid_len);
        if (ret < 0)
        {
            loge("(f:%s, l:%d) FileWriter() failed", __FUNCTION__, __LINE__);
            return ret;
        }
        p_ctx->m_valid_len = 0;
    }
    return 0;
}

CdxFsWriter *InitFsSimpleCache(CdxWriterT *p_writer, cdx_int32 n_cache_size)
{
    if(n_cache_size<=0)
    {
        loge("(f:%s, l:%d) param[%d] wrong![%s]",
            __FUNCTION__, __LINE__, n_cache_size, strerror(errno));
        return NULL;
    }
    CdxFsWriter *p_fs_writer = (CdxFsWriter*)malloc(sizeof(CdxFsWriter));
    if (NULL == p_fs_writer)
    {
        loge("(f:%s, l:%d) Failed to alloc CdxFsWriter(%s)",
            __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }
    memset(p_fs_writer, 0, sizeof(CdxFsWriter));
    FsSimpleCacheContext *p_context = (FsSimpleCacheContext*)malloc(sizeof(FsSimpleCacheContext));
    if (NULL == p_context)
    {
        loge("(f:%s, l:%d) Failed to alloc FsSimpleCacheContext(%s)",
            __FUNCTION__, __LINE__, strerror(errno));
        goto ERROR0;
    }
    memset(p_context, 0, sizeof(FsSimpleCacheContext));
    p_context->mp_writer = p_writer;
    int ret = posix_memalign((void **)&p_context->mp_cache, 4096, n_cache_size);
    if(ret!=0)
    {
        loge("(f:%s, l:%d) fatal error! malloc [%d]kByte fail.",
        __FUNCTION__, __LINE__, n_cache_size / 1024);
        goto ERROR1;
    }
    p_context->m_valid_len = 0;
    p_context->m_cache_size = n_cache_size;
    p_fs_writer->fsWrite = FsSimpleCacheWrite;
    p_fs_writer->fsSeek = FsSimpleCacheSeek;
    p_fs_writer->fsTell = FsSimpleCacheTell;
    p_fs_writer->fsTruncate = NULL;
    p_fs_writer->fsFlush = FsSimpleCacheFlush;
    p_fs_writer->m_mode = FSWRITEMODE_SIMPLECACHE;
    p_fs_writer->m_priv = (void*)p_context;
    return p_fs_writer;

ERROR1:
    free(p_context);
ERROR0:
    free(p_fs_writer);
    return NULL;
}

cdx_int32 DeinitFsSimpleCache(CdxFsWriter *p_fs_writer)
{
    if (NULL == p_fs_writer)
    {
        loge("(f:%s, l:%d) p_fs_writer is NULL!!", __FUNCTION__, __LINE__);
        return -1;
    }
    p_fs_writer->fsFlush(p_fs_writer);
    FsSimpleCacheContext *p_context = (FsSimpleCacheContext*)p_fs_writer->m_priv;
    if (NULL == p_context)
    {
        loge("(f:%s, l:%d) p_context is NULL!!", __FUNCTION__, __LINE__);
        return -1;
    }
    if(p_context->mp_cache)
    {
        free(p_context->mp_cache);
        p_context->mp_cache = NULL;
    }
    free(p_context);
    p_fs_writer->m_priv = NULL;
    free(p_fs_writer);
    return 0;
}
