/******************************************************************************
  Copyright (C), 2001-2016, Allwinner Tech. Co., Ltd.
 ******************************************************************************
  File Name     : AIChannel_Component.c
  Version       : Initial Draft
  Author        : Allwinner BU3-PD2 Team
  Created       : 2016/04/27
  Last Modified :
  Description   : mpi functions implement
  Function List :
  History       :
******************************************************************************/

//#define LOG_NDEBUG 0
#define LOG_TAG "AIChannel_Component"
#include <utils/plat_log.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <memory.h>

#include <mm_component.h>
#include <tmessage.h>
#include <tsemaphore.h>
#include <SystemBase.h>
#include "AIChannel_Component.h"
#include <AIOCompStream.h>

#include <ConfigOption.h>

#include <cdx_list.h>

#define AEC_IMPLEMENT_IN_AI_CHL 0
//#define AI_SAVE_AUDIO_PCM

static void *AIChannel_ComponentThread(void *pThreadData);


static ERRORTYPE AIChannel_GetFrameRef( PARAM_IN COMP_HANDLETYPE hComponent, PARAM_OUT AUDIO_FRAME_S *pAudioFrame)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S *)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    PcmBufferManager *pPlayMgr = pChnData->mpPlayMgr;
    ERRORTYPE eError = SUCCESS; 

    if (COMP_StateIdle != pChnData->state && COMP_StateExecuting != pChnData->state)
    {
        alogw("call GetFrame() in wrong state[0x%x]", pChnData->state);
        return ERR_AI_NOT_PERM;
    }

    if(FALSE == pChnData->mInputPortTunnelFlag[AI_CHN_PORT_INDEX_AO_IN])    // only for ao tunneled to ai mode
    {
        aloge("aec_ref_buff_invalid_tunnel_mode");
        return ERR_AI_NOT_PERM;
    } 

    if (FALSE == pPlayMgr->validFrmEmpty(pPlayMgr))
    {
        AUDIO_FRAME_S *pValidFrame = pPlayMgr->getValidFrame(pPlayMgr);

        pAudioFrame->mBitwidth = pValidFrame->mBitwidth;
        pAudioFrame->mSamplerate = pValidFrame->mSamplerate;
        pAudioFrame->mSoundmode = pValidFrame->mSoundmode;
        pAudioFrame->mTimeStamp = pValidFrame->mTimeStamp;
        pAudioFrame->tmp_pts = pValidFrame->tmp_pts;
        pAudioFrame->mLen = pValidFrame->mLen;

        if(NULL != pAudioFrame->mpAddr)
        {
            memcpy((char *)pAudioFrame->mpAddr,(char *)pValidFrame->mpAddr,pValidFrame->mLen);
        }
        else
        {
            aloge("aec_get_ref_fatal_error:%p-%p-%d",pAudioFrame->mpAddr,pValidFrame->mpAddr,pValidFrame->mLen);
        } 
        
        pPlayMgr->releaseFrame(pPlayMgr, pValidFrame);
        eError = SUCCESS;
    }
    else
    {
        eError = ERR_AI_BUF_EMPTY;
    }

    return eError;
}




/**
 * get frame, used in non-tunnel mode.
 *
 * @return SUCCESS.
 * @param hComponent ai component.
 * @param pAudioFrame store frame info, caller malloc.
 * @param nMilliSec 0:return immediately, <0:wait forever, >0:wait some time.
 */
static ERRORTYPE AIChannel_GetFrame(
    PARAM_IN COMP_HANDLETYPE hComponent,
    PARAM_OUT AUDIO_FRAME_S *pAudioFrame,
    PARAM_IN int nMilliSec)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S *)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    PcmBufferManager *pCapMgr = pChnData->mpCapMgr;
    ERRORTYPE eError = SUCCESS;
    int ret;

    if (COMP_StateIdle != pChnData->state && COMP_StateExecuting != pChnData->state)
    {
        alogw("call GetFrame() in wrong state[0x%x]", pChnData->state);
        return ERR_AI_NOT_PERM;
    }
    if (pChnData->mOutputPortTunnelFlag[AI_OUTPORT_SUFFIX_AENC] || pChnData->mOutputPortTunnelFlag[AI_OUTPORT_SUFFIX_AO])
    {
        aloge("fatal error! can't call GetFrame() in tunnel mode!");
        return ERR_AI_NOT_PERM;
    }

_TryToGetOutFrame:
    if (FALSE == pCapMgr->validFrmEmpty(pCapMgr))
    {
        AUDIO_FRAME_S *pValidFrame = pCapMgr->getValidFrame(pCapMgr);
        memcpy(pAudioFrame, pValidFrame, sizeof(AUDIO_FRAME_S));
        //cdx_sem_signal(&pChnData->mWaitGetAllOutFrameSem);
        eError = SUCCESS;
    }
    else
    {
        if (nMilliSec == 0)
        {
            eError = ERR_AI_BUF_EMPTY;
        }
        else if (nMilliSec < 0)
        {
            //pChnData->mWaitingOutFrameFlag = TRUE;
            cdx_sem_down(&pChnData->mWaitOutFrameSem);
            //pChnData->mWaitingOutFrameFlag = FALSE;
            goto _TryToGetOutFrame;
        }
        else
        {
            //pChnData->mWaitingOutFrameFlag = TRUE;
            ret = cdx_sem_down_timedwait(&pChnData->mWaitOutFrameSem, nMilliSec);
            if (ETIMEDOUT == ret)
            {
                alogv("wait output frame timeout[%d]ms, ret[%d]", nMilliSec, ret);
                eError = ERR_AI_BUF_EMPTY;
                //pChnData->mWaitingOutFrameFlag = FALSE;
            }
            else if (0 == ret)
            {
                //pChnData->mWaitingOutFrameFlag = FALSE;
                goto _TryToGetOutFrame;
            }
            else
            {
                aloge("fatal error! AI pthread cond wait timeout ret[%d]", ret);
                eError = ERR_AI_BUF_EMPTY;
                //pChnData->mWaitingOutFrameFlag = FALSE;
            }
        }
    }

    return eError;
}

/**
 * release frame, used in non-tunnel mode.
 *
 * @return SUCCESS.
 * @param hComponent ai component.
 * @param pAudioFrame frame info.
 */
static ERRORTYPE AIChannel_ReleaseFrame(
    PARAM_IN COMP_HANDLETYPE hComponent,
    PARAM_IN AUDIO_FRAME_S *pAudioFrame)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S *)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    PcmBufferManager *pCapMgr = pChnData->mpCapMgr;
    ERRORTYPE eError = SUCCESS;
    int ret;

    if (COMP_StateIdle != pChnData->state && COMP_StateExecuting != pChnData->state)
    {
        alogw("call ReleaseFrame in wrong state[0x%x]", pChnData->state);
        return ERR_AI_SYS_NOTREADY;
    }
    if (pChnData->mOutputPortTunnelFlag[AI_OUTPORT_SUFFIX_AENC] || pChnData->mOutputPortTunnelFlag[AI_OUTPORT_SUFFIX_AO])
    {
        aloge("fatal error! can't call ReleaseFrame in tunnel mode!");
        return ERR_AI_NOT_PERM;
    }

    if (FALSE == pCapMgr->usingFrmEmpty(pCapMgr))
    {
        pCapMgr->releaseFrame(pCapMgr, pAudioFrame);
        if (pChnData->mWaitAllFrameReleaseFlag)
            cdx_sem_signal(&pChnData->mAllFrameRelSem);
        eError = SUCCESS;
    }
    else
    {
        alogw("Be careful! AI frame[%p][%u] is not find, maybe reset channel before call this function?",
            pAudioFrame->mpAddr, pAudioFrame->mLen);
        eError = ERR_AI_ILLEGAL_PARAM;
    }

    return eError;
}

static ERRORTYPE AIChannel_SetSaveFileInfo(
    PARAM_IN COMP_HANDLETYPE hComponent,
    PARAM_IN AUDIO_SAVE_FILE_INFO_S *pFileInfo)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S *)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    if (COMP_StateIdle != pChnData->state && COMP_StateExecuting != pChnData->state)
    {
        aloge("call SetSaveFileInfo in wrong state[0x%x]!", pChnData->state);
        return ERR_AI_NOT_PERM;
    }

    int nPathLen = strlen(pFileInfo->mFilePath) + strlen(pFileInfo->mFileName) + 1;
    pChnData->mpSaveFileFullPath = (char*)malloc(nPathLen);
    if (NULL == pChnData->mpSaveFileFullPath)
    {
        aloge("malloc %d fail! FilePath:[%s], FileName:[%s]", nPathLen, pFileInfo->mFilePath, pFileInfo->mFileName);
        return ERR_AI_NOMEM;
    }

    memset(pChnData->mpSaveFileFullPath, 0, nPathLen);
    strcpy(pChnData->mpSaveFileFullPath, pFileInfo->mFilePath);
    strcat(pChnData->mpSaveFileFullPath, pFileInfo->mFileName);
    pChnData->mFpSaveFile = fopen(pChnData->mpSaveFileFullPath, "wb+");
    if (pChnData->mFpSaveFile)
    {
        alogd("create file(%s) to save pcm file", pChnData->mpSaveFileFullPath);
        pChnData->mSaveFileFlag = TRUE;
        pChnData->mSaveFileSize = 0;
    }
    else
    {
        aloge("create file(%s) failed!", pChnData->mpSaveFileFullPath);
        pChnData->mSaveFileFlag = FALSE;
    }

    return SUCCESS;
}

static ERRORTYPE AIChannel_QueryFileStatus(
    PARAM_IN COMP_HANDLETYPE hComponent,
    PARAM_OUT AUDIO_SAVE_FILE_INFO_S *pFileInfo)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S *)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    if (COMP_StateIdle != pChnData->state && COMP_StateExecuting != pChnData->state)
    {
        aloge("call SetSaveFileInfo in wrong state[0x%x]!", pChnData->state);
        return ERR_AI_NOT_PERM;
    }

    memset(pFileInfo, 0, sizeof(AUDIO_SAVE_FILE_INFO_S));
    if (pChnData->mSaveFileFlag)
    {
        pFileInfo->bCfg = pChnData->mSaveFileFlag;
        pFileInfo->mFileSize = pChnData->mSaveFileSize;
        const char *ptr = strrchr(pChnData->mpSaveFileFullPath, '/');
        int pathLen = ptr - pChnData->mpSaveFileFullPath;
        strncpy(pFileInfo->mFilePath, pChnData->mpSaveFileFullPath, pathLen);
        strcpy(pFileInfo->mFileName, ptr);
    }
    else
    {
        alogw("AI NOT in save file status!");
    }
    return SUCCESS;
}

