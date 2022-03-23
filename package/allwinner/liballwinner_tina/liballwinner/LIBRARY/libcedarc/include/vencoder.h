/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : vencoder.h
* Description :
* History :
*   Author  : fangning <fangning@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/

/*
 *this software is based in part on the work
 * of the Independent JPEG Group
 */
#include <sc_interface.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _VENCODER_H_
#define _VENCODER_H_

#define  DATA_TIME_LENGTH             24
#define  INFO_LENGTH                 64
#define  GPS_PROCESS_METHOD_LENGTH     100
#define  DESCRIPTOR_INFO             128

#define VENC_BUFFERFLAG_KEYFRAME 0x00000001
#define VENC_BUFFERFLAG_EOS 0x00000002
#define H264_VERSION2_USE64 0

typedef struct rational_t {
    unsigned int num;
    unsigned int den;
}rational_t;

typedef struct srational_t {
    int num;
    int den;
}srational_t;

typedef enum ExifMeteringModeType {
    UNKNOWN_AW_EXIF,
    AVERAGE_AW_EXIF,
    CENTER_AW_EXIF,
    SPOT_AW_EXIF,
    MULTISPOT_AW_EXIF,
    PATTERN_AW_EXIF,
    PARTIAL_AW_EXIF,
    OTHER_AW_EXIF     = 255,
} ExifMeteringModeType;

typedef enum ExifExposureModeType {
    EXPOSURE_AUTO_AW_EXIF,
    EXPOSURE_MANUAL_AW_EXIF,
    EXPOSURE_AUTO_BRACKET_AW_EXIF,
}ExifExposureModeType;

typedef enum {
    UNKNOWN = 0,
    SUNLIGHT = 1,
    TUNGSTEN_LAMP = 2,
    FILAMENT_LAMP = 3,
    FLASH_LAMP = 4,
    OVERCAST = 9,
    CLOUDY = 10,
    SHADOW = 11,
    INCANDESCENT_LAMP = 12,
    WHITE_DAY_FLUORESCENT_LAMP = 13,
    COOL_COLOUR_FLUORESCENT_LAMP = 14,
    WHITE_FLUORESCENT_LAMP = 15,
    STANDARD_LAMP_A = 17,
    STANDARD_LAMP_B = 18,
    STANDARD_lAMP_C = 19,
    D55 = 20,
    D65 = 21,
    D75 = 22,
    D50= 23,
    PROJECTION_ROOM_LAMP = 24,
    OTHERS = 255
}ExifLightSource;

typedef struct EXIFInfo {
    unsigned char  CameraMake[INFO_LENGTH];
    unsigned char  CameraModel[INFO_LENGTH];
    unsigned char  DateTime[DATA_TIME_LENGTH];

    unsigned int   ThumbWidth;
    unsigned int   ThumbHeight;
    unsigned char* ThumbAddrVir;
    unsigned int   ThumbLen;

    int              Orientation;  //value can be 0,90,180,270 degree
    rational_t       ExposureTime; //tag 0x829A
    rational_t       FNumber; //tag 0x829D
    short           ISOSpeed;//tag 0x8827

    srational_t    ShutterSpeedValue; //tag 0x9201
    //srational_t    BrightnessValue;   //tag 0x9203
    srational_t    ExposureBiasValue; //tag 0x9204

    short           MeteringMode; //tag 0x9207
    short           LightSource; //tag 0x9208
    short           FlashUsed;     //tag 0x9209
    rational_t       FocalLength; //tag 0x920A

    rational_t       DigitalZoomRatio; // tag 0xA404
    short           WhiteBalance; //tag 0xA403
    short           ExposureMode; //tag 0xA402

    // gps info
    int            enableGpsInfo;
    double         gps_latitude;
    double           gps_longitude;
    double         gps_altitude;
    long           gps_timestamp;
    unsigned char  gpsProcessingMethod[GPS_PROCESS_METHOD_LENGTH];

    unsigned char  CameraSerialNum[128];     //tag 0xA431 (exif 2.3 version)
    short              FocalLengthIn35mmFilm;     // tag 0xA405

    unsigned char  ImageName[128];             //tag 0x010D
    unsigned char  ImageDescription[128];     //tag 0x010E
    short            ImageWidth;                 //tag 0xA002
    short            ImageHeight;             //tag 0xA003
}EXIFInfo;

