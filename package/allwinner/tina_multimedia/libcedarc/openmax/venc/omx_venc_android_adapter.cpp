/*
 * =====================================================================================
 *
 *      Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
 *      All rights reserved.
 *
 *       Filename:  omx_venc_android.cpp
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
#define LOG_TAG "omx_venc"
#include "MetadataBufferType.h"
#include <ion/ion.h>
#include <HardwareAPI.h>
#include <utils/CallStack.h>
#include <binder/IPCThreadState.h>
#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/GraphicBuffer.h>
#include <ui/Fence.h>
#include <cutils/properties.h>

#ifdef CONF_KITKAT_AND_NEWER
#include "hardware/hal_public.h"
#endif

#ifdef CONF_MARSHMALLOW_AND_NEWER
#include <OMX_IndexExt.h>
#endif
#include "omx_venc_adapter.h"

using namespace android;

#define FENCE_TIMEOUT_MS  1000
#define ALIGN_16B(x) (((x) + (15)) & ~(15))
#define SAVE_PICTURE (0)

#ifndef CONF_NOUGAT_AND_NEWER
#ifdef CONF_ARMV7_A_NEON

extern "C" void ImgRGBA2YUV420SP_neon(unsigned char *pu8RgbBuffer,
                                      unsigned char** pu8SrcYUV,
                                      int *l32Width_stride,
                                      int l32Height);
#endif
#endif

int parseEncInputBuffer(AwOmxVenc *impl,
                                      long long aw_buf_id,
                                      int share_fd,
                                      unsigned long *phy_addr)
{
    int i;
    int ret;
    aw_ion_user_handle_t ion_handle;

    for(i=0; i<NUM_MAX_IN_BUFFERS; i++)
    {
        //parse a new share fd
        if(-1 == impl->mInputBufInfo[i].nAwBufId)
        {
            impl->mInputBufInfo[i].nAwBufId = aw_buf_id;
            //get ion buffer handle
            ret = CdcIonImport(impl->mIonFd, share_fd, &ion_handle);
            if(ret < 0)
            {
                OMX_LOGE("use CdcIonImport get ion_handle error\n");
                return -1;
            }
            impl->mInputBufInfo[i].handle_ion = ion_handle;
            impl->mInputBufInfo[i].nShareFd = share_fd;

            //get ion buffer fd
            ret = CdcIonGetFd(impl->mIonFd, (uintptr_t)ion_handle);
            if(ret < 0)
            {
                OMX_LOGE("use CdcIonImport get ion_handle error\n");
                return -1;
            }
            impl->mInputBufInfo[i].nBufFd = ret;

            //get phy address
            if(CdcIonGetMemType() == MEMORY_IOMMU)
            {
                user_iommu_param sIommuBuf;
                sIommuBuf.fd = impl->mInputBufInfo[i].nBufFd;
                VideoEncoderGetVeIommuAddr(impl->m_encoder, &sIommuBuf);
                impl->mInputBufInfo[i].nBufPhyAddr = (unsigned long)sIommuBuf.iommu_addr;
            }
            else
            {
                impl->mInputBufInfo[i].nBufPhyAddr = CdcIonGetPhyAdr(impl->mIonFd,
                                                        (uintptr_t)ion_handle);
            }
            break;
        }
        //get already parsed share fd's index
        else if(aw_buf_id == impl->mInputBufInfo[i].nAwBufId)
            break;
    }
    if(NUM_MAX_IN_BUFFERS == i)
    {
        OMX_LOGE("the omx_venc inputBuffer num is bigger than NUM_MAX_IN_BUFFERS[%d]\n",
                                                    NUM_MAX_IN_BUFFERS);
        return -1;
    }

    *phy_addr = impl->mInputBufInfo[i].nBufPhyAddr;
    logv("mInputBufInfo[%d], nShareFd:%d, nBufPhyAddr:%lx, handle:%lx, nBufFd:%d\n",
                                            i,
                                            impl->mInputBufInfo[i].nShareFd,
                                            impl->mInputBufInfo[i].nBufPhyAddr,
                                            (long)impl->mInputBufInfo[i].handle_ion,
                                            impl->mInputBufInfo[i].nBufFd);
    return 0;
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
                    OMX_LOGE("CdcIonClose close buf fd error\n");
                    return -1;
                }
            }

            //free ion handle
            if(p->handle_ion)
            {
                ret = CdcIonFree(nIonFd, p->handle_ion);
                if(ret < 0)
                {
                    OMX_LOGE("CdcIonFree free ion_handle error\n");
                    return -1;
                }
            }

            p->nShareFd = -1;
        }
    }
    return 0;
}

//Since the CustomParams are set by setDeafaultParameter in Android case, they would be got only in Android case.
//TODO: figure out in linux case whether the custom params are required!!!
OMX_ERRORTYPE getDefaultParameter(AwOmxVenc* impl, OMX_IN OMX_INDEXTYPE eParamIndex,
                                           OMX_IN OMX_PTR pParamData)
{
    logv("function %s. line %d.", __FUNCTION__, __LINE__);
    OMX_ERRORTYPE eError = OMX_ErrorNone;
#ifdef CONF_MARSHMALLOW_AND_NEWER
    if((OMX_INDEXEXTTYPE)eParamIndex == OMX_IndexParamConsumerUsageBits)
    {
        logd("get_parameter: OMX_IndexParamConsumerUsageBits");
        OMX_U32 *usageBits = (OMX_U32 *)pParamData;
        *usageBits = GRALLOC_USAGE_HW_2D;
        return eError;
    }
#endif

    switch ((VIDENC_CUSTOM_INDEX)eParamIndex)
    {
        case VideoEncodeCustomParamextendedVideo:
        {
            logd("get_parameter: VideoEncodeCustomParamextendedVideo");
            memcpy(pParamData,&impl->mVideoExtParams,  sizeof(OMX_VIDEO_PARAMS_EXTENDED));
            break;
        }
        case VideoEncodeCustomParamextendedVideoSuperframe:
        {
            logd("get_parameter: VideoEncodeCustomParamextendedVideoSuperframe");
            memcpy(pParamData,&impl->mVideoSuperFrame,  sizeof(OMX_VIDEO_PARAMS_SUPER_FRAME));
            break;
        }
        case VideoEncodeCustomParamextendedVideoSVCSkip:
        {
            logd("get_parameter: VideoEncodeCustomParamextendedVideoSVCSkip");
            memcpy(pParamData, &impl->mVideoSVCSkip,  sizeof(OMX_VIDEO_PARAMS_SVC));
            break;
        }
        case VideoEncodeCustomParamextendedVideoVBR:
        {
            logd("get_parameter: VideoEncodeCustomParamextendedVideoVBR\n");
            memcpy(pParamData, &impl->mVideoVBR, sizeof(OMX_VIDEO_PARAMS_VBR));
            break;
        }
        case VideoEncodeCustomParamStoreANWBufferInMetadata:
        {
            break;
        }
        case VideoEncodeCustomParamextendedVideoPSkip:
        {
            logd("get_parameter: VideoEncodeCustomParamextendedVideoPSkip\n");
            break;
        }
        default:
        {
            switch ((OMX_INDEXEXTTYPE)eParamIndex)
            {
                case OMX_IndexParamVideoHevc:
                {
                    logv(" get_parameter: OMX_IndexParamVideoHevc");
                    OMX_VIDEO_PARAM_HEVCTYPE* pComponentParam
                                            = (OMX_VIDEO_PARAM_HEVCTYPE*)pParamData;

                    if (pComponentParam->nPortIndex == impl->m_sH265Type.nPortIndex)
                    {
                        memcpy(pComponentParam,
                                &impl->m_sH265Type, sizeof(OMX_VIDEO_PARAM_HEVCTYPE));
                    }
                    else
                    {
                        eError = OMX_ErrorBadPortIndex;
                    }
                    break;
                }
                default:
                {
                    OMX_LOGE("get_parameter: unknown param %p\n", pParamData);
                    eError = OMX_ErrorUnsupportedIndex;
                    break;
                }
            }
        }
        break;
    }
    return eError;
}

OMX_ERRORTYPE setDefaultParameter(AwOmxVenc* impl, OMX_IN OMX_INDEXTYPE eParamIndex,
                                           OMX_IN OMX_PTR pParamData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    logv("function %s. line %d. param index %d.", __FUNCTION__, __LINE__, (int)eParamIndex);
    switch ((VIDENC_CUSTOM_INDEX)eParamIndex)
    {
        case VideoEncodeCustomParamStoreMetaDataInBuffers:
        {
            OMX_BOOL bFlag = ((StoreMetaDataInBuffersParams*)pParamData)->bStoreMetaData;;
            if (((StoreMetaDataInBuffersParams*)pParamData)->nPortIndex == 0)
            {
                impl->m_useMetaDataInBuffers = bFlag;
            }
            else
            {
                if (bFlag)
                {
                    eError = OMX_ErrorUnsupportedSetting;
                }
            }
            logd("set_parameter: VideoEncodeCustomParamStoreMetaDataInBuffers %d",
                 impl->m_useMetaDataInBuffers);
            break;
        }

        case VideoEncodeCustomParamPrependSPSPPSToIDRFrames:
        {
            impl->m_prependSPSPPSToIDRFrames =
                ((PrependSPSPPSToIDRFramesParams*)pParamData)->bEnable;
            logd("set_parameter: VideoEncodeCustomParamPrependSPSPPSToIDRFrames %d",
                 impl->m_prependSPSPPSToIDRFrames);
            break;
        }

        case VideoEncodeCustomParamEnableAndroidNativeBuffers:
        {
            OMX_BOOL bFlag = ((EnableAndroidNativeBuffersParams*)pParamData)->enable;
            OMX_U32  index = ((EnableAndroidNativeBuffersParams*)pParamData)->nPortIndex;

            logd("set_parameter: VideoEncodeCustomParamEnableAndroidNativeBuffers, \
                 nPortIndex: %d,enable:%d", (int)index, (int)bFlag);
            if(index == 0)
                impl->m_useAndroidBuffer = bFlag;

            break;
        }
        case VideoEncodeCustomParamextendedVideo:
        {
            logd("set_parameter: VideoEncodeCustomParamextendedVideo");
            memcpy(&impl->mVideoExtParams, pParamData, sizeof(OMX_VIDEO_PARAMS_EXTENDED));
            break;
        }
        case VideoEncodeCustomParamextendedVideoSuperframe:
        {
            logd("set VideoEncodeCustomParamextendedVideoSuperframe");
            memcpy(&impl->mVideoSuperFrame, pParamData, sizeof(OMX_VIDEO_PARAMS_SUPER_FRAME));
            break;
        }
        case VideoEncodeCustomParamextendedVideoSVCSkip:
        {
            logd("set_parameter: VideoEncodeCustomParamextendedVideoSVCSkip\n");
            memcpy(&impl->mVideoSVCSkip, pParamData, sizeof(OMX_VIDEO_PARAMS_SVC));
            break;
        }
        case VideoEncodeCustomParamextendedVideoVBR:
        {
            logd("set_parameter: VideoEncodeCustomParamextendedVideoVBR\n");
            memcpy(&impl->mVideoVBR, pParamData, sizeof(OMX_VIDEO_PARAMS_VBR));
            break;
        }
        case VideoEncodeCustomParamStoreANWBufferInMetadata:
        {
            OMX_BOOL bFlag = ((StoreMetaDataInBuffersParams*)pParamData)->bStoreMetaData;
            OMX_U32  index = ((StoreMetaDataInBuffersParams*)pParamData)->nPortIndex;
            logd("set_parameter: VideoEncodeCustomParamStoreANWBufferInMetadata, flag: %d", bFlag);
            if(index == 0)
                impl->m_useAndroidBuffer = bFlag;
            break;
        }
        case VideoEncodeCustomParamextendedVideoPSkip:
        {
            logd("set_parameter: VideoEncodeCustomParamextendedVideoPSkip\n");
            impl->m_usePSkip = (*(OMX_BOOL *)pParamData);
            break;
        }
        default:
        {
            switch ((OMX_INDEXEXTTYPE)eParamIndex)
            {
                case OMX_IndexParamVideoHevc:
                {
                    logv(" set_parameter: OMX_IndexParamVideoHevc");
                    OMX_VIDEO_PARAM_HEVCTYPE* pComponentParam
                                            = (OMX_VIDEO_PARAM_HEVCTYPE*)pParamData;

                    if (pComponentParam->nPortIndex == impl->m_sH265Type.nPortIndex)
                    {
                        memcpy(&impl->m_sH265Type,
                                pComponentParam, sizeof(OMX_VIDEO_PARAM_HEVCTYPE));
                        //CalculateBufferSize(pCompPortOut->pPortDef, pComponentPrivate);
                    }
                    else
                    {
                        eError = OMX_ErrorBadPortIndex;
                    }
                    break;
                }
                default:
                {
                    OMX_LOGE("set_parameter: unknown param %x\n", eParamIndex);
                    eError = OMX_ErrorUnsupportedIndex;
                    break;
                }
            }
        }
    }
    return eError;
}

void getAndAddInputBuffer(AwOmxVenc* impl, OMX_BUFFERHEADERTYPE* pInBufHdr,
                                        VencInputBuffer* sInputBuffer)
{
    int buffer_type =  *(int*)(pInBufHdr->pBuffer+pInBufHdr->nOffset);
    buffer_handle_t bufferHanle = NULL;
    unsigned long phyaddress = 0;
    int ret = 0;
    int share_fd = 0;

#ifdef CONF_NOUGAT_AND_NEWER
    if((buffer_type == kMetadataBufferTypeGrallocSource) || \
        (buffer_type == kMetadataBufferTypeANWBuffer) || \
        (buffer_type == kMetadataBufferTypeNativeHandleSource))
#else
    if(buffer_type == kMetadataBufferTypeGrallocSource)
#endif
    {
        if (buffer_type == kMetadataBufferTypeGrallocSource)
        {
            bufferHanle = *(buffer_handle_t*)(pInBufHdr->pBuffer +
                                              pInBufHdr->nOffset + 4);
        }
#ifdef CONF_NOUGAT_AND_NEWER
        else if(buffer_type == kMetadataBufferTypeANWBuffer)
        {
            VideoNativeMetadata &nativeMeta = \
            *(VideoNativeMetadata *)(pInBufHdr->pBuffer + \
            pInBufHdr->nOffset);
            ANativeWindowBuffer *buffer = \
            (ANativeWindowBuffer *)nativeMeta.pBuffer;

            bufferHanle = buffer->handle;

            if (nativeMeta.nFenceFd >= 0) {
                sp<Fence> fence = new Fence(nativeMeta.nFenceFd);
                nativeMeta.nFenceFd = -1;
                status_t err = fence->wait(FENCE_TIMEOUT_MS);
                if (err != OK) {
                    AOMX_LOGE("Timed out waiting on input fence");
                    //return NULL;
                }
            }
        }
        else if(buffer_type == kMetadataBufferTypeNativeHandleSource)
        {
            VideoNativeHandleMetadata &nativeHandleMeta = \
            *(VideoNativeHandleMetadata *)(pInBufHdr->pBuffer + \
            pInBufHdr->nOffset);
            native_handle_t *nativeHandle = \
            (native_handle_t *)nativeHandleMeta.pHandle;
            bufferHanle = (buffer_handle_t)nativeHandle;
        }
#endif
        if (impl->m_sInPortFormatType.eColorFormat !=
            OMX_COLOR_FormatAndroidOpaque)
        {
            OMX_LOGW("do not support this input format: %d",
                 impl->m_sInPortFormatType.eColorFormat);
        }

        if(bufferHanle == NULL){
            OMX_LOGE("input buffer handle is null\n");
        }else{
        }
        if (bufferHanle)
        {
            int colorFormat;
            //for mali GPU
            private_handle_t* hnd = (private_handle_t *)(bufferHanle);
            colorFormat = hnd->format;

            switch (colorFormat)
            {
                case HAL_PIXEL_FORMAT_RGBA_8888:
                case HAL_PIXEL_FORMAT_RGBX_8888:
                {
                    if (impl->mFirstInputFrame)
                    {
                        impl->m_vencColorFormat = VENC_PIXEL_ABGR;
                        post_message_to_venc_and_wait(
                            impl, OMX_Venc_Cmd_ChangeColorFormat);
                        impl->mFirstInputFrame = OMX_FALSE;
                        logd("set input color format as ABGR");
                    }
                    break;
                }
                case HAL_PIXEL_FORMAT_YCrCb_420_SP:
                {
                    if (impl->mFirstInputFrame)
                    {
                        impl->m_vencColorFormat = VENC_PIXEL_YVU420SP;
                        post_message_to_venc_and_wait(
                            impl, OMX_Venc_Cmd_ChangeColorFormat);
                        impl->mFirstInputFrame = OMX_FALSE;
                        logd("set input color format as VENC_PIXEL_YVU420SP");
                    }
                    break;
                }
                case HAL_PIXEL_FORMAT_BGRA_8888:
                {
                    logv("do nothing, defalt is ARGB");
                    break;
                }
                default:
                {
                   OMX_LOGW("do not support this input format [0x%x]", colorFormat);
                    break;
                }
            }

            if (impl->mIonFd != -1)
            {
                int ret = 0;
                int share_fd = hnd->share_fd;
                long long aw_buf_id = hnd->aw_buf_id;
                ret = parseEncInputBuffer(impl,
                                            aw_buf_id,
                                            share_fd,
                                            &phyaddress);
                if(ret < 0)
                    OMX_LOGE("parseEncInputBuffer error\n");
            }
            else
            {
                OMX_LOGE("ion_open fail");
            }

        }
        else
        {
            OMX_LOGE("input bufferHanle is null");
        }

        logv("phyaddress: %lx", phyaddress);
        // only support ARGB now
        sInputBuffer->pAddrPhyY =
                      (unsigned char *)phyaddress;
        sInputBuffer->pAddrPhyC =
            sInputBuffer->pAddrPhyY +
            ALIGN_16B(impl->m_sInPortDefType.format.video.nStride) *
            ALIGN_16B(impl->m_sInPortDefType.format.video.nFrameHeight);

    }
    else
    {
        if (buffer_type != kMetadataBufferTypeCameraSource)
        {
            OMX_LOGW("skip this input buffer, error buffer type: %d",
                 buffer_type);
            /*impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp,
                                               impl->m_pAppData,
                                               pInBufHdr);*/
            callbackEmptyBufferDone(impl, pInBufHdr);
            //pInBufHdr = NULL;
        }
        else
        {
            memcpy(sInputBuffer,
                   (pInBufHdr->pBuffer+pInBufHdr->nOffset + 4),
                   sizeof(VencInputBuffer));

            share_fd = sInputBuffer->nShareBufFd;
            ret = parseEncInputBuffer(impl,
                    (long long)sInputBuffer->nID,
                    share_fd,
                    &phyaddress);
            if(ret < 0)
                OMX_LOGE("parseEncInputBuffer error\n");

            sInputBuffer->pAddrPhyY = (unsigned char*)phyaddress;
            sInputBuffer->pAddrPhyC = sInputBuffer->pAddrPhyY +
            impl->m_sInPortDefType.format.video.nStride *
            impl->m_sInPortDefType.format.video.nFrameHeight;
        }
    }

    //* clear flag
    sInputBuffer->nFlag = 0;
    sInputBuffer->nPts = pInBufHdr->nTimeStamp;
    sInputBuffer->nID = (unsigned long)pInBufHdr;

    if (pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
    {
        sInputBuffer->nFlag |= VENC_BUFFERFLAG_EOS;
    }

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

    int result = AddOneInputBuffer(impl->m_encoder, sInputBuffer);
    if (result!=0)
    {
        impl->mInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;
    }
    else
    {
        impl->mInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
    }
}

