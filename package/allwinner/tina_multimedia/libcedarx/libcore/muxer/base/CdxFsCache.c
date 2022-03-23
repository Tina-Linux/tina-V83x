/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxFsCache.c
 * Description : Allwinner Muxer Writer with Thread Cache Operation
 * History :
 *   Author  : Q Wang <eric_wang@allwinnertech.com>
 *   Date    : 2014/12/17
 *   Comment : 创建初始版本，用于V3 CedarX_1.0
 *
 *   Author  : GJ Wu <wuguanjian@allwinnertech.com>
 *   Date    : 2016/04/18
 *   Comment_: 修改移植用于CedarX_2.0
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "FsCache"
#include <cdx_log.h>
#include <errno.h>

#include <CdxTsemaphore.h>
#include <CdxFsWriter.h>

#define WRITE_BLOCK_SIZE  (128*1024)  //unit:byte.

typedef struct tag_FsCacheThreadContext
{
    CdxWriterT         *mp_writer;
    cdx_uint8           *mp_cache;
    cdx_int32          m_cache_size;
    pthread_t          m_thread_id;
    volatile cdx_int32 m_thread_exit_flag;

    volatile cdx_uint8  *m_write_ptr;      //fwrite modify it, thread read it.
    volatile cdx_uint8  *m_read_ptr;       //fwrite read it, thread modify it.
    CdxSemT            m_write_start_sem; //fwrite notify thread to start to write data to fs.
    cdx_int32          m_flush_flag;      //fwrite modify it to 1, thread read it and clear it to 0
    pthread_mutex_t    m_flush_lock;      //work with m_flush_flag
    CdxSemT            m_flush_done_sem;  //work with m_flush_flag, thread notify to fwrite flush
    CdxSemT            m_write_done_sem;
    cdx_uint32         m_video_codec_id;
    int                m_write_error_flag;
}FsCacheThreadContext;

