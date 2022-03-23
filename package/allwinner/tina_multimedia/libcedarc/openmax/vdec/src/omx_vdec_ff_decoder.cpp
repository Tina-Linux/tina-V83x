/*
 * =====================================================================================
 *
 *       Filename:  omx_vdec_ff_decoder.cpp
 *
 *    Description: 1. soft decode with ffmpeg decoder
 *                        2. ffmpeg base on ffmpeg-3.3.8
 *                        3. ndk base on r15c
 *                        4.  cpp's required because the android namespace
 *
 *    Instructions: 1. use ffmpeg source code to make  so and copy the includes, sos to a folder
 *                        2. create a Android.mk or Makefile to release the so to sys in the folder
 *                        3. modify the cedarc/libcdclist.mk or Makefile ,configure
 *                        4. modify the vdec/Android.mk or Makefile to add includes and so
 *
 *        Version:  1.0
 *        Created:  08/02/2018 04:18:11 PM
 *       Revision:  none
 *
 *         Author:  Gan Qiuye
 *        Company: allwinnerterch.com
 *
 * =====================================================================================
 */
#define LOG_TAG "omx_vdec_ff"
#include "log.h"
#include "omx_vdec_decoder.h"
#include "omx_vdec_port.h"
#include "omx_vdec_config.h"
#include "OMX_Video.h"
#include "omx_mutex.h"
#include "omx_sem.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libavutil/mem.h"
#include "libavutil/imgutils.h"
}



#ifdef __ANDROID__
#include <hardware/hal_public.h>
#include <binder/IPCThreadState.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/Rect.h>
#include <HardwareAPI.h>


using namespace android;


#if (defined CONF_KERNEL_VERSION_3_10) || \
    (defined CONF_KERNEL_VERSION_4_4) || \
    (defined CONF_KERNEL_VERSION_4_9)
    typedef ion_user_handle_t ion_handle_abstract_t;
    #define ION_NULL_VALUE (0)
#else
    typedef struct ion_handle* ion_handle_abstract_t;
    #define ION_NULL_VALUE (NULL)
#endif
#endif


typedef struct OmxFFDecoderContext
{
    OmxDecoder             base;
    AVCodec*               m_decoder;
    AVCodecContext*        pDecodecCtx;
    AVFormatContext*       pformat;
    OMX_BOOL               bInputEosFlag;
    DecoderCallback        callback;
    void*                  pUserData;
    OMX_CONFIG_RECTTYPE    mVideoRect; //* for display crop
    OMX_S32                mGpuAlignStride;

    OMX_U32                mMaxWidth;
    OMX_U32                mMaxHeight;
    OMX_BOOL               bUseAndroidBuffer;
    OMX_BOOL               bSurpporNativeHandle;
    OMX_U32                mCodecSpecificDataLen;
    OMX_U8                 mCodecSpecificData[CODEC_SPECIFIC_DATA_LENGTH];
    uint8_t*               video_dst_data[4];
    int                    video_dst_linesize[4];
    AVFrame*               pFrame;
    AwOmxVdecPort*         m_inPort;
    AwOmxVdecPort*         m_outPort;
    OMX_BOOL               bFrameDone;
    OMX_BOOL               bValidPic;
    OMX_BOOL               bValidPacket;
    OMX_BOOL               bFirstData;
    OMX_BOOL               bHandleExtData;
    AVPacket               codedPkt;
    OmxMutexHandle         ffMutex;
    OmxSemHandle           mSemDrainDone;
    //OmxSemHandle           mSemDecodeDone;
    //OmxSemHandle           mSemSubmitDone;
    OMX_U32                nSavePicNum;

}OmxFFDecoderContext;