static ERRORTYPE AIChannel_SetChnMute(
        PARAM_IN COMP_HANDLETYPE hComponent, 
        PARAM_IN BOOL bMute)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S *)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    pChnData->mbMute = bMute;
    return SUCCESS;
}

static ERRORTYPE AIChannel_GetChnMute(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_OUT BOOL *pbMute)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S *)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    *pbMute = pChnData->mbMute;
    return SUCCESS;
}
static ERRORTYPE AIChannel_IgnoreData(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_IN BOOL bIgnore)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S *)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    pthread_mutex_lock(&pChnData->mIgnoreDataLock);
    pChnData->mbIgnore = bIgnore;
    pthread_mutex_unlock(&pChnData->mIgnoreDataLock);
    return SUCCESS;
}

static ERRORTYPE AIChannel_GetChnTunneledFlag(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_OUT BOOL *pbTunnel)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S *)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    *pbTunnel = pChnData->mInputPortTunnelFlag[AI_CHN_PORT_INDEX_AO_IN];
    return SUCCESS;
}


static ERRORTYPE AIChannel_SendCommand(PARAM_IN COMP_HANDLETYPE hComponent,
    PARAM_IN COMP_COMMANDTYPE Cmd, PARAM_IN unsigned int nParam1, PARAM_IN void* pCmdData)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S *)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    CompInternalMsgType eCmd;
    ERRORTYPE eError = SUCCESS;
    message_t msg;

    //alogv("Command: %d", Cmd);
    if (NULL == pChnData) {
        eError = ERR_AI_ILLEGAL_PARAM;
        goto COMP_CONF_CMD_BAIL;
    }

    if (Cmd == COMP_CommandMarkBuffer) {
        if (NULL == pCmdData) {
            eError = ERR_AI_ILLEGAL_PARAM;
            goto COMP_CONF_CMD_BAIL;
        }
    }

    if (pChnData->state == COMP_StateInvalid) {
        eError = ERR_AI_SYS_NOTREADY;
        goto COMP_CONF_CMD_BAIL;
    }

    switch (Cmd) {
        case COMP_CommandStateSet:
            eCmd = SetState;
            break;

        case COMP_CommandFlush:
            eCmd = Flush;
            break;

        case COMP_CommandPortDisable:
            eCmd = StopPort;
            break;

        case COMP_CommandPortEnable:
            eCmd = RestartPort;
            break;

        case COMP_CommandMarkBuffer:
            eCmd = MarkBuf;
            if (nParam1 > 0) {
                eError = ERR_AI_ILLEGAL_PARAM;
                goto COMP_CONF_CMD_BAIL;
            }
            break;

        default:
            eCmd = -1;
            break;
    }

    msg.command = eCmd;
    msg.para0 = nParam1;
    put_message(&pChnData->mCmdQueue, &msg);

COMP_CONF_CMD_BAIL:
    return eError;
}

static ERRORTYPE AIChannel_GetState(PARAM_IN COMP_HANDLETYPE hComponent, PARAM_OUT COMP_STATETYPE* pState)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S*)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    ERRORTYPE eError = SUCCESS;

    if (NULL == pChnData || NULL == pState) {
        eError = ERR_AI_ILLEGAL_PARAM;
        goto COMP_CONF_CMD_BAIL;
    }

    *pState = pChnData->state;

COMP_CONF_CMD_BAIL:
    return eError;
}

static ERRORTYPE AIChannel_SetCallbacks(PARAM_IN COMP_HANDLETYPE hComponent,
    PARAM_IN COMP_CALLBACKTYPE* pCallbacks, PARAM_IN void* pAppData)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S*)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    ERRORTYPE eError = SUCCESS;

    if (NULL == pChnData || NULL == pCallbacks || NULL == pAppData) {
        aloge("pChnData=%p, pCallbacks=%p, pAppData=%p", pChnData, pCallbacks, pAppData);
        eError = ERR_AI_ILLEGAL_PARAM;
        goto COMP_CONF_CMD_BAIL;
    }

    pChnData->pCallbacks = pCallbacks;
    pChnData->pAppData = pAppData;

COMP_CONF_CMD_BAIL:
    return eError;
}

static ERRORTYPE AIChannel_SetConfig(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_IN COMP_INDEXTYPE nIndex,
        PARAM_IN void* pComponentConfigStructure)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S*)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    ERRORTYPE eError = SUCCESS;

    MM_COMPONENTTYPE *pAOTunnelComp = NULL;
    COMP_INTERNAL_TUNNELINFOTYPE* pAOTunnel = NULL;

    switch (nIndex)
    {
        case COMP_IndexParamPortDefinition:
        {
            COMP_PARAM_PORTDEFINITIONTYPE *port = (COMP_PARAM_PORTDEFINITIONTYPE*)pComponentConfigStructure;
            int i;
            for(i = 0; i < AI_CHN_MAX_PORTS; i++) {
                if (port->nPortIndex == pChnData->sPortDef[i].nPortIndex) {
                    memcpy(&pChnData->sPortDef[i], port, sizeof(COMP_PARAM_PORTDEFINITIONTYPE));
                }
            }
            if (i == AI_CHN_MAX_PORTS) {
                eError = FAILURE;
            }
            break;
        }
        case COMP_IndexParamCompBufferSupplier:
        {
            COMP_PARAM_BUFFERSUPPLIERTYPE *pPortBufSupplier = (COMP_PARAM_BUFFERSUPPLIERTYPE*)pComponentConfigStructure;
            int i;
            for(i=0; i<AI_CHN_MAX_PORTS; i++) {
                if(pChnData->sPortBufSupplier[i].nPortIndex == pPortBufSupplier->nPortIndex) {
                    memcpy(&pChnData->sPortBufSupplier[i], pPortBufSupplier, sizeof(COMP_PARAM_BUFFERSUPPLIERTYPE));
                    break;
                }
            }
            if(i == AI_CHN_MAX_PORTS) {
                eError = FAILURE;
            }
            break;
        }
        case COMP_IndexVendorMPPChannelInfo:
        {
            pChnData->mMppChnInfo = *(MPP_CHN_S*)pComponentConfigStructure;
            break;
        }
        case COMP_IndexVendorAIChnReleaseFrame:
        {
            AudioFrame *pAudioFrame = (AudioFrame*)pComponentConfigStructure;
            eError = AIChannel_ReleaseFrame(hComponent, pAudioFrame->pFrame);
            break;
        }
        case COMP_IndexVendorAIChnParameter:
        {
            pChnData->mParam = *(AI_CHN_PARAM_S*)pComponentConfigStructure;
            break;
        }
        case COMP_IndexVendorAIOVqeAttr:
        {//
            memcpy(&pChnData->mVqeCfg, (AI_VQE_CONFIG_S*)pComponentConfigStructure, sizeof(AI_VQE_CONFIG_S));
            break;
        }
        case COMP_IndexVendorAIOVqeEnable:
        {//
            pChnData->mUseVqeLib = TRUE;
            break;
        }
        case COMP_IndexVendorAIOVqeDisable:
        {
            pChnData->mUseVqeLib = FALSE;
            break;
        }
        case COMP_IndexVendorAIOReSmpEnable:
        {
            // todo
            break;
        }
        case COMP_IndexVendorAIOReSmpDisable:
        {
            // todo
            break;
        }
        case COMP_IndexVendorAISetSaveFileInfo:
        {
            AUDIO_SAVE_FILE_INFO_S *pSaveFileInfo = (AUDIO_SAVE_FILE_INFO_S*)pComponentConfigStructure;
            eError = AIChannel_SetSaveFileInfo(hComponent, pSaveFileInfo);
            break;
        }
        case COMP_IndexVendorAIChnMute:
        {
            eError = AIChannel_SetChnMute(hComponent, *(BOOL*)pComponentConfigStructure);
            break;
        }
        case COMP_IndexVendorAIIgnoreData:
        {
            eError = AIChannel_IgnoreData(hComponent, *(BOOL*)pComponentConfigStructure);
            break;
        }
        default:
            eError = FAILURE;
            break;
    }

    return eError;
}

static ERRORTYPE AIChannel_GetConfig(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_IN COMP_INDEXTYPE nIndex,
        PARAM_INOUT void* pComponentConfigStructure)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S*)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    ERRORTYPE eError = SUCCESS;

    switch (nIndex)
    {
        case COMP_IndexParamPortDefinition:
        {
            COMP_PARAM_PORTDEFINITIONTYPE *port = (COMP_PARAM_PORTDEFINITIONTYPE*)pComponentConfigStructure;
            int i;
            for(i = 0; i < AI_CHN_MAX_PORTS; i++) {
                if (port->nPortIndex == pChnData->sPortDef[i].nPortIndex) {
                    memcpy(port, &pChnData->sPortDef[i], sizeof(COMP_PARAM_PORTDEFINITIONTYPE));
                }
            }
            if (i == AI_CHN_MAX_PORTS) {
                eError = FAILURE;
            }
            break;
        }
        case COMP_IndexParamCompBufferSupplier:
        {
            COMP_PARAM_BUFFERSUPPLIERTYPE *pPortBufSupplier = (COMP_PARAM_BUFFERSUPPLIERTYPE*)pComponentConfigStructure;
            int i;
            for(i=0; i<AI_CHN_MAX_PORTS; i++) {
                if(pChnData->sPortBufSupplier[i].nPortIndex == pPortBufSupplier->nPortIndex) {
                    memcpy(pPortBufSupplier, &pChnData->sPortBufSupplier[i], sizeof(COMP_PARAM_BUFFERSUPPLIERTYPE));
                    break;
                }
            }
            if(i == AI_CHN_MAX_PORTS) {
                eError = FAILURE;
            }
            break;
        }
        case COMP_IndexVendorMPPChannelInfo:
        {
            *(MPP_CHN_S*)pComponentConfigStructure = pChnData->mMppChnInfo;
            break;
        }
        case COMP_IndexVendorAIChnGetValidFrame:
        {
            AudioFrame *pAudioFrame = (AudioFrame *)pComponentConfigStructure;
            eError = AIChannel_GetFrame(hComponent, pAudioFrame->pFrame, pAudioFrame->nMilliSec);
            break;
        }        
        case COMP_IndexVendorAIChnGetValidFrameRef:
        {
            AUDIO_FRAME_S *pAudioFrame = (AUDIO_FRAME_S *)pComponentConfigStructure;
            eError = AIChannel_GetFrameRef(hComponent, pAudioFrame);
            break;
        }
        case COMP_IndexVendorAIChnParameter:
        {
            *(AI_CHN_PARAM_S*)pComponentConfigStructure = pChnData->mParam;
            break;
        }
        case COMP_IndexVendorAIOVqeAttr:
        {
            *(AI_VQE_CONFIG_S*)pComponentConfigStructure = pChnData->mVqeCfg;
            break;
        }
        case COMP_IndexVendorAIChnGetFreeFrame:
        {
//            if (pChnData->state != COMP_StateExecuting) {
//                eError = FAILURE;
//                break;
//            }

//            AUDIO_FRAME_S *tmp = pChnData->mpCapMgr->getFreeFrame(pChnData->mpCapMgr);
//            if (tmp != NULL) {
//                *(AUDIO_FRAME_S *)pComponentConfigStructure = *tmp;
//                eError = SUCCESS;
//            } else {
//                eError = FAILURE;
//            }
            break;
        }
        case COMP_IndexVendorAIQueryFileStatus:
        {
            AUDIO_SAVE_FILE_INFO_S *pSaveFileInfo = (AUDIO_SAVE_FILE_INFO_S*)pComponentConfigStructure;
            eError = AIChannel_QueryFileStatus(hComponent, pSaveFileInfo);
            break;
        }
        case COMP_IndexVendorAIChnMute:
        {
            eError = AIChannel_GetChnMute(hComponent, (BOOL*)pComponentConfigStructure);
            break;
        }
        case COMP_IndexVendorAIChnInportTunneled:   // check if ao is tunneled to ai
        { 
            eError = AIChannel_GetChnTunneledFlag(hComponent, (BOOL*)pComponentConfigStructure);
            break;
        }
        default:
            eError = FAILURE;
            break;
    }

    return eError;
}

