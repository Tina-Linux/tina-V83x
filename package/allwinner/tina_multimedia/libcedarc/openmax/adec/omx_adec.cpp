/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : omx_adec.c
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : init the design code
*/

/*============================================================================
                            O p e n M A X   w r a p p e r s
                             O p e n  M A X   C o r e

*//** @file omx_adec.cpp
  This module contains the implementation of the OpenMAX core & component.

*//*========================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////
// http://www.mime-type.net/

#include "log.h"

#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "omx_adec.h"
#include <fcntl.h>
#include "AWOMX_VideoIndexExtension.h"

#include "memoryAdapter.h"
#include "adecoder.h"

#include <ui/GraphicBufferMapper.h>
#include <ui/Rect.h>

#define debug logi("LINE %d, FUNCTION %s", __LINE__, __FUNCTION__);

#define OPEN_STATISTICS (0)

#ifdef __ANDROID__
    using namespace android;
#endif

#define OMX_ADEC_RESULT_OK    (0)
#define OMX_ADEC_RESULT_ERROR (-1)
#define OMX_ADEC_RESULT_EXIT  (1)
#define OMX_ADEC_RESULT_CONTINUE (2)

/*
 *     M A C R O S
 */

/*
 * Initializes a data structure using a pointer to the structure.
 * The initialization of OMX structures always sets up the nSize and nVersion fields
 *   of the structure.
 */
#define OMX_CONF_INIT_STRUCT_PTR(_variable_, _struct_name_)    \
    memset((_variable_), 0x0, sizeof(_struct_name_));    \
    (_variable_)->nSize = sizeof(_struct_name_);        \
    (_variable_)->nVersion.s.nVersionMajor = 0x1;    \
    (_variable_)->nVersion.s.nVersionMinor = 0x1;    \
    (_variable_)->nVersion.s.nRevision = 0x0;        \
    (_variable_)->nVersion.s.nStep = 0x0

static AUDIO_CUSTOM_PARAM sAudioDecCustomParams[] =
{
    {VIDDEC_CUSTOMPARAM_ENABLE_ANDROID_NATIVE_BUFFER,
        (OMX_INDEXTYPE)AWOMX_IndexParamVideoEnableAndroidNativeBuffers},
    {VIDDEC_CUSTOMPARAM_GET_ANDROID_NATIVE_BUFFER_USAGE,
        (OMX_INDEXTYPE)AWOMX_IndexParamVideoGetAndroidNativeBufferUsage},
    {VIDDEC_CUSTOMPARAM_USE_ANDROID_NATIVE_BUFFER2,
        (OMX_INDEXTYPE)AWOMX_IndexParamVideoUseAndroidNativeBuffer2}
};


static void* ComponentThread(void* pThreadData);


//* factory function executed by the core to create instances
void *get_omx_component_factory_fn(void)
{
    CEDARC_UNUSE(get_omx_component_factory_fn);
    return (new omx_adec);
}

omx_adec::omx_adec()
{
    logd("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    m_state              = OMX_StateLoaded;
    m_cRole[0]           = 0;
    m_cName[0]           = 0;
    m_eCompressionFormat = OMX_AUDIO_CodingPCM;//OMX_AUDIO_CodingUnused;
    m_pAppData           = NULL;
    m_thread_id          = 0;
    m_cmdpipe[0]         = 0;
    m_cmdpipe[1]         = 0;
    m_cmddatapipe[0]     = 0;
    m_cmddatapipe[1]     = 0;
    m_channel            = 0;
    m_sampleRate         = 0;
    m_channels           = 0;
    m_codec_tag          = 0;
    m_blockalign         = 0;
    m_bitspersample      = 0;
    m_bitRate            = 0;

    m_decoder            = NULL;
    mEosFlag             = OMX_FALSE;
    pMarkData            = NULL;
    hMarkTargetComponent = NULL;
    port_setting_match   = OMX_TRUE;
    pMarkBuf             = NULL;

    memset(&m_Callbacks, 0, sizeof(m_Callbacks));
    memset(&m_sInPortDef, 0, sizeof(m_sInPortDef));
    memset(&m_sOutPortDef, 0, sizeof(m_sOutPortDef));
    memset(&m_sInPortFormat, 0, sizeof(m_sInPortFormat));
    memset(&m_sOutPortFormat, 0, sizeof(m_sOutPortFormat));
    memset(&m_sPriorityMgmt, 0, sizeof(m_sPriorityMgmt));
    memset(&m_sInBufSupplier, 0, sizeof(m_sInBufSupplier));
    memset(&m_sOutBufSupplier, 0, sizeof(m_sOutBufSupplier));
    memset(&m_sInBufList, 0, sizeof(m_sInBufList));
    memset(&m_sOutBufList, 0, sizeof(m_sOutBufList));

    memset(&m_streamInfo, 0, sizeof(m_streamInfo));

    m_useAndroidBuffer = OMX_FALSE;
    m_nInportPrevTimeStamp = 0;
    m_JumpFlag = OMX_FALSE;
    mFirstInputDataFlag       = OMX_TRUE;
    pthread_mutex_init(&m_inBufMutex, NULL);
    pthread_mutex_init(&m_outBufMutex, NULL);

}


omx_adec::~omx_adec()
{
    OMX_S32 nIndex;
    // In case the client crashes, check for nAllocSize parameter.
    // If this is greater than zero, there are elements in the list that are not free'd.
    // In that case, free the elements.
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    pthread_mutex_lock(&m_inBufMutex);

    for(nIndex=0; nIndex<m_sInBufList.nBufArrSize; nIndex++)
    {
        if(m_sInBufList.pBufArr==NULL)
        {
            break;
        }

        if(m_sInBufList.pBufArr[nIndex].pBuffer != NULL)
        {
            if(m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
            {
                free(m_sInBufList.pBufArr[nIndex].pBuffer);
                m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
            }
        }
    }

    if (m_sInBufList.pBufArr != NULL)
    {
        free(m_sInBufList.pBufArr);
    }

    if (m_sInBufList.pBufHdrList != NULL)
    {
        free(m_sInBufList.pBufHdrList);
    }

    memset(&m_sInBufList, 0, sizeof(struct _BufferList));
    m_sInBufList.nBufArrSize = m_sInPortDef.nBufferCountActual;

    pthread_mutex_unlock(&m_inBufMutex);

    pthread_mutex_lock(&m_outBufMutex);

    for(nIndex=0; nIndex<m_sOutBufList.nBufArrSize; nIndex++)
    {
        if(m_sOutBufList.pBufArr[nIndex].pBuffer != NULL)
        {
            if(m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
            {
                free(m_sOutBufList.pBufArr[nIndex].pBuffer);
                m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
            }
        }
    }

    if (m_sOutBufList.pBufArr != NULL)
    {
        free(m_sOutBufList.pBufArr);
    }

    if (m_sOutBufList.pBufHdrList != NULL)
    {
        free(m_sOutBufList.pBufHdrList);
    }

    memset(&m_sOutBufList, 0, sizeof(struct _BufferList));
    m_sOutBufList.nBufArrSize = m_sOutPortDef.nBufferCountActual;

    pthread_mutex_unlock(&m_outBufMutex);

    if(m_decoder != NULL)
    {
        DestroyAudioDecoder(m_decoder);
        m_decoder = NULL;
        mFirstInputDataFlag = OMX_TRUE;
        logd("DestroyAudioDecoder");
        logd("*****mFirstInputDataFlag = OMX_TRUE");
    }

    pthread_mutex_destroy(&m_inBufMutex);
    pthread_mutex_destroy(&m_outBufMutex);

    logd("~omx_dec done!");
}


OMX_ERRORTYPE omx_adec::component_init(OMX_STRING pName)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    int           err;
    OMX_U32       nIndex;

    logd("(f:%s, l:%d) name = %s", __FUNCTION__, __LINE__, pName);
    strncpy((char*)m_cName, pName, OMX_MAX_STRINGNAME_SIZE);


    if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.wma", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.wma", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingWMA;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_WMA_STANDARD;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/wma");
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.aac", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.aac", OMX_MAX_STRINGNAME_SIZE);
        //OMX_AUDIO_CodingAAC;//OMX_AUDIO_CodingPCM;
        m_eCompressionFormat      = OMX_AUDIO_CodingAAC;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_MPEG_AAC_LC;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/aac");
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.mp3", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.mp3", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingMP3;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_MP3;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/mp3");
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.amrnb", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.amrnb", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingAMR;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_AMR;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/3gpp");
        m_sampleRate = 8000;
        m_channel = 1;
        m_streamInfo.eSubCodecFormat = AMR_FORMAT_NARROWBAND;
        m_codec_tag = AMR_FORMAT_NARROWBAND;
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.amrwb", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.amrwb", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingAMR;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_AMR;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/amr-wb");
        m_sampleRate = 16000;
        m_channel = 1;
        m_streamInfo.eSubCodecFormat = AMR_FORMAT_WIDEBAND;
        m_codec_tag = AMR_FORMAT_WIDEBAND;
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.g711.alaw",
              OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.g711alaw", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingG711;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_LPCM_A;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/g711-alaw");
        m_streamInfo.eSubCodecFormat = WAVE_FORMAT_ALAW;
        m_codec_tag = WAVE_FORMAT_ALAW;
        m_bitspersample = 8;
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.g711.mlaw",
              OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.g711mlaw", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingG711;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_LPCM_V;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/g711-mlaw");
        m_streamInfo.eSubCodecFormat = WAVE_FORMAT_MULAW;
        m_codec_tag = WAVE_FORMAT_MULAW;
        m_bitspersample = 8;
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.adpcm",
              OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.adpcm", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingADPCM;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_ADPCM;
        //("audio/x-adpcm-ms");
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/x-adpcm-dvi-ima");
        m_streamInfo.eSubCodecFormat = WAVE_FORMAT_DVI_ADPCM;
        m_codec_tag = WAVE_FORMAT_DVI_ADPCM;
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.vorbis",
              OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.ogg", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingVORBIS;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_OGG;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/ogg");
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.ape", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.ape", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingAPE;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_APE;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/ape");
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.dts", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.dts", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingDTS;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_DTS;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/dts");
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.raw", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.raw", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingPCM;
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/raw");
        m_streamInfo.eSubCodecFormat = WAVE_FORMAT_PCM;
        m_codec_tag = WAVE_FORMAT_PCM;
    }
    else  if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.flac", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "audio_decoder.flac", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_AUDIO_CodingFLAC;//FLAC???????
        m_streamInfo.eCodecFormat = AUDIO_CODEC_FORMAT_FLAC;
        m_sInPortDef.format.audio.cMIMEType  = const_cast<char *>("audio/flac");
    }
    else
    {
        logi("\nERROR:Unknown Component\n");
        eRet = OMX_ErrorInvalidComponentName;
        return eRet;
    }

    // Initialize component data structures to default values
    OMX_CONF_INIT_STRUCT_PTR(&m_sPortParam, OMX_PORT_PARAM_TYPE);
    m_sPortParam.nPorts           = 0x2;
    m_sPortParam.nStartPortNumber = 0x0;

    // Initialize the audio parameters for input port
    OMX_CONF_INIT_STRUCT_PTR(&m_sInPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    m_sInPortDef.nPortIndex = 0;
    m_sInPortDef.eDir = OMX_DirInput;
    m_sInPortDef.nBufferCountMin = NUM_IN_BUFFERS;
    m_sInPortDef.nBufferCountActual = m_sInPortDef.nBufferCountMin;
    m_sInPortDef.nBufferSize = OMX_VIDEO_DEC_INPUT_BUFFER_SIZE;
    m_sInPortDef.bEnabled = OMX_TRUE;
    m_sInPortDef.bPopulated = OMX_FALSE;
    m_sInPortDef.eDomain = OMX_PortDomainAudio;
    m_sInPortDef.bBuffersContiguous = OMX_FALSE;
    m_sInPortDef.nBufferAlignment = 1;

    //m_sInPortDef.format.audio.cMIMEType = const_cast<char *>("audio/aac");
    m_sInPortDef.format.audio.pNativeRender = NULL;
    m_sInPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    m_sInPortDef.format.audio.eEncoding = m_eCompressionFormat;

    // Initialize the audio parameters for output port
    OMX_CONF_INIT_STRUCT_PTR(&m_sOutPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    m_sOutPortDef.nPortIndex = 1;
    m_sOutPortDef.eDir = OMX_DirOutput;
    m_sOutPortDef.nBufferCountMin = NUM_OUT_BUFFERS;
    m_sOutPortDef.nBufferCountActual = m_sOutPortDef.nBufferCountMin;
    m_sOutPortDef.nBufferSize = OMX_VIDEO_DEC_OUTPUT_BUFFER_SIZE;
    m_sOutPortDef.bEnabled = OMX_TRUE;
    m_sOutPortDef.bPopulated = OMX_FALSE;
    m_sOutPortDef.eDomain = OMX_PortDomainAudio;
    m_sOutPortDef.bBuffersContiguous = OMX_FALSE;
    m_sOutPortDef.nBufferAlignment = 2;

    m_sOutPortDef.format.audio.cMIMEType = const_cast<char *>("audio/raw");
    m_sOutPortDef.format.audio.pNativeRender = NULL;
    m_sOutPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    m_sOutPortDef.format.audio.eEncoding = OMX_AUDIO_CodingPCM;


    // Initialize the audio compression format for input port
    OMX_CONF_INIT_STRUCT_PTR(&m_sInPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
    m_sInPortFormat.nPortIndex         = 0x0;
    m_sInPortFormat.nIndex             = 0x0;
    m_sInPortFormat.eEncoding = m_eCompressionFormat;

    // Initialize the compression format for output port
    OMX_CONF_INIT_STRUCT_PTR(&m_sOutPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
    m_sOutPortFormat.nPortIndex        = 0x1;
    m_sOutPortFormat.nIndex            = 0x0;
    m_sOutPortFormat.eEncoding = OMX_AUDIO_CodingPCM;


    OMX_CONF_INIT_STRUCT_PTR(&m_sPriorityMgmt, OMX_PRIORITYMGMTTYPE);

    OMX_CONF_INIT_STRUCT_PTR(&m_sInBufSupplier, OMX_PARAM_BUFFERSUPPLIERTYPE );
    m_sInBufSupplier.nPortIndex  = 0x0;

    OMX_CONF_INIT_STRUCT_PTR(&m_sOutBufSupplier, OMX_PARAM_BUFFERSUPPLIERTYPE );
    m_sOutBufSupplier.nPortIndex = 0x1;

    // Initialize the input buffer list
    memset(&(m_sInBufList), 0x0, sizeof(BufferList));

    m_sInBufList.pBufArr = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) *
                                                         m_sInPortDef.nBufferCountActual);
    if(m_sInBufList.pBufArr == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    memset(m_sInBufList.pBufArr, 0,
           sizeof(OMX_BUFFERHEADERTYPE) * m_sInPortDef.nBufferCountActual);
    for (nIndex = 0; nIndex < m_sInPortDef.nBufferCountActual; nIndex++)
    {
        OMX_CONF_INIT_STRUCT_PTR (&m_sInBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
    }


    m_sInBufList.pBufHdrList = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*) *
                                                              m_sInPortDef.nBufferCountActual);
    if(m_sInBufList.pBufHdrList == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    m_sInBufList.nSizeOfList       = 0;
    m_sInBufList.nAllocSize        = 0;
    m_sInBufList.nWritePos         = 0;
    m_sInBufList.nReadPos          = 0;
    m_sInBufList.nAllocBySelfFlags = 0;
    m_sInBufList.nBufArrSize       = m_sInPortDef.nBufferCountActual;
    m_sInBufList.eDir              = OMX_DirInput;

    // Initialize the output buffer list
    memset(&m_sOutBufList, 0x0, sizeof(BufferList));

    m_sOutBufList.pBufArr = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) *
                                                          m_sOutPortDef.nBufferCountActual);
    if(m_sOutBufList.pBufArr == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    memset(m_sOutBufList.pBufArr, 0,
           sizeof(OMX_BUFFERHEADERTYPE) * m_sOutPortDef.nBufferCountActual);
    for (nIndex = 0; nIndex < m_sOutPortDef.nBufferCountActual; nIndex++)
    {
        OMX_CONF_INIT_STRUCT_PTR(&m_sOutBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
    }

    m_sOutBufList.pBufHdrList = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*) *
                                                               m_sOutPortDef.nBufferCountActual);
    if(m_sOutBufList.pBufHdrList == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    m_sOutBufList.nSizeOfList       = 0;
    m_sOutBufList.nAllocSize        = 0;
    m_sOutBufList.nWritePos         = 0;
    m_sOutBufList.nReadPos          = 0;
    m_sOutBufList.nAllocBySelfFlags = 0;
    m_sOutBufList.nBufArrSize       = m_sOutPortDef.nBufferCountActual;
    m_sOutBufList.eDir              = OMX_DirOutput;

    // Create the pipe used to send commands to the thread
    err = pipe(m_cmdpipe);
    if (err)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }


    //    if (ve_mutex_init(&m_cedarv_req_ctx, CEDARV_DECODE) < 0) {
    //        ALOGE("cedarv_req_ctx init fail!!");
    //        eRet = OMX_ErrorInsufficientResources;
    //        goto EXIT;
    //    }

    // Create the pipe used to send command data to the thread
    err = pipe(m_cmddatapipe);
    if (err)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    //* create a decoder.

//    m_decoder = CreateAudioDecoder();
//    if(m_decoder == NULL)
//    {
//        logi(" can not create audio decoder.");
//        eRet = OMX_ErrorInsufficientResources;
//        goto EXIT;
//    }

    //*set omx cts flag to flush the last frame in h264
    //m_decoder->ioctrl(m_decoder, CEDARV_COMMAND_SET_OMXCTS_DECODER, 1);

    // Create the component thread
    err = pthread_create(&m_thread_id, NULL, ComponentThread, this);
    if( err || !m_thread_id )
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

EXIT:
    return eRet;
}


