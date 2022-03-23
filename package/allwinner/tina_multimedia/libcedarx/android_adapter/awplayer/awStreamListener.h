/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : awStreamListener.h
* Description :
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : first version
*
*/


#ifndef AW_STREAM_LISTENER_H
#define AW_STREAM_LISTENER_H

#include <media/IStreamSource.h>
#include <media/stagefright/foundation/ALooper.h>
#include <binder/MemoryDealer.h>

using namespace android;

#define kNumBuffers (16)
#define kBufferSize (4*1024)

struct awStreamListener : public BnStreamListener
{
    awStreamListener(const sp<IStreamSource> &source,
                            ALooper::handler_id targetID,
                            size_t numBuffers = kNumBuffers,
                            size_t bufferSize = kBufferSize);

    virtual void queueBuffer(size_t nIndex, size_t nSize);
    virtual void issueCommand(Command cmd, bool synchronous, const sp<AMessage> &extra);

    void start();
    void stop();
    void clearBuffer();

    ssize_t read(void *data, size_t size, sp<AMessage> *extra);
    ssize_t copy(void *data, size_t size, sp<AMessage> *extra);
    size_t getCachedSize();
    size_t getCacheCapacity(){return mNumBuffers*mBufferSize;};
    bool isReceiveEOS();

private:
    struct QueueEntry
    {
        size_t mOffset;
        size_t mSize;
        size_t mIndex;

        bool mIsCommand;

        enum Command mCommand;
        sp<AMessage> mExtra;
    };

    Mutex mAutoLock;

    sp<MemoryDealer>     mMemoryDealer;
    sp<IStreamSource>    mStreamSource;
    ALooper::handler_id  mTargetHandlerID;
    Vector<sp<IMemory> > mBuffersVector;
    List<QueueEntry>     mQueueList;
    bool                 mSendDataNotification;
    bool                 mReceiveEOS;
    bool                 mEOS;
    size_t                  mNumBuffers;
    size_t                  mBufferSize;
    size_t               mAvailableSize;

    DISALLOW_EVIL_CONSTRUCTORS(awStreamListener);
};

#endif // AW_STREAM_LISTENER_H
