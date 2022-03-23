/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxTcpStream.c
 * Description : TcpStream
 * History :
 *
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "tcpStream"
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <CdxTypes.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <CdxSocketUtil.h>
#include <netdb.h>

#include <CdxStream.h>
#include <CdxAtomic.h>
#include <pthread.h>
#include <SmartDnsService.h>

#define SOCKRECVBUF_LEN 512*1024// 262142 (5*1024*1024)
#define closesocket close
#define PROBE_DATA_LEN_DEFAULT (128*1024)

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_key_t oldSocketKey;

static void CdxTcpStreamDecRef(CdxStreamT *stream);

enum HttpStreamStateE
{
    TCP_STREAM_IDLE    = 0x00L,
    TCP_STREAM_CONNECTING = 0x01L,
    TCP_STREAM_READING = 0x02L,
    TCP_STREAM_WRITING = 0x03L,
    TCP_STREAM_FORCESTOPPED = 0x04L,
    //TCP_STREAM_CLOSING
};

typedef struct CdxTcpStreamImpl
{
    CdxStreamT base;
    cdx_int32 ioState;
    cdx_int32 sockRecvBufLen;
    cdx_int32 notBlockFlag;
//    cdx_int32 exitFlag;              //when close, exit
    cdx_int32 forceStopFlag;
    cdx_int32 sockFd;                  //socket fd
    //int eof;                         //all stream data is read from network
    cdx_int32 port;
    cdx_char *hostname;
    cdx_atomic_t ref;                  //reference count, for free resource while still blocking.

    enum HttpStreamStateE state;
    pthread_mutex_t lock;
    pthread_cond_t cond;

    pthread_cond_t dnsCond;
    pthread_mutex_t* dnsMutex;
    int dnsRet;
    struct addrinfo *dnsAI;

#if defined(CONF_YUNOS)
    //YUNOS
    cdx_char mTcpIP[100];
    ParserCallback callback;
    void *pUserData;
    int mYunOSstatusCode;
#endif

    int saveOldSocket;

    CdxStreamProbeDataT probeData;
    cdx_char *buffer;
    cdx_int32 validDataSize;
    cdx_char *sourceUri;
    cdx_int64 readPos;
    cdx_int32 needProbe;
}CdxTcpStreamImplT;

typedef struct CdxHttpSendBuffer
{
    void *size;
    void *buf;
}CdxHttpSendBufferT;

static int64_t GetNowUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_sec * 1000000ll + tv.tv_usec;
}

void oldSocketDestr(void *p)
{
    int oldFd = (long)p - 1;
    CDX_LOGD("close old socket %d", oldFd);
    close(oldFd);
}

