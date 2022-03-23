 /*
  * =====================================================================================
  *
  *      Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
  *      All rights reserved.
  *
  *       Filename:  omx_venc_linux_adapter.c
  *
  *    Description:
  *
  *        Version:  1.0
  *        Created:  2018/08
  *       Revision:  none
  *       Compiler:  gcc
  *
  *         Author:  SWC-PPD
  *      Company: Allwinner Technology Co. Ltd.
  *
  *       History :
  *        Author : AL3
  *        Date   : 2013/05/05
  *    Comment : complete the design code
  *
  * =====================================================================================
  */

#include "omx_venc_adapter.h"
OMX_ERRORTYPE getDefaultParameter(AwOmxVenc* impl, OMX_IN OMX_INDEXTYPE eParamIndex,
                                           OMX_IN OMX_PTR pParamData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    switch ((OMX_INDEXEXTTYPE)eParamIndex)
    {
        case OMX_IndexParamNalStreamFormat:
        {
            OMX_NALSTREAMFORMATTYPE* pComponentParam =
            (OMX_NALSTREAMFORMATTYPE*)(pParamData);
            memcpy(pComponentParam, &impl->m_avcNaluFormat, sizeof(OMX_NALSTREAMFORMATTYPE));
            logv("get_parameter: VideoEncodeCustomParamextendedAVCNaluFormat. Nalu format %s.",
                pComponentParam->eNaluFormat== OMX_NaluFormatFourByteInterleaveLength ?
                "Enable" : "Disable");
            break;
        }

        default:
        {
            eError = OMX_ErrorUnsupportedIndex;
        }
        break;
    }
    return eError;
}
OMX_ERRORTYPE setDefaultParameter(AwOmxVenc* impl, OMX_IN OMX_INDEXTYPE eParamIndex,
                                           OMX_IN OMX_PTR pParamData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    switch ((OMX_INDEXEXTTYPE)eParamIndex)
    {
        case OMX_IndexParamNalStreamFormat:
        {
            OMX_NALSTREAMFORMATTYPE* pComponentParam =
            (OMX_NALSTREAMFORMATTYPE*)(pParamData);
            if(pComponentParam->eNaluFormat != OMX_NaluFormatStartCodes &&
                pComponentParam->eNaluFormat != OMX_NaluFormatFourByteInterleaveLength)
            {
                eError =OMX_ErrorUnsupportedSetting;
                logw("set_parameter: omx only support OMX_NaluFormatStartCodes"
                    " and OMX_NaluFormatFourByteInterleaveLength");
                break;
            }

            logv("set_parameter: VideoEncodeCustomParamextendedAVCNaluFormat. Nalu format %s.",
                pComponentParam->eNaluFormat== OMX_NaluFormatFourByteInterleaveLength ?
                "Enable" : "Disable");
            memcpy(&impl->m_avcNaluFormat, pComponentParam, sizeof(OMX_NALSTREAMFORMATTYPE));
            break;
        }

        default:
        {
            eError = OMX_ErrorUnsupportedIndex;
        }
        break;
    }
    return eError;
}

void getAndAddInputBuffer(AwOmxVenc* impl, OMX_BUFFERHEADERTYPE* pInBufHdr,
                                    VencInputBuffer* sInputBuffer)
{
    CEDARC_UNUSE(impl);
    CEDARC_UNUSE(pInBufHdr);
    CEDARC_UNUSE(sInputBuffer);
    loge("getAndAddInputBuffer: do not support metadata input buffer");
}

void getInputBufferFromBufHdr (AwOmxVenc* impl, OMX_BUFFERHEADERTYPE* pInBufHdr,
                                                                VencInputBuffer* sInputBuffer)
{
    logv("function %s. line %d.", __FUNCTION__, __LINE__);
    if (impl->mVideoExtParams.bEnableCropping)
    {
        if (impl->mVideoExtParams.ui16CropLeft ||
            impl->mVideoExtParams.ui16CropTop)
        {
            sInputBuffer->bEnableCorp = 1;
            sInputBuffer->sCropInfo.nLeft =
                impl->mVideoExtParams.ui16CropLeft;
            sInputBuffer->sCropInfo.nWidth =
                impl->m_sOutPortDefType.format.video.nFrameWidth -
                impl->mVideoExtParams.ui16CropLeft -
                impl->mVideoExtParams.ui16CropRight;
            sInputBuffer->sCropInfo.nTop =
                impl->mVideoExtParams.ui16CropTop;
            sInputBuffer->sCropInfo.nHeight =
                impl->m_sOutPortDefType.format.video.nFrameHeight -
                impl->mVideoExtParams.ui16CropTop -
                impl->mVideoExtParams.ui16CropBottom;
        }
    }
    memcpy(sInputBuffer->pAddrVirY,
           pInBufHdr->pBuffer + pInBufHdr->nOffset, impl->mSizeY);
    memcpy(sInputBuffer->pAddrVirC,
           pInBufHdr->pBuffer + pInBufHdr->nOffset + impl->mSizeY, impl->mSizeC);
    impl->mSizeY = 0;
    impl->mSizeC = 0;

}

