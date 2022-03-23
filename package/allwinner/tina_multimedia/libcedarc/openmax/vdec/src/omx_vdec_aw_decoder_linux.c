/*
 * =====================================================================================
 *
 *       Filename:  omx_vdec_aw_decoder_linux.c
 *
 *    Description: 1. hardware decode with AW decoder
 *                        2. using copy process
 *
 *        Version:  1.0
 *        Created:  08/02/2018 04:18:11 PM
 *       Revision:  none
 *
 *         Author:  Gan Qiuye
 *        Company:
 *
 * =====================================================================================
 */

#define LOG_TAG "omx_vdec_aw"
#include "log.h"
#include "omx_vdec_decoder.h"
#include "vdecoder.h"
#include "omx_vdec_port.h"
#include "omx_vdec_config.h"
#include "OMX_Video.h"
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include "memoryAdapter.h"
#include "veAdapter.h"
#include "veInterface.h"
#include "CdcUtil.h"
#include "omx_mutex.h"
#include "omx_sem.h"

#define USE_ALIGN_SIZE 1

typedef struct TranBufferInfo
{
    void* pAddr;
    int  nWidth;
    int  nHeight;
}TranBufferInfo;

typedef struct OmxAwDecoderContext
{
    OmxDecoder             base;
    VideoDecoder*          m_decoder;
    VideoStreamInfo        mStreamInfo;
    VConfig                mVideoConfig;
    OMX_S32                mGpuAlignStride;
    struct ScMemOpsS*      decMemOps;

    OMX_CONFIG_RECTTYPE    mVideoRect; //* for display crop
    OMX_U32                mCodecSpecificDataLen;
    OMX_U8                 mCodecSpecificData[CODEC_SPECIFIC_DATA_LENGTH];
    DecoderCallback        callback;
    void*                  pUserData;

    OMX_BOOL               bInputEosFlag;
    OMX_BOOL               bIsFirstGetOffset;

    AwOmxVdecPort*         pInPort;
    AwOmxVdecPort*         pOutPort;

    OmxMutexHandle         awMutexHandler;
    OmxSemHandle           mSemOutBuffer;
    OmxSemHandle           mSemInData;
    OmxSemHandle           mSemValidPic;

    VideoPicture*          pPicture;

    OMX_BOOL               bUseZeroCopyBuffer;


}OmxAwDecoderContext;

static OMX_U32 liGetStreamFormat(OMX_VIDEO_CODINGTYPE videoCoding)
{
    switch(videoCoding)
    {
        case OMX_VIDEO_CodingMJPEG:
            return VIDEO_CODEC_FORMAT_MJPEG;//mjpeg
        case OMX_VIDEO_CodingMPEG1:
            return VIDEO_CODEC_FORMAT_MPEG1;//mpeg1
        case OMX_VIDEO_CodingMPEG2:
            return OMX_VIDEO_CodingMPEG2;//mpeg2
        case OMX_VIDEO_CodingMPEG4:
            return VIDEO_CODEC_FORMAT_XVID;//xvid-mpeg4
        case OMX_VIDEO_CodingMSMPEG4V1:
            return VIDEO_CODEC_FORMAT_MSMPEG4V1;//mpeg4v1
        case OMX_VIDEO_CodingMSMPEG4V2:
            return VIDEO_CODEC_FORMAT_MSMPEG4V2;//mpeg4v2
        case OMX_VIDEO_CodingMSMPEG4V3:
            return VIDEO_CODEC_FORMAT_DIVX3;//divx3-mpeg4v3
        case OMX_VIDEO_CodingDIVX4:
            return VIDEO_CODEC_FORMAT_DIVX4;//divx4
        case OMX_VIDEO_CodingRX:
            return VIDEO_CODEC_FORMAT_RX;//rx
        case OMX_VIDEO_CodingAVS:
            return VIDEO_CODEC_FORMAT_AVS;//avs
        case OMX_VIDEO_CodingDIVX:
            return VIDEO_CODEC_FORMAT_DIVX5;//divx5
        case OMX_VIDEO_CodingXVID:
            return VIDEO_CODEC_FORMAT_XVID;//xvid
        case OMX_VIDEO_CodingH263:
            return VIDEO_CODEC_FORMAT_H263;//h263
        case OMX_VIDEO_CodingS263:
            return VIDEO_CODEC_FORMAT_SORENSSON_H263;//sh263-sorensson
        case OMX_VIDEO_CodingRXG2:
            return VIDEO_CODEC_FORMAT_RXG2;//rxg2
        case OMX_VIDEO_CodingWMV1:
            return VIDEO_CODEC_FORMAT_WMV1;//wmv1
        case OMX_VIDEO_CodingWMV2:
            return VIDEO_CODEC_FORMAT_WMV2;//wmv2
        case OMX_VIDEO_CodingWMV:
            return VIDEO_CODEC_FORMAT_WMV3;//wmv3
        case OMX_VIDEO_CodingVP6:
            return VIDEO_CODEC_FORMAT_VP6;//vp6
        case OMX_VIDEO_CodingVP8:
            return VIDEO_CODEC_FORMAT_VP8;//vp8
        case OMX_VIDEO_CodingVP9:
            return VIDEO_CODEC_FORMAT_VP9;//vp9
        case OMX_VIDEO_CodingAVC:
            return VIDEO_CODEC_FORMAT_H264;//h264-avc
        case OMX_VIDEO_CodingHEVC:
            return VIDEO_CODEC_FORMAT_H265;//h265-hevc
        default:
        {
            OMX_LOGE("unsupported OMX this format:%d", videoCoding);
            break;
        }
    }
    return VIDEO_CODEC_FORMAT_UNKNOWN;
}