static cdx_int32 FsCacheThreadWrite(CdxFsWriter *thiz, const cdx_uint8 *buf, cdx_int32 size)
{
    FsCacheThreadContext *p_ctx = (FsCacheThreadContext*)thiz->m_priv;
    cdx_uint8 *p_rd_ptr;
    cdx_uint8 *p_wr_ptr;
    cdx_int32 n_free_size = 0;
    cdx_int32 n_free_size_section1;
    cdx_int32 n_free_size_section2;

    if (0 == size)
    {
        return 0;
    }

    while (p_ctx->m_write_error_flag == 0)
    {
        p_rd_ptr = (cdx_uint8*)p_ctx->m_read_ptr;
        p_wr_ptr = (cdx_uint8*)p_ctx->m_write_ptr;
        if (p_wr_ptr >= p_rd_ptr)
        {
            n_free_size_section1 = p_rd_ptr - p_ctx->mp_cache;
            n_free_size_section2 = p_ctx->mp_cache+p_ctx->m_cache_size-p_wr_ptr;
            if (n_free_size_section1 > 0)
            {
                n_free_size_section1 -= 1;
            }
            else if (n_free_size_section2>0)
            {
                n_free_size_section2-=1;
            }
            else
            {
                loge("(f:%s, l:%d) fatal error! at lease left one byte! check code!",
                    __FUNCTION__, __LINE__);
            }

            n_free_size = n_free_size_section1+n_free_size_section2;
            if (n_free_size >= size)
            {
                if (n_free_size_section2 >= size)
                {
                    memcpy(p_wr_ptr, buf, size);
                    p_wr_ptr+=size;
                    if (p_wr_ptr == p_ctx->mp_cache + p_ctx->m_cache_size)
                    {
                        p_wr_ptr = p_ctx->mp_cache;
                    }
                    else if (p_wr_ptr > p_ctx->mp_cache+p_ctx->m_cache_size)
                    {
                        loge("(f:%s, l:%d) fatal error! check code!", __FUNCTION__, __LINE__);
                    }
                }
                else
                {
                    memcpy(p_wr_ptr, buf, n_free_size_section2);
                    memcpy(p_ctx->mp_cache, buf+n_free_size_section2, size-n_free_size_section2);
                    p_wr_ptr = p_ctx->mp_cache+size-n_free_size_section2;
                }
                p_ctx->m_write_ptr = p_wr_ptr;
                break;
            }
            else
            {
                cdx_int64 tm1, tm2;
                logd("<F:%s, L:%d>codecID[%d], n_free_size[%d]<size[%d], wait begin",
                    __FUNCTION__, __LINE__, p_ctx->m_video_codec_id, n_free_size, size);
                tm1 = FsGetNowUs();
                CdxSemUpUnique(&p_ctx->m_write_start_sem);

                if (p_ctx->m_write_error_flag == 1)
                    return -1;

                CdxSemWait(&p_ctx->m_write_done_sem);
                tm2 = FsGetNowUs();
                logd("<F:%s, L:%d>codecID[%d], n_free_size[%d]<size[%d], wait end [%lld]ms",
                    __FUNCTION__, __LINE__, p_ctx->m_video_codec_id, n_free_size, size,
                    (tm2 - tm1) / 1000);
            }
        }
        else //p_wr_ptr < p_rd_ptr
        {
            n_free_size = p_rd_ptr-p_wr_ptr - 1;
            if (n_free_size>=size)
            {
                memcpy(p_wr_ptr, buf, size);
                p_wr_ptr+=size;
                p_ctx->m_write_ptr = p_wr_ptr;
                break;
            }
            else
            {
                cdx_int64 tm1, tm2;
                logd("<F:%s, L:%d>codecID[%d], n_free_size[%d]<size[%d], wait begin",
                    __FUNCTION__, __LINE__, p_ctx->m_video_codec_id, n_free_size, size);
                tm1 = FsGetNowUs();
                CdxSemUpUnique(&p_ctx->m_write_start_sem);

                if (p_ctx->m_write_error_flag == 1)
                    return -1;

                CdxSemWait(&p_ctx->m_write_done_sem);
                tm2 = FsGetNowUs();
                logd("<F:%s, L:%d>codecID[%d], n_free_size[%d]<size[%d], wait end [%lld]ms",
                    __FUNCTION__, __LINE__, p_ctx->m_video_codec_id, n_free_size, size,
                    (tm2 - tm1) / 1000);
            }
        }
    }
    if (p_ctx->m_write_error_flag == 1)
        return -1;
    if (p_ctx->m_cache_size - (n_free_size + 1 - size) >= WRITE_BLOCK_SIZE)
    {
        CdxSemUpUnique(&p_ctx->m_write_start_sem);
    }
    return size;
}

static cdx_int32 FsCacheThreadSeek(CdxFsWriter *thiz, cdx_int64 noffset, cdx_int32 nwhere)
{
    FsCacheThreadContext *p_ctx = (FsCacheThreadContext*)thiz->m_priv;
    cdx_int32 ret;
    thiz->fsFlush(thiz);
    ret = CdxWriterSeek(p_ctx->mp_writer, noffset, nwhere);
    return ret;
}

static cdx_int64 FsCacheThreadTell(CdxFsWriter *thiz)
{
    FsCacheThreadContext *p_ctx = (FsCacheThreadContext*)thiz->m_priv;
    cdx_int64 n_file_pos;
    thiz->fsFlush(thiz);
    n_file_pos = CdxWriterTell(p_ctx->mp_writer);
    return n_file_pos;
}

static cdx_int32 FsCacheThreadFlush(CdxFsWriter *thiz)
{
    FsCacheThreadContext *p_ctx = (FsCacheThreadContext*)thiz->m_priv;
    cdx_uint8 *p_rd_ptr = (cdx_uint8*)p_ctx->m_read_ptr;
    cdx_uint8 *p_wr_ptr = (cdx_uint8*)p_ctx->m_write_ptr;

    if (p_rd_ptr == p_wr_ptr)
    {
        return 0;
    }

    if (p_ctx->m_write_error_flag == 1)
        return -1;

    cdx_mutex_lock(&p_ctx->m_flush_lock);
    p_ctx->m_flush_flag = 1;
    CdxSemUpUnique(&p_ctx->m_write_start_sem);
    cdx_mutex_unlock(&p_ctx->m_flush_lock);
    CdxSemDown(&p_ctx->m_flush_done_sem);
    p_rd_ptr = (cdx_uint8*)p_ctx->m_read_ptr;
    p_wr_ptr = (cdx_uint8*)p_ctx->m_write_ptr;
    if (p_rd_ptr != p_wr_ptr)
    {
        loge("(f:%s, l:%d) fatal error! Flush fail[%p][%p]?",
            __FUNCTION__, __LINE__, p_rd_ptr, p_wr_ptr);
        return -1;
    }
    logd("<F:%s, L:%d>mp_cache=%p, m_read_ptr=%p, m_write_ptr=%p, m_cache_size=%d",
        __FUNCTION__, __LINE__, p_ctx->mp_cache, p_ctx->m_read_ptr, p_ctx->m_write_ptr,
        p_ctx->m_cache_size);
    return 0;
}