typedef struct VencRect {
    int nLeft;
    int nTop;
    int nWidth;
    int nHeight;
}VencRect;

typedef enum VENC_YUV2YUV {
    VENC_YCCToBT601,
    VENC_BT601ToYCC,
}VENC_YUV2YUV;


typedef enum VENC_CODING_MODE {
  VENC_FRAME_CODING         = 0,
  VENC_FIELD_CODING         = 1,
}VENC_CODING_MODE;

//* The Amount of Temporal SVC Layers
typedef enum {
    NO_T_SVC = 0,
    T_LAYER_2 = 2,
    T_LAYER_3 = 3,
    T_LAYER_4 = 4
}T_LAYER;

//* The Multiple of Skip_Frame
typedef enum {
    NO_SKIP = 0,
    SKIP_2 = 2,
    SKIP_4 = 4,
    SKIP_8 = 8
}SKIP_FRAME;

typedef enum VENC_CODEC_TYPE {
    VENC_CODEC_H264,
    VENC_CODEC_JPEG,
    VENC_CODEC_H264_VER2,
    VENC_CODEC_VP8,
}VENC_CODEC_TYPE;


typedef enum VENC_PIXEL_FMT {
    VENC_PIXEL_YUV420SP,
    VENC_PIXEL_YVU420SP,
    VENC_PIXEL_YUV420P,
    VENC_PIXEL_YVU420P,
    VENC_PIXEL_YUV422SP,
    VENC_PIXEL_YVU422SP,
    VENC_PIXEL_YUV422P,
    VENC_PIXEL_YVU422P,
    VENC_PIXEL_YUYV422,
    VENC_PIXEL_UYVY422,
    VENC_PIXEL_YVYU422,
    VENC_PIXEL_VYUY422,
    VENC_PIXEL_ARGB,
    VENC_PIXEL_RGBA,
    VENC_PIXEL_ABGR,
    VENC_PIXEL_BGRA,
    VENC_PIXEL_TILE_32X32,
    VENC_PIXEL_TILE_128X32,
}VENC_PIXEL_FMT;


typedef struct VencBaseConfig {
    unsigned int        nInputWidth;
    unsigned int        nInputHeight;
    unsigned int        nDstWidth;
    unsigned int        nDstHeight;
    unsigned int        nStride;
    VENC_PIXEL_FMT      eInputFormat;
    struct ScMemOpsS *memops;
}VencBaseConfig;


/**
 * H264 profile types
 */
typedef enum VENC_H264PROFILETYPE {
    VENC_H264ProfileBaseline  = 66,         /**< Baseline profile */
    VENC_H264ProfileMain      = 77,         /**< Main profile */
    VENC_H264ProfileHigh      = 100,           /**< High profile */
}VENC_H264PROFILETYPE;


/**
 * H264 level types
 */
typedef enum VENC_H264LEVELTYPE {
    VENC_H264Level1   = 10,     /**< Level 1 */
    VENC_H264Level11  = 11,     /**< Level 1.1 */
    VENC_H264Level12  = 12,     /**< Level 1.2 */
    VENC_H264Level13  = 13,     /**< Level 1.3 */
    VENC_H264Level2   = 20,     /**< Level 2 */
    VENC_H264Level21  = 21,     /**< Level 2.1 */
    VENC_H264Level22  = 22,     /**< Level 2.2 */
    VENC_H264Level3   = 30,     /**< Level 3 */
    VENC_H264Level31  = 31,     /**< Level 3.1 */
    VENC_H264Level32  = 32,     /**< Level 3.2 */
    VENC_H264Level4   = 40,     /**< Level 4 */
    VENC_H264Level41  = 41,     /**< Level 4.1 */
    VENC_H264Level42  = 42,     /**< Level 4.2 */
    VENC_H264Level5   = 50,     /**< Level 5 */
    VENC_H264Level51  = 51,     /**< Level 5.1 */
}VENC_H264LEVELTYPE;

typedef struct VencH264ProfileLevel {
    VENC_H264PROFILETYPE    nProfile;
    VENC_H264LEVELTYPE        nLevel;
}VencH264ProfileLevel;

typedef struct VencQPRange {
    int    nMaxqp;
    int    nMinqp;
}VencQPRange;