static ERRORTYPE AIChannel_ComponentTunnelRequest(
    PARAM_IN COMP_HANDLETYPE hComponent,
    PARAM_IN unsigned int nPort,
    PARAM_IN COMP_HANDLETYPE hTunneledComp,
    PARAM_IN unsigned int nTunneledPort,
    PARAM_INOUT COMP_TUNNELSETUPTYPE* pTunnelSetup)
{
    ERRORTYPE eError = SUCCESS;
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S*)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    COMP_PARAM_PORTDEFINITIONTYPE    *pPortDef;
    COMP_INTERNAL_TUNNELINFOTYPE     *pPortTunnelInfo;
    COMP_PARAM_BUFFERSUPPLIERTYPE    *pPortBufSupplier;
    int i;

    if (pChnData->state == COMP_StateExecuting)
    {
        alogw("Be careful! tunnel request may be some danger in StateExecuting");
    }
    else if(pChnData->state != COMP_StateIdle)
    {
        aloge("fatal error! tunnel request can't be in state[0x%x]", pChnData->state);
        eError = ERR_AI_INCORRECT_STATE_OPERATION;
        goto COMP_CMD_FAIL;
    }

    for (i = 0; i < AI_CHN_MAX_PORTS; ++i) {
        if (pChnData->sPortDef[i].nPortIndex == nPort) {
            pPortDef = &pChnData->sPortDef[i];
            break;
        }
    }
    if (i == AI_CHN_MAX_PORTS) {
        aloge("fatal error! portIndex[%d] wrong!", nPort);
        eError = ERR_AI_ILLEGAL_PARAM;
        goto COMP_CMD_FAIL;
    }

    for (i = 0; i < AI_CHN_MAX_PORTS; ++i) {
        if (pChnData->sPortTunnelInfo[i].nPortIndex == nPort) {
            pPortTunnelInfo = &pChnData->sPortTunnelInfo[i];
            break;
        }
    }
    if (i == AI_CHN_MAX_PORTS) {
        aloge("fatal error! portIndex[%d] wrong!", nPort);
        eError = ERR_AI_ILLEGAL_PARAM;
        goto COMP_CMD_FAIL;
    }

    for (i = 0; i < AI_CHN_MAX_PORTS; ++i) {
        if (pChnData->sPortBufSupplier[i].nPortIndex == nPort) {
            pPortBufSupplier = &pChnData->sPortBufSupplier[i];
            break;
        }
    }
    if (i == AI_CHN_MAX_PORTS) {
        aloge("fatal error! portIndex[%d] wrong!", nPort);
        eError = ERR_AI_ILLEGAL_PARAM;
        goto COMP_CMD_FAIL;
    }

    pPortTunnelInfo->nPortIndex = nPort;
    pPortTunnelInfo->hTunnel = hTunneledComp;
    pPortTunnelInfo->nTunnelPortIndex = nTunneledPort;
    pPortTunnelInfo->eTunnelType = (pPortDef->eDomain == COMP_PortDomainOther) ? TUNNEL_TYPE_CLOCK : TUNNEL_TYPE_COMMON;
    if(NULL==hTunneledComp && 0==nTunneledPort && NULL==pTunnelSetup) {
        alogd("Cancel setup tunnel on port[%d]", nPort);
        eError = SUCCESS;
        goto COMP_CMD_FAIL;
    }
    if(pPortDef->eDir == COMP_DirOutput) {
        if (pChnData->mOutputPortTunnelFlag[AI_OUTPORT_SUFFIX_AENC] || pChnData->mOutputPortTunnelFlag[AI_OUTPORT_SUFFIX_AO]) {
            aloge("AI_Comp outport already bind, why bind again?!");
            eError = FAILURE;
            goto COMP_CMD_FAIL;
        }
        pTunnelSetup->nTunnelFlags = 0;
        pTunnelSetup->eSupplier = pPortBufSupplier->eBufferSupplier;
        // judge which B: aenc or ao?
        COMP_PARAM_PORTDEFINITIONTYPE out_port_def;
        out_port_def.nPortIndex = nTunneledPort;
        if (pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AENC].nPortIndex == nPort)
            pChnData->mOutputPortTunnelFlag[AI_OUTPORT_SUFFIX_AENC] = TRUE;
        else if (pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AO].nPortIndex == nPort)
            pChnData->mOutputPortTunnelFlag[AI_OUTPORT_SUFFIX_AO] = TRUE;
        else
            aloge("fatal error! ao bind with portIndex(%d, %d)", nPort, nTunneledPort);

    } else {
        //Check the data compatibility between the ports using one or more GetParameter calls.
        //B checks if its input port is compatible with the output port of component A.
        COMP_PARAM_PORTDEFINITIONTYPE out_port_def;
        out_port_def.nPortIndex = nTunneledPort;
        ((MM_COMPONENTTYPE*)hTunneledComp)->GetConfig(hTunneledComp, COMP_IndexParamPortDefinition, &out_port_def);
        if(out_port_def.eDir != COMP_DirOutput) {
            aloge("fatal error! tunnel port index[%d] direction is not output!", nTunneledPort);
            eError = ERR_AI_ILLEGAL_PARAM;
            goto COMP_CMD_FAIL;
        }
        pPortDef->format = out_port_def.format;

        //The component B informs component A about the final result of negotiation.
        if(pTunnelSetup->eSupplier != pPortBufSupplier->eBufferSupplier) {
            alogw("Low probability! use input portIndex[%d] buffer supplier[%d] as final!", nPort, pPortBufSupplier->eBufferSupplier);
            pTunnelSetup->eSupplier = pPortBufSupplier->eBufferSupplier;
        }
        COMP_PARAM_BUFFERSUPPLIERTYPE oSupplier;
        oSupplier.nPortIndex = nTunneledPort;
        ((MM_COMPONENTTYPE*)hTunneledComp)->GetConfig(hTunneledComp, COMP_IndexParamCompBufferSupplier, &oSupplier);
        oSupplier.eBufferSupplier = pTunnelSetup->eSupplier;
        ((MM_COMPONENTTYPE*)hTunneledComp)->SetConfig(hTunneledComp, COMP_IndexParamCompBufferSupplier, &oSupplier);

        
        if (pChnData->sPortDef[AI_CHN_PORT_INDEX_CAP_IN].nPortIndex == nPort)
        {
            pChnData->mInputPortTunnelFlag[AI_CHN_PORT_INDEX_CAP_IN] = TRUE;
        }
        else if (pChnData->sPortDef[AI_CHN_PORT_INDEX_AO_IN].nPortIndex == nPort)
        {
            pChnData->mInputPortTunnelFlag[AI_CHN_PORT_INDEX_AO_IN] = TRUE;
        }
    }

COMP_CMD_FAIL:
    return eError;
}

static ERRORTYPE AIChannel_EmptyThisBuffer(PARAM_IN COMP_HANDLETYPE hComponent, PARAM_IN COMP_BUFFERHEADERTYPE* pBuffer)
{
    ERRORTYPE eError = SUCCESS;
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S*)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    //pthread_mutex_lock(&pChnData->mStateLock);
    if (pChnData->state != COMP_StateExecuting)
    {
        //aloge("send frame when AI channel state[0x%x] is not executing", pChnData->state);
        eError = ERR_AI_SYS_NOTREADY;
        goto EIXT;
    }

    if (pBuffer->nOutputPortIndex == pChnData->sPortDef[AI_CHN_PORT_INDEX_CAP_IN].nPortIndex) {
        AUDIO_FRAME_S *pSrcFrm = pBuffer->pOutputPortPrivate;
        AUDIO_FRAME_S *pDstFrm = pChnData->mpCapMgr->getFreeFrame(pChnData->mpCapMgr);
        if (NULL != pDstFrm)
        {
            pDstFrm->mLen       = pSrcFrm->mLen;
            pDstFrm->mBitwidth  = pSrcFrm->mBitwidth;
            pDstFrm->mSoundmode = pSrcFrm->mSoundmode;
            pDstFrm->tmp_pts = pSrcFrm->tmp_pts;
            if(!pChnData->mbMute)
            {
                memcpy(pDstFrm->mpAddr, pSrcFrm->mpAddr, pSrcFrm->mLen);
            }
            else
            {
                memset(pDstFrm->mpAddr, 0, pSrcFrm->mLen);
            }
        }
        else
        {
            //aloge("no node in FreeFrameList!");
            //PcmBufferManager *pBufMgr = pChnData->mpCapMgr;
            //aloge("PcmBufMgrFrmCntInfo >> FillingList:%d, ValidList:%d, UsingList:%d", pBufMgr->fillingFrmCnt(pBufMgr),
            //    pBufMgr->validFrmCnt(pBufMgr), pBufMgr->usingFrmCnt(pBufMgr));
            AUDIO_FRAME_S *pSrcFrm = pBuffer->pOutputPortPrivate;
            pChnData->mDiscardLen += pSrcFrm->mLen;
            pChnData->mDiscardNum++;
            eError = ERR_AI_BUF_FULL;
            goto EIXT;
        }
        pthread_mutex_lock(&pChnData->mCapMgrLock);
        pChnData->mpCapMgr->pushFrame(pChnData->mpCapMgr, pDstFrm);
        if (pChnData->mWaitingCapDataFlag) {
            pChnData->mWaitingCapDataFlag = FALSE;
            message_t msg;
            msg.command = AIChannel_CapDataAvailable;
            put_message(&pChnData->mCmdQueue, &msg);
        }
        pthread_mutex_unlock(&pChnData->mCapMgrLock);
        cdx_sem_up_unique(&pChnData->mWaitOutFrameSem);
    } 
    else if (pBuffer->nOutputPortIndex == pChnData->sPortDef[AI_CHN_PORT_INDEX_AO_IN].nPortIndex) 
    {
        AUDIO_FRAME_S *pPlayFrm = (AUDIO_FRAME_S *)pBuffer->pOutputPortPrivate;
        AUDIO_FRAME_S *frame = pChnData->mpPlayMgr->getFreeFrame(pChnData->mpPlayMgr);
        if ((frame != NULL) && pPlayFrm->mLen)
        {
            int size = (pPlayFrm->mLen > pChnData->mpPcmCfg->chunkBytes) ? pChnData->mpPcmCfg->chunkBytes : pPlayFrm->mLen;
            //memset(frame->mpAddr, 0, pChnData->mpPcmCfg->chunkBytes);
            memcpy(frame->mpAddr, pPlayFrm->mpAddr, size);

            frame->mBitwidth = pPlayFrm->mBitwidth;
            frame->mSoundmode = pPlayFrm->mSoundmode;
            frame->mLen = size;

            pChnData->mpPlayMgr->pushFrame(pChnData->mpPlayMgr, frame);
            cdx_sem_up_unique(&pChnData->mWaitOutFrameSem);
        }
    } else {
        aloge("fatal error! inputPortIndex[%d] match nothing!", pBuffer->nOutputPortIndex);
    }