static void createoldSocketKey()
{
    int ret = pthread_key_create(&oldSocketKey, oldSocketDestr);
    if (ret)
    {
        CDX_LOGE("pthread_key_create failed: %s", strerror(errno));
        abort();
    }
}
static CdxStreamProbeDataT *__CdxTcpStreamGetProbeData(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    return &impl->probeData;
}
static cdx_int32 __CdxTcpStreamRead(CdxStreamT *stream, void *buf, cdx_uint32 len)
{
    CdxTcpStreamImplT *impl;
    cdx_int32 ret;
    cdx_int32 recvSize = 0;
    cdx_int32 ioErr;
    cdx_int32 num = 0;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    if(stream == NULL || buf == NULL || len <= 0)
    {
        CDX_LOGW("check parameter.");
        return -1;
    }

    pthread_mutex_lock(&impl->lock);
    if(impl->forceStopFlag)
    {
        pthread_mutex_unlock(&impl->lock);
        return -2;
    }
    CdxAtomicInc(&impl->ref);
    impl->state = TCP_STREAM_READING;
    pthread_mutex_unlock(&impl->lock);
	//read from buffer first.
    if(impl->validDataSize > 0 && impl->buffer != NULL)
    {
        if(len <= (cdx_uint32)impl->validDataSize)
        {
            memcpy(buf, impl->buffer+impl->readPos, len);
            impl->validDataSize -= len;
            recvSize = len;
            goto __exit1;
        }
        else
        {
            memcpy(buf, impl->buffer+impl->readPos, impl->validDataSize);
            recvSize = impl->validDataSize;
            impl->validDataSize = 0;
        }
    }

    while(impl->notBlockFlag)
    {
        if(impl->forceStopFlag)
        {
            //CDX_LOGV("__CdxTcpStreamRead forceStop.");
            ret = -2;
            goto __exit0;
        }
        ret = CdxSockNoblockRecv(impl->sockFd, buf, len);
        if (ret < 0)
        {
            ioErr = errno;
            if (EAGAIN == ioErr)
            {
                num++;
                usleep(5000);
                if(num < 400) //* notBlockFlag, try 2s at most.
                {
                    continue;
                }
                else
                {
                    ret = 0;
                }
            }
            else
            {
                CDX_LOGE("<%s,%d>recv err(%d): %s", __FUNCTION__, __LINE__, errno, strerror(errno));
                impl->ioState = CDX_IO_STATE_ERROR;
                impl->notBlockFlag = 0;
                ret = -1;
                goto __exit0;
            }
        }
        CDX_LOGV("xxx CdxSockNoblockRecv(%d)", ret);
        impl->notBlockFlag = 0;

__exit0:
        pthread_mutex_lock(&impl->lock);
        impl->state = TCP_STREAM_IDLE;
        CdxTcpStreamDecRef(stream);
        pthread_mutex_unlock(&impl->lock);

        return ret;
    }

    while((cdx_uint32)recvSize < len)
    {
        if(impl->forceStopFlag)
        {
            CDX_LOGV("__CdxTcpStreamRead forceStop.");
            if(recvSize > 0)
                break;
            else
            {
                recvSize = -2;
                goto __exit1;
            }
        }
        ret = CdxSockAsynRecv(impl->sockFd, (char *)buf + recvSize,
                                            len - recvSize, 0, &impl->forceStopFlag);
        if(ret < 0)
        {
            if(ret == -2)
            {
                recvSize = recvSize>0 ? recvSize : -2;
                goto __exit1;
            }
            impl->ioState = CDX_IO_STATE_ERROR;
            CDX_LOGE("__CdxTcpStreamRead error(%d): %s. recvSize(%d)",
                errno, strerror(errno), recvSize);
            recvSize = -1;

#if defined(CONF_YUNOS)
            if(impl->callback)
            {
                impl->mYunOSstatusCode = 3003; //Ali YUNOS invoke info
                impl->callback(impl->pUserData, STREAM_EVT_DOWNLOAD_DOWNLOAD_ERROR,
                    &(impl->mYunOSstatusCode));
            }
#endif

            goto __exit1;
        }
        else if(ret == 0)
        {
            //CDX_LOGD("xxx ret = 0.");
            //if(++num >= 100)
            //{
            //    CDX_LOGW("xxx connection closed by peer, fd(%d)", impl->sockFd);
            //    impl->ioState = CDX_IO_STATE_ERROR;
            //    CdxAtomicDec(&impl->ref);
            //    return -1;
            //}
            break;
        }
        else
        {
            recvSize += ret;
        }
    }

__exit1:
    pthread_mutex_lock(&impl->lock);
    impl->state = TCP_STREAM_IDLE;
    CdxTcpStreamDecRef(stream);
    pthread_mutex_unlock(&impl->lock);
    pthread_cond_signal(&impl->cond);
    impl->readPos += recvSize;
    return recvSize;
}
static cdx_int64 __CdxTcpStreamTell(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    return impl->readPos;
}
static cdx_int64 __CdxTcpStreamSize(CdxStreamT *stream)
{
    CDX_CHECK(stream);

    return -1;
}