void getInputBufferFromBufHdr (AwOmxVenc* impl, OMX_BUFFERHEADERTYPE* pInBufHdr,
                                        VencInputBuffer* sInputBuffer)
{
    if (impl->m_sInPortFormatType.eColorFormat ==
        OMX_COLOR_FormatAndroidOpaque)
    {
        void* bufAddr;
        buffer_handle_t bufferHanle;

        int width;
        int height;

        //* get the argb buffer.
        bufferHanle = *(buffer_handle_t*)(pInBufHdr->pBuffer +
                                          pInBufHdr->nOffset + 4);
        android::Rect rect(
            (int)impl->m_sInPortDefType.format.video.nStride,
            (int)impl->m_sInPortDefType.format.video.nFrameHeight);
        GraphicBufferMapper::get().lock(
             bufferHanle,
             (GRALLOC_USAGE_HW_VIDEO_ENCODER |
                  GRALLOC_USAGE_SW_WRITE_OFTEN),
              rect, &bufAddr);

        width = impl->m_sInPortDefType.format.video.nStride;
        height = impl->m_sInPortDefType.format.video.nFrameHeight;
#ifndef CONF_NOUGAT_AND_NEWER
#ifdef CONF_ARMV7_A_NEON
        if (width % 32 == 0)
        {
            int widthandstride[2];
            unsigned char* addr[2];

            widthandstride[0] = width;
            widthandstride[1] = width;

            addr[0] = sInputBuffer->pAddrVirY;
            addr[1] = sInputBuffer->pAddrVirC;

            ImgRGBA2YUV420SP_neon((unsigned char *)bufAddr,
                                  addr, widthandstride, height);
        }
        else
        {
            int widthandstride[2];
            unsigned char* addr[2];

            widthandstride[0] = width;
            widthandstride[1] = (width + 31) & (~31);

            addr[0] = sInputBuffer->pAddrVirY;
            addr[1] = sInputBuffer->pAddrVirC;

            ImgRGBA2YUV420SP_neon((unsigned char *)bufAddr, addr,
                                  widthandstride, height);
        }
#endif
#endif

        CdcMemFlushCache(impl->memops, sInputBuffer->pAddrVirY,
                         width * height);
        CdcMemFlushCache(impl->memops, sInputBuffer->pAddrVirC,
                         width * height / 2);
        GraphicBufferMapper::get().unlock(bufferHanle);
    }
    else
    {
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
               pInBufHdr->pBuffer + pInBufHdr->nOffset + impl->mSizeY,
               impl->mSizeC);
    }
