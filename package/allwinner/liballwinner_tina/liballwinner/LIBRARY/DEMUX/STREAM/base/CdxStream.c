/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxStream.c
 * Description : Stream base
 * History :
 *
 */

#include <CdxTypes.h>
#include <CdxLog.h>
#include <CdxMemory.h>

#include <CdxStream.h>
#include "cdx_config.h"
#include <CdxList.h>

/*
#define STREAM_DECLARE(scheme) \
    extern CdxStreamCreatorT cdx_##scheme##_stream_ctor

#define STREAM_REGISTER(scheme) \
    {#scheme, &cdx_##scheme##_stream_ctor}
*/

struct CdxStreamListS
{
    CdxListT list;
    int size;
};

struct CdxStreamListS streamList;

extern CdxStreamCreatorT fileStreamCtor;

#if (CONFIG_HAVE_LIVE555 == OPTION_HAVE_LIVE555)
extern CdxStreamCreatorT rtspStreamCtor;
extern CdxStreamCreatorT rtmpStreamCtor;
#endif

extern CdxStreamCreatorT httpStreamCtor;
extern CdxStreamCreatorT tcpStreamCtor;
#ifdef __ANDROID__
extern CdxStreamCreatorT rtmpStreamCtor;
#endif

#if(CONFIG_MMS == OPTION_MMS_ENABLE)
extern CdxStreamCreatorT mmsStreamCtor;
#endif

extern CdxStreamCreatorT udpStreamCtor;
#ifdef __ANDROID__
extern CdxStreamCreatorT rtpStreamCreator;
extern CdxStreamCreatorT customerStreamCtor;
#endif

#if(CONFIG_HAVE_SSL == OPTION_HAVE_SSL)
extern CdxStreamCreatorT sslStreamCtor;
#endif

#ifdef __ANDROID__
extern CdxStreamCreatorT aesStreamCtor;
extern CdxStreamCreatorT bdmvStreamCtor;
extern CdxStreamCreatorT widevineStreamCtor;
extern CdxStreamCreatorT videoResizeStreamCtor;
extern CdxStreamCreatorT DTMBStreamCtor;
#endif

struct CdxStreamNodeS
{
    CdxListNodeT node;
    const cdx_char *scheme;
    CdxStreamCreatorT *creator;
};

int AwStreamRegister(CdxStreamCreatorT *creator, cdx_char *type)
{
    struct CdxStreamNodeS *streamNode;

    streamNode = malloc(sizeof(*streamNode));
    streamNode->creator = creator;
    streamNode->scheme = type;

    CdxListAddTail(&streamNode->node, &streamList.list);
    streamList.size++;
    return 0;
}

static cdx_void AwStreamInit(cdx_void) __attribute__((constructor));
cdx_void AwStreamInit(cdx_void)
{
    CDX_LOGD("aw stream init...");
    CdxListInit(&streamList.list);
    streamList.size = 0;

    AwStreamRegister(&fileStreamCtor,"fd");
    AwStreamRegister(&fileStreamCtor,"file");
#if (CONFIG_HAVE_LIVE555 == OPTION_HAVE_LIVE555)
    AwStreamRegister(&rtspStreamCtor,"rtsp");
	AwStreamRegister(&rtmpStreamCtor,"rtmp");
#endif
    AwStreamRegister(&httpStreamCtor,"http");
#if(CONFIG_HAVE_SSL == OPTION_HAVE_SSL)
    AwStreamRegister(&httpStreamCtor,"https");
#endif
    AwStreamRegister(&tcpStreamCtor,"tcp");
#ifdef __ANDROID__
    AwStreamRegister(&rtmpStreamCtor,"rtmp");
#endif

#if(CONFIG_MMS == OPTION_MMS_ENABLE)
    AwStreamRegister(&mmsStreamCtor,"mms");
    AwStreamRegister(&mmsStreamCtor,"mmsh");
    AwStreamRegister(&mmsStreamCtor,"mmst");
    AwStreamRegister(&mmsStreamCtor,"mmshttp");
#endif

    AwStreamRegister(&udpStreamCtor,"udp");
#ifdef __ANDROID__
    AwStreamRegister(&customerStreamCtor,"customer");
#endif

#if(CONFIG_HAVE_SSL == OPTION_HAVE_SSL)
    AwStreamRegister(&sslStreamCtor,"ssl");
#endif

#ifdef __ANDROID__
    AwStreamRegister(&aesStreamCtor,"aes");
    AwStreamRegister(&bdmvStreamCtor,"bdmv");
    AwStreamRegister(&DTMBStreamCtor,"dtmb");
#endif
#ifdef __ANDROID__
    AwStreamRegister(&widevineStreamCtor,"widevine");
    AwStreamRegister(&videoResizeStreamCtor,"videoResize");
#endif
    CDX_LOGD("stream list size:%d",streamList.size);
    return ;
}

