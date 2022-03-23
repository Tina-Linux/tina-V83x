/*
 * =====================================================================================
 *
 *       Filename:  omx_vdec_aw_decoder_android.cpp
 *
 *    Description: 1. cpp's required because the android namespace ,
 *                        2. hardware decode with AW decoder
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
#include "gralloc_metadata/sunxi_metadata_def.h"
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

#include <ion/ion.h>
#include <linux/ion.h>
#include <cutils/properties.h>
#include <hardware/hal_public.h>
#include <binder/IPCThreadState.h>
#include <media/stagefright/foundation/ADebug.h>
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


typedef struct OMXOutputBufferInfoA {
    ANativeWindowBuffer*  pWindowBuf;
#if (defined CONF_KERNEL_VERSION_3_10) || (defined CONF_KERNEL_VERSION_4_4) \
    || (defined CONF_KERNEL_VERSION_4_9)
               ion_user_handle_t handle_ion;
#else
      struct ion_handle *handle_ion;
#endif     //ion_handle_abstract_t handle_ion;
    OMX_BUFFERHEADERTYPE* pHeadBufInfo;
    char*                 pBufPhyAddr;
    char*                 pBufVirAddr;
    VideoPicture*         pVideoPicture;
    OutBufferStatus       mStatus;
    OMX_BOOL              mUseDecoderBufferToSetEosFlag;
    int                   nBufFd;
    int                   nVirAddrSize;
    void*                 pMetaDataVirAddr;
    int                   nMetaDataVirAddrSize;
    int                   nMetaDataMapFd;
    ion_handle_abstract_t metadata_handle_ion;
 }OMXOutputBufferInfoA;

typedef struct TranBufferInfo
{
    void* pAddr;
    int   nWidth;
    int   nHeight;
}TranBufferInfo;

typedef struct OmxAwDecoderContext
{
    OmxDecoder             base;
    VideoDecoder*          m_decoder;
    VideoStreamInfo        mStreamInfo;
    VConfig                mVideoConfig;
    OMX_S32                mGpuAlignStride;
    struct ScMemOpsS*      decMemOps;
    struct ScMemOpsS*      secureMemOps;
    int                    mIonFd;
    OMX_U32                nPhyOffset;
    OMX_U32                mMaxWidth;
    OMX_U32                mMaxHeight;

    OMX_CONFIG_RECTTYPE    mVideoRect; //* for display crop
#ifdef CONF_PIE_AND_NEWER
    ColorAspects           mColorAspects;
    HDRStaticInfo          mHdrStaticInfo;
#endif
    OMX_U32                mCodecSpecificDataLen;
    OMX_U8                 mCodecSpecificData[CODEC_SPECIFIC_DATA_LENGTH];
    OMXOutputBufferInfoA   mOutputBufferInfoArr[OMX_MAX_VIDEO_BUFFER_NUM];
    DecoderCallback        callback;
    void*                  pUserData;
    OMX_U32                mSetToDecoderBufferNum;
    OMX_U32                mNeedReSetToDecoderBufferNum;
    OMX_U32                mOutBufferNumDecoderNeed;
    OMX_S32                mExtraOutBufferNum;
    OMX_BOOL               bHadGetVideoFbmBufInfoFlag;
    OMX_BOOL               bInputEosFlag;
    OMX_BOOL               bIsSecureVideoFlag;
    OMX_BOOL               bSurpporNativeHandle;
    OMX_BOOL               bStoreOutputMetaDataFlag;
    OMX_BOOL               bUseAndroidBuffer;
    OMX_BOOL               bFlushFlag;
    OMX_BOOL               bUseZeroCopyBuffer;
    OMX_BOOL               bIsFirstGetOffset;
    OMX_U32                nActualColorFormat;
    OMX_U32                nExtendFlag;

    AwOmxVdecPort*         pInPort;
    AwOmxVdecPort*         pOutPort;

    OmxMutexHandle         awMutexHandler;
    OmxSemHandle           mSemOutBuffer;
    OmxSemHandle           mSemInData;
    OmxSemHandle           mSemValidPic;

    VideoPicture*          pPicture;
    OMX_U32                mPicNum;

}OmxAwDecoderContext;

#if ENABLE_SHOW_BUFINFO_STATUS
static void showBufferInfoArrStatus(OmxAwDecoderContext *pCtx)
{
    int i;
    logd("***********show status begin*********");
    for(i = 0; i < OMX_MAX_VIDEO_BUFFER_NUM; i++)
    {
        if(pCtx->mOutputBufferInfoArr[i].mStatus == OWNED_BY_NONE)
        {
            logd("i:%d~%d,pHeadBufInfo:      0x0, pic:       0x0, "
                 "status: OWNED_BY_NONE",
                i, OMX_MAX_VIDEO_BUFFER_NUM - 1);
            break;
        }
        logd("i:%2d, pHeadBufInfo:%10p, pic:%10p, status: %s",
            i,
            pCtx->mOutputBufferInfoArr[i].pHeadBufInfo,
            pCtx->mOutputBufferInfoArr[i].pVideoPicture,
            statusToString(pCtx->mOutputBufferInfoArr[i].mStatus));
    }
    logd("***********show status end***********");
}
#endif

#if(ENABLE_SAVE_PICTURE)
static void savePictureForDebug(OmxAwDecoderContext *pCtx,VideoPicture* pVideoPicture)
{
    pCtx->mPicNum++;
    if(pCtx->mPicNum <= 30)
    {
        char  path[1024] = {0};
        FILE* fpStream   = NULL;
        int   len = 0;
        OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->pOutPort);
        int   width = outDef->format.video.nFrameWidth;
        int   height = outDef->format.video.nFrameHeight;
        logd("save picture: w[%d],h[%d],pVirBuf[%p]",
              width, height, pVideoPicture->pData0);

        sprintf (path,"/data/camera/pic%d.dat",(int)pCtx->mPicNum);
        fpStream = fopen(path, "wb");
        if(pVideoPicture->bEnableAfbcFlag)
            len = pVideoPicture->nAfbcSize;
        else
            len  = (width*height)*3/2;
        if(fpStream != NULL)
        {
            fwrite(pVideoPicture->pData0,1,len, fpStream);
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

static OMX_U32 anGetStreamFormat(OMX_VIDEO_CODINGTYPE videoCoding)
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
            loge("unsupported OMX this format:%d", videoCoding);
            break;
        }
    }
    return VIDEO_CODEC_FORMAT_UNKNOWN;
}

void anGetStremInfo(OmxAwDecoderContext *pCtx)
{
    OMX_PARAM_PORTDEFINITIONTYPE* inDef = getPortDef(pCtx->pInPort);
    OMX_VIDEO_PARAM_PORTFORMATTYPE* inFormatType = \
           getPortFormatType(pCtx->pInPort);

    pCtx->mStreamInfo.eCodecFormat = \
           anGetStreamFormat(inFormatType->eCompressionFormat);
    pCtx->mStreamInfo.nWidth  = inDef->format.video.nFrameWidth;
    pCtx->mStreamInfo.nHeight = inDef->format.video.nFrameHeight;
}

static int anDealWithInitData(OmxAwDecoderContext *pCtx)
{
    OMX_BUFFERHEADERTYPE* pInBufHdr = doRequestPortBuffer(pCtx->pInPort);

    if(pInBufHdr == NULL)
    {
        return -1;
    }

    if(pInBufHdr->nFlags & OMX_BUFFERFLAG_CODECCONFIG)
    {
        OMX_ASSERT((pInBufHdr->nFilledLen + pCtx->mCodecSpecificDataLen) <=
                CODEC_SPECIFIC_DATA_LENGTH);
        //omxAddNaluStartCodePrefix(pCtx, pInBufHdr);
        OMX_U8* pInBuffer = NULL;
        if(pCtx->bIsSecureVideoFlag)
        {
            if(pCtx->bSurpporNativeHandle)
            {
                native_handle_t *nh = (native_handle_t *)pInBufHdr->pBuffer;
                pInBuffer = (OMX_U8*)(uintptr_t)nh->data[0];
            }
            else
                pInBuffer = pInBufHdr->pBuffer;

            if(pCtx->secureMemOps == NULL)
            {
                pCtx->secureMemOps = SecureMemAdapterGetOpsS();
                CdcMemOpen(pCtx->secureMemOps);
            }
            CdcMemCopy(pCtx->secureMemOps,
                       pCtx->mCodecSpecificData + pCtx->mCodecSpecificDataLen,
                       pInBuffer, pInBufHdr->nFilledLen);
        }
        else
        {

            pInBuffer = pInBufHdr->pBuffer;
            memcpy(pCtx->mCodecSpecificData + pCtx->mCodecSpecificDataLen,
                   pInBuffer,
                   pInBufHdr->nFilledLen);
        }
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
            if(pCtx->mStreamInfo.pCodecSpecificData)
                free(pCtx->mStreamInfo.pCodecSpecificData);
            pCtx->mStreamInfo.nCodecSpecificDataLen = \
                pCtx->mCodecSpecificDataLen;
            pCtx->mStreamInfo.pCodecSpecificData    = \
                (char*)malloc(pCtx->mCodecSpecificDataLen);
            memset(pCtx->mStreamInfo.pCodecSpecificData, 0,
                     pCtx->mCodecSpecificDataLen);
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

static int anGetVideoFbmBufInfo(OmxAwDecoderContext *pCtx)
{
    OmxAcquireMutex(pCtx->awMutexHandler);
    FbmBufInfo* pFbmBufInfo =  GetVideoFbmBufInfo(pCtx->m_decoder);
    OmxReleaseMutex(pCtx->awMutexHandler);

    if(pFbmBufInfo != NULL)
    {
        logd("video buffer info: nWidth[%d],nHeight[%d],"
            "nBufferCount[%d],ePixelFormat[%d]",
              pFbmBufInfo->nBufWidth,pFbmBufInfo->nBufHeight,
              pFbmBufInfo->nBufNum,pFbmBufInfo->ePixelFormat);
        logd("video buffer info: nAlignValue[%d],"
             "bProgressiveFlag[%d],bIsSoftDecoderFlag[%d]",
              pFbmBufInfo->nAlignValue,pFbmBufInfo->bProgressiveFlag,
              pFbmBufInfo->bIsSoftDecoderFlag);
        pCtx->nActualColorFormat = (OMX_U32)pFbmBufInfo->ePixelFormat;
        if(pCtx->bUseAndroidBuffer)
        {
            if(PIXEL_FORMAT_YV12 == pFbmBufInfo->ePixelFormat)
                setPortColorFormat(pCtx->pOutPort, HAL_PIXEL_FORMAT_YV12);
            else if(PIXEL_FORMAT_NV21 == pFbmBufInfo->ePixelFormat)
                setPortColorFormat(pCtx->pOutPort, HAL_PIXEL_FORMAT_YCrCb_420_SP);
            //else default OMX_COLOR_FormatYUV420Planar
        }
        if(pFbmBufInfo->nBufNum > (OMX_MAX_VIDEO_BUFFER_NUM - 2))
            pFbmBufInfo->nBufNum = OMX_MAX_VIDEO_BUFFER_NUM - 2;

        pCtx->mOutBufferNumDecoderNeed  = pFbmBufInfo->nBufNum;

        OMX_U32 nCurExtendFlag = 0;

        #ifdef CONF_SURPPORT_METADATA_BUFFER
        if(pFbmBufInfo->b10bitVideoFlag == 1)
            nCurExtendFlag |= AW_VIDEO_10BIT_FLAG;

        if(pFbmBufInfo->bAfbcModeFlag == 1)
            nCurExtendFlag |= AW_VIDEO_AFBC_FLAG;

        if(pFbmBufInfo->bHdrVideoFlag == 1)
            nCurExtendFlag |= AW_VIDEO_HDR_FLAG;
        #endif

        logd("**** nExtendFlag = %lx", nCurExtendFlag);
        OMX_PARAM_PORTDEFINITIONTYPE* inDef  = getPortDef(pCtx->pInPort);
        OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->pOutPort);

        OMX_BOOL bBufferNumMatchFlag = OMX_FALSE;
        //* if init-buffer-num is equal to or more than real-buffer-num,
        //* not callback the event "OMX_EventPortSettingsChanged" to avoid remalloc buffer
        //* which cost some time
        if(outDef->nBufferCountActual >= (OMX_U32)pFbmBufInfo->nBufNum)
            bBufferNumMatchFlag = OMX_TRUE;
        logd("*** bBufferNumMatchFlag = %d",bBufferNumMatchFlag);
        OMX_U32 nInitWidht  =  outDef->format.video.nFrameWidth;
        OMX_U32 nInitHeight =  outDef->format.video.nFrameHeight;
        if(bBufferNumMatchFlag
            && nInitWidht  == (OMX_U32)pFbmBufInfo->nBufWidth
            && nInitHeight == (OMX_U32)pFbmBufInfo->nBufHeight
            && nCurExtendFlag == pCtx->nExtendFlag)
        {
            logd("buffer match!!!!");
            return 0;
        }
        logd("************** port setting change ************");
        logd("****  buffer num: %lu, %d; w&h: %lu, %lu, %d, %d",
              outDef->nBufferCountActual,
              pFbmBufInfo->nBufNum,
              nInitWidht, nInitHeight,
              pFbmBufInfo->nBufWidth,
              pFbmBufInfo->nBufHeight);

        if(inDef->format.video.nFrameWidth   == WIDTH_DEFAULT ||
            inDef->format.video.nFrameHeight == HEIGHT_DEFAULT)
        {
            inDef->format.video.nFrameWidth  = pFbmBufInfo->nBufWidth;
            inDef->format.video.nFrameHeight = pFbmBufInfo->nBufHeight;
        }

        //*ACodec will add the mExtraOutBufferNum to nBufferCountActual,
        //*so we decrease it here
        //*if here not - pCtx->mExtraOutBufferNum, it will keep in omx

        OMX_U32 num = pFbmBufInfo->nBufNum - pCtx->mExtraOutBufferNum;
        outDef->nBufferCountMin    = num;
        outDef->nBufferCountActual = num;
        outDef->format.video.nFrameWidth  = pFbmBufInfo->nBufWidth;
        outDef->format.video.nFrameHeight = pFbmBufInfo->nBufHeight;
        outDef->nBufferSize = pFbmBufInfo->nBufWidth*pFbmBufInfo->nBufHeight*3/2;
        pCtx->nExtendFlag = nCurExtendFlag;

        pCtx->mVideoRect.nLeft   = pFbmBufInfo->nLeftOffset;
        pCtx->mVideoRect.nTop    = pFbmBufInfo->nTopOffset;
        pCtx->mVideoRect.nWidth  = \
                        pFbmBufInfo->nRightOffset - pFbmBufInfo->nLeftOffset;
        pCtx->mVideoRect.nHeight = \
                        pFbmBufInfo->nBottomOffset - pFbmBufInfo->nTopOffset;
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_NOTIFY_RECT, &(pCtx->mVideoRect));
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_PORT_CHANGE, NULL);
    }
    else
    {
        logv("pFbmBufInfo = NULL, m_decoder(%p)", pCtx->m_decoder);
        return -1;
    }

    return 0;
}

static void anReopenVideoEngine(OmxAwDecoderContext *pCtx)
{
    logd("***ReopenVideoEngine!");

    if(pCtx->mStreamInfo.pCodecSpecificData != NULL)
    {
        free(pCtx->mStreamInfo.pCodecSpecificData);
        pCtx->mStreamInfo.pCodecSpecificData    = NULL;
        pCtx->mStreamInfo.nCodecSpecificDataLen = 0;
    }

    OmxAcquireMutex(pCtx->awMutexHandler);
    ReopenVideoEngine(pCtx->m_decoder, &pCtx->mVideoConfig, &(pCtx->mStreamInfo));
    OmxReleaseMutex(pCtx->awMutexHandler);
    pCtx->bHadGetVideoFbmBufInfoFlag = OMX_FALSE;
    return ;
}

static OMX_BOOL isToInitBuffer(OmxAwDecoderContext *pCtx,
                                 OMX_BUFFERHEADERTYPE* pOutBufHdr,
                                 OMX_U32* index)
{
    OMX_U32 i;
    OMX_BOOL ret = OMX_TRUE;
    for(i=0; i< OMX_MAX_VIDEO_BUFFER_NUM; i++)
    {
        OMX_BUFFERHEADERTYPE* tmpBufHdr = \
            pCtx->mOutputBufferInfoArr[i].pHeadBufInfo;
        if(pOutBufHdr == tmpBufHdr)
        {
            ret = OMX_FALSE;
            break;
        }
        if(tmpBufHdr == NULL)
        {
            break;
        }
    }
    //if i>= OMX_MAX_VIDEO_BUFFER_NUM ,
    //something is wrong or the OMX_MAX_VIDEO_BUFFER_NUM is not enough
    *index = i;
    return ret;
}

static int getVirAddrOfMetadataBuffer(OmxAwDecoderContext *pCtx,
                                buffer_handle_t buffer_handle,
                                ion_handle_abstract_t* pHandle_ion,
                                int* pMapfd,
                                int* pVirsize,
                                unsigned char** ppViraddress)
{
    ion_handle_abstract_t handle_ion = ION_NULL_VALUE;
    int nMapfd = -1;
    unsigned char* nViraddress = 0;
    int nVirsize = 0;

#ifdef CONF_SURPPORT_METADATA_BUFFER
    int ret = 0;
    private_handle_t* hnd = (private_handle_t *)(buffer_handle);
    if(hnd != NULL)
    {
        ret = ion_import(pCtx->mIonFd, hnd->metadata_fd, &handle_ion);
        if(ret < 0)
        {
            loge("ion_import fail, maybe the buffer was free by display!");
            return -1;
        }
        nVirsize = hnd->ion_metadata_size;
    }
    else
    {
        logd("the hnd is wrong : hnd = %p", hnd);
        return -1;
    }

    ret = ion_map(pCtx->mIonFd, handle_ion, nVirsize,
            PROT_READ | PROT_WRITE, MAP_SHARED, 0, &nViraddress, &nMapfd);
    if(ret < 0)
    {
        loge("ion_map fail!");
        if(nMapfd >= 0)
            close(nMapfd);
        ion_free(pCtx->mIonFd, handle_ion);
        *ppViraddress = 0;
        *pVirsize = 0;
        *pMapfd = 0;
        *pHandle_ion = 0;
        return -1;
    }
#else
    CEDARC_UNUSE(pCtx);
    CEDARC_UNUSE(buffer_handle);
    CEDARC_UNUSE(pHandle_ion);
    CEDARC_UNUSE(pMapfd);
    CEDARC_UNUSE(pVirsize);
    CEDARC_UNUSE(ppViraddress);
#endif

    *ppViraddress = nViraddress;
    *pVirsize = nVirsize;
    *pMapfd = nMapfd;
    *pHandle_ion = handle_ion;
    return 0;
}

static void setIonMetadataFlag(OmxAwDecoderContext *pCtx,
                                    buffer_handle_t buffer_handle)
{

#ifdef CONF_SURPPORT_METADATA_BUFFER
    private_handle_t* hnd = (private_handle_t *)(buffer_handle);
    hnd->ion_metadata_flag = 0;

    if (pCtx->nExtendFlag & AW_VIDEO_HDR_FLAG)
    {
        hnd->ion_metadata_flag |= SUNXI_METADATA_FLAG_HDR_SATIC_METADATA;
    }

    if (pCtx->nExtendFlag & AW_VIDEO_AFBC_FLAG)
    {
        hnd->ion_metadata_flag |= SUNXI_METADATA_FLAG_AFBC_HEADER;
    }
#else
    CEDARC_UNUSE(pCtx);
    CEDARC_UNUSE(buffer_handle);
#endif
    return ;
}

//* this function just for 2D rotate case.
//* just init alignment buffer to black color.
//* when the display to 2D,the appearance of "Green edges" in ratation if not init.
//* if init the whole buffer, it would take too much time.
int initPartialGpuBuffer(OmxAwDecoderContext *pCtx, char* pDataBuf)
{
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->pOutPort);

    if(pCtx->nActualColorFormat == PIXEL_FORMAT_NV21)
    {
        //* Y
        int nRealHeight = outDef->format.video.nFrameHeight;
        int nInitHeight = pCtx->mGpuAlignStride;
        int nSkipLen = outDef->format.video.nStride*
            (nRealHeight - nInitHeight);
        int nCpyLenY = outDef->format.video.nStride*nInitHeight;
        memset(pDataBuf+nSkipLen, 0x10, nCpyLenY);

        //*UV
        nSkipLen += nCpyLenY;
        nSkipLen += (outDef->format.video.nStride)*
            (nRealHeight/2 - nInitHeight/2);
        int nCpyLenUV = (outDef->format.video.nStride)*(nInitHeight/2);
        memset(pDataBuf+nSkipLen, 0x80, nCpyLenUV);
    }
    else if(pCtx->nActualColorFormat == PIXEL_FORMAT_YV12)
    {
        //* Y
        int nRealHeight = outDef->format.video.nFrameHeight;
        int nInitHeight = pCtx->mGpuAlignStride;
        int nSkipLen = outDef->format.video.nStride*
            (nRealHeight - nInitHeight);
        int nCpyLenY = outDef->format.video.nStride*nInitHeight;
        memset(pDataBuf+nSkipLen, 0x10, nCpyLenY);

        //*U
        nSkipLen += nCpyLenY;
        nSkipLen += (outDef->format.video.nStride/2)*
            (nRealHeight/2 - nInitHeight/2);
        int nCpyLenU = (outDef->format.video.nStride/2)*
            (nInitHeight/2);
        memset(pDataBuf+nSkipLen, 0x80, nCpyLenU);

        //*V
        nSkipLen += nCpyLenU;
        nSkipLen += (outDef->format.video.nStride/2)*
            (nRealHeight/2 - nInitHeight/2);
        int nCpyLenV = (outDef->format.video.nStride/2)*
            (nInitHeight/2);
        memset(pDataBuf+nSkipLen, 0x80, nCpyLenV);
    }

    return 0;
}

/*
 *   use in Android platform,  through allocate buffer by GPU,
 *   which has private_handle_t, API use_buffer
 */
