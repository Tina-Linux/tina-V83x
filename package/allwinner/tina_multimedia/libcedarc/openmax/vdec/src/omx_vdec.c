#define LOG_TAG "omx_vdec"
#include "log.h"

#include "omx_vdec.h"
#include "omx_pub_def.h"


static inline OMX_U32 toIndex(OMX_STATETYPE state)
{
    return ((OMX_U32)state - 1);
}

static OMX_ERRORTYPE omxSetRole(AwOmxVdec* impl, const OMX_U8* srcRole)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.mjpeg", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.mjpeg", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }
        strncpy((char *)impl->m_cRole, "video_decoder.mjpeg", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingMJPEG;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.mpeg1", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.mpeg1", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.mpeg1", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingMPEG1;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.mpeg2", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.mpeg2", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.mpeg2", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingMPEG2;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.mpeg4", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.mpeg4", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char*)impl->m_cRole, "video_decoder.mpeg4", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingMPEG4;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.msmpeg4v1",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.msmpeg4v1", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char*)impl->m_cRole, "video_decoder.msmpeg4v1", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingMSMPEG4V1;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.msmpeg4v2",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.msmpeg4v2", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char*)impl->m_cRole, "video_decoder.msmpeg4v2", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingMSMPEG4V2;
    }

    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.msmpeg4v3",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.msmpeg4v3", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char*)impl->m_cRole, "video_decoder.msmpeg4v3", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingMSMPEG4V2;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.divx4",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.divx4", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char*)impl->m_cRole, "video_decoder.divx4", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingDIVX4;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.rx",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.rx", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char*)impl->m_cRole, "video_decoder.rx", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingRX;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.avs",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.avs", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char*)impl->m_cRole, "video_decoder.avs", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingAVS;
    }

    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.divx", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.divx", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char*)impl->m_cRole, "video_decoder.divx", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingDIVX;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.xvid", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.xvid", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char*)impl->m_cRole, "video_decoder.xvid", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingXVID;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.h263", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.h263", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.h263", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingH263;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.s263", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.s263", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.s263", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingS263;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.rxg2", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.rxg2", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.rxg2", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingRXG2;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.wmv1", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.wmv1", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.wmv1", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingWMV1;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.wmv2", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.wmv2", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.wmv2", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingWMV2;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.vc1", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.vc1", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.vc1", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingWMV;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.vp6", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.vp6", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.vp6", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingVP6; //OMX_VIDEO_CodingVPX
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.vp8", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.vp8", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.vp8", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingVP8; //OMX_VIDEO_CodingVPX
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.vp9", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.vp9", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.vp9", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingVP9; //OMX_VIDEO_CodingVPX
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.avc.secure",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.avc", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.avc", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat       = OMX_VIDEO_CodingAVC;

#if(PLATFORM_SURPPORT_SECURE_OS == 1)
        //impl->mIsSecureVideoFlag         = OMX_TRUE;
#endif
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.avc", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.avc", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.avc", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingAVC;
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.hevc.secure",
             OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.hevc", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.hevc", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat       = OMX_VIDEO_CodingHEVC;

#if(PLATFORM_SURPPORT_SECURE_OS == 1)
        //impl->mIsSecureVideoFlag         = OMX_TRUE;
#endif
    }
    else if(!strncmp((char*)impl->m_cName, "OMX.allwinner.video.decoder.hevc", OMX_MAX_STRINGNAME_SIZE))
    {
        if(srcRole != NULL && strncmp((const char*)srcRole,"video_decoder.hevc", OMX_MAX_STRINGNAME_SIZE))
        {
            OMX_LOGE("Setparameter: unknown Index %s\n", srcRole);
            return OMX_ErrorUnsupportedSetting;
        }

        strncpy((char *)impl->m_cRole, "video_decoder.hevc", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat      = OMX_VIDEO_CodingHEVC;
    }
    else
    {
        OMX_LOGE("\nERROR:Unknown Component\n");
        err = OMX_ErrorInvalidComponentName;
    }
    return err;
}

static inline AwOmxVdecPort* getPort(AwOmxVdec* impl, OMX_U32 nPortIndex)
{
    if (nPortIndex == impl->m_InPort->m_sPortDefType.nPortIndex)
        return impl->m_InPort;
    else if (nPortIndex == impl->m_OutPort->m_sPortDefType.nPortIndex)
        return impl->m_OutPort;
    else
        return NULL;
}

static inline void freeBuffersAllocBySelf(AwOmxVdec* impl, AwOmxVdecPort* port)
{
    if(getPortAllocSize(port) > 0)
    {
        OMX_U32       nIndex;
        for(nIndex=0; nIndex < port->m_sBufList.nBufNeedSize; nIndex++)
        {
            OMX_U8* pBuffer = port->m_sBufList.pBufArr[nIndex].pBuffer;
            if(pBuffer != NULL)
            {
                if(port->bAllocBySelfFlags)
                {
                    OmxVdecoderFreePortBuffer(impl->pDec, port, pBuffer);
                    pBuffer = NULL;
                }
            }
        }
    }
}

static inline void postMessage(CdcMessageQueue* msgHandle, AW_OMX_MSG_TYPE type, uintptr_t para)
{
    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));
    msg.messageId = type;
    msg.params[0] = para;

    CdcMessageQueuePostMessage(msgHandle, &msg);
}

static void controlSetState(AwOmxVdec* impl, OMX_STATETYPE target_state)
{
    OMX_LOGD("current state:%s, target state:%s",
        OmxState2String(impl->m_state), OmxState2String(target_state));
    if (impl->m_state == target_state)
    {
        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                        OMX_EventError, OMX_ErrorSameState, 0 , NULL);
    }
    else if (target_state ==  OMX_StateInvalid)
    {
        impl->m_state = OMX_StateInvalid;
        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                        OMX_EventError, OMX_ErrorInvalidState, 0 , NULL);
        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                        OMX_EventCmdComplete, OMX_CommandStateSet,
                                        OMX_StateInvalid, NULL);
    }
    else if(impl->m_StateTransitTable[toIndex(impl->m_state)][toIndex(target_state)])
    {
        int ret = impl->m_StateTransitTable[toIndex(impl->m_state)][toIndex(target_state)](impl);
        if(ret != -1)
        {
            OMX_LOGD("Transit current state:%s --> target state:%s --OK!",
                 OmxState2String(impl->m_state), OmxState2String(target_state));
            impl->m_state = target_state;
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandStateSet,
                                            target_state, NULL);
        }
        else
        {
            OMX_LOGW("Transit current state:%s --> target state:%s --Failed!",
                 OmxState2String(impl->m_state), OmxState2String(target_state));
        }
    }
    else
    {
        OMX_LOGE("Transit current state:%s --> target state:%s --Invalid!",
             OmxState2String(impl->m_state), OmxState2String(target_state));
        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                        OMX_EventError, OMX_ErrorIncorrectStateTransition,
                                        0 , NULL);
    }
}

static inline void controlFlush(AwOmxVdec* impl, OMX_U32 portIdx)
{
    OMX_LOGD(" flush command, portIdx:%lu", portIdx);
    impl->m_InputNum  = 0;
    impl->m_OutputNum = 0;
    if (portIdx == kInputPortIndex || portIdx == OMX_ALL)
    {
        doFlushPortBuffer(impl->m_InPort);
        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                        OMX_EventCmdComplete, OMX_CommandFlush,
                                        kInputPortIndex, NULL);
    }

    if (portIdx == kOutputPortIndex || portIdx == OMX_ALL)
    {
        OmxVdecoderFlush(impl->pDec);
        doFlushPortBuffer(impl->m_OutPort);
        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                        OMX_EventCmdComplete, OMX_CommandFlush,
                                        kOutputPortIndex, NULL);
    }

}

