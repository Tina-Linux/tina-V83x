#include <stdlib.h>
#include <stdio.h>
#include "aw_omx_component.h"
#include "log.h"

/************************************************************************/
/*               COMPONENT INTERFACE                                    */
/************************************************************************/
void AwOmxComponentDestrory(OMX_HANDLETYPE pHComp)
{
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;

    if(pThis)
    {
        // call the init function
        pThis->destroy(pHComp);
    }

    return;
}

OMX_ERRORTYPE AwOmxComponentInit(OMX_IN OMX_HANDLETYPE pHComp, OMX_IN OMX_STRING pComponentName)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logd("OMXCORE: aw_omx_component_init %x\n",(unsigned)pHComp);

    if(pThis)
    {
        // call the init function
        eReturn = pThis->init(pHComp, pComponentName);

        if(eReturn != OMX_ErrorNone)
        {
            //  in case of error, please destruct the component created
            AwOmxComponentDestrory(pHComp);
        }
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentDeinit(OMX_IN OMX_HANDLETYPE pHComp)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logd("OMXCORE: aw_omx_component_deinit %x\n",(unsigned)pHComp);

    if(pThis)
    {
        // call the deinit fuction first
        OMX_STATETYPE eState;
        //pThis->mOmxComp.GetState(pHComp,&eState);
        //logv("Calling FreeHandle in eState %d \n", eState);


        eReturn = pThis->mOmxComp.ComponentDeInit(pHComp);
        // destroy the component.
        // This will Caught SIGSEGV
        //AwOmxComponentDestrory(pHComp);
    }

    return eReturn;
}