void liGetStremInfo(OmxAwDecoderContext *pCtx)
{
    OMX_PARAM_PORTDEFINITIONTYPE* inDef = getPortDef(pCtx->pInPort);
    OMX_VIDEO_PARAM_PORTFORMATTYPE* inFormatType = getPortFormatType(pCtx->pInPort);

    pCtx->mStreamInfo.eCodecFormat = liGetStreamFormat(inFormatType->eCompressionFormat);
    pCtx->mStreamInfo.nWidth  = inDef->format.video.nFrameWidth;
    pCtx->mStreamInfo.nHeight = inDef->format.video.nFrameHeight;
}

static int liDealWithInitData(OmxAwDecoderContext *pCtx)
{
    OMX_BUFFERHEADERTYPE* pInBufHdr = doRequestPortBuffer(pCtx->pInPort);

    if(pInBufHdr == NULL)
    {
        OMX_LOGD("fatal error! pInBufHdr is NULL, check code!");
        return -1;
    }

    if(pInBufHdr->nFlags & OMX_BUFFERFLAG_CODECCONFIG)
    {
        OMX_ASSERT((pInBufHdr->nFilledLen + pCtx->mCodecSpecificDataLen) <=
                    CODEC_SPECIFIC_DATA_LENGTH);

        OMX_U8* pInBuffer = pInBufHdr->pBuffer;
        memcpy(pCtx->mCodecSpecificData + pCtx->mCodecSpecificDataLen,
               pInBuffer,
               pInBufHdr->nFilledLen);

        pCtx->mCodecSpecificDataLen += pInBufHdr->nFilledLen;
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_EMPTY_BUFFER_DONE, (void*)pInBufHdr);
    }
    else
    {
        OMX_LOGD("++++++++++++++++pCtx->mCodecSpecificDataLen[%d]",(int)pCtx->mCodecSpecificDataLen);
        if(pCtx->mCodecSpecificDataLen > 0)
        {
            if(pCtx->mStreamInfo.pCodecSpecificData)
                free(pCtx->mStreamInfo.pCodecSpecificData);
            pCtx->mStreamInfo.nCodecSpecificDataLen = pCtx->mCodecSpecificDataLen;
            pCtx->mStreamInfo.pCodecSpecificData    = (char*)malloc(pCtx->mCodecSpecificDataLen);
            memset(pCtx->mStreamInfo.pCodecSpecificData, 0, pCtx->mCodecSpecificDataLen);
            memcpy(pCtx->mStreamInfo.pCodecSpecificData,
                   pCtx->mCodecSpecificData, pCtx->mCodecSpecificDataLen);
        }
        else
        {
            pCtx->mStreamInfo.pCodecSpecificData      = NULL;
            pCtx->mStreamInfo.nCodecSpecificDataLen = 0;
        }
        returnPortBuffer(pCtx->pInPort);
        return 0;
    }
    return -1;
}