static inline void controlStopPort(AwOmxVdec* impl, OMX_U32 portIdx)
{
    OMX_U32 nTimeout = 0x0;
    OMX_PARAM_PORTDEFINITIONTYPE* inDef  = getPortDef(impl->m_InPort);
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(impl->m_OutPort);
    OMX_LOGD("gqy****stop port****, portIdx:%lu", portIdx);
    if (portIdx == kInputPortIndex || portIdx == OMX_ALL)
    {
        doFlushPortBuffer(impl->m_InPort);
        inDef->bEnabled = OMX_FALSE;
        impl->m_InPort->bEnabled = OMX_FALSE;
    }

    if (portIdx == kOutputPortIndex || portIdx == OMX_ALL)
    {
        OmxVdecoderStandbyBuffer(impl->pDec);
        doFlushPortBuffer(impl->m_OutPort);
        outDef->bEnabled = OMX_FALSE;
        impl->m_OutPort->bEnabled = OMX_FALSE;
    }

    // Wait for all buffers to be freed
    OMX_U32 nPerSecond = 100;
    while (1)
    {
        if (portIdx == kInputPortIndex && !inDef->bPopulated)
        {
            //*Return cmdcomplete event if input unpopulated
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            kInputPortIndex, NULL);
            break;
        }
        if (portIdx == kOutputPortIndex && !outDef->bPopulated)
        {
            //*Return cmdcomplete event if output unpopulated
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            kOutputPortIndex, NULL);
            break;
        }
        if (portIdx == OMX_ALL && !inDef->bPopulated && !outDef->bPopulated)
        {
            //*Return cmdcomplete event if inout & output unpopulated
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            kInputPortIndex, NULL);
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            kOutputPortIndex, NULL);
            break;
        }
        //* sleep 60s here, or some gts not pass when the network is slow
        if (nTimeout++ > (nPerSecond*60))
        {
            OMX_LOGD("callback timeout when stop port, nTimeout = %lu s",nTimeout/100);
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventError,
                                            OMX_ErrorPortUnresponsiveDuringDeallocation, 0 , NULL);
               break;
        }
        //* printf the wait time per 1s
        if(nTimeout%(nPerSecond) == 0)
        {
            OMX_LOGD("we had sleep %lu s here",nTimeout/100);
        }
        usleep(10*1000);
    }
}


static inline void controlRestartPort(AwOmxVdec* impl, OMX_U32 portIdx)
{
    OMX_LOGD(" restart port command. portIdx:%lx,m_state:%s",
             portIdx, OmxState2String(impl->m_state));
    OMX_U32 nTimeout = 0x0;
    OMX_PARAM_PORTDEFINITIONTYPE* inDef  = getPortDef(impl->m_InPort);
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(impl->m_OutPort);

    while (1)
    {
        if (portIdx == kInputPortIndex
            && (impl->m_state == OMX_StateLoaded || inDef->bPopulated))
        {
            inDef->bEnabled = OMX_TRUE;
            impl->m_InPort->bEnabled = OMX_TRUE;
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            kInputPortIndex, NULL);
            break;
        }
        else if (portIdx == kOutputPortIndex
                 && (impl->m_state == OMX_StateLoaded || outDef->bPopulated))
        {
            outDef->bEnabled = OMX_TRUE;
            impl->m_OutPort->bEnabled = OMX_TRUE;
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            kOutputPortIndex, NULL);
            break;
        }
        else if (portIdx == OMX_ALL &&
                 (impl->m_state == OMX_StateLoaded ||
                   (inDef->bPopulated && outDef->bPopulated)))
        {
            inDef->bEnabled  = OMX_TRUE;
            outDef->bEnabled = OMX_TRUE;
            impl->m_InPort->bEnabled = OMX_TRUE;
            impl->m_OutPort->bEnabled = OMX_TRUE;
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            kInputPortIndex, NULL);
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            kOutputPortIndex, NULL);
            break;
        }
        else if (nTimeout++ > OMX_MAX_TIMEOUTS)
        {
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                   OMX_EventError, OMX_ErrorPortUnresponsiveDuringAllocation, 0, NULL);
            break;
        }

        usleep(OMX_TIMEOUT*1000);
    }
}

static int doStateWaitforResources2Idle(AwOmxVdec* impl)
{
    OMX_U32 nTimeout = 0x0;
    OMX_PARAM_PORTDEFINITIONTYPE* inDef  = getPortDef(impl->m_InPort);
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(impl->m_OutPort);
    while (1)
    {
       OMX_LOGD("bEnabled[%d],[%d],bPopulated[%d],[%d]",
             inDef->bEnabled,   outDef->bEnabled,
             inDef->bPopulated, outDef->bPopulated);
       //*Ports have to be populated before transition completes
       if ((!inDef->bEnabled && !outDef->bEnabled)   ||
           (inDef->bPopulated && outDef->bPopulated))
       {
           //* Open decoder
           //* TODO.
           //*allocat input buffer?
            break;
       }
       else if (nTimeout++ > OMX_MAX_TIMEOUTS)
       {
           impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                           OMX_EventError, OMX_ErrorInsufficientResources,
                                           0 , NULL);
           OMX_LOGW("Idle transition failed\n");
           return -1;
       }
       usleep(OMX_TIMEOUT*1000);
   }
   return 0;
}

static int doStateWaitforResources2Loaded(AwOmxVdec* impl)
{
    OMX_U32 nTimeout = 0x0;
    OMX_PARAM_PORTDEFINITIONTYPE* inDef  = getPortDef(impl->m_InPort);
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(impl->m_OutPort);

    while (1)
    {
        //*Transition happens only when the ports are unpopulated
        if (!inDef->bPopulated && !outDef->bPopulated)
        {
            //* close decoder
            //* TODO.
            break;
        }
        else if (nTimeout++ > OMX_MAX_TIMEOUTS)
        {
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventError, OMX_ErrorInsufficientResources,
                                            0 , NULL);
            OMX_LOGW("Transition to loaded failed\n");
            return -1;
        }

        usleep(OMX_TIMEOUT*1000);
    }
    return 0;
}

static int doStateIdle2Loaded(AwOmxVdec* impl)
{
    return doStateWaitforResources2Loaded(impl);
}

static int doStateIdle2Executing(AwOmxVdec* impl)
{
    CEDARC_UNUSE(impl);
    return 0;
}

static int doStateLoaded2Idle(AwOmxVdec* impl)
{
    CEDARC_UNUSE(impl);
    //*TODO
    //secure
    return doStateWaitforResources2Idle(impl);

}

static int doStateLoaded2WaitforResources(AwOmxVdec* impl)
{
    CEDARC_UNUSE(impl);
    return 0;
}

static int doStateIdle2Pause(AwOmxVdec* impl)
{
    CEDARC_UNUSE(impl);
    return 0;
}

static int doStateExecuting2Idle(AwOmxVdec* impl)
{
    int ret = 0;
    OmxTryPostSem(impl->m_semKeepDrain);
    ret = impl->mDrainThreadHdl->suspend(impl->mDrainThreadHdl);
    OmxTryPostSem(impl->m_semKeepDecode);
    impl->mDecodeThreadHdl->suspend(impl->mDecodeThreadHdl);
    OmxTryPostSem(impl->m_semKeepSubmit);
    impl->mSubmitThreadHdl->suspend(impl->mSubmitThreadHdl);

    doFlushPortBuffer(impl->m_InPort);
    doFlushPortBuffer(impl->m_OutPort);

    OmxVdecoderClose(impl->pDec);
    impl->bHadInitDecoderFlag = OMX_FALSE;
    OmxVdecoderStandbyBuffer(impl->pDec);
    return doStateWaitforResources2Idle(impl);
}

static int doStateExecuting2Pause(AwOmxVdec* impl)
{
    return doStateIdle2Pause(impl);
}

static int doStatePause2Idle(AwOmxVdec* impl)
{
    return doStateExecuting2Idle(impl);
}

static int doStatePause2Executing(AwOmxVdec* impl)
{
    doFlushPortBuffer(impl->m_InPort);
    doFlushPortBuffer(impl->m_OutPort);

    return 0;
}

