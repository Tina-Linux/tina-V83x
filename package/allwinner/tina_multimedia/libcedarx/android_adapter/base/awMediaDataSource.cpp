/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : awMediaDataSource.cpp
* Description :
* History :
*   Author  : SW5
*   Date    : 2017/04/12
*   Comment : first version
*
*/
#if (CONF_ANDROID_MAJOR_VER >= 6)
//#define LOG_NDEBUG 0
#define LOG_TAG "awMediaDataSource"
#include "cdx_log.h"
#include <CdxAtomic.h>
#include "awMediaDataSource.h"

#define SAVE_FILE    (0)

#if SAVE_FILE
    FILE    *file = NULL;
#endif

using namespace android;

struct awMediaDataSource : public RefBase
{
    awMediaDataSource(const sp<DataSource> &source);

    virtual ssize_t readAt(char* buffer, size_t size);

    virtual off64_t getSize();

    virtual off64_t getOffset();

    virtual ssize_t setOffset(off64_t offset);

    virtual void disconnect();

protected:
    virtual ~awMediaDataSource();

private:
    sp<DataSource> mSource;
    Mutex mLock;
    off64_t mOffset;
    off64_t mSourceSize;
    DISALLOW_EVIL_CONSTRUCTORS(awMediaDataSource);
};


awMediaDataSource::awMediaDataSource(const sp<DataSource> &source)
    : mSource(source),
      mOffset(0),
      mSourceSize(0)
{
    CDX_LOGV("awMediaDataSource Constructor");
}

awMediaDataSource::~awMediaDataSource() {
    CDX_LOGV("awMediaDataSource Constructor ~De");
}

ssize_t awMediaDataSource::readAt(char* buffer, size_t size) {
    CDX_LOGV("awMediaDataSource readAt, size: %zu", size);
    ssize_t err = mSource->readAt(mOffset, buffer, size);
    if (err < (ssize_t)size) {
        CDX_LOGE("readAt failed, err: %s", strerror(errno));
        return -1;
    }
    mOffset += err;
    return err;
}

off64_t awMediaDataSource::getSize() {
    CDX_LOGV("awMediaDataSource getSize");

    off64_t sourceSize;
    status_t ret = mSource->getSize(&sourceSize);
    if (ret != 0) {
        CDX_LOGE("get size fail,unknown data source size, ret : %d sourceSize: %lld",
            (int)ret, sourceSize);
        mSourceSize = 0;
        return 0;
    }
    CDX_LOGV("sourceSize: %lld", sourceSize);
    mSourceSize = (sourceSize > 0 ? sourceSize:0);
    return mSourceSize;
}

off64_t awMediaDataSource::getOffset() {
    CDX_LOGV("awMediaDataSource getOffset, mOffset: %lld", mOffset);
    return mOffset;
}

ssize_t awMediaDataSource::setOffset(off64_t offset) {
    CDX_LOGV("awMediaDataSource setOffset, offset: %lld", offset);

    if(mSourceSize && (offset > mSourceSize)) {
        CDX_LOGE("invalid argument");
        return -1;
    }
    mOffset = offset;
    CDX_LOGV("setOffset: mOffset: %lld", mOffset);
    return 0;
}

void awMediaDataSource::disconnect() {
    CDX_LOGV("awMediaDataSource disconnect");
    if(mSource != NULL)
        mSource->close();
    else
        CDX_LOGW("mSource is NULL");
}


////////////////////////////////////////////////////////////////////////

#define ProbeDataLen (2*1024)

enum CdxStreamStatus
{
    DATA_SOURCE_STREAM_IDLE,
    DATA_SOURCE_STREAM_CONNECTING,
    DATA_SOURCE_STREAM_SEEKING,
    DATA_SOURCE_STREAM_READING,
};

typedef struct MediaDataSource MediaDataSource;
struct MediaDataSource
{
    CdxStreamT base;
    cdx_uint32 attribute;
    enum CdxStreamStatus status;
    cdx_int32 ioState;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    cdx_int32 forceStop;
    CdxStreamProbeDataT probeData;
    off64_t sourceSize;
    sp<awMediaDataSource> mAwMediaDataSource;
};