static cdx_uint32 __CdxTcpStreamAttribute(CdxStreamT *stream)
{
    CDX_CHECK(stream);
    cdx_uint32 flag = 0;

    return flag|CDX_STREAM_FLAG_NET;
}


static cdx_int32 __CdxTcpStreamGetIOState(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    return impl->ioState;
}
static cdx_int32 __CdxTcpStreamEos(CdxStreamT *stream)
{
    CDX_CHECK(stream);

    return -1;
}

static cdx_int32 __CdxTcpStreamGetMetaData(CdxStreamT *stream, const cdx_char *key,
                                        void **pVal)
{
    CdxTcpStreamImplT *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    if(strcmp(key, "uri") == 0)
    {
        *pVal = impl->sourceUri;
        return 0;
    }

    return -1;
}
static cdx_int32 __CdxTcpStreamWrite(CdxStreamT *stream, void *buf, cdx_uint32 len)
{
    CdxTcpStreamImplT *impl;
    size_t size = 0;
    ssize_t ret = 0;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    pthread_mutex_lock(&impl->lock);
    if(impl->forceStopFlag)
    {
        pthread_mutex_unlock(&impl->lock);
        return -1;
    }
    CdxAtomicInc(&impl->ref);
    impl->state = TCP_STREAM_WRITING;
    pthread_mutex_unlock(&impl->lock);

    while(size < len)
    {
        ret = CdxSockAsynSend(impl->sockFd, (char *)buf + size, len - size,
                                                        0, &impl->forceStopFlag);
        if(ret < 0)
        {
            CDX_LOGE("send failed. error(%d): %s.", errno, strerror(errno));
            break;
        }
        else if(ret == 0)
        {
            break;
        }
        size += ret;
    }

    pthread_mutex_lock(&impl->lock);
    impl->state = TCP_STREAM_IDLE;
    CdxTcpStreamDecRef(stream);
    pthread_mutex_unlock(&impl->lock);
    pthread_cond_signal(&impl->cond);

    return (size == len) ? 0 : -1;
}
static cdx_int32 CdxTcpStreamForceStop(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;
    long ref;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    CDX_LOGV("begin tcp force stop");
    pthread_mutex_lock(&impl->lock);
    if(impl->forceStopFlag == 1)
    {
        pthread_mutex_unlock(&impl->lock);
        return 0;
    }
    CdxAtomicInc(&impl->ref);
    impl->forceStopFlag = 1;
    while(impl->state != TCP_STREAM_IDLE)
    {
        pthread_cond_wait(&impl->cond, &impl->lock);
    }
    pthread_mutex_unlock(&impl->lock);

    CdxTcpStreamDecRef(stream);
    CDX_LOGV("finish tcp force stop");
    return 0;
}
static cdx_int32 CdxTcpStreamClrForceStop(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    pthread_mutex_lock(&impl->lock);
    impl->forceStopFlag = 0;
    impl->state = TCP_STREAM_IDLE;
    pthread_mutex_unlock(&impl->lock);

    return 0;
}

static cdx_int32 __CdxTcpStreamControl(CdxStreamT *stream, cdx_int32 cmd, void *param)
{
    CdxTcpStreamImplT *impl;

    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    switch(cmd)
    {
        case STREAM_CMD_READ_NOBLOCK:
        {
            impl->notBlockFlag = 1;
            break;
        }
        case STREAM_CMD_GET_SOCKRECVBUFLEN:
        {
            *(int *)param = impl->sockRecvBufLen;
            break;
        }
        case STREAM_CMD_SET_FORCESTOP:
        {
            return CdxTcpStreamForceStop(stream);
        }
        case STREAM_CMD_CLR_FORCESTOP:
        {
            return CdxTcpStreamClrForceStop(stream);
        }

#if defined(CONF_YUNOS)
        case STREAM_CMD_GET_IP:
        {
            if(impl->mTcpIP[0])
                strcpy((char *)param,impl->mTcpIP);
            break;
        }
        case STREAM_CMD_SET_CALLBACK:
        {
            struct CallBack *cb = (struct CallBack *)param;
            impl->callback = cb->callback;
            impl->pUserData = cb->pUserData;
            break;
        }
#endif
        case STREAM_CMD_SET_EOF:
            impl->saveOldSocket = 1;
            break;

        default:
        {
            CDX_LOGV("control cmd %d is not supported by tcp",cmd);
            break;
        }
    }
    return 0;
}