#if SAVE_PICTURE
static void savePicture(OmxFFDecoderContext *pCtx)
{
    pCtx->nSavePicNum++;
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->m_outPort);
    if(pCtx->nSavePicNum <= 30)
    {
        char  path[1024] = {0};
        FILE* fpStream   = NULL;

        sprintf (path,"/data/camera/pic%lu.dat",pCtx->nSavePicNum);
        fpStream = fopen(path, "wb");

        OMX_U32 len = (outDef->format.video.nFrameWidth*
            outDef->format.video.nFrameHeight)*3/2;
        if(fpStream != NULL)
        {
            fwrite(pCtx->pFrame->data[0],1,len, fpStream);
            fclose(fpStream);
        }
        else
        {
            logd("++the fpStream is null when save picture");
        }
    }
    return;
}
#endif

static enum AVCodecID getFFmpegCodecId(OMX_VIDEO_CODINGTYPE videoCoding)
{
    switch(videoCoding)
    {
        case OMX_VIDEO_CodingAVC:
            return AV_CODEC_ID_H264;
        case OMX_VIDEO_CodingMPEG2:
            return AV_CODEC_ID_MPEG2VIDEO;
        case OMX_VIDEO_CodingH263:
            return AV_CODEC_ID_H263;
        case OMX_VIDEO_CodingMPEG4:
            return AV_CODEC_ID_MPEG4;
        case OMX_VIDEO_CodingWMV:
            return AV_CODEC_ID_WMAV2;
        case OMX_VIDEO_CodingMJPEG:
            return AV_CODEC_ID_MJPEG;
        case OMX_VIDEO_CodingVP8:
            return AV_CODEC_ID_VP8;
        case OMX_VIDEO_CodingHEVC:
            return AV_CODEC_ID_HEVC;
        default:
        {
            loge("unsupported OMX this format:%d", videoCoding);
            break;
        }
    }
    return AV_CODEC_ID_NONE;
}


static int handleExtradata(OmxFFDecoderContext *pCtx, OMX_BUFFERHEADERTYPE* pInBufHdr)
{
    if(pInBufHdr->nFlags & OMX_BUFFERFLAG_CODECCONFIG)
    {
        OMX_ASSERT((pInBufHdr->nFilledLen + pCtx->mCodecSpecificDataLen) <=
                CODEC_SPECIFIC_DATA_LENGTH);
        //omxAddNaluStartCodePrefix(pCtx, pInBufHdr);
        OMX_U8* pInBuffer = pInBufHdr->pBuffer;
        //if(pSelf->mIsSecureVideoFlag == OMX_TRUE)
        memcpy(pCtx->mCodecSpecificData + pCtx->mCodecSpecificDataLen,
               pInBuffer,
               pInBufHdr->nFilledLen);

        pCtx->mCodecSpecificDataLen += pInBufHdr->nFilledLen;
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_EMPTY_BUFFER_DONE,
                        (void*)pInBufHdr);
    }
    else
    {
        logd("++++++++++++++++pCtx->mCodecSpecificDataLen[%d]",
            (int)pCtx->mCodecSpecificDataLen);
        if(pCtx->mCodecSpecificDataLen > 0)
        {
            if(pCtx->pDecodecCtx->extradata)
                av_free(pCtx->pDecodecCtx->extradata);
            pCtx->pDecodecCtx->extradata_size = pCtx->mCodecSpecificDataLen;
            pCtx->pDecodecCtx->extradata = (uint8_t *)av_realloc(pCtx->pDecodecCtx->extradata,
            pCtx->pDecodecCtx->extradata_size);

            memset(pCtx->pDecodecCtx->extradata, 0, pCtx->mCodecSpecificDataLen);
            memcpy(pCtx->pDecodecCtx->extradata,
                pCtx->mCodecSpecificData, pCtx->mCodecSpecificDataLen);
        }
        else
        {
            pCtx->pDecodecCtx->extradata      = NULL;
            pCtx->pDecodecCtx->extradata_size = 0;
        }
        pCtx->bHandleExtData = OMX_TRUE;
        return 0;
    }
    return -1;
}

