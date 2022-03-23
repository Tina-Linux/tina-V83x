/******************************************************************************
  Copyright (C), 2001-2016, Allwinner Tech. Co., Ltd.
 ******************************************************************************
  File Name     : media_common.c
  Version       : Initial Draft
  Author        : Allwinner BU3-PD2 Team
  Created       : 2016/03/15
  Last Modified :
  Description   : multimedia common function for internal use.
  Function List :
  History       :
******************************************************************************/
// ref platform headers
#include <linux/videodev2.h>
#include <string.h>
#include <utils/plat_log.h>
#include "cdx_list.h"
#include "plat_defines.h"
#include "plat_errno.h"
#include "plat_math.h"
#include "plat_type.h"

// media api headers to app
#include "mm_comm_demux.h"
#include "mm_comm_venc.h"
#include "mm_comm_aenc.h"
#include "mm_comm_adec.h"
#include "mm_comm_video.h"
#include "mm_comm_uvc.h"
#include "mm_common.h"

// media internal common headers.
#include "media_common.h"
#include "mm_component.h"
#include "vencoder.h"

ERRORTYPE copy_MPP_CHN_S(MPP_CHN_S *pDst, MPP_CHN_S *pSrc)
{
    *pDst = *pSrc;
    return SUCCESS;
}

ERRORTYPE copy_ADEC_CHN_ATTR_S(ADEC_CHN_ATTR_S *pDst, ADEC_CHN_ATTR_S *pSrc)
{
    *pDst = *pSrc;
    return SUCCESS;
}

ERRORTYPE copy_VENC_CHN_ATTR_S(VENC_CHN_ATTR_S *pDst, VENC_CHN_ATTR_S *pSrc)
{
    *pDst = *pSrc;
    return SUCCESS;
}

ERRORTYPE copy_ISE_GRP_ATTR_S(ISE_GROUP_ATTR_S *pDst, ISE_GROUP_ATTR_S *pSrc)
{
    *pDst = *pSrc;
    return SUCCESS;
}

ERRORTYPE copy_ISE_CHN_ATTR_S(ISE_CHN_ATTR_S *pDst, ISE_CHN_ATTR_S *pSrc)
{
    *pDst = *pSrc;
    return SUCCESS;
}

ERRORTYPE copy_UVC_CHN_ATTR_S(UVC_CHN_ATTR_S *pDst, UVC_CHN_ATTR_S *pSrc)
{
    *pDst = *pSrc;
    return SUCCESS;
}

VENC_CODEC_TYPE map_PAYLOAD_TYPE_E_to_VENC_CODEC_TYPE(PAYLOAD_TYPE_E nPayLoadType)
{
    VENC_CODEC_TYPE nVenclibType;
    switch (nPayLoadType) {
        case PT_H264:
            nVenclibType = VENC_CODEC_H264;
            break;
        case PT_H265:
            nVenclibType = VENC_CODEC_H265;
            break;
        case PT_JPEG:
        case PT_MJPEG:
            nVenclibType = VENC_CODEC_JPEG;
            break;
        default:
            aloge("fatal error! unknown PlayLoadType[%d]", nPayLoadType);
            nVenclibType = VENC_CODEC_H264;
            break;
    }
    return nVenclibType;
}

AUDIO_ENCODER_TYPE map_PAYLOAD_TYPE_E_to_AUDIO_ENCODER_TYPE(PAYLOAD_TYPE_E nPayLoadType)
{
    AUDIO_ENCODER_TYPE nAenclibType;
    switch (nPayLoadType) {
        case PT_AAC:
            nAenclibType = AUDIO_ENCODER_AAC_TYPE;
            break;
        case PT_LPCM:
            nAenclibType = AUDIO_ENCODER_LPCM_TYPE;
            break;
        case PT_PCM_VOICE:
        case PT_PCM_AUDIO:
            nAenclibType = AUDIO_ENCODER_PCM_TYPE;
            break;
        case PT_MP3:
            nAenclibType = AUDIO_ENCODER_MP3_TYPE;
            break;
        default:
            aloge("fatal error! unknown PlayLoadType[%d]", nPayLoadType);
            nAenclibType = AUDIO_ENCODER_AAC_TYPE;
            break;
    }
    return nAenclibType;
}

