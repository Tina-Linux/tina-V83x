/*
 * Copyright (C) 2008-2015 Allwinner Technology Co. Ltd.
 * Author: Ning Fang <fangning@allwinnertech.com>
 *         Caoyuan Yang <yangcaoyuan@allwinnertech.com>
 *
 * This software is confidential and proprietary and may be used
 * only as expressly authorized by a licensing agreement from
 * Softwinner Products.
 *
 * The entire notice above must be reproduced on all copies
 * and should not be removed.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "vencoder.h"
#include "FrameBufferManager.h"
#include "venc_device.h"
#include "EncAdapter.h"
#include "veAdapter.h"
#include "memoryAdapter.h"
#include "CdcUtil.h"
#include "cdc_version.h"

//* should include other  '*.h'  before CdcMalloc.h
#include "CdcMalloc.h"

#define FRAME_BUFFER_NUM 4

typedef struct VencContext
{
   VENC_DEVICE*         pVEncDevice;
   void*                pEncoderHandle;
   FrameBufferManager*  pFBM;
   VencBaseConfig        baseConfig;
   unsigned int            nFrameBufferNum;
   VencHeaderData        headerData;
   VencInputBuffer      curEncInputbuffer;
   VENC_CODEC_TYPE         codecType;
   unsigned int         ICVersion;
   int                  bInit;
   struct ScMemOpsS    *memops;
   VeOpsS*           veOpsS;
   void*             pVeOpsSelf;
}VencContext;

extern struct ScMemOpsS* MemAdapterGetOpsS();

extern VENC_DEVICE video_encoder_h264_ver1;
extern VENC_DEVICE video_encoder_jpeg;
extern VENC_DEVICE video_encoder_h264_ver2;
extern VENC_DEVICE video_encoder_h265;
extern VENC_DEVICE video_encoder_vp8;

extern void* EncIspCreate();
extern void EncIspDestroy(void* isp);
extern int EncIspFunction(void* isp,
                         VencIspBufferInfo* pInBuffer,
                         VencIspBufferInfo* pOutBuffer,
                         VencIspFunction* pIspFunction);


VENC_DEVICE* video_encoder_devices[] =
{
    &video_encoder_h264_ver1,
    &video_encoder_jpeg,
    &video_encoder_h264_ver2,
    &video_encoder_h265,
    //&video_encoder_vp8,  // not any product use vp8 encoder now. (bz)
    0
};

static VENC_DEVICE *VencoderDeviceCreate(VENC_CODEC_TYPE type)
{
    VENC_DEVICE *vencoder_device_handle;

    vencoder_device_handle = (VENC_DEVICE *)malloc(sizeof(VENC_DEVICE));
    if(vencoder_device_handle == NULL)
    {
        return NULL;
    }

    memcpy(vencoder_device_handle, video_encoder_devices[type], sizeof(VENC_DEVICE));

    return vencoder_device_handle;
}
static void VencoderDeviceDestroy(void *handle)
{
    if (handle != NULL) {
        free(handle);
    }
}

VideoEncoder* VideoEncCreate(VENC_CODEC_TYPE eCodecType)
{
    CdcMallocCheckFlag();

    LogVersionInfo();

    VencContext* venc_ctx = NULL;

    venc_ctx = (VencContext*)malloc(sizeof(VencContext));
    if(!venc_ctx){
        loge("malloc VencContext fail!");
        return NULL;
    }

    memset(venc_ctx, 0,sizeof(VencContext));

    venc_ctx->nFrameBufferNum = FRAME_BUFFER_NUM;
    venc_ctx->codecType = eCodecType;
    venc_ctx->bInit = 0;

    int type = VE_OPS_TYPE_NORMAL;
    venc_ctx->veOpsS = GetVeOpsS(type);
    if(venc_ctx->veOpsS == NULL)
    {
        loge("get ve ops failed , type = %d",type);
        free(venc_ctx);
        return NULL;
    }

    venc_ctx->memops = MemAdapterGetOpsS();
    if (venc_ctx->memops == NULL)
    {
        loge("MemAdapterGetOpsS failed\n");
        free(venc_ctx);
        return NULL;
    }

    VeConfig mVeConfig;
    memset(&mVeConfig, 0, sizeof(VeConfig));
    mVeConfig.nDecoderFlag = 0;
    mVeConfig.nEncoderFlag = 1;
    mVeConfig.nEnableAfbcFlag = 0;
    mVeConfig.nFormat = eCodecType;
    mVeConfig.nWidth = 0;
    mVeConfig.nResetVeMode = 0;
    mVeConfig.memops = venc_ctx->memops;
    venc_ctx->pVeOpsSelf = CdcVeInit(venc_ctx->veOpsS, &mVeConfig);
    if(venc_ctx->pVeOpsSelf == NULL)
    {
        loge("init ve ops failed");
        CdcVeRelease(venc_ctx->veOpsS,venc_ctx->pVeOpsSelf);
        free(venc_ctx);
        return NULL;
    }

    char* baseAddr = CdcVeGetGroupRegAddr(venc_ctx->veOpsS,
                                          venc_ctx->pVeOpsSelf,
                                          REG_GROUP_VETOP);
    CdcVeLock(venc_ctx->veOpsS, venc_ctx->pVeOpsSelf);
    venc_ctx->ICVersion = EncAdapterGetICVersion(baseAddr);
    CdcVeUnLock(venc_ctx->veOpsS, venc_ctx->pVeOpsSelf);

    //for h264 enc use new driver from ic_1708
    if(eCodecType == VENC_CODEC_H264 && venc_ctx->ICVersion >= 0x1708)
        eCodecType = VENC_CODEC_H264_VER2;

    venc_ctx->pVEncDevice = VencoderDeviceCreate(eCodecType);
    if(venc_ctx->pVEncDevice == NULL)
    {
        loge("VencoderDeviceCreate failed\n");
        free(venc_ctx);
        return NULL;
    }

    if(EncAdapterInitializeMem(venc_ctx->memops) != 0)
      {
        loge("can not set up memory runtime environment.");
        free(venc_ctx);
        return NULL;
    }

    venc_ctx->baseConfig.veOpsS = venc_ctx->veOpsS;
    venc_ctx->baseConfig.pVeOpsSelf = venc_ctx->pVeOpsSelf;
    venc_ctx->baseConfig.memops = venc_ctx->memops;

    venc_ctx->pEncoderHandle =
                    venc_ctx->pVEncDevice->open(&venc_ctx->baseConfig, venc_ctx->ICVersion);
    if(!venc_ctx->pEncoderHandle)
    {
        logd("open VEncDevice error\n");
        VencoderDeviceDestroy(venc_ctx->pVEncDevice);
        venc_ctx->pVEncDevice = NULL;
        free(venc_ctx);
        return NULL;
    }

    return (VideoEncoder*)venc_ctx;
}

void VideoEncDestroy(VideoEncoder* pEncoder)
{
    VencContext* venc_ctx = (VencContext*)pEncoder;

    VideoEncUnInit(pEncoder);

    if(venc_ctx->pVEncDevice)
    {
        venc_ctx->pVEncDevice->close(venc_ctx->pEncoderHandle);
        VencoderDeviceDestroy(venc_ctx->pVEncDevice);
        venc_ctx->pVEncDevice = NULL;
        venc_ctx->pEncoderHandle = NULL;
    }

    EncAdpaterRelease(venc_ctx->memops);

    if(venc_ctx->veOpsS)
    {
        CdcVeRelease(venc_ctx->veOpsS, venc_ctx->pVeOpsSelf);
    }

    free(venc_ctx);

    CdcMallocReport();
}

int VideoEncInit(VideoEncoder* pEncoder, VencBaseConfig* pConfig)
{
    int result = 0;
    VencContext* venc_ctx = (VencContext*)pEncoder;

    if(pEncoder == NULL || pConfig == NULL || venc_ctx->bInit)
    {
        loge("InitVideoEncoder, param is NULL");
        return VENC_RESULT_NULL_PTR;
    }

    pConfig->memops = venc_ctx->memops;
    pConfig->veOpsS = venc_ctx->veOpsS;
    pConfig->pVeOpsSelf = venc_ctx->pVeOpsSelf;

    venc_ctx->pFBM = FrameBufferManagerCreate(venc_ctx->nFrameBufferNum, pConfig->memops,
                                    (void *)venc_ctx->veOpsS, venc_ctx->pVeOpsSelf);

    if(venc_ctx->pFBM == NULL)
    {

        loge("venc_ctx->pFBM == NULL");
        return VENC_RESULT_NO_MEMORY;
    }

    logd("(f:%s, l:%d)", __FUNCTION__, __LINE__);

    if(venc_ctx->ICVersion == 0x1639)
    {
        if(pConfig->nDstWidth >= 3840 || pConfig->nDstHeight>= 2160)
        {
            CdcVeInitEncoderPerformance(pConfig->veOpsS, pConfig->pVeOpsSelf, 1);
        }
        else
        {
            CdcVeInitEncoderPerformance(pConfig->veOpsS, pConfig->pVeOpsSelf, 0);
            logd("VeInitEncoderPerformance");
        }
    }

    logd("(f:%s, l:%d)", __FUNCTION__, __LINE__);

    memcpy(&venc_ctx->baseConfig, pConfig, sizeof(VencBaseConfig));

    venc_ctx->baseConfig.veOpsS = venc_ctx->veOpsS;
    venc_ctx->baseConfig.pVeOpsSelf = venc_ctx->pVeOpsSelf;

    result = venc_ctx->pVEncDevice->init(venc_ctx->pEncoderHandle, &venc_ctx->baseConfig);

    venc_ctx->bInit = 1;
    return result;
}

int VideoEncUnInit(VideoEncoder* pEncoder)
{
    VencContext* venc_ctx = (VencContext*)pEncoder;

    if(!venc_ctx->bInit)
    {
        logw("the VideoEnc is not init currently\n");
        return -1;
    }

     venc_ctx->pVEncDevice->uninit(venc_ctx->pEncoderHandle);

    if(venc_ctx->ICVersion == 0x1639)
    {
        if(venc_ctx->baseConfig.nDstWidth >= 3840 || venc_ctx->baseConfig.nDstHeight >= 2160)
        {
            CdcVeUnInitEncoderPerformance(venc_ctx->baseConfig.veOpsS,
                                          venc_ctx->baseConfig.pVeOpsSelf, 1);

        }
        else
        {
            CdcVeUnInitEncoderPerformance(venc_ctx->baseConfig.veOpsS,
                                          venc_ctx->baseConfig.pVeOpsSelf, 0);
            logd("VeUninitEncoderPerformance");
        }
    }

    if(venc_ctx->pFBM)
    {
        FrameBufferManagerDestroy(venc_ctx->pFBM);
        venc_ctx->pFBM = NULL;
    }

    venc_ctx->bInit = 0;
    return 0;
}

int AllocInputBuffer(VideoEncoder* pEncoder, VencAllocateBufferParam *pBufferParam)
{
    VencContext* venc_ctx = (VencContext*)pEncoder;

    if(pEncoder == NULL || pBufferParam == NULL)
    {
        loge("InitVideoEncoder, param is NULL");
        return VENC_RESULT_NULL_PTR;
    }

    if(venc_ctx->pFBM == NULL)
    {

        loge("venc_ctx->pFBM == NULL, must call InitVideoEncoder firstly");
        return VENC_RESULT_NO_RESOURCE;
    }

    if(AllocateInputBuffer(venc_ctx->pFBM, pBufferParam)!=0)
    {

        loge("allocat inputbuffer failed");
        return VENC_RESULT_NO_MEMORY;
    }

    return 0;
}

int GetOneAllocInputBuffer(VideoEncoder* pEncoder, VencInputBuffer* pInputbuffer)
{
    VencContext* venc_ctx = (VencContext*)pEncoder;
    int result = 0;

    if(venc_ctx == NULL)
    {
        loge("pEncoder == NULL");
        return VENC_RESULT_NULL_PTR;
    }

    result = GetOneAllocateInputBuffer(venc_ctx->pFBM, pInputbuffer);
    if(result != 0)
        loge("get one allocate inputbuffer failed");

    return result;
}

int FlushCacheAllocInputBuffer(VideoEncoder* pEncoder,  VencInputBuffer *pInputbuffer)
{
    VencContext* venc_ctx = (VencContext*)pEncoder;

    if(venc_ctx == NULL)
    {
        loge("pEncoder == NULL");
        return VENC_RESULT_NULL_PTR;
    }

    FlushCacheAllocateInputBuffer(venc_ctx->pFBM, pInputbuffer);

    return 0;
}

int ReturnOneAllocInputBuffer(VideoEncoder* pEncoder,  VencInputBuffer *pInputbuffer)
{
    VencContext* venc_ctx = (VencContext*)pEncoder;
    int result = 0;

    if(venc_ctx == NULL)
    {
        loge("pEncoder == NULL");
        return VENC_RESULT_NULL_PTR;
    }

    result = ReturnOneAllocateInputBuffer(venc_ctx->pFBM, pInputbuffer);
    if(result != 0)
        loge("ReturnOneAllocInputBuffer failed");

    return result;
}

int ReleaseAllocInputBuffer(VideoEncoder* pEncoder)
{
    CEDARC_UNUSE(ReleaseAllocInputBuffer);

    if(pEncoder == NULL)
    {
        loge("ReleaseAllocInputBuffer, pEncoder is NULL");
        return -1;
    }
    return 0;
}

int AddOneInputBuffer(VideoEncoder* pEncoder, VencInputBuffer* pBuffer)
{
    int result = 0;
    VencContext* venc_ctx = (VencContext*)pEncoder;

    if(venc_ctx == NULL || pBuffer == NULL)
    {
        loge("AddInputBuffer, param is NULL");
        return -1;
    }

    result = AddInputBuffer(venc_ctx->pFBM, pBuffer);

    return result;
}

int VideoEncodeOneFrame(VideoEncoder* pEncoder)
{
    int result = 0;
    VencContext* venc_ctx = (VencContext*)pEncoder;

    if(!venc_ctx) {
        return -1;
    }

    struct ScMemOpsS *_memops = venc_ctx->baseConfig.memops;
    unsigned int phyOffset = EncAdapterGetVeAddrOffset();

    if(GetInputBuffer(venc_ctx->pFBM, &venc_ctx->curEncInputbuffer) != 0)
    {
        return VENC_RESULT_NO_FRAME_BUFFER;
    }

    venc_ctx->curEncInputbuffer.pAddrPhyY -= phyOffset;
    venc_ctx->curEncInputbuffer.pAddrPhyC -= phyOffset;

    CdcVeLock(venc_ctx->veOpsS, venc_ctx->pVeOpsSelf);
    result = venc_ctx->pVEncDevice->encode(venc_ctx->pEncoderHandle, &venc_ctx->curEncInputbuffer);
    CdcVeUnLock(venc_ctx->veOpsS, venc_ctx->pVeOpsSelf);

    AddUsedInputBuffer(venc_ctx->pFBM, &venc_ctx->curEncInputbuffer);

    return result;
}

int AlreadyUsedInputBuffer(VideoEncoder* pEncoder, VencInputBuffer* pBuffer)
{
    int result = 0;
    VencContext* venc_ctx = (VencContext*)pEncoder;

    if(venc_ctx == NULL || pBuffer == NULL)
    {
        loge("AddInputBuffer, param is NULL");
        return -1;
    }

    result = GetUsedInputBuffer(venc_ctx->pFBM, pBuffer);
    return result;
}

int ValidBitstreamFrameNum(VideoEncoder* pEncoder)
{
    CEDARC_UNUSE(ValidBitstreamFrameNum);
    VencContext* venc_ctx = (VencContext*)pEncoder;
    return venc_ctx->pVEncDevice->ValidBitStreamFrameNum(venc_ctx->pEncoderHandle);
}

int GetOneBitstreamFrame(VideoEncoder* pEncoder, VencOutputBuffer* pBuffer)
{
    VencContext* venc_ctx = (VencContext*)pEncoder;

    if(!venc_ctx) {
        return -1;
    }

    if(venc_ctx->pVEncDevice->GetOneBitStreamFrame(venc_ctx->pEncoderHandle, pBuffer)!=0)
    {
        return VENC_RESULT_BITSTREAM_IS_EMPTY;
    }

    return 0;
}

int FreeOneBitStreamFrame(VideoEncoder* pEncoder, VencOutputBuffer* pBuffer)
{
    VencContext* venc_ctx = (VencContext*)pEncoder;

    if(!venc_ctx) {
        return -1;
    }

    if(venc_ctx->pVEncDevice->FreeOneBitStreamFrame(venc_ctx->pEncoderHandle, pBuffer)!=0)
    {
        return -1;
    }
    return 0;
}

int VideoEncoderReset(VideoEncoder* pEncoder)
{
    CEDARC_UNUSE(VideoEncoderReset);

    int result = 0;
    VencContext* venc_ctx = (VencContext*)pEncoder;

    if(!venc_ctx) {
        return -1;
    }

    //reset input buffer
    result = ResetFrameBuffer(venc_ctx->pFBM);
    if(result)
        return -1;

    //reset output buffer
    result = venc_ctx->pVEncDevice->ResetBitStreamFrame(venc_ctx->pEncoderHandle);
    if(result)
        return -1;

    return result;
}

unsigned int VideoEncoderGetUnencodedBufferNum(VideoEncoder* pEncoder)
{
    CEDARC_UNUSE(VideoEncoderGetUnencodedBufferNum);

    VencContext* venc_ctx = (VencContext*)pEncoder;

    if(!venc_ctx) {
        return -1;
    }
    return GetUnencodedBufferNum(venc_ctx->pFBM);
}

int VideoEncGetParameter(VideoEncoder* pEncoder, VENC_INDEXTYPE indexType, void* paramData)
{
    CEDARC_UNUSE(VideoEncGetParameter);
    VencContext* venc_ctx = (VencContext*)pEncoder;
    return venc_ctx->pVEncDevice->GetParameter(venc_ctx->pEncoderHandle, indexType, paramData);
}

int VideoEncSetParameter(VideoEncoder* pEncoder, VENC_INDEXTYPE indexType, void* paramData)
{
    VencContext* venc_ctx = (VencContext*)pEncoder;
    return venc_ctx->pVEncDevice->SetParameter(venc_ctx->pEncoderHandle, indexType, paramData);
}

void VideoEncoderGetVeIommuAddr(VideoEncoder* pEncoder, struct user_iommu_param *pIommuBuf)
{
    VencContext* venc_ctx = (VencContext*)pEncoder;
    CdcVeGetIommuAddr(venc_ctx->veOpsS, venc_ctx->pVeOpsSelf, pIommuBuf);
    return;
}
void VideoEncoderFreeVeIommuAddr(VideoEncoder* pEncoder, struct user_iommu_param *pIommuBuf)
{
    VencContext* venc_ctx = (VencContext*)pEncoder;
    CdcVeFreeIommuAddr(venc_ctx->veOpsS, venc_ctx->pVeOpsSelf, pIommuBuf);
    return;
}

int VideoEncoderSetFreq(VideoEncoder* pEncoder, int nVeFreq)
{
    CEDARC_UNUSE(VideoEncoderSetFreq);
    VencContext* venc_ctx = (VencContext*)pEncoder;
    int ret;
    ret = CdcVeSetSpeed(venc_ctx->veOpsS, venc_ctx->pVeOpsSelf, nVeFreq);
    if(ret < 0)
        loge("VideoEncoderSetFreq %d error, ret is %d\n", nVeFreq, ret);

    return ret;
}

void VideoEncoderSetDdrMode(VideoEncoder* pEncoder, int nDdrType)
{
    CEDARC_UNUSE(VideoEncoderSetDdrMode);
    VencContext* venc_ctx = (VencContext*)pEncoder;

    CdcVeSetDdrMode(venc_ctx->veOpsS, venc_ctx->pVeOpsSelf, nDdrType);

    return;
}


int AWJpecEnc(JpegEncInfo* pJpegInfo, EXIFInfo* pExifInfo, void* pOutBuffer, int* pOutBufferSize)
{
    CEDARC_UNUSE(AWJpecEnc);

    VencAllocateBufferParam bufferParam;
    VideoEncoder* pVideoEnc = NULL;
    VencInputBuffer inputBuffer;
    VencOutputBuffer outputBuffer;
    int result = 0;
    struct user_iommu_param iommu_buffer;
    int ion_fd = -1;
    aw_ion_user_handle_t  handle_ion = ION_USER_HANDLE_INIT_VALUE;
    int                   nBufFd = -1;
    unsigned char         memory_type;

    memory_type = CdcIonGetMemType();
    memset(&inputBuffer, 0, sizeof(VencInputBuffer));

    logd("memory_type[%d](0:normal, 1:iommu), pAddrPhyY[%p], pAddrPhyC[%p],\
    quality[%d], nShareBufFd[%d], cropFlag[%d]\n",
                                                        memory_type,
                                                        pJpegInfo->pAddrPhyY,
                                                        pJpegInfo->pAddrPhyC,
                                                        pJpegInfo->quality,
                                                        pJpegInfo->nShareBufFd,
                                                        pJpegInfo->bEnableCorp);
    logd("input_size[%dx%d], output_size[%dx%d], stride[%d], color_format[%d]\n",
                                                        pJpegInfo->sBaseInfo.nInputWidth,
                                                        pJpegInfo->sBaseInfo.nInputHeight,
                                                        pJpegInfo->sBaseInfo.nDstWidth,
                                                        pJpegInfo->sBaseInfo.nDstHeight,
                                                        pJpegInfo->sBaseInfo.nStride,
                                                        pJpegInfo->sBaseInfo.eInputFormat);
    logd("thumb_size[%dx%d]\n",
                                                        pExifInfo->ThumbWidth,
                                                        pExifInfo->ThumbHeight);

    pVideoEnc = VideoEncCreate(VENC_CODEC_JPEG);

    VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegExifInfo, pExifInfo);
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegQuality, &pJpegInfo->quality);

    if(VideoEncInit(pVideoEnc, &pJpegInfo->sBaseInfo) != 0)
    {
        result = -1;
        goto ERROR;
    }

    if(pJpegInfo->bNoUseAddrPhy)
    {
        bufferParam.nSizeY = pJpegInfo->sBaseInfo.nStride*pJpegInfo->sBaseInfo.nInputHeight;
        bufferParam.nSizeC = bufferParam.nSizeY>>1;
        bufferParam.nBufferNum = 1;

        if(AllocInputBuffer(pVideoEnc, &bufferParam)<0)
        {
            result = -1;
            goto ERROR;
        }

        GetOneAllocInputBuffer(pVideoEnc, &inputBuffer);
        memcpy(inputBuffer.pAddrVirY, pJpegInfo->pAddrPhyY, bufferParam.nSizeY);
        memcpy(inputBuffer.pAddrVirC, pJpegInfo->pAddrPhyC, bufferParam.nSizeC);

        FlushCacheAllocInputBuffer(pVideoEnc, &inputBuffer);
    }
    else
    {
        //get ion fd
        ion_fd = CdcIonOpen();
        if (ion_fd < 0)
        {
            loge("open ion fd failed \n");
            result = -1;
            goto ERROR;
        }

        //import ion buffer handle
        result = CdcIonImport(ion_fd, pJpegInfo->nShareBufFd, &handle_ion);
        if (result < 0)
        {
            loge("CdcIonImport get handle failed\n");
            result = -1;
            goto ERROR;
        }

        //get ion buffer fd
        nBufFd = CdcIonGetFd(ion_fd, (uintptr_t)handle_ion);
        if (nBufFd < 0)
        {
            loge("CdcIonGetFd get fd failed\n");
            result = -1;
            goto ERROR;
        }

        //get iommu addr
        if(memory_type == MEMORY_IOMMU)
        {
            iommu_buffer.fd = nBufFd;
            VideoEncoderGetVeIommuAddr(pVideoEnc, &iommu_buffer);
            inputBuffer.pAddrPhyY = (unsigned char*)(uintptr_t)iommu_buffer.iommu_addr;
            inputBuffer.pAddrPhyC = inputBuffer.pAddrPhyY +
                pJpegInfo->sBaseInfo.nStride * ((pJpegInfo->sBaseInfo.nInputHeight + 15) & (~15));
        }
        else
        {
            inputBuffer.pAddrPhyY = (unsigned char *) CdcIonGetPhyAdr(ion_fd,
                                                            (uintptr_t)handle_ion);
            inputBuffer.pAddrPhyC = inputBuffer.pAddrPhyY +
                pJpegInfo->sBaseInfo.nStride * pJpegInfo->sBaseInfo.nInputHeight;
        }
    }

    inputBuffer.bEnableCorp         = pJpegInfo->bEnableCorp;
    inputBuffer.sCropInfo.nLeft     =  pJpegInfo->sCropInfo.nLeft;
    inputBuffer.sCropInfo.nTop        =  pJpegInfo->sCropInfo.nTop;
    inputBuffer.sCropInfo.nWidth    =  pJpegInfo->sCropInfo.nWidth;
    inputBuffer.sCropInfo.nHeight   =  pJpegInfo->sCropInfo.nHeight;

    AddOneInputBuffer(pVideoEnc, &inputBuffer);
    if(VideoEncodeOneFrame(pVideoEnc)!= 0)
    {
        loge("jpeg encoder error");
    }

    if(!pJpegInfo->bNoUseAddrPhy)
    {
        //deattach buf fd from ve
        if(memory_type == MEMORY_IOMMU)
             VideoEncoderFreeVeIommuAddr(pVideoEnc, &iommu_buffer);

        //close buf fd
        if(nBufFd != -1)
        {
            result = CdcIonClose(nBufFd);
            if(result < 0)
            {
                loge("CdcIonClose close buf fd error\n");
                result = -1;
                goto ERROR;
            }
        }

        //free ion handle
        if(handle_ion)
        {
            result = CdcIonFree(ion_fd, handle_ion);
            if(result < 0)
            {
                loge("CdcIonFree free ion_handle error\n");
                goto ERROR;
            }
        }

        if (ion_fd != -1)
        {
            result = CdcIonClose(ion_fd);
            if(result < 0)
            {
                loge("CdcIonClose ion fd error\n");
                goto ERROR;
            }
            ion_fd = -1;
        }
    }
    AlreadyUsedInputBuffer(pVideoEnc,&inputBuffer);

    if(pJpegInfo->bNoUseAddrPhy)
    {
        ReturnOneAllocInputBuffer(pVideoEnc, &inputBuffer);
    }

    GetOneBitstreamFrame(pVideoEnc, &outputBuffer);

    memcpy(pOutBuffer, outputBuffer.pData0, outputBuffer.nSize0);
    if(outputBuffer.nSize1)
    {
        memcpy(((unsigned char*)pOutBuffer + outputBuffer.nSize0), \
            outputBuffer.pData1, outputBuffer.nSize1);

        *pOutBufferSize = outputBuffer.nSize0 + outputBuffer.nSize1;
    }
    else
    {
        *pOutBufferSize = outputBuffer.nSize0;
    }

    FreeOneBitStreamFrame(pVideoEnc, &outputBuffer);

ERROR:

    if(pVideoEnc)
    {
        VideoEncDestroy(pVideoEnc);
    }

    return result;
}

VideoEncoderIsp* VideoEncIspCreate()
{
    CdcMallocCheckFlag();

    return (VideoEncoderIsp*)EncIspCreate();
}

void VideoEncIspDestroy(VideoEncoderIsp* pEncIsp)
{
    EncIspDestroy((void*)pEncIsp);
    CdcMallocReport();
}

int VideoEncIspFunction(VideoEncoderIsp* pEncIsp,
                              VencIspBufferInfo* pInBuffer,
                              VencIspBufferInfo* pOutBuffer,
                              VencIspFunction* pIspFunction)
{

    int ret = EncIspFunction((void*)pEncIsp, pInBuffer, pOutBuffer, pIspFunction);

    return ret;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
