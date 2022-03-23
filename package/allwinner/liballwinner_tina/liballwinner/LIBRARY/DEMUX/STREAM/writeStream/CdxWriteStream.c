/****************************************************
*    this stream for muxer wrting data
*
*
*
*************************************************/

#include <CdxStream.h>
#include <CdxAtomic.h>
#include <CdxMemory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define WRITE_STREAM_SCHEME "write://"

#define fopen64(uri) open(uri, O_RDWR | O_CREAT, 666)

#define fseek64(fd, offset, whence) lseek64(fd, offset, whence)

#define ftell64(fd) lseek64(fd, 0, SEEK_CUR)

#define fread64(fd, buf, len) read(fd, buf, len)

#define fwrite64(fd, buf, len) write(fd, buf, len)

//#define feof64(fd) lseek64(fd, 0, SEEK_CUR)

#define fclose64(fd) close(fd)


enum FileStreamStateE
{
    WRITE_STREAM_IDLE    = 0x00L,
    WRITE_STREAM_READING = 0x01L,
    WRITE_STREAM_WRITING = 0x02L,
    WRITE_STREAM_SEEKING = 0x03L,
    WRITE_STREAM_CLOSING = 0x04L,
};

/*fmt: "file://xxx" */
struct CdxWriteStreamImplS
{
    CdxStreamT base;
    cdx_atomic_t state;

    cdx_int32 ioErr;

    int fd;
    cdx_int64 offset;
    cdx_int64 size;
    char *filePath;

    char* tmpBuf;
    cdx_int64   tmpSize;

    char* cache_buf;
    int   cache_buf_size;
    int   cache_size;
};

static inline cdx_int32 WaitIdleAndSetState(cdx_atomic_t *state, cdx_ssize val)
{
    while (!CdxAtomicCAS(state, WRITE_STREAM_IDLE, val))
    {
        if (CdxAtomicRead(state) == WRITE_STREAM_CLOSING)
        {
            CDX_LOGW("file is closing.");
            return CDX_FAILURE;
        }
    }
    return CDX_SUCCESS;
}

/*
static cdx_int32 __WStreamRead(CdxStreamT *stream, cdx_void *buf, cdx_uint32 len)
{
    struct CdxWriteStreamImplS *impl;
    cdx_int32 ret;
	cdx_int64 nHadReadLen;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);

	// * we must limit the HadReadLen within impl->size,
    // * or in some case will be wrong, such as cts
    nHadReadLen = ftell64(impl->fd) - impl->offset;
    if(nHadReadLen >= impl->size)
    {
        CDX_LOGD("eos, pos(%lld)",impl->size);
        return 0;
    }

    if (WaitIdleAndSetState(&impl->state, WRITE_STREAM_READING) != CDX_SUCCESS)
    {
        CDX_LOGE("set state(%d) fail.", CdxAtomicRead(&impl->state));
        return -1;
    }

    if((nHadReadLen + len) > impl->size)
    {
        len = impl->size - nHadReadLen;
    }

    ret = fread64(impl->fd, buf, len);

    if (ret < (cdx_int32)len)
    {
        if ((ftell64(impl->fd) - impl->offset) == impl->size) // * end of file
        {
            CDX_LOGD("eos, ret(%d), pos(%lld)...", ret, impl->size);
            impl->ioErr = CDX_IO_STATE_EOS;
        }
        else
        {
            impl->ioErr = errno;
            CDX_LOGE("ret(%d), errno(%d), cur pos:(%lld), impl->size(%lld)",
                    ret, impl->ioErr, ftell64(impl->fd) - impl->offset, impl->size);
        }
    }

    CdxAtomicSet(&impl->state, WRITE_STREAM_IDLE);
    return ret;
}
*/