static int initBufferInfo(OmxAwDecoderContext *pCtx,
                                        OMX_BUFFERHEADERTYPE*   pOutBufHdr,
                                        int *nBufFd)
{
    OMX_U32 i;
    void* dst;
    ion_handle_abstract_t handle_ion    = ION_NULL_VALUE;
    uintptr_t             nPhyaddress   = -1;
    //uintptr_t             nTEEaddress   = -1;
    buffer_handle_t       pBufferHandle = NULL;


#ifdef CONF_KITKAT_AND_NEWER
    if(pCtx->bStoreOutputMetaDataFlag)
    {
        VideoDecoderOutputMetaData *pMetaData
            = (VideoDecoderOutputMetaData*)pOutBufHdr->pBuffer;
        pBufferHandle = pMetaData->pHandle;
    }
    else
    {
        pBufferHandle = (buffer_handle_t)pOutBufHdr->pBuffer;
    }
#else
    pBufferHandle = (buffer_handle_t)pOutBufHdr->pBuffer;
#endif


    //for mali GPU
#if defined(CONF_MALI_GPU)
    private_handle_t* hnd = (private_handle_t *)(pBufferHandle);
    if(hnd != NULL)
    {
        ion_import(pCtx->mIonFd, hnd->share_fd, &handle_ion);
    }
    else
    {
        loge("the hnd is wrong : hnd = %p",hnd);
        return -1;
    }
#elif defined(CONF_IMG_GPU)
    IMG_native_handle_t* hnd = (IMG_native_handle_t*)(pBufferHandle);
    if(hnd != NULL)
    {
        ion_import(pCtx->mIonFd, hnd->fd[0], &handle_ion);
    }
    else
    {
        loge("the hnd is wrong : hnd = %p",hnd);
        return -1;
    }
#else
#error invalid GPU type config
#endif

    for(i = 0;i<getPortAllocSize(pCtx->pOutPort);i++)
    {
        if(pCtx->mOutputBufferInfoArr[i].pHeadBufInfo == NULL)
            break;
    }

    if(pCtx->mIonFd > 0)
    {
        *nBufFd = CdcIonGetFd(pCtx->mIonFd, (uintptr_t)handle_ion);
        if(*nBufFd < 0)
        {
            loge("get ion_buffer fd error\n");
            return -1;
        }
        nPhyaddress = CdcIonGetPhyAdr(pCtx->mIonFd, (uintptr_t)handle_ion);
        //if(pCtx->bIsSecureVideoFlag)
        //    nTEEaddress = CdcIonGetTEEAdr(pCtx->mIonFd, (uintptr_t)handle_ion);
    }
    else
    {
        logd("the ion fd is wrong : fd = %d",(int)pCtx->mIonFd);
        return -1;
    }

    nPhyaddress -= pCtx->nPhyOffset;

  //  if(!pCtx->bIsSecureVideoFlag)
    {
        CdcIonMmap(*nBufFd,pOutBufHdr->nAllocLen,(unsigned char **)&dst);

        if(pCtx->mStreamInfo.eCodecFormat == VIDEO_CODEC_FORMAT_H265)
        {
            initPartialGpuBuffer(pCtx,(char*)dst);
        }
    }

    ion_handle_abstract_t metaData_handle_ion = ION_NULL_VALUE;
    int mMetaDataMapFd = -1;
    int mMetaDataVirSize = 0;
    unsigned char* pMetaDataVirAddr = NULL;

    getVirAddrOfMetadataBuffer(pCtx, pBufferHandle, &metaData_handle_ion,
                               &mMetaDataMapFd, & mMetaDataVirSize,
                               &pMetaDataVirAddr);

    setIonMetadataFlag(pCtx, pBufferHandle);

    pCtx->mOutputBufferInfoArr[i].nVirAddrSize = pOutBufHdr->nAllocLen;
    pCtx->mOutputBufferInfoArr[i].handle_ion   = handle_ion;
    pCtx->mOutputBufferInfoArr[i].nBufFd   = *nBufFd;
    //if(!pCtx->bIsSecureVideoFlag)
        pCtx->mOutputBufferInfoArr[i].pBufVirAddr  = (char*)dst;
   // else
   //     pCtx->mOutputBufferInfoArr[i].pBufVirAddr  = (char*)nTEEaddress;
    pCtx->mOutputBufferInfoArr[i].pBufPhyAddr  = (char*)nPhyaddress;
    pCtx->mOutputBufferInfoArr[i].pHeadBufInfo = pOutBufHdr;
    pCtx->mOutputBufferInfoArr[i].pMetaDataVirAddr     = pMetaDataVirAddr;
    pCtx->mOutputBufferInfoArr[i].nMetaDataVirAddrSize = mMetaDataVirSize;
    pCtx->mOutputBufferInfoArr[i].nMetaDataMapFd       = mMetaDataMapFd;
    pCtx->mOutputBufferInfoArr[i].metadata_handle_ion  = metaData_handle_ion;
    return 0;
}