static cdx_int32 DataSourceStreamConnect(CdxStreamT *stream)
{
    CDX_LOGV("L: %d, func: %s", __LINE__, __func__);
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;
    pthread_mutex_lock(&dataSource->lock);
    cdx_int32 ret = 0;
    ssize_t err;
    if(dataSource->forceStop)
    {
        pthread_mutex_unlock(&dataSource->lock);
        return -1;
    }
    dataSource->status = DATA_SOURCE_STREAM_CONNECTING;
    pthread_mutex_unlock(&dataSource->lock);
    dataSource->sourceSize = dataSource->mAwMediaDataSource->getSize();
    if(dataSource->sourceSize == 0)
    {
        CDX_LOGE("unable to get soure size");
        dataSource->attribute = CDX_STREAM_FLAG_NET;
    }
    err = dataSource->mAwMediaDataSource->readAt(dataSource->probeData.buf,
        ProbeDataLen);
    if (err < (ssize_t)ProbeDataLen) {
        CDX_LOGE("readAt failed");
        ret = -1;
        goto __failure;
    }
    err = dataSource->mAwMediaDataSource->setOffset(0);
    if(err != 0)
    {
        CDX_LOGE("setOffset failed");
        ret = -1;
        goto __failure;

    }
    dataSource->ioState = CDX_IO_STATE_OK;
    pthread_mutex_lock(&dataSource->lock);
    dataSource->status = DATA_SOURCE_STREAM_IDLE;
    pthread_mutex_unlock(&dataSource->lock);
    pthread_cond_signal(&dataSource->cond);
    CDX_LOGV("connect succeed");
    return ret;
__failure:
    dataSource->ioState = CDX_IO_STATE_ERROR;
    pthread_mutex_lock(&dataSource->lock);
    dataSource->status = DATA_SOURCE_STREAM_IDLE;
    pthread_mutex_unlock(&dataSource->lock);
    pthread_cond_signal(&dataSource->cond);
    CDX_LOGW("connect fail");
    return ret;
}

static cdx_int32 DataSourceStreamGetIOState(CdxStreamT *stream)
{
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;
    return dataSource->ioState;
}

static CdxStreamProbeDataT *DataSourceStreamGetProbeData(CdxStreamT *stream)
{
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;
    return &dataSource->probeData;
}

static cdx_uint32 DataSourceStreamAttribute(CdxStreamT *stream)
{
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;
    return dataSource->attribute;
}

static cdx_int32 DataSourceStreamForceStop(CdxStreamT *stream)
{
    CDX_LOGV("L: %d, func: %s", __LINE__, __func__);
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;
    pthread_mutex_lock(&dataSource->lock);
    dataSource->forceStop = 1;
    while(dataSource->status != DATA_SOURCE_STREAM_IDLE)
    {
        pthread_cond_wait(&dataSource->cond, &dataSource->lock);
    }
    pthread_mutex_unlock(&dataSource->lock);
    CDX_LOGV("force stop end");
    return 0;
}

static cdx_int32 DataSourceStreamClrForceStop(CdxStreamT *stream)
{
    CDX_LOGV("L: %d, func: %s", __LINE__, __func__);
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;

    pthread_mutex_lock(&dataSource->lock);
    dataSource->forceStop = 0;
    dataSource->status = DATA_SOURCE_STREAM_IDLE;
    pthread_mutex_unlock(&dataSource->lock);

    return 0;
}

static cdx_int32 DataSourceStreamControl(CdxStreamT *stream, cdx_int32 cmd, void *param)
{
    CDX_LOGV("L: %d, func: %s", __LINE__, __func__);
    CDX_UNUSE(param);
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;
    switch(cmd)
    {
        case STREAM_CMD_SET_FORCESTOP:
            return DataSourceStreamForceStop(stream);
        case STREAM_CMD_CLR_FORCESTOP:
            return DataSourceStreamClrForceStop(stream);
        default:
            CDX_LOGW("not implement cmd(%d)", cmd);
            break;
    }
    return -1;
}