void getInputBufferFromBufHdrWithoutCrop(AwOmxVenc* impl, OMX_BUFFERHEADERTYPE* pInBufHdr,
                                                                VencInputBuffer* sInputBuffer)
{
    logv("function %s. line %d.", __FUNCTION__, __LINE__);
    memcpy(sInputBuffer->pAddrVirY,
           pInBufHdr->pBuffer + pInBufHdr->nOffset, impl->mSizeY);
    memcpy(sInputBuffer->pAddrVirC,
           pInBufHdr->pBuffer + pInBufHdr->nOffset + impl->mSizeY, impl->mSizeC);
    impl->mSizeY = 0;
    impl->mSizeC = 0;
}

void determineVencColorFormat(AwOmxVenc* impl)
{
    switch (impl->m_sInPortFormatType.eColorFormat)
    {
        case OMX_COLOR_FormatYUV420SemiPlanar:
        {
            OMX_LOGD("color format: VENC_PIXEL_YUV420SP/NV12");
            impl->m_vencColorFormat = VENC_PIXEL_YUV420SP;
            break;
        }
        case OMX_COLOR_FormatYVU420Planar:
        {
            OMX_LOGD("color format: VENC_PIXEL_YVU420P/YV12");
            impl->m_vencColorFormat = VENC_PIXEL_YVU420P;
            break;
        }
        case OMX_COLOR_FormatYVU420SemiPlanar:
        {
            OMX_LOGD("color format: VENC_PIXEL_YVU420SP/NV21");
            impl->m_vencColorFormat = VENC_PIXEL_YVU420SP;
            break;
        }
        case OMX_COLOR_FormatYUV420Planar:
        {
            OMX_LOGD("color format: VENC_PIXEL_YUV420P/I420");
            impl->m_vencColorFormat = VENC_PIXEL_YUV420P;
            break;
        }
        default:
        {
            logw("determine none format 0x%x!!!!!!!!!!!", impl->m_sInPortFormatType.eColorFormat);
            break;
        }

    }
}

void determineVencBufStrideIfNecessary(AwOmxVenc* impl, VencBaseConfig* pBaseConfig)
{
    CEDARC_UNUSE(impl);
    CEDARC_UNUSE(pBaseConfig);
    //do nothing.
}

int deparseEncInputBuffer(int nIonFd,
                                VideoEncoder *pEncoder,
                                OMXInputBufferInfoT *pInputBufInfo)
{
    OMX_S32 i;
    int ret;
    for(i=0; i<NUM_MAX_IN_BUFFERS; i++)
    {
        OMXInputBufferInfoT *p;
        p = pInputBufInfo + i;

        if(p->nAwBufId != -1)
        {
            if(CdcIonGetMemType() == MEMORY_IOMMU)
            {
                struct user_iommu_param sIommuBuf;
                sIommuBuf.fd = p->nBufFd;
                VideoEncoderFreeVeIommuAddr(pEncoder, &sIommuBuf);
             }

            //close buf fd
            if(p->nBufFd != -1)
            {
                ret = CdcIonClose(p->nBufFd);
                if(ret < 0)
                {
                    loge("CdcIonClose close buf fd error\n");
                    return -1;
                }
            }

            //free ion handle
            if(p->handle_ion)
            {
                ret = CdcIonFree(nIonFd, p->handle_ion);
                if(ret < 0)
                {
                    loge("CdcIonFree free ion_handle error\n");
                    return -1;
                }
            }

            p->nShareFd = -1;
        }
    }
    return 0;
}

void updateOmxDebugFlag(AwOmxVenc* impl)
{
    char* envchar = getenv("DEBUG_TYPE");
    OMX_LOGD("DEBUG_TYPE is %s.", envchar);
    if(envchar)
    {
        char* debugType = "SAVE_BIT";
        if(!strncasecmp(envchar, debugType, 8))
            impl->bSaveStreamFlag = OMX_TRUE;
        debugType = "SAVE_INPUT";
        if(!strncasecmp(envchar, debugType, 10))
            impl->bSaveInputDataFlag = OMX_TRUE;
        debugType = "OPEN_STATISTICS";
        if(!strncasecmp(envchar, debugType, 15))
            impl->bOpenStatisticFlag = OMX_TRUE;
        debugType = "SHOW_FRAMESIZE";
        if(!strncasecmp(envchar, debugType, 14))
            impl->bShowFrameSizeFlag = OMX_TRUE;
    }

    #if SAVE_BITSTREAM
    impl->bSaveStreamFlag = OMX_TRUE;
    #endif
    #if OPEN_STATISTICS
    impl->bOpenStatisticFlag = OMX_TRUE;
    #endif
    #if PRINTF_FRAME_SIZE
    impl->bShowFrameSizeFlag = OMX_TRUE;
    #endif
    #if SAVE_INPUTDATA
    impl->bSaveInputDataFlag = OMX_TRUE;
    #endif
}