static int requestOutputBuffer(OmxAwDecoderContext *pCtx,
                                    VideoPicture* pPicBufInfo,
                                    OMX_BUFFERHEADERTYPE*   pOutBufHdr,
                                    OMX_BOOL bInitBufferFlag)
{
    OMX_U32 i;
    int mYsize;
    int nBufFd;
    int ret = 0;
    if(pOutBufHdr == NULL)
    {
        loge(" the pOutBufHdr is null when request OutPut Buffer");
        return -1;
    }

    //* init the buffer
    if(bInitBufferFlag)
    {
        ret = initBufferInfo(pCtx, pOutBufHdr, &nBufFd);
        if(ret == -1)
            return ret;
    }
    OMX_U32 num = getPortAllocSize(pCtx->pOutPort);
    for(i = 0;i<num ;i++)
    {
        if(pCtx->mOutputBufferInfoArr[i].pHeadBufInfo == pOutBufHdr)
            break;
    }

    OMX_ASSERT(i!=num);

    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->pOutPort);

    //* set the buffer address
    mYsize = outDef->format.video.nFrameWidth * outDef->format.video.nFrameHeight;
    pPicBufInfo->pData0      = pCtx->mOutputBufferInfoArr[i].pBufVirAddr;
    pPicBufInfo->pData1      = pPicBufInfo->pData0 + mYsize;
    pPicBufInfo->phyYBufAddr = \
        (uintptr_t)pCtx->mOutputBufferInfoArr[i].pBufPhyAddr;
    pPicBufInfo->phyCBufAddr = pPicBufInfo->phyYBufAddr + mYsize;
    pPicBufInfo->nBufId      = i;
    pPicBufInfo->pPrivate    = \
        (void*)(uintptr_t)pCtx->mOutputBufferInfoArr[i].handle_ion;
    pPicBufInfo->nBufFd      =  pCtx->mOutputBufferInfoArr[i].nBufFd;
    pPicBufInfo->nWidth      =  outDef->format.video.nFrameWidth;
    pPicBufInfo->nHeight     =  outDef->format.video.nFrameHeight;
    pPicBufInfo->nLineStride =  outDef->format.video.nFrameWidth;
    pPicBufInfo->pMetaData   =  pCtx->mOutputBufferInfoArr[i].pMetaDataVirAddr;

    pCtx->mOutputBufferInfoArr[i].mStatus = OWNED_BY_DECODER;

    return 0;
}

static int returnInitBuffer(OmxAwDecoderContext *pCtx,
                            OMX_BUFFERHEADERTYPE* pOutBufHdr,
                            OMX_U32 index)
{
    int mRet = -1;
    VideoPicture mVideoPicture;
    VideoPicture* pVideoPicture = NULL;
    memset(&mVideoPicture, 0, sizeof(VideoPicture));
    if(pCtx->mSetToDecoderBufferNum >= pCtx->mOutBufferNumDecoderNeed)
    {
        pCtx->mOutputBufferInfoArr[index].pHeadBufInfo = pOutBufHdr;
        pCtx->mOutputBufferInfoArr[index].mStatus      = OWNED_BY_US;
#if ENABLE_SHOW_BUFINFO_STATUS
        showBufferInfoArrStatus(pCtx);
#endif
        return 0;
    }
    else
    {
        mRet = requestOutputBuffer(pCtx, &mVideoPicture, pOutBufHdr, OMX_TRUE);
        if(mRet == 0)
        {
            pCtx->mSetToDecoderBufferNum++;
            if(pCtx->mNeedReSetToDecoderBufferNum > 0)
            {
                if(RequestReleasePicture(pCtx->m_decoder) != NULL)
                {
                    pCtx->mNeedReSetToDecoderBufferNum--;
                    pVideoPicture = ReturnRelasePicture(pCtx->m_decoder,
                                                        &mVideoPicture, 0);
                }
                else
                {
                    logw("** can not reqeust release picture");
                    return -1;
                }
            }
            else
            {
                pVideoPicture = SetVideoFbmBufAddress(pCtx->m_decoder,
                                                      &mVideoPicture, 0);
                logd("*** call SetVideoFbmBufAddress: pVideoPicture(%p)",
                     pVideoPicture);
            }
            OmxTryPostSem(pCtx->mSemOutBuffer);
            pCtx->mOutputBufferInfoArr[mVideoPicture.nBufId].pVideoPicture = pVideoPicture;

#if ENABLE_SHOW_BUFINFO_STATUS
            showBufferInfoArrStatus(pCtx);
#endif
            return 0;
        }
        else
            return mRet;
    }
}

