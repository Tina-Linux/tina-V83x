/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : AwOMXPlugin.h
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : init the design code
*/

#ifndef AW_OMX_PLUGIN_H_

#define AW_OMX_PLUGIN_H_

#include <OMXPluginBase.h>

namespace android
{

struct AwOMXPlugin : public OMXPluginBase
{
    AwOMXPlugin();
    virtual ~AwOMXPlugin();

    virtual OMX_ERRORTYPE makeComponentInstance(const char *pName,
                                                const OMX_CALLBACKTYPE *pCallbacks,
                                                OMX_PTR pAppData,
                                                OMX_COMPONENTTYPE **ppComponent);
    virtual OMX_ERRORTYPE destroyComponentInstance(OMX_COMPONENTTYPE *pComponent);
    virtual OMX_ERRORTYPE enumerateComponents(OMX_STRING pName, size_t nSize, OMX_U32 uIndex);
    virtual OMX_ERRORTYPE getRolesOfComponent(const char *pName, Vector<String8> *pRoles);

private:
    void *mOmxLibHandle;

    typedef OMX_ERRORTYPE (*OmxInitFunc)();
    typedef OMX_ERRORTYPE (*OmxDeinitFunc)();
    typedef OMX_ERRORTYPE (*OmxComponentNameEnumFunc)(OMX_STRING, OMX_U32, OMX_U32);
    typedef OMX_ERRORTYPE (*OmxGetHandleFunc)(OMX_HANDLETYPE *,
                                              OMX_STRING, OMX_PTR, OMX_CALLBACKTYPE *);
    typedef OMX_ERRORTYPE (*OmxFreeHandleFunc)(OMX_HANDLETYPE *);
    typedef OMX_ERRORTYPE (*OmxGetRolesOfComponentFunc)(OMX_STRING, OMX_U32 *, OMX_U8 **);

    OmxInitFunc                    mInitFunc;
    OmxDeinitFunc                  mDeinitFunc;
    OmxComponentNameEnumFunc       mComponentNameEnumFunc;
    OmxGetHandleFunc               mGetHandleFunc;
    OmxFreeHandleFunc              mFreeHandleFunc;
    OmxGetRolesOfComponentFunc     mGetRolesOfComponentHandleFunc;

    AwOMXPlugin(const AwOMXPlugin &);
    AwOMXPlugin &operator=(const AwOMXPlugin &);
};

}  // namespace android

#endif  // QCOM_OMX_PLUGIN_H_