static cdx_int32 __WStreamClose(CdxStreamT *stream)
{
    struct CdxWriteStreamImplS *impl;
    cdx_int32 ret;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);

    ret = WaitIdleAndSetState(&impl->state, WRITE_STREAM_CLOSING);

    CDX_FORCE_CHECK(CDX_SUCCESS == ret);

    if(impl->cache_size > 0)
    {
	ret = fwrite64(impl->fd, impl->cache_buf, impl->cache_size);
	    if(ret < impl->cache_size)
	    {
		loge("wrie err(%d)", errno);
		return -1;
	    }
    }

    //* the fd may be invalid when close, such as in TF-card test
    ret = fclose64(impl->fd);
    if(ret != 0)
    {
        logw(" close fd may be not normal, ret = %d, errno = %d",ret,errno);
    }

    if (impl->filePath)
    {
        CdxFree(impl->filePath);
    }

    if(impl->tmpBuf)
    {
	free(impl->tmpBuf);
    }

    if(impl->cache_buf)
    {
	free(impl->cache_buf);
    }

    CdxFree(impl);
    // TODO: use refence
    return CDX_SUCCESS;
}

static cdx_int32 __WStreamGetIoState(CdxStreamT *stream)
{
    struct CdxWriteStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);

    return impl->ioErr;
}

static cdx_uint32 __WStreamAttribute(CdxStreamT *stream)
{
	CDX_UNUSE(stream);
    return CDX_STREAM_FLAG_SEEK;
}

static cdx_int32 __WStreamControl(CdxStreamT *stream, cdx_int32 cmd, cdx_void *param)
{

    struct CdxWriteStreamImplS *impl;

	CDX_UNUSE(param);

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);

    switch (cmd)
    {
    default :
        break;
    }

    return CDX_SUCCESS;
}

static int SeekBeyondFileSize(struct CdxWriteStreamImplS *impl, cdx_int64 seekPos)
{
	cdx_int64 ret;
	cdx_int64 size = seekPos - impl->size;
	cdx_int64 totalSize = size;
	cdx_int64 writeLen = 0;
	fseek64(impl->fd, impl->size, SEEK_SET);
	while(totalSize > 0)  //it si a bug
	{
		writeLen = (totalSize > impl->tmpSize) ? impl->tmpSize : totalSize;
		CDX_LOGD("++++ writeLen(%lld)", writeLen);
		ret = fwrite64(impl->fd, impl->tmpBuf, writeLen);
		if(ret < writeLen)
		{
			CDX_LOGE("write failed, ret(%lld), writeLen(%lld)", ret, writeLen);
		}
		totalSize -= ret;
	}

	// update file size
	impl->size = seekPos;
	return 0;
}

static cdx_int32 __WStreamSeek(CdxStreamT *stream, cdx_int64 offset, cdx_int32 whence)
{
    struct CdxWriteStreamImplS *impl;
    cdx_int32 ret = 0;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);

    if (WaitIdleAndSetState(&impl->state, WRITE_STREAM_SEEKING) != CDX_SUCCESS)
    {
        CDX_LOGE("set state(%d) fail.", CdxAtomicRead(&impl->state));
        impl->ioErr = CDX_IO_STATE_INVALID;
        return -1;
    }

    if(impl->cache_size > 0)
    {
	ret = fwrite64(impl->fd, impl->cache_buf, impl->cache_size);
	    if(ret < impl->cache_size)
	    {
		loge("wrie err(%d)", errno);
		return -1;
	    }
	    impl->cache_size = 0;
    }

    switch (whence)
    {
    case STREAM_SEEK_SET:
    {
        if (offset < 0)
        {
            CDX_LOGE("invalid arguments, offset(%lld), size(%lld)", offset, impl->size);
            CdxDumpThreadStack(gettid());
            CdxAtomicSet(&impl->state, WRITE_STREAM_IDLE);
            return -1;
        }
        else if(offset < impl->size)
        {
	        ret = fseek64(impl->fd, impl->offset + offset, SEEK_SET);
	        break;
        }
        else
        {
		SeekBeyondFileSize(impl, offset);
			break;
        }
    }
    case STREAM_SEEK_CUR:
    {
        cdx_int64 curPos = ftell64(impl->fd) - impl->offset;
        cdx_int64 seekPos = curPos + offset;
        if (seekPos < 0)
        {
            CDX_LOGE("invalid arguments, offset(%lld), size(%lld), curPos(%lld)", offset, impl->size, curPos);
            CdxDumpThreadStack(gettid());
            CdxAtomicSet(&impl->state, WRITE_STREAM_IDLE);
            return -1;
        }
        else if(seekPos <= impl->size)
        {
		ret = fseek64(impl->fd, offset, SEEK_CUR);
		break;
        }
        else
        {
		SeekBeyondFileSize(impl, seekPos);
		break;
        }
    }
    case STREAM_SEEK_END:
    {
        cdx_int64 absOffset = impl->offset + impl->size + offset;
        if (absOffset < impl->offset || absOffset > impl->offset + impl->size)
        {
            CDX_LOGE("invalid arguments, offset(%lld), size(%lld)", absOffset, impl->offset + impl->size);
            CdxDumpThreadStack(gettid());
            CdxAtomicSet(&impl->state, WRITE_STREAM_IDLE);
            return -1;
        }
        ret = fseek64(impl->fd, absOffset, SEEK_SET);
        break;
    }
    default :
        CDX_CHECK(0);
        break;
    }

    if (ret < 0)
    {
        impl->ioErr = errno;
        CDX_LOGE("seek failure, io error(%d); 'whence(%d), base-offset(%lld), offset(%lld)' ",
                impl->ioErr, whence, impl->offset, offset);
    }

    CdxAtomicSet(&impl->state, WRITE_STREAM_IDLE);
    return (ret >= 0 ? 0 : -1);
}