static int returnArrangedBuffer(OmxAwDecoderContext *pCtx,
                                OMX_BUFFERHEADERTYPE* pOutBufHdr,
                                OMX_U32 index)
{
    int mRet = -1;
    VideoPicture mVideoPicture;
    VideoPicture* pVideoPicture = NULL;
    memset(&mVideoPicture, 0, sizeof(VideoPicture));

    mRet = requestOutputBuffer(pCtx, &mVideoPicture, pOutBufHdr, OMX_FALSE);
    if(mRet == 0)
    {
        //* we should not return to decoder again if it had been used to set eos to render
        if(pCtx->mOutputBufferInfoArr[index].mUseDecoderBufferToSetEosFlag)
        {
            pCtx->mOutputBufferInfoArr[index].mUseDecoderBufferToSetEosFlag = OMX_FALSE;
            return 0;
        }
        pVideoPicture = pCtx->mOutputBufferInfoArr[index].pVideoPicture;
        ReturnPicture(pCtx->m_decoder, pVideoPicture);
        OmxTryPostSem(pCtx->mSemOutBuffer);
        return 0;
    }
    else
        return mRet;
 }

static int returnBufferToDecdoer(OmxAwDecoderContext *pCtx)
{
    OMX_U32 index = 0xff;
    OMX_BUFFERHEADERTYPE*  pOutBufHdr = NULL;
    pOutBufHdr = doRequestPortBuffer(pCtx->pOutPort);
    if(pOutBufHdr != NULL)
    {
        if(isToInitBuffer(pCtx, pOutBufHdr, &index))
        {
            return returnInitBuffer(pCtx, pOutBufHdr, index);
        }
        else
        {
            return returnArrangedBuffer(pCtx, pOutBufHdr, index);
        }
    }
    else
        return -1;
}

static int anCheckResolutionChange(OmxAwDecoderContext *pCtx, VideoPicture* picture)
{
    int nPicRealWidth  = picture->nRightOffset  - picture->nLeftOffset;
    int nPicRealHeight = picture->nBottomOffset - picture->nTopOffset;

    //* if the offset is not right, we should not use them to compute width and height
    if(nPicRealWidth <= 0 || nPicRealHeight <= 0)
    {
        nPicRealWidth  = picture->nWidth;
        nPicRealHeight = picture->nHeight;
    }

    int width_align  = picture->nWidth;
    int height_align = picture->nHeight;

    OMX_PARAM_PORTDEFINITIONTYPE* inDef  = getPortDef(pCtx->pInPort);
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->pOutPort);

    if(inDef->format.video.nFrameWidth == WIDTH_DEFAULT ||
       inDef->format.video.nFrameHeight == HEIGHT_DEFAULT)
    {
        inDef->format.video.nFrameHeight = nPicRealHeight;
        inDef->format.video.nFrameWidth  = nPicRealWidth;
    }

    pCtx->mVideoRect.nLeft   = picture->nLeftOffset;
    pCtx->mVideoRect.nTop    = picture->nTopOffset;
    pCtx->mVideoRect.nWidth  = nPicRealWidth;
    pCtx->mVideoRect.nHeight = nPicRealHeight;

    if(pCtx->bUseAndroidBuffer)
    {
        if((OMX_U32)width_align  != outDef->format.video.nFrameWidth ||
           (OMX_U32)height_align != outDef->format.video.nFrameHeight)
        {
            logw("bUseAndroidBuffer: video picture size changed: "
                "org_height = %lu, org_width = %lu,"
                "new_height = %d, new_width = %d.",
                outDef->format.video.nFrameHeight,
                outDef->format.video.nFrameWidth,
                height_align, width_align);

            outDef->format.video.nFrameHeight = height_align;
            outDef->format.video.nFrameWidth  = width_align;
            outDef->nBufferSize = height_align*width_align *3/2;
            pCtx->callback(pCtx->pUserData, AW_OMX_CB_NOTIFY_RECT, &(pCtx->mVideoRect));
            pCtx->callback(pCtx->pUserData, AW_OMX_CB_PORT_CHANGE, NULL);
            return 0;
         }
    }
    else
    {
        if((OMX_U32)nPicRealWidth  != outDef->format.video.nFrameWidth ||
           (OMX_U32)nPicRealHeight != outDef->format.video.nFrameHeight)
        {
            logw("not bUseAndroidBuffer: video picture size changed: "
                "org_height = %lu, org_width = %lu,"
                "new_height = %d, new_width = %d.",
                outDef->format.video.nFrameHeight,
                outDef->format.video.nFrameWidth,
                nPicRealHeight, nPicRealWidth);

            outDef->format.video.nFrameHeight = nPicRealHeight;
            outDef->format.video.nFrameWidth  = nPicRealWidth;
            outDef->nBufferSize = nPicRealHeight*nPicRealWidth *3/2;
            pCtx->callback(pCtx->pUserData, AW_OMX_CB_NOTIFY_RECT, &(pCtx->mVideoRect));
            pCtx->callback(pCtx->pUserData, AW_OMX_CB_PORT_CHANGE, NULL);
            return 0;
        }
    }
    return -1;
}

static void copyPictureDataToAndroidBuffer(OmxAwDecoderContext *pCtx,
                                                    VideoPicture* picture,
                                                    OMX_BUFFERHEADERTYPE* pOutBufHdr)
{
    TranBufferInfo        mTranBufferInfo;
    memset(&mTranBufferInfo, 0 ,sizeof(TranBufferInfo));
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->pOutPort);
    void* dst;
    buffer_handle_t         pBufferHandle = NULL;
    android::GraphicBufferMapper &mapper = android::GraphicBufferMapper::get();
    //*when lock gui buffer, we must according gui buffer's width and height, not by decoder!
    android::Rect bounds(outDef->format.video.nFrameWidth,
                         outDef->format.video.nFrameHeight);

#ifdef CONF_KITKAT_AND_NEWER
    if(pCtx->bStoreOutputMetaDataFlag ==OMX_TRUE)
    {
        VideoDecoderOutputMetaData *pMetaData =
        (VideoDecoderOutputMetaData*)pOutBufHdr->pBuffer;
        pBufferHandle = pMetaData->pHandle;
    }
    else
        pBufferHandle = (buffer_handle_t)pOutBufHdr->pBuffer;
#else
    pBufferHandle = (buffer_handle_t)pOutBufHdr->pBuffer;
#endif
    if(0 != mapper.lock(pBufferHandle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &dst))
    {
        logw("Lock GUIBuf fail!");
    }

    memcpy(dst,picture->pData0,picture->nWidth*picture->nHeight*3/2);

    if(0 != mapper.unlock(pBufferHandle))
    {
        logw("Unlock GUIBuf fail!");
    }
}

