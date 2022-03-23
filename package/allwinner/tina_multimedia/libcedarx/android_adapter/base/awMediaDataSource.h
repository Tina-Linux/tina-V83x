/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : awMediaDataSource.h
* Description :
* History :
*   Author  : SW5
*   Date    : 2017/04/12
*   Comment : first version
*
*/


#ifndef AW_MEDIA_DATA_SOURCE_H
#define AW_MEDIA_DATA_SOURCE_H

#include <media/stagefright/DataSource.h>
#include <CdxStream.h>

using namespace android;

CdxStreamT *MediaDataSourceOpen(const sp<DataSource> &source);
cdx_int32 MediaDataSourceClose(CdxStreamT *stream);

#endif  // AW_MEDIA_DATA_SOURCE_H