static int liCheckResolutionChange(OmxAwDecoderContext *pCtx, VideoPicture* picture)
{
    int width_align  = 0;
    int height_align = 0;
    OMX_CONFIG_RECTTYPE mCurVideoRect;
    memset(&mCurVideoRect, 0, sizeof(OMX_CONFIG_RECTTYPE));

    if(pCtx->bIsFirstGetOffset)
    {
        pCtx->mVideoRect.nLeft   = picture->nLeftOffset;
        pCtx->mVideoRect.nTop    = picture->nTopOffset;
        pCtx->mVideoRect.nWidth  = picture->nRightOffset - picture->nLeftOffset;
        pCtx->mVideoRect.nHeight = picture->nBottomOffset - picture->nTopOffset;
        OMX_LOGD("gqy********l:%ld, t:%ld, w:%ld, h:%ld",
            pCtx->mVideoRect.nLeft,
            pCtx->mVideoRect.nTop,
            pCtx->mVideoRect.nWidth,
            pCtx->mVideoRect.nHeight);
        pCtx->bIsFirstGetOffset = OMX_FALSE;
    }

    mCurVideoRect.nLeft   = picture->nLeftOffset;
    mCurVideoRect.nTop    = picture->nTopOffset;
    mCurVideoRect.nWidth  = picture->nRightOffset  - picture->nLeftOffset;
    mCurVideoRect.nHeight = picture->nBottomOffset - picture->nTopOffset;
#if USE_ALIGN_SIZE
    width_align  = picture->nWidth;
    height_align = picture->nHeight;

#else
    width_align  = picture->nRightOffset  - picture->nLeftOffset;
    height_align = picture->nBottomOffset - picture->nTopOffset;
    if(width_align <= 0 || height_align <= 0)
    {
        width_align  = picture->nWidth;
        height_align = picture->nHeight;
    }
#endif
    //* make the height to 2 align
   // height_align = (height_align + 1) & ~1;
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->pOutPort);

    if((OMX_U32)width_align != outDef->format.video.nFrameWidth
        || (OMX_U32)height_align  != outDef->format.video.nFrameHeight
        || (mCurVideoRect.nLeft   != pCtx->mVideoRect.nLeft)
        || (mCurVideoRect.nTop    != pCtx->mVideoRect.nTop)
        || (mCurVideoRect.nWidth  != pCtx->mVideoRect.nWidth)
        || (mCurVideoRect.nHeight != pCtx->mVideoRect.nHeight))
    {
        logw(" port setting changed -- old info : widht = %d, height = %d, "
               "mVideoRect: %d, %d, %d, %d ",
                (int)outDef->format.video.nFrameWidth,
                (int)outDef->format.video.nFrameHeight,
                (int)pCtx->mVideoRect.nTop,(int)pCtx->mVideoRect.nLeft,
                (int)pCtx->mVideoRect.nWidth,(int)pCtx->mVideoRect.nHeight);
        logw(" port setting changed -- new info : widht = %d, height = %d, "
               "mVideoRect: %d, %d, %d, %d ",
                (int)width_align, (int)height_align,
                (int)mCurVideoRect.nTop,(int)mCurVideoRect.nLeft,
                (int)mCurVideoRect.nWidth,(int)mCurVideoRect.nHeight);

        memcpy(&pCtx->mVideoRect, &mCurVideoRect, sizeof(OMX_CONFIG_RECTTYPE));
        outDef->format.video.nFrameHeight = height_align;
        outDef->format.video.nFrameWidth  = width_align;
        outDef->format.video.nStride      = width_align;
        outDef->format.video.nSliceHeight = height_align;

        outDef->nBufferSize = height_align*width_align *3/2;
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_NOTIFY_RECT, &(pCtx->mVideoRect));
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_PORT_CHANGE, NULL);
        return 0;
    }
    return -1;
}


static void AlignCopyYV12(unsigned char* dstPtr,
                                                 unsigned char* srcPtr,
                                                 int w, int h)
{
    int yPlaneSz = w*h;
    int uvPlaneSz = yPlaneSz/4;
    //copy y
    memcpy(dstPtr, srcPtr, yPlaneSz);

    //copy v
    unsigned char* uSrcPtr = srcPtr + yPlaneSz;
    unsigned char* uDstPtr = dstPtr + yPlaneSz;
    memcpy(uDstPtr, uSrcPtr, uvPlaneSz);

    //*copy u
    unsigned char* vSrcPtr = srcPtr  + yPlaneSz*5/4;
    unsigned char* vDstPtr = uDstPtr + uvPlaneSz;
    memcpy(vDstPtr, vSrcPtr, uvPlaneSz);
}
static void AlignTransformYV12ToYUV420(unsigned char* dstPtr,
                                                 unsigned char* srcPtr,
                                                 int w, int h)
{
    int yPlaneSz = w*h;
    int uvPlaneSz = yPlaneSz/4;
    //copy y
    memcpy(dstPtr, srcPtr, yPlaneSz);

    //copy u
    unsigned char* uSrcPtr = srcPtr + yPlaneSz*5/4;
    unsigned char* uDstPtr = dstPtr + yPlaneSz;
    memcpy(uDstPtr, uSrcPtr, uvPlaneSz);

    //*copy v
    unsigned char* vSrcPtr = srcPtr  + yPlaneSz;
    unsigned char* vDstPtr = uDstPtr + uvPlaneSz;
    memcpy(vDstPtr, vSrcPtr, uvPlaneSz);
}

