/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxSSLStream.h
 * Description : SSLStream
 * History :
 *
 */

#ifndef CDX_TCP_STREAM_H
#define CDX_TCP_STREAM_H

#include <CdxStream.h>
#include <CdxAtomic.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SOCKRECVBUF_LEN 512*1024// 262142 (5*1024*1024)
#define closesocket close

typedef struct CdxSSLStreamImpl
{
    CdxStreamT base;
    cdx_int32 ioState;
    cdx_int32 sockRecvBufLen;
    cdx_int32 notBlockFlag;
    cdx_int32 exitFlag;                  //when close, exit
    cdx_int32 forceStopFlag;
    cdx_int32 sockFd;                    //socket fd
    //int eof;                           //all stream data is read from network
    cdx_int32 port;
    cdx_char *hostname;
    cdx_atomic_t ref;                    //reference count, for free resource while still blocking.
    pthread_t threadId;
    cdx_atomic_t state;
    pthread_mutex_t stateLock;
    pthread_cond_t  stateCond;

    SSL *ssl;
    SSL_CTX *ctx;
    //add more
}CdxSSLStreamImplT;
#endif