typedef struct MotionParam {
    int nMotionDetectEnable;
    int nMotionDetectRatio; /* 0~12, advise set 0 */
    int nStaticDetectRatio; /* 0~12, should be larger than  nMotionDetectRatio, advise set 2 */
    int nMaxNumStaticFrame; /* advise set 4 */
    double nStaticBitsRatio; /* advise set 0.2~0.3 at daytime, set 0.1 at night */
    double nMV64x64Ratio; /* advise set 0.01 */
    short nMVXTh; /* advise set 6 */
    short nMVYTh; /* advise set 6 */
}MotionParam;

typedef struct VencHeaderData {
    unsigned char*  pBuffer;
    unsigned int    nLength;
}VencHeaderData;

/* support 4 ROI region */
typedef struct VencROIConfig {
    int                     bEnable;
    int                        index; /* (0~3) */
    int                     nQPoffset;
    VencRect                sRect;
}VencROIConfig;

typedef struct VencInputBuffer {
    unsigned long  nID;
    long long         nPts;
    unsigned int   nFlag;
    unsigned char* pAddrPhyY;
    unsigned char* pAddrPhyC;
    unsigned char* pAddrVirY;
    unsigned char* pAddrVirC;
    int            bEnableCorp;
    VencRect       sCropInfo;

    int               ispPicVar;
    VencROIConfig  roi_param[4];
}VencInputBuffer;

typedef struct FrameInfo {
    int             CurrQp;
    int             avQp;
    int             nGopIndex;
    int             nFrameIndex;
    int             nTotalIndex;
}FrameInfo;

typedef struct VencOutputBuffer {
    int               nID;
    long long         nPts;
    unsigned int   nFlag;
    unsigned int   nSize0;
    unsigned int   nSize1;
    unsigned char* pData0;
    unsigned char* pData1;

    FrameInfo       frame_info;
}VencOutputBuffer;

typedef struct VencAllocateBufferParam {
    unsigned int   nBufferNum;
    unsigned int   nSizeY;
    unsigned int   nSizeC;
}VencAllocateBufferParam;


typedef struct VencH264FixQP {
    int                     bEnable;
    int                     nIQp;
    int                     nPQp;
}VencH264FixQP;

#define EXTENDED_SAR 255
typedef struct VencH264AspectRatio {
    unsigned char aspect_ratio_idc;
    unsigned short  sar_width;
    unsigned short  sar_height;
}VencH264AspectRatio;

typedef enum VENC_COLOR_SPACE {
    RESERVED0    = 0,
    VENC_BT709  = 1,                 /* bt709 */
    RESERVED1    = 2,
    RESERVED2    = 3,
    RESERVED3    = 4,
    VENC_BT601  = 5,                 /* bt601-625 default use this colorspace */
    BT601_525   = 6,                 /* bt601-525 */
    RESERVED4    = 7,
    VENC_YCC    = 8,                /* YCC: full range BT.601  */
}VENC_COLOR_SPACE;

typedef enum VENC_VIDEO_FORMAT {
    COMPONENT       = 0,                 /* component */
    PAL               = 1,                 /* pal*/
    NTSC            = 2,                 /* ntsc */
    SECAM            = 3,                /* secam  */
    MAC                = 4,                /* mac  */
    DEFAULT            = 5,                /* Unspecified video format  */
}VENC_VIDEO_FORMAT;

typedef struct VencJpegVideoSignal {
    VENC_COLOR_SPACE src_colour_primaries;
    VENC_COLOR_SPACE dst_colour_primaries;
}VencJpegVideoSignal;

typedef struct VencH264VideoSignal {
    VENC_VIDEO_FORMAT video_format;

    unsigned char full_range_flag;

    VENC_COLOR_SPACE src_colour_primaries;
    VENC_COLOR_SPACE dst_colour_primaries;
}VencH264VideoSignal;

// Add for setting SVC and Skip_Frame
typedef struct VencH264SVCSkip {
    T_LAYER        nTemporalSVC;
    SKIP_FRAME     nSkipFrame;
    int            bEnableLayerRatio;
    unsigned int   nLayerRatio[4];
}VencH264SVCSkip;


typedef struct VencCyclicIntraRefresh {
    int                     bEnable;
    int                     nBlockNumber;
}VencCyclicIntraRefresh;


typedef struct VencSize {
    int                     nWidth;
    int                     nHeight;
}VencSize;