static void TransformYV12ToYUV420(VideoPicture* pPicture,
                                  TranBufferInfo* pTranBufferInfo)
{
    int i;
    int nPicRealWidth;
    int nPicRealHeight;
    int nSrcBufWidth;
    int nSrcBufHeight;
    int nDstBufWidth;
    int nDstBufHeight;
    int nCopyDataWidth;
    int nCopyDataHeight;
    unsigned char* dstPtr;
    unsigned char* srcPtr;
    dstPtr      = (unsigned char*)pTranBufferInfo->pAddr;
    srcPtr      = (unsigned char*)pPicture->pData0;

    nPicRealWidth  = pPicture->nRightOffset  - pPicture->nLeftOffset;
    nPicRealHeight = pPicture->nBottomOffset - pPicture->nTopOffset;

    //* if the uOffset is not right, we should not use them to compute width and height
    if(nPicRealWidth <= 0 || nPicRealHeight <= 0)
    {
        nPicRealWidth  = pPicture->nWidth;
        nPicRealHeight = pPicture->nHeight;
    }

    nSrcBufWidth  = (pPicture->nWidth + 15) & ~15;
    nSrcBufHeight = (pPicture->nHeight + 15) & ~15;

    //* On chip-1673, the gpu is 32 align ,but here is not copy to gpu, so also 16 align.
    //* On other chip, gpu buffer is 16 align.
    //nDstBufWidth = (pTranBufferInfo->nWidth+ 15)&~15;
    nDstBufWidth  = pTranBufferInfo->nWidth;

    nDstBufHeight = pTranBufferInfo->nHeight;

    nCopyDataWidth  = nPicRealWidth;
    nCopyDataHeight = nPicRealHeight;

    logv("nPicRealWidth & H = %d, %d, nSrcBufWidth & H = %d, %d,"
         "nDstBufWidth & H = %d, %d,  nCopyDataWidth & H = %d, %d",
          nPicRealWidth,nPicRealHeight,
          nSrcBufWidth,nSrcBufHeight,
          nDstBufWidth,nDstBufHeight,
          nCopyDataWidth,nCopyDataHeight);

    //*copy y
    for(i=0; i < nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth;
        srcPtr += nSrcBufWidth;
    }

    //*copy u
    srcPtr = ((unsigned char*)pPicture->pData0) + nSrcBufWidth*nSrcBufHeight*5/4;
    nCopyDataWidth  = (nCopyDataWidth+1)/2;
    nCopyDataHeight = (nCopyDataHeight+1)/2;

    for(i=0; i < nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth/2;
        srcPtr += nSrcBufWidth/2;
    }

    //*copy v
    srcPtr = ((unsigned char*)pPicture->pData0) + nSrcBufWidth*nSrcBufHeight;
    for(i=0; i<nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth/2;    //a31 gpu, uv is half of y
        srcPtr += nSrcBufWidth/2;
    }

    return;
}

static void liReopenVideoEngine(OmxAwDecoderContext *pCtx)
{
    OMX_LOGD("***ReopenVideoEngine!");

    if(pCtx->mStreamInfo.pCodecSpecificData != NULL)
    {
        free(pCtx->mStreamInfo.pCodecSpecificData);
        pCtx->mStreamInfo.pCodecSpecificData  = NULL;
        pCtx->mStreamInfo.nCodecSpecificDataLen = 0;
    }

    OmxAcquireMutex(pCtx->awMutexHandler);
    ReopenVideoEngine(pCtx->m_decoder, &pCtx->mVideoConfig, &(pCtx->mStreamInfo));
    OmxReleaseMutex(pCtx->awMutexHandler);

    return ;
}

static int __liCallback(OmxDecoder* pDec, DecoderCallback callback,
                                    void* pUserData)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    pCtx->callback  = callback;
    pCtx->pUserData = pUserData;
    return 0;
}
static OMX_ERRORTYPE __liGetExtPara(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eParamIndex,
                                       OMX_PTR pParamData)
{
    CEDARC_UNUSE(pDec);
    CEDARC_UNUSE(pParamData);
    CEDARC_UNUSE(eParamIndex);
    logw("get_parameter: unknown param %08x\n", eParamIndex);
    return OMX_ErrorUnsupportedIndex;
}

static OMX_ERRORTYPE __liSetExtPara(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eParamIndex,
                                       OMX_PTR pParamData)
{
    CEDARC_UNUSE(pDec);
    CEDARC_UNUSE(pParamData);
    logw("Setparameter: unknown param %d\n", eParamIndex);
    return OMX_ErrorUnsupportedIndex;
}

static OMX_ERRORTYPE __liGetExtConfig(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eConfigIndex,
                                       OMX_PTR pConfigData)
{
    CEDARC_UNUSE(pDec);
    CEDARC_UNUSE(pConfigData);
    logw("Setparameter: unknown param %d\n", eConfigIndex);
    return OMX_ErrorUnsupportedIndex;
}

static OMX_ERRORTYPE __liSetExtConfig(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eConfigIndex,
                                       OMX_PTR pConfigData)
{
    CEDARC_UNUSE(pDec);
    CEDARC_UNUSE(pConfigData);
    logw("Setparameter: unknown param %d\n", eConfigIndex);
    return OMX_ErrorUnsupportedIndex;
}