static void judgeThreadKeepGoing(AwOmxVdec* impl)
{
    OMX_PARAM_PORTDEFINITIONTYPE* inDef  = getPortDef(impl->m_InPort);
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(impl->m_OutPort);
    if (impl->m_state == OMX_StateExecuting &&
        inDef->bEnabled && outDef->bEnabled)
    {
        if(impl->bHadInitDecoderFlag)
        {
            OmxTryPostSem(impl->m_semKeepDecode);
            if((impl->m_InputNum - impl->m_OutputNum < MAX_INOUT_DIFF_NUM)
               || impl->bContinueToSubmitFlag)
                OmxTryPostSem(impl->m_semKeepSubmit);
            if(!impl->bResolutionChangeFlag)
                OmxTryPostSem(impl->m_semKeepDrain);
        }
        else
        {
            if(0 == OmxVdecoderPrepare(impl->pDec))
            {
                impl->bHadInitDecoderFlag = OMX_TRUE;
                postMessage(impl->mqMsgHandle, AW_OMX_MSG_START, 0);
            }
        }
    }
}

static void* onMessageReceived(void* pThreadData)
{
    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));
    AwOmxVdec *impl = (AwOmxVdec*)pThreadData;

    while(1)
    {
        if(CdcMessageQueueTryGetMessage(impl->mqMsgHandle, &msg, 5) == 0)
        {
            int cmd = msg.messageId;
            uintptr_t  cmdData = msg.params[0];
            logv("onMessageReceived, cmd:%s", cmdToString(cmd));
            switch(cmd){
                case AW_OMX_MSG_EMPTY_BUF:
                {
                    doEmptyThisBuffer(impl->m_InPort, (OMX_BUFFERHEADERTYPE*)cmdData);
                    OmxTryPostSem(impl->m_semSubmitData);
                    break;
                }
                case AW_OMX_MSG_FILL_BUF:
                {
                    doFillThisBuffer(impl->m_OutPort, (OMX_BUFFERHEADERTYPE*)cmdData);
                    break;
                }
                case AW_OMX_MSG_MARK_BUF:
                {
                    doSetPortMarkBuffer(impl->m_InPort, (OMX_MARKTYPE*)cmdData);
                    break;
                }
                case AW_OMX_MSG_SET_STATE:
                {
                    controlSetState(impl, (OMX_STATETYPE)cmdData);
                    break;
                }
                case AW_OMX_MSG_FLUSH:
                {
                    OmxTryPostSem(impl->m_semKeepDrain);
                    impl->mDrainThreadHdl->suspend(impl->mDrainThreadHdl);
                    OmxTryPostSem(impl->m_semKeepDecode);
                    impl->mDecodeThreadHdl->suspend(impl->mDecodeThreadHdl);
                    OmxTryPostSem(impl->m_semKeepSubmit);
                    impl->mSubmitThreadHdl->suspend(impl->mSubmitThreadHdl);

                    controlFlush(impl, (OMX_U32)cmdData);
                    impl->mSubmitThreadHdl->resume(impl->mSubmitThreadHdl);
                    impl->mDecodeThreadHdl->resume(impl->mDecodeThreadHdl);
                    impl->mDrainThreadHdl->resume(impl->mDrainThreadHdl);
                    break;
                }
                case AW_OMX_MSG_STOP_PORT:
                {
                    OmxTryPostSem(impl->m_semKeepDrain);
                    impl->mDrainThreadHdl->suspend(impl->mDrainThreadHdl);
                    controlStopPort(impl, (OMX_U32)cmdData);
                    break;
                }
                case AW_OMX_MSG_RESTART_PORT:
                {
                    controlRestartPort(impl, (OMX_U32)cmdData);
                    impl->mDrainThreadHdl->resume(impl->mDrainThreadHdl);
                    impl->bResolutionChangeFlag = OMX_FALSE;

                    break;
                }
                case AW_OMX_MSG_START:
                {
                    OMX_LOGD("********start***********");
                    impl->mSubmitThreadHdl->run(impl->mSubmitThreadHdl);
                    impl->mDecodeThreadHdl->run(impl->mDecodeThreadHdl);
                    impl->mDrainThreadHdl->run(impl->mDrainThreadHdl);
                    break;
                }
                case AW_OMX_MSG_QUIT:
                {
                    OMX_LOGD("********quit************");
                    OmxTryPostSem(impl->m_semFlushAllFrame);
                    OmxTryPostSem(impl->m_semKeepDrain);
                    impl->mDrainThreadHdl->requestExitAndWait(impl->mDrainThreadHdl, -1);
                    OmxTryPostSem(impl->m_semKeepDecode);
                    impl->mDecodeThreadHdl->requestExitAndWait(impl->mDecodeThreadHdl, -1);
                    OmxTryPostSem(impl->m_semKeepSubmit);
                    impl->mSubmitThreadHdl->requestExitAndWait(impl->mSubmitThreadHdl, -1);
                    goto EXIT_MSG;
                }
                 default:
                {
                    OMX_LOGW("Warning: maybe cmd(%d) is not defined!!", cmd);
                    break;
                }
            }
        }
        judgeThreadKeepGoing(impl);
    }

    EXIT_MSG:
        return (void*)OMX_ErrorNone;
}

static int decoderCallbackProcess(void* pSelf, int eMessageId, void* param)
{
    AwOmxVdec *impl = (AwOmxVdec*)pSelf;
    switch(eMessageId)
    {
        case AW_OMX_CB_EMPTY_BUFFER_DONE:
        {
            OMX_BUFFERHEADERTYPE*  pInBufHdr   = (OMX_BUFFERHEADERTYPE*)param;
            impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp, impl->m_pAppData, pInBufHdr);
            break;
        }
        case AW_OMX_CB_FILL_BUFFER_DONE:
        {
            OMX_BUFFERHEADERTYPE*  pOutBufHdr  = (OMX_BUFFERHEADERTYPE*)param;
            impl->m_Callbacks.FillBufferDone(&impl->base.mOmxComp, impl->m_pAppData, pOutBufHdr);
            break;
        }
        case AW_OMX_CB_EVENT_ERROR:
        {
            impl->bDecodeForceStopFlag = OMX_TRUE;
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventError, OMX_ErrorHardware, 0, NULL);
            break;
        }
        case AW_OMX_CB_NOTIFY_EOS:
        {
            impl->bVdrvNotifyEosFlag = OMX_TRUE;
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                        OMX_EventBufferFlag, 0x1, OMX_BUFFERFLAG_EOS, NULL);

            break;
        }
        case AW_OMX_CB_PORT_CHANGE:
        {
            OMX_LOGD("*******AW_OMX_CB_PORT_CHANGE");
            impl->bResolutionChangeFlag = OMX_TRUE;
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventPortSettingsChanged, 0x01, 0, NULL);
            break;
        }
        case AW_OMX_CB_NOTIFY_RECT:
        {
            memcpy(&(impl->m_VideoRect), param, sizeof(OMX_CONFIG_RECTTYPE));
            break;
        }
        case AW_OMX_CB_FLUSH_ALL_PIC:
        {
            impl->nNeedToFlushPicNum = *(OMX_S32*)param;
            OMX_LOGD("****need flush pics num:%ld", impl->nNeedToFlushPicNum);
            impl->bFlushingPicFlag = OMX_TRUE;
            OmxTimedWaitSem(impl->m_semFlushAllFrame, -1);
            OmxTryPostSem(impl->m_semKeepDrain);
            impl->mDrainThreadHdl->suspend(impl->mDrainThreadHdl);
            break;
        }
        case AW_OMX_CB_FININSH_REOPEN_VE:
        {
            impl->mDrainThreadHdl->resume(impl->mDrainThreadHdl);
        }
        case AW_OMX_CB_CONTINUE_SUBMIT:
        {
            impl->bContinueToSubmitFlag = OMX_TRUE;
            break;
        }
        default:
        {
            OMX_LOGW("Warning: maybe msg(%d) is not defined!!", eMessageId);
            break;
        }
    }
    return 0;

}