static void TransformYV12ToYUV420(VideoPicture* pPicture, TranBufferInfo* pTranBufferInfo)
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

    nPicRealWidth  = pPicture->nRightOffset - pPicture->nLeftOffset;
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

    logd("nPicRealWidth & H = %d, %d,"
          "nSrcBufWidth & H = %d, %d,"
          "nDstBufWidth & H = %d, %d,"
          "nCopyDataWidth & H = %d, %d",
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

#ifdef CONF_PIE_AND_NEWER
static OMX_BOOL isColorAspectsDiffer(const ColorAspects a, const ColorAspects b) {
    if (a.mRange != b.mRange
        || a.mPrimaries != b.mPrimaries
        || a.mTransfer != b.mTransfer
        || a.mMatrixCoeffs != b.mMatrixCoeffs) {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

static void updateColorAspects(OmxAwDecoderContext *pCtx,
                                     const ColorAspects aspects) {
    ColorAspects newAspects;
    newAspects.mRange = aspects.mRange != ColorAspects::RangeUnspecified ?
        aspects.mRange : pCtx->mColorAspects.mRange;
    newAspects.mPrimaries = aspects.mPrimaries != ColorAspects::PrimariesUnspecified ?
        aspects.mPrimaries : pCtx->mColorAspects.mPrimaries;
    newAspects.mTransfer = aspects.mTransfer != ColorAspects::TransferUnspecified ?
        aspects.mTransfer : pCtx->mColorAspects.mTransfer;
    newAspects.mMatrixCoeffs = aspects.mMatrixCoeffs != ColorAspects::MatrixUnspecified ?
        aspects.mMatrixCoeffs : pCtx->mColorAspects.mMatrixCoeffs;
    pCtx->mColorAspects = newAspects;
}

static void adapteColorAspects(ColorAspects* aspects,
                                     int32_t mTransferCharacteristics,
                                     int32_t mMatrixCoeffs,
                                     int32_t mVideoFullRange,
                                     int32_t mPrimaries)
{
    switch (mTransferCharacteristics)
    {
        case VIDEO_TRANSFER_GAMMA2_2:
            aspects->mTransfer = ColorAspects::TransferGamma22;
            break;
        case VIDEO_TRANSFER_GAMMA2_8:
            aspects->mTransfer = ColorAspects::TransferGamma28;
            break;
        case VIDEO_TRANSFER_SMPTE_170M:
            aspects->mTransfer = ColorAspects::TransferSMPTE170M;
            break;
        case VIDEO_TRANSFER_SMPTE_240M:
            aspects->mTransfer = ColorAspects::TransferSMPTE240M;
            break;
        case VIDEO_TRANSFER_LINEAR:
            aspects->mTransfer = ColorAspects::TransferLinear;
            break;
        case VIDEO_TRANSFER_BT1361:
            aspects->mTransfer = ColorAspects::TransferBT1361;
            break;
        case VIDEO_TRANSFER_SRGB:
            aspects->mTransfer = ColorAspects::TransferSRGB;
            break;
        case VIDEO_TRANSFER_ST2084:
            aspects->mTransfer = ColorAspects::TransferST2084;
            break;
        case VIDEO_TRANSFER_ST428_1:
            aspects->mTransfer = ColorAspects::TransferST428;
            break;
        case VIDEO_TRANSFER_HLG:
            aspects->mTransfer = ColorAspects::TransferHLG;
            break;
        case VIDEO_TRANSFER_LOGARITHMIC_0:
        case VIDEO_TRANSFER_LOGARITHMIC_1:
        case VIDEO_TRANSFER_IEC61966:
        case VIDEO_TRANSFER_BT1361_EXTENDED:
        case VIDEO_TRANSFER_BT2020_0:
        case VIDEO_TRANSFER_BT2020_1:
            aspects->mTransfer = ColorAspects::TransferOther;
            break;
        case VIDEO_TRANSFER_RESERVED_0:
        case VIDEO_TRANSFER_UNSPECIFIED:
        case VIDEO_TRANSFER_RESERVED_1:
        default:
            aspects->mTransfer = ColorAspects::TransferUnspecified;
            break;
    }
    switch (mMatrixCoeffs)
    {
        case VIDEO_MATRIX_COEFFS_BT709:
            aspects->mMatrixCoeffs = ColorAspects::MatrixBT709_5;
            break;
        case VIDEO_MATRIX_COEFFS_BT470M:
            aspects->mMatrixCoeffs = ColorAspects::MatrixBT470_6M;
            break;
        case VIDEO_MATRIX_COEFFS_BT601_625_0:
        case VIDEO_MATRIX_COEFFS_BT601_625_1:
            aspects->mMatrixCoeffs = ColorAspects::MatrixBT601_6;
            break;
        case VIDEO_MATRIX_COEFFS_SMPTE_240M:
            aspects->mMatrixCoeffs = ColorAspects::MatrixSMPTE240M;
            break;
        case VIDEO_MATRIX_COEFFS_BT2020:
            aspects->mMatrixCoeffs = ColorAspects::MatrixBT2020;
            break;
        case VIDEO_MATRIX_COEFFS_BT2020_CONSTANT_LUMINANCE:
            aspects->mMatrixCoeffs = ColorAspects::MatrixBT2020Constant;
            break;
        case VIDEO_MATRIX_COEFFS_SOMPATE:
        case VIDEO_MATRIX_COEFFS_CD_NON_CONSTANT_LUMINANCE:
        case VIDEO_MATRIX_COEFFS_CD_CONSTANT_LUMINANCE:
        case VIDEO_MATRIX_COEFFS_BTICC:
        case VIDEO_MATRIX_COEFFS_YCGCO:
            aspects->mMatrixCoeffs = ColorAspects::MatrixOther;
            break;
        case VIDEO_MATRIX_COEFFS_UNSPECIFIED_0:
        case VIDEO_MATRIX_COEFFS_RESERVED_0:
        case VIDEO_MATRIX_COEFFS_IDENTITY:
        default:
            aspects->mMatrixCoeffs = ColorAspects::MatrixUnspecified;
            break;
    }
    switch (mVideoFullRange)
    {
        case VIDEO_FULL_RANGE_LIMITED:
            aspects->mRange = ColorAspects::RangeLimited;
            break;
        case VIDEO_FULL_RANGE_FULL:
            aspects->mRange = ColorAspects::RangeFull;
            break;
        default:
        {
            aspects->mRange = ColorAspects::RangeUnspecified;
            loge("should not be here, mVideoFullRange = %d", mVideoFullRange);
            break;
        }
    }
    switch(mPrimaries)
    {
        case 1:
            aspects->mPrimaries = ColorAspects::PrimariesBT709_5;
            break;
        case 4:
            aspects->mPrimaries = ColorAspects::PrimariesBT470_6M;
            break;
        case 5:
            aspects->mPrimaries = ColorAspects::PrimariesBT601_6_625;
            break;
        case 6:
            aspects->mPrimaries = ColorAspects::PrimariesBT601_6_525;
            break;
        case 7:
            aspects->mPrimaries = ColorAspects::PrimariesOther;
            break;
        case 8:
            aspects->mPrimaries = ColorAspects::PrimariesGenericFilm;
            break;
        case 9:
            aspects->mPrimaries = ColorAspects::PrimariesBT2020;
            break;
        case 0:
        case 3:
        case 2:
        default:
        {
            aspects->mPrimaries = ColorAspects::PrimariesUnspecified;
            break;
        }
    }
    return ;
}
#endif

static OMX_BUFFERHEADERTYPE* anDrainCopy(OmxAwDecoderContext *pCtx)
{
    VideoPicture*           pPicture     = NULL;
    OMX_BUFFERHEADERTYPE*   pOutBufHdr  = NULL;
    TranBufferInfo          mTranBufferInfo;
    memset(&mTranBufferInfo, 0 ,sizeof(TranBufferInfo));

    OmxAcquireMutex(pCtx->awMutexHandler);
    pPicture = NextPictureInfo(pCtx->m_decoder,0);
    OmxReleaseMutex(pCtx->awMutexHandler);
    if(pPicture == NULL)
        return NULL;

    if(anCheckResolutionChange(pCtx, pPicture) == 0)
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

#ifdef CONF_PIE_AND_NEWER
    ColorAspects tmpColorAspects;
    adapteColorAspects(&tmpColorAspects,
                       pCtx->pPicture->transfer_characteristics,
                       pCtx->pPicture->matrix_coeffs,
                       pCtx->pPicture->video_full_range_flag,
                       pCtx->pPicture->colour_primaries);

    if(isColorAspectsDiffer(pCtx->mColorAspects, tmpColorAspects))
    {
        updateColorAspects(pCtx, tmpColorAspects);
    }
#endif
#if(ENABLE_SAVE_PICTURE)
    savePictureForDebug(pCtx, pPicture);
#endif
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->pOutPort);

    if(!pCtx->bUseAndroidBuffer)
    {
        mTranBufferInfo.pAddr   = pOutBufHdr->pBuffer;
        mTranBufferInfo.nWidth  = outDef->format.video.nFrameWidth;
        mTranBufferInfo.nHeight = outDef->format.video.nFrameHeight;
        TransformYV12ToYUV420(pCtx->pPicture, &mTranBufferInfo);  // YUV420 planar
    }
    else
    {
        copyPictureDataToAndroidBuffer(pCtx, pCtx->pPicture, pOutBufHdr);
    }
    pOutBufHdr->nTimeStamp = pCtx->pPicture->nPts;
    pOutBufHdr->nOffset    = 0;
    pOutBufHdr->nFilledLen = (outDef->format.video.nFrameWidth *
                              outDef->format.video.nFrameHeight) * 3 / 2;
    return pOutBufHdr;
}

static OMX_BUFFERHEADERTYPE* anDrainZeroCopy(OmxAwDecoderContext *pCtx)
{
    OMX_U32      i = 0;
    VideoPicture*           pPicture     = NULL;
    OMX_BUFFERHEADERTYPE*   pOutBufHdr  = NULL;

    pPicture = RequestPicture(pCtx->m_decoder, 0);
    if(pPicture == NULL)
    {
        logw("the pPicture is null when request displayer picture!");
        return NULL;
    }
    logv("*** picture info: w(%d),h(%d),offset,t(%d),b(%d),l(%d),r(%d)",
         pPicture->nWidth,pPicture->nHeight,
         pPicture->nTopOffset,pPicture->nBottomOffset,
         pPicture->nLeftOffset,pPicture->nRightOffset);

#if(ENABLE_SAVE_PICTURE)
    savePictureForDebug(pCtx, pPicture);
#endif

    OMX_U32 num = getPortAllocSize(pCtx->pOutPort);
    for(i = 0; i<num; i++)
    {
        if(pCtx->mOutputBufferInfoArr[i].pVideoPicture == pPicture)
            break;
    }

    OMX_ASSERT(i!=num);

    //* we should return buffer to decoder if it was used as set eos to render
    if(pCtx->mOutputBufferInfoArr[i].mUseDecoderBufferToSetEosFlag)
    {
        logw("detect the buffer had been used to set eos to render,\
              here, we not callback again!");
        pCtx->mOutputBufferInfoArr[i].mStatus = OWNED_BY_RENDER;
        pCtx->mOutputBufferInfoArr[i].mUseDecoderBufferToSetEosFlag = OMX_FALSE;
        return NULL;
    }

    pOutBufHdr =  pCtx->mOutputBufferInfoArr[i].pHeadBufInfo;

    pCtx->mOutputBufferInfoArr[i].mStatus = OWNED_BY_RENDER;

    pOutBufHdr->nTimeStamp = pPicture->nPts;
    pOutBufHdr->nOffset    = 0;
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(pCtx->pOutPort);
#ifdef CONF_PIE_AND_NEWER
    ColorAspects tmpColorAspects;
    adapteColorAspects(&tmpColorAspects,
                       pPicture->transfer_characteristics,
                       pPicture->matrix_coeffs,
                       pPicture->video_full_range_flag,
                       pPicture->colour_primaries);
    if(isColorAspectsDiffer(pCtx->mColorAspects, tmpColorAspects))
    {
        updateColorAspects(pCtx, tmpColorAspects);
    }

#else
    if(pOutBufHdr->pOutputPortPrivate != NULL)
    {
        AW_OMX_VIDEO_HDR_INFO* pHdrInfo =
            (AW_OMX_VIDEO_HDR_INFO*)pOutBufHdr->pOutputPortPrivate;

        pHdrInfo->nExtMatrixCoeffs = pPicture->matrix_coeffs;
        pHdrInfo->nExtVideoFullRangeFlag = pPicture->video_full_range_flag;
        pHdrInfo->nExtTransferCharacteristics = pPicture->transfer_characteristics;

        logv("***hdr info: %lu, %lu, %lu",
            pHdrInfo->nExtMatrixCoeffs,
            pHdrInfo->nExtVideoFullRangeFlag,
            pHdrInfo->nExtTransferCharacteristics);
    }
#endif

#ifdef CONF_KITKAT_AND_NEWER
    if(pCtx->bStoreOutputMetaDataFlag)
    {
        pOutBufHdr->nFilledLen = sizeof(VideoDecoderOutputMetaData);
    }
    else
    {
        pOutBufHdr->nFilledLen = (outDef->format.video.nFrameWidth* \
                                  outDef->format.video.nFrameHeight)*3/2;
    }
#else
    pOutBufHdr->nFilledLen = (outDef->format.video.nFrameWidth *
                              outDef->format.video.nFrameHeight) * 3 / 2;
#endif
    return pOutBufHdr;

}

static int freeVirAddrOfMetadataBuffer(int mIonFd,
                                ion_handle_abstract_t handle_ion,
                                int mapfd,
                                int virsize,
                                void* viraddress)
{

#ifdef CONF_SURPPORT_METADATA_BUFFER
    if (viraddress != 0) {
        munmap(viraddress, virsize);
    }
    if (mapfd != 0) {
        close(mapfd);
    }
    if (handle_ion != 0) {
        ion_free(mIonFd, handle_ion);
    }
#else
    CEDARC_UNUSE(mIonFd);
    CEDARC_UNUSE(handle_ion);
    CEDARC_UNUSE(mapfd);
    CEDARC_UNUSE(virsize);
    CEDARC_UNUSE(viraddress);
#endif
    return 0;
}

static void freeOutputBufferInfoArr(OmxAwDecoderContext *pCtx)
{
    int i, ret;
    for(i = 0; i < OMX_MAX_VIDEO_BUFFER_NUM; i++)
    {

        if(pCtx->mOutputBufferInfoArr[i].handle_ion != ION_NULL_VALUE)
        {
            //logd("ion_free: handle_ion[%p],i[%d]",handle_ion,i);
            if(pCtx->mOutputBufferInfoArr[i].nBufFd >= 0)
            {
                if(!pCtx->bIsSecureVideoFlag)
                {
                    int len = pCtx->mOutputBufferInfoArr[i].nVirAddrSize;
                    unsigned char *pVirAddr = \
                     (unsigned char *)pCtx->mOutputBufferInfoArr[i].pBufVirAddr;
                    CdcIonUnmap(len, pVirAddr);
                }
                close(pCtx->mOutputBufferInfoArr[i].nBufFd);
            }
            ret = ion_free(pCtx->mIonFd, pCtx->mOutputBufferInfoArr[i].handle_ion);
            if(ret != 0)
                loge("ion_free( %d ) failed", pCtx->mIonFd);
            pCtx->mOutputBufferInfoArr[i].handle_ion = ION_NULL_VALUE;
        }

        if(pCtx->mOutputBufferInfoArr[i].metadata_handle_ion != ION_NULL_VALUE)
        {
            freeVirAddrOfMetadataBuffer(pCtx->mIonFd,
                             pCtx->mOutputBufferInfoArr[i].metadata_handle_ion,
                             pCtx->mOutputBufferInfoArr[i].nMetaDataMapFd,
                             pCtx->mOutputBufferInfoArr[i].nMetaDataVirAddrSize,
                             pCtx->mOutputBufferInfoArr[i].pMetaDataVirAddr);
        }
    }

    memset(&pCtx->mOutputBufferInfoArr, 0,
             sizeof(OMXOutputBufferInfoA)*OMX_MAX_VIDEO_BUFFER_NUM);
}

static void standbyOutBufferArr(OmxAwDecoderContext *pCtx)
{
    int i;
    OMX_BOOL        out_buffer_flag   = OMX_FALSE;

    for(i = 0; i < OMX_MAX_VIDEO_BUFFER_NUM; i++)
    {
        OutBufferStatus out_buffer_status = pCtx->mOutputBufferInfoArr[i].mStatus;
        out_buffer_flag = \
            pCtx->mOutputBufferInfoArr[i].mUseDecoderBufferToSetEosFlag;
        if(out_buffer_status == OWNED_BY_DECODER)
        {
            logd("fillBufferDone: i[%d],pHeadBufInfo[%p],mStatus[%s]",
                 i,
                 pCtx->mOutputBufferInfoArr[i].pHeadBufInfo,
                 statusToString(out_buffer_status));

            pCtx->callback(pCtx->pUserData, AW_OMX_CB_FILL_BUFFER_DONE,
                            pCtx->mOutputBufferInfoArr[i].pHeadBufInfo);

            pCtx->mOutputBufferInfoArr[i].mStatus = OWNED_BY_RENDER;
        }
        else if(out_buffer_status == OWNED_BY_RENDER
                && !out_buffer_flag && pCtx->bFlushFlag)
        {
            if(pCtx->mOutputBufferInfoArr[i].pVideoPicture != NULL)
            {

                logw("** return pic when flush,i[%d],pPic[%p]",
                     i,pCtx->mOutputBufferInfoArr[i].pVideoPicture);
                ReturnPicture(pCtx->m_decoder,
                              pCtx->mOutputBufferInfoArr[i].pVideoPicture);
                OmxTryPostSem(pCtx->mSemOutBuffer);
            }
        }
        else if(out_buffer_status == OWNED_BY_US)
        {
            logd("** fillbufferDone owned_by_us, i[%d]", i);
            pCtx->callback(pCtx->pUserData, AW_OMX_CB_FILL_BUFFER_DONE,
                            pCtx->mOutputBufferInfoArr[i].pHeadBufInfo);
        }
    }
    freeOutputBufferInfoArr(pCtx);
}

static int __anCallback(OmxDecoder* pDec,
                                    DecoderCallback callback,
                                    void* pUserData)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    pCtx->callback  = callback;
    pCtx->pUserData = pUserData;
    return 0;
}

static OMX_ERRORTYPE __anGetExtPara(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eParamIndex,
                                       OMX_PTR pParamData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    OMX_PARAM_PORTDEFINITIONTYPE* inDef = getPortDef(pCtx->pInPort);
    switch(eParamIndex)
    {
        case AWOMX_IndexParamVideoGetAndroidNativeBufferUsage:
        {
            GetAndroidNativeBufferUsageParams* param;
            param = (GetAndroidNativeBufferUsageParams*)pParamData;
            if(inDef->format.video.eCompressionFormat == OMX_VIDEO_CodingVP6 ||
               inDef->format.video.eCompressionFormat == OMX_VIDEO_CodingWMV1||
               inDef->format.video.eCompressionFormat == OMX_VIDEO_CodingWMV2)
            {
                // software decoder, gpu use this usage to malloc buffer with cache.
                param->nUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN;
            }
            else
            {
                //* gpu use this usage to malloc continuous physical buffer.
                param->nUsage |= GRALLOC_USAGE_HW_2D;
            }
            logd(" ===== get uage: %lu", param->nUsage);
            break;
        }

        case AWOMX_IndexParamVideoGetAfbcHdrFlag:
        {
            logd(" COMPONENT_GET_PARAMETER: AWOMX_IndexParamVideoGetAfbcHdrFlag:%lu",
                 pCtx->nExtendFlag);
            OMX_U32* pExtendFlag = (OMX_U32*)pParamData;
            *pExtendFlag = pCtx->nExtendFlag;
            *(pExtendFlag + 1) = pCtx->nExtendFlag;
            break;
        }
        default:
        {
            logw("get_parameter: unknown param %08x\n", eParamIndex);
            eError =OMX_ErrorUnsupportedIndex;
            break;
        }
    }
    return eError;
}

static OMX_ERRORTYPE __anSetExtPara(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eParamIndex,
                                       OMX_PTR pParamData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    logv("set_parameter: %x",eParamIndex);

    switch(eParamIndex)
    {
        case AWOMX_IndexParamVideoUseAndroidNativeBuffer2:
        {
            pCtx->bUseAndroidBuffer = OMX_TRUE;
            break;
        }
        case AWOMX_IndexParamVideoEnableAndroidNativeBuffers:
        {
            EnableAndroidNativeBuffersParams *EnableAndroidBufferParams
                = (EnableAndroidNativeBuffersParams*)pParamData;
            logv(" enbleParam = %d\n",EnableAndroidBufferParams->enable);
            if(1==EnableAndroidBufferParams->enable)
            {
                pCtx->bUseAndroidBuffer = OMX_TRUE;
            }
            break;
        }
        case AWOMX_IndexParamVideoAllocateNativeHandle:
        {

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

        //* not perfectly support the meta data, so forbid it.
#if 0
        case AWOMX_IndexParamVideoUseStoreMetaDataInBuffer:
        {
            logv("set_parameter: AWOMX_IndexParamVideoUseStoreMetaDataInBuffer");

            StoreMetaDataInBuffersParams *pStoreMetaData
                = (StoreMetaDataInBuffersParams*)pParamData;
            if(pStoreMetaData->nPortIndex != 1)
            {
                logd("error: not support set "
                    "AWOMX_IndexParamVideoUseStoreMetaDataInBuffer for inputPort");
                eError = OMX_ErrorUnsupportedIndex;
            }
            if(pStoreMetaData->nPortIndex==1 &&
                pStoreMetaData->bStoreMetaData)
            {
                logv("***set m_storeOutputMetaDataFlag to TRUE");
                pCtx->bStoreOutputMetaDataFlag = OMX_TRUE;
            }
            break;
        }
#endif

#ifdef CONF_KITKAT_AND_NEWER
        case AWOMX_IndexParamVideoUsePrepareForAdaptivePlayback:
        {
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
            logw("Setparameter: unknown param %d\n", eParamIndex);
            eError = OMX_ErrorUnsupportedIndex;
            break;
        }
    }
    return eError;
}

static OMX_ERRORTYPE __anGetExtConfig(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eConfigIndex,
                                       OMX_PTR pConfigData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    switch(eConfigIndex)
    {
        case AWOMX_IndexParamVideoDescribeColorAspects:
        {
            DescribeColorAspectsParams* colorAspectsParams =
                    (DescribeColorAspectsParams *)pConfigData;

            if (colorAspectsParams->nPortIndex != kOutputPortIndex) {
                eError = OMX_ErrorBadParameter;
                break;
            }

            colorAspectsParams->sAspects = pCtx->mColorAspects;

            logd("get ColorAspects: R:%d, P:%d, M:%d, T:%d",
            colorAspectsParams->sAspects.mRange,
            colorAspectsParams->sAspects.mPrimaries,
            colorAspectsParams->sAspects.mMatrixCoeffs,
            colorAspectsParams->sAspects.mTransfer);
            if (colorAspectsParams->bRequestingDataSpace || colorAspectsParams->bDataSpaceChanged) {
                eError = OMX_ErrorUnsupportedSetting;
                break;
            }
            break;
        }
        case AWOMX_IndexParamVideoDescribeHDRStaticInfo:
        {
            DescribeHDRStaticInfoParams* hdrStaticInfoParams =
                    (DescribeHDRStaticInfoParams *)pConfigData;

            if (hdrStaticInfoParams->nPortIndex != kOutputPortIndex) {
                eError = OMX_ErrorBadPortIndex;
                break;
            }

            hdrStaticInfoParams->sInfo = pCtx->mHdrStaticInfo;
            break;
        }
        default:
        {
            logd("get_config: unknown param %d\n",eConfigIndex);
            eError = OMX_ErrorUnsupportedIndex;
            break;
        }
    }
    return eError;
}

static OMX_ERRORTYPE __anSetExtConfig(OmxDecoder* pDec,
                                       AW_VIDEO_EXTENSIONS_INDEXTYPE eConfigIndex,
                                       OMX_PTR pConfigData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    switch(eConfigIndex)
    {
        case AWOMX_IndexParamVideoDescribeColorAspects:
        {
            const DescribeColorAspectsParams* colorAspectsParams =
                    (const DescribeColorAspectsParams *)pConfigData;

            if (colorAspectsParams->nPortIndex != kOutputPortIndex) {
                eError = OMX_ErrorBadParameter;
                break;
            }
            pCtx->mColorAspects = colorAspectsParams->sAspects;
            logd("set ColorAspects: R:%d, P:%d, M:%d, T:%d",
            colorAspectsParams->sAspects.mRange,
            colorAspectsParams->sAspects.mPrimaries,
            colorAspectsParams->sAspects.mMatrixCoeffs,
            colorAspectsParams->sAspects.mTransfer);
            break;
        }
        case AWOMX_IndexParamVideoDescribeHDRStaticInfo:
        {
            const DescribeHDRStaticInfoParams* hdrStaticInfoParams =
                    (DescribeHDRStaticInfoParams *)pConfigData;

            if (hdrStaticInfoParams->nPortIndex != kOutputPortIndex) {
                eError =  OMX_ErrorBadPortIndex;
                break;
            }
            pCtx->mHdrStaticInfo = hdrStaticInfoParams->sInfo;
            //TODO
            //updatePortDefinitions(false);
            break;
        }
        default:
        {
            logd("get_config: unknown param %d\n",eConfigIndex);
            eError = OMX_ErrorUnsupportedIndex;
            break;
        }
    }
    return eError;
}

static int __anPrepare(OmxDecoder* pDec)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;

    int ret = -1;
    if(-1 == anDealWithInitData(pCtx))
        return -1;
    logd("Prepare decoder begin!");
    anGetStremInfo(pCtx);
    OmxAcquireMutex(pCtx->awMutexHandler);
    //*if mdecoder had closed before, we should create it
    if(pCtx->m_decoder==NULL)
    {
        pCtx->m_decoder = CreateVideoDecoder();
    }

#ifdef CONF_OMX_USE_ZERO_COPY
    if(!pCtx->pOutPort->bAllocBySelfFlags)
    {
        pCtx->bUseZeroCopyBuffer = OMX_TRUE;
        pCtx->mVideoConfig.bGpuBufValid = 1;
    }
/*
    if(pCtx->bIsSecureVideoFlag == OMX_TRUE)
        pCtx->m_videoConfig.bSecureosEn = 1;
*/
#endif


    pCtx->mVideoConfig.nAlignStride = pCtx->mGpuAlignStride;

    pCtx->mVideoConfig.eOutputPixelFormat = PIXEL_FORMAT_YV12;//* Used to be YV12.


    //pCtx->mVideoConfig.bCalledByOmxFlag   = 1;

#ifdef CONF_SURPPORT_METADATA_BUFFER
    pCtx->mVideoConfig.eCtlAfbcMode       = ENABLE_AFBC_JUST_BIG_SIZE;
#endif

#if (ENABLE_SCALEDOWN_WHEN_RESOLUTION_MOER_THAN_1080P)
    // omx decoder make out buffer no more than 1920x1080
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
        logd("*** pCtx->mStreamInfo.bIsFramePackage to 1 when it is vc1");
        pCtx->mStreamInfo.bIsFramePackage = 1;
    }

    pCtx->mVideoConfig.bDispErrorFrame = 1;

    pCtx->mVideoConfig.nDeInterlaceHoldingFrameBufferNum = 0;
    if(pCtx->bUseZeroCopyBuffer)
    {
        pCtx->mVideoConfig.nDisplayHoldingFrameBufferNum \
                             = pCtx->mExtraOutBufferNum; //* ACodec decide the value
        logd("gqy****mExtraOutBufferNum:%lu", pCtx->mExtraOutBufferNum);
    }
    pCtx->mVideoConfig.nRotateHoldingFrameBufferNum \
                         = NUM_OF_PICTURES_KEEPPED_BY_ROTATE;

    pCtx->mVideoConfig.nDecodeSmoothFrameBufferNum \
                         = NUM_OF_PICTURES_FOR_EXTRA_SMOOTH;


   // if(pCtx->mIsSecureVideoFlag == OMX_TRUE)
    pCtx->mVideoConfig.memops = MemAdapterGetOpsS();

    pCtx->decMemOps     =  pCtx->mVideoConfig.memops;
    CdcMemOpen(pCtx->decMemOps);

    //pCtx->mStreamInfo.bIsFramePackage = 1;
    ret = InitializeVideoDecoder(pCtx->m_decoder,
                                 &(pCtx->mStreamInfo),
                                 &pCtx->mVideoConfig);
    if(ret != 0)
    {
        DestroyVideoDecoder(pCtx->m_decoder);
        pCtx->m_decoder           = NULL;
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_EVENT_ERROR, NULL);
        loge("Idle transition failed, set_vstream_info() return fail.\n");
        OmxReleaseMutex(pCtx->awMutexHandler);
        return ret;
    }
    OmxReleaseMutex(pCtx->awMutexHandler);
    OmxTryPostSem(pCtx->mSemInData);
    OmxTryPostSem(pCtx->mSemOutBuffer);
    return ret;
}