static int __liPrepare(OmxDecoder* pDec)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    int ret = -1;
    if(-1 == liDealWithInitData(pCtx))
        return -1;
    OMX_LOGD("decoder prepare");
    liGetStremInfo(pCtx);

    OmxAcquireMutex(pCtx->awMutexHandler);
    //*if mdecoder had closed before, we should create it
    if(pCtx->m_decoder==NULL)
    {
        AddVDPlugin();
        pCtx->m_decoder = CreateVideoDecoder();
    }

    pCtx->mVideoConfig.nAlignStride       = pCtx->mGpuAlignStride;

    pCtx->mVideoConfig.eOutputPixelFormat = PIXEL_FORMAT_YV12;//* Used to be YV12.

#if (ENABLE_SCALEDOWN_WHEN_RESOLUTION_MOER_THAN_1080P)
    if (pCtx->mStreamInfo.nWidth > 1920
        && pCtx->mStreamInfo.nHeight > 1088)
    {
        pCtx->mVideoConfig.bScaleDownEn = 1;
        pCtx->mVideoConfig.nHorizonScaleDownRatio = 1;
        pCtx->mVideoConfig.nVerticalScaleDownRatio = 1;
    }
#endif

    if(pCtx->mStreamInfo.eCodecFormat == VIDEO_CODEC_FORMAT_WMV3)
    {
        OMX_LOGD("*** pCtx->mStreamInfo.bIsFramePackage to 1 when it is vc1");
        pCtx->mStreamInfo.bIsFramePackage = 1;
    }


    pCtx->mVideoConfig.bDispErrorFrame = 1;

    pCtx->mVideoConfig.nDeInterlaceHoldingFrameBufferNum = 0;
    pCtx->mVideoConfig.nDisplayHoldingFrameBufferNum  = 0;
    pCtx->mVideoConfig.nRotateHoldingFrameBufferNum \
                         = NUM_OF_PICTURES_KEEPPED_BY_ROTATE;
    pCtx->mVideoConfig.nDecodeSmoothFrameBufferNum \
                         = NUM_OF_PICTURES_FOR_EXTRA_SMOOTH;

    pCtx->mVideoConfig.memops = MemAdapterGetOpsS();
    pCtx->decMemOps  = pCtx->mVideoConfig.memops;
    CdcMemOpen(pCtx->decMemOps);
    pCtx->mStreamInfo.bIsFramePackage = 1;
    ret = InitializeVideoDecoder(pCtx->m_decoder,
                                 &(pCtx->mStreamInfo),
                                 &pCtx->mVideoConfig);
    if(ret != 0)
    {
        DestroyVideoDecoder(pCtx->m_decoder);
        pCtx->m_decoder           = NULL;
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_EVENT_ERROR, NULL);
        OMX_LOGE("Idle transition failed, set_vstream_info() return fail.\n");
        OmxReleaseMutex(pCtx->awMutexHandler);
        return ret;
    }
    OmxReleaseMutex(pCtx->awMutexHandler);
    OmxTryPostSem(pCtx->mSemInData);
    OmxTryPostSem(pCtx->mSemOutBuffer);
    return ret;
}

static int __liSubmit(OmxDecoder* pDec, OMX_BUFFERHEADERTYPE* pInBufHdr)
{
    logv("OmxCopyInputDataToDecoder()");

    char* pBuf0;
    char* pBuf1;
    int   size0;
    int   size1;
    int   require_size;
    unsigned char*   pData;
    VideoStreamDataInfo DataInfo;
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;

    memset(&DataInfo, 0, sizeof(VideoStreamDataInfo));

    require_size = pInBufHdr->nFilledLen;

    OmxAcquireMutex(pCtx->awMutexHandler);
    if(0 != RequestVideoStreamBuffer(pCtx->m_decoder, require_size,
                                     &pBuf0, &size0, &pBuf1, &size1,0))
    {
        logv("req vbs fail! maybe vbs buffer is full! require_size[%d]",
             require_size);
        OmxReleaseMutex(pCtx->awMutexHandler);
        return -1;
    }

    if(require_size != (size0 + size1))
    {
        OmxReleaseMutex(pCtx->awMutexHandler);
        logw(" the requestSize[%d] is not equal to needSize[%d]!",(size0+size1),require_size);
        return -1;
    }

    DataInfo.nLength      = require_size;
    DataInfo.bIsFirstPart = 1;
    DataInfo.bIsLastPart  = 1;
    DataInfo.pData        = pBuf0;
    if(pInBufHdr->nTimeStamp >= 0)
    {
        DataInfo.nPts   = pInBufHdr->nTimeStamp;
        DataInfo.bValid = 1;
    }
    else
    {
        DataInfo.nPts   = -1;
        DataInfo.bValid = 0;
    }

    pData = pInBufHdr->pBuffer + pInBufHdr->nOffset;
    if(require_size <= size0)
    {
        memcpy(pBuf0, pData, require_size);
    }
    else
    {
        memcpy(pBuf0, pData, size0);
        pData += size0;
        memcpy(pBuf1, pData, require_size - size0);
    }
    SubmitVideoStreamData(pCtx->m_decoder, &DataInfo,0);

    OmxReleaseMutex(pCtx->awMutexHandler);
    return 0 ;
}