EIXT:
    //pthread_mutex_unlock(&pChnData->mStateLock);
    return eError;
}

/**
 * release frame, used in tunnel mode.
 * usualy use it when ao return frame to ai
 *
 * @return SUCCESS.
 * @param hComponent ai component.
 * @param pAudioFrame frame info.
 */
static ERRORTYPE AIChannel_FillThisBuffer(PARAM_IN COMP_HANDLETYPE hComponent, PARAM_IN COMP_BUFFERHEADERTYPE* pBuffer)
{
    ERRORTYPE eError = SUCCESS;
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S*)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    if (pBuffer->nOutputPortIndex == pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AENC].nPortIndex)
    {
        AUDIO_FRAME_S *pFrm = (AUDIO_FRAME_S*)pBuffer->pOutputPortPrivate;
        alogw("AEnc not need return frame to AI!");
        pChnData->mpCapMgr->releaseFrame(pChnData->mpCapMgr, pFrm);
    }
    else if (pBuffer->nOutputPortIndex == pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AO].nPortIndex)
    {
        AUDIO_FRAME_S *pFrm = (AUDIO_FRAME_S*)pBuffer->pOutputPortPrivate;
        pChnData->mpCapMgr->releaseFrame(pChnData->mpCapMgr, pFrm);
    }
    else
    {
        aloge("fatal error! return frame with portIndex(%d, %d)", pBuffer->nOutputPortIndex, pBuffer->nInputPortIndex);
    }
    return eError;
}