static CDX_PTR FsCacheWriteThread(CDX_PTR p_thread_data)
{
    cdx_int32 ret = 0;
    FsCacheThreadContext *p_ctx = (FsCacheThreadContext *)p_thread_data;
    cdx_uint8 *p_rd_ptr;
    cdx_uint8 *p_wr_ptr;
    cdx_int32 n_valid_size;
    cdx_int32 n_valid_size_section1;
    cdx_int32 n_valid_size_section2;
    cdx_int32 n_write_block_num;

    //logv("FsCacheWriteThread[%d] started", p_ctx->m_thread_id);
    while (1)
    {
        CdxSemDown(&p_ctx->m_write_start_sem);
        //(1) detect basic info before start to fwrite().
        p_rd_ptr = (cdx_uint8*)p_ctx->m_read_ptr;
        p_wr_ptr = (cdx_uint8*)p_ctx->m_write_ptr;
        if (p_wr_ptr >= p_rd_ptr)
        {
            n_valid_size = p_wr_ptr - p_rd_ptr;
        }
        else //p_wr_ptr < p_rd_ptr
        {
            n_valid_size_section1 = p_wr_ptr - p_ctx->mp_cache;
            n_valid_size_section2 = p_ctx->mp_cache + p_ctx->m_cache_size-p_rd_ptr;
            n_valid_size = n_valid_size_section1 + n_valid_size_section2;
        }
        n_write_block_num = n_valid_size / WRITE_BLOCK_SIZE;
        if (n_write_block_num>0)
        {
            if (n_write_block_num>1)
            {
                logv("(f:%s, l:%d) n_valid_size[%d]kB, n_write_block_num[%d], maybe write slow?",
                    __FUNCTION__, __LINE__, n_valid_size / 1024, n_write_block_num);
            }
        }
        else
        {
            logv("(f:%s, l:%d) n_valid_size[%d]kB is not enough!",
                __FUNCTION__, __LINE__, n_valid_size / 1024);
        }
        //(2) start to fwrite()
        while (1)
        {
            int ret;
            p_rd_ptr = (cdx_uint8*)p_ctx->m_read_ptr;
            p_wr_ptr = (cdx_uint8*)p_ctx->m_write_ptr;
            if (p_wr_ptr >= p_rd_ptr)
            {
                n_valid_size = p_wr_ptr - p_rd_ptr;
                n_write_block_num = n_valid_size/WRITE_BLOCK_SIZE;
                if (n_write_block_num>0)
                {
                    //n_write_block_num = 1;
                    ret = FileWriter(p_ctx->mp_writer, p_rd_ptr,
                        WRITE_BLOCK_SIZE * n_write_block_num);
                    if(ret < 0)
                    {
                        loge("FileWriter error");
                        goto EXIT;
                    }

                    p_rd_ptr += WRITE_BLOCK_SIZE*n_write_block_num;
                    p_ctx->m_read_ptr = p_rd_ptr;
                    CdxSemSignal(&p_ctx->m_write_done_sem);
                }
                else
                {
                    break;
                }
            }
            else //p_wr_ptr < p_rd_ptr
            {
                n_valid_size_section1 = p_wr_ptr - p_ctx->mp_cache;
                n_valid_size_section2 = p_ctx->mp_cache + p_ctx->m_cache_size-p_rd_ptr;
                n_valid_size = n_valid_size_section1 + n_valid_size_section2;
                n_write_block_num = n_valid_size_section2/WRITE_BLOCK_SIZE;
                if (n_write_block_num > 0)
                {
                    ret = FileWriter(p_ctx->mp_writer, p_rd_ptr,
                        WRITE_BLOCK_SIZE * n_write_block_num);
                    if(ret < 0)
                    {
                        loge("FileWriter fail");
                        goto EXIT;
                    }

                    p_rd_ptr += WRITE_BLOCK_SIZE * n_write_block_num;
                    if (p_rd_ptr == p_ctx->mp_cache + p_ctx->m_cache_size)
                    {
                        p_rd_ptr = p_ctx->mp_cache;
                    }
                    else if (p_rd_ptr > p_ctx->mp_cache + p_ctx->m_cache_size)
                    {
                        loge("(f:%s, l:%d) fatal error! cache overflow![%p][%p][%d]",
                            __FUNCTION__, __LINE__, p_rd_ptr, p_ctx->mp_cache, p_ctx->m_cache_size);
                    }
                    p_ctx->m_read_ptr = p_rd_ptr;
                }
                else
                {
                    loge("(f:%s, l:%d) fatal error! Cache status has something wrong, \
                        need check[%p][%p][%p][%d][%d]",
                        __FUNCTION__, __LINE__, p_rd_ptr, p_wr_ptr, p_ctx->mp_cache,
                        p_ctx->m_cache_size, n_valid_size_section2);
                    ret = FileWriter(p_ctx->mp_writer, p_rd_ptr, n_valid_size_section2);
                    if(ret < 0)
                    {
                        loge("FileWriter fail");
                        goto EXIT;
                    }
                    p_rd_ptr = p_ctx->mp_cache;
                    p_ctx->m_read_ptr = p_rd_ptr;
                }
                CdxSemSignal(&p_ctx->m_write_done_sem);
            }
        }
        //(3) process m_flush_flag.
        cdx_mutex_lock(&p_ctx->m_flush_lock);
        if (p_ctx->m_flush_flag)
        {
            //flush data to fs
            p_rd_ptr = (cdx_uint8*)p_ctx->m_read_ptr;
            p_wr_ptr = (cdx_uint8*)p_ctx->m_write_ptr;
            if (p_wr_ptr >= p_rd_ptr)
            {
                n_valid_size = p_wr_ptr - p_rd_ptr;
                if (n_valid_size>0)
                {
                    ret = FileWriter(p_ctx->mp_writer, p_rd_ptr, n_valid_size);
                    if(ret < 0)
                    {
                        loge("FileWriter fail");
                        goto EXIT;
                    }
                    p_rd_ptr += n_valid_size;
                    p_ctx->m_read_ptr = p_rd_ptr;
                    CdxSemSignal(&p_ctx->m_write_done_sem);
                }
            }
            else    //p_wr_ptr < p_rd_ptr
            {
                n_valid_size_section1 = p_wr_ptr - p_ctx->mp_cache;
                n_valid_size_section2 = p_ctx->mp_cache + p_ctx->m_cache_size-p_rd_ptr;
                n_valid_size = n_valid_size_section1 + n_valid_size_section2;
                if (n_valid_size_section2 > 0)
                {
                    ret = FileWriter(p_ctx->mp_writer, p_rd_ptr, n_valid_size_section2);
                    if(ret < 0)
                    {
                        loge("FileWriter fail");
                        goto EXIT;
                    }
                    p_rd_ptr = p_ctx->mp_cache;
                    p_ctx->m_read_ptr = p_rd_ptr;
                    CdxSemSignal(&p_ctx->m_write_done_sem);
                }
                else
                {
                    loge("(f:%s, l:%d) fatal error! n_valid_size_section2==0!",
                        __FUNCTION__, __LINE__);
                }
                if (n_valid_size_section1 > 0)
                {
                    ret = FileWriter(p_ctx->mp_writer, p_rd_ptr, n_valid_size_section1);
                    if(ret < 0)
                    {
                        loge("FileWriter fail");
                        goto EXIT;
                    }
                    p_rd_ptr += n_valid_size_section1;
                    p_ctx->m_read_ptr = p_rd_ptr;
                    CdxSemSignal(&p_ctx->m_write_done_sem);
                }
            }
            if (p_ctx->m_read_ptr == p_ctx->m_write_ptr)
            {
                p_ctx->m_read_ptr = p_ctx->m_write_ptr = p_ctx->mp_cache;
            }
            else
            {
                loge("(f:%s, l:%d) fatal error! rdPtr[%p]!=wtPtr[%p]!",
                    __FUNCTION__, __LINE__, p_ctx->m_read_ptr, p_ctx->m_write_ptr);
            }
            CdxSemUp(&p_ctx->m_flush_done_sem);
            p_ctx->m_flush_flag = 0;
        }
        cdx_mutex_unlock(&p_ctx->m_flush_lock);

        //(4) process m_thread_exit_flag, same as flush.
        if (p_ctx->m_thread_exit_flag)
        {
            //flush data to fs
            p_rd_ptr = (cdx_uint8*)p_ctx->m_read_ptr;
            p_wr_ptr = (cdx_uint8*)p_ctx->m_write_ptr;
            if (p_wr_ptr >= p_rd_ptr)
            {
                n_valid_size = p_wr_ptr - p_rd_ptr;
                if (n_valid_size>0)
                {
                    ret = FileWriter(p_ctx->mp_writer, p_rd_ptr, n_valid_size);
                    if(ret < 0)
                    {
                        loge("FileWriter fail");
                        goto EXIT;
                    }
                    p_rd_ptr += n_valid_size;
                    p_ctx->m_read_ptr = p_rd_ptr;
                    CdxSemSignal(&p_ctx->m_write_done_sem);
                }
            }
            else    //p_wr_ptr < p_rd_ptr
            {
                n_valid_size_section1 = p_wr_ptr - p_ctx->mp_cache;
                n_valid_size_section2 = p_ctx->mp_cache + p_ctx->m_cache_size-p_rd_ptr;
                n_valid_size = n_valid_size_section1 + n_valid_size_section2;
                if (n_valid_size_section2 > 0)
                {
                    ret = FileWriter(p_ctx->mp_writer, p_rd_ptr, n_valid_size_section2);
                    if(ret < 0)
                    {
                        loge("FileWriter fail");
                        goto EXIT;
                    }
                    p_rd_ptr = p_ctx->mp_cache;
                    p_ctx->m_read_ptr = p_rd_ptr;
                    CdxSemSignal(&p_ctx->m_write_done_sem);
                }
                else
                {
                    loge("(f:%s, l:%d) fatal error! n_valid_size_section2==0!",
                        __FUNCTION__, __LINE__);
                }
                if (n_valid_size_section1 > 0)
                {
                    ret = FileWriter(p_ctx->mp_writer, p_rd_ptr, n_valid_size_section1);
                    if(ret < 0)
                    {
                        loge("FileWriter fail");
                        goto EXIT;
                    }
                    p_rd_ptr += n_valid_size_section1;
                    p_ctx->m_read_ptr = p_rd_ptr;
                    CdxSemSignal(&p_ctx->m_write_done_sem);
                }
            }
            goto EXIT;
        }
    }

EXIT:
    logv("(f:%s, l:%d) FsCacheWriteThread[%d] will quit now.",
        __FUNCTION__, __LINE__, (int)p_ctx->m_thread_id);
    p_ctx->m_write_error_flag = 1;
    CdxSemSignal(&p_ctx->m_write_done_sem);
    return NULL;
}