static void copyPictureDataToAndroidBuffer(OmxFFDecoderContext *pCtx,
                                              OMX_BUFFERHEADERTYPE* pOutBufHdr)
{
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->m_outPort);
#ifdef __ANDROID__
    void* dst;
    buffer_handle_t         pBufferHandle = NULL;
    android::GraphicBufferMapper &mapper = android::GraphicBufferMapper::get();
    //*when lock gui buffer, we must according gui buffer's width and height, not by decoder!
    android::Rect bounds(outDef->format.video.nFrameWidth,
                         outDef->format.video.nFrameHeight);

    pBufferHandle = (buffer_handle_t)pOutBufHdr->pBuffer;
    if(0 != mapper.lock(pBufferHandle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &dst))
    {
        logw("Lock GUIBuf fail!");
    }

    OMX_U32 yPlaneSz = outDef->format.video.nFrameWidth *
                       outDef->format.video.nFrameHeight;
    OMX_U32 uvPlaneSz = yPlaneSz /4;
#if 0
    OMX_U32 wAlign = OmxAlign(outDef->format.video.nFrameWidth, pCtx->mGpuAlignStride);
    OMX_U32 hAlign = OmxAlign(outDef->format.video.nFrameHeight, pCtx->mGpuAlignStride);
    OMX_U32 yPlanSzAlign = wAlign * hAlign;
    OMX_U32 uvPlaneSzAlign = yPlanSzAlign/4;
#endif
    //the ffmpeg default color format is AV_PIX_FMT_YUV420P
    memcpy(dst, pCtx->pFrame->data[0], yPlaneSz);//Y
    dst = (void*)((unsigned char*)dst + yPlaneSz);
    memcpy(dst, pCtx->pFrame->data[2], uvPlaneSz);//V
    dst = (void*)((unsigned char*)dst + uvPlaneSz);
    memcpy(dst, pCtx->pFrame->data[1], uvPlaneSz);//U

#endif
}

static int __ffCallback(OmxDecoder* pDec, DecoderCallback callback, void* pUserData)
{
    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;
    pCtx->callback  = callback;
    pCtx->pUserData = pUserData;
    return 0;
}

static int __ffPrepare(OmxDecoder* pDec)
{
    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;
    logd("Prepare decoder begin!");
    av_register_all();
    enum AVCodecID codec_id = getFFmpegCodecId(pCtx->m_inPort->m_sPortFormatType.eCompressionFormat);
    pCtx->m_decoder = avcodec_find_decoder(codec_id);
    if(!pCtx->m_decoder)
    {
        loge("can not find the decoder!");
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_EVENT_ERROR, NULL);
        return -1;
    }
    pCtx->pDecodecCtx = avcodec_alloc_context3(pCtx->m_decoder);
    if(!pCtx->pDecodecCtx)
    {
        loge("failed to allocate the context!");
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_EVENT_ERROR, NULL);
        return -1;
    }
    if(avcodec_open2(pCtx->pDecodecCtx, pCtx->m_decoder, NULL) < 0)
    {
        loge("can not open decoder!");
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_EVENT_ERROR, NULL);
        return -1;
    }

    pCtx->bFirstData   = OMX_TRUE;
    pCtx->pFrame = av_frame_alloc();
    return 0;
}

