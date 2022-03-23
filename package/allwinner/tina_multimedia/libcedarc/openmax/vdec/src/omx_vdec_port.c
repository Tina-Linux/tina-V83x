/*
 * =====================================================================================
 *
 *       Filename:  omx_vdec_port.c
 *
 *    Description: 1. ports are malloced by omx_vdec
 *                 2. this file will be called by omx_vdec and omx_decoder
 *                 3. the ports' callback is the omx_vdec's callback
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

#define LOG_TAG "omx_vdec"
#include "log.h"

#include "omx_vdec_port.h"
#include <string.h>
#include <stdlib.h>
#include "omx_pub_def.h"
#include "omx_vdec_config.h"

void doEmptyThisBuffer(AwOmxVdecPort* mPort, OMX_BUFFERHEADERTYPE* pBufferHeader)
{
    if(isInputPort(mPort))
    {
        OmxAcquireMutex(mPort->m_bufMutex);
        if (mPort->m_sBufList.nSizeOfList < mPort->m_sBufList.nAllocSize)
        {
            mPort->m_sBufList.nSizeOfList++;
            mPort->m_sBufList.pBufHdrList[mPort->m_sBufList.nWritePos++]
                                = pBufferHeader;

            if (mPort->m_sBufList.nWritePos >= mPort->m_sBufList.nAllocSize)
                mPort->m_sBufList.nWritePos = 0;
        }

        OMX_LOGV("nTimeStamp[%lld], nAllocLen[%d], nFilledLen[%d],"
              "nOffset[%d], nFlags[0x%x], nOutputPortIndex[%d], nInputPortIndex[%d],"
            "mPort->m_sBufList.nSizeOfList:%ld",
            pBufferHeader->nTimeStamp,(int)pBufferHeader->nAllocLen,
            (int)pBufferHeader->nFilledLen,(int)pBufferHeader->nOffset,
            (int)pBufferHeader->nFlags,(int)pBufferHeader->nOutputPortIndex,
            (int)pBufferHeader->nInputPortIndex,
            mPort->m_sBufList.nSizeOfList);

        OmxReleaseMutex(mPort->m_bufMutex);
        //*Mark current buffer if there is outstanding command
        if (mPort->pMarkBuf)
        {
            pBufferHeader->hMarkTargetComponent = &mPort->mOmxCmp;
            pBufferHeader->pMarkData = mPort->pMarkBuf->pMarkData;
            mPort->pMarkBuf = NULL;
        }
    }
    else
    {
        OMX_LOGE("error port!!!!");
        return;
    }
}

void doSetPortMarkBuffer(AwOmxVdecPort* mPort, OMX_MARKTYPE* pMarkBuf)
{
    OMX_LOGD("mark buffer, port:%s", isInputPort(mPort)?"<<<<<<<<in":">>>>>>>out");
    if(!mPort->pMarkBuf)
        mPort->pMarkBuf = pMarkBuf;
}

void doFillThisBuffer(AwOmxVdecPort* mPort, OMX_BUFFERHEADERTYPE* pBufferHeader)
{
    OMX_LOGV("fill this buffer:%p", pBufferHeader);
    if(!isInputPort(mPort))
    {
        OmxAcquireMutex(mPort->m_bufMutex);
        if (mPort->m_sBufList.nSizeOfList < mPort->m_sBufList.nAllocSize)
        {
            mPort->m_sBufList.nSizeOfList++;
            mPort->m_sBufList.pBufHdrList[mPort->m_sBufList.nWritePos++] = pBufferHeader;

            if (mPort->m_sBufList.nWritePos >= mPort->m_sBufList.nAllocSize)
                mPort->m_sBufList.nWritePos = 0;
        }
        OmxReleaseMutex(mPort->m_bufMutex);
    }
    else
    {
        OMX_LOGE("error port!!!!");
        return;
    }
}

void doFlushPortBuffer(AwOmxVdecPort* mPort)
{
    OmxAcquireMutex(mPort->m_bufMutex);

    while (mPort->m_sBufList.nSizeOfList > 0)
    {
        mPort->m_sBufList.nSizeOfList--;
        if(isInputPort(mPort))
        {
            mPort->m_Callbacks.EmptyBufferDone(&mPort->mOmxCmp, mPort->m_pAppData,
                                 mPort->m_sBufList.pBufHdrList[mPort->m_sBufList.nReadPos++]);
        }
        else
        {
            mPort->m_Callbacks.FillBufferDone(&mPort->mOmxCmp, mPort->m_pAppData,
                                 mPort->m_sBufList.pBufHdrList[mPort->m_sBufList.nReadPos++]);
        }

        if (mPort->m_sBufList.nReadPos >= mPort->m_sBufList.nBufNeedSize)
            mPort->m_sBufList.nReadPos = 0;
    }

    mPort->m_sBufList.nReadPos  = 0;
    mPort->m_sBufList.nWritePos = 0;

    OmxReleaseMutex(mPort->m_bufMutex);
}

OMX_BUFFERHEADERTYPE* doRequestPortBuffer(AwOmxVdecPort* mPort)
{
    OMX_BUFFERHEADERTYPE* pBufHdr = NULL;

    OmxAcquireMutex(mPort->m_bufMutex);
    if(mPort->m_sBufList.nSizeOfList<=0)
    {
        OmxReleaseMutex(mPort->m_bufMutex);
        return NULL;
    }

    pBufHdr = mPort->m_sBufList.pBufHdrList[mPort->m_sBufList.nReadPos];

    if(pBufHdr == NULL)
    {
        OMX_LOGD("fatal error! pInBufHdr is NULL, check code!");
        OmxReleaseMutex(mPort->m_bufMutex);
        return NULL;
    }
    mPort->m_sBufList.nSizeOfList--;
    mPort->m_sBufList.nReadPos++;
    if (mPort->m_sBufList.nReadPos >= mPort->m_sBufList.nAllocSize)
        mPort->m_sBufList.nReadPos = 0;
    OmxReleaseMutex(mPort->m_bufMutex);
    return pBufHdr;

}

void returnPortBuffer(AwOmxVdecPort* mPort)
{
    OmxAcquireMutex(mPort->m_bufMutex);
    mPort->m_sBufList.nSizeOfList++;
    mPort->m_sBufList.nReadPos--;//OMX_U32
    if (mPort->m_sBufList.nReadPos > mPort->m_sBufList.nAllocSize)
        mPort->m_sBufList.nReadPos = mPort->m_sBufList.nAllocSize - 1;
    OmxReleaseMutex(mPort->m_bufMutex);
}

OMX_ERRORTYPE AwOmxVdecPortGetDefinitioin(AwOmxVdecPort* mPort,
                                          OMX_PARAM_PORTDEFINITIONTYPE *value)
{
    memcpy(value, &mPort->m_sPortDefType, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    OMX_LOGV("port:%s, width = %lu, height = %lu,"
          "nPortIndex[%lu], nBufferCountActual[%lu], nBufferCountMin[%lu], nBufferSize[%lu]",
         isInputPort(mPort)?"<<<<<<<<in":">>>>>>>out",
         mPort->m_sPortDefType.format.video.nFrameWidth,
         mPort->m_sPortDefType.format.video.nFrameHeight,
         mPort->m_sPortDefType.nPortIndex,
         mPort->m_sPortDefType.nBufferCountActual,
         mPort->m_sPortDefType.nBufferCountMin,
         mPort->m_sPortDefType.nBufferSize);

    if (mPort->m_sBufList.nAllocSize == mPort->m_sPortDefType.nBufferCountActual)
    {
        mPort->m_sPortDefType.bPopulated = OMX_TRUE;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE AwOmxVdecPortSetDefinitioin(AwOmxVdecPort* mPort,
                                          OMX_PARAM_PORTDEFINITIONTYPE *value)
{
    OMX_LOGD("port:%s,nBufferCountActual = %lu, mBufferCntActual = %lu",
        isInputPort(mPort)?"<<<<<<<<in":">>>>>>>out",
        value->nBufferCountActual, mPort->m_BufferCntActual);

    if(value->nBufferCountActual != mPort->m_BufferCntActual)
    {
        OMX_U32       nIndex;
        if(!isInputPort(mPort) && mPort->mExtraBufferNum == 0)
        {
            mPort->mExtraBufferNum = \
                    value->nBufferCountActual - mPort->m_BufferCntActual;
            if(mPort->mExtraBufferNum < 0)
            {
                OMX_LOGE("error: mExtraOutBufferNum is %ld, modify it to 4\n", mPort->mExtraBufferNum);
                mPort->mExtraBufferNum = DISPLAY_HOLH_BUFFER_NUM_DEFAULT;
            }
        }
        OMX_LOGD("gqy****port, extbuf:%ld", mPort->mExtraBufferNum);
        OmxAcquireMutex(mPort->m_bufMutex);

        if(mPort->m_sBufList.pBufArr != NULL)
            free(mPort->m_sBufList.pBufArr);

        if(mPort->m_sBufList.pBufHdrList != NULL)
            free(mPort->m_sBufList.pBufHdrList);

        OMX_LOGD("PORT:%s, x allocate %lu buffers.",
                isInputPort(mPort)?"<<<<<<<<in":">>>>>>>out",
                value->nBufferCountActual);
        mPort->m_sBufList.pBufArr
            = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE)* value->nBufferCountActual);
        mPort->m_sBufList.pBufHdrList
            = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*)* value->nBufferCountActual);
        for(nIndex = 0; nIndex < value->nBufferCountActual; nIndex++)
        {
            OMX_CONF_INIT_STRUCT_PTR (&mPort->m_sBufList.pBufArr[nIndex],
                                      OMX_BUFFERHEADERTYPE);
        }

        mPort->m_sBufList.nSizeOfList       = 0;
        mPort->m_sBufList.nAllocSize        = 0;
        mPort->m_sBufList.nWritePos         = 0;
        mPort->m_sBufList.nReadPos          = 0;
        mPort->m_sBufList.nAllocBySelfFlags = 0;
        mPort->m_sBufList.nBufNeedSize      = value->nBufferCountActual;
        mPort->m_sBufList.eDir              = isInputPort(mPort)?OMX_DirInput:OMX_DirOutput;
        OmxReleaseMutex(mPort->m_bufMutex);

    }
    mPort->m_BufferCntActual = value->nBufferCountActual;

    memcpy(&mPort->m_sPortDefType, value, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    OMX_LOGV("port:%s, width = %lu, height = %lu,"
          "nPortIndex[%lu], nBufferCountActual[%lu], nBufferCountMin[%lu], nBufferSize[%lu]",
         isInputPort(mPort)?"<<<<<<<<in":">>>>>>>out",
         mPort->m_sPortDefType.format.video.nFrameWidth,
         mPort->m_sPortDefType.format.video.nFrameHeight,
         mPort->m_sPortDefType.nPortIndex,
         mPort->m_sPortDefType.nBufferCountActual,
         mPort->m_sPortDefType.nBufferCountMin,
         mPort->m_sPortDefType.nBufferSize,
         mPort->m_sPortDefType.bEnabled?"bEnabled":"Disable",
         mPort->m_sPortDefType.bPopulated?"bPopulated":"Unpopulated");
    if (mPort->m_sBufList.nAllocSize < mPort->m_sPortDefType.nBufferCountActual)
    {
        mPort->m_sPortDefType.bPopulated = OMX_FALSE;
    }
    if (mPort->m_sPortDefType.bEnabled != mPort->bEnabled)
    {
        mPort->m_sPortDefType.bEnabled = mPort->bEnabled;
    }
    return OMX_ErrorNone;

}

OMX_ERRORTYPE AwOmxVdecPortSetFormat(AwOmxVdecPort* mPort, OMX_VIDEO_PARAM_PORTFORMATTYPE *value)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_LOGV(" OMX_IndexParamVideoPortFormat, port:%s",isInputPort(mPort)?"<<<<<<<<in":">>>>>>>out");

    if (value->nIndex > mPort->m_sPortFormatType.nIndex)
    {
        OMX_LOGE("erro: pParamData->nIndex > m_sPortFormatType.nIndex");
    }
    else
    {
        memcpy(&mPort->m_sPortFormatType, value, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    }
    return eError;
}

OMX_ERRORTYPE AwOmxVdecPortGetFormat(AwOmxVdecPort* mPort, OMX_VIDEO_PARAM_PORTFORMATTYPE *value)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_LOGV(" OMX_IndexParamVideoPortFormat, port:%s",isInputPort(mPort)?"<<<<<<<<in":">>>>>>>out");

    if (value->nIndex > mPort->m_sPortFormatType.nIndex)
    {
        OMX_LOGE("erro: pParamData->nIndex > m_sPortFormatType.nIndex");
        if(!isInputPort(mPort))
        {
            mPort->m_sPortFormatType.eColorFormat = COLOR_FORMAT_DEFAULT;
            memcpy(value, &mPort->m_sPortFormatType, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
        }
        else
            eError = OMX_ErrorNoMore;
    }
    else
    {
        memcpy(value, &mPort->m_sPortFormatType, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    }
    return eError;
}

OMX_ERRORTYPE AwOmxVdecPortGetBufferSupplier(AwOmxVdecPort* mPort,
                                          OMX_PARAM_BUFFERSUPPLIERTYPE *value)
{
    memcpy((void*)value, &mPort->m_sBufSupplierType, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
    return OMX_ErrorNone;
}

OMX_ERRORTYPE AwOmxVdecPortSetBufferSupplier(AwOmxVdecPort* mPort,
                                         OMX_PARAM_BUFFERSUPPLIERTYPE *value)
{
    memcpy(&mPort->m_sBufSupplierType, value, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
    return OMX_ErrorNone;
}

OMX_ERRORTYPE AwOmxVdecPortGetProfileLevel(AwOmxVdecPort* mPort,
                                  OMX_VIDEO_PARAM_PROFILELEVELTYPE *value)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    VIDEO_PROFILE_LEVEL_TYPE* pProfileLevel = NULL;
    OMX_U32 nNumberOfProfiles = 0;

    value->nPortIndex = getPortIndex(mPort);

    /* Choose table based on compression format */
    switch(mPort->m_sPortDefType.format.video.eCompressionFormat)
    {
    case OMX_VIDEO_CodingH263:
        pProfileLevel = SupportedH263ProfileLevels;
        nNumberOfProfiles
            = sizeof(SupportedH263ProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
        break;

    case OMX_VIDEO_CodingMPEG4:
        pProfileLevel = SupportedMPEG4ProfileLevels;
        nNumberOfProfiles
            = sizeof(SupportedMPEG4ProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
        break;

    case OMX_VIDEO_CodingAVC:
        pProfileLevel = SupportedAVCProfileLevels;
        nNumberOfProfiles
            = sizeof(SupportedAVCProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
        break;

    case OMX_VIDEO_CodingHEVC:
        pProfileLevel = SupportedHEVCProfileLevels;
        nNumberOfProfiles
            = sizeof(SupportedHEVCProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
        break;
    default:
        OMX_LOGW("OMX_IndexParamVideoProfileLevelQuerySupported, Format[0x%x] not support",
              mPort->m_sPortDefType.format.video.eCompressionFormat);
        return OMX_ErrorBadParameter;
    }

    if(((int)value->nProfileIndex < 0)
        || (value->nProfileIndex >= (nNumberOfProfiles - 1)))
    {
        OMX_LOGW("pParamProfileLevel->nProfileIndex[0x%x] error!",
             (unsigned int)value->nProfileIndex);
        return OMX_ErrorBadParameter;
    }

    /* Point to table entry based on index */
    pProfileLevel += value->nProfileIndex;

    /* -1 indicates end of table */
    if(pProfileLevel->nProfile != -1)
    {
        value->eProfile = pProfileLevel->nProfile;
        value->eLevel   = pProfileLevel->nLevel;
        eError = OMX_ErrorNone;
    }
    else
    {
        OMX_LOGW("pProfileLevel->nProfile error!");
        eError = OMX_ErrorNoMore;
    }
    return eError;
}

OMX_ERRORTYPE AwOmxVdecPortPopBuffer(AwOmxVdecPort* mPort,
                                        OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                        OMX_IN    OMX_PTR                pAppPrivate,
                                        OMX_IN    OMX_U32                nSizeBytes,
                                        OMX_IN    OMX_U8*                pBuffer,
                                        OMX_IN    OMX_BOOL               bAllocBySelfFlags)
{
    OMX_S8 nIndex = 0x0;
    OMX_LOGD("*******port pop buffer:%s", isInputPort(mPort)?"<<<<<<<<in":">>>>>>>out");

    OmxAcquireMutex(mPort->m_bufMutex);
    if(nSizeBytes != mPort->m_sPortDefType.nBufferSize || mPort->m_sPortDefType.bPopulated)
    {
        OmxReleaseMutex(mPort->m_bufMutex);
        return OMX_ErrorBadParameter;
    }

    if(mPort->m_sBufList.nAllocSize >= mPort->m_sBufList.nBufNeedSize)
    {
        OmxReleaseMutex(mPort->m_bufMutex);
        return OMX_ErrorInsufficientResources;
    }

    if(pBuffer == NULL)
    {
        OmxReleaseMutex(mPort->m_bufMutex);
        return OMX_ErrorInsufficientResources;
    }

    nIndex = mPort->m_sBufList.nAllocSize;
    OMX_BUFFERHEADERTYPE* pBufHdr = &mPort->m_sBufList.pBufArr[nIndex];
    pBufHdr->pBuffer              = pBuffer;
    if(!pBufHdr->pBuffer)
    {
        OmxReleaseMutex(mPort->m_bufMutex);
        return OMX_ErrorInsufficientResources;
    }

    if(bAllocBySelfFlags)
    {
        mPort->m_sBufList.nAllocBySelfFlags |= (1<<nIndex);
        mPort->bAllocBySelfFlags = OMX_TRUE;
    }
    else
    {
        mPort->bAllocBySelfFlags = OMX_FALSE;
    }

    pBufHdr->nAllocLen          = nSizeBytes;
    pBufHdr->pAppPrivate        = pAppPrivate;
    pBufHdr->nInputPortIndex    = isInputPort(mPort)?getPortIndex(mPort):kInvalidPortIndex;
    pBufHdr->nOutputPortIndex   = isInputPort(mPort)?kInvalidPortIndex:getPortIndex(mPort);
    pBufHdr->pInputPortPrivate  = isInputPort(mPort)?mPort:NULL;
    pBufHdr->pOutputPortPrivate = isInputPort(mPort)?NULL:mPort;

    *ppBufferHdr = pBufHdr;

    mPort->m_sBufList.nAllocSize++;

    if (mPort->m_sBufList.nAllocSize == mPort->m_sPortDefType.nBufferCountActual)
        mPort->m_sPortDefType.bPopulated = OMX_TRUE;

    OmxReleaseMutex(mPort->m_bufMutex);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE AwOmxVdecPortFreeBuffer(AwOmxVdecPort* mPort,
                                      OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr)
{

    OMX_LOGD("port:%s, pBufferHdr:%p", isInputPort(mPort)?"<<<<<<<<in":">>>>>>>out", pBufferHdr);

    OMX_U32 nIndex;

    OmxAcquireMutex(mPort->m_bufMutex);
    for(nIndex = 0; nIndex < mPort->m_sBufList.nBufNeedSize; nIndex++)
    {
        if(pBufferHdr == &mPort->m_sBufList.pBufArr[nIndex])
            break;
    }

    if(nIndex == mPort->m_sBufList.nBufNeedSize)
    {
        OmxReleaseMutex(mPort->m_bufMutex);
        return OMX_ErrorBadParameter;
    }
    if(mPort->m_sBufList.nAllocBySelfFlags & (1<<nIndex))
    {
        pBufferHdr->pBuffer = NULL;
        mPort->m_sBufList.nAllocBySelfFlags &= ~(1<<nIndex);
    }
    mPort->m_sBufList.nAllocSize--;

    if(mPort->m_sBufList.nAllocSize == 0)
        mPort->m_sPortDefType.bPopulated = OMX_FALSE;

    OmxReleaseMutex(mPort->m_bufMutex);
    return OMX_ErrorNone;

}

OMX_ERRORTYPE AwOmxVdecPortSetCallbacks(AwOmxVdecPort* mPort,
                                        OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
                                        OMX_IN  OMX_PTR pAppData)
{
    if(pCallbacks == NULL || pAppData == NULL)
        return OMX_ErrorBadParameter;
    memcpy(&mPort->m_Callbacks, pCallbacks, sizeof(OMX_CALLBACKTYPE));
    mPort->m_pAppData = pAppData;
    return OMX_ErrorNone;
}

OMX_ERRORTYPE AwOmxVdecInPortInit(AwOmxVdecPort* pInPort, OMX_VIDEO_CODINGTYPE type)
{
    OMX_U32       nIndex;
    pInPort->mExtraBufferNum    = 0;

    OmxCreateMutex(&pInPort->m_bufMutex, OMX_TRUE);
    // Initialize the video parameters for input port
    OMX_CONF_INIT_STRUCT_PTR(&pInPort->m_sPortDefType, OMX_PARAM_PORTDEFINITIONTYPE);
    pInPort->m_sPortDefType.nPortIndex                      = kInputPortIndex;
    pInPort->m_sPortDefType.bEnabled                        = OMX_TRUE;
    pInPort->bEnabled                                       = OMX_TRUE;
    pInPort->m_sPortDefType.bPopulated                      = OMX_FALSE;
    pInPort->m_sPortDefType.eDomain                         = OMX_PortDomainVideo;
    pInPort->m_sPortDefType.format.video.nFrameWidth        = WIDTH_DEFAULT;
    pInPort->m_sPortDefType.format.video.nFrameHeight       = HEIGHT_DEFAULT;
    pInPort->m_sPortDefType.eDir                            = OMX_DirInput;
    pInPort->m_sPortDefType.format.video.eCompressionFormat = type;
    pInPort->m_sPortDefType.format.video.cMIMEType          = (OMX_STRING)"";

#if 0
    if(pInPort->bIsSecureVideoFlag == OMX_TRUE)
    {
        pInPort->m_sPortDefType.nBufferCountMin    = NUM_IN_BUFFERS_SECURE;
        pInPort->m_sPortDefType.nBufferCountActual = NUM_IN_BUFFERS_SECURE;
        pInPort->m_sPortDefType.nBufferSize        = OMX_VIDEO_DEC_INPUT_BUFFER_SIZE_SECURE;
    }
    else
#endif
    {
        pInPort->m_sPortDefType.nBufferCountMin    = NUM_IN_BUFFERS;
        pInPort->m_sPortDefType.nBufferCountActual = NUM_IN_BUFFERS;
        pInPort->m_BufferCntActual                 = NUM_IN_BUFFERS;
        pInPort->m_sPortDefType.nBufferSize        = OMX_VIDEO_DEC_INPUT_BUFFER_SIZE;
    }

    // Initialize the video compression format for input port
    OMX_CONF_INIT_STRUCT_PTR(&pInPort->m_sPortFormatType, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    pInPort->m_sPortFormatType.nPortIndex         = kInputPortIndex;
    pInPort->m_sPortFormatType.nIndex             = 0x0;
    pInPort->m_sPortFormatType.eCompressionFormat = type;

    OMX_CONF_INIT_STRUCT_PTR(&pInPort->m_sBufSupplierType, OMX_PARAM_BUFFERSUPPLIERTYPE );
    pInPort->m_sBufSupplierType.nPortIndex  = kInputPortIndex;

    memset(&pInPort->m_sBufList, 0x0, sizeof(BufferList));

    pInPort->m_sBufList.pBufArr = \
        (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE)\
                                  *pInPort->m_sPortDefType.nBufferCountActual);
    if(pInPort->m_sBufList.pBufArr == NULL)
    {
        return OMX_ErrorInsufficientResources;
    }

    memset(pInPort->m_sBufList.pBufArr, 0,
           sizeof(OMX_BUFFERHEADERTYPE) * pInPort->m_sPortDefType.nBufferCountActual);
    for (nIndex = 0; nIndex < pInPort->m_sPortDefType.nBufferCountActual; nIndex++)
    {
        OMX_CONF_INIT_STRUCT_PTR (&pInPort->m_sBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
    }

    pInPort->m_sBufList.pBufHdrList = \
      (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*)*
                                            pInPort->m_sPortDefType.nBufferCountActual);
    if(pInPort->m_sBufList.pBufHdrList == NULL)
    {
        return OMX_ErrorInsufficientResources;
    }

    pInPort->m_sBufList.nSizeOfList       = 0;
    pInPort->m_sBufList.nAllocSize        = 0;
    pInPort->m_sBufList.nWritePos         = 0;
    pInPort->m_sBufList.nReadPos          = 0;
    pInPort->m_sBufList.nAllocBySelfFlags = 0;
    pInPort->m_sBufList.nBufNeedSize      = pInPort->m_sPortDefType.nBufferCountActual;
    pInPort->m_sBufList.eDir              = OMX_DirInput;

    return  OMX_ErrorNone;
}


OMX_ERRORTYPE AwOmxVdecOutPortInit(AwOmxVdecPort* pOutPort)
{
    OMX_U32  nIndex;
    pOutPort->mExtraBufferNum = 0;

    OmxCreateMutex(&pOutPort->m_bufMutex, OMX_TRUE);

    // Initialize the video parameters for output port
    OMX_CONF_INIT_STRUCT_PTR(&pOutPort->m_sPortDefType, OMX_PARAM_PORTDEFINITIONTYPE);
    pOutPort->m_sPortDefType.nPortIndex                = kOutputPortIndex;
    pOutPort->m_sPortDefType.bEnabled                  = OMX_TRUE;
    pOutPort->bEnabled                                 = OMX_TRUE;
    pOutPort->m_sPortDefType.bPopulated                = OMX_FALSE;
    pOutPort->m_sPortDefType.eDomain                   = OMX_PortDomainVideo;
    pOutPort->m_sPortDefType.format.video.cMIMEType    = (OMX_STRING)"YUV420";
    pOutPort->m_sPortDefType.format.video.nFrameWidth  = WIDTH_DEFAULT;
    pOutPort->m_sPortDefType.format.video.nFrameHeight = HEIGHT_DEFAULT;
    pOutPort->m_sPortDefType.eDir                      = OMX_DirOutput;
    pOutPort->m_sPortDefType.nBufferCountMin           = MAX_NUM_OUT_BUFFERS;
    pOutPort->m_sPortDefType.nBufferCountActual        = MAX_NUM_OUT_BUFFERS;
    pOutPort->m_BufferCntActual                        = MAX_NUM_OUT_BUFFERS;

    pOutPort->m_sPortDefType.nBufferSize =
    pOutPort->m_sPortDefType.format.video.nFrameWidth \
                *pOutPort->m_sPortDefType.format.video.nFrameHeight*3/2;

    pOutPort->m_sPortDefType.format.video.eColorFormat = COLOR_FORMAT_DEFAULT;

#if 0
    if(pOutPort->mIsSecureVideoFlag == OMX_TRUE)
    {
        pOutPort->m_sPortDefType.nBufferCountMin      = MAX_NUM_OUT_BUFFERS_SECURE;
        pOutPort->m_sPortDefType.nBufferCountActual   = MAX_NUM_OUT_BUFFERS_SECURE;
    }
    else
#endif
    {
        pOutPort->m_sPortDefType.nBufferCountMin      = MAX_NUM_OUT_BUFFERS;
        pOutPort->m_sPortDefType.nBufferCountActual   = MAX_NUM_OUT_BUFFERS;
    }

    // Initialize the compression format for output port
    OMX_CONF_INIT_STRUCT_PTR(&pOutPort->m_sPortFormatType, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    pOutPort->m_sPortFormatType.nPortIndex        = kOutputPortIndex;
    pOutPort->m_sPortFormatType.nIndex            = 0x0;

    pOutPort->m_sPortFormatType.eColorFormat      = COLOR_FORMAT_DEFAULT; //YV12

    OMX_CONF_INIT_STRUCT_PTR(&pOutPort->m_sBufSupplierType, OMX_PARAM_BUFFERSUPPLIERTYPE );
    pOutPort->m_sBufSupplierType.nPortIndex = kOutputPortIndex;

    // Initialize the output buffer list
    memset(&pOutPort->m_sBufList, 0x0, sizeof(BufferList));

    pOutPort->m_sBufList.pBufArr =
        (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE)* \
                                      pOutPort->m_sPortDefType.nBufferCountActual);
    if(pOutPort->m_sBufList.pBufArr == NULL)
    {
        return OMX_ErrorInsufficientResources;
    }

    memset(pOutPort->m_sBufList.pBufArr, 0,
           sizeof(OMX_BUFFERHEADERTYPE) * pOutPort->m_sPortDefType.nBufferCountActual);
    for (nIndex = 0; nIndex < pOutPort->m_sPortDefType.nBufferCountActual; nIndex++)
    {
        OMX_CONF_INIT_STRUCT_PTR(&pOutPort->m_sBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
    }

    pOutPort->m_sBufList.pBufHdrList =
        (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*)* \
                                      pOutPort->m_sPortDefType.nBufferCountActual);
    if(pOutPort->m_sBufList.pBufHdrList == NULL)
    {
        return OMX_ErrorInsufficientResources;
    }

    pOutPort->m_sBufList.nSizeOfList       = 0;
    pOutPort->m_sBufList.nAllocSize        = 0;
    pOutPort->m_sBufList.nWritePos         = 0;
    pOutPort->m_sBufList.nReadPos          = 0;
    pOutPort->m_sBufList.nAllocBySelfFlags = 0;
    pOutPort->m_sBufList.nBufNeedSize      = pOutPort->m_sPortDefType.nBufferCountActual;
    pOutPort->m_sBufList.eDir              = OMX_DirOutput;
    return OMX_ErrorNone;
}

OMX_ERRORTYPE AwOmxVdecPortDeinit(AwOmxVdecPort* mPort)
{
    OmxAcquireMutex(mPort->m_bufMutex);

    if (mPort->m_sBufList.pBufArr != NULL)
        free(mPort->m_sBufList.pBufArr);

    if (mPort->m_sBufList.pBufHdrList != NULL)
        free(mPort->m_sBufList.pBufHdrList);

    memset(&mPort->m_sBufList, 0, sizeof(struct _BufferList));
    mPort->m_sBufList.nBufNeedSize = mPort->m_sPortDefType.nBufferCountActual;

    OmxReleaseMutex(mPort->m_bufMutex);
    OmxDestroyMutex(&mPort->m_bufMutex);
    return OMX_ErrorNone;
}
