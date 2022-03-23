/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : tmetadataretriever.h
 * Description : tmetadataretriever
 * History :
 *
 */


#ifndef T_METADATA_RETRIEVER
#define T_METADATA_RETRIEVER


#ifdef __cplusplus
extern "C" {
#endif

#include "vdecoder.h"           //* video decode library in "libcedarc/include/"
#include "CdxParser.h"          //* parser library in "libcore/parser/include/"
#include "CdxStream.h"          //* parser library in "libcore/stream/include/"

typedef enum TmetadataRetrieverScaleDownRatio{
    TMETADATA_RETRIEVER_SCALE_DOWN_1      = 0, /*no scale down*/
    TMETADATA_RETRIEVER_SCALE_DOWN_2      = 1, /*scale down 1/2*/
    TMETADATA_RETRIEVER_SCALE_DOWN_4      = 2, /*scale down 1/4*/
    TMETADATA_RETRIEVER_SCALE_DOWN_8      = 3, /*scale down 1/8*/
}TmetadataRetrieverScaleDownRatio;

typedef enum TmetadataRetrieverOutputDataType
{
    TmetadataRetrieverOutputDataNV21 = 1,
    TmetadataRetrieverOutputDataYV12 = 2,
    TmetadataRetrieverOutputDataRGB565 = 3,
}TmetadataRetrieverOutputDataType;

typedef struct VideoFrame
{
    uint32_t mWidth;                    //the width which contains aligned buffer
    uint32_t mHeight;                  //the height which contains aligned buffer
    uint32_t mDisplayWidth;       //the actural frame width
    uint32_t mDisplayHeight;     //the actural frame height
    uint32_t mYuvSize;              // Number of bytes in mYuvData
    uint8_t* mYuvData;            // Actual yuv data
    uint32_t mRGB565Size;            // Number of bytes in mRGB565Data
    uint8_t* mRGB565Data;            // Actual RGB565 data
    int32_t  mRotationAngle;   // rotation angle, clockwise
}VideoFrame;

typedef enum TMetaDataType
{
    TMETADATA_VIDEO_WIDTH     = 0,
    TMETADATA_VIDEO_HEIGHT    = 1,
    TMETADATA_DURATION        = 2,  // file duration (ms)
    TMETADATA_MEDIAINFO       = 3,
    TMETADATA_PARSER_TYPE     = 4,
}TMetaDataType;

typedef void* TRetriever;

TRetriever* TRetrieverCreate();
int  TRetrieverDestory(TRetriever* v);
int   TRetrieverSetDataSource(TRetriever* v, const char* pUrl, TmetadataRetrieverScaleDownRatio scaleRatio,TmetadataRetrieverOutputDataType outputType);
VideoFrame *TRetrieverGetFrameAtTime(TRetriever* v, int64_t timeUs);
int TRetrieverGetMetaData(TRetriever* v, TMetaDataType type, void* pVal);

#ifdef __cplusplus
}
#endif

#endif