CdxStreamT *CdxStreamCreate(CdxDataSourceT *source)
{
    cdx_char scheme[16] = {0};
    cdx_char *colon;
    CdxStreamCreatorT *ctor = NULL;
    struct CdxStreamNodeS *streamNode;

    colon = strchr(source->uri, ':');
    CDX_CHECK(colon && (colon - source->uri < 16));
    memcpy(scheme, source->uri, colon - source->uri);

    CdxListForEachEntry(streamNode, &streamList.list, node)
    {
        CDX_CHECK(streamNode->creator);
        if (strcasecmp(streamNode->scheme, scheme) == 0)
        {
            ctor = streamNode->creator;
            break;
        }
    }

    if (NULL == ctor)
    {
        CDX_LOGE("unsupport stream. scheme(%s)", scheme);
        return NULL;
    }

    CDX_CHECK(ctor->create);
    CdxStreamT *stream = ctor->create(source);
    if (!stream)
    {
        CDX_LOGE("open stream fail, uri(%s)", source->uri);
        return NULL;
    }

    return stream;
}

int CdxStreamOpen(CdxDataSourceT *source, pthread_mutex_t *mutex, cdx_bool *exit,
        CdxStreamT **stream, ContorlTask *streamTasks)
{
    if(mutex)
        pthread_mutex_lock(mutex);
    if(exit && *exit)
    {
        CDX_LOGW("open stream user cancel.");
        if(mutex) pthread_mutex_unlock(mutex);
        return -1;
    }
    *stream = CdxStreamCreate(source);
    if(mutex)
        pthread_mutex_unlock(mutex);
    if (!*stream)
    {
        CDX_LOGW("open stream failure.");
        return -1;
    }
    int ret;
    while(streamTasks)
    {
        ret = CdxStreamControl(*stream, streamTasks->cmd, streamTasks->param);
        if(ret < 0)
        {
            CDX_LOGE("CdxStreamControl fail, cmd=%d", streamTasks->cmd);
        }
        streamTasks = streamTasks->next;
    }
    return CdxStreamConnect(*stream);
}

typedef struct KeyValuePairS {
    char *key;
    char *val;
} KeyValuePairT;

struct CdxKeyedVectorS {
    int size;
    int index;
    KeyValuePairT item[0];
};

CdxKeyedVectorT *CdxKeyedVectorCreate(int num)
{
    CdxKeyedVectorT *p;

    if (num <= 0)
        return NULL;

    p = calloc(1, sizeof(*p) + num * sizeof(KeyValuePairT));
    if (p == NULL)
        return NULL;

    p->size = num;
    p->index = -1;
    return p;
}

void CdxKeyedVectorDestroy(CdxKeyedVectorT *keyedVector)
{
    int i;

    if (keyedVector == NULL)
        return;

    for (i = 0; i <= keyedVector->index; i++)
    {
        free(keyedVector->item[i].key);
        free(keyedVector->item[i].val);
    }
    free(keyedVector);
}

int CdxKeyedVectorAdd(CdxKeyedVectorT *keyedVector,
        const char *key, const char *val)
{
    if (key == NULL && val == NULL)
        return 0;

    if (keyedVector->index >= keyedVector->size - 1)
    {
        CDX_LOGW("keyedVector is full");
        return -1;
    }

    int index = keyedVector->index + 1;
    if (key != NULL)
    {
        keyedVector->item[index].key = strdup(key);
        if (keyedVector->item[index].key == NULL)
            goto err;
    }
    if (val != NULL)
    {
        keyedVector->item[index].val = strdup(val);
        if (keyedVector->item[index].val == NULL)
            goto err;
    }

    keyedVector->index++;
    return 0;

err:
    free(keyedVector->item[index].key);
    free(keyedVector->item[index].val);
    logw("strdup error");
    return -1;
}

int CdxKeyedVectorGetSize(CdxKeyedVectorT *keyedVector)
{
    return keyedVector->size;
}

char *CdxKeyedVectorGetKey(CdxKeyedVectorT *keyedVector, int index)
{
    if (index >= 0 && index <= keyedVector->index)
        return keyedVector->item[index].key;

    return NULL;
}

char *CdxKeyedVectorGetValue(CdxKeyedVectorT *keyedVector, int index)
{
    if (index >= 0 && index <= keyedVector->index)
        return keyedVector->item[index].val;

    return NULL;
}