static int __anSubmit(OmxDecoder* pDec,
                                 OMX_BUFFERHEADERTYPE* pInBufHdr)
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

    //* if the size is 0, we should not copy it to decoder
    OmxAcquireMutex(pCtx->awMutexHandler);
    if(RequestVideoStreamBuffer(pCtx->m_decoder, require_size, &pBuf0,
                                &size0, &pBuf1, &size1,0)!= 0)
    {
        logv("req vbs fail! maybe vbs buffer is full! require_size[%d]",
             require_size);

        OmxReleaseMutex(pCtx->awMutexHandler);
        return -1;
    }

    if(require_size != (size0 + size1))
    {
        logw(" the requestSize[%d] is not equal to needSize[%d]!",
              (size0+size1),require_size);

        OmxReleaseMutex(pCtx->awMutexHandler);
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

    //* copy input data
    if(pCtx->bIsSecureVideoFlag)
    {

        if(pCtx->secureMemOps == NULL)
        {
            pCtx->secureMemOps = SecureMemAdapterGetOpsS();
            CdcMemOpen(pCtx->secureMemOps);
        }

        OMX_U8* pInBuffer = NULL;
        if(pCtx->bSurpporNativeHandle == OMX_TRUE)
        {
            native_handle_t *nh = (native_handle_t *)pInBufHdr->pBuffer;
            pInBuffer = (OMX_U8*)(uintptr_t)nh->data[0];
        }
        else
            pInBuffer = pInBufHdr->pBuffer;

#if(ADJUST_ADDRESS_FOR_SECURE_OS_OPTEE)
        pData  = (unsigned char*)pInBuffer;
#else
        pData  = (unsigned char*)CdcMemGetVirtualAddressCpu(pCtx->secureMemOps, pInBuffer);
#endif
        pData += pInBufHdr->nOffset;
        if(require_size <= size0)
        {
            //SecureMemAdapterCopy(pBuf0,pData,require_size);
            CdcMemCopy(pCtx->secureMemOps, pBuf0, pData, require_size);
        }
        else
        {
            //SecureMemAdapterCopy(pBuf0, pData, size0);
            CdcMemCopy(pCtx->secureMemOps, pBuf0, pData, size0);
            pData += size0;
            //SecureMemAdapterCopy(pBuf1, pData, require_size - size0);
            CdcMemCopy(pCtx->secureMemOps, pBuf1, pData, require_size - size0);
        }
    }
    else
    {
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
    }
    SubmitVideoStreamData(pCtx->m_decoder, &DataInfo, 0);

    OmxTryPostSem(pCtx->mSemInData);
    OmxReleaseMutex(pCtx->awMutexHandler);
    return 0 ;
}