#if SAVE_PICTURE
    FILE* fp;
    char name[128];
    sprintf(name, "/data/camera/venc_pic_%lux%lu_%lu.dat",
            impl->m_sInPortDefType.format.video.nFrameWidth,
            impl->m_sInPortDefType.format.video.nFrameHeight,impl->m_nSavePicCount);
    fp = fopen(name, "ab");
    if(fp != NULL)
    {
        logd("saving picture, size: %lu x %lu, venc format: %d, count: %lu",
                impl->m_sInPortDefType.format.video.nFrameWidth,
                impl->m_sInPortDefType.format.video.nFrameHeight, impl->m_vencColorFormat,
                impl->m_nSavePicCount);
        OMX_U32 wh = impl->m_sInPortDefType.format.video.nFrameWidth*\
            impl->m_sInPortDefType.format.video.nFrameHeight;
        fwrite(sInputBuffer->pAddrVirY, 1, wh, fp);
        fwrite(sInputBuffer->pAddrVirC, 1, wh / 2, fp);
        fclose(fp);
        impl->m_nSavePicCount++;
    }
    else
    {
        OMX_LOGE("saving picture failed: open file error, frame number = %lu", impl->m_nSavePicCount);
    }
#endif

}

void getInputBufferFromBufHdrWithoutCrop (AwOmxVenc* impl, OMX_BUFFERHEADERTYPE* pInBufHdr,
                                        VencInputBuffer* sInputBuffer)
{
    getInputBufferFromBufHdr (impl, pInBufHdr, sInputBuffer);
}