static int __ffSubmit(OmxDecoder* pDec, OMX_BUFFERHEADERTYPE* pInBufHdr)
{
    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;

    if(!pCtx->bHandleExtData && handleExtradata(pCtx, pInBufHdr) == -1)
        return 0;
    if(pCtx->bFirstData || pCtx->bFrameDone)
    {
        OmxAcquireMutex(pCtx->ffMutex);
        av_init_packet(&pCtx->codedPkt);
        if(pInBufHdr->pBuffer)
        {
            pCtx->codedPkt.data = pInBufHdr->pBuffer;
            pCtx->codedPkt.size = pInBufHdr->nFilledLen;
            pCtx->codedPkt.pts  = pInBufHdr->nTimeStamp;
            pCtx->codedPkt.dts  = pInBufHdr->nTimeStamp;
            pCtx->codedPkt.stream_index = 0;//??
            pCtx->codedPkt.flags = AV_PKT_FLAG_KEY;//??
            //pCtx->codedPkt.duration =
            //pCtx->codedPkt.pos =
        }
        if(avcodec_send_packet(pCtx->pDecodecCtx, &pCtx->codedPkt) < 0)
        {
            logw("gqy*****send packet failed");
            OmxReleaseMutex(pCtx->ffMutex);
            return -1;
        }
        pCtx->bValidPacket = OMX_TRUE;
        pCtx->bFirstData   = OMX_FALSE;
        pCtx->bFrameDone   = OMX_FALSE;
#if ENABLE_SAVE_ES_DATA
        FILE *fp = fopen("/data/camera/ff_es_data.raw", "a+");
        fwrite(pCtx->codedPkt.data, 1, pCtx->codedPkt.size, fp);
        fclose(fp);
#endif
        //OmxTryPostSem(pCtx->mSemSubmitDone);
        OmxReleaseMutex(pCtx->ffMutex);
        return 0;
    }
    else
    {
        usleep(2000);
        return -1;
    }

}

static inline void __ffDecode(OmxDecoder* pDec)
{
    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;
    if(pCtx->bValidPacket)
    {
        OmxAcquireMutex(pCtx->ffMutex);
        int ret = avcodec_receive_frame(pCtx->pDecodecCtx, pCtx->pFrame);
        OmxReleaseMutex(pCtx->ffMutex);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            logd("current pkt decode success!!");
            pCtx->bFrameDone = OMX_TRUE;
            pCtx->bValidPacket = OMX_FALSE;
            //OmxTryPostSem(pCtx->mSemDecodeDone);
        }
        else if (ret < 0) {
            loge("Error during decoding");
            pCtx->bFrameDone = OMX_TRUE;
            pCtx->bValidPacket = OMX_FALSE;
            //OmxTryPostSem(pCtx->mSemDecodeDone);
        }
        else{
            logd("saving frame %3d\n", pCtx->pDecodecCtx->frame_number);
            pCtx->bFrameDone = OMX_FALSE;
            pCtx->bValidPic  = OMX_TRUE;
            OmxTimedWaitSem(pCtx->mSemDrainDone, -1);
        }
    }
    else
    {
        usleep(2000);
    }
    return;
}

static OMX_BUFFERHEADERTYPE* __ffDrain(OmxDecoder* pDec)
{
    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->m_outPort);

    if(pCtx->bValidPic)
    {
        OMX_BUFFERHEADERTYPE* pOutBufHdr = doRequestPortBuffer(pCtx->m_outPort);
        OmxAcquireMutex(pCtx->ffMutex);
        pCtx->pFrame->pts = av_frame_get_best_effort_timestamp(pCtx->pFrame);
        if(!pCtx->bUseAndroidBuffer)
            pOutBufHdr->pBuffer = (OMX_U8*)av_frame_clone(pCtx->pFrame);
        else
            copyPictureDataToAndroidBuffer(pCtx, pOutBufHdr);
        logd("gqy*****fmt:%d, w:%d, h:%d, pts:%lld",
            pCtx->pDecodecCtx->pix_fmt,
            pCtx->pDecodecCtx->width,
            pCtx->pDecodecCtx->height,
            pCtx->pFrame->pts);
        #if ENABLE_SAVE_PICTURE
        savePicture(pCtx);
        #endif

        pOutBufHdr->nTimeStamp = pCtx->pFrame->pts;
        pOutBufHdr->nOffset    = 0;

        pOutBufHdr->nFilledLen = (outDef->format.video.nFrameWidth *
                                  outDef->format.video.nFrameHeight) * 3 / 2;
        pCtx->bValidPic  = OMX_FALSE;
        OmxReleaseMutex(pCtx->ffMutex);

        OmxTryPostSem(pCtx->mSemDrainDone);
        return pOutBufHdr;
    }
    else
    {
        usleep(2000);
        return NULL;
    }

}