static cdx_int64 __WStreamTell(CdxStreamT *stream)
{
    struct CdxWriteStreamImplS *impl;
    cdx_int64 pos;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);
    pos = ftell64(impl->fd) - impl->offset;
    if (-1 == pos)
    {
        impl->ioErr = errno;
        CDX_LOGE("ftello failure, io error(%d)", impl->ioErr);
    }
    return pos;
}

static cdx_bool __WStreamEos(CdxStreamT *stream)
{
    struct CdxWriteStreamImplS *impl;
    cdx_int64 pos = -1;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);
    pos = ftell64(impl->fd) - impl->offset;
    CDX_LOGD("(%lld / %lld / %lld)", pos, impl->offset, impl->size);
    return (pos == impl->size);
}

static cdx_int64 __WStreamSize(CdxStreamT *stream)
{
    struct CdxWriteStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);

    return impl->size;
}

static cdx_int32 __WStreamGetMetaData(CdxStreamT *stream, const cdx_char *key, cdx_void **pVal)
{
    struct CdxWriteStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);

    if (strcmp(key, "uri") == 0)
    {
        *pVal = impl->filePath;
        return 0;
    }

    CDX_LOGW("key(%s) not found...", key);
    return -1;
}


static cdx_int32 __WStreamConnect(CdxStreamT *stream)
{
    cdx_int32 ret = 0;
    struct CdxWriteStreamImplS *impl;
    CDX_LOGD("__WStreamConnect");
    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);
	logd("impl->filePath; %s", impl->filePath);
	if (strncmp(impl->filePath, WRITE_STREAM_SCHEME, 8) == 0) /*write://... */
	{
		impl->fd = fopen64(impl->filePath + 8);
		if (impl->fd <= 0)
		{
			CDX_LOGE("open file failure, errno(%d)", errno);
			ret = -1;
			goto failure;
		}

		CDX_LOGD("++++++impl->fd(%d)", impl->fd);
		impl->offset = 0;
		impl->size = 0;
		//impl->size = fseek64(impl->fd, 0, SEEK_END);
		//ret = (cdx_int32)fseek64(impl->fd, 0, SEEK_SET);
		//CDX_LOG_CHECK(ret == 0, "errno(%d)", errno);
	}
	else
	{
		CDX_LOG_CHECK(0, "uri(%s) not write stream...", impl->filePath);
	}

	if(impl->fd < 3)
	{
		CDX_LOGD("++++ fd error(%d)", impl->fd);
		return -1;
	}
	CDX_LOGD("open success");
	CdxAtomicSet(&impl->state, WRITE_STREAM_IDLE);
	impl->ioErr = CDX_SUCCESS;

	return ret;

failure:
	return ret;

}