static inline void __liDecode(OmxDecoder* pDec)
{
    int decodeResult;
    int validPicNum = 0;

    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;

    OmxAcquireMutex(pCtx->awMutexHandler);
    decodeResult = DecodeVideoStream(pCtx->m_decoder,0,0,0,0);
    logv("***decodeResult = %d",decodeResult);
    OmxReleaseMutex(pCtx->awMutexHandler);

    if(decodeResult == VDECODE_RESULT_KEYFRAME_DECODED ||
       decodeResult == VDECODE_RESULT_FRAME_DECODED ||
       decodeResult == VDECODE_RESULT_OK)
    {
        OmxTryPostSem(pCtx->mSemValidPic);
    }
    else if(decodeResult == VDECODE_RESULT_NO_FRAME_BUFFER)
    {
        OmxTimedWaitSem(pCtx->mSemOutBuffer, 20);
    }
    else if(decodeResult == VDECODE_RESULT_NO_BITSTREAM ||
            decodeResult == VDECODE_RESULT_CONTINUE)
    {
        if(pCtx->bInputEosFlag)
        {
            //pCtx->bInputEosFlag = OMX_FALSE;

            //*set eos to decoder ,decoder should flush all fbm
            int mRet = 0;
            int mDecodeCount = 0;
            while(mRet != VDECODE_RESULT_NO_BITSTREAM)
            {
                OmxAcquireMutex(pCtx->awMutexHandler);
                mRet = DecodeVideoStream(pCtx->m_decoder,1,0,0,0);
                OmxReleaseMutex(pCtx->awMutexHandler);
                if(mRet == VDECODE_RESULT_RESOLUTION_CHANGE)
                    goto resolution_change;
                usleep(5*1000);
                mDecodeCount++;
                if(mDecodeCount > 1000)
                {
                    logw(" decoder timeOut when set eos to decoder!");
                    break;
                }
            }

            pCtx->callback(pCtx->pUserData, AW_OMX_CB_NOTIFY_EOS, NULL);
            OmxTimedWaitSem(pCtx->mSemInData, 20);
        }
        else
        {
            OmxTimedWaitSem(pCtx->mSemInData, 20);
        }
    }
    else if(decodeResult == VDECODE_RESULT_RESOLUTION_CHANGE)
    {
    resolution_change:
        validPicNum = ValidPictureNum(pCtx->m_decoder, 0);
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_FLUSH_ALL_PIC, &validPicNum);
        liReopenVideoEngine(pCtx);
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_FININSH_REOPEN_VE, NULL);
    }
    else if(decodeResult < 0)
    {
        logw("decode fatal error[%d]", decodeResult);
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_EVENT_ERROR, NULL);
    }
    else
    {
        OMX_LOGD("decode ret[%d], ignore? why?", decodeResult);
    }
    return ;
}


static OMX_BUFFERHEADERTYPE* __liDrain(OmxDecoder* pDec)
{
    VideoPicture*           pPicture     = NULL;
    OMX_BUFFERHEADERTYPE*   pOutBufHdr  = NULL;
    TranBufferInfo          mTranBufferInfo;
    memset(&mTranBufferInfo, 0 ,sizeof(TranBufferInfo));
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;

    int validPicNum = ValidPictureNum(pCtx->m_decoder, 0);
    if(validPicNum <= 0)
    {
        return NULL;
    }
    OmxAcquireMutex(pCtx->awMutexHandler);
    pPicture = NextPictureInfo(pCtx->m_decoder,0);
    OmxReleaseMutex(pCtx->awMutexHandler);
    if(pPicture == NULL)
        return NULL;

    if(liCheckResolutionChange(pCtx, pPicture) == 0)
    {
        return NULL;
    }
    pOutBufHdr = doRequestPortBuffer(pCtx->pOutPort);
    if(pOutBufHdr == NULL)
    {
        return NULL;
    }

    OmxAcquireMutex(pCtx->awMutexHandler);
    pCtx->pPicture = RequestPicture(pCtx->m_decoder, 0);
    OmxReleaseMutex(pCtx->awMutexHandler);
    logv("*** get picture[%p]",pCtx->pPicture);
    if(pCtx->pPicture == NULL)
    {
        logw("the pPicture is null when request displayer picture!");
        return NULL;
    }
    logv("*** picture info: w(%d),h(%d),offset,t(%d),b(%d),l(%d),r(%d)",
         pCtx->pPicture->nWidth,    pCtx->pPicture->nHeight,
         pCtx->pPicture->nTopOffset,pCtx->pPicture->nBottomOffset,
         pCtx->pPicture->nLeftOffset,pCtx->pPicture->nRightOffset);
    CdcMemFlushCache(pCtx->decMemOps, (void*)pCtx->pPicture->pData0,
             pCtx->pPicture->nLineStride * pCtx->pPicture->nHeight*3/2);
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->pOutPort);
#if USE_ALIGN_SIZE
    AlignCopyYV12((unsigned char*)pOutBufHdr->pBuffer,
                                (unsigned char*)pCtx->pPicture->pData0,
                                outDef->format.video.nFrameWidth,
                                outDef->format.video.nFrameHeight);