static int __ffReturnBuffer(OmxDecoder* pDec)
{
    CEDARC_UNUSE(pDec);
    return 0;
}
static inline void __ffFlush(OmxDecoder* pDec)
{
    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;

    //OmxTryPostSem(pCtx->mSemSubmitDone);
    //OmxTryPostSem(pCtx->mSemDecodeDone);
    OmxTryPostSem(pCtx->mSemDrainDone);
    OmxAcquireMutex(pCtx->ffMutex);
    avcodec_flush_buffers(pCtx->pDecodecCtx);
    pCtx->bFrameDone   = OMX_FALSE;
    pCtx->bValidPic    = OMX_FALSE;
    pCtx->bValidPacket = OMX_FALSE;
    pCtx->bFirstData   = OMX_TRUE;
    OmxReleaseMutex(pCtx->ffMutex);
}

static  void __ffClose(OmxDecoder* pDec)
{
    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;

    //OmxTryPostSem(pCtx->mSemSubmitDone);
    //OmxTryPostSem(pCtx->mSemDecodeDone);
    OmxTryPostSem(pCtx->mSemDrainDone);
    OmxAcquireMutex(pCtx->ffMutex);

    if(pCtx->pDecodecCtx)
    {
        if(pCtx->pDecodecCtx->extradata)
            av_free(pCtx->pDecodecCtx->extradata);
        avcodec_close(pCtx->pDecodecCtx);
        avcodec_free_context(&pCtx->pDecodecCtx);
    }

    pCtx->pDecodecCtx  = NULL;
    av_frame_free(&pCtx->pFrame);
    pCtx->pFrame       = NULL;
    pCtx->bFrameDone   = OMX_FALSE;
    pCtx->bValidPic    = OMX_FALSE;
    pCtx->bValidPacket = OMX_FALSE;
    pCtx->bFirstData   = OMX_TRUE;
    OmxReleaseMutex(pCtx->ffMutex);
}

static OMX_ERRORTYPE __ffGetExtPara(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eParamIndex,
                                       OMX_PTR pParamData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    CEDARC_UNUSE(pDec);
    CEDARC_UNUSE(pParamData);
    CEDARC_UNUSE(eParamIndex);
    if(eParamIndex  == AWOMX_IndexParamVideoGetAndroidNativeBufferUsage)
    {
        logd(" COMPONENT_GET_PARAMETER: AWOMX_IndexParamVideoGetAndroidNativeBufferUsage");
    }
    else
    {
        logw("get_parameter: unknown param %08x\n", eParamIndex);
        eError = OMX_ErrorUnsupportedIndex;
    }
    return eError;
}