static cdx_int32 DataSourceStreamRead(CdxStreamT *stream, void *buf, cdx_uint32 len)
{
    CDX_LOGV("L: %d, func: %s", __LINE__, __func__);
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;
    pthread_mutex_lock(&dataSource->lock);
    if(dataSource->forceStop)
    {
        pthread_mutex_unlock(&dataSource->lock);
        return -1;
    }
    dataSource->status = DATA_SOURCE_STREAM_READING;
    pthread_mutex_unlock(&dataSource->lock);

    char *data = (char *)buf;
    ssize_t ret = 0;
    cdx_int64 pos = dataSource->mAwMediaDataSource->getOffset();
    CDX_LOGV("read--pos: %lld", pos);
    if(pos >= dataSource->sourceSize
        && dataSource->sourceSize != 0)
    {
        CDX_LOGD("eos");
        pthread_mutex_lock(&dataSource->lock);
        dataSource->status = DATA_SOURCE_STREAM_IDLE;
        dataSource->ioState = CDX_IO_STATE_EOS;
        pthread_mutex_unlock(&dataSource->lock);
        pthread_cond_signal(&dataSource->cond);
        return 0;
    }
    ret = dataSource->mAwMediaDataSource->readAt(data, len);
    CDX_LOGV("len: %d", len);
    //CDX_BUF_DUMP(data, len);
    if(ret == (ssize_t)0)
    {
        dataSource->ioState = CDX_IO_STATE_EOS;
    }
    else if(ret < (ssize_t)0)
    {
        CDX_LOGE("readAt fail");
    }
#if SAVE_FILE
    if(ret > (ssize_t)0)
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
    pthread_mutex_lock(&dataSource->lock);
    dataSource->status = DATA_SOURCE_STREAM_IDLE;
    pthread_mutex_unlock(&dataSource->lock);
    pthread_cond_signal(&dataSource->cond);
    return ret;
}

static cdx_int32 DataSourceStreamSeek(CdxStreamT *stream, cdx_int64 offset,
                                     cdx_int32 whence)
{
    CDX_LOGV("L: %d, func: %s, offset: %lld", __LINE__, __func__, offset);
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;
    cdx_int64 ret = 0;
    pthread_mutex_lock(&dataSource->lock);
    if(dataSource->forceStop)
    {
        pthread_mutex_unlock(&dataSource->lock);
        return -1;
    }
    dataSource->status = DATA_SOURCE_STREAM_SEEKING;
    pthread_mutex_unlock(&dataSource->lock);
    switch (whence)
    {
    case STREAM_SEEK_SET:
    {
        if(offset < 0 || offset > dataSource->sourceSize)
        {
            CDX_LOGE("invalid arguments, offset(%lld), size(%lld)",
                offset, dataSource->sourceSize);
            ret = -1;
            goto _exit;
        }
        ret = dataSource->mAwMediaDataSource->setOffset(offset);
        break;
    }
    case STREAM_SEEK_CUR:
    {
        cdx_int64 cur_pos = dataSource->mAwMediaDataSource->getOffset();

        if (cur_pos + offset < 0 || cur_pos + offset > dataSource->sourceSize)
        {
            CDX_LOGE("invalid arguments, offset(%lld), size(%lld), curPos(%lld)",
                     offset, dataSource->sourceSize, cur_pos);
            ret = -1;
            goto _exit;
        }
        ret = dataSource->mAwMediaDataSource->setOffset(cur_pos + offset);
        break;
    }
    case STREAM_SEEK_END:
    {
        cdx_int64 cur_pos = dataSource->mAwMediaDataSource->getOffset();
        cdx_int64 absOffset = cur_pos + dataSource->sourceSize + offset;

        if (absOffset < cur_pos || absOffset > cur_pos + dataSource->sourceSize)
        {
            CDX_LOGE("invalid arguments, offset(%lld), size(%lld)",
                     absOffset, cur_pos + dataSource->sourceSize);
            ret = -1;
            goto _exit;
        }
        ret = dataSource->mAwMediaDataSource->setOffset(dataSource->sourceSize - offset);
        break;
    }
    default :
        CDX_CHECK(0);
        break;
    }
_exit:
    pthread_mutex_lock(&dataSource->lock);
    dataSource->status = DATA_SOURCE_STREAM_IDLE;
    pthread_mutex_unlock(&dataSource->lock);
    pthread_cond_signal(&dataSource->cond);
    return ret;
}