static void doSubmit(AwOmxVdec *impl)
{
    OMX_BUFFERHEADERTYPE*   pInBufHdr   = NULL;
    int ret = 0;
    pInBufHdr = doRequestPortBuffer(impl->m_InPort);
    if(pInBufHdr == NULL)
    {
        OmxTimedWaitSem(impl->m_semSubmitData, 2);
        return ;
    }
    if(pInBufHdr->nFilledLen > 0)
    {
        ret = OmxVdecoderSubmit(impl->pDec, pInBufHdr);
        if(ret != 0)
        {
            returnPortBuffer(impl->m_InPort);
            return ;
        }
    }

    if (pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
    {
        OmxVdecoderSetInputEos(impl->pDec, OMX_TRUE);
        OMX_LOGD("found eos flag in input data");
        pInBufHdr->nFlags = 0;
    }

    if (pInBufHdr->pMarkData)
    {
        impl->pMarkData            = pInBufHdr->pMarkData;
        impl->hMarkTargetComponent = pInBufHdr->hMarkTargetComponent;
    }

    if (pInBufHdr->hMarkTargetComponent == &impl->base.mOmxComp && pInBufHdr->pMarkData)
    {
        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                        OMX_EventMark, 0, 0, pInBufHdr->pMarkData);
    }
    impl->m_InputNum++;
    logv("gqy*****input num:%ld", impl->m_InputNum);
    impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp, impl->m_pAppData, pInBufHdr);
}

static int doDrain(AwOmxVdec *impl)
{
    OMX_BUFFERHEADERTYPE* pOutBufHdr  = OmxVdecoderDrain(impl->pDec);
    if(pOutBufHdr == NULL)
        return -1;
    if (impl->pMarkData != NULL && impl->hMarkTargetComponent != NULL)
    {
        pOutBufHdr->pMarkData = impl->pMarkData;
        pOutBufHdr->hMarkTargetComponent = impl->hMarkTargetComponent;
        impl->pMarkData = NULL;
        impl->hMarkTargetComponent = NULL;
    }
    logv("****FillBufferDone is called, pOutBufHdr[%p],"
         "pts[%lld], nAllocLen[%lu],"
         "nFilledLen[%lu], nOffset[%lu], nFlags[0x%lu],"
         "nOutputPortIndex[%lu], nInputPortIndex[%lu]",
         pOutBufHdr,
         pOutBufHdr->nTimeStamp, pOutBufHdr->nAllocLen,
         pOutBufHdr->nFilledLen, pOutBufHdr->nOffset, pOutBufHdr->nFlags,
         pOutBufHdr->nOutputPortIndex, pOutBufHdr->nInputPortIndex);
    impl->m_OutputNum++;
    logv("gqy*****output num:%ld", impl->m_OutputNum);

    impl->m_Callbacks.FillBufferDone(&impl->base.mOmxComp, impl->m_pAppData, pOutBufHdr);
    pOutBufHdr = NULL;
    return 0;
}

static OMX_BOOL submitThreadEntry(void* pThreadData)
{
    AwOmxVdec *impl = (AwOmxVdec*)pThreadData;

    OMX_PARAM_PORTDEFINITIONTYPE* inDef  = getPortDef(impl->m_InPort);
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(impl->m_OutPort);
    OMX_S32 diffNum = impl->m_InputNum - impl->m_OutputNum;
    logv("gqy****difffNum :%ld", diffNum);

    if (impl->m_state == OMX_StateExecuting &&
        inDef->bEnabled && outDef->bEnabled &&
        impl->bHadInitDecoderFlag &&
        (diffNum < MAX_INOUT_DIFF_NUM || impl->bContinueToSubmitFlag))
    {
        doSubmit(impl);
        if(impl->bContinueToSubmitFlag)
            impl->bContinueToSubmitFlag = OMX_FALSE;
    }
    else
    {
        impl->mWaitCount++;
        if(impl->mWaitCount > 20)
        {
            impl->bContinueToSubmitFlag = OMX_TRUE;
            impl->mWaitCount = 0;
        }
        OmxTimedWaitSem(impl->m_semKeepSubmit, 5);
    }
    return OMX_TRUE;
}
static OMX_BOOL decodeThreadEntry(void* pThreadData)
{
    AwOmxVdec *impl = (AwOmxVdec*)pThreadData;

    OMX_PARAM_PORTDEFINITIONTYPE* inDef  = getPortDef(impl->m_InPort);
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(impl->m_OutPort);

    if (impl->m_state == OMX_StateExecuting &&
        inDef->bEnabled && outDef->bEnabled &&
        impl->bHadInitDecoderFlag &&
        !impl->bDecodeForceStopFlag)
    {
        OmxVdecoderDecode(impl->pDec);
    }
    else
    {
        OmxTimedWaitSem(impl->m_semKeepDecode, 5);
    }

    return OMX_TRUE;
}

static OMX_BOOL drainThreadEntry(void* pThreadData)
{
    AwOmxVdec *impl = (AwOmxVdec*)pThreadData;
    OMX_PARAM_PORTDEFINITIONTYPE* inDef  = getPortDef(impl->m_InPort);
    OMX_PARAM_PORTDEFINITIONTYPE* outDef = getPortDef(impl->m_OutPort);
    if (impl->m_state == OMX_StateExecuting &&
        inDef->bEnabled && outDef->bEnabled &&
        impl->bHadInitDecoderFlag &&
        !impl->bResolutionChangeFlag)
    {
        if(doDrain(impl) == -1)
        {
            if(impl->bVdrvNotifyEosFlag)
            {
                if(0 == OmxVdecoderSetOutputEos(impl->pDec))
                    impl->bVdrvNotifyEosFlag = OMX_FALSE;
            }
            else if(impl->bFlushingPicFlag)
            {
                if(impl->nNeedToFlushPicNum <= 0)
                {
                    // here to set false for bFlushingPicFlag,
                    // because sometimes the callback run slow,
                    // then here will post sem angain
                    impl->bFlushingPicFlag = OMX_FALSE;
                    OmxTryPostSem(impl->m_semFlushAllFrame);
                }
                else
                {
                    //sometimes the flushPics will drain before the callback
                    impl->nNeedToFlushPicNum--;
                }
            }
        }
        else
        {
            if(impl->bFlushingPicFlag)
                impl->nNeedToFlushPicNum--;
        }
        OmxVdecoderReturnBuffer(impl->pDec);
    }
    else
    {
        OmxTimedWaitSem(impl->m_semKeepDrain, 5);
    }
    return OMX_TRUE;
}