void determineVencColorFormat(AwOmxVenc* impl)
{
    switch (impl->m_sInPortFormatType.eColorFormat)
    {
#ifdef CONF_NOUGAT_AND_NEWER
        //* android 7.0 cannot get mIsFromCts, so we extend OMX_COLOR_FormatYVU420SemiPlanar
        case OMX_COLOR_FormatYUV420SemiPlanar:
        {
            impl->m_vencColorFormat = VENC_PIXEL_YUV420SP;
            break;
        }
        case OMX_COLOR_FormatYVU420SemiPlanar:
        {
            impl->m_vencColorFormat = VENC_PIXEL_YVU420SP;
            break;
        }
#else
        case OMX_COLOR_FormatYUV420SemiPlanar:
        {
            if(impl->mIsFromCts)
            {
                impl->m_vencColorFormat = VENC_PIXEL_YUV420SP;
            }
            else//* from camera
            {
                impl->m_vencColorFormat = VENC_PIXEL_YVU420SP;
            }
            break;
        }
#endif
        case OMX_COLOR_FormatAndroidOpaque:
        {
            impl->m_vencColorFormat = VENC_PIXEL_ARGB; //maybe change this later;
            break;
        }
        default:
        {
            break;
        }
    }
        logd("venc color format [%d]. while venc YUV420SP [%d]. YVU420SP [%d]. ARGB [%d].",
            impl->m_vencColorFormat, VENC_PIXEL_YUV420SP, VENC_PIXEL_YVU420SP, VENC_PIXEL_ARGB);
        logd("Inport format [0x%08x]. while OMX YUV420SP [0x%08x], YVU420SP [0x%08x], Android-opaque [0x%08x].",
            impl->m_sInPortFormatType.eColorFormat, OMX_COLOR_FormatYUV420SemiPlanar,
            OMX_COLOR_FormatYVU420SemiPlanar, OMX_COLOR_FormatAndroidOpaque);
}

