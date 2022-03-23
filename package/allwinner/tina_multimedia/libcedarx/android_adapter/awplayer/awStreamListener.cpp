/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : awStreamListener.cpp
* Description :
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : first version
*
*/

#define LOG_TAG "awStreamListener"
#include <cdx_log.h>
#include "awStreamListener.h"
#include <media/stagefright/foundation/AMessage.h>

awStreamListener::awStreamListener(const sp<IStreamSource> &source, ALooper::handler_id id,
                                    size_t numBuffers, size_t bufferSize)
    : mStreamSource(source),
      mTargetHandlerID(id),
      mSendDataNotification(true),
      mReceiveEOS(false),
      mEOS(false),
      mAvailableSize(0)
{
    mStreamSource->setListener(this);

    mNumBuffers = numBuffers;
    mBufferSize = bufferSize;

    mMemoryDealer = new MemoryDealer(mNumBuffers * mBufferSize);
    for (size_t i = 0; i < mNumBuffers; ++i)
    {
        sp<IMemory> mem = mMemoryDealer->allocate(mBufferSize);
        CDX_CHECK(mem != NULL);

        mBuffersVector.push(mem);
    }
    mStreamSource->setBuffers(mBuffersVector);
}

void awStreamListener::start()
{
    logv("awStreamListener::start");
    for (size_t i = 0; i < mNumBuffers; ++i)
    {
        if(mReceiveEOS) {
            break;
        }
        mStreamSource->onBufferAvailable(i);
    }
}

void awStreamListener::stop()
{
    //If not set listener as NULL,
    //BnStreamSource has a ref of this handle,
    //while this handle has a ref of BnStreamSource.
    //This is a dead loop which causes memeroy leak.
    logv("awStreamListener::stop");
    Mutex::Autolock autoLock(mAutoLock);
    mEOS = true;
    mStreamSource->setListener(NULL);
}

void awStreamListener::queueBuffer(size_t nIndex, size_t nSize)
{
    Mutex::Autolock autoLock(mAutoLock);
    QueueEntry mQueueEntry;
    mQueueEntry.mIsCommand = false;
    mQueueEntry.mIndex     = nIndex;
    mQueueEntry.mSize      = nSize;
    mQueueEntry.mOffset    = 0;

    mAvailableSize += nSize;

    mQueueList.push_back(mQueueEntry);

//    if (mSendDataNotification) {
//        mSendDataNotification = false;
//
//        if (mTargetHandlerID != 0) {
//            (new AMessage(kWhatMoreDataQueued, mTargetHandlerID))->post();
//        }
//    }
}
void awStreamListener::clearBuffer() {
    Mutex::Autolock autoLock(mAutoLock);

    logv("queue size %d", mQueueList.size());
    QueueEntry *pQueueEntry = NULL;
    while(mQueueList.size() > 0 && !mReceiveEOS) {
        pQueueEntry = &*mQueueList.begin();
        CDX_CHECK(pQueueEntry != NULL);
        mStreamSource->onBufferAvailable(pQueueEntry->mIndex);
        mQueueList.erase(mQueueList.begin());
        pQueueEntry = NULL;
    }
    mAvailableSize = 0;
    CDX_CHECK(mQueueList.empty() || mReceiveEOS);
}

void awStreamListener::issueCommand(
        Command cmd, bool synchronous, const sp<AMessage> &extra) {
    CDX_CHECK(!synchronous);
    CEDARX_UNUSE(synchronous);

    logv("awStreamListener::issueCommand");
    Mutex::Autolock autoLock(mAutoLock);
    QueueEntry mQueueEntry;
    mQueueEntry.mIsCommand = true;
    mQueueEntry.mCommand = cmd;
    mQueueEntry.mExtra = extra;

    mQueueList.push_back(mQueueEntry);
    if(mQueueEntry.mCommand == EOS) {
        mReceiveEOS = true;
    }

//    if (mSendDataNotification) {
//        mSendDataNotification = false;
//
//        if (mTargetHandlerID != 0) {
//            (new AMessage(kWhatMoreDataQueued, mTargetHandlerID))->post();
//        }
//    }
}