static cdx_int64 DataSourceStreamSize(CdxStreamT *stream)
{
    CDX_LOGV("L: %d, func: %s", __LINE__, __func__);
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;

    return dataSource->sourceSize;

}

static cdx_int64 DataSourceStreamTell(CdxStreamT *stream)
{
    CDX_LOGV("L: %d, func: %s", __LINE__, __func__);
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;
    return dataSource->mAwMediaDataSource->getOffset();
}

static cdx_bool DataSourceStreamEos(CdxStreamT *stream)
{
    CDX_LOGV("L: %d, func: %s", __LINE__, __func__);
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;
    cdx_int64 pos = dataSource->mAwMediaDataSource->getOffset();
    cdx_int64 size = dataSource->sourceSize;

    return (pos == size);
}

static cdx_int32 DataSourceStreamClose(CdxStreamT *stream)
{
    CDX_LOGV("L: %d, func: %s", __LINE__, __func__);
    CDX_UNUSE(stream);
    return 0;
}

CdxStreamT* MediaDataSourceOpen(const sp<DataSource> &source)
{
    CDX_LOGV("L: %d, func: %s", __LINE__, __func__);
    MediaDataSource *dataSource = (MediaDataSource *)malloc(sizeof(MediaDataSource));
    if(!dataSource)
    {
        CDX_LOGE("malloc fail!");
        return NULL;
    }
    memset(dataSource, 0x00, sizeof(MediaDataSource));

    int ret = pthread_mutex_init(&dataSource->lock, NULL);
    CDX_CHECK(!ret);
    ret = pthread_cond_init(&dataSource->cond, NULL);
    CDX_CHECK(!ret);

    dataSource->probeData.len = ProbeDataLen;
    dataSource->probeData.buf = (cdx_char *)malloc(ProbeDataLen);
    CDX_CHECK(dataSource->probeData.buf);

    dataSource->mAwMediaDataSource = new awMediaDataSource(source);
    dataSource->ioState = CDX_IO_STATE_INVALID;
    dataSource->attribute = CDX_STREAM_FLAG_SEEK;
    dataSource->status = DATA_SOURCE_STREAM_IDLE;
    dataSource->base.ops = (struct CdxStreamOpsS *)malloc(sizeof(struct CdxStreamOpsS));
    CDX_CHECK(dataSource->base.ops);
    memset(dataSource->base.ops, 0, sizeof(struct CdxStreamOpsS));
    dataSource->base.ops->getProbeData = DataSourceStreamGetProbeData;
    dataSource->base.ops->read = DataSourceStreamRead;
    dataSource->base.ops->close = DataSourceStreamClose;
    dataSource->base.ops->getIOState = DataSourceStreamGetIOState;
    dataSource->base.ops->attribute = DataSourceStreamAttribute;
    dataSource->base.ops->control = DataSourceStreamControl;
    dataSource->base.ops->connect = DataSourceStreamConnect;
    dataSource->base.ops->seek = DataSourceStreamSeek;
    dataSource->base.ops->eos = DataSourceStreamEos;
    dataSource->base.ops->tell = DataSourceStreamTell;
    dataSource->base.ops->size = DataSourceStreamSize;
#if SAVE_FILE
    file = fopen("/data/camera/save.dat", "wb+");
    if (!file)
    {
        CDX_LOGE("open file failure errno(%d)", errno);
    }
#endif
    return &dataSource->base;
}

cdx_int32 MediaDataSourceClose(CdxStreamT *stream)
{
    CDX_LOGV("L: %d, func: %s", __LINE__, __func__);
    CDX_CHECK(stream);
    MediaDataSource *dataSource = (MediaDataSource *)stream;
    DataSourceStreamForceStop(stream);
    dataSource->mAwMediaDataSource->disconnect();
    if(dataSource->probeData.buf)
        free(dataSource->probeData.buf);

    if(dataSource->base.ops)
        free(dataSource->base.ops);
#if SAVE_FILE
    if (file)
    {
        fclose(file);
        file = NULL;
    }
#endif
    pthread_mutex_destroy(&dataSource->lock);
    pthread_cond_destroy(&dataSource->cond);
    free(dataSource);
    return 0;
}
#endif