typedef struct VencH264Param {
    VencH264ProfileLevel    sProfileLevel;
    int                     bEntropyCodingCABAC; /* 0:CAVLC 1:CABAC*/
    VencQPRange               sQPRange;
    int                     nFramerate; /* fps*/
    int                     nBitrate;   /* bps*/
    int                     nMaxKeyInterval;
    VENC_CODING_MODE        nCodingMode;
}VencH264Param;

typedef struct VencCheckColorFormat {
    int                        index;
    VENC_PIXEL_FMT          eColorFormat;
}VencCheckColorFormat;


typedef struct VencVP8Param {
    int                     nFramerate; /* fps*/
    int                     nBitrate;   /* bps*/
    int                     nMaxKeyInterval;
}VencVP8Param;


typedef enum VENC_SUPERFRAME_MODE {
    VENC_SUPERFRAME_NONE,
    VENC_SUPERFRAME_DISCARD,
    VENC_SUPERFRAME_REENCODE,
}VENC_SUPERFRAME_MODE;


typedef struct VencSuperFrameConfig {
    VENC_SUPERFRAME_MODE    eSuperFrameMode;
    unsigned int            nMaxIFrameBits;
    unsigned int            nMaxPFrameBits;
}VencSuperFrameConfig;

typedef struct VencBitRateRange {
    int            bitRateMax;
    int            bitRateMin;
}VencBitRateRange;

typedef enum VENC_INDEXTYPE {
    VENC_IndexParamBitrate                = 0x0,
    /**< reference type: int */
    VENC_IndexParamFramerate,
    /**< reference type: int */
    VENC_IndexParamMaxKeyInterval,
    /**< reference type: int */
    VENC_IndexParamIfilter,
    /**< reference type: int */
    VENC_IndexParamRotation,
    /**< reference type: int */
    VENC_IndexParamSliceHeight,
    /**< reference type: int */
    VENC_IndexParamForceKeyFrame,
    /**< reference type: int (write only)*/
    VENC_IndexParamMotionDetectEnable,
    /**< reference type: MotionParam(write only) */
    VENC_IndexParamMotionDetectStatus,
    /**< reference type: int(read only) */
    VENC_IndexParamRgb2Yuv,
    /**< reference type: VENC_COLOR_SPACE */
    VENC_IndexParamYuv2Yuv,
    /**< reference type: VENC_YUV2YUV */
    VENC_IndexParamROIConfig,
    /**< reference type: VencROIConfig */
    VENC_IndexParamStride,
    /**< reference type: int */
    VENC_IndexParamColorFormat,
    /**< reference type: VENC_PIXEL_FMT */
    VENC_IndexParamSize,
    /**< reference type: VencSize(read only) */
    VENC_IndexParamSetVbvSize,
    /**< reference type: setVbvSize(write only) */
    VENC_IndexParamVbvInfo,
    /**< reference type: getVbvInfo(read only) */
    VENC_IndexParamSuperFrameConfig,
    /**< reference type: VencSuperFrameConfig */
    VENC_IndexParamSetPSkip,
    /**< reference type: unsigned int */
    VENC_IndexParamResetEnc,
    /**< reference type: */

    /* check capabiliy */
    VENC_IndexParamMAXSupportSize,
    /**< reference type: VencSize(read only) */
    VENC_IndexParamCheckColorFormat,
    /**< reference type: VencCheckFormat(read only) */

    /* H264 param */
    VENC_IndexParamH264Param,
    /**< reference type: VencH264Param */
    VENC_IndexParamH264SPSPPS,
    /**< reference type: VencHeaderData (read only)*/
    VENC_IndexParamH264QPRange            = 0x100,
    /**< reference type: VencQPRange */
    VENC_IndexParamH264ProfileLevel,
    /**< reference type: VencProfileLevel */
    VENC_IndexParamH264EntropyCodingCABAC,
    /**< reference type: int(0:CAVLC 1:CABAC) */
    VENC_IndexParamH264CyclicIntraRefresh,
    /**< reference type: VencCyclicIntraRefresh */
    VENC_IndexParamH264FixQP,
    /**< reference type: VencH264FixQP */
    VENC_IndexParamH264SVCSkip,
    /**< reference type: VencH264SVCSkip */
    VENC_IndexParamH264AspectRatio,
    /**< reference type: VencH264AspectRatio */
    VENC_IndexParamH264FastEnc,
    /**< reference type: int */
    VENC_IndexParamH264VideoSignal,
    /**< reference type: VencH264VideoSignal */
    VENC_IndexParamH264IQpOffset,
    /**< reference type: int */
    /* jpeg param */
    VENC_IndexParamJpegQuality            = 0x200,
    /**< reference type: int (1~100) */
    VENC_IndexParamJpegExifInfo,
    /**< reference type: EXIFInfo */
    VENC_IndexParamJpegEncMode,
    /**< reference type: 0:jpeg; 1:motion_jepg */
    VENC_IndexParamJpegVideoSignal,
    /**< reference type: VencJpegVideoSignal */

    /* VP8 param */
    VENC_IndexParamVP8Param,
    /* max one frame length */
    VENC_IndexParamSetFrameLenThreshold,
    /**< reference type: int */
    /* decrease the a20 dram bands */
    VENC_IndexParamSetA20LowBands,
    /**< reference type: 0:disable; 1:enable */
    VENC_IndexParamSetBitRateRange,
    /**< reference type: VencBitRateRange */
}VENC_INDEXTYPE;

