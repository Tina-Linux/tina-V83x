/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : awStreamSource.cpp
* Description :
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : first version
*
*/

#define LOG_TAG "awStreamingSource"
#include "cdx_log.h"
#include <CdxAtomic.h>
#include "awStreamingSource.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>

#define SAVE_FILE    (0)

#if SAVE_FILE
    FILE    *file = NULL;
#endif

using namespace android;

struct awStreamingSource : public RefBase
{
    awStreamingSource(const sp<IStreamSource> &source);

    virtual void start(size_t numBuffer, size_t bufferSize);
    virtual void stop();
    virtual void clearBuffer();
    virtual status_t feedMoreTSData() {return OK;};

    //virtual sp<MetaData> getFormat(bool audio) { return NULL;};
    virtual status_t dequeueAccessUnit(bool audio, sp<ABuffer> *accessUnit)
      {CEDARX_UNUSE(audio);CEDARX_UNUSE(accessUnit);return OK;};
    virtual int dequeueAccessData(char *accessData, int size);
    virtual int copyAccessData(char *accessData, int size);
    virtual size_t getCachedSize(){return mStreamListener->getCachedSize();};
    virtual size_t getCacheCapacity(){return mStreamListener->getCacheCapacity();};
    virtual int isReceiveEOS(){return mStreamListener->isReceiveEOS();};

    virtual status_t getDuration(int64_t *durationUs) {
        CEDARX_UNUSE(durationUs);
        return -1;
    }

    virtual status_t seekTo(int64_t seekTimeUs) {
        CEDARX_UNUSE(seekTimeUs);
        return -1;
    }

    virtual bool isSeekable() {
        return false;
    }

protected:
    virtual ~awStreamingSource();

private:
    sp<IStreamSource> mSource;
    //status_t mFinalResult;
    sp<awStreamListener> mStreamListener;
    Mutex mLock;
    DISALLOW_EVIL_CONSTRUCTORS(awStreamingSource);
};


awStreamingSource::awStreamingSource(const sp<IStreamSource> &source)
    : mSource(source)
      //mFinalResult(OK)
{
    logv("awStreamingSource Constructor");
}

awStreamingSource::~awStreamingSource() {
    logv("awStreamingSource Constructor ~De");
}

void awStreamingSource::start(size_t numBuffer, size_t bufferSize)
{
    mStreamListener = new awStreamListener(mSource, 0, numBuffer, bufferSize);

    mStreamListener->start();
}

void awStreamingSource::stop()
{
    mStreamListener->stop();
}

void awStreamingSource::clearBuffer()
{
    mStreamListener->clearBuffer();
}


int awStreamingSource::dequeueAccessData(char *accessData, int size)
{
    sp<AMessage> extra;
    ssize_t n;

    n = mStreamListener->read(accessData, size , &extra);
    if (n == 0) {
        logi("input data EOS reached.");
        //mFinalResult = ERROR_END_OF_STREAM;
    }
    if(n == -EAGAIN)
    {
        loge("should not be here!!");
    }
    return n;
}

int awStreamingSource::copyAccessData(char *accessData, int size)
{
    sp<AMessage> extra;
    ssize_t n;

    n = mStreamListener->copy(accessData, size , &extra);
    if (n == 0) {
        logi("input data EOS reached.");
        //mFinalResult = ERROR_END_OF_STREAM;
    }
    else if(n == -EAGAIN)
    {
        loge("should not be here!!");
    }
    return n;
}

////////////////////////////////////////////////////////////////////////

#define ProbeDataLen (2*1024)

enum CdxStreamStatus
{
    STREAM_IDLE,
    STREAM_CONNECTING,
    STREAM_SEEKING,
    STREAM_READING,
};

typedef struct StreamingSource StreamingSource;
struct StreamingSource
{
    CdxStreamT base;
    cdx_uint32 attribute;
    enum CdxStreamStatus status;
    cdx_int32 ioState;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    cdx_int32 forceStop;
    CdxStreamProbeDataT probeData;

    sp<awStreamingSource> mAwStreamingSource;
};