static ERRORTYPE AIChannel_ComponentDeInit(PARAM_IN COMP_HANDLETYPE hComponent)
{
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S*)(((MM_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    ERRORTYPE eError = SUCCESS;
    CompInternalMsgType eCmd = Stop;
    message_t msg;
    int ret = 0;

    alogd("aiDev[%d]Chn[%d] discard audio block num:%d, discard audio data len:%d", 
        pChnData->mMppChnInfo.mDevId, pChnData->mMppChnInfo.mChnId, pChnData->mDiscardNum, pChnData->mDiscardLen);
    msg.command = eCmd;
    put_message(&pChnData->mCmdQueue, &msg);
    alogv("wait AI channel component exit!...");

    // Wait for thread to exit so we can get the status into "error"
    pthread_join(pChnData->mThreadId, (void*) &eError);
    message_destroy(&pChnData->mCmdQueue);

    cdx_sem_deinit(&pChnData->mAllFrameRelSem);
    cdx_sem_deinit(&pChnData->mWaitOutFrameSem);
    //cdx_sem_deinit(&pChnData->mWaitGetAllOutFrameSem);

    pthread_mutex_destroy(&pChnData->mStateLock);
    pthread_mutex_destroy(&pChnData->mCapMgrLock);
    pthread_mutex_destroy(&pChnData->mIgnoreDataLock);

    if (pChnData->mpCapMgr != NULL) {
        pcmBufMgrDestroy(pChnData->mpCapMgr);
        pChnData->mpCapMgr = NULL;
    }
    if (pChnData->mpPlayMgr != NULL) {
        pcmBufMgrDestroy(pChnData->mpPlayMgr);
        pChnData->mpPlayMgr = NULL;
    }

#ifdef AI_SAVE_AUDIO_PCM
    fclose(pChnData->pcm_fp);
    alogd("AI_Comp pcm_file size: %d", pChnData->pcm_sz);

    fclose(pChnData->tmp_pcm_fp_in);
    fclose(pChnData->tmp_pcm_fp_ref);
    fclose(pChnData->tmp_pcm_fp_out);
#endif
    if (pChnData->mSaveFileFlag)
    {
        fclose(pChnData->mFpSaveFile);
        free(pChnData->mpSaveFileFullPath);
    }

#ifdef CFG_AUDIO_EFFECT_AEC
    //store_aec_coef(pChnData->ec);
    if (pChnData->ec != NULL)
    {
        ec_destroy(pChnData->ec);
        pChnData->ec = NULL;
    }
#endif

#if AEC_IMPLEMENT_IN_AI_CHL
    if(NULL != pChnData->aecmInst)
    {
        ret = WebRtcAec_Free(pChnData->aecmInst);
        if(0 == ret)
        {
            aloge("aec_released");  
            pChnData->aecmInst = NULL;
        }
        else
        {
            aloge("aec_free_failed");
        }
    }

    if (pChnData->tmpBuf != NULL)
    {
       free(pChnData->tmpBuf);
       pChnData->tmpBuf = NULL;
    }

    if(NULL != pChnData->near_buff)
    {
        free(pChnData->near_buff);
        pChnData->near_buff = NULL;
    }

    if(NULL != pChnData->ref_buff)
    {
        free(pChnData->ref_buff);
        pChnData->ref_buff = NULL;
    }

    if(NULL != pChnData->out_buff)
    {
        free(pChnData->out_buff);
        pChnData->out_buff = NULL;
    } 
#endif

#ifdef CFG_AUDIO_EFFECT_RNR
    if (pChnData->nosc != NULL)
    {
        NOSCdestroy(pChnData->nosc);
        pChnData->nosc = NULL;
    }
#endif

#ifdef CFG_AUDIO_EFFECT_DRC
    if (pChnData->drc != NULL)
    {
        drcLog_destroy(pChnData->drc);
        pChnData->drc = NULL;
    }
#endif

    free(pChnData);
    alogd("Ai component exited!");
    return eError;
}


#ifdef CFG_AUDIO_EFFECT_AEC
//just 8k/16k 16bits mono
static void initAecParas(AI_CHN_DATA_S *pChnData)
{
    static eq_core_prms_t eq_core_prms[1] =
    {
        //{-6, 3000, 4, BANDPASS_PEAK},
        {-10, 6300, 4, BANDPASS_PEAK},
    };

    unsigned int samplerate = pChnData->mpPcmCfg->sampleRate;

    memset(&pChnData->ec_prms, 0, sizeof(pChnData->ec_prms));
    /* create echo controller handle */

/* common init prms */
    pChnData->ec_prms.enable_cdc = 0;
    pChnData->ec_prms.enable_aec = 1;
    pChnData->ec_prms.aec_prms.sampling_rate = samplerate;
    pChnData->ec_prms.aec_prms.frame_size = FRAMESIZ2TIME(FRMSIZE(samplerate), samplerate); //8ms or 16 ms
    pChnData->ec_prms.aec_prms.tail_length = 128; //128ms
    pChnData->ec_prms.aec_prms.enable_nlp = 1;
    pChnData->ec_prms.aec_prms.nlp_prms.nlp_overlap_percent = OVERLAP_SEVENTY_FIVE;
/* tx drc init prms */
    pChnData->ec_prms.enable_txdrc = 1;
    pChnData->ec_prms.txdrc_prms.sampling_rate = samplerate;
    pChnData->ec_prms.txdrc_prms.attack_time = 1;
    pChnData->ec_prms.txdrc_prms.max_gain = 6;
    pChnData->ec_prms.txdrc_prms.min_gain = -9;
    pChnData->ec_prms.txdrc_prms.noise_threshold = -45;
    pChnData->ec_prms.txdrc_prms.release_time = 100;
    pChnData->ec_prms.txdrc_prms.target_level = -9;
/* tx eq init prms */
    pChnData->ec_prms.enable_txeq = 1;
    pChnData->ec_prms.txeq_prms.biq_num = 1;
    pChnData->ec_prms.txeq_prms.sampling_rate = samplerate;
    pChnData->ec_prms.txeq_prms.core_prms = eq_core_prms;
/* bulk delay estimation prms */
    pChnData->ec_prms.enable_bdc = 1;
    pChnData->ec_prms.bdc_prms.block_size = FRMSIZE(samplerate);
    pChnData->ec_prms.bdc_prms.delay_margin = 7;
    pChnData->ec_prms.bdc_prms.max_delay = 375;
    pChnData->ec_prms.bdc_prms.near_delay = 12;
    pChnData->ec_prms.bdc_prms.sampling_rate = samplerate;
/* noise suppression prms */
    pChnData->ec_prms.enable_ns = 1;
    pChnData->ec_prms.ns_prms.sampling_rate = samplerate;
    pChnData->ec_prms.ns_prms.max_suppression = -21;
    pChnData->ec_prms.ns_prms.nonstat = LOW_NONSTATIONAL;
    pChnData->ec_prms.ns_prms.overlap_percent = OVERLAP_SEVENTY_FIVE;
/* fade in processor enable in transimtter */
    pChnData->ec_prms.enable_txfade = 1;
}

//for audio call(voice) only, which means samplerate must be 8k or 16k with mono.
static ERRORTYPE audioAecProcess(AI_CHN_DATA_S *pChnData, AUDIO_FRAME_S *pFrm, AEC_FRAME_S *pRefFrm)
{
    short far_buffer[FRMSIZE(pChnData->mpPcmCfg->sampleRate)];
    short near_buffer[FRMSIZE(pChnData->mpPcmCfg->sampleRate)];
    short out_buffer[FRMSIZE(pChnData->mpPcmCfg->sampleRate)];
    short out_receiver[FRMSIZE(pChnData->mpPcmCfg->sampleRate)];    // reserved
    int frm_size = FRMSIZE(pChnData->mpPcmCfg->sampleRate);     //64 or 128(cap frames in 8ms)

    unsigned int sampleRate = pChnData->mpPcmCfg->sampleRate;
    int trackCnt = pChnData->mpPcmCfg->chnCnt;

    if (((sampleRate!=8000) && (sampleRate!=16000)) || (trackCnt!=1))
    {
        aloge("error rec sampleRate(%u) or trackCnt(%d)", sampleRate, trackCnt);
        return FAILURE;
    }

    if (!pRefFrm->bValid)
    {
        aloge("error frame invalid");
        return FAILURE;
    }

    if (pChnData->ec == NULL)
    {
        alogd("create ec");
#if 0
        memcpy(&pChnData->ec_prms, &pChnData->mVqeCfg.stAecCfg.prms, sizeof(ec_prms_t));
        pChnData->ec_prms.aec_prms.sampling_rate = sampleRate;
        pChnData->ec_prms.aec_prms.frame_size = FRAMESIZ2TIME(FRMSIZE(sampleRate), sampleRate); //8ms
        pChnData->ec_prms.txdrc_prms.sampling_rate = sampleRate;
        pChnData->ec_prms.txeq_prms.sampling_rate = sampleRate;
        pChnData->ec_prms.bdc_prms.sampling_rate = sampleRate;
        pChnData->ec_prms.bdc_prms.block_size = FRMSIZE(sampleRate);
        pChnData->ec_prms.ns_prms.sampling_rate = sampleRate;
#else
        initAecParas(pChnData);
#endif

        pChnData->ec = ec_create(&pChnData->ec_prms);
        if (pChnData->ec == NULL)
        {
            aloge("aec_create fail!");
            return FAILURE;
        }
    }

    memset(pChnData->tmpBuf, 0, pChnData->tmpBufLen);

    short *ptr = (short *)pFrm->mpAddr;
    short *ptrRef = (short *)pRefFrm->stRefFrame.mpAddr;
    short *ptrOut = (short *)pChnData->tmpBuf;
    int size = (pFrm->mLen < pRefFrm->stRefFrame.mLen) ? pFrm->mLen : pRefFrm->stRefFrame.mLen;
    int left = size / sizeof(short);

    while (left >= frm_size)
    {
        memcpy((char *)near_buffer, (char *)ptr, frm_size*sizeof(short));
        memcpy((char *)far_buffer,  (char *)ptrRef, frm_size*sizeof(short));

        ec_process_frame(pChnData->ec, near_buffer, far_buffer, out_buffer, out_receiver);

        memcpy((char *)ptrOut, (char *)out_buffer, frm_size*sizeof(short));

        left -= frm_size;
        ptr += frm_size;
        ptrRef += frm_size;
        ptrOut += frm_size;
    }

    if (left > 0)
    {
        memset(near_buffer, 0, sizeof(near_buffer));
        memset(far_buffer, 0, sizeof(far_buffer));

        memcpy((char *)near_buffer, (char *)ptr, left*sizeof(short));
        memcpy((char *)far_buffer,  (char *)ptrRef, left*sizeof(short));

        ec_process_frame(pChnData->ec, near_buffer, far_buffer, out_buffer, out_receiver);

        memcpy((char *)ptrOut, (char *)out_buffer, left*sizeof(short));
    }

    memcpy(pFrm->mpAddr, pChnData->tmpBuf, size);

    return SUCCESS;
}
#endif //aec

#ifdef CFG_AUDIO_EFFECT_RNR
//for record noise reduce, only in record mode
static ERRORTYPE audioRnrProcess(AI_CHN_DATA_S *pChnData, AUDIO_FRAME_S *pFrm)
{
    if (pChnData->nosc == NULL)
    {
        aw_ns_prms_t prms;
        prms.sampling_rate = pChnData->mpPcmCfg->sampleRate;
        prms.channum = (pFrm->mSoundmode == AUDIO_SOUND_MODE_MONO) ? 1 : 2;
        prms.max_suppression = pChnData->mVqeCfg.stRnrCfg.sMaxNoiseSuppression;
        prms.overlap_percent = pChnData->mVqeCfg.stRnrCfg.sOverlapPercent;
        prms.nonstat = pChnData->mVqeCfg.stRnrCfg.sNonstat;

        pChnData->nosc = NOSCinit(&prms);
        if (pChnData->nosc == NULL)
        {
            aloge("rnr init fail!");
            return FAILURE;
        }
    }

    NOSCdec(pChnData->nosc, pFrm->mpAddr, pFrm->mLen/sizeof(short));

    return SUCCESS;
}
#endif


#ifdef CFG_AUDIO_EFFECT_DRC
//for audio call(voice)(8k/16k) only in record mode.
static ERRORTYPE audioDrcProcess(AI_CHN_DATA_S *pChnData, AUDIO_FRAME_S *pFrm)
{
    int fr = 0;
    aw_drcLog_prms_t drc_prms;
    int chnNum = (pFrm->mSoundmode==AUDIO_SOUND_MODE_MONO) ? 1 : 2;

    if (pChnData->drc == NULL)
    {
        drc_prms.sampling_rate = pChnData->mpPcmCfg->sampleRate;
        drc_prms.attack_time = pChnData->mVqeCfg.stDrcCfg.sAttackTime;
        drc_prms.release_time = pChnData->mVqeCfg.stDrcCfg.sReleaseTime;
        drc_prms.max_gain = pChnData->mVqeCfg.stDrcCfg.sMaxGain;
        drc_prms.min_gain = pChnData->mVqeCfg.stDrcCfg.sMinGain;
        drc_prms.noise_threshold = pChnData->mVqeCfg.stDrcCfg.sNoiseThreshold;
        drc_prms.target_level = pChnData->mVqeCfg.stDrcCfg.sTargetLevel;

        pChnData->drc = drcLog_create(&drc_prms);

        if (pChnData->drc == NULL)
        {
            aloge("drc create fail!");
            return FAILURE;
        }
    }

    drcLog_process(pChnData->drc, pFrm->mpAddr, pFrm->mLen/sizeof(short), chnNum);

    return SUCCESS;
}
#endif

#if CFG_AUDIO_EFFECT_EQ
static int audioEQHandle(AI_CHN_DATA_S *pChnData, AUDIO_FRAME_S *pInFrm)
{
    short *proc_ptr;
    int sample_rate = pChnData->mpAioAttr->enSamplerate;

    if (AUDIO_BIT_WIDTH_16 != pInFrm->mBitwidth)
    {
        aloge("audio pcm format error! bitwidth=%d", pInFrm->mBitwidth);
        return FAILURE;
    }

    eq_prms_t prms[4] =
    {
        {4, 600, 1, BANDPASS_PEAK, sample_rate},
        {4, 1000, 1, BANDPASS_PEAK, sample_rate},
        {4, 2000, 2, BANDPASS_PEAK, sample_rate},
        {4, 4000, 1, BANDPASS_PEAK, sample_rate},
    };

    if (pChnData->equalizer == NULL)
    {
        pChnData->equalizer = eq_create(&prms[0], sizeof(prms)/sizeof(prms[0]));
        if (pChnData->equalizer == NULL)
        {
            aloge("eq create fail!");
            return FAILURE;
        }
    }

    proc_ptr = (short*)pInFrm->mpAddr;
    int left_item_cnt = sizeof(pInFrm->mLen);
    while (left_item_cnt>0)
    {
        int proc_sz = (left_item_cnt>64)? 64:left_item_cnt;
        eq_process(pChnData->equalizer, proc_ptr, proc_sz);
        proc_ptr += proc_sz;
        left_item_cnt -= proc_sz;
    }

    return SUCCESS;
}
#endif
#if AEC_IMPLEMENT_IN_AI_CHL
static ERRORTYPE audio_aec_process_new(AI_CHN_DATA_S *pChnData, AUDIO_FRAME_S *pFrm, AEC_FRAME_S *pRefFrm)
{
    int ret = 0;

    if (!pRefFrm->bValid)       // got refrence frame or not
    {
        aloge("aec_error ref_frame invalid");
        return FAILURE;
    }

    if(1 != pChnData->mpPcmCfg->chnCnt)
    {
        aloge("aec_invalid_chl_number:%d",pChnData->mpPcmCfg->chnCnt);
        return FAILURE;
    }

//    if(8000 < pChnData->mpPcmCfg->sampleRate)
//    {
//        alogw("aec_sample_rate>8000:%d",pChnData->mpPcmCfg->sampleRate);
//    }

    if(NULL == pChnData->aecmInst)
    { 
        aloge("aec_to_init:%d-%d",pChnData->mpPcmCfg->sampleRate,pChnData->mpPcmCfg->aec_delay_ms);
       ret = WebRtcAec_Create(&pChnData->aecmInst);
        if(NULL == pChnData->aecmInst || 0 != ret)
        {
            aloge("aec_instance_create_fail:%d",ret);
            return FAILURE;
        }

        ret = WebRtcAec_Init(pChnData->aecmInst, pChnData->mpPcmCfg->sampleRate, pChnData->mpPcmCfg->sampleRate);
        if(0 != ret)
        {
            aloge("aec_init_failed:%d",ret);
        }
        
        AecConfig config;
        
        memset(&config,0,sizeof(AecConfig));
        config.nlpMode = kAecNlpConservative;   
        ret = WebRtcAec_set_config(pChnData->aecmInst, config);
        if(0 != ret)
        {
            aloge("aec_cfg_failed:%d",ret);
        }
    }
    
    memset(pChnData->tmpBuf, 0, pChnData->tmpBufLen); 

    // move data in near buffer and reference buffer to internal buffer for conjunction with remaining data for last process. 
    if(pChnData->near_buff_data_remain_len+pFrm->mLen <= pChnData->mpPcmCfg->chunkBytes*2)
    {
        memcpy((char*)pChnData->near_buff+pChnData->near_buff_data_remain_len,(char*)pFrm->mpAddr,pFrm->mLen);
        pChnData->near_buff_data_remain_len += pFrm->mLen;
            
    }
    else
    {
        aloge("fatal_err_near_buff_over_flow:%d-%d-%d-%d",pChnData->near_buff_data_remain_len,pChnData->near_buff_len,
                                                            pFrm->mLen,pChnData->mpPcmCfg->chunkBytes);
    }
    
    if(pChnData->ref_buff_data_remain_len+pRefFrm->stRefFrame.mLen <= pChnData->mpPcmCfg->chunkBytes*2)
    {
        memcpy((char*)pChnData->ref_buff+pChnData->ref_buff_data_remain_len,(char*)pRefFrm->stRefFrame.mpAddr,
                                                                                            pRefFrm->stRefFrame.mLen);
        pChnData->ref_buff_data_remain_len += pRefFrm->stRefFrame.mLen;
    }
    else
    {
        aloge("fatal_err_ref_buff_over_flow:%d-%d-%d-%d",pChnData->ref_buff_data_remain_len,pChnData->ref_buff_len,
                                                                pRefFrm->stRefFrame.mLen,pChnData->mpPcmCfg->chunkBytes);
    }

    int frm_size = 160;         // 160 samples as one unit processed by aec library     
    short tmp_near_buffer[160];
    short tmp_far_buffer[160];
    short tmp_out_buffer[160]; 

    short *near_frm_ptr = (short *)pChnData->near_buff;
    short *ref_frm_ptr = (short *)pChnData->ref_buff;
    short *processed_frm_ptr = (short *)pChnData->tmpBuf; 

    int size = (pChnData->near_buff_data_remain_len < pChnData->ref_buff_data_remain_len)? pChnData->near_buff_data_remain_len: pChnData->ref_buff_data_remain_len;
    int left = size / sizeof(short); 

    // start to process
    while(left >= frm_size)
    {
        memcpy((char *)tmp_near_buffer,(char *)near_frm_ptr,frm_size*sizeof(short));
        memcpy((char*)tmp_far_buffer,(char*)ref_frm_ptr,frm_size*sizeof(short));
        
        ret = WebRtcAec_BufferFarend(pChnData->aecmInst, tmp_far_buffer, frm_size);
        if(0 != ret)
        {
            aloge("aec_insert_far_data_failed:%d-%d",ret,((aecpc_t*)pChnData->aecmInst)->lastError);
        }
        
        ret = WebRtcAec_Process(pChnData->aecmInst, tmp_near_buffer, NULL, processed_frm_ptr, NULL, frm_size,pChnData->mpPcmCfg->aec_delay_ms,0);
        if(0 != ret)
        {
            aloge("aec_process_failed:%d-%d",ret,((aecpc_t*)pChnData->aecmInst)->lastError);
        } 

        #ifdef AI_SAVE_AUDIO_PCM
        fwrite(tmp_near_buffer, 1, frm_size*sizeof(short), pChnData->tmp_pcm_fp_in);
        pChnData->tmp_pcm_in_size += frm_size*sizeof(short);
        
        fwrite(tmp_far_buffer, 1, frm_size*sizeof(short), pChnData->tmp_pcm_fp_ref);
        pChnData->tmp_pcm_ref_size += frm_size*sizeof(short);

        aloge("zjx_tbin:%d-%d",pChnData->tmp_pcm_in_size,pChnData->tmp_pcm_ref_size); 
        #endif

        near_frm_ptr += frm_size;
        ref_frm_ptr += frm_size;
        processed_frm_ptr += frm_size;
        left -= frm_size; 

        pChnData->near_buff_data_remain_len -= frm_size*sizeof(short);
        pChnData->ref_buff_data_remain_len -= frm_size*sizeof(short);
    } 

    // move remaining data in internal buffer to the beginning of the buffer 
    if(left > 0)
    { 
        unsigned int near_buff_offset = (unsigned int)near_frm_ptr-(unsigned int)pChnData->near_buff;
        unsigned int far_buff_offset = (unsigned int)ref_frm_ptr-(unsigned int)pChnData->ref_buff;
        if( near_buff_offset < pChnData->near_buff_data_remain_len || 
                       far_buff_offset < pChnData->ref_buff_data_remain_len)
        {
            aloge("aec_fatal_err_buff_left:%d-%d-%d-%d",near_buff_offset,pChnData->near_buff_data_remain_len,
                                                            far_buff_offset, pChnData->ref_buff_data_remain_len);
        }

        memcpy((char*)pChnData->near_buff,(char*)near_frm_ptr,pChnData->near_buff_data_remain_len);
        memcpy((char*)pChnData->ref_buff,(char*)ref_frm_ptr,pChnData->ref_buff_data_remain_len); 
    }

    unsigned int out_offset = (unsigned int)processed_frm_ptr - (unsigned int)pChnData->tmpBuf; 
    

    // move the out data produced by aec library to the internal buffer for conjunction with remaining data left for last process
    if(out_offset + pChnData->out_buff_data_remain_len > pChnData->out_buff_len)
    {
        aloge("aec_fatal_err_out_buff_over_flow:%d-%d-%d",out_offset, pChnData->out_buff_data_remain_len, pChnData->out_buff_len);
    }
    else
    {
        memcpy((char *)pChnData->out_buff+pChnData->out_buff_data_remain_len,(char *)pChnData->tmpBuf,out_offset); 
        pChnData->out_buff_data_remain_len += out_offset;
    } 

    // fetch one valid output frame from output internal buffer, the length of valid frame must equal to chunsize.
    if(pChnData->out_buff_data_remain_len >= pChnData->mpPcmCfg->chunkSize*sizeof(short))
    {
        memcpy((char *)pFrm->mpAddr, (char *)pChnData->out_buff, pChnData->mpPcmCfg->chunkSize*sizeof(short));
        pChnData->out_buff_data_remain_len -= pChnData->mpPcmCfg->chunkSize*sizeof(short);

        if(pFrm->mLen != pChnData->mpPcmCfg->chunkSize*sizeof(short))
        {
            aloge("aec_fatal_error:%d-%d",pChnData->mpPcmCfg->chunkSize*sizeof(short),pFrm->mLen);
        }

        #ifdef AI_SAVE_AUDIO_PCM
        fwrite(pFrm->mpAddr, 1, pFrm->mLen, pChnData->tmp_pcm_fp_out);
        pChnData->tmp_pcm_out_size += pFrm->mLen; 
        aloge("zjx_tbo:%d-%d-%d",pChnData->tmp_pcm_out_size,pFrm->mLen,pChnData->mpPcmCfg->chunkSize*sizeof(short));
        #endif
        
        pChnData->aec_valid_frm = 1;
        if(pChnData->out_buff_data_remain_len > pChnData->mpPcmCfg->chunkSize*sizeof(short))
        {
            aloge("aec_fatal_err_out_buff_data_mov:%d-%d",pChnData->mpPcmCfg->chunkSize*sizeof(short),pChnData->out_buff_data_remain_len);
        }
        else
        {
            memcpy((char *)pChnData->out_buff,((char *)pChnData->out_buff+pChnData->mpPcmCfg->chunkSize*sizeof(short)), pChnData->out_buff_data_remain_len);
        }
    }
    else
    {
        pChnData->aec_valid_frm = 0;
    } 

    return SUCCESS;    
}
#endif

static int audioEffectProc(AI_CHN_DATA_S *pChnData, AUDIO_FRAME_S *pInFrm)
{
    if (1)//pChnData->mVqeCfg.bAecOpen)
    {
        AUDIO_FRAME_S *pPlayFrm = pChnData->mpPlayMgr->getValidFrame(pChnData->mpPlayMgr);
        if (pPlayFrm != NULL)
        {
          #if AEC_IMPLEMENT_IN_AI_CHL
            AEC_FRAME_S refFrm;
            refFrm.bValid = 1;

            memcpy(&refFrm.stRefFrame, pPlayFrm, sizeof(AUDIO_FRAME_S));
//            audioAecProcess(pChnData, pInFrm, &refFrm);
            audio_aec_process_new(pChnData, pInFrm, &refFrm);
          #endif
            pChnData->mpPlayMgr->releaseFrame(pChnData->mpPlayMgr, pPlayFrm);
        }
        else
        {
            aloge("aec_no_ref");
        }
    }

    return 0;
}


#if 0
static int audioEffectProc(AI_CHN_DATA_S *pChnData, AUDIO_FRAME_S *pInFrm)
{
#ifdef CFG_AUDIO_EFFECT_AEC
    else
#else
    if (1)
#endif
    {
#ifdef CFG_AUDIO_EFFECT_RNR
        if (pChnData->mVqeCfg.bRnrOpen)
        {
            audioRnrProcess(pChnData, pInFrm);
        }
#endif
#ifdef CFG_AUDIO_EFFECT_EQ
        if (pChnData->mVqeCfg.bEqOpen)
        {
            audioEQHandle(pChnData, pInFrm);
        }
#endif
#ifdef CFG_AUDIO_EFFECT_DRC
        if (pChnData->mVqeCfg.bDrcOpen)
        {
            audioDrcProcess(pChnData, pInFrm);
        }
#endif
    }

    return 0;
}
#endif /* 0 */


ERRORTYPE AIChannel_ComponentInit(PARAM_IN COMP_HANDLETYPE hComponent)
{
    MM_COMPONENTTYPE *pComp;
    AI_CHN_DATA_S *pChnData;
    ERRORTYPE eError = SUCCESS;
    int err;

    pComp = (MM_COMPONENTTYPE*)hComponent;

    // Create private data
    pChnData = (AI_CHN_DATA_S*)malloc(sizeof(AI_CHN_DATA_S));
    if (pChnData == NULL) {
        aloge("alloc AI_CHN_DATA_S error!");
        return FAILURE;
    }
    memset(pChnData, 0x0, sizeof(AI_CHN_DATA_S));

    pComp->pComponentPrivate = (void*)pChnData;
    pChnData->state = COMP_StateLoaded;
    pChnData->hSelf = hComponent;

    // Fill in function pointers
    pComp->SetCallbacks             = AIChannel_SetCallbacks;
    pComp->SendCommand              = AIChannel_SendCommand;
    pComp->GetConfig                = AIChannel_GetConfig;
    pComp->SetConfig                = AIChannel_SetConfig;
    pComp->GetState                 = AIChannel_GetState;
    pComp->ComponentTunnelRequest   = AIChannel_ComponentTunnelRequest;
    pComp->ComponentDeInit          = AIChannel_ComponentDeInit;
    pComp->EmptyThisBuffer          = AIChannel_EmptyThisBuffer;
    pComp->FillThisBuffer           = AIChannel_FillThisBuffer;

    pChnData->sPortParam.nPorts = 0;
    pChnData->sPortParam.nStartPortNumber = 0x0;

    pChnData->sPortDef[AI_CHN_PORT_INDEX_CAP_IN].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_CAP_IN].bEnabled = TRUE;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_CAP_IN].eDomain = COMP_PortDomainAudio;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_CAP_IN].eDir = COMP_DirInput;
    pChnData->sPortBufSupplier[AI_CHN_PORT_INDEX_CAP_IN].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortBufSupplier[AI_CHN_PORT_INDEX_CAP_IN].eBufferSupplier = COMP_BufferSupplyOutput;
    pChnData->sPortTunnelInfo[AI_CHN_PORT_INDEX_CAP_IN].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortTunnelInfo[AI_CHN_PORT_INDEX_CAP_IN].eTunnelType = TUNNEL_TYPE_COMMON;
    pChnData->sPortParam.nPorts++;

    pChnData->sPortDef[AI_CHN_PORT_INDEX_AO_IN].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_AO_IN].bEnabled = TRUE;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_AO_IN].eDomain = COMP_PortDomainAudio;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_AO_IN].eDir = COMP_DirInput;
    pChnData->sPortBufSupplier[AI_CHN_PORT_INDEX_AO_IN].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortBufSupplier[AI_CHN_PORT_INDEX_AO_IN].eBufferSupplier = COMP_BufferSupplyOutput;
    pChnData->sPortTunnelInfo[AI_CHN_PORT_INDEX_AO_IN].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortTunnelInfo[AI_CHN_PORT_INDEX_AO_IN].eTunnelType = TUNNEL_TYPE_COMMON;
    pChnData->sPortParam.nPorts++;

    pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AENC].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AENC].bEnabled = TRUE;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AENC].eDomain = COMP_PortDomainAudio;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AENC].eDir = COMP_DirOutput;
    pChnData->sPortBufSupplier[AI_CHN_PORT_INDEX_OUT_AENC].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortBufSupplier[AI_CHN_PORT_INDEX_OUT_AENC].eBufferSupplier = COMP_BufferSupplyOutput;
    pChnData->sPortTunnelInfo[AI_CHN_PORT_INDEX_OUT_AENC].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortTunnelInfo[AI_CHN_PORT_INDEX_OUT_AENC].eTunnelType = TUNNEL_TYPE_COMMON;
    pChnData->sPortParam.nPorts++;

    pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AO].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AO].bEnabled = TRUE;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AO].eDomain = COMP_PortDomainAudio;
    pChnData->sPortDef[AI_CHN_PORT_INDEX_OUT_AO].eDir = COMP_DirOutput;
    pChnData->sPortBufSupplier[AI_CHN_PORT_INDEX_OUT_AO].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortBufSupplier[AI_CHN_PORT_INDEX_OUT_AO].eBufferSupplier = COMP_BufferSupplyOutput;
    pChnData->sPortTunnelInfo[AI_CHN_PORT_INDEX_OUT_AO].nPortIndex = pChnData->sPortParam.nPorts;
    pChnData->sPortTunnelInfo[AI_CHN_PORT_INDEX_OUT_AO].eTunnelType = TUNNEL_TYPE_COMMON;
    pChnData->sPortParam.nPorts++;

    if (audioHw_AI_GetPcmConfig(pChnData->mMppChnInfo.mDevId, &pChnData->mpPcmCfg) != SUCCESS) {
        aloge("audioHw_AI_GetPcmConfig error!");
        eError = FAILURE;
        goto ERR_EXIT0;
    }
    if (audioHw_AI_GetAIOAttr(pChnData->mMppChnInfo.mDevId, &pChnData->mpAioAttr) != SUCCESS) {
        aloge("audioHw_AI_GetAIOAttr error!");
        eError = FAILURE;
        goto ERR_EXIT0;
    }

    pChnData->mpCapMgr = pcmBufMgrCreate(AI_CHN_MAX_CACHE_FRAME, pChnData->mpPcmCfg->chunkBytes);
    if (pChnData->mpCapMgr == NULL) {
        aloge("pcmBufMgrCreate error!");
        eError = FAILURE;
        goto ERR_EXIT0;
    }

    pChnData->mpPlayMgr = pcmBufMgrCreate(AI_CHN_MAX_CACHE_FRAME, pChnData->mpPcmCfg->chunkBytes);
    if (pChnData->mpPlayMgr == NULL) {
        aloge("play pcmBufMgrCreate error!");
        eError = FAILURE;
        goto ERR_EXIT1;
    }