static cdx_int32 __CdxTcpStreamClose(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);
    CDX_LOGV("xxx tcp close begin.");
    CdxAtomicInc(&impl->ref);

    CdxTcpStreamForceStop(stream);
    CdxTcpStreamDecRef(stream);
    CdxTcpStreamDecRef(stream);

    return 0;
}
static void CdxTcpStreamDecRef(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    CdxAtomicDec(&impl->ref);
    if(CdxAtomicRead(&impl->ref) != 0)
        return;

    if (impl->sockFd >= 0)
    {
        if (impl->saveOldSocket)
        {
            CDX_LOGD("save old socket");
            /* sockFd can be zero (at least in theory).
             * The value initially associated with oldSocketKey is NULL.
             * If we don't add one to sockFd, it's difficult to figure out
             * whether pthread_getspecific return a valid file descriptor or
             * not.
             */
            int oldsock = (long)pthread_getspecific(oldSocketKey) - 1;
            if (oldsock != impl->sockFd)
            {
                if (oldsock >= 0)
                {
                    CDX_LOGD("stream open and close in different threads, "
                            "socket reuse like HTTP keep alive won't work");
                    close(oldsock);
                }
                pthread_setspecific(oldSocketKey, (void *)(long)(impl->sockFd + 1));
            }
        }
        else
        {
            /* 保留此函数，原因是曾经遇到基地抓包分析，发现虽然客户端关闭了fd，
             * 但是服务器未关，导致抓包里出现探查报文。
             */
            CdxSockDisableTcpKeepalive(impl->sockFd);

            shutdown(impl->sockFd, SHUT_RDWR);
            closesocket(impl->sockFd);
        }
    }
    pthread_mutex_destroy(&impl->lock);
    pthread_cond_destroy(&impl->cond);
    pthread_mutex_destroy(impl->dnsMutex);
    pthread_cond_destroy(&impl->dnsCond);
    free(impl->dnsMutex);
    impl->dnsMutex = NULL;
	if(impl->hostname != NULL)
    {
        free(impl->hostname);
    }
    if(impl->probeData.buf != NULL)
    {
        free(impl->probeData.buf);
        impl->probeData.buf = NULL;
    }
    if(impl->buffer != NULL)
    {
        free(impl->buffer);
        impl->buffer = NULL;
    }
    if(impl->sourceUri != NULL)
    {
        free(impl->sourceUri);
        impl->sourceUri = NULL;
    }
    free(impl);
    impl=NULL;
    CDX_LOGV("xxx tcp close end.");
}

static void DnsResponeHook(void *userhdr, int ret, struct addrinfo *ai)
{
    CdxTcpStreamImplT *impl = (CdxTcpStreamImplT *)userhdr;

    if (impl == NULL)
      return;

    if (ret == SDS_OK)
    {
        impl->dnsAI = ai;
        /*CDX_LOGD("%x%x%x", ai->ai_addr->sa_data[0],
                                                  ai->ai_addr->sa_data[1],
                                                  ai->ai_addr->sa_data[2]);*/
    }

    impl->dnsRet = ret;
    if (impl->dnsMutex != NULL)
    {
        pthread_mutex_lock(impl->dnsMutex);
        pthread_cond_signal(&impl->dnsCond);
        pthread_mutex_unlock(impl->dnsMutex);
    }

    CdxTcpStreamDecRef(&impl->base);
    return ;
}