static OMX_ERRORTYPE __AwOmxVdecInit(OMX_HANDLETYPE hComponent, OMX_STRING pComponentName)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    int           eRet;
    OMX_LOGD("++++++++++++++++++++++omx begin++++++++++++++++++");
    OMX_LOGD("name = %s", pComponentName);
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    impl->m_state               = OMX_StateLoaded;
    impl->m_eCompressionFormat  = OMX_VIDEO_CodingUnused;
    impl->bHadInitDecoderFlag   = OMX_FALSE;
    impl->bDecodeForceStopFlag  = OMX_FALSE;

    strncpy((char*)impl->m_cName, pComponentName, OMX_MAX_STRINGNAME_SIZE);
    err = omxSetRole(impl, NULL);
    if(err != OMX_ErrorNone)
        return err;

    impl->m_InPort = (AwOmxVdecPort*)malloc(sizeof(AwOmxVdecPort));
    memset(impl->m_InPort, 0, sizeof(AwOmxVdecPort));
    impl->m_OutPort = (AwOmxVdecPort*)malloc(sizeof(AwOmxVdecPort));
    memset(impl->m_InPort, 0, sizeof(AwOmxVdecPort));
    impl->m_InPort->mOmxCmp  = impl->base.mOmxComp;
    impl->m_OutPort->mOmxCmp = impl->base.mOmxComp;

    // Initialize component data structures to default values
    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sPortParam, OMX_PORT_PARAM_TYPE);
    impl->m_sPortParam.nPorts           = 0x2;
    impl->m_sPortParam.nStartPortNumber = 0x0;

    if(OMX_ErrorNone != AwOmxVdecInPortInit(impl->m_InPort, impl->m_eCompressionFormat) ||
       OMX_ErrorNone != AwOmxVdecOutPortInit(impl->m_OutPort))
       return OMX_ErrorInsufficientResources;

    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sPriorityMgmtType, OMX_PRIORITYMGMTTYPE);

    impl->pDec = OmxDecoderCreate(impl->m_InPort, impl->m_OutPort);
    if(impl->pDec == NULL)
        return OMX_ErrorInsufficientResources;
    impl->m_semFlushAllFrame = OmxCreateSem("flushSem",      0, 0, OMX_TRUE);
    impl->m_semKeepDecode    = OmxCreateSem("keepDecodeSem", 0, 0, OMX_FALSE);
    impl->m_semKeepSubmit    = OmxCreateSem("keepSubmitSem", 0, 0, OMX_FALSE);
    impl->m_semKeepDrain     = OmxCreateSem("keepDrainSem",  0, 0, OMX_FALSE);
    impl->m_semSubmitData    = OmxCreateSem("submitDataSem", 0, 0, OMX_FALSE);

    OmxVdecoderSetCallback(impl->pDec, decoderCallbackProcess, (void*)impl);

    impl->m_StateTransitTable[toIndex(OMX_StateLoaded)][toIndex(OMX_StateIdle)]
                           = doStateLoaded2Idle;
    impl->m_StateTransitTable[toIndex(OMX_StateLoaded)][toIndex(OMX_StateWaitForResources)]
                           = doStateLoaded2WaitforResources;
    impl->m_StateTransitTable[toIndex(OMX_StateIdle)][toIndex(OMX_StateLoaded)]
                           = doStateIdle2Loaded;
    impl->m_StateTransitTable[toIndex(OMX_StateIdle)][toIndex(OMX_StateExecuting)]
                           = doStateIdle2Executing;
    impl->m_StateTransitTable[toIndex(OMX_StateIdle)][toIndex(OMX_StatePause)]
                           = doStateIdle2Pause;
    impl->m_StateTransitTable[toIndex(OMX_StateExecuting)][toIndex(OMX_StateIdle)]
                           = doStateExecuting2Idle;
    impl->m_StateTransitTable[toIndex(OMX_StateExecuting)][toIndex(OMX_StatePause)]
                           = doStateExecuting2Pause;
    impl->m_StateTransitTable[toIndex(OMX_StatePause)][toIndex(OMX_StateIdle)]
                           = doStatePause2Idle;
    impl->m_StateTransitTable[toIndex(OMX_StatePause)][toIndex(OMX_StateExecuting)]
                           = doStatePause2Executing;
    impl->m_StateTransitTable[toIndex(OMX_StateWaitForResources)][toIndex(OMX_StateLoaded)]
                           = doStateWaitforResources2Loaded;
    impl->m_StateTransitTable[toIndex(OMX_StateWaitForResources)][toIndex(OMX_StateIdle)]
                           = doStateWaitforResources2Idle;
    impl->mqMsgHandle      = CdcMessageQueueCreate(64, "onMessageReceivedThread");

    impl->mSubmitThreadHdl = OmxCreateThread("submit", submitThreadEntry, impl);
    impl->mDecodeThreadHdl = OmxCreateThread("decode", decodeThreadEntry, impl);
    impl->mDrainThreadHdl  = OmxCreateThread("drain",  drainThreadEntry,  impl);
    if(impl->mSubmitThreadHdl == NULL ||
       impl->mDecodeThreadHdl == NULL ||
       impl->mDrainThreadHdl  == NULL)
    {
        return OMX_ErrorInsufficientResources;
    }

    eRet = pthread_create(&impl->m_msgId, NULL, onMessageReceived, impl);
    if( eRet || !impl->m_msgId )
    {
        return OMX_ErrorInsufficientResources;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE  __AwOmxVdecGetComponentVersion(
                                               OMX_IN OMX_HANDLETYPE hComponent,
                                               OMX_OUT OMX_STRING pComponentName,
                                               OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                                               OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                                               OMX_OUT OMX_UUIDTYPE* pComponentUUID)
{
    CEDARC_UNUSE(pComponentUUID);

    if (!hComponent || !pComponentName || !pComponentVersion || !pSpecVersion)
    {
        return OMX_ErrorBadParameter;
    }
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    strcpy((char*)pComponentName, (char*)impl->m_cName);

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

static OMX_ERRORTYPE __AwOmxVdecSendCommand(
                                      OMX_IN OMX_HANDLETYPE  hComponent,
                                      OMX_IN OMX_COMMANDTYPE eCmd,
                                      OMX_IN OMX_U32         uParam1,
                                      OMX_IN OMX_PTR         pCmdData)
{
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    if(impl->m_state == OMX_StateInvalid)
    {
        OMX_LOGE("ERROR: Send Command in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (eCmd == OMX_CommandMarkBuffer && pCmdData == NULL)
    {
        OMX_LOGE("ERROR: Send OMX_CommandMarkBuffer command but pCmdData invalid.");
        return OMX_ErrorBadParameter;
    }

    switch (eCmd)
    {
        logv(" COMPONENT_SEND_COMMAND: %d", eCmd);

        case OMX_CommandStateSet:
        {
            postMessage(impl->mqMsgHandle,AW_OMX_MSG_SET_STATE,(uintptr_t)uParam1);
            break;
        }
        case OMX_CommandFlush:
        {
            if ((int)uParam1 > 1 && (int)uParam1 != -1)
            {
                OMX_LOGE("Error: Send OMX_CommandFlush command but uParam1 invalid.");
                return OMX_ErrorBadPortIndex;
            }
            postMessage(impl->mqMsgHandle,AW_OMX_MSG_FLUSH,(uintptr_t)uParam1);
            break;
        }
        case OMX_CommandPortDisable:
        {
            postMessage(impl->mqMsgHandle,AW_OMX_MSG_STOP_PORT,(uintptr_t)uParam1);
            break;
        }
        case OMX_CommandPortEnable:
        {
            postMessage(impl->mqMsgHandle,AW_OMX_MSG_RESTART_PORT,(uintptr_t)uParam1);
            break;
        }
        case OMX_CommandMarkBuffer:
        {
             if (uParam1 > 0)
            {
                OMX_LOGE("Error: Send OMX_CommandMarkBuffer command but uParam1 invalid.");
                return OMX_ErrorBadPortIndex;
            }
            postMessage(impl->mqMsgHandle,AW_OMX_MSG_MARK_BUF,(uintptr_t)pCmdData);
            break;
        }
        default:
        {
            OMX_LOGW("ignore other command[0x%x]",eCmd);
            return OMX_ErrorBadParameter;
        }
    }

    return OMX_ErrorNone;
}


static OMX_ERRORTYPE __AwOmxVdecGetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                                        OMX_IN OMX_INDEXTYPE  eParamIndex,
                                                        OMX_INOUT OMX_PTR     pParamData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    if(hComponent == NULL)
        return OMX_ErrorBadParameter;

    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    logv("eParamIndex = 0x%x", eParamIndex);
    if(impl->m_state == OMX_StateInvalid)
    {
        OMX_LOGW("Get Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if(pParamData == NULL)
    {
        OMX_LOGW("Get Param in Invalid pParamData \n");
        return OMX_ErrorBadParameter;
    }

    switch(eParamIndex)
    {
        case OMX_IndexParamVideoInit:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoInit");
            memcpy(pParamData, &impl->m_sPortParam, sizeof(OMX_PORT_PARAM_TYPE));
            break;
        }

        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *value = (OMX_PARAM_PORTDEFINITIONTYPE *)pParamData;
            AwOmxVdecPort* port = getPort(impl, value->nPortIndex);
            if(port == NULL)
            {
                OMX_LOGW("error. pParamData->nPortIndex=[%lu]", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
            OmxVdecoderGetFormat(impl->pDec);
            eError = AwOmxVdecPortGetDefinitioin(port, value);
            break;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *value = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)pParamData;
            AwOmxVdecPort* port = getPort(impl, value->nPortIndex);
            if(port == NULL)
            {
                OMX_LOGW("error. pParamData->nPortIndex=[%lu]", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
            eError = AwOmxVdecPortGetFormat(port, value);
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamStandardComponentRole");
            OMX_PARAM_COMPONENTROLETYPE* comp_role;

            comp_role                    = (OMX_PARAM_COMPONENTROLETYPE *) pParamData;
            comp_role->nVersion.nVersion = OMX_SPEC_VERSION;
            comp_role->nSize             = sizeof(*comp_role);

            strncpy((char*)comp_role->cRole, (const char*)impl->m_cRole, OMX_MAX_STRINGNAME_SIZE);
            break;
        }

        case OMX_IndexParamPriorityMgmt:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamPriorityMgmt");
            memcpy(pParamData, &impl->m_sPriorityMgmtType, sizeof(OMX_PRIORITYMGMTTYPE));
            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamCompBufferSupplier");
            OMX_PARAM_BUFFERSUPPLIERTYPE* value  = (OMX_PARAM_BUFFERSUPPLIERTYPE*)pParamData;
            AwOmxVdecPort* port = getPort(impl, value->nPortIndex);
            if(port == NULL)
            {
                OMX_LOGW("error. pParamData->nPortIndex=[%lu]", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
            eError = AwOmxVdecPortGetBufferSupplier(port, value);
            break;
        }
        case OMX_IndexParamImageInit:
        case OMX_IndexParamAudioInit:
        case OMX_IndexParamOtherInit:
        {
            logv(" COMPONENT_GET_PARAMETER: %d", eParamIndex);
            OMX_PORT_PARAM_TYPE *portParamType = (OMX_PORT_PARAM_TYPE *) pParamData;
            portParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            portParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            portParamType->nPorts            = 0;
            portParamType->nStartPortNumber  = 0;
            break;
        }
        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoProfileLevelQuerySupported");
            OMX_VIDEO_PARAM_PROFILELEVELTYPE* value = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*)pParamData;
            eError = AwOmxVdecPortGetProfileLevel(impl->m_InPort, value);
            break;
        }
        default:
        {
            eError = OmxVdecoderGetExtPara(impl->pDec,
                                           (AW_VIDEO_EXTENSIONS_INDEXTYPE)eParamIndex,
                                            pParamData);
            break;
        }
    }

    return eError;
}

static OMX_ERRORTYPE __AwOmxVdecSetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                                        OMX_IN OMX_INDEXTYPE eParamIndex,
                                                        OMX_IN OMX_PTR pParamData)
{
    OMX_ERRORTYPE         eError      = OMX_ErrorNone;
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    logv("eParamIndex = 0x%x",  eParamIndex);
    //if(impl->m_state != OMX_StateLoaded)
    if(impl->m_state == OMX_StateInvalid)
    {
        OMX_LOGW("Set Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if(pParamData == NULL)
    {
        OMX_LOGW("Get Param in Invalid pParamData \n");
        return OMX_ErrorBadParameter;
    }

    switch(eParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *value = (OMX_PARAM_PORTDEFINITIONTYPE *)pParamData;
            AwOmxVdecPort* port = getPort(impl, value->nPortIndex);
            if(port == NULL)
            {
                OMX_LOGW("error. pParamData->nPortIndex=[%lu]", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
            eError = AwOmxVdecPortSetDefinitioin(port, value);
            if(isInputPort(port))
            {
                impl->m_VideoRect.nLeft   = 0;
                impl->m_VideoRect.nTop    = 0;
                impl->m_VideoRect.nWidth  = port->m_sPortDefType.format.video.nFrameWidth;
                impl->m_VideoRect.nHeight = port->m_sPortDefType.format.video.nFrameHeight;
            }
            else
            {
                port->m_sPortDefType.format.video.nStride
                = port->m_sPortDefType.format.video.nFrameWidth;
                port->m_sPortDefType.format.video.nSliceHeight
                = port->m_sPortDefType.format.video.nFrameHeight;

                //check some key parameter
                OMX_U32 buffer_size = (port->m_sPortDefType.format.video.nFrameWidth* \
                  port->m_sPortDefType.format.video.nFrameHeight)*3/2;
                if(buffer_size != port->m_sPortDefType.nBufferSize)
                {
                    OMX_LOGW("set_parameter, OMX_IndexParamPortDefinition, OutPortDef :"
                    "change nBufferSize[%d] to [%d] to suit frame width[%d] and height[%d]",
                    (int)port->m_sPortDefType.nBufferSize,
                    (int)buffer_size,
                    (int)port->m_sPortDefType.format.video.nFrameWidth,
                    (int)port->m_sPortDefType.format.video.nFrameHeight);
                    port->m_sPortDefType.nBufferSize = buffer_size;
                }
                OmxVdecoderSetExtBufNum(impl->pDec, port->mExtraBufferNum);
            }
            break;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *value = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)pParamData;
            AwOmxVdecPort* port = getPort(impl, value->nPortIndex);
            if(port == NULL)
            {
                OMX_LOGW("error. pParamData->nPortIndex=[%lu]", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
            eError = AwOmxVdecPortSetFormat(port, value);
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *comp_role;
            comp_role = (OMX_PARAM_COMPONENTROLETYPE *) pParamData;
            logv("set_parameter: OMX_IndexParamStandardComponentRole %s\n", comp_role->cRole);

            if(OMX_StateLoaded == impl->m_state)
            {
                OMX_LOGW("Set Parameter called in valid state");
            }
            else
            {
                OMX_LOGW("Set Parameter called in Invalid State\n");
                return OMX_ErrorIncorrectStateOperation;
            }

            eError = omxSetRole(impl, comp_role->cRole);
            break;
        }

        case OMX_IndexParamPriorityMgmt:
        {
            logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamPriorityMgmt");
            if(impl->m_state != OMX_StateLoaded)
            {
                logv("Set Parameter called in Invalid State\n");
                return OMX_ErrorIncorrectStateOperation;
            }
            OMX_PRIORITYMGMTTYPE *priorityMgmtype = (OMX_PRIORITYMGMTTYPE*) pParamData;
            memcpy(&impl->m_sPriorityMgmtType, priorityMgmtype, sizeof(OMX_PRIORITYMGMTTYPE));
            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamCompBufferSupplier");
            OMX_PARAM_BUFFERSUPPLIERTYPE* value  = (OMX_PARAM_BUFFERSUPPLIERTYPE*)pParamData;
            AwOmxVdecPort* port = getPort(impl, value->nPortIndex);
            if(port == NULL)
            {
                OMX_LOGW("error. pParamData->nPortIndex=[%lu]", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
            eError = AwOmxVdecPortSetBufferSupplier(port, value);
            break;
        }
        default:
        {

            eError = OmxVdecoderSetExtPara(impl->pDec,
                                           (AW_VIDEO_EXTENSIONS_INDEXTYPE)eParamIndex,
                                            pParamData);
            break;
        }
    }

    return eError;
}

static OMX_ERRORTYPE __AwOmxVdecGetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                                    OMX_IN OMX_INDEXTYPE eConfigIndex,
                                                    OMX_INOUT OMX_PTR pConfigData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    logv("index = %d", eConfigIndex);

    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    if (impl->m_state == OMX_StateInvalid)
    {
        logv("get_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    switch (eConfigIndex)
    {
        case OMX_IndexConfigCommonOutputCrop:
        {
            OMX_CONFIG_RECTTYPE *pRect = (OMX_CONFIG_RECTTYPE *)pConfigData;

            OMX_LOGW("+++++ get display crop: top[%d],left[%d],width[%d],height[%d]",
                  (int)impl->m_VideoRect.nTop,(int)impl->m_VideoRect.nLeft,
                  (int)impl->m_VideoRect.nWidth,(int)impl->m_VideoRect.nHeight);

            if(impl->m_VideoRect.nHeight != 0)
            {
                memcpy(pRect,&(impl->m_VideoRect),sizeof(OMX_CONFIG_RECTTYPE));
            }
            else
            {
                OMX_LOGW("the crop is invalid!");
                eError = OMX_ErrorUnsupportedIndex;
            }
            break;
        }
        default:
        {
            eError = OmxVdecoderGetExtConfig(impl->pDec,
                                           (AW_VIDEO_EXTENSIONS_INDEXTYPE)eConfigIndex,
                                            pConfigData);
            break;
        }
    }

    return eError;
}


static OMX_ERRORTYPE __AwOmxVdecSetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                                   OMX_IN OMX_INDEXTYPE eConfigIndex,
                                                   OMX_IN OMX_PTR pConfigData)
{

    logv("index = %d", eConfigIndex);

    OMX_ERRORTYPE eError = OMX_ErrorNone;
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    if(impl->m_state == OMX_StateInvalid)
    {
        logv("set_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (impl->m_state == OMX_StateExecuting)
    {
        logv("set_config: Ignore in Executing state\n");
        return OMX_ErrorNone;
    }

    switch(eConfigIndex)
    {
        default:
        {
            eError = OmxVdecoderSetExtConfig(impl->pDec,
                                           (AW_VIDEO_EXTENSIONS_INDEXTYPE)eConfigIndex,
                                            pConfigData);
            break;
        }
    }
    return eError;
}

static OMX_ERRORTYPE __AwOmxVdecGetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                                              OMX_IN OMX_STRING pParamName,
                                                              OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    logv("param name = %s", pParamName);
    unsigned int  nIndex;
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;
    if(hComponent == NULL)
        return OMX_ErrorBadParameter;

    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    if(impl->m_state == OMX_StateInvalid)
    {
        logv("Get Extension Index in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    for(nIndex = 0; nIndex < sizeof(sVideoDecCustomParams)/sizeof(VIDDEC_CUSTOM_PARAM); nIndex++)
    {
        if(strcmp((char *)pParamName, (char *)&(sVideoDecCustomParams[nIndex].cCustomParamName))
           == 0)
        {
            *pIndexType = sVideoDecCustomParams[nIndex].nCustomParamIndex;
            eError = OMX_ErrorNone;
            break;
        }
    }

    return eError;
}


static OMX_ERRORTYPE __AwOmxVdecGetState(OMX_IN OMX_HANDLETYPE hComponent,
                                                  OMX_OUT OMX_STATETYPE* pState)
{
    if(hComponent == NULL || pState == NULL)
        return OMX_ErrorBadParameter;

    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    *pState = impl->m_state;
    logv("COMPONENT_GET_STATE, state: %s", OmxState2String(impl->m_state));
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE __AwOmxVdecComponentTunnelRequest(
                                                 OMX_IN    OMX_HANDLETYPE       hComponent,
                                                 OMX_IN    OMX_U32              uPort,
                                                 OMX_IN    OMX_HANDLETYPE       pPeerComponent,
                                                 OMX_IN    OMX_U32              uPeerPort,
                                                 OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    OMX_LOGW("Error: component_tunnel_request Not Implemented\n");

    CEDARC_UNUSE(hComponent);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pPeerComponent);
    CEDARC_UNUSE(uPeerPort);
    CEDARC_UNUSE(pTunnelSetup);
    return OMX_ErrorNotImplemented;
}

static OMX_ERRORTYPE __AwOmxVdecUseBuffer(OMX_IN OMX_HANDLETYPE    hComponent,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE**  ppBufferHdr,
                                          OMX_IN    OMX_U32                 nPortIndex,
                                          OMX_IN    OMX_PTR                 pAppPrivate,
                                          OMX_IN    OMX_U32                 nSizeBytes,
                                          OMX_IN    OMX_U8*                 pBuffer)
{
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    OMX_LOGD("PortIndex[%d], nSizeBytes[%d], pBuffer[%p]",
          (int)nPortIndex, (int)nSizeBytes, pBuffer);
    if(hComponent == NULL || ppBufferHdr == NULL || pBuffer == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    AwOmxVdecPort* port = getPort(impl,nPortIndex);
    if(port == NULL)
        return OMX_ErrorBadParameter;

    OMX_PARAM_PORTDEFINITIONTYPE* def = getPortDef(port);

    if (impl->m_state!=OMX_StateLoaded
        && impl->m_state!=OMX_StateWaitForResources
        && def->bEnabled)
    {
        OMX_LOGW("pPortDef[%d]->bEnabled=%d, m_state=%s, Can't allocate_buffer!",
             (int)nPortIndex, def->bEnabled, OmxState2String(impl->m_state));
        return OMX_ErrorIncorrectStateOperation;
    }
    OMX_LOGD("pPortDef[%d]->bEnabled=%d, m_state=%s, can allocate_buffer.",
         (int)nPortIndex, def->bEnabled, OmxState2String(impl->m_state));
    return AwOmxVdecPortPopBuffer(port, ppBufferHdr, pAppPrivate, nSizeBytes, pBuffer, OMX_FALSE);

}

static OMX_ERRORTYPE __AwOmxVdecAllocateBuffer(OMX_IN OMX_HANDLETYPE  hComponent,
                                              OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                              OMX_IN    OMX_U32                nPortIndex,
                                              OMX_IN    OMX_PTR                pAppPrivate,
                                              OMX_IN    OMX_U32                nSizeBytes)
{
    OMX_LOGD("kay: __AwOmxVdecAllocateBuffer");
    if(hComponent == NULL || ppBufferHdr == NULL)
        return OMX_ErrorBadParameter;

    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    AwOmxVdecPort* port = getPort(impl,nPortIndex);
    if(port == NULL)
        return OMX_ErrorBadParameter;

    OMX_PARAM_PORTDEFINITIONTYPE* def = getPortDef(port);

    if (impl->m_state!=OMX_StateLoaded
        && impl->m_state!=OMX_StateWaitForResources
        && def->bEnabled)
    {
        OMX_LOGW("pPortDef[%d]->bEnabled=%d, m_state=%s, Can't allocate_buffer!",
             (int)nPortIndex, def->bEnabled, OmxState2String(impl->m_state));
        return OMX_ErrorIncorrectStateOperation;
    }
    OMX_LOGD("kay: __AwOmxVdecAllocateBuffer 2 ");
    OMX_U8* pBuffer = OmxVdecoderAllocatePortBuffer(impl->pDec, port, nSizeBytes);
    OMX_LOGD("kay: __AwOmxVdecAllocateBuffer 3");
    return AwOmxVdecPortPopBuffer(port, ppBufferHdr, pAppPrivate, nSizeBytes, pBuffer, OMX_TRUE);
}


static OMX_ERRORTYPE __AwOmxVdecFreeBuffer(OMX_IN  OMX_HANDLETYPE        hComponent,
                                                    OMX_IN  OMX_U32               nPortIndex,
                                                    OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    if(hComponent == NULL || pBufferHdr == NULL)
    {
        return OMX_ErrorBadParameter;
    }
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);


    AwOmxVdecPort* port = getPort(impl,nPortIndex);
    if(port == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if(port->bAllocBySelfFlags)
        OmxVdecoderFreePortBuffer(impl->pDec, port, pBufferHdr->pBuffer);

    eError = AwOmxVdecPortFreeBuffer(port, pBufferHdr);
    if(eError == OMX_ErrorNone &&
        !isInputPort(port) &&
        !port->m_sPortDefType.bPopulated)
    {
        OmxVdecoderStandbyBuffer(impl->pDec);
    }
    return eError;
}

static OMX_ERRORTYPE __AwOmxVdecEmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                                OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    logv("***emptyThisBuffer: pts = %lld , videoFormat = %d, pBufferHdr = %p",
         pBufferHdr->nTimeStamp,
         impl->m_eCompressionFormat, pBufferHdr);

    if(hComponent == NULL || pBufferHdr == NULL)
        return OMX_ErrorBadParameter;

    AwOmxVdecPort* port = getPort(impl, pBufferHdr->nInputPortIndex);
    if(port == NULL)
    {
        OMX_LOGE("failed to get the prot with index: %lu", pBufferHdr->nInputPortIndex);
        return OMX_ErrorUnsupportedIndex;
    }

    OMX_PARAM_PORTDEFINITIONTYPE* def = getPortDef(port);
    if (!def->bEnabled)
        return OMX_ErrorIncorrectStateOperation;

    if (pBufferHdr->nInputPortIndex != 0x0  || pBufferHdr->nOutputPortIndex != OMX_NOPORT)
        return OMX_ErrorBadPortIndex;

    if (impl->m_state != OMX_StateExecuting && impl->m_state != OMX_StatePause)
        return OMX_ErrorIncorrectStateOperation;

    postMessage(impl->mqMsgHandle, AW_OMX_MSG_EMPTY_BUF, (uintptr_t)pBufferHdr);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE __AwOmxVdecFillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    if(hComponent == NULL || pBufferHdr == NULL)
        return OMX_ErrorBadParameter;
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_PARAM_PORTDEFINITIONTYPE* def = getPortDef(impl->m_OutPort);

    if (!def->bEnabled)
        return OMX_ErrorIncorrectStateOperation;

    if (pBufferHdr->nOutputPortIndex != 0x1 || pBufferHdr->nInputPortIndex != OMX_NOPORT)
        return OMX_ErrorBadPortIndex;

    if (impl->m_state != OMX_StateExecuting && impl->m_state != OMX_StatePause)
        return OMX_ErrorIncorrectStateOperation;

    postMessage(impl->mqMsgHandle, AW_OMX_MSG_FILL_BUF, (uintptr_t)pBufferHdr);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE __AwOmxVdecSetCallbacks(OMX_IN  OMX_HANDLETYPE hComponent,
                                             OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
                                             OMX_IN  OMX_PTR pAppData)
{
    OMX_LOGD("===== vdec set callbacks");
    if(hComponent == NULL || pCallbacks == NULL || pAppData == NULL)
        return OMX_ErrorBadParameter;
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    memcpy(&impl->m_Callbacks, pCallbacks, sizeof(OMX_CALLBACKTYPE));
    impl->m_pAppData = pAppData;
    OMX_ERRORTYPE err1 = AwOmxVdecPortSetCallbacks(impl->m_InPort, pCallbacks, pAppData);
    OMX_ERRORTYPE err2 = AwOmxVdecPortSetCallbacks(impl->m_OutPort, pCallbacks, pAppData);
    if(err1 == OMX_ErrorNone && err2 == OMX_ErrorNone)
        return OMX_ErrorNone;
    else
        return OMX_ErrorBadParameter;
}


static OMX_ERRORTYPE __AwOmxVdecComponentDeinit(OMX_IN OMX_HANDLETYPE hComponent)
{
    logd("Omx Vdec Component Deinit begin");
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    postMessage(impl->mqMsgHandle, AW_OMX_MSG_QUIT, 0);
    pthread_join(impl->m_msgId, (void**)&eError);
    logd("threads exit!");
    return eError;
}

static OMX_ERRORTYPE  __AwOmxVdecUseEGLimage(OMX_IN OMX_HANDLETYPE            hComponent,
                                             OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                             OMX_IN OMX_U32                   uPort,
                                             OMX_IN OMX_PTR                   pAppData,
                                             OMX_IN void*                     pEglImage)
{
    OMX_LOGW("Error : use_EGL_image:  Not Implemented \n");

    CEDARC_UNUSE(hComponent);
    CEDARC_UNUSE(ppBufferHdr);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pAppData);
    CEDARC_UNUSE(pEglImage);

    return OMX_ErrorNotImplemented;
}


static OMX_ERRORTYPE  __AwOmxVdecComponentRoleEnum(OMX_IN  OMX_HANDLETYPE hComponent,
                                                                  OMX_OUT OMX_U8*        pRole,
                                                                  OMX_IN  OMX_U32        uIndex)
{
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    if(0 != uIndex && !pRole)
    {
        return OMX_ErrorNoMore;
    }
    else
    {
        strncpy((char*)pRole, (char *)impl->m_cRole, OMX_MAX_STRINGNAME_SIZE);
        return OMX_ErrorNone;
    }
}

static void __AwOmxVdecDestroy(OMX_HANDLETYPE hComponent)
{
    AwOmxVdec *impl = (AwOmxVdec*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OmxDestroySem(&impl->m_semFlushAllFrame);
    OmxDestroySem(&impl->m_semKeepSubmit);
    OmxDestroySem(&impl->m_semKeepDecode);
    OmxDestroySem(&impl->m_semKeepDrain);
    OmxDestroySem(&impl->m_semSubmitData);
    if(impl->mqMsgHandle != NULL)
        CdcMessageQueueDestroy(impl->mqMsgHandle);
    if(impl->mSubmitThreadHdl != NULL)
        OmxDestroyThread(&impl->mSubmitThreadHdl);
    if(impl->mDecodeThreadHdl != NULL)
        OmxDestroyThread(&impl->mDecodeThreadHdl);
    if(impl->mDrainThreadHdl != NULL)
        OmxDestroyThread(&impl->mDrainThreadHdl);

    freeBuffersAllocBySelf(impl, impl->m_InPort);
    freeBuffersAllocBySelf(impl, impl->m_OutPort);
    AwOmxVdecPortDeinit(impl->m_InPort);
    AwOmxVdecPortDeinit(impl->m_OutPort);
    OmxDestroyDecoder(impl->pDec);
    free(impl);
    impl = NULL;
    OMX_LOGD("++++++++++++++++++++++omx end++++++++++++++++++");
}

static OMX_COMPONENTTYPE* __AwOmxVdecComponentCreate()
{
    AwOmxVdec *impl = (AwOmxVdec*)malloc(sizeof(AwOmxVdec));
    memset(impl, 0x00, sizeof(AwOmxVdec));

    impl->base.destroy = &__AwOmxVdecDestroy;
    impl->base.init    = &__AwOmxVdecInit;

    impl->base.mOmxComp.nSize = sizeof(OMX_COMPONENTTYPE);
    impl->base.mOmxComp.nVersion.nVersion = OMX_SPEC_VERSION;
    impl->base.mOmxComp.pComponentPrivate = (OMX_PTR)impl;

    impl->base.mOmxComp.GetComponentVersion    = &__AwOmxVdecGetComponentVersion;
    impl->base.mOmxComp.SendCommand            = &__AwOmxVdecSendCommand;
    impl->base.mOmxComp.GetParameter           = &__AwOmxVdecGetParameter;
    impl->base.mOmxComp.SetParameter           = &__AwOmxVdecSetParameter;
    impl->base.mOmxComp.GetConfig              = &__AwOmxVdecGetConfig;
    impl->base.mOmxComp.SetConfig              = &__AwOmxVdecSetConfig;
    impl->base.mOmxComp.GetExtensionIndex      = &__AwOmxVdecGetExtensionIndex;
    impl->base.mOmxComp.GetState               = &__AwOmxVdecGetState;
    impl->base.mOmxComp.ComponentTunnelRequest = &__AwOmxVdecComponentTunnelRequest;
    impl->base.mOmxComp.UseBuffer              = &__AwOmxVdecUseBuffer;
    impl->base.mOmxComp.AllocateBuffer         = &__AwOmxVdecAllocateBuffer;
    impl->base.mOmxComp.FreeBuffer             = &__AwOmxVdecFreeBuffer;
    impl->base.mOmxComp.EmptyThisBuffer        = &__AwOmxVdecEmptyThisBuffer;
    impl->base.mOmxComp.FillThisBuffer         = &__AwOmxVdecFillThisBuffer;
    impl->base.mOmxComp.SetCallbacks           = &__AwOmxVdecSetCallbacks;
    impl->base.mOmxComp.ComponentDeInit        = &__AwOmxVdecComponentDeinit;
    impl->base.mOmxComp.UseEGLImage            = &__AwOmxVdecUseEGLimage;
    impl->base.mOmxComp.ComponentRoleEnum      = &__AwOmxVdecComponentRoleEnum;

    return &impl->base.mOmxComp;
}

void* AwOmxComponentCreate()
{
    return __AwOmxVdecComponentCreate();
}