#if AEC_IMPLEMENT_IN_AI_CHL
    pChnData->near_buff_len = pChnData->mpPcmCfg->chunkBytes*2;
    pChnData->near_buff = (short *)malloc(pChnData->near_buff_len);
    if(NULL == pChnData->near_buff)
    {
        eError = FAILURE;
        goto ERR_EXIT2;
    }
    pChnData->near_buff_data_remain_len = 0;

    pChnData->ref_buff_len = pChnData->mpPcmCfg->chunkBytes*2;
    pChnData->ref_buff = (short *)malloc(pChnData->ref_buff_len);
    if(NULL == pChnData->ref_buff)
    {
        eError = FAILURE;
        goto ERR_EXIT2_0;
    }
    pChnData->ref_buff_data_remain_len = 0;

    pChnData->out_buff_len = pChnData->mpPcmCfg->chunkBytes*2;
    pChnData->out_buff = (short *)malloc(pChnData->out_buff_len);
    if(NULL == pChnData->out_buff)
    {
        eError = FAILURE;
        goto ERR_EXIT2_1;
    }
    pChnData->out_buff_data_remain_len = 0;

    pChnData->aec_first_frm_pts = -1;
    pChnData->aec_first_frm_sent_to_next_module = 0;
    pChnData->aec_valid_frm = 0;
    pChnData->ai_sent_to_ao_frms = 0;

    pChnData->tmpBufLen = pChnData->mpPcmCfg->chunkBytes*2;
    pChnData->tmpBuf = (short *)malloc(pChnData->tmpBufLen);
    if(pChnData->tmpBuf == NULL)
    {
        aloge("alloc aec tmpbuffer fail!");
        eError = FAILURE;
        goto ERR_EXIT2_2;
    }