enum EVIDEOCODECFORMAT map_PAYLOAD_TYPE_E_to_EVIDEOCODECFORMAT(PAYLOAD_TYPE_E nPayLoadType)
{
    enum EVIDEOCODECFORMAT nVDecLibType;
    switch (nPayLoadType) {
        case PT_H264:
            nVDecLibType = VIDEO_CODEC_FORMAT_H264;
            break;
        case PT_H265:
            nVDecLibType = VIDEO_CODEC_FORMAT_H265;
            break;
        case PT_JPEG:
        case PT_MJPEG:
            nVDecLibType = VIDEO_CODEC_FORMAT_MJPEG;
            break;
        default:
            alogw("fatal error! unsupported format[0x%x]", nPayLoadType);
            nVDecLibType = VIDEO_CODEC_FORMAT_MJPEG;
            break;
    }
    return nVDecLibType;
}

PAYLOAD_TYPE_E map_EVIDEOCODECFORMAT_to_PAYLOAD_TYPE_E(enum EVIDEOCODECFORMAT eCodecFormat)
{
    PAYLOAD_TYPE_E dstType;
    switch (eCodecFormat) {
        case VIDEO_CODEC_FORMAT_H264:
            dstType = PT_H264;
            break;
        case VIDEO_CODEC_FORMAT_MJPEG:
            dstType = PT_MJPEG;
            break;
        case VIDEO_CODEC_FORMAT_H265:
            dstType = PT_H265;
            break;
        default:
            aloge("fatal error! unsupported format[0x%x]", eCodecFormat);
            dstType = PT_MAX;
            break;
    }
    return dstType;
}

PAYLOAD_TYPE_E map_EAUDIOCODECFORMAT_to_PAYLOAD_TYPE_E(enum EAUDIOCODECFORMAT eCodecFormat)
{
    PAYLOAD_TYPE_E dstType;
    switch(eCodecFormat)
    {
        case AUDIO_CODEC_FORMAT_MPEG_AAC_LC:
        case AUDIO_CODEC_FORMAT_RAAC:
            dstType = PT_AAC;
            break;
        case AUDIO_CODEC_FORMAT_MP1:
        case AUDIO_CODEC_FORMAT_MP2:
        case AUDIO_CODEC_FORMAT_MP3:
            dstType = PT_MP3;
            break;
        default:
            aloge("fatal error! unsupported format[0x%x]", eCodecFormat);
            dstType = PT_MAX;
            break;
    }
    return dstType;
}

unsigned int GetBitRateFromVENC_CHN_ATTR_S(VENC_CHN_ATTR_S *pAttr)
{
    unsigned int nBitRate;
    switch (pAttr->RcAttr.mRcMode) 
    {
        case VENC_RC_MODE_H264CBR:
            nBitRate = pAttr->RcAttr.mAttrH264Cbr.mBitRate;
            break;
        case VENC_RC_MODE_H264VBR:
            nBitRate = pAttr->RcAttr.mAttrH264Vbr.mMaxBitRate;
            break;
        case VENC_RC_MODE_H264FIXQP:
            nBitRate = 0;
            break;
        case VENC_RC_MODE_H264ABR:
            nBitRate = pAttr->RcAttr.mAttrH264Abr.mMaxBitRate;
            break;
        case VENC_RC_MODE_H265CBR:
            nBitRate = pAttr->RcAttr.mAttrH265Cbr.mBitRate;
            break;
        case VENC_RC_MODE_H265VBR:
            nBitRate = pAttr->RcAttr.mAttrH265Vbr.mMaxBitRate;
            break;
        case VENC_RC_MODE_H265FIXQP:
            nBitRate = 0;
            break;
        case VENC_RC_MODE_H265ABR:
            nBitRate = pAttr->RcAttr.mAttrH265Abr.mMaxBitRate;
            break;
        case VENC_RC_MODE_MJPEGCBR:
            nBitRate = pAttr->RcAttr.mAttrMjpegeCbr.mBitRate;
            break;
        case VENC_RC_MODE_MJPEGFIXQP:
            nBitRate = 0;
            break;
        default:
            alogw("unsupported temporary: chn attr RcAttr RcMode[0x%x]", pAttr->RcAttr.mRcMode);
            nBitRate = 0;
            break;
    }
    return nBitRate;
}
