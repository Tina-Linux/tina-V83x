#ifndef AW_OMX_COMPONENT_H
#define AW_OMX_COMPONENT_H

#include "OMX_Core.h"
#include "OMX_Component.h"
#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OMX_SPEC_VERSION       0x00000101 // OMX Version

typedef struct _AwOmxComponentType
{
    OMX_COMPONENTTYPE mOmxComp;

    // destroy the component
    void (*destroy)(OMX_HANDLETYPE pCmp_handle);

    // Initialize the component after creation
    OMX_ERRORTYPE (*init)(OMX_HANDLETYPE pCmp_handle, OMX_IN OMX_STRING pComponentName);

}AwOmxComponentType;

// this interface should impliment in omx_vdec/omx_venc
OMX_API void* AwOmxComponentCreate();

void AwOmxComponentDestrory(OMX_HANDLETYPE mOmxComp);

OMX_ERRORTYPE AwOmxComponentInit(OMX_IN OMX_HANDLETYPE pHComp, OMX_IN OMX_STRING pComponentName);

OMX_ERRORTYPE AwOmxComponentGetVersion(OMX_IN OMX_HANDLETYPE               pHComp,
                                           OMX_OUT OMX_STRING          pComponentName,
                                           OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                                           OMX_OUT OMX_VERSIONTYPE*      pSpecVersion,
                                           OMX_OUT OMX_UUIDTYPE*       pComponentUUID);

OMX_ERRORTYPE AwOmxComponentSendCommand(OMX_IN OMX_HANDLETYPE pHComp,
                                            OMX_IN OMX_COMMANDTYPE  eCmd,
                                            OMX_IN OMX_U32       uParam1,
                                            OMX_IN OMX_PTR      pCmdData);

OMX_ERRORTYPE AwOmxComponentGetParameter(OMX_IN OMX_HANDLETYPE     pHComp,
                                             OMX_IN OMX_INDEXTYPE eParamIndex,
                                             OMX_INOUT OMX_PTR     pParamData);

OMX_ERRORTYPE AwOmxComponentSetParameter(OMX_IN OMX_HANDLETYPE     pHComp,
                                             OMX_IN OMX_INDEXTYPE eParamIndex,
                                             OMX_IN OMX_PTR        pParamData);

OMX_ERRORTYPE AwOmxComponentGetConfig(OMX_IN OMX_HANDLETYPE      pHComp,
                                          OMX_IN OMX_INDEXTYPE eConfigIndex,
                                          OMX_INOUT OMX_PTR     pConfigData);

OMX_ERRORTYPE AwOmxComponentSetConfig(OMX_IN OMX_HANDLETYPE      pHComp,
                                          OMX_IN OMX_INDEXTYPE eConfigIndex,
                                          OMX_IN OMX_PTR        pConfigData);

OMX_ERRORTYPE AwOmxComponentGetExtensionIndex(OMX_IN OMX_HANDLETYPE      pHComp,
                                                   OMX_IN OMX_STRING      pParamName,
                                                   OMX_OUT OMX_INDEXTYPE* pIndexType);

OMX_ERRORTYPE AwOmxComponentGetState(OMX_IN OMX_HANDLETYPE  pHComp,
                                         OMX_OUT OMX_STATETYPE* pState);

OMX_ERRORTYPE AwOmxComponentTunnelRequest(OMX_IN OMX_HANDLETYPE                pHComp,
                                              OMX_IN OMX_U32                        uPort,
                                              OMX_IN OMX_HANDLETYPE        pPeerComponent,
                                              OMX_IN OMX_U32                    uPeerPort,
                                              OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup);

OMX_ERRORTYPE AwOmxComponentUseBuffer(OMX_IN OMX_HANDLETYPE                pHComp,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                          OMX_IN OMX_U32                        uPort,
                                          OMX_IN OMX_PTR                     pAppData,
                                          OMX_IN OMX_U32                       uBytes,
                                          OMX_IN OMX_U8*                      pBuffer);

// aw_omx_component_allocate_buffer  -- API Call
OMX_ERRORTYPE AwOmxComponentAllocateBuffer(OMX_IN OMX_HANDLETYPE                pHComp,
                                               OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                               OMX_IN OMX_U32                        uPort,
                                               OMX_IN OMX_PTR                     pAppData,
                                               OMX_IN OMX_U32                       uBytes);

OMX_ERRORTYPE AwOmxComponentFreeBuffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                           OMX_IN OMX_U32                 uPort,
                                           OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

OMX_ERRORTYPE AwOmxComponentEmptyThisBuffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                                 OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

OMX_ERRORTYPE AwOmxComponentFillThisBuffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                                OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

OMX_ERRORTYPE AwOmxComponentSetCallbacks(OMX_IN OMX_HANDLETYPE        pHComp,
                                             OMX_IN OMX_CALLBACKTYPE* pCallbacks,
                                             OMX_IN OMX_PTR             pAppData);

OMX_ERRORTYPE AwOmxComponentDeinit(OMX_IN OMX_HANDLETYPE pHComp);

OMX_ERRORTYPE AwOmxComponentUseEGLImage(OMX_IN OMX_HANDLETYPE                pHComp,
                                             OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                             OMX_IN OMX_U32                        uPort,
                                             OMX_IN OMX_PTR                     pAppData,
                                             OMX_IN void*                      pEglImage);

OMX_ERRORTYPE AwOmxComponentRoleEnum(OMX_IN OMX_HANDLETYPE pHComp,
                                         OMX_OUT OMX_U8*        pRole,
                                         OMX_IN OMX_U32        uIndex);


#ifdef __cplusplus
}
#endif

#endif