OMX_ERRORTYPE AwOmxComponentGetVersion(OMX_IN OMX_HANDLETYPE pHComp,
                                           OMX_OUT OMX_STRING          pComponentName,
                                           OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                                           OMX_OUT OMX_VERSIONTYPE*      pSpecVersion,
                                           OMX_OUT OMX_UUIDTYPE*       pComponentUUID)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;

    logd("OMXCORE: aw_omx_component_get_version %x, %s , %x\n",
                 (unsigned)pHComp,pComponentName,(unsigned)pComponentVersion);
    if(pThis)
    {
        eReturn = pThis->mOmxComp.GetComponentVersion(pHComp,pComponentName,
                                               pComponentVersion,pSpecVersion,pComponentUUID);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentSendCommand(OMX_IN OMX_HANDLETYPE pHComp,
                                            OMX_IN OMX_COMMANDTYPE  eCmd,
                                            OMX_IN OMX_U32       uParam1,
                                            OMX_IN OMX_PTR      pCmdData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logd("OMXCORE: aw_omx_component_send_command %x, %d , %d\n",
                 (unsigned)pHComp,(unsigned)eCmd,(unsigned)uParam1);

    if(pThis)
    {
        eReturn = pThis->mOmxComp.SendCommand(pHComp,eCmd,uParam1,pCmdData);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentGetParameter(OMX_IN OMX_HANDLETYPE     pHComp,
                                             OMX_IN OMX_INDEXTYPE eParamIndex,
                                             OMX_INOUT OMX_PTR     pParamData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logv("OMXCORE: aw_omx_component_get_parameter %x, %x , %d\n",
                (unsigned)pHComp,(unsigned)pParamData,eParamIndex);

    if(pThis)
    {
        eReturn = pThis->mOmxComp.GetParameter(pHComp,eParamIndex,pParamData);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentSetParameter(OMX_IN OMX_HANDLETYPE     pHComp,
                                             OMX_IN OMX_INDEXTYPE eParamIndex,
                                             OMX_IN OMX_PTR        pParamData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;

    logv("OMXCORE: aw_omx_component_set_parameter %x, %x , %d\n",
        (unsigned)pHComp,(unsigned)pParamData,eParamIndex);

    if(pThis)
    {
        eReturn = pThis->mOmxComp.SetParameter(pHComp,eParamIndex,pParamData);
    }
    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentGetConfig(OMX_IN OMX_HANDLETYPE      pHComp,
                                          OMX_IN OMX_INDEXTYPE eConfigIndex,
                                          OMX_INOUT OMX_PTR     pConfigData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logd("OMXCORE: aw_omx_component_get_config %x\n",(unsigned)pHComp);

    if(pThis)
    {
        eReturn = pThis->mOmxComp.GetConfig(pHComp, eConfigIndex, pConfigData);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentSetConfig(OMX_IN OMX_HANDLETYPE      pHComp,
                                          OMX_IN OMX_INDEXTYPE eConfigIndex,
                                          OMX_IN OMX_PTR        pConfigData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logd("OMXCORE: aw_omx_component_set_config %x\n",(unsigned)pHComp);

    if(pThis)
    {
        eReturn = pThis->mOmxComp.SetConfig(pHComp, eConfigIndex, pConfigData);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentGetExtensionIndex(OMX_IN OMX_HANDLETYPE      pHComp,
                                                   OMX_IN OMX_STRING      pParamName,
                                                   OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    if(pThis )
    {
        eReturn = pThis->mOmxComp.GetExtensionIndex(pHComp,pParamName,pIndexType);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentGetState(OMX_IN OMX_HANDLETYPE  pHComp,
                                         OMX_OUT OMX_STATETYPE* pState)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logv("OMXCORE: aw_omx_component_get_state %x\n",(unsigned)pHComp);

    if(pThis)
    {
        eReturn = pThis->mOmxComp.GetState(pHComp, pState);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentTunnelRequest(OMX_IN OMX_HANDLETYPE                pHComp,
                                              OMX_IN OMX_U32                        uPort,
                                              OMX_IN OMX_HANDLETYPE        pPeerComponent,
                                              OMX_IN OMX_U32                    uPeerPort,
                                              OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    logv("Error: aw_omx_component_tunnel_request Not Implemented\n");

    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pPeerComponent);
    CEDARC_UNUSE(uPeerPort);
    CEDARC_UNUSE(pTunnelSetup);

    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE AwOmxComponentUseBuffer(OMX_IN OMX_HANDLETYPE                pHComp,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                          OMX_IN OMX_U32                        uPort,
                                          OMX_IN OMX_PTR                     pAppData,
                                          OMX_IN OMX_U32                       uBytes,
                                          OMX_IN OMX_U8*                      pBuffer)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;

    logv("OMXCORE: aw_omx_component_use_buffer %x\n",
                (unsigned)pHComp);

    if(pThis)
    {
        eReturn = pThis->mOmxComp.UseBuffer(pHComp, ppBufferHdr, uPort, pAppData, uBytes, pBuffer);
    }

    return eReturn;
}

// aw_omx_component_allocate_buffer  -- API Call
OMX_ERRORTYPE AwOmxComponentAllocateBuffer(OMX_IN OMX_HANDLETYPE                pHComp,
                                               OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                               OMX_IN OMX_U32                        uPort,
                                               OMX_IN OMX_PTR                     pAppData,
                                               OMX_IN OMX_U32                       uBytes)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logv("OMXCORE: aw_omx_component_allocate_buffer %x, %x , %d\n",
                (unsigned)pHComp,(unsigned)ppBufferHdr,(unsigned)uPort);

    if(pThis)
    {
        eReturn = pThis->mOmxComp.AllocateBuffer(pHComp,ppBufferHdr,uPort,pAppData,uBytes);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentFreeBuffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                           OMX_IN OMX_U32                 uPort,
                                           OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logv("OMXCORE: aw_omx_component_free_buffer[%d] %x, %x\n",
                (unsigned)uPort, (unsigned)pHComp, (unsigned)pBuffer);

    if(pThis)
    {
        eReturn = pThis->mOmxComp.FreeBuffer(pHComp,uPort,pBuffer);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentEmptyThisBuffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                                 OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;

    if(pThis)
    {
        eReturn = pThis->mOmxComp.EmptyThisBuffer(pHComp,pBuffer);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentFillThisBuffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                                OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;

    if(pThis)
    {
        eReturn = pThis->mOmxComp.FillThisBuffer(pHComp,pBuffer);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentSetCallbacks(OMX_IN OMX_HANDLETYPE        pHComp,
                                             OMX_IN OMX_CALLBACKTYPE* pCallbacks,
                                             OMX_IN OMX_PTR             pAppData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logd("OMXCORE: aw_omx_component_set_callbacks %x, %x , %x\n",
                 (unsigned)pHComp,(unsigned)pCallbacks,(unsigned)pAppData);

    if(pThis)
    {
        eReturn = pThis->mOmxComp.SetCallbacks(pHComp,pCallbacks,pAppData);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentUseEGLImage(OMX_IN OMX_HANDLETYPE                pHComp,
                                             OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                             OMX_IN OMX_U32                        uPort,
                                             OMX_IN OMX_PTR                     pAppData,
                                             OMX_IN void*                      pEglImage)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logv("OMXCORE: aw_omx_component_use_EGL_image %x, %x , %d\n",
                (unsigned)pHComp,(unsigned)ppBufferHdr,(unsigned)uPort);
    if(pThis)
    {
        eReturn = pThis->mOmxComp.UseEGLImage(pHComp,ppBufferHdr,uPort,pAppData,pEglImage);
    }

    return eReturn;
}

OMX_ERRORTYPE AwOmxComponentRoleEnum(OMX_IN OMX_HANDLETYPE pHComp,
                                         OMX_OUT OMX_U8*        pRole,
                                         OMX_IN OMX_U32        uIndex)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    AwOmxComponentType *pThis = (AwOmxComponentType *)pHComp;
    logv("OMXCORE: aw_omx_component_role_enum %x, %x , %d\n",
                (unsigned)pHComp,(unsigned)pRole,(unsigned)uIndex);

    if(pThis)
    {
        eReturn = pThis->mOmxComp.ComponentRoleEnum(pHComp,pRole,uIndex);
    }

    return eReturn;
}