typedef enum VENC_RESULT_TYPE {
    VENC_RESULT_ERROR             = -1,
    VENC_RESULT_OK                   = 0,
    VENC_RESULT_NO_FRAME_BUFFER   = 1,
    VENC_RESULT_BITSTREAM_IS_FULL = 2,
    VENC_RESULT_ILLEGAL_PARAM     = 3,
    VENC_RESULT_NOT_SUPPORT     = 4,
    VENC_RESULT_BITSTREAM_IS_EMPTY = 5,
    VENC_RESULT_NO_MEMORY     = 6,
}VENC_RESULT_TYPE;

typedef struct JpegEncInfo {
    VencBaseConfig  sBaseInfo;
    int             bNoUseAddrPhy;
    unsigned char*  pAddrPhyY;
    unsigned char*  pAddrPhyC;
    unsigned char*  pAddrVirY;
    unsigned char*  pAddrVirC;
    int             bEnableCorp;
    VencRect        sCropInfo;
    int                quality;
}JpegEncInfo;

typedef struct VbvInfo {
    unsigned int vbv_size;
    unsigned int coded_frame_num;
    unsigned int coded_size;
    unsigned int maxFrameLen;
}VbvInfo;


int AWJpecEnc(JpegEncInfo* pJpegInfo, EXIFInfo* pExifInfo, void* pOutBuffer, int* pOutBufferSize);


typedef void* VideoEncoder;

VideoEncoder* VideoEncCreate(VENC_CODEC_TYPE eCodecType);
void VideoEncDestroy(VideoEncoder* pEncoder);
int VideoEncInit(VideoEncoder* pEncoder, VencBaseConfig* pConfig);
int VideoEncUnInit(VideoEncoder* pEncoder);

int AllocInputBuffer(VideoEncoder* pEncoder, VencAllocateBufferParam *pBufferParam);
int GetOneAllocInputBuffer(VideoEncoder* pEncoder, VencInputBuffer* pInputbuffer);
int FlushCacheAllocInputBuffer(VideoEncoder* pEncoder,  VencInputBuffer *pInputbuffer);
int ReturnOneAllocInputBuffer(VideoEncoder* pEncoder,  VencInputBuffer *pInputbuffer);
int ReleaseAllocInputBuffer(VideoEncoder* pEncoder);

int AddOneInputBuffer(VideoEncoder* pEncoder, VencInputBuffer* pInputbuffer);
int VideoEncodeOneFrame(VideoEncoder* pEncoder);
int AlreadyUsedInputBuffer(VideoEncoder* pEncoder, VencInputBuffer* pBuffer);

int ValidBitstreamFrameNum(VideoEncoder* pEncoder);
int GetOneBitstreamFrame(VideoEncoder* pEncoder, VencOutputBuffer* pBuffer);
int FreeOneBitStreamFrame(VideoEncoder* pEncoder, VencOutputBuffer* pBuffer);

int VideoEncGetParameter(VideoEncoder* pEncoder, VENC_INDEXTYPE indexType, void* paramData);
int VideoEncSetParameter(VideoEncoder* pEncoder, VENC_INDEXTYPE indexType, void* paramData);

int VideoEncoderReset(VideoEncoder* pEncoder);
unsigned int VideoEncoderGetUnencodedBufferNum(VideoEncoder* pEncoder);

#endif    //_VENCODER_H_

#ifdef __cplusplus
}
#endif /* __cplusplus */