#else
    mTranBufferInfo.pAddr   = pOutBufHdr->pBuffer;
    mTranBufferInfo.nWidth  = outDef->format.video.nFrameWidth;
    mTranBufferInfo.nHeight = outDef->format.video.nFrameHeight;
    TransformYV12ToYUV420(pCtx->pPicture, &mTranBufferInfo);  // YUV420 planar
#endif
    pOutBufHdr->nTimeStamp = pCtx->pPicture->nPts;
    pOutBufHdr->nOffset    = 0;
    pOutBufHdr->nFilledLen = (outDef->format.video.nFrameWidth *
                              outDef->format.video.nFrameHeight) * 3 / 2;

    return pOutBufHdr;

}

static void __liStandbyBuffer(OmxDecoder* pDec)
{
    CEDARC_UNUSE(pDec);
    return ;
}

static int __liReturnBuffer(OmxDecoder* pDec)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;

    if(pCtx->pPicture != NULL)
    {
        OmxAcquireMutex(pCtx->awMutexHandler);
        ReturnPicture(pCtx->m_decoder, pCtx->pPicture);
        pCtx->pPicture = NULL;
        OmxReleaseMutex(pCtx->awMutexHandler);
        OmxTryPostSem(pCtx->mSemOutBuffer);
    }
    return 0;
}

static inline void __liFlush(OmxDecoder* pDec)
{
    OMX_LOGD("decoder flush");
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;

    //* we should reset the mInputEosFlag when flush vdecoder
    pCtx->bInputEosFlag = OMX_FALSE;

    if(pCtx->m_decoder)
    {
        OmxTryPostSem(pCtx->mSemInData);
        OmxTryPostSem(pCtx->mSemOutBuffer);
        OmxAcquireMutex(pCtx->awMutexHandler);
        ResetVideoDecoder(pCtx->m_decoder);
        OmxReleaseMutex(pCtx->awMutexHandler);
    }
    else
    {
        logw(" fatal error, m_decoder is not malloc when flush all ports!");
    }
}

static  void __liClose(OmxDecoder* pDec)
{
    OMX_LOGD("decoder close");
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    OmxTryPostSem(pCtx->mSemInData);
    OmxTryPostSem(pCtx->mSemOutBuffer);
    OmxReleaseMutex(pCtx->awMutexHandler);
    if(pCtx->m_decoder != NULL)
    {
        DestroyVideoDecoder(pCtx->m_decoder);
        pCtx->m_decoder           = NULL;
    }
    pCtx->mCodecSpecificDataLen = 0;
    memset(pCtx->mCodecSpecificData, 0 , CODEC_SPECIFIC_DATA_LENGTH);
    OmxReleaseMutex(pCtx->awMutexHandler);
}

static void __liSetInputEos(OmxDecoder* pDec, OMX_BOOL bEos)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    pCtx->bInputEosFlag = bEos;
}

static int __liSetOutputEos(OmxDecoder* pDec)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    OMX_U32 size = getPortValidSize(pCtx->pOutPort);
    logv("*** OutBufList.nSizeOfList = %lu",size);

    if(size > 0)
    {
        while (getPortValidSize(pCtx->pOutPort) > 0)
        {
            OMX_BUFFERHEADERTYPE* pOutBufHdr = doRequestPortBuffer(pCtx->pOutPort);

            if(pOutBufHdr==NULL)
                continue;

            if (pOutBufHdr->nFilledLen != 0)
            {
                pCtx->callback(pCtx->pUserData, AW_OMX_CB_FILL_BUFFER_DONE,
                               (void*)pOutBufHdr);
                pOutBufHdr = NULL;
            }
            else
            {
                OMX_LOGD("++++ set output eos(normal)");
                pOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                pCtx->callback(pCtx->pUserData, AW_OMX_CB_FILL_BUFFER_DONE,
                               (void*)pOutBufHdr);
                pOutBufHdr = NULL;

                break;
            }
        }
    }
    return 0;
}

static void __liGetFormat(OmxDecoder* pDec)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    pCtx->mVideoConfig.eOutputPixelFormat = PIXEL_FORMAT_YV12;
}