static cdx_int32 PrepareThread(CdxStreamT *stream)
{
    CDX_LOGI("PrepareThread start");
    StreamingSource *streamingSource = (StreamingSource *)stream;
    pthread_mutex_lock(&streamingSource->lock);
    if(streamingSource->forceStop)
    {
        pthread_mutex_unlock(&streamingSource->lock);
        return -1;
    }
    streamingSource->status = STREAM_CONNECTING;
    pthread_mutex_unlock(&streamingSource->lock);

    cdx_uint32 ret;
    while((ret = streamingSource->mAwStreamingSource->getCachedSize()) < ProbeDataLen)
    {
        //CDX_LOGI("getCachedSize(%d)", ret);
        if(streamingSource->forceStop || streamingSource->mAwStreamingSource->isReceiveEOS())
        {
            goto _exit;
        }
        usleep(20000);
    }
    streamingSource->mAwStreamingSource->copyAccessData(streamingSource->probeData.buf,
                                         ProbeDataLen);
    streamingSource->ioState = CDX_IO_STATE_OK;
    pthread_mutex_lock(&streamingSource->lock);
    streamingSource->status = STREAM_IDLE;
    pthread_mutex_unlock(&streamingSource->lock);
    pthread_cond_signal(&streamingSource->cond);
    CDX_LOGI("PrepareThread succeed");
    return 0;
_exit:
    streamingSource->ioState = CDX_IO_STATE_ERROR;
    pthread_mutex_lock(&streamingSource->lock);
    streamingSource->status = STREAM_IDLE;
    pthread_mutex_unlock(&streamingSource->lock);
    pthread_cond_signal(&streamingSource->cond);
    CDX_LOGI("PrepareThread end");
    return -1;
}

cdx_int32 StreamingSourceGetIOState(CdxStreamT *stream)
{
    StreamingSource *streamingSource = (StreamingSource *)stream;
    return streamingSource->ioState;
}

static CdxStreamProbeDataT *StreamingSourceGetProbeData(CdxStreamT *stream)
{
    StreamingSource *streamingSource = (StreamingSource *)stream;
    return &streamingSource->probeData;
}

cdx_uint32 StreamingSourceAttribute(CdxStreamT *stream)
{
    StreamingSource *streamingSource = (StreamingSource *)stream;
    return streamingSource->attribute;
}
static cdx_int32 GetCacheState(StreamingSource *streamingSource,
                             struct StreamCacheStateS *cacheState)
{
    memset(cacheState, 0, sizeof(struct StreamCacheStateS));
    cacheState->nCacheCapacity = (cdx_int32)streamingSource->mAwStreamingSource->getCacheCapacity();
    cacheState->nCacheSize = (cdx_int32)streamingSource->mAwStreamingSource->getCachedSize();
    return 0;
}

static cdx_int32 StreamingSourceForceStop(CdxStreamT *stream)
{
    StreamingSource *streamingSource = (StreamingSource *)stream;
    pthread_mutex_lock(&streamingSource->lock);
    streamingSource->forceStop = 1;
    while(streamingSource->status != STREAM_IDLE)
    {
        pthread_cond_wait(&streamingSource->cond, &streamingSource->lock);
    }
    pthread_mutex_unlock(&streamingSource->lock);
    return 0;
}
cdx_int32 StreamingSourceControl(CdxStreamT *stream, cdx_int32 cmd, void *param)
{
    StreamingSource *streamingSource = (StreamingSource *)stream;
    switch(cmd)
    {
        case STREAM_CMD_GET_CACHESTATE:
            return GetCacheState(streamingSource, (struct StreamCacheStateS *)param);
        case STREAM_CMD_SET_FORCESTOP:
            return StreamingSourceForceStop(stream);
        default:
            CDX_LOGW("not implement cmd(%d)", cmd);
            break;
    }
    return -1;
}
cdx_int32 StreamingSourceClose(CdxStreamT *stream)
{
    StreamingSource *streamingSource = (StreamingSource *)stream;
    StreamingSourceForceStop(stream);
    if(streamingSource->probeData.buf)
    {
        free(streamingSource->probeData.buf);
    }
    streamingSource->mAwStreamingSource->stop();
    streamingSource->mAwStreamingSource.clear();
#if SAVE_FILE
    if (file)
    {
        fclose(file);
        file = NULL;
    }
#endif
    pthread_mutex_destroy(&streamingSource->lock);
    pthread_cond_destroy(&streamingSource->cond);
    free(streamingSource);
    return 0;
}