static int StartTcpStreamConnect(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;
    cdx_int32 ret;
    int64_t start, end;
    struct addrinfo *ai = NULL;

    start = GetNowUs();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);
    CDX_FORCE_CHECK(impl);

    CdxAtomicInc(&impl->ref);
    //impl->dnsRet = SDSRequest(impl->hostname, impl->port, &ai, impl, DnsResponeHook);
    ret = SDSRequest(impl->hostname, impl->port, &ai, impl, DnsResponeHook);
    if (ret == SDS_OK)
    {
        CdxTcpStreamDecRef(&impl->base);
        CDX_FORCE_CHECK(ai);
    }
    else if (ret == SDS_PENDING)
    {
        while (1)
        {
            struct timespec abstime;

            abstime.tv_sec = time(0);
            abstime.tv_nsec = 100000000L;

            pthread_mutex_lock(impl->dnsMutex);
            pthread_cond_timedwait(&impl->dnsCond, impl->dnsMutex, &abstime); /* wait 100 ms */
            pthread_mutex_unlock(impl->dnsMutex);

            if (impl->forceStopFlag)
            {
                ai = NULL;
                break;
            }

            if (impl->dnsRet == SDS_OK)
            {
                ai = impl->dnsAI;
                break;
            }
            else if (impl->dnsRet != SDS_PENDING)
            {
                ai = NULL;
                break;
            }

        }

    }
    else
    {
        CdxTcpStreamDecRef(&impl->base);
    }

    if (ai == NULL)
    {
        goto err_out;
    }

#if defined(CONF_YUNOS)
    //* Ali YUNOS invoke info
    //* get tcp IP in control STREAM_CMD_GET_IP
    struct addrinfo *curInfo;
    struct sockaddr_in *addrInfo;
    curInfo = ai;
    cdx_char ipbufInfo[100];
    addrInfo = (struct sockaddr_in *)curInfo->ai_addr;
    memcpy(impl->mTcpIP,inet_ntop(AF_INET, &addrInfo->sin_addr, ipbufInfo, 100),100);
#endif

    int oldFd = (long)pthread_getspecific(oldSocketKey) - 1;
    pthread_setspecific(oldSocketKey, NULL);
    if (oldFd >= 0)
    {
        struct sockaddr_storage peerAddr;
        socklen_t addrlen = sizeof(peerAddr);

        getpeername(oldFd, (struct sockaddr *)&peerAddr, &addrlen);
        /* not robust. should check ai_next in a loop */
        if (addrlen == ai->ai_addrlen &&
                memcmp(&peerAddr, ai->ai_addr, addrlen) == 0)
        {
            char c;
            ret = recv(oldFd, &c, 1, MSG_DONTWAIT);
            if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
            {
                // Todo: set impl->sockRecvBufLen or remove this field completely
                impl->sockFd = oldFd;
                CDX_LOGD("reuse old socket");
                return 0;
            }
            CDX_LOGD("ret %d, error: %s", ret, strerror(errno));
        }

        CDX_LOGD("close old socket");
        close(oldFd);
    }

    do
    {
        impl->sockRecvBufLen = 0;
        impl->sockFd = CdxAsynSocket(ai->ai_family, &impl->sockRecvBufLen);
        if(impl->sockFd < 0)
            continue;

        ret = CdxSockAsynConnect(impl->sockFd, ai->ai_addr, ai->ai_addrlen, 0,
            &impl->forceStopFlag);
        if(ret == 0)
        {
            break;
        }
        else if(ret < 0)
        {
            CDX_LOGE("connect failed. error(%d): %s.", errno, strerror(errno));

#if defined(CONF_YUNOS)
            if(impl->callback)
            {
                impl->mYunOSstatusCode = 3002; //Ali YUNOS invoke info
                impl->callback(impl->pUserData, STREAM_EVT_DOWNLOAD_DOWNLOAD_ERROR,
                    &(impl->mYunOSstatusCode));
            }
#endif
            close(impl->sockFd);
            impl->sockFd = -1;
        }

        if(impl->forceStopFlag == 1)
        {
            CDX_LOGV("force stop connect.");
            goto err_out;
        }
    } while ((ai = ai->ai_next) != NULL);

    if (ai == NULL)
    {
        CDX_LOGE("connect failed. error(%d): %s.", errno, strerror(errno));
        goto err_out;
    }

    end = GetNowUs();
    //CDX_LOGV("Start tcp time(%lld)", end-start);
    return 0;

