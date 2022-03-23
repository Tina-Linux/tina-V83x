/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : AWOMX_VideoIndexExtension.h
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : complete the design code
*/

#ifndef _AWOMX_VIDEO_INDEX_EXTENSION_H_
#define _AWOMX_VIDEO_INDEX_EXTENSION_H_

/*========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <OMX_Core.h>

/*========================================================================

                      DEFINITIONS AND DECLARATIONS

========================================================================== */

#if defined( __cplusplus )
extern "C"
{
#endif /* end of macro __cplusplus */

#define OMX_VIDEO_PARAMS_EXTENDED_FLAGS_SCALING 1;
#define OMX_VIDEO_PARAMS_EXTENDED_FLAGS_CROPPING 2;

//* make  the struct standard after android5.1
#ifdef CONF_LOLLIPOP_AND_NEWER
typedef struct OMX_VIDEO_PARAMS_EXTENDED {
     OMX_U32           nSize;
     OMX_VERSIONTYPE   nVersion;
     OMX_U32           ui32Flags;       // To check bEnableScaling and bEnableCropping enable or not
     OMX_BOOL          bEnableScaling;  // Resolution Scaling
     OMX_U16           ui16ScaledWidth; // pixel of width after scale
     OMX_U16           ui16ScaledHeight;// pixel of height after scale
     OMX_BOOL          bEnableCropping; // Resolution Cropping
     OMX_U16           ui16CropLeft;    // Pixel of columns to be cropped from lefthand-side edge
     OMX_U16           ui16CropRight;   // Pixel of columns to be cropped from righthand-side edge
     OMX_U16           ui16CropTop;     // Pixel of rows to be cropped from the top edge
     OMX_U16           ui16CropBottom;  // Pixel of rows to be cropped from the bottom edge
} OMX_VIDEO_PARAMS_EXTENDED;
#else
typedef struct _OMX_VIDEO_PARAMS_EXTENDED {
     OMX_U32         ui32Flags;
     OMX_BOOL        bEnableScaling; // Resolution Scaling
     OMX_U16         ui16ScaledWidth;
     OMX_U16         ui16ScaledHeight;
     OMX_BOOL        bEnableCropping; // Resolution Cropping
     OMX_U16         ui16CropLeft;//Number of columns to be cropped from lefthand-side edge
     OMX_U16         ui16CropRight;//Number of columns to be cropped from righthand-side edge
     OMX_U16         ui16CropTop;//Number of rows to be cropped from the top edge
     OMX_U16         ui16CropBottom;// Number of rows to be cropped from the bottom edge
} OMX_VIDEO_PARAMS_EXTENDED;
#endif


typedef enum _OMX_VIDEO_SUPERFRAME_MODE {
    OMX_VIDEO_SUPERFRAME_NONE,
    OMX_VIDEO_SUPERFRAME_DISCARD,
    OMX_VIDEO_SUPERFRAME_REENCODE,
} OMX_VIDEO_SUPERFRAME_MODE;

typedef struct _OMX_VIDEO_SUPER_FRAME {
     OMX_BOOL                   bEnable;
     OMX_VIDEO_SUPERFRAME_MODE  eSuperFrameMode;
     OMX_U32                    nMaxIFrameBits;
     OMX_U32                    nMaxPFrameBits;
} OMX_VIDEO_PARAMS_SUPER_FRAME;

typedef struct _OMX_VIDEO_MD {
    OMX_BOOL   bMotionDetectEnable;
    OMX_S32    nMotionDetectRatio;/* 0~12, advise set 0 */
    OMX_S32    nStaticDetectRatio;/* 0~12, should be larger than nMotionDetectRatio,advise set 2 */
    OMX_S32    nMaxNumStaticFrame;/* advise set 4 */
    double     nStaticBitsRatio; /* advise set 0.2~0.3 at daytime, set 0.1 at night */
    double     dMV64x64Ratio; /* advise set 0.01 */
    OMX_S16    sMVXTh; /* advise set 6 */
    OMX_S16    sMVYTh; /* advise set 6 */
} OMX_VIDEO_PARAMS_MD;

typedef struct _OMX_VIDEO_VBR {
     OMX_BOOL        bEnable;
     OMX_S32         uQpMin;
     OMX_S32         uQpMax;
     OMX_S32         uBitRate;
     OMX_VIDEO_PARAMS_MD sMdParam;
} OMX_VIDEO_PARAMS_VBR;

//* The Amount of Temporal SVC Layers
typedef enum {
    OMX_VIDEO_NO_SVC = 0,
    OMX_VIDEO_LAYER_2 = 2,
    OMX_VIDEO_LAYER_3 = 3,
    OMX_VIDEO_LAYER_4 = 4
} OMX_VIDEO_LAYER;

typedef struct _OMX_VIDEO_SVC {
     OMX_BOOL               bEnable;
     OMX_VIDEO_LAYER        eSvcLayer;
     OMX_U32                uLayerRatio[4];
} OMX_VIDEO_PARAMS_SVC;


typedef struct VIDDEC_CUSTOM_PARAM
{
    unsigned char cCustomParamName[128];
    OMX_INDEXTYPE nCustomParamIndex;
} VIDDEC_CUSTOM_PARAM;


typedef struct VIDEO_PROFILE_LEVEL
{
    OMX_S32  nProfile;
    OMX_S32  nLevel;
} VIDEO_PROFILE_LEVEL_TYPE;

typedef struct _OMX_VIDEO_AVC_NALU_FORMAT {
     OMX_BOOL                   bEnable;
} OMX_VIDEO_PARAMS_AVC_NALU_FORMAT;


/**
 * Enumeration used to define Allwinner's vendor extensions for
 * video. The video extensions occupy a range of
 * 0x7F100000-0x7F1FFFFF, inclusive.
 */

typedef enum AW_VIDEO_EXTENSIONS_INDEXTYPE
{
    /* OMX.google.android.index.enableAndroidNativeBuffers */
    AWOMX_IndexParamVideoEnableAndroidNativeBuffers    = 0x7F100000,
    /* OMX.google.android.index.getAndroidNativeBufferUsage */
    AWOMX_IndexParamVideoGetAndroidNativeBufferUsage   = 0x7F100001,
    /* OMX.google.android.index.useAndroidNativeBuffer2 */
    AWOMX_IndexParamVideoUseAndroidNativeBuffer2       = 0x7F100002,
    AWOMX_IndexParamVideoUseStoreMetaDataInBuffer      = 0x7F100003,
    AWOMX_IndexParamVideoUsePrepareForAdaptivePlayback = 0x7F100004,
    AWOMX_IndexParamVideoGetAfbcHdrFlag                = 0x7F100005,
    AWOMX_IndexParamVideoAllocateNativeHandle          = 0x7F100006,
    AWOMX_IndexParamVideoDescribeColorAspects          = 0x7F100007,
    AWOMX_IndexParamVideoDescribeHDRStaticInfo         = 0x7F100008,
    AWOMX_IndexParamVideoUnused                        = 0x7F2FFFFF
} AW_VIDEO_EXTENSIONS_INDEXTYPE;

#define VIDDEC_CUSTOMPARAM_ENABLE_ANDROID_NATIVE_BUFFER  \
        "OMX.google.android.index.enableAndroidNativeBuffers"
#define VIDDEC_CUSTOMPARAM_GET_ANDROID_NATIVE_BUFFER_USAGE \
        "OMX.google.android.index.getAndroidNativeBufferUsage"
#define VIDDEC_CUSTOMPARAM_USE_ANDROID_NATIVE_BUFFER2 \
        "OMX.google.android.index.useAndroidNativeBuffer2"
#define VIDDEC_CUSTOMPARAM_STORE_META_DATA_IN_BUFFER  \
        "OMX.google.android.index.storeMetaDataInBuffers"
#define VIDDEC_CUSTOMPARAM_PREPARE_FOR_ADAPTIVE_PLAYBACK  \
        "OMX.google.android.index.prepareForAdaptivePlayback"
#define VIDDEC_CUSTOMPARAM_ALLOCATE_NATIVE_HANDLE \
        "OMX.google.android.index.allocateNativeHandle"
#define VIDDEC_CUSTOMPARAM_GET_AFBC_HDR_FLAG  \
        "OMX.google.android.index.getAfbcHdrFlag"
#define VIDDEC_CUSTOMPARAM_DESCRIBE_COLORASPECTS \
        "OMX.google.android.index.describeColorAspects"
#define VIDDEC_CUSTOMPARAM_DESCRIBE_HDR_STATIC_INFO \
        "OMX.google.android.index.describeHDRStaticInfo"

#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* end of macro _AWOMX_VIDEO_INDEX_EXTENSION_H_ */
