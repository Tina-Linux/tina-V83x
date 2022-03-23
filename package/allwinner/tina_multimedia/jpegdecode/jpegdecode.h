/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : jpegdecode.h
 * Description : jpegdecode
 * History :
 *
 */


#ifndef JPEG_DECODE
#define JPEG_DECODE

#ifdef __cplusplus
extern "C" {
#endif

#include "vdecoder.h"           //* video decode library in "libcedarc/include/"
#include "memoryAdapter.h"

typedef enum JpegDecodeScaleDownRatio{
    JPEG_DECODE_SCALE_DOWN_1      = 0, /*no scale down*/
    JPEG_DECODE_SCALE_DOWN_2      = 1, /*scale down 1/2*/
    JPEG_DECODE_SCALE_DOWN_4      = 2, /*scale down 1/4*/
    JPEG_DECODE_SCALE_DOWN_8      = 3, /*scale down 1/8*/
}JpegDecodeScaleDownRatio;
typedef enum JpegDecodeOutputDataType
{
    JpegDecodeOutputDataNV21 = 1,
    JpegDecodeOutputDataNV12 = 2,
    JpegDecodeOutputDataYU12 = 3,
    JpegDecodeOutputDataYV12 = 4,
    JpegDecodeOutputDataRGB565 = 5,
}JpegDecodeOutputDataType;

typedef struct ImgFrame
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
}ImgFrame;

typedef struct JpegDecoderContext
{
    VideoDecoder*               mVideoDecoder;
    ImgFrame                 mImgFrame;
    char*                     mUrl;
    char*                     mSrcBuf;
    int                          mSrcBufLen;
    int                          mScaleDownEn;
    int                          mHorizonScaleDownRatio;
    int                          mVerticalScaleDownRatio;
    int                          mDecodedPixFormat;
    int                          mOutputDataType;
    struct ScMemOpsS *memops;
}JpegDecoderContext;

typedef void* JpegDecoder;

JpegDecoder* JpegDecoderCreate();
void JpegDecoderDestory(JpegDecoder* v);
void JpegDecoderSetDataSource(JpegDecoder* v, const char* pUrl,JpegDecodeScaleDownRatio scaleRatio,JpegDecodeOutputDataType outputType);
void JpegDecoderSetDataSourceBuf(JpegDecoder* v, char* buffer,int bufLen,JpegDecodeScaleDownRatio scaleRatio,JpegDecodeOutputDataType outputType);
ImgFrame *JpegDecoderGetFrame(JpegDecoder* v);

#ifdef __cplusplus
}
#endif

#endif