err_out:
    end = GetNowUs();
    if(errno == 101)
    {
        CDX_LOGD("errno 101, reconnect");
        return -2;
    }
    //CDX_LOGV("Start tcp time(%lld)", end-start);
    return -1;
}

static int tcpGetProbeData(CdxTcpStreamImplT *impl)
{
    int recvSize = 0;
    int ret, len;

    if (impl->probeData.len <= 0 || impl->probeData.len > PROBE_DATA_LEN_DEFAULT)
    {
        impl->probeData.len = PROBE_DATA_LEN_DEFAULT;
    }

    impl->probeData.buf = (cdx_char *)calloc(1, impl->probeData.len);
    if(impl->probeData.buf == NULL)
    {
        loge("calloc fail, impl->probeData.len=%d", impl->probeData.len);
        goto err_out;
    }
    impl->buffer = (cdx_char *)calloc(1, impl->probeData.len);
    if(impl->buffer == NULL)
    {
        loge("calloc fail, impl->probeData.len=%d", impl->probeData.len);
        goto err_out;
    }
    len = impl->probeData.len;
    while(recvSize < len)
    {
        if(impl->forceStopFlag)
        {
            CDX_LOGV("__CdxTcpStreamRead forceStop.");
            goto err_out;
        }
        ret = CdxSockAsynRecv(impl->sockFd, impl->buffer + recvSize,
                                            len - recvSize, 0, &impl->forceStopFlag);
        if(ret < 0)
        {
            CDX_LOGE("__CdxTcpStreamRead error(%d): %s. recvSize(%d)",
                errno, strerror(errno), recvSize);
            goto err_out;
        }
        else if(ret == 0)
        {
            logd("xxx ret = 0.");
            break;
        }
        else
        {
            recvSize += ret;
        }
    }

    impl->probeData.len = recvSize;
    impl->validDataSize = recvSize;
    CDX_LOGD("probe data len %d", impl->probeData.len);
    memcpy(impl->probeData.buf, impl->buffer, recvSize);
    return 0;

err_out:
    if(impl->probeData.buf)
    {
        free(impl->probeData.buf);
        impl->probeData.buf = NULL;
    }
    if(impl->buffer)
    {
        free(impl->buffer);
        impl->buffer = NULL;
    }

    impl->ioState = CDX_IO_STATE_ERROR;
    return -1;
}
static cdx_int32 __CdxTcpStreamConnect(CdxStreamT *stream)
{
    CdxTcpStreamImplT *impl;
    cdx_int32 result;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, CdxTcpStreamImplT, base);

    pthread_mutex_lock(&impl->lock);
    if(impl->forceStopFlag)
    {
        pthread_mutex_unlock(&impl->lock);
        return -1;
    }
    impl->state = TCP_STREAM_CONNECTING;
    CdxAtomicInc(&impl->ref);
    pthread_mutex_unlock(&impl->lock);

    result = StartTcpStreamConnect(stream);
    if (result < 0)
    {
        CDX_LOGE("StartTcpStreamConnect failed!");
        pthread_mutex_lock(&impl->lock);
        impl->ioState = CDX_IO_STATE_ERROR;
        pthread_mutex_unlock(&impl->lock);
    }
    else
    {
	if(impl->needProbe == 1)
        {
            result = tcpGetProbeData(impl);
            if(result < 0)
            {
                CDX_LOGE("get probe data failed.");
            }else{
		CDX_LOGD("get probe data successfully.");
		pthread_mutex_lock(&impl->lock);
		impl->ioState = CDX_IO_STATE_OK;
		pthread_mutex_unlock(&impl->lock);
	    }
        }else{
		pthread_mutex_lock(&impl->lock);
		impl->ioState = CDX_IO_STATE_OK;
		pthread_mutex_unlock(&impl->lock);
	}
    }

    pthread_mutex_lock(&impl->lock);
    impl->state = TCP_STREAM_IDLE;
    CdxTcpStreamDecRef(&impl->base);
    pthread_mutex_unlock(&impl->lock);
    pthread_cond_signal(&impl->cond);
    return (impl->ioState == CDX_IO_STATE_ERROR || impl->forceStopFlag == 1) ? -1 : 0;
}