static OMX_ERRORTYPE __ffSetExtPara(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eParamIndex,
                                       OMX_PTR pParamData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;
    switch(eParamIndex)
    {
        case AWOMX_IndexParamVideoUseAndroidNativeBuffer2:
        {
            logv(" COMPONENT_SET_PARAMETER: AWOMX_IndexParamVideoUseAndroidNativeBuffer2");
            logv("set_parameter: AWOMX_IndexParamVideoUseAndroidNativeBuffer2, do nothing.\n");
            pCtx->bUseAndroidBuffer = OMX_TRUE;
            break;
        }
        case AWOMX_IndexParamVideoEnableAndroidNativeBuffers:
        {
            logv(" COMPONENT_SET_PARAMETER: AWOMX_IndexParamVideoEnableAndroidNativeBuffers");
            logv("set_parameter: AWOMX_IndexParamVideoEnableAndroidNativeBuffers,\
                  set m_useAndroidBuffer to OMX_TRUE\n");

            EnableAndroidNativeBuffersParams *EnableAndroidBufferParams
                =  (EnableAndroidNativeBuffersParams*) pParamData;
            logv(" enbleParam = %d\n",EnableAndroidBufferParams->enable);
            if(1==EnableAndroidBufferParams->enable)
            {
                pCtx->bUseAndroidBuffer = OMX_TRUE;
            }
            break;
        }
        case AWOMX_IndexParamVideoAllocateNativeHandle:
        {
            logv(" COMPONENT_SET_PARAMETER: AWOMX_IndexParamVideoAllocateNativeHandle");

            EnableAndroidNativeBuffersParams *EnableAndroidBufferParams
                =  (EnableAndroidNativeBuffersParams*) pParamData;
            logv(" enbleParam = %d\n",EnableAndroidBufferParams->enable);
            if(1==EnableAndroidBufferParams->enable)
            {
                logd("set bSurpporNativeHandle to OMX_TRUE");
                pCtx->bSurpporNativeHandle = OMX_TRUE;
            }
            break;
        }

#ifdef CONF_KITKAT_AND_NEWER
        case AWOMX_IndexParamVideoUsePrepareForAdaptivePlayback:
        {
            logv(" COMPONENT_SET_PARAMETER: \
                  AWOMX_IndexParamVideoUsePrepareForAdaptivePlayback");

            PrepareForAdaptivePlaybackParams *pPlaybackParams;
            pPlaybackParams = (PrepareForAdaptivePlaybackParams *)pParamData;

            if(pPlaybackParams->nPortIndex==1 && pPlaybackParams->bEnable)
            {
                logv("set adaptive playback ,maxWidth = %d, maxHeight = %d",
                       (int)pPlaybackParams->nMaxFrameWidth,
                       (int)pPlaybackParams->nMaxFrameHeight);

                pCtx->mMaxWidth  = pPlaybackParams->nMaxFrameWidth;
                pCtx->mMaxHeight = pPlaybackParams->nMaxFrameHeight;
            }
            break;
        }
#endif
        default:
        {
            logv("Setparameter: unknown param %d\n", eParamIndex);
            eError = OMX_ErrorUnsupportedIndex;
            break;
        }
    }
    return eError;
}

static OMX_ERRORTYPE __ffGetExtConfig(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eConfigIndex,
                                       OMX_PTR pConfigData)
{
    CEDARC_UNUSE(pDec);
    CEDARC_UNUSE(pConfigData);
    logw("Setparameter: unknown param %d\n", eConfigIndex);
    return OMX_ErrorUnsupportedIndex;
}

static OMX_ERRORTYPE __ffSetExtConfig(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eConfigIndex,
                                       OMX_PTR pConfigData)
{
    CEDARC_UNUSE(pDec);
    CEDARC_UNUSE(pConfigData);
    logw("Setparameter: unknown param %d\n", eConfigIndex);
    return OMX_ErrorUnsupportedIndex;
}

static void __ffGetFormat(OmxDecoder* pDec)
{
    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;
#ifdef __ANDROID__
    setPortColorFormat(pCtx->m_outPort, HAL_PIXEL_FORMAT_YV12);
#endif
}

static inline void __ffSetExtBufNum(OmxDecoder* pDec, OMX_S32 num)
{
    CEDARC_UNUSE(pDec);
    CEDARC_UNUSE(num);

}
static void __ffStandbyBuffer(OmxDecoder* pDec)
{
    CEDARC_UNUSE(pDec);
}

static void __ffSetInputEos(OmxDecoder* pDec, OMX_BOOL bEos)
{
    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;
    pCtx->bInputEosFlag = bEos;
}

