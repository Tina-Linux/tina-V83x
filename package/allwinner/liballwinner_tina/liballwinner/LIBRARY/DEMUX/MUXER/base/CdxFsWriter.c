/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxFsWriter.c
 * Description : Allwinner Muxer Writer for SD Card
 * History :
 *   Author  : Q Wang <eric_wang@allwinnertech.com>
 *   Date    : 2014/12/17
 *   Comment : 创建初始版本，用于V3 CedarX_1.0
 *
 *   Author  : GJ Wu <wuguanjian@allwinnertech.com>
 *   Date    : 2016/04/18
 *   Comment_: 修改移植用于CedarX_2.0
 */

#define LOG_TAG "FsWriter"
#include "CdxFsWriter.h"
#include <sys/time.h>

extern CdxFsWriter *InitFsDirectWrite(CdxWriterT *p_writer);
extern CdxFsWriter *InitFsSimpleCache(CdxWriterT *p_writer, cdx_int32 n_cache_size);
extern CdxFsWriter *InitFsCacheThreadContext(CdxWriterT *p_writer, cdx_int8 *p_cache,
                                             cdx_int32 n_cache_size, cdx_uint32 v_codec);
extern cdx_int32 DeinitFsDirectWrite(CdxFsWriter *p_fs_writer);
extern cdx_int32 DeinitFsSimpleCache(CdxFsWriter *p_fs_writer);
extern cdx_int32 DeinitFsCacheThreadContext(CdxFsWriter *p_fs_writer);

cdx_int64 FsGetNowUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (cdx_int64)tv.tv_usec + (cdx_int64)tv.tv_sec * 1000000ll;
}

CdxFsWriter* CreateFsWriter(FSWRITEMODE mode, CdxWriterT *p_writer, cdx_int8 *p_cache,
                            cdx_uint32 n_cache_size, cdx_uint32 v_codec)
{
    if (FSWRITEMODE_CACHETHREAD == mode)
    {
        logd("fianlally FSWRITEMODE_CACHETHREAD == mode\n");
        return InitFsCacheThreadContext(p_writer, p_cache, n_cache_size, v_codec);
    }
    else if (FSWRITEMODE_SIMPLECACHE == mode)
    {
        logd("fianlally FSWRITEMODE_SIMPLECACHE == mode\n");
        return InitFsSimpleCache(p_writer, n_cache_size);
    }
    else if (FSWRITEMODE_DIRECT == mode)
    {
        logd("fianlally FSWRITEMODE_DIRECT == mode\n");
        return InitFsDirectWrite(p_writer);
    }
    else
    {
        loge("(f:%s, l:%d) not support mode[%d]", __FUNCTION__, __LINE__, mode);
        return NULL;
    }
}

cdx_int32 DestroyFsWriter(CdxFsWriter *thiz)
{
    if (NULL == thiz)
    {
        loge("(f:%s, l:%d) FsWriter is NULL", __FUNCTION__, __LINE__);
        return -1;
    }
    if (FSWRITEMODE_CACHETHREAD == thiz->m_mode)
    {
        return DeinitFsCacheThreadContext(thiz);
    }
    else if (FSWRITEMODE_SIMPLECACHE == thiz->m_mode)
    {
        return DeinitFsSimpleCache(thiz);
    }
    else if (FSWRITEMODE_DIRECT == thiz->m_mode)
    {
        return DeinitFsDirectWrite(thiz);
    }
    else
    {
        loge("(f:%s, l:%d) not support mode[%d]", __FUNCTION__, __LINE__, thiz->m_mode);
        return -1;
    }
    return 0;
}

#define SHOW_FWRITE_TIME
#define SHOW_TIME_THRESHOLD  (1000 * 1000ll)

cdx_int32 FileWriter(CdxWriterT *p_writer, const cdx_int8 *buffer, cdx_int32 size)
{
    cdx_int32 total_writen = 0;
#ifdef SHOW_FWRITE_TIME
    cdx_int64 tm1, tm2;
#endif


#ifdef SHOW_FWRITE_TIME
    tm1 = FsGetNowUs();
#endif
    if ((total_writen = p_writer->cdxWrite(p_writer, (void*)buffer, size)) < 0)
    {
        loge("(f:%s, l:%d) Stream[%p]fwrite error [%d]!=[%u](%s)",
            __FUNCTION__, __LINE__, p_writer, total_writen, size, strerror(errno));
        if (errno == EIO)
        {
            loge("(f:%s, l:%d) disk io error, stop write disk!!", __FUNCTION__, __LINE__);
        }
    }
#ifdef SHOW_FWRITE_TIME
    tm2 = FsGetNowUs();
    if (tm2-tm1 > SHOW_TIME_THRESHOLD)
    {
        logd("(f:%s, l:%d)write %d Byte, [%lld]ms",
            __FUNCTION__, __LINE__, total_writen, (tm2 - tm1) / 1000);
    }
#endif
    return total_writen;
}