OMX_ERRORTYPE  omx_adec::get_component_version(OMX_IN OMX_HANDLETYPE pHComp,
                                               OMX_OUT OMX_STRING pComponentName,
                                               OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                                               OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                                               OMX_OUT OMX_UUIDTYPE* pComponentUUID)
{
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    CEDARC_UNUSE(pComponentUUID);
    if (!pHComp || !pComponentName || !pComponentVersion || !pSpecVersion)
    {
        return OMX_ErrorBadParameter;
    }

    strcpy((char*)pComponentName, (char*)m_cName);

    pComponentVersion->s.nVersionMajor = 1;
    pComponentVersion->s.nVersionMinor = 1;
    pComponentVersion->s.nRevision     = 0;
    pComponentVersion->s.nStep         = 0;

    pSpecVersion->s.nVersionMajor = 1;
    pSpecVersion->s.nVersionMinor = 1;
    pSpecVersion->s.nRevision     = 0;
    pSpecVersion->s.nStep         = 0;

    return OMX_ErrorNone;
}


OMX_ERRORTYPE  omx_adec::send_command(OMX_IN OMX_HANDLETYPE  pHComp,
                                      OMX_IN OMX_COMMANDTYPE eCmd,
                                      OMX_IN OMX_U32         uParam1,
                                      OMX_IN OMX_PTR         pCmdData)
{
    ThrCmdType    eCmdNative;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    CEDARC_UNUSE(pHComp);


    if(m_state == OMX_StateInvalid)
    {
        logd("ERROR: Send Command in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (eCmd == OMX_CommandMarkBuffer && pCmdData == NULL)
    {
        logd("ERROR: Send OMX_CommandMarkBuffer command but pCmdData invalid.");
        return OMX_ErrorBadParameter;
    }

    switch (eCmd)
    {
        case OMX_CommandStateSet:
            logi(" COMPONENT_SEND_COMMAND: OMX_CommandStateSet");
            eCmdNative = SetState;
            break;

        case OMX_CommandFlush:
            logi(" COMPONENT_SEND_COMMAND: OMX_CommandFlush");
            eCmdNative = Flush;
            if ((int)uParam1 > 1 && (int)uParam1 != -1)
            {
                logd("Error: Send OMX_CommandFlush command but uParam1 invalid.");
                return OMX_ErrorBadPortIndex;
            }
            break;

        case OMX_CommandPortDisable:
            logi(" COMPONENT_SEND_COMMAND: OMX_CommandPortDisable");
            eCmdNative = StopPort;
            break;

        case OMX_CommandPortEnable:
            logi(" COMPONENT_SEND_COMMAND: OMX_CommandPortEnable");
            eCmdNative = RestartPort;
            eCmdNative;

        case OMX_CommandMarkBuffer:
            logi(" COMPONENT_SEND_COMMAND: OMX_CommandMarkBuffer");
            eCmdNative = MarkBuf;
            if (uParam1 > 0)
            {
                logd("Error: Send OMX_CommandMarkBuffer command but uParam1 invalid.");
                return OMX_ErrorBadPortIndex;
            }
            break;

        default:
            break;
    }

    write(m_cmdpipe[1], &eCmdNative, sizeof(eCmdNative));

    // In case of MarkBuf, the pCmdData parameter is used to carry the data.
    // In other cases, the nParam1 parameter carries the data.
    if(eCmdNative == MarkBuf)
    {
        write(m_cmddatapipe[1], &pCmdData, sizeof(OMX_PTR));
    }
    else
    {
        write(m_cmddatapipe[1], &uParam1, sizeof(uParam1));
    }

    return eError;
}


OMX_ERRORTYPE  omx_adec::get_parameter(OMX_IN OMX_HANDLETYPE pHComp,
                                       OMX_IN OMX_INDEXTYPE  eParamIndex,
                                       OMX_INOUT OMX_PTR     pParamData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    CEDARC_UNUSE(pHComp);

    logd("(f:%s, l:%d) eParamIndex = 0x%x", __FUNCTION__, __LINE__, eParamIndex);
    if(m_state == OMX_StateInvalid)
    {
        logi("Get Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if(pParamData == NULL)
    {
        logi("Get Param in Invalid pParamData \n");
        return OMX_ErrorBadParameter;
    }

    switch(eParamIndex)
    {
        case OMX_IndexParamVideoInit:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoInit");
            memcpy(pParamData, &m_sPortParam, sizeof(OMX_PORT_PARAM_TYPE));
            break;
        }

        case OMX_IndexParamPortDefinition:
        {
            logd(" COMPONENT_GET_PARAMETER: OMX_IndexParamPortDefinition");
            if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex
                == m_sInPortDef.nPortIndex)
            {
                memcpy(pParamData, &m_sInPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                logd("****eDomain:%d,eEncoding:%d",
                      m_sInPortDef.eDomain,m_sInPortDef.format.audio.eEncoding);
                logd(" get_OMX_IndexParamPortDefinition: m_sInPortDef.nPortIndex[%d]",
                     (int)m_sInPortDef.nPortIndex);
            }
            else if (((OMX_PARAM_PORTDEFINITIONTYPE*)(pParamData))->nPortIndex
                     == m_sOutPortDef.nPortIndex)
            {
                memcpy(pParamData, &m_sOutPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                logd("****eDomain:%d,eEncoding:%d",
                     m_sOutPortDef.eDomain,m_sOutPortDef.format.audio.eEncoding);
                logd("(omx_adec, f:%s, l:%d) OMX_IndexParamPortDefinition,  nPortIndex[%d], \
                      nBufferCountActual[%d], nBufferCountMin[%d], nBufferSize[%d]",
                      __FUNCTION__, __LINE__,
                      (int)m_sOutPortDef.nPortIndex, (int)m_sOutPortDef.nBufferCountActual,
                      (int)m_sOutPortDef.nBufferCountMin, (int)m_sOutPortDef.nBufferSize);
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
                logd(" get_OMX_IndexParamPortDefinition: error. pParamData->nPortIndex=[%d]",
                     (int)((OMX_PARAM_PORTDEFINITIONTYPE*)(pParamData))->nPortIndex);
            }

            break;
        }

        case OMX_IndexParamAudioPortFormat:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioPortFormat");
            if (((OMX_AUDIO_PARAM_PORTFORMATTYPE *)(pParamData))->nPortIndex
                == m_sInPortFormat.nPortIndex)
            {
                if (((OMX_AUDIO_PARAM_PORTFORMATTYPE*)(pParamData))->nIndex
                    > m_sInPortFormat.nIndex)
                {
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(pParamData, &m_sInPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
                }
            }
            else if (((OMX_AUDIO_PARAM_PORTFORMATTYPE*)(pParamData))->nPortIndex
                     == m_sOutPortFormat.nPortIndex)
            {
                if (((OMX_AUDIO_PARAM_PORTFORMATTYPE*)(pParamData))->nIndex
                    > m_sOutPortFormat.nIndex)
                {
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(pParamData, &m_sOutPortFormat,
                           sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
                }
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }

            logi("OMX_IndexParamAudioPortFormat, eError[0x%x]", eError);
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamStandardComponentRole");
            OMX_PARAM_COMPONENTROLETYPE* comp_role;

            comp_role                    = (OMX_PARAM_COMPONENTROLETYPE *) pParamData;
            comp_role->nVersion.nVersion = OMX_SPEC_VERSION;
            comp_role->nSize             = sizeof(*comp_role);

            strncpy((char*)comp_role->cRole, (const char*)m_cRole, OMX_MAX_STRINGNAME_SIZE);
            break;
        }

        case OMX_IndexParamPriorityMgmt:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamPriorityMgmt");
            memcpy(pParamData, &m_sPriorityMgmt, sizeof(OMX_PRIORITYMGMTTYPE));
            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamCompBufferSupplier");
            OMX_PARAM_BUFFERSUPPLIERTYPE* pBuffSupplierParam
                = (OMX_PARAM_BUFFERSUPPLIERTYPE*)pParamData;

            if (pBuffSupplierParam->nPortIndex == 1)
            {
                pBuffSupplierParam->eBufferSupplier = m_sOutBufSupplier.eBufferSupplier;
            }
            else if (pBuffSupplierParam->nPortIndex == 0)
            {
                pBuffSupplierParam->eBufferSupplier = m_sInBufSupplier.eBufferSupplier;
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioInit:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioInit");
            OMX_PORT_PARAM_TYPE *audioPortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            audioPortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            audioPortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            audioPortParamType->nPorts            = 0;
            audioPortParamType->nStartPortNumber  = 0;

            break;
        }

        case OMX_IndexParamImageInit:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamImageInit");
            OMX_PORT_PARAM_TYPE *imagePortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            imagePortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            imagePortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            imagePortParamType->nPorts            = 0;
            imagePortParamType->nStartPortNumber  = 0;

            break;
        }

        case OMX_IndexParamOtherInit:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamOtherInit");
            OMX_PORT_PARAM_TYPE *otherPortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            otherPortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            otherPortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            otherPortParamType->nPorts            = 0;
            otherPortParamType->nStartPortNumber  = 0;

            break;
        }

        case OMX_IndexParamAudioWma:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioWma");
            logi("get_parameter: OMX_IndexParamAudioWma, do nothing.\n");
            break;
        }
        case OMX_IndexParamAudioAac:
        {
            logd(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioAac");
            logd("get_parameter: OMX_IndexParamAudioAac, do nothing.\n");
            break;
        }
        case OMX_IndexParamAudioMp3:
        {
            logd(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioMp3");
            logd("get_parameter: OMX_IndexParamAudioMp3, do nothing.\n");
            break;
        }
        case OMX_IndexParamAudioAmr:
        {
            logd(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioAmr");
            logd("get_parameter: OMX_IndexParamAudioAmr, do nothing.\n");
            break;
        }

        case OMX_IndexParamAudioVorbis:
        {
            logd(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioOgg");
            logd("get_parameter: OMX_IndexParamAudioOgg, do nothing.\n");
            break;
        }
        case OMX_IndexParamAudioApe:
        {
            logd(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioApe");
            logd("get_parameter: OMX_IndexParamAudioApe, do nothing.\n");
            break;
        }
        case OMX_IndexParamAudioAc3:
        {
            logd(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioAc3");
            logd("get_parameter: OMX_IndexParamAudioAc3, do nothing.\n");
            break;
        }
        case OMX_IndexParamAudioDts:
        {
            logd(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioDts");
            logd("get_parameter: OMX_IndexParamAudioDts, do nothing.\n");
            break;
        }
        case OMX_IndexParamAudioFlac://FLAC
        {
            logd(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioFlac");
            logd("get_parameter: OMX_IndexParamAudioFlac, do nothing.\n");
            break;
        }
        case OMX_IndexParamAudioAdpcm:
        {
            logd(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioADPCM");
            logd("get_parameter: OMX_IndexParamAudioADPCM, do nothing.\n");
            break;
        }
        case OMX_IndexParamAudioPcm:
        {
            logd(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioPcm");

            OMX_AUDIO_PARAM_PCMMODETYPE* params = (OMX_AUDIO_PARAM_PCMMODETYPE*)pParamData;
            params->eNumData = OMX_NumericalDataSigned;
            params->nBitPerSample = 16;
            params->ePCMMode = OMX_AUDIO_PCMModeLinear;
            if(m_channel)
            {
                params->nChannels = m_channel;
            }
            else
            {
                m_channel = 2;
                params->nChannels = 2;
            }
            if(m_sampleRate)
            {
                params->nSamplingRate = m_sampleRate;
            }
            else
            {
                m_sampleRate = 44100;
                params->nSamplingRate = 44100;
            }
            logd("eNumData[%d],nChannels[%d],m_streamInfo[%d],nSamplingRate[%d],m_streamInfo[%d]",
                 params->eNumData,(int)params->nChannels,m_channel,
                 (int)params->nSamplingRate,m_sampleRate);
            break;
        }
        default:
        {
            if((AW_VIDEO_EXTENSIONS_INDEXTYPE)eParamIndex
                == AWOMX_IndexParamVideoGetAndroidNativeBufferUsage)
            {
                logi(" COMPONENT_GET_PARAMETER: AWOMX_IndexParamVideoGetAndroidNativeBufferUsage");
                break;
            }
            else
            {
                logi("get_parameter: unknown param %08x\n", eParamIndex);
                eError =OMX_ErrorUnsupportedIndex;
                break;
            }
        }
    }

    return eError;
}

OMX_ERRORTYPE  omx_adec::set_parameter(OMX_IN OMX_HANDLETYPE pHComp,
                                       OMX_IN OMX_INDEXTYPE eParamIndex,
                                       OMX_IN OMX_PTR pParamData)
{
    OMX_ERRORTYPE         eError      = OMX_ErrorNone;

    logd("(f:%s, l:%d) eParamIndex = 0x%x", __FUNCTION__, __LINE__, eParamIndex);
    CEDARC_UNUSE(pHComp);

    if(m_state == OMX_StateInvalid)
    {
        logd("Set Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if(pParamData == NULL)
    {
        logd("Get Param in Invalid pParamData \n");
        return OMX_ErrorBadParameter;
    }

    switch(eParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamPortDefinition");
            if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex
                 == m_sInPortDef.nPortIndex)
            {
                logd("set_OMX_IndexParamPortDefinition, m_sInPortDef.nPortIndex=%d",
                     (int)m_sInPortDef.nPortIndex);
                if(((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual
                    != m_sInPortDef.nBufferCountActual)
                {
                    int nBufCnt;
                    int nIndex;


                    pthread_mutex_lock(&m_inBufMutex);

                    if(m_sInBufList.pBufArr != NULL)
                    {
                        free(m_sInBufList.pBufArr);
                    }

                    if(m_sInBufList.pBufHdrList != NULL)
                    {
                        free(m_sInBufList.pBufHdrList);
                    }

                    nBufCnt = ((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual;
                    logi("x allocate %d buffers.", nBufCnt);

                    m_sInBufList.pBufArr
                        = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE)* nBufCnt);
                    m_sInBufList.pBufHdrList
                        = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*)* nBufCnt);
                    for (nIndex = 0; nIndex < nBufCnt; nIndex++)
                    {
                        OMX_CONF_INIT_STRUCT_PTR (&m_sInBufList.pBufArr[nIndex],
                                                  OMX_BUFFERHEADERTYPE);
                    }

                    m_sInBufList.nSizeOfList       = 0;
                    m_sInBufList.nAllocSize        = 0;
                    m_sInBufList.nWritePos         = 0;
                    m_sInBufList.nReadPos          = 0;
                    m_sInBufList.nAllocBySelfFlags = 0;
                    m_sInBufList.nBufArrSize       = nBufCnt;
                    m_sInBufList.eDir              = OMX_DirInput;

                    pthread_mutex_unlock(&m_inBufMutex);
                }

                memcpy(&m_sInPortDef, pParamData, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex
                    == m_sOutPortDef.nPortIndex)
            {
                logd("set_OMX_IndexParamPortDefinition, m_sOutPortDef.nPortIndex=%d",
                     (int)m_sOutPortDef.nPortIndex);
                if(((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual
                    != m_sOutPortDef.nBufferCountActual)
                {
                    int nBufCnt;
                    int nIndex;

                    pthread_mutex_lock(&m_outBufMutex);

                    if(m_sOutBufList.pBufArr != NULL)
                    {
                        free(m_sOutBufList.pBufArr);
                    }

                    if(m_sOutBufList.pBufHdrList != NULL)
                    {
                        free(m_sOutBufList.pBufHdrList);
                    }

                    nBufCnt = ((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual;
                    logi("x allocate %d buffers.", nBufCnt);

                    //*Initialize the output buffer list
                    m_sOutBufList.pBufArr
                        = (OMX_BUFFERHEADERTYPE*) malloc(sizeof(OMX_BUFFERHEADERTYPE) * nBufCnt);
                    m_sOutBufList.pBufHdrList
                        = (OMX_BUFFERHEADERTYPE**) malloc(sizeof(OMX_BUFFERHEADERTYPE*) * nBufCnt);
                    for (nIndex = 0; nIndex < nBufCnt; nIndex++)
                    {
                        OMX_CONF_INIT_STRUCT_PTR (&m_sOutBufList.pBufArr[nIndex],
                                                  OMX_BUFFERHEADERTYPE);
                    }

                    m_sOutBufList.nSizeOfList       = 0;
                    m_sOutBufList.nAllocSize        = 0;
                    m_sOutBufList.nWritePos         = 0;
                    m_sOutBufList.nReadPos          = 0;
                    m_sOutBufList.nAllocBySelfFlags = 0;
                    m_sOutBufList.nBufArrSize       = nBufCnt;
                    m_sOutBufList.eDir              = OMX_DirOutput;

                    pthread_mutex_unlock(&m_outBufMutex);
                }

                memcpy(&m_sOutPortDef, pParamData, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else
            {
                logw("set_OMX_IndexParamPortDefinition, error, paramPortIndex=%d",
                     (int)((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex);
                eError = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioPortFormat:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioPortFormat");

            if (((OMX_AUDIO_PARAM_PORTFORMATTYPE *)(pParamData))->nPortIndex
                == m_sInPortFormat.nPortIndex)
            {
                if (((OMX_AUDIO_PARAM_PORTFORMATTYPE *)(pParamData))->nIndex
                    > m_sInPortFormat.nIndex)
                {
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(&m_sInPortFormat, pParamData, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
                }
            }
            else if (((OMX_AUDIO_PARAM_PORTFORMATTYPE*)(pParamData))->nPortIndex
                     == m_sOutPortFormat.nPortIndex)
            {
                if (((OMX_AUDIO_PARAM_PORTFORMATTYPE*)(pParamData))->nIndex
                    > m_sOutPortFormat.nIndex)
                {
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(&m_sOutPortFormat, pParamData, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
                }
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamStandardComponentRole");
            OMX_PARAM_COMPONENTROLETYPE *comp_role;
            comp_role = (OMX_PARAM_COMPONENTROLETYPE *) pParamData;
            logi("set_parameter: OMX_IndexParamStandardComponentRole %s\n", comp_role->cRole);

            if(m_state == OMX_StateLoaded)
            {
                logi("Set Parameter called in valid state");
            }
            else
            {
                logi("Set Parameter called in Invalid State\n");
                return OMX_ErrorIncorrectStateOperation;
            }

            if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.wma",
                OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.wma",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.wma",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.aac",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.aac",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.aac",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.mp3",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.mp3",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.mp3",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.amrnb",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.amrnb",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.amrnb",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.amrwb",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.amrwb",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.amrwb",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.g711.alaw",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.g711alaw",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.g711alaw",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.g711.mlaw",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.g711mlaw",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.g711mlaw",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.adpcm",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.adpcm",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.adpcm",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.vorbis",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.ogg",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.ogg",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.ape",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.ape",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.ape",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.dts",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.dts",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.dts",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.raw",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.raw",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.raw",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.flac",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"audio_decoder.flac",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"audio_decoder.flac",OMX_MAX_STRINGNAME_SIZE);
                }

                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else
            {
                logi("Setparameter: unknown param %s\n", m_cName);
                eError = OMX_ErrorInvalidComponentName;
            }

            break;
        }

        case OMX_IndexParamPriorityMgmt:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamPriorityMgmt");
            if(m_state != OMX_StateLoaded)
            {
                logi("Set Parameter called in Invalid State\n");
                return OMX_ErrorIncorrectStateOperation;
            }

            OMX_PRIORITYMGMTTYPE *priorityMgmtype = (OMX_PRIORITYMGMTTYPE*) pParamData;

            m_sPriorityMgmt.nGroupID = priorityMgmtype->nGroupID;
            m_sPriorityMgmt.nGroupPriority = priorityMgmtype->nGroupPriority;

            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamCompBufferSupplier");
            OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplierType
                = (OMX_PARAM_BUFFERSUPPLIERTYPE*) pParamData;

            logi("set_parameter: OMX_IndexParamCompBufferSupplier %d\n",
                bufferSupplierType->eBufferSupplier);
            if(bufferSupplierType->nPortIndex == 0)
            {
                m_sInBufSupplier.eBufferSupplier = bufferSupplierType->eBufferSupplier;
            }
            else if(bufferSupplierType->nPortIndex == 1)
            {
                m_sOutBufSupplier.eBufferSupplier = bufferSupplierType->eBufferSupplier;
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioWma:
        {
            OMX_AUDIO_PARAM_WMATYPE* params = (OMX_AUDIO_PARAM_WMATYPE*)pParamData;
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioWma");
            logd("nChannels[%d],nSamplingRate[%d],bitrate:%d",
                 (int)params->nChannels,(int)params->nSamplingRate,(int)params->nBitRate);
            m_channel = params->nChannels>2? 2:params->nChannels;
            m_sampleRate = params->nSamplingRate;
            m_channels = params->nChannels ;
            m_codec_tag = params->nEncodeOptions;
            m_bitRate = params->nBitRate;
            m_blockalign = params->nBlockAlign;
            m_bitspersample = params->nSuperBlockAlign;
            break;
        }
        case OMX_IndexParamAudioAac:
        {
            OMX_AUDIO_PARAM_AACPROFILETYPE* params = (OMX_AUDIO_PARAM_AACPROFILETYPE*)pParamData;
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioAac");
            logd("nChannels[%d],nSamplingRate[%d]",
                 (int)params->nChannels,(int)params->nSampleRate);
            m_channels = params->nChannels ;
            if(params->nChannels)
            {
                m_channel = params->nChannels>2? 2:params->nChannels;
            }
            if(params->nSampleRate)
            {
                m_sampleRate = params->nSampleRate;
            }
            break;
        }
        case OMX_IndexParamAudioMp3:
        {
            OMX_AUDIO_PARAM_MP3TYPE* params = (OMX_AUDIO_PARAM_MP3TYPE*)pParamData;
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioMp3");
            logd("nChannels[%d],nSamplingRate[%d]",
                 (int)params->nChannels,(int)params->nSampleRate);
            m_channels = params->nChannels ;
            if(params->nChannels)
            {
                m_channel = params->nChannels>2? 2:params->nChannels;
            }
            if(params->nSampleRate)
            {
                m_sampleRate = params->nSampleRate;
            }
            break;
        }
        case OMX_IndexParamAudioAmr:
        {
            OMX_AUDIO_PARAM_AMRTYPE* params = (OMX_AUDIO_PARAM_AMRTYPE*)pParamData;
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioAmr");
            logd("nChannels[%d],eAMRBandMode[%d]",
                 (int)params->nChannels,(int)params->eAMRBandMode);
            m_channel = 1;
            m_channels = 1 ;
            //if(params->nSampleRate)
            if((params->eAMRBandMode>OMX_AUDIO_AMRBandModeUnused)
                &&(params->eAMRBandMode<OMX_AUDIO_AMRBandModeWB0))
            {
                m_sampleRate = 8000;//params->nSampleRate;
            }
            else
            {
                 m_sampleRate = 16000;
            }
            break;
        }
        case OMX_IndexParamAudioVorbis:
        {
            OMX_AUDIO_PARAM_VORBISTYPE* params = (OMX_AUDIO_PARAM_VORBISTYPE*)pParamData;
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioOgg");
            logd("nChannels[%d],nSamplingRate[%d]",
                 (int)params->nChannels,(int)params->nSampleRate);
            m_channels = params->nChannels ;
            if(params->nChannels)
            {
                m_channel = params->nChannels>2? 2:params->nChannels;
            }
            if(params->nSampleRate)
            {
                m_sampleRate = params->nSampleRate;
            }
            break;
        }
        case OMX_IndexParamAudioApe:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE* params = (OMX_AUDIO_PARAM_PCMMODETYPE*)pParamData;
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioApe");
            logd("nChannels[%d],nSamplingRate[%d],nBitPerSample:%d",
                 (int)params->nChannels,(int)params->nSamplingRate,(int)params->nBitPerSample);
            m_channels = params->nChannels ;
            m_channel = params->nChannels>2? 2:params->nChannels;
            m_sampleRate = params->nSamplingRate;
            m_channels = params->nChannels ;
            m_bitspersample = params->nBitPerSample;
            break;
        }
        case OMX_IndexParamAudioAc3:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE* params = (OMX_AUDIO_PARAM_PCMMODETYPE*)pParamData;
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioAc3");
            logd("nChannels[%d],nSamplingRate[%d],nBitPerSample:%d",
                 (int)params->nChannels,(int)params->nSamplingRate,(int)params->nBitPerSample);
            m_channels = params->nChannels ;
            m_channel = params->nChannels>2? 2:params->nChannels;
            m_sampleRate = params->nSamplingRate;
            m_channels = params->nChannels ;
            m_bitspersample = params->nBitPerSample;
            break;
        }
        case OMX_IndexParamAudioDts:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE* params = (OMX_AUDIO_PARAM_PCMMODETYPE*)pParamData;
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioDts");
            logd("nChannels[%d],nSamplingRate[%d],nBitPerSample:%d",
                 (int)params->nChannels,(int)params->nSamplingRate,(int)params->nBitPerSample);
            m_channels = params->nChannels ;
            m_channel = params->nChannels>2? 2:params->nChannels;
            m_sampleRate = params->nSamplingRate;
            m_channels = params->nChannels ;
            m_bitspersample = params->nBitPerSample;
            break;
        }
        case OMX_IndexParamAudioFlac:
        {
            OMX_AUDIO_PARAM_FLACTYPE* params = (OMX_AUDIO_PARAM_FLACTYPE*)pParamData;
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioFlac");
            logd("nChannels[%d],nSamplingRate[%d]",
                 (int)params->nChannels,(int)params->nSampleRate);
            m_channels = params->nChannels ;
            if(params->nChannels)
            {
                m_channel = params->nChannels>2? 2:params->nChannels;
            }
            if(params->nSampleRate)
            {
                m_sampleRate = params->nSampleRate;
            }
            break;
        }
        case OMX_IndexParamAudioAdpcm:
        {
            OMX_AUDIO_PARAM_ADPCMTYPE* params = (OMX_AUDIO_PARAM_ADPCMTYPE*)pParamData;
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioADPCM");
            logd("nChannels[%d],nSamplingRate[%d],nBitsPerSample[%d]",
                 (int)params->nChannels,(int)params->nSampleRate,(int)params->nBitsPerSample);
            m_channels = params->nChannels ;
            if(params->nChannels)
            {
                m_channel = params->nChannels>2? 2:params->nChannels;
            }
            if(params->nSampleRate)
            {
                m_sampleRate = params->nSampleRate;
            }
            m_bitspersample = params->nBitsPerSample;
            break;
        }
        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE* params = (OMX_AUDIO_PARAM_PCMMODETYPE*)pParamData;
            logd(" COMPONENT_SET_PARAMETER: OMX_IndexParamAudioPcm");
            logd("set_parameter: OMX_IndexParamAudioPcm, do nothing.\n");
            logd("nChannels[%d],nSamplingRate[%d]",
                 (int)params->nChannels,(int)params->nSamplingRate);
            m_channels = params->nChannels ;
            if(params->nChannels)
            {
                m_channel = params->nChannels>2? 2:params->nChannels;
            }
            if(params->nSamplingRate)
            {
                m_sampleRate = params->nSamplingRate;
            }
            if(!m_bitspersample)
            {
                m_bitspersample = params->nBitPerSample;
            }

            break;
        }
        default:
        {
            if((AW_VIDEO_EXTENSIONS_INDEXTYPE)eParamIndex
                == AWOMX_IndexParamVideoUseAndroidNativeBuffer2)
            {
                logi(" COMPONENT_SET_PARAMETER: AWOMX_IndexParamVideoUseAndroidNativeBuffer2");
                logi("set_parameter: AWOMX_IndexParamVideoUseAndroidNativeBuffer2, do nothing.\n");
                m_useAndroidBuffer = OMX_TRUE;
                break;
            }
            else if((AW_VIDEO_EXTENSIONS_INDEXTYPE)eParamIndex
                    == AWOMX_IndexParamVideoEnableAndroidNativeBuffers)
            {
                logi(" COMPONENT_SET_PARAMETER: AWOMX_IndexParamVideoEnableAndroidNativeBuffers");
                logi("set_parameter: AWOMX_IndexParamVideoEnableAndroidNativeBuffers, \
                      set m_useAndroidBuffer to OMX_TRUE\n");
                m_useAndroidBuffer = OMX_TRUE;
                break;
            }
            else
            {
                logi("Setparameter: unknown param %d\n", eParamIndex);
                eError = OMX_ErrorUnsupportedIndex;
                break;
            }
        }
    }
    logd("err:%d",eError);
    return eError;
}

OMX_ERRORTYPE  omx_adec::get_config(OMX_IN OMX_HANDLETYPE pHComp,
                                    OMX_IN OMX_INDEXTYPE eConfigIndex,
                                    OMX_INOUT OMX_PTR pConfigData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    logi("(f:%s, l:%d) uIndex = %d", __FUNCTION__, __LINE__, eConfigIndex);
    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(pConfigData);
    if (m_state == OMX_StateInvalid)
    {
        logi("get_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    switch (eConfigIndex)
    {
        default:
        {
            logi("get_config: unknown param %d\n",eConfigIndex);
            eError = OMX_ErrorUnsupportedIndex;
        }
    }

    return eError;
}

OMX_ERRORTYPE omx_adec::set_config(OMX_IN OMX_HANDLETYPE pHComp,
                                   OMX_IN OMX_INDEXTYPE eConfigIndex,
                                   OMX_IN OMX_PTR pConfigData)
{

    logi("(f:%s, l:%d) uIndex = %d", __FUNCTION__, __LINE__, eConfigIndex);

    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(pConfigData);

    if(m_state == OMX_StateInvalid)
    {
        logi("set_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    OMX_ERRORTYPE eError = OMX_ErrorNone;

    if (m_state == OMX_StateExecuting)
    {
        logi("set_config: Ignore in Executing state\n");
        return eError;
    }

    switch(eConfigIndex)
    {
        default:
        {
            eError = OMX_ErrorUnsupportedIndex;
        }
    }

    return eError;
}


OMX_ERRORTYPE  omx_adec::get_extension_index(OMX_IN OMX_HANDLETYPE pHComp,
                                             OMX_IN OMX_STRING pParamName,
                                             OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    unsigned int  nIndex;
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;

    logi("(f:%s, l:%d) param name = %s", __FUNCTION__, __LINE__, pParamName);
    if(m_state == OMX_StateInvalid)
    {
        logi("Get Extension uIndex in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if(pHComp == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    for(nIndex = 0; nIndex < sizeof(sAudioDecCustomParams)/sizeof(AUDIO_CUSTOM_PARAM); nIndex++)
    {
        if(strcmp((char *)pParamName, (char *)&(sAudioDecCustomParams[nIndex].cCustomParamName))
           == 0)
        {
            *pIndexType = sAudioDecCustomParams[nIndex].nCustomParamIndex;
            eError = OMX_ErrorNone;
            break;
        }
    }

    return eError;
}

OMX_ERRORTYPE omx_adec::get_state(OMX_IN OMX_HANDLETYPE pHComp, OMX_OUT OMX_STATETYPE* pState)
{
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);

    if(pHComp == NULL || pState == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    *pState = m_state;
    logi("COMPONENT_GET_STATE, state[0x%x]", m_state);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_adec::component_tunnel_request(OMX_IN    OMX_HANDLETYPE       pHComp,
                                                 OMX_IN    OMX_U32              uPort,
                                                 OMX_IN    OMX_HANDLETYPE       pPeerComponent,
                                                 OMX_IN    OMX_U32              uPeerPort,
                                                 OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    logi(" COMPONENT_TUNNEL_REQUEST");

    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pPeerComponent);
    CEDARC_UNUSE(uPeerPort);
    CEDARC_UNUSE(pTunnelSetup);

    logw("Error: component_tunnel_request Not Implemented\n");
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE  omx_adec::use_buffer(OMX_IN    OMX_HANDLETYPE          hComponent,
                                      OMX_INOUT OMX_BUFFERHEADERTYPE**  ppBufferHdr,
                                      OMX_IN    OMX_U32                 nPortIndex,
                                      OMX_IN    OMX_PTR                 pAppPrivate,
                                      OMX_IN    OMX_U32                 nSizeBytes,
                                      OMX_IN    OMX_U8*                 pBuffer)
{
    OMX_PARAM_PORTDEFINITIONTYPE*   pPortDef;
    OMX_U32                         nIndex = 0x0;

    if(hComponent == NULL || ppBufferHdr == NULL || pBuffer == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if (nPortIndex == m_sInPortDef.nPortIndex)
    {
        pPortDef = &m_sInPortDef;
    }
    else if (nPortIndex == m_sOutPortDef.nPortIndex)
    {
        pPortDef = &m_sOutPortDef;
    }
    else
    {
        return OMX_ErrorBadParameter;
    }

    if (m_state!=OMX_StateLoaded
        && m_state!=OMX_StateWaitForResources
        && pPortDef->bEnabled!=OMX_FALSE)
    {
        logd("pPortDef[%d]->bEnabled=%d, m_state=0x%x, Can't use_buffer!",
             (int)nPortIndex, pPortDef->bEnabled, m_state);
        return OMX_ErrorIncorrectStateOperation;
    }
    logi("pPortDef[%d]->bEnabled=%d, m_state=0x%x, can use_buffer.",
         (int)nPortIndex, pPortDef->bEnabled, m_state);

    if (nSizeBytes != pPortDef->nBufferSize || pPortDef->bPopulated)
    {
        return OMX_ErrorBadParameter;
    }

    // Find an empty position in the BufferList and allocate memory for the buffer header.
    // Use the buffer passed by the client to initialize the actual buffer
    // inside the buffer header.
    if (nPortIndex == m_sInPortDef.nPortIndex)
    {
        logi("use_buffer, m_sInPortDef.nPortIndex=[%d]", (int)m_sInPortDef.nPortIndex);
        pthread_mutex_lock(&m_inBufMutex);

        if((OMX_S32)m_sInBufList.nAllocSize >= m_sInBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_inBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = m_sInBufList.nAllocSize;
        m_sInBufList.nAllocSize++;

        m_sInBufList.pBufArr[nIndex].pBuffer          = pBuffer;
        m_sInBufList.pBufArr[nIndex].nAllocLen        = nSizeBytes;
        m_sInBufList.pBufArr[nIndex].pAppPrivate      = pAppPrivate;
        m_sInBufList.pBufArr[nIndex].nInputPortIndex  = nPortIndex;
        m_sInBufList.pBufArr[nIndex].nOutputPortIndex = 0xFFFFFFFE;
        *ppBufferHdr = &m_sInBufList.pBufArr[nIndex];
        if (m_sInBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&m_inBufMutex);
    }
    else
    {
        logi("use_buffer, m_sOutPortDef.nPortIndex=[%d]", (int)m_sOutPortDef.nPortIndex);
        pthread_mutex_lock(&m_outBufMutex);

        if((OMX_S32)m_sOutBufList.nAllocSize >= m_sOutBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_outBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = m_sOutBufList.nAllocSize;
        m_sOutBufList.nAllocSize++;
        m_sOutBufList.pBufArr[nIndex].pBuffer          = pBuffer;
        m_sOutBufList.pBufArr[nIndex].nAllocLen        = nSizeBytes;
        m_sOutBufList.pBufArr[nIndex].pAppPrivate      = pAppPrivate;
        m_sOutBufList.pBufArr[nIndex].nInputPortIndex  = 0xFFFFFFFE;
        m_sOutBufList.pBufArr[nIndex].nOutputPortIndex = nPortIndex;
        *ppBufferHdr = &m_sOutBufList.pBufArr[nIndex];
        if (m_sOutBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&m_outBufMutex);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_adec::allocate_buffer(OMX_IN    OMX_HANDLETYPE         hComponent,
                                        OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                        OMX_IN    OMX_U32                nPortIndex,
                                        OMX_IN    OMX_PTR                pAppPrivate,
                                        OMX_IN    OMX_U32                nSizeBytes)
{
    OMX_S8                        nIndex = 0x0;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;

    //logi(" COMPONENT_ALLOCATE_BUFFER");
    logd("(f:%s, l:%d) nPortIndex[%d], nSizeBytes[%d]",
          __FUNCTION__, __LINE__, (int)nPortIndex, (int)nSizeBytes);
    if(hComponent == NULL || ppBufferHdr == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if (nPortIndex == m_sInPortDef.nPortIndex)
    {
        pPortDef = &m_sInPortDef;
    }
    else
    {
        if (nPortIndex == m_sOutPortDef.nPortIndex)
        {
            pPortDef = &m_sOutPortDef;
        }
        else
        {
            return OMX_ErrorBadParameter;
        }
    }

    //    if (!pPortDef->bEnabled)
    //        return OMX_ErrorIncorrectStateOperation;

    if (m_state!=OMX_StateLoaded
        && m_state!=OMX_StateWaitForResources
        && pPortDef->bEnabled!=OMX_FALSE)
    {
        logw("pPortDef[%d]->bEnabled=%d, m_state=0x%x, Can't allocate_buffer!",
             (int)nPortIndex, pPortDef->bEnabled, m_state);
        return OMX_ErrorIncorrectStateOperation;
    }
    logi("pPortDef[%d]->bEnabled=%d, m_state=0x%x, can allocate_buffer.",
         (int)nPortIndex, pPortDef->bEnabled, m_state);

    if (nSizeBytes != pPortDef->nBufferSize || pPortDef->bPopulated)
    {
        return OMX_ErrorBadParameter;
    }

    // Find an empty position in the BufferList and allocate memory for the buffer header
    // and the actual buffer
    if (nPortIndex == m_sInPortDef.nPortIndex)
    {
        logi("allocate_buffer, m_sInPortDef.nPortIndex[%d]", (int)m_sInPortDef.nPortIndex);
        pthread_mutex_lock(&m_inBufMutex);

        if((OMX_S32)m_sInBufList.nAllocSize >= m_sInBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_inBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = m_sInBufList.nAllocSize;
        logd("malloc m_sInBufList.pBufArr[nIndex[%d]].pBuffer:",(int)m_sInBufList.nAllocSize);
        m_sInBufList.pBufArr[nIndex].pBuffer = (OMX_U8*)malloc(nSizeBytes);

        if (!m_sInBufList.pBufArr[nIndex].pBuffer)
        {
            pthread_mutex_unlock(&m_inBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        m_sInBufList.nAllocBySelfFlags |= (1<<nIndex);

        m_sInBufList.pBufArr[nIndex].nAllocLen        = nSizeBytes;
        m_sInBufList.pBufArr[nIndex].pAppPrivate      = pAppPrivate;
        m_sInBufList.pBufArr[nIndex].nInputPortIndex  = nPortIndex;
        m_sInBufList.pBufArr[nIndex].nOutputPortIndex = 0xFFFFFFFE;
        *ppBufferHdr = &m_sInBufList.pBufArr[nIndex];

        m_sInBufList.nAllocSize++;

        if (m_sInBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&m_inBufMutex);
    }
    else
    {
        logi("allocate_buffer, m_sOutPortDef.nPortIndex[%d]", (int)m_sOutPortDef.nPortIndex);
        pthread_mutex_lock(&m_outBufMutex);

        if((OMX_S32)m_sOutBufList.nAllocSize >= m_sOutBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_outBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = m_sOutBufList.nAllocSize;
        logd("malloc m_sOutBufList.pBufArr[nIndex[%d]].pBuffer:",(int)m_sInBufList.nAllocSize);
        m_sOutBufList.pBufArr[nIndex].pBuffer = (OMX_U8*)malloc(nSizeBytes);

        if (!m_sOutBufList.pBufArr[nIndex].pBuffer)
        {
            pthread_mutex_unlock(&m_outBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        m_sOutBufList.nAllocBySelfFlags |= (1<<nIndex);

        m_sOutBufList.pBufArr[nIndex].nAllocLen        = nSizeBytes;
        m_sOutBufList.pBufArr[nIndex].pAppPrivate      = pAppPrivate;
        m_sOutBufList.pBufArr[nIndex].nInputPortIndex  = 0xFFFFFFFE;
        m_sOutBufList.pBufArr[nIndex].nOutputPortIndex = nPortIndex;
        *ppBufferHdr = &m_sOutBufList.pBufArr[nIndex];

        m_sOutBufList.nAllocSize++;

        if (m_sOutBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&m_outBufMutex);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_adec::free_buffer(OMX_IN  OMX_HANDLETYPE        hComponent,
                                    OMX_IN  OMX_U32               nPortIndex,
                                    OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;
    OMX_S32                       nIndex;

    logd("(f:%s, l:%d) nPortIndex = %d, pBufferHdr = %p, m_state=0x%x",
         __FUNCTION__, __LINE__, (int)nPortIndex, pBufferHdr, m_state);
    if(hComponent == NULL || pBufferHdr == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    // Match the pBufferHdr to the appropriate entry in the BufferList
    // and free the allocated memory
    if (nPortIndex == m_sInPortDef.nPortIndex)
    {
        pPortDef = &m_sInPortDef;

        pthread_mutex_lock(&m_inBufMutex);

        for(nIndex = 0; nIndex < m_sInBufList.nBufArrSize; nIndex++)
        {
            if(pBufferHdr == &m_sInBufList.pBufArr[nIndex])
            break;
        }

        if(nIndex == m_sInBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_inBufMutex);
            return OMX_ErrorBadParameter;
        }

        if(m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
        {
            free(m_sInBufList.pBufArr[nIndex].pBuffer);
            m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
            m_sInBufList.nAllocBySelfFlags &= ~(1<<nIndex);
        }

        m_sInBufList.nAllocSize--;
        if(m_sInBufList.nAllocSize == 0)
        {
            pPortDef->bPopulated = OMX_FALSE;
        }

        pthread_mutex_unlock(&m_inBufMutex);
    }
    else if (nPortIndex == m_sOutPortDef.nPortIndex)
    {
        pPortDef = &m_sOutPortDef;

        pthread_mutex_lock(&m_outBufMutex);

        for(nIndex = 0; nIndex < m_sOutBufList.nBufArrSize; nIndex++)
        {
            logi("pBufferHdr = %p, &m_sOutBufList.pBufArr[%d] = %p",
                  pBufferHdr, (int)nIndex, &m_sOutBufList.pBufArr[nIndex]);
            if(pBufferHdr == &m_sOutBufList.pBufArr[nIndex])
            {
                break;
            }
        }

        logi("uIndex = %d", (int)nIndex);

        if(nIndex == m_sOutBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_outBufMutex);
            return OMX_ErrorBadParameter;
        }

        if(m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
        {
            free(m_sOutBufList.pBufArr[nIndex].pBuffer);
            m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
            m_sOutBufList.nAllocBySelfFlags &= ~(1<<nIndex);
        }

        m_sOutBufList.nAllocSize--;
        if(m_sOutBufList.nAllocSize == 0)
        {
            pPortDef->bPopulated = OMX_FALSE;
        }

        pthread_mutex_unlock(&m_outBufMutex);
    }
    else
    {
        return OMX_ErrorBadParameter;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE  omx_adec::empty_this_buffer(OMX_IN OMX_HANDLETYPE hComponent,
                                           OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    logd("***emptyThisBuffer: pts = %lld , audioFormat = %d",
    pBufferHdr->nTimeStamp,
    m_eCompressionFormat);

    ThrCmdType eCmd   = EmptyBuf;
    if(hComponent == NULL || pBufferHdr == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if (!m_sInPortDef.bEnabled)
    {
        return OMX_ErrorIncorrectStateOperation;
    }

    if (pBufferHdr->nInputPortIndex != 0x0  || pBufferHdr->nOutputPortIndex != OMX_NOPORT)
    {
        return OMX_ErrorBadPortIndex;
    }

    if (m_state != OMX_StateExecuting && m_state != OMX_StatePause)
    {
        return OMX_ErrorIncorrectStateOperation;
    }

    //fwrite(pBufferHdr->pBuffer, 1, pBufferHdr->nFilledLen, ph264File);
    //DBG_WARNING("BHF[0x%x],len[%d]", pBufferHdr->nFlags, pBufferHdr->nFilledLen);
    // Put the command and data in the pipe
    write(m_cmdpipe[1], &eCmd, sizeof(eCmd));
    write(m_cmddatapipe[1], &pBufferHdr, sizeof(OMX_BUFFERHEADERTYPE*));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE  omx_adec::fill_this_buffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    ThrCmdType eCmd = FillBuf;

//    logd("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    if(hComponent == NULL || pBufferHdr == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if (!m_sOutPortDef.bEnabled)
    {
        return OMX_ErrorIncorrectStateOperation;
    }

    if (pBufferHdr->nOutputPortIndex != 0x1 || pBufferHdr->nInputPortIndex != OMX_NOPORT)
    {
        return OMX_ErrorBadPortIndex;
    }

    if (m_state != OMX_StateExecuting && m_state != OMX_StatePause)
    {
        return OMX_ErrorIncorrectStateOperation;
    }

    // Put the command and data in the pipe
    write(m_cmdpipe[1], &eCmd, sizeof(eCmd));
    write(m_cmddatapipe[1], &pBufferHdr, sizeof(OMX_BUFFERHEADERTYPE*));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE  omx_adec::set_callbacks(OMX_IN OMX_HANDLETYPE        pHComp,
                                           OMX_IN OMX_CALLBACKTYPE* pCallbacks,
                                           OMX_IN OMX_PTR           appData)
{
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);

    if(pHComp == NULL || pCallbacks == NULL || appData == NULL)
    {
        return OMX_ErrorBadParameter;
    }
    memcpy(&m_Callbacks, pCallbacks, sizeof(OMX_CALLBACKTYPE));
    m_pAppData = appData;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE  omx_adec::component_deinit(OMX_IN OMX_HANDLETYPE pHComp)
{
    //logi(" COMPONENT_DEINIT");
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    ThrCmdType    eCmd   = Stop;
    OMX_S32       nIndex = 0;


    CEDARC_UNUSE(pHComp);

    // In case the client crashes, check for nAllocSize parameter.
    // If this is greater than zero, there are elements in the list that are not free'd.
    // In that case, free the elements.
    if (m_sInBufList.nAllocSize > 0)
    {
        for(nIndex=0; nIndex<m_sInBufList.nBufArrSize; nIndex++)
        {
            if(m_sInBufList.pBufArr[nIndex].pBuffer != NULL)
            {
                if(m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
                {
                    free(m_sInBufList.pBufArr[nIndex].pBuffer);
                    m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
                }
            }
        }

        if (m_sInBufList.pBufArr != NULL)
        {
            free(m_sInBufList.pBufArr);
        }

        if (m_sInBufList.pBufHdrList != NULL)
        {
            free(m_sInBufList.pBufHdrList);
        }

        memset(&m_sInBufList, 0, sizeof(struct _BufferList));
        m_sInBufList.nBufArrSize = m_sInPortDef.nBufferCountActual;
    }

    if (m_sOutBufList.nAllocSize > 0)
    {
        for(nIndex=0; nIndex<m_sOutBufList.nBufArrSize; nIndex++)
        {
            if(m_sOutBufList.pBufArr[nIndex].pBuffer != NULL)
            {
                if(m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
                {
                    free(m_sOutBufList.pBufArr[nIndex].pBuffer);
                    m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
                }
            }
        }

        if (m_sOutBufList.pBufArr != NULL)
        {
            free(m_sOutBufList.pBufArr);
        }

        if (m_sOutBufList.pBufHdrList != NULL)
        {
            free(m_sOutBufList.pBufHdrList);
        }

        memset(&m_sOutBufList, 0, sizeof(struct _BufferList));
        m_sOutBufList.nBufArrSize = m_sOutPortDef.nBufferCountActual;
    }

    // Put the command and data in the pipe
    write(m_cmdpipe[1], &eCmd, sizeof(eCmd));
    write(m_cmddatapipe[1], &eCmd, sizeof(eCmd));

    // Wait for thread to exit so we can get the status into "error"
    pthread_join(m_thread_id, (void**)&eError);

    // close the pipe handles
    close(m_cmdpipe[0]);
    close(m_cmdpipe[1]);
    close(m_cmddatapipe[0]);
    close(m_cmddatapipe[1]);

    if(m_decoder != NULL)
    {
        DestroyAudioDecoder(m_decoder);
        m_decoder = NULL;
        mFirstInputDataFlag = OMX_TRUE;logd("****mFirstInputDataFlag = OMX_TRUE");
    }

    return eError;
}


OMX_ERRORTYPE  omx_adec::use_EGL_image(OMX_IN OMX_HANDLETYPE               pHComp,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                          OMX_IN OMX_U32                   uPort,
                                          OMX_IN OMX_PTR                   pAppData,
                                          OMX_IN void*                     pEglImage)
{
    logi("Error : use_EGL_image:  Not Implemented \n");

    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(ppBufferHdr);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pAppData);
    CEDARC_UNUSE(pEglImage);

    return OMX_ErrorNotImplemented;
}


OMX_ERRORTYPE  omx_adec::component_role_enum(OMX_IN  OMX_HANDLETYPE pHComp,
                                             OMX_OUT OMX_U8*        pRole,
                                             OMX_IN  OMX_U32        uIndex)
{
    //logi(" COMPONENT_ROLE_ENUM");
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    CEDARC_UNUSE(pHComp);


    if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.wma", OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.wma", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.aac", OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.aac", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.mp3", OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.mp3", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.amrnb", OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.amrnb", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.amrwb", OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.amrwb", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.g711.alaw",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.g711alaw", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.g711.mlaw",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.g711mlaw", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.adpcm",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.adpcm", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.vorbis",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.ogg", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.ape",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.ape", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.dts",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.dts", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.raw",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.raw", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.audio.decoder.flac",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "audio_decoder.flac", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else
    {
        logd("\nERROR:Querying pRole on Unknown Component\n");
        eRet = OMX_ErrorInvalidComponentName;
    }

    return eRet;
}

static inline void setStateLoaded(omx_adec* pSelf)
{
    if (pSelf->m_state == OMX_StateIdle || pSelf->m_state == OMX_StateWaitForResources)
    {
        OMX_U32 nTimeout = 0x0;
        while (1)
        {
            //*Transition happens only when the ports are unpopulated
            if (!pSelf->m_sInPortDef.bPopulated && !pSelf->m_sOutPortDef.bPopulated)
            {
                pSelf->m_state = OMX_StateLoaded;
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventCmdComplete, OMX_CommandStateSet,
                                                pSelf->m_state, NULL);

                //* close decoder
                //* TODO.

                break;
            }
            else if (nTimeout++ > OMX_MAX_TIMEOUTS)
            {
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventError, OMX_ErrorInsufficientResources,
                                                0 , NULL);
                logw("Transition to loaded failed\n");
                break;
            }

            usleep(OMX_TIMEOUT*1000);logd("wait OMX_TIMEOUT ms");
        }

        if(pSelf->m_decoder != NULL)
        {
            DestroyAudioDecoder(pSelf->m_decoder);
            pSelf->m_decoder           = NULL;
            pSelf->mFirstInputDataFlag = OMX_TRUE;logd("****mFirstInputDataFlag = OMX_TRUE");
        }
        logd("(f:%s, l:%d)  DestroyAudioDecoder", __FUNCTION__, __LINE__);
    }
    else
    {
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventError, OMX_ErrorIncorrectStateTransition,
                                        0 , NULL);
    }
    return ;
}
static inline void setStateIdle(omx_adec* pSelf)
{
    if (pSelf->m_state == OMX_StateInvalid)
    {
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventError, OMX_ErrorIncorrectStateTransition,
                                        0 , NULL);
    }
    else
    {
        //*Return buffers if currently in pause and executing
        if (pSelf->m_state == OMX_StatePause || pSelf->m_state == OMX_StateExecuting)
        {
            pthread_mutex_lock(&pSelf->m_inBufMutex);

            while (pSelf->m_sInBufList.nSizeOfList > 0)
            {
                pSelf->m_sInBufList.nSizeOfList--;
                logd("*****nSizeOfList:%d",(int)pSelf->m_sInBufList.nSizeOfList);
                pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                pSelf->m_pAppData,
                pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

                if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                {
                    pSelf->m_sInBufList.nReadPos = 0;
                }
            }

            pthread_mutex_unlock(&pSelf->m_inBufMutex);


            pthread_mutex_lock(&pSelf->m_outBufMutex);

            while (pSelf->m_sOutBufList.nSizeOfList > 0)
            {
                pSelf->m_sOutBufList.nSizeOfList--;
                logd("*****nSizeOfList:%d",(int)pSelf->m_sOutBufList.nSizeOfList);
                pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp,
                pSelf->m_pAppData,
                pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

                if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                {
                    pSelf->m_sOutBufList.nReadPos = 0;
                }
            }

            pthread_mutex_unlock(&pSelf->m_outBufMutex);
        }
        else
        {
            ;
        }

        OMX_U32 nTimeout = 0x0;
        while (1)
        {
            //*Ports have to be populated before transition completes
            if ((!pSelf->m_sInPortDef.bEnabled && !pSelf->m_sOutPortDef.bEnabled)   ||
            (pSelf->m_sInPortDef.bPopulated && pSelf->m_sOutPortDef.bPopulated))
            {
                pSelf->m_state = OMX_StateIdle;
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventCmdComplete, OMX_CommandStateSet,
                                                pSelf->m_state, NULL);

                //* Open decoder
                //* TODO.
                break;
            }
            else if (nTimeout++ > OMX_MAX_TIMEOUTS)
            {
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventError, OMX_ErrorInsufficientResources,
                                                0 , NULL);
                logw("Idle transition failed\n");
                break;
            }

            usleep(OMX_TIMEOUT*1000);logd("wait OMX_TIMEOUT ms");
        }
    }
    return ;
}

static inline void setStateExecuting(omx_adec* pSelf)
{
    //*Transition can only happen from pause or idle state
    if (pSelf->m_state == OMX_StateIdle || pSelf->m_state == OMX_StatePause)
    {
        //*Return buffers if currently in pause
        if (pSelf->m_state == OMX_StatePause)
        {
            pthread_mutex_lock(&pSelf->m_inBufMutex);

            while (pSelf->m_sInBufList.nSizeOfList > 0)
            {
                pSelf->m_sInBufList.nSizeOfList--;
                logd("*****nSizeOfList:%d",(int)pSelf->m_sInBufList.nSizeOfList);
                pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                pSelf->m_pAppData,
                pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

                if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                {
                    pSelf->m_sInBufList.nReadPos = 0;
                }
            }

            pthread_mutex_unlock(&pSelf->m_inBufMutex);

            pthread_mutex_lock(&pSelf->m_outBufMutex);

            while (pSelf->m_sOutBufList.nSizeOfList > 0)
            {
                pSelf->m_sOutBufList.nSizeOfList--;
                logd("*****nSizeOfList:%d",(int)pSelf->m_sOutBufList.nSizeOfList);
                pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp,
                pSelf->m_pAppData,
                pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

                if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                {
                    pSelf->m_sOutBufList.nReadPos = 0;
                }
            }

            pthread_mutex_unlock(&pSelf->m_outBufMutex);
        }

        pSelf->m_state = OMX_StateExecuting;
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventCmdComplete, OMX_CommandStateSet,
                                        pSelf->m_state, NULL);
    }
    else
    {
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventError, OMX_ErrorIncorrectStateTransition,
                                        0 , NULL);
    }

    pSelf->mEosFlag             = OMX_FALSE;
    pSelf->pMarkData            = NULL;
    pSelf->hMarkTargetComponent = NULL;
    return ;
}

static inline void controlSetState(omx_adec* pSelf, OMX_U32 cmddata)
{
    //*If the parameter states a transition to the same state
    // raise a same state transition error.
    if (pSelf->m_state == (OMX_STATETYPE)(cmddata))
    {
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventError, OMX_ErrorSameState,
                                        0 , NULL);
    }
    else
    {
        //*transitions/callbacks made based on state transition table
        // cmddata contains the target state
        switch ((OMX_STATETYPE)(cmddata))
        {
            case OMX_StateInvalid:
                pSelf->m_state = OMX_StateInvalid;
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventError, OMX_ErrorInvalidState,
                                                0 , NULL);
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventCmdComplete, OMX_CommandStateSet,
                                                pSelf->m_state, NULL);
                break;

            case OMX_StateLoaded:
                setStateLoaded(pSelf);

                break;

            case OMX_StateIdle:
                setStateIdle(pSelf);
                break;

            case OMX_StateExecuting:
                setStateExecuting(pSelf);
                break;

            case OMX_StatePause:
                // Transition can only happen from idle or executing state
                if (pSelf->m_state == OMX_StateIdle || pSelf->m_state == OMX_StateExecuting)
                {
                    pSelf->m_state = OMX_StatePause;
                    pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandStateSet,
                                                    pSelf->m_state, NULL);
                }
                else
                {
                    pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventError, OMX_ErrorIncorrectStateTransition,
                                            0 , NULL);
                }

                break;

            case OMX_StateWaitForResources:
                if (pSelf->m_state == OMX_StateLoaded)
                {
                    pSelf->m_state = OMX_StateWaitForResources;
                    pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandStateSet,
                                                    pSelf->m_state, NULL);
                }
                else
                {
                    pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                        OMX_EventError, OMX_ErrorIncorrectStateTransition,
                        0 , NULL);
                }
                break;

            default:
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                    OMX_EventError, OMX_ErrorIncorrectStateTransition,
                                    0 , NULL);
                break;

        }
    }
}

static inline void controlStopPort(omx_adec* pSelf, OMX_U32 cmddata)
{
    logd(" stop port command, cmddata = %d.", (int)cmddata);

    //*Stop Port(s)
    // cmddata contains the port index to be stopped.
    // It is assumed that 0 is input and 1 is output port for this component
    // The cmddata value -1 means that both input and output ports will be stopped.
    if (cmddata == 0x0 || (OMX_S32)cmddata == -1)
    {
        //*Return all input buffers
        pthread_mutex_lock(&pSelf->m_inBufMutex);

        while (pSelf->m_sInBufList.nSizeOfList > 0)
        {
            pSelf->m_sInBufList.nSizeOfList--;
            logd("*****nSizeOfList:%d",(int)pSelf->m_sInBufList.nSizeOfList);
            pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
            pSelf->m_pAppData,
            pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

            if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
            {
                pSelf->m_sInBufList.nReadPos = 0;
            }
        }

        pthread_mutex_unlock(&pSelf->m_inBufMutex);

        //*Disable port
        pSelf->m_sInPortDef.bEnabled = OMX_FALSE;
    }

    if (cmddata == 0x1 || (OMX_S32)cmddata == -1)
    {
        //*Return all output buffers
        pthread_mutex_lock(&pSelf->m_outBufMutex);

        while (pSelf->m_sOutBufList.nSizeOfList > 0)
        {
            pSelf->m_sOutBufList.nSizeOfList--;
            logd("*****nSizeOfList:%d",(int)pSelf->m_sOutBufList.nSizeOfList);
            pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp,
            pSelf->m_pAppData,
            pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

            if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
            {
                pSelf->m_sOutBufList.nReadPos = 0;
            }
        }

        pthread_mutex_unlock(&pSelf->m_outBufMutex);

        // Disable port
        pSelf->m_sOutPortDef.bEnabled = OMX_FALSE;
    }

    // Wait for all buffers to be freed
    OMX_U32 nTimeout = 0x0;
    while (1)
    {
        if (cmddata == 0x0 && !pSelf->m_sInPortDef.bPopulated)
        {
            //*Return cmdcomplete event if input unpopulated
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            0x0, NULL);
            break;
        }

        if (cmddata == 0x1 && !pSelf->m_sOutPortDef.bPopulated)
        {
            //*Return cmdcomplete event if output unpopulated
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            0x1, NULL);
            break;
        }

        if ((OMX_S32)cmddata == -1
            &&  !pSelf->m_sInPortDef.bPopulated
            && !pSelf->m_sOutPortDef.bPopulated)
        {
            //*Return cmdcomplete event if inout & output unpopulated
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            0x0, NULL);
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            0x1, NULL);
            break;
        }

        if (nTimeout++ > OMX_MAX_TIMEOUTS)
        {
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                    OMX_EventError, OMX_ErrorPortUnresponsiveDuringDeallocation,
                                    0 , NULL);
            break;
        }

        usleep(OMX_TIMEOUT*1000);logd("wait OMX_TIMEOUT ms");
    }
    return ;
}

static inline void controlRestartPort(omx_adec* pSelf, OMX_U32 cmddata)
{
    logd(" restart port command.pSelf->m_state[%d]", pSelf->m_state);

    //*Restart Port(s)
    // cmddata contains the port index to be restarted.
    // It is assumed that 0 is input and 1 is output port for this component.
    // The cmddata value -1 means both input and output ports will be restarted.

    /*
    if (cmddata == 0x0 || (OMX_S32)cmddata == -1)
    pSelf->m_sInPortDef.bEnabled = OMX_TRUE;

    if (cmddata == 0x1 || (OMX_S32)cmddata == -1)
    pSelf->m_sOutPortDef.bEnabled = OMX_TRUE;
    */

    // Wait for port to be populated
    OMX_U32 nTimeout = 0x0;
    while (1)
    {
        // Return cmdcomplete event if input port populated
        if (cmddata == 0x0
            && (pSelf->m_state == OMX_StateLoaded || pSelf->m_sInPortDef.bPopulated))
        {
            pSelf->m_sInPortDef.bEnabled = OMX_TRUE;
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            0x0, NULL);
            break;
        }
        // Return cmdcomplete event if output port populated
        else if (cmddata == 0x1
                 && (pSelf->m_state == OMX_StateLoaded || pSelf->m_sOutPortDef.bPopulated))
        {
            pSelf->m_sOutPortDef.bEnabled = OMX_TRUE;
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            0x1, NULL);
            break;
        }
        // Return cmdcomplete event if input and output ports populated
        else if ((OMX_S32)cmddata == -1
                && (pSelf->m_state == OMX_StateLoaded
                    || (pSelf->m_sInPortDef.bPopulated && pSelf->m_sOutPortDef.bPopulated)))
        {
            pSelf->m_sInPortDef.bEnabled = OMX_TRUE;
            pSelf->m_sOutPortDef.bEnabled = OMX_TRUE;
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            0x0, NULL);
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            0x1, NULL);
            break;
        }
        else if (nTimeout++ > OMX_MAX_TIMEOUTS)
        {
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                        OMX_EventError, OMX_ErrorPortUnresponsiveDuringAllocation,
                        0, NULL);
            break;
        }

        usleep(OMX_TIMEOUT*1000);logd("wait OMX_TIMEOUT ms");
    }

    if(pSelf->port_setting_match == OMX_FALSE)
    {
        pSelf->port_setting_match = OMX_TRUE;
    }
}

static inline void controlFlush(omx_adec* pSelf, OMX_U32 cmddata)
{
    logd(" flush command.");

    //*Flush port(s)
    // cmddata contains the port index to be flushed.
    // It is assumed that 0 is input and 1 is output port for this component
    // The cmddata value -1 means that both input and output ports will be flushed.

    //if request flush input and output port, we reset decoder!
    if(cmddata == OMX_ALL || cmddata == 0x1 || (OMX_S32)cmddata == -1)
    {
        logd(" flush all port! we reset decoder!");
        if(pSelf->m_decoder)
        {
            ResetAudioDecoder(pSelf->m_decoder,0);
        }
        else
        {
            logw(" fatal error, m_decoder is not malloc when flush all ports!");
        }
    }
    if (cmddata == 0x0 || (OMX_S32)cmddata == -1)
    {
        // Return all input buffers and send cmdcomplete
        pthread_mutex_lock(&pSelf->m_inBufMutex);

        while (pSelf->m_sInBufList.nSizeOfList > 0)
        {
            pSelf->m_sInBufList.nSizeOfList--;
            logd("*****nSizeOfList:%d",(int)pSelf->m_sInBufList.nSizeOfList);
            pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
            pSelf->m_pAppData,
            pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

            if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
            {
                pSelf->m_sInBufList.nReadPos = 0;
            }
        }

        pthread_mutex_unlock(&pSelf->m_inBufMutex);

        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventCmdComplete, OMX_CommandFlush,
                                        0x0, NULL);
    }

    if (cmddata == 0x1 || (OMX_S32)cmddata == -1)
    {
        // Return all output buffers and send cmdcomplete
        pthread_mutex_lock(&pSelf->m_outBufMutex);

        while (pSelf->m_sOutBufList.nSizeOfList > 0)
        {
             pSelf->m_sOutBufList.nSizeOfList--;
             logd("*****nSizeOfList:%d",(int)pSelf->m_sOutBufList.nSizeOfList);
             pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp,
             pSelf->m_pAppData,
             pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

             if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
             {
                 pSelf->m_sOutBufList.nReadPos = 0;
             }
        }

        pthread_mutex_unlock(&pSelf->m_outBufMutex);

        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventCmdComplete, OMX_CommandFlush,
                                        0x1, NULL);
    }
    return ;
}

static inline void controlFillBuf(omx_adec* pSelf, OMX_U32 cmddata)
{
    //*Fill buffer
    logd("****cmd = FillBuf,pSelf->m_sOutBufList.nSizeOfList:%d,nAllocSize:%d",
         (int)pSelf->m_sOutBufList.nSizeOfList,(int)pSelf->m_sOutBufList.nAllocSize);
    pthread_mutex_lock(&pSelf->m_outBufMutex);
    if (pSelf->m_sOutBufList.nSizeOfList < pSelf->m_sOutBufList.nAllocSize)
    {
        pSelf->m_sOutBufList.nSizeOfList++;
        pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nWritePos++]
            = ((OMX_BUFFERHEADERTYPE*) cmddata);
        if (pSelf->m_sOutBufList.nWritePos >= (OMX_S32)pSelf->m_sOutBufList.nAllocSize)
        {
            pSelf->m_sOutBufList.nWritePos = 0;
        }
    }
    pthread_mutex_unlock(&pSelf->m_outBufMutex);
    return ;
}

static inline int controlEmptyBuf(omx_adec* pSelf, OMX_U32 cmddata)
{
    OMX_BUFFERHEADERTYPE* pTmpInBufHeader = (OMX_BUFFERHEADERTYPE*) cmddata;
    OMX_TICKS   nInterval;
    OMX_BUFFERHEADERTYPE*   pInBufHdr   = NULL;
    logd("****cmd = EmptyBuf,pSelf->m_sInBufList.nSizeOfList:%d,nAllocSize:%d",
        (int)pSelf->m_sInBufList.nSizeOfList,(int)pSelf->m_sInBufList.nAllocSize);
    //*Empty buffer
    pthread_mutex_lock(&pSelf->m_inBufMutex);
    if(pSelf->m_nInportPrevTimeStamp)
    {
        //compare pts to decide if jump
        nInterval = pTmpInBufHeader->nTimeStamp - pSelf->m_nInportPrevTimeStamp;
        if(nInterval < -AWOMX_PTS_JUMP_THRESH || nInterval > AWOMX_PTS_JUMP_THRESH)
        {
            logd("pts jump [%lld]us, prev[%lld],cur[%lld],need reset vdeclib",
                  nInterval, pSelf->m_nInportPrevTimeStamp, pTmpInBufHeader->nTimeStamp);
            pSelf->m_JumpFlag = OMX_TRUE;
        }

        pSelf->m_nInportPrevTimeStamp = pTmpInBufHeader->nTimeStamp;
    }
    else //first InBuf data
    {
        pSelf->m_nInportPrevTimeStamp = pTmpInBufHeader->nTimeStamp;
    }

    if (pSelf->m_sInBufList.nSizeOfList < pSelf->m_sInBufList.nAllocSize)
    {
        pSelf->m_sInBufList.nSizeOfList++;
        pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nWritePos++]
            = ((OMX_BUFFERHEADERTYPE*) cmddata);

        if (pSelf->m_sInBufList.nWritePos >= (OMX_S32)pSelf->m_sInBufList.nAllocSize)
        {
            pSelf->m_sInBufList.nWritePos = 0;
        }
    }

    logd("(omx_adec, f:%s, l:%d) nTimeStamp[%lld], nAllocLen[%d], nFilledLen[%d],\
          nOffset[%d], nFlags[0x%x], nOutputPortIndex[%d], nInputPortIndex[%d],nSizeOfList[%d]",
          __FUNCTION__, __LINE__,
    pTmpInBufHeader->nTimeStamp,
    (int)pTmpInBufHeader->nAllocLen,
    (int)pTmpInBufHeader->nFilledLen,
    (int)pTmpInBufHeader->nOffset,
    (int)pTmpInBufHeader->nFlags,
    (int)pTmpInBufHeader->nOutputPortIndex,
    (int)pTmpInBufHeader->nInputPortIndex,
    (int)pSelf->m_sInBufList.nSizeOfList);

    pthread_mutex_unlock(&pSelf->m_inBufMutex);
    logd("****mFirstInputDataFlag:%d",pSelf->mFirstInputDataFlag);
    if(pSelf->mFirstInputDataFlag==OMX_TRUE && pSelf->m_sInBufList.nSizeOfList>0)
    {
        pInBufHdr = pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos];

        if(pInBufHdr != NULL)
        {
            int ret = -1;
            pSelf->mFirstInputDataFlag = OMX_FALSE;
            logd("*** the first pInBufHdr->nFlags = 0x%x",(int)pInBufHdr->nFlags);
            pSelf->m_streamInfo.eSubCodecFormat = pSelf->m_codec_tag;
            pSelf->m_streamInfo.nChannelNum     = pSelf->m_channels;
            pSelf->m_streamInfo.nSampleRate     = pSelf->m_sampleRate;
            pSelf->m_streamInfo.nAvgBitrate     = pSelf->m_bitRate;
            pSelf->m_streamInfo.nBlockAlign     = pSelf->m_blockalign;
            pSelf->m_streamInfo.nBitsPerSample  = pSelf->m_bitspersample;
            if (pInBufHdr->nFlags & OMX_BUFFERFLAG_CODECCONFIG)
            {
                int i =0;
                logd("***nFilledLen %d",(int)pInBufHdr->nFilledLen);
                for(i=0;i<(int)pInBufHdr->nFilledLen;i++)
                {
                    logd("***pCodecSpecificData[0x%02x] 0x%02x",i,pInBufHdr->pBuffer[i]);
                }
                if(pSelf->m_streamInfo.pCodecSpecificData)
                {
                    free(pSelf->m_streamInfo.pCodecSpecificData);
                }

                pSelf->m_streamInfo.nCodecSpecificDataLen = pInBufHdr->nFilledLen;
                pSelf->m_streamInfo.pCodecSpecificData
                    = (char*)malloc(pSelf->m_streamInfo.nCodecSpecificDataLen);
                memset(pSelf->m_streamInfo.pCodecSpecificData,0,
                       pSelf->m_streamInfo.nCodecSpecificDataLen);
                memcpy(pSelf->m_streamInfo.pCodecSpecificData,
                       pInBufHdr->pBuffer,pSelf->m_streamInfo.nCodecSpecificDataLen);
                if (pSelf->m_state != OMX_StateExecuting)
                {
                    logd("fatal error! when vdrv OMX_AdrvCommand_PrepareAdecLib,\
                          m_state[0x%x] should be OMX_StateLoaded", pSelf->m_state);
                }

                //*if mdecoder had closed before, we should create it
                if(pSelf->m_decoder==NULL)
                {
                    pSelf->m_decoder = CreateAudioDecoder();
                    BsInFor m_audioConfig;
                    memset(&m_audioConfig,0,sizeof(BsInFor));
                    ret = InitializeAudioDecoder(pSelf->m_decoder,
                                                 &(pSelf->m_streamInfo),&m_audioConfig);
                    if(ret != 0)
                    {
                        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                        OMX_EventError, OMX_ErrorHardware,
                                                        0 , NULL);
                        logd("Idle transition failed, set_vstream_info() return fail.\n");
                        return OMX_ADEC_RESULT_EXIT;
                    }
                }
                else
                {
                    logd("CreateAudioDecoder again");
                }

                pSelf->m_sInBufList.nSizeOfList--;
                pSelf->m_sInBufList.nReadPos++;
                logd("*****nSizeOfList:%d",(int)pSelf->m_sInBufList.nSizeOfList);
                if (pSelf->m_sInBufList.nReadPos >= (OMX_S32)pSelf->m_sInBufList.nAllocSize)
                {
                    pSelf->m_sInBufList.nReadPos = 0;
                }

                pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData, pInBufHdr);
            }
            else
            {
                if (pSelf->m_state != OMX_StateExecuting)
                {
                    logw("fatal error! when vdrv OMX_AdrvCommand_PrepareAdecLib, \
                          m_state[0x%x] should be OMX_StateLoaded", pSelf->m_state);
                }

                //*if mdecoder had closed before, we should create it
                if(pSelf->m_decoder==NULL)
                {
                    pSelf->m_decoder = CreateAudioDecoder();
                    BsInFor m_audioConfig;
                    memset(&m_audioConfig,0,sizeof(BsInFor));
                    ret = InitializeAudioDecoder(pSelf->m_decoder,
                                                 &(pSelf->m_streamInfo),&m_audioConfig);
                    if(ret != 0)
                    {
                        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                        OMX_EventError, OMX_ErrorHardware,
                                                        0 , NULL);
                        logd("Idle transition failed, set_vstream_info() return fail.\n");
                        return OMX_ADEC_RESULT_EXIT;
                    }
                }
                else
                {
                    logd("CreateAudioDecoder again");
                }
            }
            pInBufHdr = NULL;

        }


    }
    //*Mark current buffer if there is outstanding command
    if (pSelf->pMarkBuf)
    {
        ((OMX_BUFFERHEADERTYPE *)(cmddata))->hMarkTargetComponent = &pSelf->mOmxCmp;
        ((OMX_BUFFERHEADERTYPE *)(cmddata))->pMarkData = pSelf->pMarkBuf->pMarkData;
        pSelf->pMarkBuf = NULL;
    }
    return OMX_ADEC_RESULT_OK;
}

static inline int processThreadCommand(omx_adec* pSelf, ThrCmdType cmd, OMX_U32 cmddata)
{
    logd(" set state command, cmd = %d, cmddata = %d.", (int)cmd, (int)cmddata);
    int ret = OMX_ADEC_RESULT_OK;
    //*State transition command
    if (cmd == SetState)
    {
        controlSetState(pSelf, cmddata);
    }
    else if (cmd == StopPort)
    {
        controlStopPort(pSelf, cmddata);
    }
    else if (cmd == RestartPort)
    {
        controlRestartPort(pSelf, cmddata);
    }
    else if (cmd == Flush)
    {
        controlFlush(pSelf, cmddata);
    }
    else if (cmd == Stop)
    {
        logd(" stop command.");
        // Kill thread
        ret = OMX_ADEC_RESULT_EXIT;
    }
    else if (cmd == FillBuf)
    {
        controlFillBuf(pSelf, cmddata);
    }
    else if (cmd == EmptyBuf)
    {
        ret = controlEmptyBuf(pSelf, cmddata);
    }
    else if (cmd == MarkBuf)
    {
        if (!pSelf->pMarkBuf)
            pSelf->pMarkBuf = (OMX_MARKTYPE *)(cmddata);
    }

    return ret;
}

static inline int sendStreamDataToDecoder(omx_adec * pSelf)
{
    OMX_U8* pBuf0;
    OMX_U8* pBuf1;
    int size0;
    int size1;
    OMX_U8* pData;
    OMX_BUFFERHEADERTYPE*   pInBufHdr   = NULL;
    int nOffset = 0;

    pInBufHdr = NULL;
    //* check if there is a input bit stream.
    pthread_mutex_lock(&pSelf->m_inBufMutex);

    if(pSelf->m_sInBufList.nSizeOfList > 0)
    {
        pSelf->m_sInBufList.nSizeOfList;
        pInBufHdr = pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos];
    }

    pthread_mutex_unlock(&pSelf->m_inBufMutex);
    logd("****inputnum:%d,ptr:%p",(int)pSelf->m_sInBufList.nSizeOfList,pInBufHdr);
    logd("****line:%d,nOffset:%d",__LINE__,nOffset);
    //* add stream to decoder.
    if(pInBufHdr)
    {
        int require_size = pInBufHdr->nFilledLen;
        logd("****line:%d,nOffset:%d",__LINE__,nOffset);
        //DBG_WARNING("vbs data size[%d]", require_size);
        int ret = ParserRequestBsBuffer(pSelf->m_decoder, require_size,
                                    &pBuf0, &size0, &pBuf1, &size1,&nOffset);
        if(ret != 0)
        {
            ALOGD("req vbs fail!");
            return OMX_ADEC_RESULT_CONTINUE;
        }
        logd("****line:%d,nOffset:%d",__LINE__,nOffset);

        if(require_size <= size0)
        {
            pData = pInBufHdr->pBuffer + pInBufHdr->nOffset;
            memcpy(pBuf0, pData, require_size);
        }
        else
        {
            pData = pInBufHdr->pBuffer + pInBufHdr->nOffset;
            memcpy(pBuf0, pData, size0);
            pData += size0;
            memcpy(pBuf1, pData, require_size - size0);
        }
        logd("****line:%d,nOffset:%d",__LINE__,nOffset);
        ParserUpdateBsBuffer(pSelf->m_decoder, pInBufHdr->nFilledLen,
        pInBufHdr->nTimeStamp,
        nOffset);logd("****line:%d,nOffset:%d",__LINE__,nOffset);
        //DBG_WARNING("EmptyBufferDone is called");
        pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData, pInBufHdr);
        pSelf->m_sInBufList.nSizeOfList--;
        pSelf->m_sInBufList.nReadPos++;
        if (pSelf->m_sInBufList.nReadPos >= (OMX_S32)pSelf->m_sInBufList.nAllocSize)
        {
            pSelf->m_sInBufList.nReadPos = 0;
        }
        if (pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
        {
            // Copy flag to output buffer header
            pSelf->mEosFlag = OMX_TRUE;

            ALOGD(" set up eos flag.");

            // Trigger event handler
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventBufferFlag, 0x1,
                                            pInBufHdr->nFlags, NULL);

            // Clear flag
            pInBufHdr->nFlags = 0;
        }

        // Check for mark buffers
        if (pInBufHdr->pMarkData)
        {
            // Copy mark to output buffer header
            if (pSelf->pMarkData == NULL && pSelf->hMarkTargetComponent == NULL)
            {
                pSelf->pMarkData = pInBufHdr->pMarkData;
                // Copy handle to output buffer header
                pSelf->hMarkTargetComponent = pInBufHdr->hMarkTargetComponent;
            }
        }

        // Trigger event handler
        if (pInBufHdr->hMarkTargetComponent == &pSelf->mOmxCmp && pInBufHdr->pMarkData)
        {
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventMark, 0, 0,
                                            pInBufHdr->pMarkData);
        }
        //pInBufHdr = NULL;
    }
    return OMX_ADEC_RESULT_OK;
}

/*
 *  Component Thread
 *    The ComponentThread function is exeuted in a separate pThread and
 *    is used to implement the actual component functions.
 */
 /*****************************************************************************/
static void* ComponentThread(void* pThreadData)
{
    int                     decodeResult;
    //    VideoPicture*           picture;

    int                     i;
    int                     fd1;
    fd_set                  rfds;
    OMX_U32                 cmddata;
    ThrCmdType              cmd;

    // Variables related to decoder buffer handling
    OMX_BUFFERHEADERTYPE*   pInBufHdr   = NULL;
    OMX_BUFFERHEADERTYPE*   pOutBufHdr  = NULL;
    OMX_U8*                 pInBuf      = NULL;
    OMX_U32                 nInBufSize;

    // Variables related to decoder timeouts
    OMX_U32                 nTimeout;


    struct timeval          timeout;

    int readRet = 0;


    // Recover the pointer to my component specific data
    omx_adec* pSelf = static_cast<omx_adec*>(pThreadData);

    while (1)
    {
        fd1 = pSelf->m_cmdpipe[0];
        FD_ZERO(&rfds);
        FD_SET(fd1, &rfds);

        //*Check for new command
        timeout.tv_sec  = 0;
        timeout.tv_usec = 0;

        i = select(pSelf->m_cmdpipe[0]+1, &rfds, NULL, NULL, &timeout);
        //logd("*****************");
        if (FD_ISSET(pSelf->m_cmdpipe[0], &rfds))
        {
            //*retrieve command and data from pipe
            //logd("*****************");
            readRet = read(pSelf->m_cmdpipe[0], &cmd, sizeof(cmd));
            if(readRet<0)
            {
                logd("error: read pipe data failed!,ret = %d",(int)readRet);
                goto EXIT;
            }

            readRet = read(pSelf->m_cmddatapipe[0], &cmddata, sizeof(cmddata));
            if(readRet<0)
            {
                logd("error: read pipe data failed!,ret = %d",(int)readRet);
                goto EXIT;
            }

            logd(" set state command, cmd = %d, cmddata = %d.", (int)cmd, (int)cmddata);//test
            if(processThreadCommand(pSelf, cmd, cmddata) == OMX_ADEC_RESULT_EXIT)
                goto EXIT;
        }

        if (pSelf->m_state == OMX_StateExecuting  &&
        pSelf->m_sInPortDef.bEnabled          &&
        pSelf->m_sOutPortDef.bEnabled         &&
        pSelf->port_setting_match == OMX_TRUE)
        {
            if(OMX_TRUE == pSelf->m_JumpFlag)
            {
                logd("reset vdeclib for jump!");
                pSelf->m_JumpFlag = OMX_FALSE;
            }
            pthread_mutex_lock(&pSelf->m_outBufMutex);

            if(pSelf->m_sOutBufList.nSizeOfList > 0)
            {
                pSelf->m_sOutBufList.nSizeOfList;
                pOutBufHdr = pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos];
            }
            else
            {
                pOutBufHdr = NULL;
            }

            pthread_mutex_unlock(&pSelf->m_outBufMutex);

            //* if no output buffer, wait for some time.
            if(pOutBufHdr == NULL)
            {
                continue;
            }

            if(pSelf->mFirstInputDataFlag==OMX_TRUE)
            {
                logd("mFirstInputDataFlag init no ok");
                continue;
            }
            int vbv = 0;
            int ValidPercent = 0;
            BsQueryQuality(pSelf->m_decoder,&ValidPercent,&vbv);
            if((pSelf->mEosFlag)&&(vbv==0))
            {
                //set eof flag, MediaCodec use this flag
                //to determine whether a playback is finished.
                pthread_mutex_lock(&pSelf->m_outBufMutex);
                while(pSelf->m_sOutBufList.nSizeOfList > 0)
                {
                    pSelf->m_sOutBufList.nSizeOfList--;
                    pOutBufHdr = pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++];
                    if (pSelf->m_sOutBufList.nReadPos >= (OMX_S32)pSelf->m_sOutBufList.nAllocSize)
                    {
                        pSelf->m_sOutBufList.nReadPos = 0;
                    }
                    //                                CHECK(pOutBufHdr != NULL);
                    if(pSelf->m_sOutBufList.nSizeOfList == 0)
                    {
                        logd("(f:%s, l:%d), EOS msg send with FillBufferDone",
                              __FUNCTION__, __LINE__);
                        pOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                    }
                    pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp,
                                                      pSelf->m_pAppData, pOutBufHdr);
                    pOutBufHdr = NULL;
                }
                pthread_mutex_unlock(&pSelf->m_outBufMutex);
                logd("end file!!!!!!!");
                break;

            }
            if((pSelf->m_sInBufList.nSizeOfList==0)&&(vbv==0))
            {
                  logd("pInBufHdr = 0,no bs data");
                 continue;
            }
            logd("****list:%d,vbv:%d",(int)pSelf->m_sInBufList.nSizeOfList,vbv);
            if((pSelf->m_sInBufList.nSizeOfList>0)&&(vbv<4))
            {
                if(sendStreamDataToDecoder(pSelf) == OMX_ADEC_RESULT_CONTINUE)
                    continue ;
            }

            pOutBufHdr->nOffset    = 0;
            decodeResult = DecodeAudioStream(pSelf->m_decoder,&(pSelf->m_streamInfo),
                                            (char*)(pOutBufHdr->pBuffer + pOutBufHdr->nOffset),
                                            (int*)(&pOutBufHdr->nFilledLen));
            pOutBufHdr->nTimeStamp = PlybkRequestPcmPts(pSelf->m_decoder);
            logd("***decodeResult = %d",decodeResult);
            if((decodeResult==ERR_AUDIO_DEC_NONE)
                &&((pSelf->m_channel != pSelf->m_streamInfo.nChannelNum)
                   ||(pSelf->m_sampleRate != pSelf->m_streamInfo.nSampleRate)))
            {
                logd("****fs:%d,m_fs:%d,ch:%d,m_ch:%d",
                     pSelf->m_streamInfo.nSampleRate,pSelf->m_sampleRate,
                     pSelf->m_streamInfo.nChannelNum,pSelf->m_channel);
                pSelf->m_channel = pSelf->m_streamInfo.nChannelNum;
                pSelf->m_sampleRate = pSelf->m_streamInfo.nSampleRate;
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventPortSettingsChanged, 0x01,
                                                OMX_IndexConfigCommonOutputCrop, NULL);
            }

            if(pOutBufHdr->nFilledLen)
            {
                pSelf->m_sOutBufList.nSizeOfList--;
                pSelf->m_sOutBufList.nReadPos++;
                logd("*****nSizeOfList:%d",(int)pSelf->m_sOutBufList.nSizeOfList);
                if (pSelf->m_sOutBufList.nReadPos >= (OMX_S32)pSelf->m_sOutBufList.nAllocSize)
                {
                    pSelf->m_sOutBufList.nReadPos = 0;
                }
                pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData, pOutBufHdr);
                pOutBufHdr = NULL;

            }

            if(decodeResult == ERR_AUDIO_DEC_NONE ||
            decodeResult == ERR_AUDIO_DEC_FFREVRETURN ||
            decodeResult == ERR_AUDIO_DEC_VIDEOJUMP)
            {
                continue;
            }
            else if(decodeResult == ERR_AUDIO_DEC_NO_BITSTREAM )
            {
                continue;
            }
            else if(decodeResult < 0)
            {
                logd("decode fatal error[%d]", decodeResult);
                continue;
            }
        }
    }

    EXIT:
    if(pSelf->m_decoder != NULL)
    {
        DestroyAudioDecoder(pSelf->m_decoder);
        pSelf->m_decoder = NULL;
        pSelf->mFirstInputDataFlag = OMX_TRUE;
        logd("DestroyAudioDecoder");
            logd("*****mFirstInputDataFlag = OMX_TRUE");
    }
    return (void*)OMX_ErrorNone;
}
