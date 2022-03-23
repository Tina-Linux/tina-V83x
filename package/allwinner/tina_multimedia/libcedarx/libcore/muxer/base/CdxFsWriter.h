/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxFsWriter.h
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

#ifndef __CDX__FS_WRITER_H__
#define __CDX_FS_WRITER_H__

#include <stdlib.h>
#include <stdint.h>
#include "CdxTypes.h"
#include "CdxMuxer.h"
#include "CdxWriter.h"
#include "CdxMuxerBaseDef.h"

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef CDX_PTR
typedef void* CDX_PTR;
#endif

typedef struct tag_FsWriter CdxFsWriter;
struct tag_FsWriter
{
    FSWRITEMODE m_mode;
    cdx_int32 (*fsWrite)(CdxFsWriter *thiz, const cdx_uint8 *buf, cdx_int32 size);
    cdx_int32 (*fsSeek)(CdxFsWriter *thiz, cdx_int64 noffset, cdx_int32 nwhere);
    cdx_int64 (*fsTell)(CdxFsWriter *thiz);
    cdx_int32 (*fsTruncate)(CdxFsWriter *thiz, cdx_int64 nlength);
    cdx_int32 (*fsFlush)(CdxFsWriter *thiz);
    void *m_priv;
};

typedef struct CdxFsWriterInfo
{
    CdxFsWriter        *mp_fs_writer;
    CdxFsCacheMemInfo  m_cache_mem_info;
    FSWRITEMODE        m_fs_writer_mode;
    cdx_int32          m_fs_simple_cache_size;
}CdxFsWriterInfo;


CdxFsWriter* CreateFsWriter(FSWRITEMODE mode, CdxWriterT *p_writer, cdx_uint8 *p_cache,
                            cdx_uint32 n_cache_size, cdx_uint32 v_codec);
cdx_int32 DestroyFsWriter(CdxFsWriter *thiz);
extern cdx_int32 FileWriter(CdxWriterT *p_writer, const cdx_uint8 *buffer, cdx_int32 size);
cdx_int64 FsGetNowUs();


#if defined(__cplusplus)
}
#endif

#endif