cdx_int32 StreamingSourceRead(CdxStreamT *stream, void *buf, cdx_uint32 len)
{
    StreamingSource *streamingSource = (StreamingSource *)stream;
    pthread_mutex_lock(&streamingSource->lock);
    if(streamingSource->forceStop)
    {
        pthread_mutex_unlock(&streamingSource->lock);
        return -1;
    }
    streamingSource->status = STREAM_READING;
    pthread_mutex_unlock(&streamingSource->lock);

    char *data = (char *)buf;
    int ret = 0;
    cdx_uint32 cachedSize;
    while((cachedSize = streamingSource->mAwStreamingSource->getCachedSize()) < len)
    {
        if(streamingSource->forceStop)
        {
            CDX_LOGD("streamingSource->exitFlag");
            ret = -1;
            goto _exit;
        }
        if(streamingSource->mAwStreamingSource->isReceiveEOS())
        {
            len = cachedSize;
            break;
        }
        usleep(5000);
    }
    ret = streamingSource->mAwStreamingSource->dequeueAccessData(data, len);
    if(ret == 0)
    {
        streamingSource->ioState = CDX_IO_STATE_EOS;
    }
    else if(ret < 0)
    {
        CDX_LOGE("dequeueAccessData fail");
    }
#if SAVE_FILE
    if(ret > 0)
    {
        if (file)
        {
            fwrite(data, 1, ret, file);
            sync();
        }
        else
        {
            CDX_LOGW("save file = NULL");
        }
    }
#endif
_exit:

    pthread_mutex_lock(&streamingSource->lock);
    streamingSource->status = STREAM_IDLE;
    pthread_mutex_unlock(&streamingSource->lock);
    pthread_cond_signal(&streamingSource->cond);
    return ret;
}


CdxStreamT *StreamingSourceOpen(const sp<IStreamSource> &source,
                                size_t numBuffer, size_t bufferSize)
{
    StreamingSource *streamingSource = (StreamingSource *)malloc(sizeof(StreamingSource));
    if(!streamingSource)
    {
        CDX_LOGE("malloc fail!");
        return NULL;
    }
    memset(streamingSource, 0x00, sizeof(StreamingSource));

    int ret = pthread_mutex_init(&streamingSource->lock, NULL);
    CDX_CHECK(!ret);
    ret = pthread_cond_init(&streamingSource->cond, NULL);
    CDX_CHECK(!ret);

    streamingSource->probeData.len = ProbeDataLen;
    streamingSource->probeData.buf = (cdx_char *)malloc(ProbeDataLen);
    CDX_CHECK(streamingSource->probeData.buf);

    streamingSource->mAwStreamingSource = new awStreamingSource(source);
    //CDX_CHECK(streamingSource->mAwStreamingSource);
    streamingSource->mAwStreamingSource->start(numBuffer, bufferSize);

    streamingSource->ioState = CDX_IO_STATE_INVALID;
    streamingSource->attribute = CDX_STREAM_FLAG_NET;
    streamingSource->status = STREAM_IDLE;
    streamingSource->base.ops = (struct CdxStreamOpsS *)malloc(sizeof(struct CdxStreamOpsS));
    CDX_CHECK(streamingSource->base.ops);
    memset(streamingSource->base.ops, 0, sizeof(struct CdxStreamOpsS));
    streamingSource->base.ops->getProbeData = StreamingSourceGetProbeData;
    streamingSource->base.ops->read = StreamingSourceRead;
    streamingSource->base.ops->close = StreamingSourceClose;
    streamingSource->base.ops->getIOState = StreamingSourceGetIOState;
    streamingSource->base.ops->attribute = StreamingSourceAttribute;
    streamingSource->base.ops->control = StreamingSourceControl;
    streamingSource->base.ops->connect = PrepareThread;

#if SAVE_FILE
    file = fopen("/data/camera/save.dat", "wb+");
    if (!file)
    {
        CDX_LOGE("open file failure errno(%d)", errno);
    }
#endif
    return &streamingSource->base;
}