#endif

    err = pthread_mutex_init(&pChnData->mIgnoreDataLock, NULL);
    if(err != 0)
    {
        aloge("pthread mutex init fail!");
        eError = FAILURE;
        goto ERR_EXIT3_3;
    }
    err = pthread_mutex_init(&pChnData->mStateLock, NULL);
    if(err != 0)
    {
        aloge("pthread mutex init fail!");
        eError = FAILURE;
        goto ERR_EXIT3;
    }
    err = pthread_mutex_init(&pChnData->mCapMgrLock, NULL);
    if(err != 0)
    {
        aloge("fatal error! pthread mutex init fail!");
    }
 
    if (message_create(&pChnData->mCmdQueue) < 0){
        aloge("message_create error!");
        eError = FAILURE;
        goto ERR_EXIT4;
    }
    err = cdx_sem_init(&pChnData->mAllFrameRelSem, 0);
    if (err != 0) {
        aloge("cdx_sem_init AllFrameRelSem error!");
        goto ERR_EXIT5;
    }

    err = cdx_sem_init(&pChnData->mWaitOutFrameSem, 0);
    if (err != 0) {
        aloge("cdx_sem_init mWaitOutFrameSem error!");
        goto ERR_EXIT6;
    }

//    err = cdx_sem_init(&pChnData->mWaitGetAllOutFrameSem, 0);
//    if (err != 0) {
//        aloge("cdx_sem_init mWaitGetAllOutFrameSem error!");
//        goto ERR_EXIT7;
//    }

#ifdef AI_SAVE_AUDIO_PCM
    pChnData->pcm_fp = fopen("/mnt/extsd/ai_pcm", "wb");
    pChnData->tmp_pcm_fp_in = fopen("/mnt/extsd/tmp_in_ai_pcm", "wb"); 
    pChnData->tmp_pcm_fp_ref = fopen("/mnt/extsd/tmp_ref_ai_pcm", "wb");
    pChnData->tmp_pcm_fp_out = fopen("/mnt/extsd/tmp_out_ai_pcm", "wb");
#endif

    err = pthread_create(&pChnData->mThreadId, NULL, AIChannel_ComponentThread, pChnData);
    if (err || !pChnData->mThreadId) {
        aloge("pthread_create error!");
        eError = FAILURE;
        goto ERR_EXIT8;
    }
    return SUCCESS;


ERR_EXIT8:
    //cdx_sem_deinit(&pChnData->mWaitGetAllOutFrameSem);
ERR_EXIT7:
    cdx_sem_deinit(&pChnData->mWaitOutFrameSem);
ERR_EXIT6:
    cdx_sem_deinit(&pChnData->mAllFrameRelSem);
ERR_EXIT5:
    message_destroy(&pChnData->mCmdQueue);
ERR_EXIT4:
    pthread_mutex_destroy(&pChnData->mStateLock);
ERR_EXIT3:
    pthread_mutex_destroy(&pChnData->mIgnoreDataLock);
ERR_EXIT3_3:
#if AEC_IMPLEMENT_IN_AI_CHL    
    if(NULL != pChnData->tmpBuf)
    {
        free(pChnData->tmpBuf);
        pChnData->tmpBuf = NULL;
    }
ERR_EXIT2_2:
    if(NULL != pChnData->out_buff)
    {
        free(pChnData->out_buff);
        pChnData->out_buff = NULL;
    }
ERR_EXIT2_1:
    if(NULL != pChnData->ref_buff)    
    {
        free(pChnData->ref_buff);
        pChnData->ref_buff = NULL;
    }
ERR_EXIT2_0:
    if(NULL != pChnData->near_buff)
    {
        free(pChnData->near_buff);
        pChnData->near_buff = NULL;
    }
#endif    
ERR_EXIT2:
    pcmBufMgrDestroy(pChnData->mpPlayMgr);
ERR_EXIT1:
    pcmBufMgrDestroy(pChnData->mpCapMgr);
ERR_EXIT0:
    free(pChnData);
    return eError;
}