static int __WStreamWrite(CdxStreamT *stream, void * buf, cdx_uint32 len)
{
	struct CdxWriteStreamImplS *impl;
	int ret = 0;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);

	if (WaitIdleAndSetState(&impl->state, WRITE_STREAM_WRITING) != CDX_SUCCESS)
	{
		CDX_LOGE("set state(%d) fail.", CdxAtomicRead(&impl->state));
		return -1;
	}

	int remaind_size = impl->cache_buf_size - impl->cache_size;
	if(len < remaind_size)
	{
		memcpy(impl->cache_buf+impl->cache_size, buf, len);
		impl->cache_size += len;
		ret = len;
	}
	else
	{
		memcpy(impl->cache_buf+impl->cache_size, buf, remaind_size);
		ret = fwrite64(impl->fd, impl->cache_buf, impl->cache_buf_size);
	    if(ret < impl->cache_buf_size)
	    {
		loge("wrie err(%d), ret: %d, len: %d", errno, ret, len);
		CdxAtomicSet(&impl->state, WRITE_STREAM_IDLE);
		return -1;
	    }
	    memcpy(impl->cache_buf, buf+remaind_size, len-remaind_size);

		impl->cache_size = len-remaind_size;
	}

    impl->size += len;

	CdxAtomicSet(&impl->state, WRITE_STREAM_IDLE);
    return ret;
}

static cdx_int32 __WStreamRead(CdxStreamT *stream, cdx_void *buf, cdx_uint32 len)
{
    struct CdxWriteStreamImplS *impl;
    cdx_int32 ret;
	  cdx_int64 nHadReadLen;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxWriteStreamImplS, base);

	//* we must limit the HadReadLen within impl->size,
    //* or in some case will be wrong, such as cts
    nHadReadLen = ftell64(impl->fd) - impl->offset;
    if(nHadReadLen >= impl->size)
    {
        CDX_LOGD("eos, pos(%lld)",impl->size);
        return 0;
    }

    if (WaitIdleAndSetState(&impl->state, WRITE_STREAM_READING) != CDX_SUCCESS)
    {
        CDX_LOGE("set state(%d) fail.", CdxAtomicRead(&impl->state));
        return -1;
    }

    if((nHadReadLen + len) > impl->size)
    {
        len = impl->size - nHadReadLen;
    }

    ret = fread64(impl->fd, buf, len);

    if (ret < (cdx_int32)len)
    {
        if ((ftell64(impl->fd) - impl->offset) == impl->size) /*end of file*/
        {
            CDX_LOGD("eos, ret(%d), pos(%lld)...", ret, impl->size);
            impl->ioErr = CDX_IO_STATE_EOS;
        }
        else
        {
            impl->ioErr = errno;
            CDX_LOGE("ret(%d), errno(%d), cur pos:(%lld), impl->size(%lld)",
                    ret, impl->ioErr, ftell64(impl->fd) - impl->offset, impl->size);
        }
    }

    CdxAtomicSet(&impl->state, WRITE_STREAM_IDLE);
    return ret;
}

static struct CdxStreamOpsS writeStreamOps =
{
	.connect		= __WStreamConnect,
    .getProbeData	= NULL,
    .read			= __WStreamRead,
    .write			= __WStreamWrite,
    .close			= __WStreamClose,
    .getIOState			= __WStreamGetIoState,
    .attribute		= __WStreamAttribute,
    .control		= __WStreamControl,
    .getMetaData	= __WStreamGetMetaData,
    .seek			= __WStreamSeek,
    .seekToTime			= NULL,
    .eos		= __WStreamEos,
    .tell			= __WStreamTell,
    .size			= __WStreamSize,
};

static CdxStreamT *__WStreamCreate(CdxDataSourceT *source)
{
    struct CdxWriteStreamImplS *impl;

    impl = CdxMalloc(sizeof(*impl));
    CDX_FORCE_CHECK(impl);
    memset(impl, 0x00, sizeof(*impl));

    impl->base.ops = &writeStreamOps;
    impl->filePath = CdxStrdup(source->uri);
    impl->tmpSize = 64*1024;
    impl->tmpBuf = malloc(impl->tmpSize);
    memset(impl->tmpBuf, 0x00, impl->tmpSize);

	impl->cache_buf_size = 512*1024;
	impl->cache_size = 0;
    impl->cache_buf = malloc(impl->cache_buf_size);
    if(impl->cache_buf == NULL)
    {
	loge("cannot malloc cache buf");
    }
	impl->ioErr = -1;
	CDX_LOGD("__WStreamCreate local file '%s'", source->uri);

	return &impl->base;
}

CdxStreamCreatorT writeStreamCtor =
{
    .create = __WStreamCreate
};