ssize_t awStreamListener::read(void *data, size_t size, sp<AMessage> *pExtra)
{
    CDX_CHECK(size > 0u);

    pExtra->clear();

    Mutex::Autolock autoLock(mAutoLock);

    if (mEOS)
    {
        return 0;
    }

    if ((mQueueList.empty() || mAvailableSize < size) && !mReceiveEOS) {
        mSendDataNotification = true;
        return -EAGAIN;
    }
    if(mAvailableSize < size)
    {
        size = mAvailableSize;
    }

    size_t copiedSize = 0;
    while(copiedSize < size) {
        if(mQueueList.empty()) {
            logw("Have not gotten enough data, but queue is empty. %u vs %u",
                    mAvailableSize, size);
            break;
        }

        QueueEntry *pQueueEntry = &*mQueueList.begin();

        if (pQueueEntry->mIsCommand) {
            logv("awStreamListener mCommand:%d",pQueueEntry->mCommand);
            switch (pQueueEntry->mCommand) {
                case EOS:
                {
                    mQueueList.erase(mQueueList.begin());
                    pQueueEntry = NULL;
                    mEOS = true;
                    return copiedSize;
                }

                case DISCONTINUITY:
                {
                    *pExtra = pQueueEntry->mExtra;

                    mQueueList.erase(mQueueList.begin());
                    pQueueEntry = NULL;
                    logw("DISCONTINUITY");
                    continue;
                }

                default:
                    CDX_TRESPASS();
                    break;
            }
        }

        size_t copySize = pQueueEntry->mSize;
        if (copySize > size - copiedSize)
        {
            copySize = size - copiedSize;
        }

        memcpy((uint8_t *)data + copiedSize,
           (const uint8_t *)mBuffersVector.editItemAt(pQueueEntry->mIndex)->pointer()
            + pQueueEntry->mOffset,
           copySize);

        pQueueEntry->mOffset += copySize;
        pQueueEntry->mSize -= copySize;
        mAvailableSize     -= copySize;
        copiedSize         += copySize;
        if (pQueueEntry->mSize == 0) {
            mStreamSource->onBufferAvailable(pQueueEntry->mIndex);
            mQueueList.erase(mQueueList.begin());
            pQueueEntry = NULL;
        }
    }
    CDX_CHECK(copiedSize == size);
    return copiedSize;
}

ssize_t awStreamListener::copy(void *data, size_t size, sp<AMessage> *pExtra)
{
    CDX_CHECK(size > 0u);

    pExtra->clear();

    Mutex::Autolock autoLock(mAutoLock);

    if (mEOS)
    {
        return 0;
    }

    if ((mQueueList.empty() || mAvailableSize < size) && !mReceiveEOS) {
        mSendDataNotification = true;
        return -EAGAIN;
    }
    if(mAvailableSize < size)
    {
        size = mAvailableSize;
    }

    size_t copiedSize = 0;
    while(copiedSize < size) {
        if(mQueueList.empty()) {
            logw("Have not gotten enough data, but queue is empty. %u vs %u",
                    mAvailableSize, size);
            break;
        }
        List<QueueEntry>::iterator it = mQueueList.begin();
        QueueEntry *pQueueEntry = &*it;

        if (pQueueEntry->mIsCommand) {
            logv("awStreamListener mCommand:%d",pQueueEntry->mCommand);
            switch (pQueueEntry->mCommand) {
                case EOS:
                {
                    mQueueList.erase(mQueueList.begin());
                    pQueueEntry = NULL;
                    mEOS = true;
                    return copiedSize;
                }

                case DISCONTINUITY:
                {
                    *pExtra = pQueueEntry->mExtra;

                    mQueueList.erase(mQueueList.begin());
                    pQueueEntry = NULL;
                    logw("DISCONTINUITY");
                    continue;
                }

                default:
                    CDX_TRESPASS();
                    break;
            }
        }

        size_t copySize = pQueueEntry->mSize;
        if (copySize > size - copiedSize)
        {
            copySize = size - copiedSize;
        }

        memcpy((uint8_t *)data + copiedSize,
           (const uint8_t *)mBuffersVector.editItemAt(pQueueEntry->mIndex)->pointer(),
           copySize);

        copiedSize += copySize;
        ++it;
    }
    CDX_CHECK(copiedSize == size);
    return copiedSize;
}
size_t awStreamListener::getCachedSize()
{
    return mAvailableSize;
}
bool awStreamListener::isReceiveEOS()
{
    return mReceiveEOS;
}