CdxFsWriter *InitFsCacheThreadContext(CdxWriterT *p_writer, cdx_uint8 *p_cache,
cdx_int32 n_cache_size, cdx_uint32 v_codec)
{
    int err;

    CdxFsWriter *p_fs_writer = (CdxFsWriter*)malloc(sizeof(CdxFsWriter));
    if (NULL == p_fs_writer)
    {
        loge("(f:%s, l:%d) Failed to alloc CdxFsWriter(%s)",
            __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }
    FsCacheThreadContext *p_context = (FsCacheThreadContext*)malloc(sizeof(FsCacheThreadContext));
    if (NULL == p_context)
    {
        loge("(f:%s, l:%d) Failed to alloc FsCacheThreadContext(%s)",
            __FUNCTION__, __LINE__, strerror(errno));
        goto ERROR0;
    }
    p_context->m_write_error_flag = 0;
    p_context->mp_writer = p_writer;
    p_context->m_cache_size = n_cache_size;
    p_context->mp_cache = p_cache;
    if (NULL == p_context->mp_cache)
    {
        logw("(f:%s, l:%d) fatal error! malloc [%d]kByte fail.",
            __FUNCTION__, __LINE__, n_cache_size / 1024);
        goto ERROR1;
    }
    p_context->m_thread_exit_flag = 0;
    p_context->m_write_ptr = p_context->m_read_ptr = p_context->mp_cache;
    p_context->m_flush_flag = 0;
    p_context->m_video_codec_id = v_codec;
    err = pthread_mutex_init(&p_context->m_flush_lock, NULL);
    if (err)
    {
        loge("(f:%s, l:%d) err[%d]", __FUNCTION__, __LINE__, err);
        goto ERROR2;
    }
    err = CdxSemInit(&p_context->m_write_start_sem,0);
    if (err)
    {
        loge("(f:%s, l:%d) err[%d]", __FUNCTION__, __LINE__, err);
        goto ERROR3;
    }
    err = CdxSemInit(&p_context->m_flush_done_sem,0);
    if (err)
    {
        loge("(f:%s, l:%d) err[%d]", __FUNCTION__, __LINE__, err);
        goto ERROR4;
    }
    err = CdxSemInit(&p_context->m_write_done_sem, 0);
    if (err)
    {
        loge("(f:%s, l:%d) err[%d]", __FUNCTION__, __LINE__, err);
        goto ERROR5;
    }

    // Create writer thread
    err = pthread_create(&p_context->m_thread_id, NULL, FsCacheWriteThread, p_context);
    if (err || !p_context->m_thread_id)
    {
        loge("FsCacheThread create writer thread err");
        goto ERROR6;
    }

    p_fs_writer->fsWrite = FsCacheThreadWrite;
    p_fs_writer->fsSeek = FsCacheThreadSeek;
    p_fs_writer->fsTell = FsCacheThreadTell;
    p_fs_writer->fsTruncate = NULL;
    p_fs_writer->fsFlush = FsCacheThreadFlush;
    p_fs_writer->m_mode = FSWRITEMODE_CACHETHREAD;
    p_fs_writer->m_priv = (void*)p_context;
    return p_fs_writer;

ERROR6:
    CdxSemDeinit(&p_context->m_write_done_sem);
ERROR5:
    CdxSemDeinit(&p_context->m_flush_done_sem);
ERROR4:
    CdxSemDeinit(&p_context->m_write_start_sem);
ERROR3:
    pthread_mutex_destroy(&p_context->m_flush_lock);
ERROR2:
ERROR1:
    free(p_context);
ERROR0:
    free(p_fs_writer);
    return NULL;
}

cdx_int32 DeinitFsCacheThreadContext(CdxFsWriter *p_fs_writer)
{
    int eError = 0;
    if (NULL == p_fs_writer)
    {
        loge("(f:%s, l:%d) p_fs_writer is NULL!!", __FUNCTION__, __LINE__);
        return -1;
    }
    p_fs_writer->fsFlush(p_fs_writer);
    FsCacheThreadContext *p_context = (FsCacheThreadContext*)p_fs_writer->m_priv;
    if (NULL == p_context)
    {
        loge("(f:%s, l:%d) p_context is NULL!!", __FUNCTION__, __LINE__);
        return -1;
    }
    p_context->m_thread_exit_flag = 1;
    CdxSemUpUnique(&p_context->m_write_start_sem);
    pthread_join(p_context->m_thread_id, (void*) &eError);
    CdxSemDeinit(&p_context->m_write_done_sem);
    CdxSemDeinit(&p_context->m_flush_done_sem);
    CdxSemDeinit(&p_context->m_write_start_sem);
    pthread_mutex_destroy(&p_context->m_flush_lock);
    p_context->mp_cache = NULL;
    p_context->m_thread_id = 0;
    free(p_context);
    p_fs_writer->m_priv = NULL;
    free(p_fs_writer);
    return 0;
}
