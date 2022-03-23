/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : awHDCPModule.cpp
* Description :
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : first version
*
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "AwHDCPModule"
#include <utils/Log.h>

#include <dlfcn.h>
#include <utils/Mutex.h>
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <binder/IServiceManager.h>
#include <media/IHDCP.h>
#include <media/IMediaPlayerService.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaErrors.h>

#include "AwHDCPModule.h"
using namespace android;
//namespace android {
class AwHDCPModule {
public:
    AwHDCPModule();
    ~AwHDCPModule();
    uint32_t decrypt(const uint8_t privateData[16],
            const uint8_t *msgIn, uint8_t *msgOut, uint32_t msgLen, int streamType);//HdcpStreamType
private:
    sp<IHDCP> mHDCP;
    status_t makeHDCP();
    AwHDCPModule(const AwHDCPModule&);
    AwHDCPModule& operator=(const AwHDCPModule&);
};

status_t AwHDCPModule::makeHDCP() {
    sp<IServiceManager> mServiceManager= defaultServiceManager();
    sp<IBinder> mBinder = mServiceManager->getService(String16("media.player"));
    sp<IMediaPlayerService> mMediaPlayerService = interface_cast<IMediaPlayerService>(mBinder);
    CHECK(mMediaPlayerService != NULL);

    mHDCP = mMediaPlayerService->makeHDCP(false);

    if (mHDCP == NULL) {
        return ERROR_UNSUPPORTED;
    }
    mHDCP->setObserver(NULL);
    return OK;
}

AwHDCPModule::AwHDCPModule() {
    makeHDCP();
}

AwHDCPModule::~AwHDCPModule() {
    if (mHDCP != NULL) {
        mHDCP->setObserver(NULL);
        mHDCP.clear();
    }
}
uint32_t AwHDCPModule::decrypt(const uint8_t privateData[16],
        const uint8_t *msgIn, uint8_t *msgOut, uint32_t msgLen, int streamType) {

    /*unuse*/
    (void)streamType;

    ALOGV("decrypt");
    if(mHDCP == NULL) {
        return 0x1;
    }

    uint32_t trackIndex = ((privateData[1] & 0x06) >> 1) << 30 |
                 privateData[2] << 22 |
                 ((privateData[3] & 0xFE) >> 1) << 15 |
                 privateData[4] << 7 |
                 ((privateData[5] & 0xFE) >> 1);

    uint64_t outputCTR = ((privateData[7] & 0x1E) >> 1) << 28 |
            privateData[8] << 20 |
            ((privateData[9] & 0xFE) >> 1) << 13 |
            privateData[10] << 5 |
            ((privateData[11] & 0xF1) >> 3);

    outputCTR = outputCTR << 32;

    outputCTR |= ((privateData[11] & 0x07) >> 1) << 30 |
            privateData[12] << 22 |
            ((privateData[13] & 0xFE) >> 1) << 15 |
            privateData[14] << 7 |
            ((privateData[15] & 0xFE) >> 1);

    status_t err = mHDCP->decrypt(
            msgIn, msgLen,
            trackIndex  /* streamCTR */,
            outputCTR,
            msgOut);

    return err;
}

//}//namespace android

int32_t HDCP_Init(void **handle)
{
    *handle = NULL;
    AwHDCPModule *hdcp = new AwHDCPModule();
    *handle = hdcp;
    return 0;
}

void HDCP_Deinit(void *handle)
{
    AwHDCPModule *hdcp = static_cast<AwHDCPModule *>(handle);
    if(hdcp)
    {
        delete hdcp;
    }
}
uint32_t HDCP_Decrypt(void *handle, const uint8_t privateData[16],
        const uint8_t *msgIn, uint8_t *msgOut, uint32_t msgLen, int streamType)
{
    AwHDCPModule *hdcp = static_cast<AwHDCPModule *>(handle);
    if (hdcp != NULL)
    {
        return hdcp->decrypt(privateData, msgIn, msgOut, msgLen, streamType);
    }
    return 1;
}