void determineVencBufStrideIfNecessary(AwOmxVenc* impl, VencBaseConfig* pBaseConfig)
{
    CEDARC_UNUSE(impl);
    CEDARC_UNUSE(pBaseConfig);
#ifdef CONF_IMG_GPU
    if (impl->m_sInPortFormatType.eColorFormat == OMX_COLOR_FormatAndroidOpaque)
        pBaseConfig->nStride = (pBaseConfig->nStride + 31) & (~31);
#endif
}

void updateOmxDebugFlag(AwOmxVenc* impl)
{
    impl->bSaveStreamFlag = OMX_FALSE;
    impl->bOpenStatisticFlag = OMX_FALSE;
    impl->bShowFrameSizeFlag = OMX_FALSE;
    #ifdef __ANDROID__
    char val[PROPERTY_VALUE_MAX];
    property_get("sys.media.video.debug", val, "0");
    if(strstr(val, "encstream")){
        impl->bSaveStreamFlag = OMX_TRUE;
        logd("include encstream, save enc stream in data/camera as bistream.dat");
    }
    if(strstr(val, "encstatistic")){
        impl->bOpenStatisticFlag = OMX_TRUE;
        logd("include encstatistic.");
    }
    if(strstr(val, "encframesize")){
        impl->bShowFrameSizeFlag = OMX_TRUE;
        logd("include encframesize.");
    }
    #endif
    #if SAVE_BITSTREAM
    impl->bSaveStreamFlag = OMX_TRUE;
    #endif
    #if OPEN_STATISTICS
    impl->bOpenStatisticFlag = OMX_TRUE;
    #endif
    #if PRINTF_FRAME_SIZE
    impl->bShowFrameSizeFlag = OMX_TRUE;
    #endif

}
