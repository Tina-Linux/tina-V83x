/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : xmetadataretriever.h
 * Description : xmetadataretriever
 * History :
 *
 */


#ifndef X_METADATA_RETRIEVER
#define X_METADATA_RETRIEVER

#ifdef __cplusplus
extern "C" {
#endif

#include "vdecoder.h"           //* video decode library in "libcedarc/include/"
#include "CdxParser.h"          //* parser library in "libcore/parser/include/"
#include "CdxStream.h"          //* parser library in "libcore/stream/include/"
#include "vencoder.h"           //* video encode library in "libcedarc/include/"

typedef struct XVideoFrame
{
    // Intentional public access modifier:
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mDisplayWidth;
    uint32_t mDisplayHeight;
    uint32_t mSize;            // Number of bytes in mData
    uint8_t* mData;            // Actual binary data
    int32_t  mRotationAngle;   // rotation angle, clockwise
}XVideoFrame;

typedef struct XVideoStream
{
    uint8_t* mBuf;
    int32_t  mSize;
}XVideoStream;

enum MetaDataType
{
    METADATA_VIDEO_WIDTH     = 0,
    METADATA_VIDEO_HEIGHT    = 1,
    METADATA_DURATION        = 2,  // file duration (ms)
    METADATA_MEDIAINFO       = 3,
    METADATA_PARSER_TYPE     = 4,
};

typedef void* XRetriever;

XRetriever* XRetrieverCreate();
int  XRetrieverDestory(XRetriever* v);
int   XRetrieverSetDataSource(XRetriever* v, const char* pUrl, const CdxKeyedVectorT* pHeaders);
XVideoFrame *XRetrieverGetFrameAtTime(XRetriever* v, int64_t timeUs, int option);
XVideoStream *XRetrieverGetStreamAtTime(XRetriever* v,int64_t timeUs);
int XRetrieverGetMetaData(XRetriever* v, int type, void* pVal);

#ifdef __cplusplus
}
#endif

#endif