static struct CdxStreamOpsS CdxTcpStreamOps = {
    .connect    = __CdxTcpStreamConnect,
    .getProbeData = __CdxTcpStreamGetProbeData,
    .read       = __CdxTcpStreamRead,
    .close      = __CdxTcpStreamClose,
    .getIOState = __CdxTcpStreamGetIOState,
//    .forceStop  = __CdxTcpStreamForceStop,
    .attribute  = __CdxTcpStreamAttribute,

    .write      = __CdxTcpStreamWrite,
    .getMetaData = __CdxTcpStreamGetMetaData,
    .eos        = __CdxTcpStreamEos,
    .control    = __CdxTcpStreamControl,
	.tell       = __CdxTcpStreamTell,
	.size       = __CdxTcpStreamSize,
};

static CdxStreamT *__CdxTcpStreamCreate(CdxDataSourceT *source)
{
    CdxTcpStreamImplT *impl = NULL;

    impl = (CdxTcpStreamImplT *)malloc(sizeof(CdxTcpStreamImplT));
    if(NULL == impl)
    {
        CDX_LOGE("malloc failed");
        return NULL;
    }

    memset(impl, 0x00, sizeof(CdxTcpStreamImplT));
    impl->base.ops = &CdxTcpStreamOps;
    impl->ioState = CDX_IO_STATE_INVALID;

    impl->sockFd = -1;
    if(source->extraData != NULL)
    {
	impl->port = *(cdx_int32 *)((CdxHttpSendBufferT *)source->extraData)->size;
        impl->hostname = strdup((char *)((CdxHttpSendBufferT *)source->extraData)->buf);
    }
	else if(source->uri != NULL)
    {
        char hn[1024]={0};
        sscanf(source->uri, "%*[^/]//%768[^:]:%d", hn, &impl->port);
        impl->hostname = strdup(hn);
        impl->needProbe = 1;
        logd("uri(%s), hostname(%s), port(%d)", source->uri, impl->hostname, impl->port);
    }
    else
    {
        loge("no host name, check.");
        abort();
    }
	logd("port(%d), hostname(%s)", impl->port, impl->hostname);
    if(source->uri != NULL)
    {
        impl->sourceUri = strdup(source->uri);
    }
    //CDX_LOGV("port (%d), hostname(%s)", impl->port, impl->hostname);
    CdxAtomicSet(&impl->ref, 1);
    pthread_mutex_init(&impl->lock, NULL);
    pthread_cond_init(&impl->cond, NULL);

    impl->dnsMutex = (pthread_mutex_t*)calloc(1,sizeof(pthread_mutex_t));
    if (impl->dnsMutex == NULL)
    {
        CDX_LOGE("malloc failed");
        return NULL;
    }

    pthread_mutex_init(impl->dnsMutex, NULL);
    pthread_cond_init(&impl->dnsCond, NULL);
    impl->dnsRet = SDS_PENDING;
    impl->state = TCP_STREAM_IDLE;
    pthread_once(&once, createoldSocketKey);

    return &impl->base;
}

CdxStreamCreatorT tcpStreamCtor = {
    .create = __CdxTcpStreamCreate
};