static inline void __anDecode(OmxDecoder* pDec)
{
    int     decodeResult;
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
#if ENABLE_STATISTICS_TIME
    int64_t nTimeUs1;
    int64_t nTimeUs2;
    nTimeUs1 = OmxGetNowUs();
#endif
    OmxAcquireMutex(pCtx->awMutexHandler);
    decodeResult = DecodeVideoStream(pCtx->m_decoder,0,0,0,0);
    logv("gqy*****decodeResult: %d", decodeResult);
    OmxReleaseMutex(pCtx->awMutexHandler);
#if ENABLE_STATISTICS_TIME
    nTimeUs2 = OmxGetNowUs();
    logd("decode use time:%lld", nTimeUs2 - nTimeUs1);
#endif
    if(decodeResult == VDECODE_RESULT_KEYFRAME_DECODED ||
       decodeResult == VDECODE_RESULT_FRAME_DECODED ||
       decodeResult == VDECODE_RESULT_OK)
    {
        OmxTryPostSem(pCtx->mSemValidPic);
    }
    else if(decodeResult == VDECODE_RESULT_NO_FRAME_BUFFER)
    {
        OmxTimedWaitSem(pCtx->mSemOutBuffer, 2);
    }
    else if(decodeResult == VDECODE_RESULT_NO_BITSTREAM ||
            decodeResult == VDECODE_RESULT_CONTINUE)
    {
        if(pCtx->bInputEosFlag)
        {
            pCtx->bInputEosFlag = OMX_FALSE;

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

        }
        else
        {
            OmxTimedWaitSem(pCtx->mSemInData, 2);
        }
    }
    else if(decodeResult == VDECODE_RESULT_RESOLUTION_CHANGE)
    {
    resolution_change:
        int validPicNum = ValidPictureNum(pCtx->m_decoder, 0);
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_FLUSH_ALL_PIC, &validPicNum);
        anReopenVideoEngine(pCtx);
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_FININSH_REOPEN_VE, NULL);
    }
    else if(decodeResult < 0)
    {
        logw("decode fatal error[%d]", decodeResult);
        //* report a fatal error.
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_EVENT_ERROR, NULL);
    }
    else
    {
        logd("decode ret[%d], ignore? why?", decodeResult);
    }
    return ;
}

static OMX_BUFFERHEADERTYPE* __anDrain(OmxDecoder* pDec)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    int validPicNum = ValidPictureNum(pCtx->m_decoder, 0);

    if(validPicNum <= 0)
    {
        //pCtx->callback(pCtx->pUserData, AW_OMX_CB_CONTINUE_SUBMIT, NULL);
        if(pCtx->bHadGetVideoFbmBufInfoFlag)
            OmxTimedWaitSem(pCtx->mSemValidPic, 2);
        return NULL;
    }
    if(pCtx->bUseZeroCopyBuffer)
        return anDrainZeroCopy(pCtx);
    else
        return anDrainCopy(pCtx);
}

static void __anStandbyBuffer(OmxDecoder* pDec)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    if(pCtx->bUseZeroCopyBuffer)
    {
        pCtx->mNeedReSetToDecoderBufferNum = 0;
        pCtx->mSetToDecoderBufferNum = 0;
        return standbyOutBufferArr(pCtx);
    }
    else
        return ;
}

static int __anReturnBuffer(OmxDecoder* pDec)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    if(pCtx->bUseZeroCopyBuffer)
    {
        if(!pCtx->bHadGetVideoFbmBufInfoFlag)
        {
            if(0 == anGetVideoFbmBufInfo(pCtx))
            {
                pCtx->bHadGetVideoFbmBufInfoFlag = OMX_TRUE;
                return 0;
            }
            else
            {
                pCtx->callback(pCtx->pUserData, AW_OMX_CB_CONTINUE_SUBMIT, NULL);
                if(pCtx->bInputEosFlag) //for cts test
                {
                    pCtx->callback(pCtx->pUserData, AW_OMX_CB_NOTIFY_EOS, NULL);
                }
            }
            return -1;
        }
        return returnBufferToDecdoer(pCtx);
    }
    else
    {
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

}