static inline void __liSetExtBufNum(OmxDecoder* pDec, OMX_S32 num)
{
    CEDARC_UNUSE(pDec);
    CEDARC_UNUSE(num);
}

static OMX_U8* __liAllocatePortBuffer(OmxDecoder* pDec, AwOmxVdecPort* port, OMX_U32 size)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    OMX_U8* pBuffer = NULL;
    OMX_LOGD("kay: *** %d.", pCtx->bUseZeroCopyBuffer);
    pCtx->bUseZeroCopyBuffer = OMX_FALSE;

    if(!isInputPort(port) && pCtx->bUseZeroCopyBuffer)
    {
    OMX_LOGD("kay: mem palloc");
        pBuffer = (OMX_U8*)CdcMemPalloc(pCtx->decMemOps, size, NULL, NULL);
         OMX_LOGD("kay: mem palloc 2");

    }
    else
        {
        OMX_LOGD("kay: malloc");
        pBuffer = (OMX_U8*)malloc(size);
        OMX_LOGD("kay: malloc2");
        }
    return pBuffer;
}

static void __liFreePortBuffer(OmxDecoder* pDec, AwOmxVdecPort* port, OMX_U8* pBuffer)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    if(!isInputPort(port) && pCtx->bUseZeroCopyBuffer)
        CdcMemPfree(pCtx->decMemOps, pBuffer, NULL, NULL);
    else
        free(pBuffer);
    return ;
}

static OmxDecoderOpsT mAwDecoderOps =
{
    .getExtPara   =   __liGetExtPara,
    .setExtPara   =   __liSetExtPara,
    .getExtConfig =   __liGetExtConfig,
    .setExtConfig =   __liSetExtConfig,
    .prepare      =   __liPrepare,
    .close        =   __liClose,
    .flush        =   __liFlush,
    .decode       =   __liDecode,
    .submit       =   __liSubmit,
    .drain        =   __liDrain,
    .callback     =   __liCallback,
    .setInputEos  =   __liSetInputEos,
    .setOutputEos =   __liSetOutputEos,
    .standbyBuf   =   __liStandbyBuffer,
    .returnBuf    =   __liReturnBuffer,
    .getFormat    =   __liGetFormat,
    .setExtBufNum =   __liSetExtBufNum,
    .allocPortBuf =   __liAllocatePortBuffer,
    .freePortBuf  =   __liFreePortBuffer,
};


OmxDecoder* OmxDecoderCreate(AwOmxVdecPort* in, AwOmxVdecPort* out)
{
    logv("OmxDecoderCreate.");
    OmxAwDecoderContext* pCtx;
    pCtx = (OmxAwDecoderContext*)malloc(sizeof(OmxAwDecoderContext));
    if(pCtx == NULL)     {
        OMX_LOGE("malloc memory fail.");
        return NULL;
    }
    memset(pCtx, 0, sizeof(OmxAwDecoderContext));
    pCtx->pInPort = in;
    pCtx->pOutPort = out;
    pCtx->bIsFirstGetOffset = OMX_TRUE;

#ifdef CONF_OMX_USE_ZERO_COPY
    pCtx->bUseZeroCopyBuffer = OMX_TRUE;
    OMX_LOGD("kay: * %d.", pCtx->bUseZeroCopyBuffer);
#else
    pCtx->bUseZeroCopyBuffer = OMX_FALSE;
#endif
    OMX_LOGD("kay: ** %d.", pCtx->bUseZeroCopyBuffer);
    pCtx->mGpuAlignStride = 16;

    OmxCreateMutex(&pCtx->awMutexHandler, OMX_TRUE);
    pCtx->mSemInData    = OmxCreateSem("InDataSem",    0, 0, OMX_FALSE);
    pCtx->mSemOutBuffer = OmxCreateSem("OutBufferSem", 0, 0, OMX_FALSE);
    pCtx->mSemValidPic  = OmxCreateSem("ValidPicSem",  0, 0, OMX_FALSE);

    pCtx->base.ops = &mAwDecoderOps;

    return (OmxDecoder*)&pCtx->base;
}

void OmxDestroyDecoder(OmxDecoder* pDec)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;

    if(pCtx->m_decoder != NULL)
    {
        DestroyVideoDecoder(pCtx->m_decoder);
        pCtx->m_decoder = NULL;
    }

    OmxDestroyMutex(&pCtx->awMutexHandler);
    OmxDestroySem(&pCtx->mSemInData);
    OmxDestroySem(&pCtx->mSemOutBuffer);
    OmxDestroySem(&pCtx->mSemValidPic);

    if(pCtx->mStreamInfo.pCodecSpecificData)
        free(pCtx->mStreamInfo.pCodecSpecificData);

    if(pCtx->decMemOps)
    {
        CdcMemClose(pCtx->decMemOps);
    }
    free(pCtx);
    pCtx = NULL;
}