static int __ffSetOutputEos(OmxDecoder* pDec)
{
    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;
    while (getPortValidSize(pCtx->m_outPort) > 0)
    {
        OMX_BUFFERHEADERTYPE*   pOutBufHdr  = doRequestPortBuffer(pCtx->m_outPort);

        if(pOutBufHdr==NULL)
            continue;

        if (pOutBufHdr->nFilledLen != 0)
        {
            pCtx->callback(pCtx->pUserData, AW_OMX_CB_FILL_BUFFER_DONE, (void*)pOutBufHdr);
            pOutBufHdr = NULL;
        }
        else
        {
            logd("++++ set output eos(normal)");
            pOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
            pCtx->callback(pCtx->pUserData, AW_OMX_CB_FILL_BUFFER_DONE, (void*)pOutBufHdr);
            pOutBufHdr = NULL;
            break;
        }
    }
    return 0;
}

static OMX_U8* __ffAllocatePortBuffer(OmxDecoder* pDec, AwOmxVdecPort* port, OMX_U32 size)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    CEDARC_UNUSE(port);
    OMX_U8* pBuffer = (OMX_U8*)malloc(size);
    return pBuffer;
}

static void __ffFreePortBuffer(OmxDecoder* pDec, AwOmxVdecPort* port, OMX_U8* pBuffer)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    CEDARC_UNUSE(port);
    free(pBuffer);
    return ;
}


static OmxDecoderOpsT mFFDecoderOps =
{
    .getExtPara   =   __ffGetExtPara,
    .setExtPara   =   __ffSetExtPara,
    .getExtConfig =   __ffGetExtConfig,
    .setExtConfig =   __ffSetExtConfig,
    .prepare      =   __ffPrepare,
    .close        =   __ffClose,
    .flush        =   __ffFlush,
    .decode       =   __ffDecode,
    .submit       =   __ffSubmit,
    .drain        =   __ffDrain,
    .callback     =   __ffCallback,
    .setInputEos  =   __ffSetInputEos,
    .setOutputEos =   __ffSetOutputEos,
    .standbyBuf   =   __ffStandbyBuffer,
    .returnBuf    =   __ffReturnBuffer,
    .getFormat    =   __ffGetFormat,
    .setExtBufNum =   __ffSetExtBufNum,
    .allocPortBuf =   __ffAllocatePortBuffer,
    .freePortBuf  =   __ffFreePortBuffer,
};

OmxDecoder* OmxDecoderCreate(AwOmxVdecPort* in, AwOmxVdecPort* out)
{
    logv("OmxDecoderCreate.");
    OmxFFDecoderContext* pCtx;
    pCtx = (OmxFFDecoderContext*)malloc(sizeof(OmxFFDecoderContext));
    if(pCtx == NULL)     {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(pCtx, 0, sizeof(OmxFFDecoderContext));
    OmxCreateMutex(&pCtx->ffMutex, OMX_FALSE);
    pCtx->mSemDrainDone  = OmxCreateSem("DrainDoneSem", 0, 0, OMX_FALSE);
    //pCtx->mSemDecodeDone = OmxCreateSem("DecodeDoneSem",0, 0, OMX_FALSE);
    //pCtx->mSemSubmitDone = OmxCreateSem("SubmitDoneSem",0, 0, OMX_FALSE);
    pCtx->m_inPort  = in;
    pCtx->m_outPort = out;

    pCtx->mGpuAlignStride = 16;
    pCtx->base.ops = &mFFDecoderOps;
    return (OmxDecoder*)&pCtx->base;
}

void OmxDestroyDecoder(OmxDecoder* pDec)
{
    OmxFFDecoderContext *pCtx = (OmxFFDecoderContext*)pDec;
    OmxDestroyMutex(&pCtx->ffMutex);
    OmxDestroySem(&pCtx->mSemDrainDone);
    //OmxDestroySem(&pCtx->mSemDecodeDone);
    //OmxDestroySem(&pCtx->mSemSubmitDone);
    free(pCtx);
    pCtx = NULL;
}