static inline void __anFlush(OmxDecoder* pDec)
{
    logd("decoder flush");
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    //* we should reset the mInputEosFlag when flush vdecoder
    pCtx->bInputEosFlag = OMX_FALSE;

    OmxTryPostSem(pCtx->mSemInData);
    OmxTryPostSem(pCtx->mSemOutBuffer);

    OmxAcquireMutex(pCtx->awMutexHandler);
    if(pCtx->m_decoder)
    {
        ResetVideoDecoder(pCtx->m_decoder);
    }
    else
    {
        logw(" fatal error, m_decoder is not malloc when flush all ports!");
    }
    if(pCtx->bUseZeroCopyBuffer)
    {
        pCtx->bFlushFlag = OMX_TRUE;
        standbyOutBufferArr(pCtx);
        pCtx->bFlushFlag = OMX_FALSE;
        pCtx->mNeedReSetToDecoderBufferNum += pCtx->mSetToDecoderBufferNum;
        pCtx->mSetToDecoderBufferNum = 0;
        SetVideoFbmBufRelease(pCtx->m_decoder);
    }
    OmxReleaseMutex(pCtx->awMutexHandler);
}

static  void __anClose(OmxDecoder* pDec)
{
    logd("decoder close");
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    OmxTryPostSem(pCtx->mSemInData);
    OmxTryPostSem(pCtx->mSemOutBuffer);
    OmxAcquireMutex(pCtx->awMutexHandler);

    //* stop and close decoder.
    if(pCtx->m_decoder != NULL)
    {
        DestroyVideoDecoder(pCtx->m_decoder);
        pCtx->m_decoder = NULL;
    }
    //* clear specific data
    pCtx->mCodecSpecificDataLen = 0;
    memset(pCtx->mCodecSpecificData, 0 , CODEC_SPECIFIC_DATA_LENGTH);
    pCtx->mNeedReSetToDecoderBufferNum = 0;
    pCtx->bHadGetVideoFbmBufInfoFlag   = OMX_FALSE;
    OmxReleaseMutex(pCtx->awMutexHandler);
}

static void __anSetInputEos(OmxDecoder* pDec, OMX_BOOL bEos)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    pCtx->bInputEosFlag = bEos;
}

static int __anSetOutputEos(OmxDecoder* pDec)
{
    //*set eos flag, MediaCodec use this flag
    // to determine whether a playback is finished.
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    int validPicNum = ValidPictureNum(pCtx->m_decoder, 0);
    if(validPicNum > 0)
    {
        logd("**validPicNum:%d , when setOutPutEos!!", validPicNum);
        return -1;
    }
    OMX_U32 nValidSize = getPortValidSize(pCtx->pOutPort);
    logv("*** OutBufList.nSizeOfList = %lu", nValidSize);

    if(nValidSize > 0)
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
                logd("++++ set output eos(normal)");
                pOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                pCtx->callback(pCtx->pUserData, AW_OMX_CB_FILL_BUFFER_DONE,
                               (void*)pOutBufHdr);
                pOutBufHdr = NULL;

                break;
            }
        }
    }
    else
    {
        OMX_U32 i = 0;
        OMX_U32 nAllocSize = getPortAllocSize(pCtx->pOutPort);
        for(i = 0; i<nAllocSize; i++)
        {
            if(pCtx->mOutputBufferInfoArr[i].mStatus == OWNED_BY_DECODER)
                break;
        }
        if(i == nAllocSize)
        {
            logw("** have no buffer to set eos, try again");
            return -1;
        }
        logd("*** set out eos(use buffer owned by decoder), pic = %p",
             pCtx->mOutputBufferInfoArr[i].pVideoPicture);
        OMX_BUFFERHEADERTYPE* pOutBufHdr =
               pCtx->mOutputBufferInfoArr[i].pHeadBufInfo;
        pOutBufHdr->nFilledLen = 0;
        pOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
        pCtx->callback(pCtx->pUserData, AW_OMX_CB_FILL_BUFFER_DONE,
                        (void*)pOutBufHdr);
        pCtx->mOutputBufferInfoArr[i].mStatus = OWNED_BY_RENDER;
        pCtx->mOutputBufferInfoArr[i].mUseDecoderBufferToSetEosFlag = OMX_TRUE;
    }
    return 0;
}

static void __anGetFormat(OmxDecoder* pDec)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    pCtx->mVideoConfig.eOutputPixelFormat = PIXEL_FORMAT_YV12;
    if(pCtx->bUseAndroidBuffer)
        setPortColorFormat(pCtx->pOutPort, HAL_PIXEL_FORMAT_YV12);
}

static void __anSetExtBufNum(OmxDecoder* pDec, OMX_S32 num)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    pCtx->mExtraOutBufferNum  = num;
}

static OMX_U8* __anAllocatePortBuffer(OmxDecoder* pDec, AwOmxVdecPort* port, OMX_U32 size)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;
    if(isInputPort(port) && pCtx->bIsSecureVideoFlag)
    {
        OMX_U8* pInBuffer = NULL;
        if(pCtx->secureMemOps == NULL)
        {
            pCtx->secureMemOps = SecureMemAdapterGetOpsS();
            CdcMemOpen(pCtx->secureMemOps);
        }
#if(ADJUST_ADDRESS_FOR_SECURE_OS_OPTEE)
        pInBuffer = (OMX_U8*)VideoDecoderPallocIonBuf(pCtx->m_decoder, size);
#else
        char* pSecureBuf = (char *)VideoDecoderPallocIonBuf(pCtx->m_decoder, size);
        pInBuffer = (OMX_U8*)CdcMemGetPhysicAddressCpu(pCtx->secureMemOps, pSecureBuf);
#endif

        if(pCtx->bSurpporNativeHandle)
        {
            native_handle_t *nh = native_handle_create(0 /*numFds*/, 1 /*numInts*/);
            nh->data[0] = (int)(uintptr_t)pInBuffer;
            return (OMX_U8*)nh;
        }
        else
        {
            return pInBuffer;
        }
    }
    else
    {
        OMX_U8* pBuffer = (OMX_U8*)malloc(size);
        return pBuffer;
    }

}

static void __anFreePortBuffer(OmxDecoder* pDec, AwOmxVdecPort* port, OMX_U8* pBuffer)
{
    OmxAwDecoderContext *pCtx = (OmxAwDecoderContext*)pDec;


    if(isInputPort(port) && pCtx->bIsSecureVideoFlag)
    {
        OMX_U8* pInBuffer = NULL;
        if(pCtx->bSurpporNativeHandle)
        {
            native_handle_t *nh =(native_handle_t*)pBuffer;
            pInBuffer = (OMX_U8*)(uintptr_t)nh->data[0];

            native_handle_close(nh);

            native_handle_delete(nh);
        }
        else
            pInBuffer = pBuffer;

#if(ADJUST_ADDRESS_FOR_SECURE_OS_OPTEE)
        char*   pVirtAddr = (char*)pInBuffer;
#else
        OMX_U8* pPhyAddr  = pInBuffer;
        char*   pVirtAddr = (char*)CdcMemGetVirtualAddressCpu(pCtx->secureMemOps, pPhyAddr);
#endif

        VideoDecoderFreeIonBuf(pCtx->m_decoder, pVirtAddr);
    }
    else
    {
        free(pBuffer);
    }
    return ;
}

static OmxDecoderOpsT mAwDecoderOps =
{
    .getExtPara   =   __anGetExtPara,
    .setExtPara   =   __anSetExtPara,
    .getExtConfig =   __anGetExtConfig,
    .setExtConfig =   __anSetExtConfig,
    .prepare      =   __anPrepare,
    .close        =   __anClose,
    .flush        =   __anFlush,
    .decode       =   __anDecode,
    .submit       =   __anSubmit,
    .drain        =   __anDrain,
    .callback     =   __anCallback,
    .setInputEos  =   __anSetInputEos,
    .setOutputEos =   __anSetOutputEos,
    .standbyBuf   =   __anStandbyBuffer,
    .returnBuf    =   __anReturnBuffer,
    .getFormat    =   __anGetFormat,
    .setExtBufNum =   __anSetExtBufNum,
    .allocPortBuf =   __anAllocatePortBuffer,
    .freePortBuf  =   __anFreePortBuffer,
};


OmxDecoder* OmxDecoderCreate(AwOmxVdecPort* in, AwOmxVdecPort* out)
{
    logv("OmxDecoderCreate.");
    OmxAwDecoderContext* pCtx;
    pCtx = (OmxAwDecoderContext*)malloc(sizeof(OmxAwDecoderContext));
    if(pCtx == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(pCtx, 0, sizeof(OmxAwDecoderContext));
    memset(&pCtx->mOutputBufferInfoArr, 0,
             sizeof(OMXOutputBufferInfoA)*OMX_MAX_VIDEO_BUFFER_NUM);
    pCtx->pInPort  = in;
    pCtx->pOutPort = out;
    OmxCreateMutex(&pCtx->awMutexHandler, OMX_FALSE);
    pCtx->mSemInData    = OmxCreateSem("InDataSem",    0, 0, OMX_FALSE);
    pCtx->mSemOutBuffer = OmxCreateSem("OutBufferSem", 0, 0, OMX_FALSE);
    pCtx->mSemValidPic  = OmxCreateSem("ValidPicSem",  0, 0, OMX_FALSE);

    pCtx->mIonFd = -1;
    pCtx->mIonFd = ion_open();
    logd("ion open fd = %d",pCtx->mIonFd);
    if(pCtx->mIonFd < -1)
    {
        loge("ion open fail ! ");
        return NULL;
    }
    pCtx->mGpuAlignStride = 32;
    //* get the phy offset
    int type = VE_OPS_TYPE_NORMAL;
    VeOpsS* veOps = GetVeOpsS(type);
    if(veOps == NULL)
    {
        loge("get ve ops failed");
    }
    else
    {
        VeConfig mVeConfig;
        memset(&mVeConfig, 0, sizeof(VeConfig));
        mVeConfig.nDecoderFlag = 1;

        void* pVeopsSelf = CdcVeInit(veOps,&mVeConfig);
        if(pVeopsSelf == NULL)
        {
            loge("init ve ops failed");
            CdcVeRelease(veOps, pVeopsSelf);
            pCtx->nPhyOffset = 0x40000000;
        }
        else
        {
            pCtx->nPhyOffset = CdcVeGetPhyOffset(veOps, pVeopsSelf);
            CdcVeRelease(veOps, pVeopsSelf);
        }
        logd("** nPhyOffset = %lx",pCtx->nPhyOffset);
    }

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

    freeOutputBufferInfoArr(pCtx);

    if(pCtx->mIonFd > 0)
    {
        ion_close(pCtx->mIonFd);
    }

    if(pCtx->decMemOps)
    {
        CdcMemClose(pCtx->decMemOps);
    }
    if(pCtx->secureMemOps != NULL)
    {
        CdcMemClose(pCtx->secureMemOps);
        pCtx->secureMemOps = NULL;
    }
    free(pCtx);
    pCtx = NULL;
}