static void *AIChannel_ComponentThread(void *pThreadData)
{
    unsigned int cmddata;
    CompInternalMsgType cmd;
    message_t cmd_msg;
    AI_CHN_DATA_S *pChnData = (AI_CHN_DATA_S*)pThreadData;
    ERRORTYPE eError;
    BOOL bIgnoreFlag;
    alogv("AI channel ComponentThread start run...");

    while (1) {
PROCESS_MESSAGE:
        if (get_message(&pChnData->mCmdQueue, &cmd_msg) == 0)
        {
            cmd = cmd_msg.command;
            cmddata = (unsigned int)cmd_msg.para0;
            if (cmd == SetState)
            {
                //alogv("cmd=SetState, cmddata=%d", cmddata);
                //pthread_mutex_lock(&pChnData->mStateLock);
                if (pChnData->state == (COMP_STATETYPE) (cmddata))
                {
                    CompSendEvent(pChnData, COMP_EventError, ERR_AI_SAMESTATE, 0);
                    CompSendEvent(pChnData, COMP_EventCmdComplete, COMP_CommandStateSet, pChnData->state);
                }
                else
                {
                    switch ((COMP_STATETYPE)cmddata)
                    {
                        case COMP_StateInvalid:
                        {
                            pChnData->state = COMP_StateInvalid;
                            CompSendEvent(pChnData, COMP_EventError, ERR_AI_INVALIDSTATE, 0);
                            CompSendEvent(pChnData, COMP_EventCmdComplete, COMP_CommandStateSet, pChnData->state);
                            break;
                        }
                        case COMP_StateLoaded:
                        {
                            if (pChnData->state != COMP_StateIdle)
                            {
                                CompSendEvent(pChnData, COMP_EventError, ERR_AI_INCORRECT_STATE_TRANSITION, 0);
                            }
                            alogv("AI_Comp: idle->loaded. StateLoaded begin...");

                            pChnData->mWaitAllFrameReleaseFlag = 1;
                            while (!pChnData->mpCapMgr->usingFrmEmpty(pChnData->mpCapMgr)) {
                                cdx_sem_wait(&pChnData->mAllFrameRelSem);
                            }
                            //while (!pChnData->mpCapMgr->fillingFrmEmpty(pChnData->mpCapMgr)) {
                            //    cdx_sem_wait(&pChnData->mAllFrameRelSem);
                            //}
                            pChnData->mWaitAllFrameReleaseFlag = 0;
                            pChnData->state = COMP_StateLoaded;
                            CompSendEvent(pChnData, COMP_EventCmdComplete, COMP_CommandStateSet, pChnData->state);
                            break;
                        }
                        case COMP_StateIdle:
                        {
                            if (pChnData->state == COMP_StateLoaded)
                            {
                                alogv("AI_Comp: loaded->idle ...");
                                pChnData->state = COMP_StateIdle;
                            }
                            else if (pChnData->state == COMP_StatePause || pChnData->state == COMP_StateExecuting)
                            {
                                alogv("AI_Comp: pause/executing[0x%x]->idle ...", pChnData->state);
                                pChnData->state = COMP_StateIdle;
                            }
                            else
                            {
                                CompSendEvent(pChnData, COMP_EventError, ERR_AI_INCORRECT_STATE_TRANSITION, 0);
                            }
                            CompSendEvent(pChnData, COMP_EventCmdComplete, COMP_CommandStateSet, pChnData->state);
                            break;
                        }
                        case COMP_StateExecuting:
                        {
                            if (pChnData->state == COMP_StateIdle || pChnData->state == COMP_StatePause)
                            {
                                alogv("AI_Comp: idle/pause[0x%x]->executing ...", pChnData->state);
                                pChnData->state = COMP_StateExecuting;
                            }
                            else
                            {
                                CompSendEvent(pChnData, COMP_EventError, ERR_AI_INCORRECT_STATE_TRANSITION, 0);
                            }
                            CompSendEvent(pChnData, COMP_EventCmdComplete, COMP_CommandStateSet, pChnData->state);
                            break;
                        }
                        case COMP_StatePause:
                        {
                            if (pChnData->state == COMP_StateIdle || pChnData->state == COMP_StateExecuting)
                            {
                                pChnData->state = COMP_StatePause;
                            }
                            else
                            {
                                CompSendEvent(pChnData, COMP_EventError, ERR_AI_INCORRECT_STATE_TRANSITION, 0);
                            }
                            CompSendEvent(pChnData, COMP_EventCmdComplete, COMP_CommandStateSet, pChnData->state);
                            break;
                        }
                        default:
                            break;
                    }
                }
                //pthread_mutex_unlock(&pChnData->mStateLock);
            }
            else if (cmd == AIChannel_CapDataAvailable)
            {
                pChnData->mWaitingCapDataFlag = FALSE;
            }
            else if (cmd == AIChannel_PlayDataAvailable)
            {
            }
            else if (cmd == StopPort)
            {
            }
            else if (cmd == Stop)
            {
                goto EXIT;
            }
            goto PROCESS_MESSAGE;
        }

        if (pChnData->state == COMP_StateExecuting)
        {
            PcmBufferManager *pCapMgr = pChnData->mpCapMgr;
            pthread_mutex_lock(&pChnData->mCapMgrLock);
            if (TRUE == pCapMgr->validFrmEmpty(pCapMgr))
            {
                pChnData->mWaitingCapDataFlag = TRUE;
                pthread_mutex_unlock(&pChnData->mCapMgrLock);
                TMessage_WaitQueueNotEmpty(&pChnData->mCmdQueue, 0);
                goto PROCESS_MESSAGE;
            }
            else
            {
                pthread_mutex_unlock(&pChnData->mCapMgrLock);
                if (TRUE == pChnData->mOutputPortTunnelFlag[AI_OUTPORT_SUFFIX_AENC])
                {
                    COMP_INTERNAL_TUNNELINFOTYPE *pPortTunnelInfo = &pChnData->sPortTunnelInfo[AI_CHN_PORT_INDEX_OUT_AENC];
                    MM_COMPONENTTYPE *pOutTunnelComp = (MM_COMPONENTTYPE*)pPortTunnelInfo->hTunnel;
                    AUDIO_FRAME_S *pFrm = pCapMgr->getValidFrame(pCapMgr);
#ifdef AI_SAVE_AUDIO_PCM
                    fwrite(pFrm->mpAddr, 1, pFrm->mLen, pChnData->pcm_fp);
                    pChnData->pcm_sz += pFrm->mLen;
#endif
                    if (pChnData->mSaveFileFlag)
                    {
                        fwrite(pFrm->mpAddr, 1, pFrm->mLen, pChnData->mFpSaveFile);
                        pChnData->mSaveFileSize += pFrm->mLen;
                    }

                    if(-1 == pChnData->aec_first_frm_pts)
                    {
                        pChnData->aec_first_frm_pts = pFrm->tmp_pts;
                    } 
                    pthread_mutex_lock(&pChnData->mIgnoreDataLock);
                    bIgnoreFlag = pChnData->mbIgnore;
                    pthread_mutex_unlock(&pChnData->mIgnoreDataLock);
                    if(bIgnoreFlag != TRUE)
                    {
                        if (pChnData->mUseVqeLib)
                        {
                            audioEffectProc(pChnData, pFrm);
                            if(pChnData->aec_valid_frm)
                            {
                                if(0 == pChnData->aec_first_frm_sent_to_next_module)
                                {
                                    pChnData->aec_first_frm_sent_to_next_module = 1;
                                    pFrm->tmp_pts = pChnData->aec_first_frm_pts;    // the pts of first frame is important for aenc,so keep it.
                                }
                                COMP_BUFFERHEADERTYPE obh;
                                obh.nOutputPortIndex = pPortTunnelInfo->nPortIndex;
                                obh.nInputPortIndex = pPortTunnelInfo->nTunnelPortIndex;
                                obh.pOutputPortPrivate = pFrm;
                                eError = pOutTunnelComp->EmptyThisBuffer(pOutTunnelComp, &obh);
                                if(SUCCESS != eError)
                                {
                                    alogv("[AI->AEnc] send pcm failed!");
                                }
                                CompSendEvent(pChnData, COMP_EventBufferPrefilled, pFrm->mLen, 0);
                            }
                        }
                        else
                        {
                            COMP_BUFFERHEADERTYPE obh;
                            obh.nOutputPortIndex = pPortTunnelInfo->nPortIndex;
                            obh.nInputPortIndex = pPortTunnelInfo->nTunnelPortIndex;
                            obh.pOutputPortPrivate = pFrm;
                            eError = pOutTunnelComp->EmptyThisBuffer(pOutTunnelComp, &obh);
                            if(SUCCESS != eError)
                            {
                                alogv("[AI->AEnc] send pcm failed!");
                            }
                            CompSendEvent(pChnData, COMP_EventBufferPrefilled, pFrm->mLen, 0);
                        }
                    }
                    else
                    {
                        CompSendEvent(pChnData, COMP_EventBufferPrefilled, pFrm->mLen, 1);
                    }
                    // AEnc copy pcm to AEncLib InputBuf, so release pcm directly by this AI thread
                    // if use ref to occupy data, MUST release by the other thread!
                    pCapMgr->releaseFrame(pChnData->mpCapMgr, pFrm);
                }
                else if (TRUE == pChnData->mOutputPortTunnelFlag[AI_OUTPORT_SUFFIX_AO])
                {
                    COMP_INTERNAL_TUNNELINFOTYPE *pPortTunnelInfo = &pChnData->sPortTunnelInfo[AI_CHN_PORT_INDEX_OUT_AO];
                    MM_COMPONENTTYPE *pOutTunnelComp = (MM_COMPONENTTYPE*)pPortTunnelInfo->hTunnel;
                    AUDIO_FRAME_S *pFrm = pCapMgr->getValidFrame(pCapMgr);
                    COMP_BUFFERHEADERTYPE obh;
                    obh.nOutputPortIndex = pPortTunnelInfo->nPortIndex;
                    obh.nInputPortIndex = pPortTunnelInfo->nTunnelPortIndex;
                    obh.pOutputPortPrivate = pFrm;

                    if (pChnData->mUseVqeLib)
                    {
                        if( 2 > pChnData->ai_sent_to_ao_frms)   // must send at least 2 frames to ao at first in order to get reference frame for ai.
                        {
                            pChnData->ai_sent_to_ao_frms ++;
                            eError = pOutTunnelComp->EmptyThisBuffer(pOutTunnelComp, &obh);
                            if(SUCCESS != eError)
                            {
                                alogw("[AI->AO] send pcm failed!");
                            } 
                        }
                        else
                        {
                            audioEffectProc(pChnData, pFrm);
                            if(pChnData->aec_valid_frm)
                            {
                                eError = pOutTunnelComp->EmptyThisBuffer(pOutTunnelComp, &obh);
                                if(SUCCESS != eError)
                                {
                                    alogw("[AI->AO] send pcm failed!");
                                } 
                            }
                            else
                            { 
                                pCapMgr->releaseFrame(pChnData->mpCapMgr, pFrm);
                            }
                        } 
                    }
                    else
                    {
                        eError = pOutTunnelComp->EmptyThisBuffer(pOutTunnelComp, &obh);
                        if(SUCCESS != eError)
                        {
                            alogw("[AI->AO] send pcm failed!");
                        }
                    }
                }
                else
                {
//                    if (pChnData->mWaitingOutFrameFlag)
//                        cdx_sem_signal(&pChnData->mWaitOutFrameSem);
//                    while (!pCapMgr->validFrmEmpty(pCapMgr))
//                    {
//                        cdx_sem_wait(&pChnData->mWaitGetAllOutFrameSem);
//                    }
                    //do nothing in non-tunnel mode.
                    TMessage_WaitQueueNotEmpty(&pChnData->mCmdQueue, 0);
                }
            }
        }
        else
        {
            alogv("AI_Comp not StateExecuting");
            TMessage_WaitQueueNotEmpty(&pChnData->mCmdQueue, 0);
        }
    }

EXIT:
    alogv("AI channel ComponentThread stopped!");
    return NULL;
}

