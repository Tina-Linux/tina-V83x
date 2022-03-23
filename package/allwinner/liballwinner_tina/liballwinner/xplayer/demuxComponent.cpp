/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : demuxComponent.cpp
 * Description : demuxComponent
 * History :
 *
 */


#include <semaphore.h>
#include <pthread.h>
#include "demuxComponent.h"
#include "awMessageQueue.h"
#include "CdxParser.h"          //* parser library in "LIBRARY/DEMUX/PARSER/include/"
#include "CdxStream.h"          //* parser library in "LIBRARY/DEMUX/STREAM/include/"
#include "player.h"             //* player library in "LIBRARY/PLAYER/"
#include "cache.h"
#include "log.h"
#include "cdx_config.h"

#define CONFIG_DISABLE_VIDEO 0
#define CONFIG_DISABLE_AUDIO 0
#define CONFIG_DISABLE_SUBTITLE 0

//* demux status, same with the awplayer.
static const int DEMUX_STATUS_IDLE        = 0;      //* the very beginning status.
static const int DEMUX_STATUS_INITIALIZED = 1<<0;   //* after data source set.
static const int DEMUX_STATUS_PREPARING   = 1<<1;   //* when preparing.
static const int DEMUX_STATUS_PREPARED    = 1<<2;   //* after parser is opened and media info get.
static const int DEMUX_STATUS_STARTED     = 1<<3;   //* parsing and sending data.
static const int DEMUX_STATUS_PAUSED      = 1<<4;   //* sending job paused.
static const int DEMUX_STATUS_STOPPED     = 1<<5;   //* parser closed.
static const int DEMUX_STATUS_COMPLETE    = 1<<6;   //* all data parsed.

//* command id, same with the awplayer.
static const int DEMUX_COMMAND_SET_SOURCE     = 0x101;
static const int DEMUX_COMMAND_PREPARE        = 0x104;
static const int DEMUX_COMMAND_START          = 0x105;
static const int DEMUX_COMMAND_PAUSE          = 0x106;
static const int DEMUX_COMMAND_STOP           = 0x107;
static const int DEMUX_COMMAND_QUIT           = 0x109;
static const int DEMUX_COMMAND_SEEK           = 0x10a;
static const int DEMUX_COMMAND_CLEAR          = 0x10b;
static const int DEMUX_COMMAND_CANCEL_PREPARE = 0x10c;
static const int DEMUX_COMMAND_CANCEL_SEEK    = 0x10d;
static const int DEMUX_COMMAND_READ           = 0x10e;

//* cache start play size and max buffer size.
static const int CACHE_START_PLAY_SIZE               = 128*1024;
static const int CACHE_START_PLAY_SIZE_WITHOUT_VIDEO = 1*1024;
static const int CACHE_MAX_BUFFER_SIZE = 20*1024*1024;

static void* DemuxThread(void* arg);
static void* CacheThread(void* arg);
static int setDataSourceFields(CdxDataSourceT* source, char* uri, map<string,string>* pHeader);
static void clearDataSourceFields(CdxDataSourceT* source);
static int setMediaInfo(MediaInfo* pMediaInfo, CdxMediaInfoT* pInfoFromParser);
static void clearMediaInfo(MediaInfo* pMediaInfo);
static int PlayerBufferOverflow(Player* p);
static int PlayerBufferUnderflow(Player* p);

typedef struct DemuxCompContext_t
{
    int                 eStatus;
    int                 bLiveStream;        //* live streaming from network.
    int                 bFileStream;        //* local media file.
    int                 bVodStream;         //* vod stream from network.

    //* data source.
    int                 nSourceType;        //* url or fd or IStreamSource.
    CdxDataSourceT      source;
    MediaInfo           mediaInfo;

    pthread_t           threadId;
    AwMessageQueue*     mq;

    CdxParserT*         pParser;
    CdxStreamT*            pStream;
    Player*             pPlayer;
    DemuxCallback       callback;
    void*               pUserData;
    int                 nCacheStatReportIntervalMs;

    pthread_mutex_t     mutex;
    sem_t               semSetDataSource;
    sem_t               semStart;
    sem_t               semStop;
    sem_t               semQuit;
    sem_t               semClear;
    sem_t               semCancelPrepare;
    sem_t               semCancelSeek;

    int                 nSetDataSourceReply;
    int                 nStartReply;
    int                 nStopReply;

    pthread_t           cacheThreadId;
    AwMessageQueue*     mqCache;
    sem_t               semCache;
    int                 nCacheReply;
    StreamCache*        pCache;
    int                 bBufferring;
    int                 bEOS;
    int                 bIOError;

    int                 bCancelPrepare;
    int                 bCancelSeek;
    int                 bSeeking;
    int                 bStopping;

}DemuxCompContext;


DemuxComp* DemuxCompCreate(void)
{
    DemuxCompContext* d;

    d = (DemuxCompContext*)malloc(sizeof(DemuxCompContext));
    if(d == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(d, 0, sizeof(DemuxCompContext));

    d->nCacheStatReportIntervalMs = 1000;

    pthread_mutex_init(&d->mutex, NULL);
    sem_init(&d->semSetDataSource, 0, 0);
    sem_init(&d->semStart, 0, 0);
    sem_init(&d->semStop, 0, 0);
    sem_init(&d->semQuit, 0, 0);
    sem_init(&d->semClear, 0, 0);
    sem_init(&d->semCancelPrepare, 0, 0);
    sem_init(&d->semCancelSeek, 0, 0);
    sem_init(&d->semCache, 0, 0);

    d->mq = AwMessageQueueCreate(64);
    if(d->mq == NULL)
    {
        loge("AwMessageQueueCreate() return fail.");
        pthread_mutex_destroy(&d->mutex);
        sem_destroy(&d->semSetDataSource);
        sem_destroy(&d->semStart);
        sem_destroy(&d->semStop);
        sem_destroy(&d->semQuit);
        sem_destroy(&d->semClear);
        sem_destroy(&d->semCancelPrepare);
        sem_destroy(&d->semCancelSeek);
        sem_destroy(&d->semCache);
        free(d);
        return NULL;
    }

    d->mqCache = AwMessageQueueCreate(64);
    if(d->mqCache == NULL)
    {
        loge("AwMessageQueueCreate() return fail.");
        AwMessageQueueDestroy(d->mq);
        pthread_mutex_destroy(&d->mutex);
        sem_destroy(&d->semSetDataSource);
        sem_destroy(&d->semStart);
        sem_destroy(&d->semStop);
        sem_destroy(&d->semQuit);
        sem_destroy(&d->semClear);
        sem_destroy(&d->semCancelPrepare);
        sem_destroy(&d->semCancelSeek);
        sem_destroy(&d->semCache);
        free(d);
        return NULL;
    }

    if(pthread_create(&d->threadId, NULL, DemuxThread, (void*)d) != 0)
    {
        loge("can not create thread for demux component.");
        AwMessageQueueDestroy(d->mq);
        AwMessageQueueDestroy(d->mqCache);
        pthread_mutex_destroy(&d->mutex);
        sem_destroy(&d->semSetDataSource);
        sem_destroy(&d->semStart);
        sem_destroy(&d->semStop);
        sem_destroy(&d->semQuit);
        sem_destroy(&d->semClear);
        sem_destroy(&d->semCancelPrepare);
        sem_destroy(&d->semCancelSeek);
        free(d);
        return NULL;
    }

    d->pCache = StreamCacheCreate();
    StreamCacheSetSize(d->pCache, CACHE_START_PLAY_SIZE, CACHE_MAX_BUFFER_SIZE);

    return (DemuxComp*)d;
}


void DemuxCompDestroy(DemuxComp* d)
{
    void* status;

    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    //* send a quit message.
    demux->bStopping = 1;
    setMessage(&msg, DEMUX_COMMAND_QUIT, (uintptr_t)&demux->semQuit);
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semQuit, -1);

    pthread_join(demux->threadId, &status);

    StreamCacheDestroy(demux->pCache);

    clearMediaInfo(&demux->mediaInfo);

    if(demux->mq != NULL)
        AwMessageQueueDestroy(demux->mq);

    if(demux->mqCache != NULL)
        AwMessageQueueDestroy(demux->mqCache);

    pthread_mutex_destroy(&demux->mutex);
    sem_destroy(&demux->semSetDataSource);
    sem_destroy(&demux->semStart);
    sem_destroy(&demux->semStop);
    sem_destroy(&demux->semQuit);
    sem_destroy(&demux->semClear);
    sem_destroy(&demux->semCancelPrepare);
    sem_destroy(&demux->semCancelSeek);
    sem_destroy(&demux->semCache);
    free(demux);

    return;
}


void DemuxCompClear(DemuxComp* d)  //* clear the data source, like just created.
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;
    //* send clear message.
    demux->bStopping = 1;
    setMessage(&msg, DEMUX_COMMAND_CLEAR, (uintptr_t)&demux->semClear);
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semClear, -1);

    return;
}

int DemuxCompSetUrlSource(DemuxComp* d, const char* pUrl, const map<string, string>* pHeaders)
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    //* send a set data source message.
    setMessage(&msg,
               DEMUX_COMMAND_SET_SOURCE,                    //* message id.
               (uintptr_t)&demux->semSetDataSource,      //* params[0] = &semSetDataSource.
               (uintptr_t)&demux->nSetDataSourceReply,   //* params[1] = &nSetDataSourceReply.
               SOURCE_TYPE_URL,                             //* params[2] = SOURCE_TYPE_URL.
               (uintptr_t)pUrl,           //* params[3] = pUrl.
               (uintptr_t)pHeaders);     //* params[4] = KeyedVector<String8, String8>* pHeaders;
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semSetDataSource, -1);

    return demux->nSetDataSourceReply;
}


int DemuxCompSetFdSource(DemuxComp* d, int fd, int64_t nOffset, int64_t nLength)
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    //* send a set data source message.
    setMessage(&msg,
               DEMUX_COMMAND_SET_SOURCE,                    //* message id.
               (uintptr_t)&demux->semSetDataSource,      //* params[0] = &semSetDataSource.
               (uintptr_t)&demux->nSetDataSourceReply,   //* params[1] = &nSetDataSourceReply.
               SOURCE_TYPE_FD,                              //* params[2] = SOURCE_TYPE_FD.
               fd,                                          //* params[3] = fd.
               (unsigned int)(nOffset>>32),                 //* params[4] = high 32 bits of offset.
               (unsigned int)(nOffset & 0xffffffff),        //* params[5] = low 32 bits of offset.
               (unsigned int)(nLength>>32),                 //* params[6] = high 32 bits of length.
               (unsigned int)(nLength & 0xffffffff));       //* params[7] = low 32 bits of length.
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semSetDataSource, -1);

    return demux->nSetDataSourceReply;
}


int DemuxCompSetPlayer(DemuxComp* d, Player* player)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    demux->pPlayer = player;
    return 0;
}


int DemuxCompSetCallback(DemuxComp* d, DemuxCallback callback, void* pUserData)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    demux->callback  = callback;
    demux->pUserData = pUserData;
    return 0;
}


int DemuxCompPrepareAsync(DemuxComp* d)
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    demux->bCancelPrepare = 0;
    demux->eStatus = DEMUX_STATUS_PREPARING;

    //* send a prepare message.
    setMessage(&msg, DEMUX_COMMAND_PREPARE);
    AwMessageQueuePostMessage(demux->mq, &msg);
    return 0;
}


int DemuxCompCancelPrepare(DemuxComp* d)   //* should call back DEMUX_PREPARE_FINISH message.
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    demux->bCancelPrepare = 1;      //* set this flag to make the parser quit preparing.

    pthread_mutex_lock(&demux->mutex);
	if(demux->pParser)
	{
		CdxParserForceStop(demux->pParser);
	}
	else if(demux->pStream)
	{
		CdxStreamForceStop(demux->pStream);
	}
	pthread_mutex_unlock(&demux->mutex);

    //* send a prepare.
    setMessage(&msg,
               DEMUX_COMMAND_CANCEL_PREPARE,            //* message id.
               (uintptr_t)&demux->semCancelPrepare,  //* params[0] = &semCancelPrepare.
               0);                                      //* no reply.
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semCancelPrepare, -1);
    return 0;
}


MediaInfo* DemuxCompGetMediaInfo(DemuxComp* d)
{
    DemuxCompContext*   demux;
    MediaInfo*          mi;
    MediaInfo*          myMediaInfo;
    int                 i;
    VideoStreamInfo*    pVideoStreamInfo;
    AudioStreamInfo*    pAudioStreamInfo;
    SubtitleStreamInfo* pSubtitleStreamInfo;
    int                 nCodecSpecificDataLen;
    char*               pCodecSpecificData;

    demux = (DemuxCompContext*)d;

    myMediaInfo = &demux->mediaInfo;
    mi = (MediaInfo*)malloc(sizeof(MediaInfo));
    if(mi == NULL)
    {
        loge("can not alloc memory for media info.");
        return NULL;
    }
    memset(mi, 0, sizeof(MediaInfo));
    mi->nFileSize      = myMediaInfo->nFileSize;
    mi->nDurationMs    = myMediaInfo->nDurationMs;
    mi->eContainerType = myMediaInfo->eContainerType;
    mi->bSeekable      = myMediaInfo->bSeekable;

    mi->albumsz = myMediaInfo->albumsz;
    mi->albumCharEncode = myMediaInfo->albumCharEncode;
    memcpy(mi->album, myMediaInfo->album, mi->albumsz);

    mi->authorsz = myMediaInfo->authorsz;
    mi->authorCharEncode = myMediaInfo->authorCharEncode;
    memcpy(mi->author, myMediaInfo->author, mi->authorsz);

    mi->genresz = myMediaInfo->genresz;
    mi->genreCharEncode = myMediaInfo->genreCharEncode;
    memcpy(mi->genre, myMediaInfo->genre, mi->genresz);

    mi->titlesz = myMediaInfo->titlesz;
    mi->titleCharEncode = myMediaInfo->titleCharEncode;
    memcpy(mi->title, myMediaInfo->title, mi->titlesz);

    logv("video stream num = %d, video stream info = %p",
            myMediaInfo->nVideoStreamNum, myMediaInfo->pVideoStreamInfo);

    if(myMediaInfo->nVideoStreamNum > 0)
    {
        pVideoStreamInfo = (VideoStreamInfo*)malloc(
                sizeof(VideoStreamInfo)*myMediaInfo->nVideoStreamNum);
        if(pVideoStreamInfo == NULL)
        {
            loge("can not alloc memory for media info.");
            clearMediaInfo(mi);
            return NULL;
        }
        memset(pVideoStreamInfo, 0, sizeof(VideoStreamInfo)*myMediaInfo->nVideoStreamNum);
        mi->pVideoStreamInfo = pVideoStreamInfo;

        for(i=0; i<myMediaInfo->nVideoStreamNum; i++)
        {
            pVideoStreamInfo = &mi->pVideoStreamInfo[i];
            memcpy(pVideoStreamInfo, &myMediaInfo->pVideoStreamInfo[i], sizeof(VideoStreamInfo));

            pCodecSpecificData    = pVideoStreamInfo->pCodecSpecificData;
            nCodecSpecificDataLen = pVideoStreamInfo->nCodecSpecificDataLen;
            pVideoStreamInfo->pCodecSpecificData    = NULL;
            pVideoStreamInfo->nCodecSpecificDataLen = 0;

            if(pCodecSpecificData != NULL && nCodecSpecificDataLen > 0)
            {
                pVideoStreamInfo->pCodecSpecificData = (char*)malloc(nCodecSpecificDataLen);
                if(pVideoStreamInfo->pCodecSpecificData == NULL)
                {
                    loge("can not alloc memory for media info.");
                    clearMediaInfo(mi);
                    return NULL;
                }

                memcpy(pVideoStreamInfo->pCodecSpecificData,
                        pCodecSpecificData, nCodecSpecificDataLen);
                pVideoStreamInfo->nCodecSpecificDataLen = nCodecSpecificDataLen;
            }
        }

        mi->nVideoStreamNum = myMediaInfo->nVideoStreamNum;
    }

    logv("video stream num = %d, video stream info = %p",
            mi->nVideoStreamNum, mi->pVideoStreamInfo);

    if(myMediaInfo->nAudioStreamNum > 0)
    {
        pAudioStreamInfo = (AudioStreamInfo*)malloc(
                sizeof(AudioStreamInfo)*myMediaInfo->nAudioStreamNum);
        if(pAudioStreamInfo == NULL)
        {
            loge("can not alloc memory for media info.");
            clearMediaInfo(mi);
            return NULL;
        }
        memset(pAudioStreamInfo, 0, sizeof(AudioStreamInfo)*myMediaInfo->nAudioStreamNum);
        mi->pAudioStreamInfo = pAudioStreamInfo;

        for(i=0; i<myMediaInfo->nAudioStreamNum; i++)
        {
            pAudioStreamInfo = &mi->pAudioStreamInfo[i];
            memcpy(pAudioStreamInfo, &myMediaInfo->pAudioStreamInfo[i], sizeof(AudioStreamInfo));

            pCodecSpecificData    = pAudioStreamInfo->pCodecSpecificData;
            nCodecSpecificDataLen = pAudioStreamInfo->nCodecSpecificDataLen;
            pAudioStreamInfo->pCodecSpecificData    = NULL;
            pAudioStreamInfo->nCodecSpecificDataLen = 0;

            if(pCodecSpecificData != NULL && nCodecSpecificDataLen > 0)
            {
                pAudioStreamInfo->pCodecSpecificData = (char*)malloc(nCodecSpecificDataLen);
                if(pAudioStreamInfo->pCodecSpecificData == NULL)
                {
                    loge("can not alloc memory for media info.");
                    clearMediaInfo(mi);
                    return NULL;
                }

                memcpy(pAudioStreamInfo->pCodecSpecificData, pCodecSpecificData,
                                nCodecSpecificDataLen);
                pAudioStreamInfo->nCodecSpecificDataLen = nCodecSpecificDataLen;
            }
        }

        mi->nAudioStreamNum = myMediaInfo->nAudioStreamNum;
    }

    if(myMediaInfo->nSubtitleStreamNum > 0)
    {
        pSubtitleStreamInfo = (SubtitleStreamInfo*)malloc(
                sizeof(SubtitleStreamInfo)*myMediaInfo->nSubtitleStreamNum);
        if(pSubtitleStreamInfo == NULL)
        {
            loge("can not alloc memory for media info.");
            clearMediaInfo(mi);
            return NULL;
        }
        memset(pSubtitleStreamInfo, 0, sizeof(SubtitleStreamInfo)*myMediaInfo->nSubtitleStreamNum);
        mi->pSubtitleStreamInfo = pSubtitleStreamInfo;

        for(i=0; i<myMediaInfo->nSubtitleStreamNum; i++)
        {
            pSubtitleStreamInfo = &mi->pSubtitleStreamInfo[i];
            memcpy(pSubtitleStreamInfo, &myMediaInfo->pSubtitleStreamInfo[i],
                        sizeof(SubtitleStreamInfo));

            //* parser only process imbedded subtitle stream in media file.
            pSubtitleStreamInfo->pUrl  = NULL;
            pSubtitleStreamInfo->fd    = -1;
            pSubtitleStreamInfo->fdSub = -1;
        }

        mi->nSubtitleStreamNum = myMediaInfo->nSubtitleStreamNum;
    }

    return mi;
}


int DemuxCompStart(DemuxComp* d)
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    if(demux->eStatus == DEMUX_STATUS_STARTED ||
       demux->eStatus == DEMUX_STATUS_COMPLETE)
    {
        logv("demux component already in started or complete status.");
        return 0;
    }

    if(pthread_equal(pthread_self(), demux->threadId))
    {
        //* called from demux callback to awplayer.
        if(demux->bSeeking)
        {
            demux->eStatus = DEMUX_STATUS_STARTED;
        }
        return 0;
    }

    //* send a start message.
    setMessage(&msg,
               DEMUX_COMMAND_START,                 //* message id.
               (uintptr_t)&demux->semStart,      //* params[0] = &SemStart.
               (uintptr_t)&demux->nStartReply);  //* params[1] = &nStartReply.
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semStart, -1);
    return demux->nStartReply;
}


int DemuxCompStop(DemuxComp* d)    //* close the data source, must call prepare again to restart.
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    demux->bStopping = 1;
    if(demux->pParser != NULL)
        CdxParserForceStop(demux->pParser); //* quit from reading or seeking.
	else if(demux->pStream != NULL)
		CdxStreamForceStop(demux->pStream);
    //* send a start message.
    setMessage(&msg,
               DEMUX_COMMAND_STOP,                  //* message id.
               (uintptr_t)&demux->semStop,       //* params[0] = &mSemStart.
               (uintptr_t)&demux->nStopReply);   //* params[1] = &mStartReply.
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semStop, -1);
    return demux->nStopReply;
}


int DemuxCompPause(DemuxComp* d)   //* no pause status in demux component, return OK immediately.
{
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;
    if(demux->eStatus != DEMUX_STATUS_STARTED)
    {
        logw("invalid pause operation, demux component not in started status.");
        return -1;
    }

    //* currently the demux component has no pause status, it will keep sending data until stopped.
    return 0;
}


int DemuxCompGetStatus(DemuxComp* d)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    return demux->eStatus;
}


int DemuxCompSeekTo(DemuxComp* d, int mSec)
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    demux->bCancelSeek = 0;
    demux->bSeeking = 1;
    if(demux->pParser != NULL && demux->eStatus == DEMUX_STATUS_STARTED)
        CdxParserForceStop(demux->pParser); //* quit from reading.

    //* send a start message.
    setMessage(&msg, DEMUX_COMMAND_SEEK, 0, 0, mSec);
    AwMessageQueuePostMessage(demux->mq, &msg);
    return 0;
}


int DemuxCompCancelSeek(DemuxComp* d)  //* should not call back DEMUX_SEEK_FINISH message.
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    demux->bCancelSeek = 1;
    if(demux->pParser != NULL)
        CdxParserForceStop(demux->pParser);

    //* send a prepare.
    setMessage(&msg, DEMUX_COMMAND_CANCEL_SEEK,
                (uintptr_t)&demux->semCancelSeek);
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semCancelSeek, -1);
    return 0;
}


int DemuxCompSetCacheStatReportInterval(DemuxComp* d, int ms)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    demux->nCacheStatReportIntervalMs = ms;
    return 0;
}


int DemuxCompSetCacheSize(DemuxComp* d, int nStartPlaySize, int nMaxBufferSize)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    StreamCacheSetSize(demux->pCache, nStartPlaySize, nMaxBufferSize);
    return 0;
}


static void* DemuxThread(void* arg)
{
    AwMessage         msg;
    AwMessage         newMsg;
    int               ret;
    sem_t*            pReplySem;
    int*              pReplyValue;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)arg;

    while(1)
    {
        if(AwMessageQueueGetMessage(demux->mq, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }

process_message:
        pReplySem   = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == DEMUX_COMMAND_SET_SOURCE)
        {
            logv("process message DEMUX_COMMAND_SET_SOURCE.");

            demux->nSourceType = (int)msg.params[2];

            if(demux->nSourceType == SOURCE_TYPE_URL)
            {
                //* data source of url path.
                char*                          uri;
                map<string, string>*           pHeaders;

                uri = (char*)msg.params[3];
                pHeaders = (map<string, string>*)msg.params[4];

                if(setDataSourceFields(&demux->source, uri, pHeaders) == 0)
                {
                    demux->eStatus = DEMUX_STATUS_INITIALIZED;
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                }
                else
                {
                    demux->eStatus = DEMUX_STATUS_IDLE;
                    if(pReplyValue != NULL)
                        *pReplyValue = -1;
                }

                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }
            else if(demux->nSourceType == SOURCE_TYPE_FD)
            {
                //* data source is a file descriptor.
                int     fd;
                long long nOffset;
                long long nLength;
                char    str[128];

                clearDataSourceFields(&demux->source);

                fd = msg.params[3];
                nOffset = msg.params[4];
                nOffset<<=32;
                nOffset |= msg.params[5];
                nLength = msg.params[6];
                nLength<<=32;
                nLength |= msg.params[7];

                sprintf(str, "fd://%d?offset=%lld&length=%lld", fd, nOffset, nLength);
                demux->source.uri = strdup(str);
                if(demux->source.uri != NULL)
                {
                    demux->eStatus = DEMUX_STATUS_INITIALIZED;
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                }
                else
                {
                    loge("can not dump string to represent fd source.");
                    demux->eStatus = DEMUX_STATUS_IDLE;
                    if(pReplyValue != NULL)
                        *pReplyValue = -1;
                }

                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }
            else
            {
                //* data source of IStringSource interface.
                //* not supported in linux demo.
                exit(1);
            }
        } //* end DEMUX_COMMAND_SET_SOURCE.
        else if(msg.messageId == DEMUX_COMMAND_PREPARE)
        {
            int flags;

            logv("process message DEMUX_COMMAND_PREPARE.");

            if(demux->pParser != NULL)
            {
                //* should not run here, pParser should be NULL under INITIALIZED or STOPPED status.
                logw("demux->pParser != NULL when DEMUX_COMMAND_PREPARE message received.");
                CdxParserClose(demux->pParser);
                demux->pParser = NULL;
				demux->pStream = NULL;
            }else if(demux->pStream != NULL){
				CdxStreamClose(demux->pStream);
				demux->pStream = NULL;
			}

            flags  = 0;
#if DEMO_CONFIG_DISABLE_SUBTITLE
            flags |= DISABLE_SUBTITLE;
#endif
#if DEMO_CONFIG_DISABLE_AUDIO
            flags |= DISABLE_AUDIO;
#endif
#if DEMO_CONFIG_DISABLE_VIDEO
            flags |= DISABLE_VIDEO;
#endif
#if DEMO_CONFIG_DISALBE_MULTI_AUDIO
            flags |= MUTIL_AUDIO;
#endif
            ret = CdxParserPrepare(&demux->source, flags, &demux->mutex, &demux->bCancelPrepare,
                                      &demux->pParser, &demux->pStream, NULL, NULL );
            if(demux->pParser != NULL && ret >= 0)
            {
                CdxMediaInfoT parserMediaInfo;
                memset(&parserMediaInfo, 0, sizeof(CdxMediaInfoT));
                CdxParserGetMediaInfo(demux->pParser, &parserMediaInfo);
                setMediaInfo(&demux->mediaInfo, &parserMediaInfo);
                demux->mediaInfo.eContainerType = (enum ECONTAINER)demux->pParser->type;

                demux->bEOS     = 0;
                demux->bIOError = 0;

                if(demux->nSourceType == SOURCE_TYPE_URL)
                {
                    if(strncasecmp(demux->source.uri, "file://", 7) == 0)
                        demux->bFileStream = 1;
                    else if(demux->mediaInfo.nDurationMs == 0)
                        demux->bLiveStream = 1;
                    else
                        demux->bVodStream = 1;
                }
                else if(demux->nSourceType == SOURCE_TYPE_FD)
                    demux->bFileStream = 1;
                else
                    demux->bLiveStream = 1;     //* treat IStreamSource(miracast) as a live stream.

                if(demux->bFileStream == 0) //* start the cache trhead for netstream.
                {
                    if(pthread_create(&demux->cacheThreadId, NULL, CacheThread, (void*)demux) == 0)
                    {
                        //* send a fetch message to start the cache loop.
                        setMessage(&newMsg, DEMUX_COMMAND_START);
                        AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                    }
                    else
                        demux->cacheThreadId = 0;
                }

                demux->eStatus = DEMUX_STATUS_PREPARED;
                if(demux->bFileStream == 0)
                {
                    logd("+++++++++++++++++++++ demux->mediaInfo.nVideoStreamNum: %d",
                                demux->mediaInfo.nVideoStreamNum);
                    if(demux->mediaInfo.nVideoStreamNum == 0)
                    {
                        StreamCacheSetSize(demux->pCache, CACHE_START_PLAY_SIZE_WITHOUT_VIDEO,
                                    CACHE_MAX_BUFFER_SIZE);
                    }
                    else
                    {
                        StreamCacheSetSize(demux->pCache, CACHE_START_PLAY_SIZE,
                                        CACHE_MAX_BUFFER_SIZE);
                    }
                }
                demux->callback(demux->pUserData, DEMUX_NOTIFY_PREPARED, 0);
            }
            else
            {
                demux->eStatus = DEMUX_STATUS_INITIALIZED;
                if(demux->bCancelPrepare)
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_PREPARED,
                                (void*)DEMUX_ERROR_USER_CANCEL);
                else
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_PREPARED, (void*)DEMUX_ERROR_IO);
            }

            continue;
        } //* end DEMUX_COMMAND_PREPARE.
        else if(msg.messageId == DEMUX_COMMAND_START)
        {
            logv("process message DEMUX_COMMAND_START.");

            if(demux->eStatus != DEMUX_STATUS_PREPARED)
            {
                loge("demux not in prepared or paused \
                        status when DEMUX_COMMAND_START message received.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            demux->eStatus = DEMUX_STATUS_STARTED;
            //* send a read message to start the read loop.
            setMessage(&newMsg, DEMUX_COMMAND_READ);
            AwMessageQueuePostMessage(demux->mq, &newMsg);
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end DEMUX_COMMAND_START
        else if(msg.messageId == DEMUX_COMMAND_STOP)
        {
            logv("process message DEMUX_COMMAND_STOP.");

            //* stop the cache thread.
            if(demux->cacheThreadId != 0)
            {
                void* status;
                setMessage(&newMsg, DEMUX_COMMAND_QUIT, (uintptr_t)&demux->semCache);
                AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                SemTimedWait(&demux->semCache, -1);
                pthread_join(demux->cacheThreadId, &status);
                demux->cacheThreadId = 0;
            }

            if(demux->pParser != NULL)
            {
                CdxParserClose(demux->pParser);
                demux->pParser = NULL;
				demux->pStream = NULL;
            }else if(demux->pStream != NULL){
				CdxStreamClose(demux->pStream);
				demux->pStream = NULL;
			}

            demux->eStatus = DEMUX_STATUS_STOPPED;
            demux->bStopping = 0;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end DEMUX_COMMAND_STOP.
        else if(msg.messageId == DEMUX_COMMAND_QUIT || msg.messageId == DEMUX_COMMAND_CLEAR)
        {
            logd("process message DEMUX_COMMAND_QUIT or DEMUX_COMMAND_CLEAR.");

            //* stop the cache thread if it is not stopped yet.
            if(demux->cacheThreadId != 0)
            {
                void* status;
                setMessage(&newMsg, DEMUX_COMMAND_QUIT, (uintptr_t)&demux->semCache);
                AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                SemTimedWait(&demux->semCache, -1);
                pthread_join(demux->cacheThreadId, &status);
                demux->cacheThreadId = 0;
            }

            if(demux->pParser != NULL)
            {
                CdxParserClose(demux->pParser);
                demux->pParser = NULL;
				demux->pStream = NULL;
            }else if(demux->pStream != NULL){
				CdxStreamClose(demux->pStream);
				demux->pStream = NULL;
			}

            clearDataSourceFields(&demux->source);
            demux->eStatus = DEMUX_STATUS_IDLE;
            demux->bStopping = 0;

            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            if(msg.messageId == DEMUX_COMMAND_QUIT)
                break;  //* quit the thread.

            continue;
        } //* end DEMUX_COMMAND_QUIT or DEMUX_COMMAND_CLEAR.
        else if(msg.messageId == DEMUX_COMMAND_SEEK)
        {
            int mSeekTimeMs;
            int params[2];

            logv("process message DEMUX_COMMAND_SEEK.");

            mSeekTimeMs = msg.params[2];
            if(demux->pParser != NULL)
            {
                //* flush the cache.
                if(demux->cacheThreadId != 0)
                {
                    setMessage(&newMsg, DEMUX_COMMAND_PAUSE, (uintptr_t)&demux->semCache);
                    AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                    SemTimedWait(&demux->semCache, -1);
                    setMessage(&newMsg, DEMUX_COMMAND_SEEK, (uintptr_t)&demux->semCache);
                    AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                    SemTimedWait(&demux->semCache, -1);
                }

                if(demux->bCancelSeek == 0 && demux->bStopping == 0)
                {
                    ret = CdxParserClrForceStop(demux->pParser);
                    if(ret < 0)
                    {
                        logw("CdxParserClrForceStop fail, ret(%d)", ret);
                    }
                }
                ret = CdxParserSeekTo(demux->pParser, ((int64_t)mSeekTimeMs) * 1000);
                if(ret == 0)
                {
                    params[0] = 0;
                    params[1] = mSeekTimeMs;
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_SEEK_FINISH, (void*)params);

                    demux->bSeeking = 0;
                    demux->bEOS     = 0;
                    demux->bIOError = 0;

                    //* send a flush message to the cache thread.
                    if(demux->cacheThreadId != 0)
                    {
                        setMessage(&newMsg, DEMUX_COMMAND_START, (uintptr_t)&demux->semCache);
                        AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                        SemTimedWait(&demux->semCache, -1);
                    }
                    if(demux->eStatus == DEMUX_STATUS_COMPLETE)
                    {
                        demux->eStatus = DEMUX_STATUS_STARTED;
                    }

                    if(demux->eStatus == DEMUX_STATUS_STARTED)
                    {
                        setMessage(&newMsg, DEMUX_COMMAND_READ);
                        AwMessageQueuePostMessage(demux->mq, &newMsg);
                    }

                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;
                }
                else
                {
                    loge("CdxParserSeekTo() return fail");
                    demux->eStatus = DEMUX_STATUS_COMPLETE;
                    demux->bSeeking = 0;
                    if(demux->bCancelSeek == 1 || demux->bStopping == 1)
                        params[0] = DEMUX_ERROR_USER_CANCEL;
                    else
                        params[0] = DEMUX_ERROR_IO;
                    params[1] = mSeekTimeMs;
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_SEEK_FINISH, (void*)params);

                    if(pReplyValue != NULL)
                        *pReplyValue = -1;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;
                }
            }
            else
            {
                params[0] = DEMUX_ERROR_UNKNOWN;
                params[1] = mSeekTimeMs;
                demux->bSeeking = 0;
                demux->callback(demux->pUserData, DEMUX_NOTIFY_SEEK_FINISH, (void*)params);

                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }
        }
        else if(msg.messageId == DEMUX_COMMAND_CANCEL_PREPARE)
        {
            logv("process message DEMUX_COMMAND_CANCEL_PREPARE.");

            demux->bCancelPrepare = 0;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        }
        else if(msg.messageId == DEMUX_COMMAND_CANCEL_SEEK)
        {
            logv("process message DEMUX_COMMAND_CANCEL_SEEK.");
            demux->bCancelSeek = 0;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        }
        else if(msg.messageId == DEMUX_COMMAND_READ)
        {
            logv("process message DEMUX_COMMAND_READ.");

            if(demux->eStatus != DEMUX_STATUS_STARTED)
            {
                logw("demux component not in started status, ignore read message.");
                continue;
            }

            if(demux->cacheThreadId != 0)
            {
                //**************************************************************
                //* read data from cache.
                //**************************************************************
				logv("demux->bBufferring = %d",demux->bBufferring);
                if(demux->bBufferring)
                {
                    //* the player is paused and caching stream data.

                    logv("buffering, wait...");
                    //* wait some time for caching.
                    ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 100);
                    if(ret == 0)    //* new message come, quit loop to process.
                        goto process_message;

                    //* check whether data in cache is enough for play.
                    if(StreamCacheDataEnough(demux->pCache) || demux->bEOS || demux->bIOError)
                    {
                        logd("detect data enough, notify BUFFER_END.");
                        demux->bBufferring = 0;
						//data enouth,continue play
						demux->callback(demux->pUserData, DEMUX_NOTIFY_RESUME_PLAYER, NULL);
                        demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_END, NULL);
                    }

                    //* post a read message to continue the reading job.
                    setMessage(&newMsg, DEMUX_COMMAND_READ);
                    AwMessageQueuePostMessage(demux->mq, &newMsg);
                    continue;
                }
                else
                {
                    //* check whether cache underflow.
                    if(StreamCacheUnderflow(demux->pCache))
                    {
                        logv("detect cache data underflow.");
                        //* cache underflow, if not eos, we need to notify pausing,
                        //* otherwise we need to notify complete.
                        if(demux->bEOS)
                        {
                            //* end of stream, notify complete.
                            logv("detect eos, notify EOS.");
                            demux->callback(demux->pUserData, DEMUX_NOTIFY_EOS, 0);
                            demux->eStatus = DEMUX_STATUS_COMPLETE;
                            continue;
                        }
                        else if(demux->bIOError)
                        {
                            logv("detect io error, notify IOERROR.");
                            //* end of stream, notify complete.
                            demux->callback(demux->pUserData, DEMUX_NOTIFY_IOERROR, 0);
                            continue;
                        }
                        else
                        {
                            //* no data in cache, check whether player hold enough data,
                            //* if not, we need to notify pausing to wait for caching
                            //* more data for player.
                            if(PlayerBufferUnderflow(demux->pPlayer))
                            {
                                logd("detect player data underflow, notify BUFFER_START.");
                                demux->bBufferring = 1;
								//first pause the player to stop the avtimer
								demux->callback(demux->pUserData, DEMUX_NOTIFY_PAUSE_PLAYER, NULL);
                                demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_START, NULL);
                            }
                            else
                            {
                                //* wait some time for caching.
                                ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 50);
                                if(ret == 0)    //* new message come, quit loop to process.
                                    goto process_message;
                            }

                            //* post a read message to continue the reading job.
                            setMessage(&newMsg, DEMUX_COMMAND_READ);
                            AwMessageQueuePostMessage(demux->mq, &newMsg);
                            continue;
                        }
                    }
                    else
                    {
                        //* there is some data in cache for player.
                        //* if data in player is not too much, send it to player,
                        //* otherwise, just keep it in the cache.
                        if(PlayerBufferOverflow(demux->pPlayer))
                        {
                            logv("detect player data overflow.");
                            //* too much data in player, wait some time.
                            ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 200);
                            if(ret == 0)    //* new message come, quit loop to process.
                                goto process_message;

                            //* post a read message to continue the reading job.
                            setMessage(&newMsg, DEMUX_COMMAND_READ);
                            AwMessageQueuePostMessage(demux->mq, &newMsg);
                            continue;
                        }
                        else
                        {
                            //*************************************
                            //* send data from cache to player.
                            //*************************************
                            CacheNode*          node;
                            enum EMEDIATYPE     ePlayerMediaType;
                            MediaStreamDataInfo streamDataInfo;
                            int                 nStreamIndex;
                            void*               pBuf0;
                            void*               pBuf1;
                            int                 nBufSize0;
                            int                 nBufSize1;

                            //********************************
                            //* 1. get one frame from cache.
                            //********************************
                            node = StreamCacheNextFrame(demux->pCache);
                            if(node == NULL)
                            {
                                loge("Cache not underflow but cannot get stream frame,.");
                                abort();
                            }

                            //********************************
                            //* 2. request buffer from player.
                            //********************************
                            if(node->eMediaType == CDX_MEDIA_VIDEO)
                            {
                                ePlayerMediaType = MEDIA_TYPE_VIDEO;
                                nStreamIndex     = (node->nFlags&MINOR_STREAM)==0 ? 0 : 1;
                            }
                            else if(node->eMediaType == CDX_MEDIA_AUDIO)
                            {
                                ePlayerMediaType = MEDIA_TYPE_AUDIO;
                                nStreamIndex     = node->nStreamIndex;
                            }
                            else if(node->eMediaType == CDX_MEDIA_SUBTITLE)
                            {
                                ePlayerMediaType = MEDIA_TYPE_SUBTITLE;
                                nStreamIndex     = node->nStreamIndex;
                            }
                            else
                            {
                                loge("media type from parser not valid, abort().");
                                abort();
                            }

                            if(ePlayerMediaType == MEDIA_TYPE_VIDEO ||
                                ePlayerMediaType == MEDIA_TYPE_AUDIO)
                            {
                                while(1)
                                {
                                     ret = PlayerRequestStreamBuffer(demux->pPlayer,
                                                                     node->nLength,
                                                                     &pBuf0,
                                                                     &nBufSize0,
                                                                     &pBuf1,
                                                                     &nBufSize1,
                                                                     ePlayerMediaType,
                                                                     nStreamIndex);
                                    if(ret<0 || (nBufSize0+nBufSize1)<node->nLength)
                                    {
                                        logi("waiting for stream buffer.");
                                        //* no buffer, try to wait sometime.
                                        ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 200);
                                        if(ret == 0)    //* new message come, quit loop to process.
                                            goto process_message;
                                    }
                                    else
                                        break;  //* get buffer ok.
                                }
                            }
                            else
                            {
                                //* request buffer from text decoder.
                                //* TODO.
                                loge("do not support subtitle yet, abort().");
                                abort();
                            }

                            //**********************************************
                            //* 3. copy data to player's buffer and submit.
                            //**********************************************
                            if(node->nLength > nBufSize0)
                            {
                                memcpy(pBuf0, node->pData, nBufSize0);
                                memcpy(pBuf1, node->pData + nBufSize0, node->nLength-nBufSize0);
                            }
                            else
                                memcpy(pBuf0, node->pData, node->nLength);

                            streamDataInfo.pData        = (char*)pBuf0;
                            streamDataInfo.nLength      = node->nLength;
                            streamDataInfo.nPts         = node->nPts;
                            streamDataInfo.nPcr         = -1;
                            streamDataInfo.bIsFirstPart = 1;
                            streamDataInfo.bIsLastPart  = 1;

                            PlayerSubmitStreamData(demux->pPlayer, &streamDataInfo,
                                            ePlayerMediaType, nStreamIndex);

                            StreamCacheFlushOneFrame(demux->pCache);

                            //* post a read message to continue the reading job.
                            setMessage(&newMsg, DEMUX_COMMAND_READ);
                            AwMessageQueuePostMessage(demux->mq, &newMsg);
                            continue;
                        }   //* end if(PlayerBufferOverflow(...)){}else {}
                    }   //* end if(StreamCacheUnderflow(...)){}else {}
                }   //* end if(demux->bBufferring){}else {}
            }
            else
            {
                //**************************************************************
                //* read data directly from parser.
                //**************************************************************

                CdxPacketT          packet;
                memset(&packet, 0x00, sizeof(CdxPacketT));
                enum EMEDIATYPE     ePlayerMediaType;
                MediaStreamDataInfo streamDataInfo;
                int                 nStreamIndex;

                //* if data in player is not too much, send it to player,
                //* otherwise don't read.
                if(PlayerBufferOverflow(demux->pPlayer))
                {
                    //* too much data in player, wait some time.
                    ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 200);
                    if(ret == 0)    //* new message come, quit loop to process.
                        goto process_message;

                    //* post a read message to continue the reading job.
                    setMessage(&newMsg, DEMUX_COMMAND_READ);
                    AwMessageQueuePostMessage(demux->mq, &newMsg);
                    continue;
                }

                //* 1. get data type.
                if(CdxParserPrefetch(demux->pParser, &packet) != 0)
                {
                    if(demux->bStopping == 0 && demux->bSeeking == 0)
                    {
                        int err = CdxParserGetStatus(demux->pParser);

                        if(err == PSR_IO_ERR)
                        {
                            demux->bIOError = 1;
                            demux->callback(demux->pUserData, DEMUX_NOTIFY_IOERROR, 0);
                        }
                        else
                        {
                            demux->bEOS = 1;
                            demux->callback(demux->pUserData, DEMUX_NOTIFY_EOS, 0);
                            demux->eStatus = DEMUX_STATUS_COMPLETE;
                        }
                    }

                    continue;
                }

                //* 2. request buffer from player.
                if(packet.type == CDX_MEDIA_VIDEO)
                {
                    ePlayerMediaType = MEDIA_TYPE_VIDEO;
                    nStreamIndex     = (packet.flags&MINOR_STREAM)==0 ? 0 : 1;
                }
                else if(packet.type == CDX_MEDIA_AUDIO)
                {
                    ePlayerMediaType = MEDIA_TYPE_AUDIO;
                    nStreamIndex     = packet.streamIndex;
                }
                else if(packet.type == CDX_MEDIA_SUBTITLE)
                {
                    ePlayerMediaType = MEDIA_TYPE_SUBTITLE;
                    nStreamIndex     = packet.streamIndex;
                }
                else
                {
                    loge("media type from parser not valid, should not run here, abort().");
                    abort();
                }

                if(ePlayerMediaType == MEDIA_TYPE_VIDEO ||
                    ePlayerMediaType == MEDIA_TYPE_AUDIO ||
                    ePlayerMediaType == MEDIA_TYPE_SUBTITLE)
                {
                    while(1)
                    {
                        if((!CONFIG_DISABLE_VIDEO && ePlayerMediaType == MEDIA_TYPE_VIDEO) ||
                           (!CONFIG_DISABLE_AUDIO && ePlayerMediaType == MEDIA_TYPE_AUDIO) ||
                           (!CONFIG_DISABLE_SUBTITLE && ePlayerMediaType == MEDIA_TYPE_SUBTITLE))
                        {
                            ret = PlayerRequestStreamBuffer(demux->pPlayer,
                                                            packet.length,
                                                            &packet.buf,
                                                            &packet.buflen,
                                                            &packet.ringBuf,
                                                            &packet.ringBufLen,
                                                            ePlayerMediaType,
                                                            nStreamIndex);
                        }
                        else
                        {
                            //* allocate a buffer to read uncare media data and skip it.
                            packet.buf = malloc(packet.length);
                            if(packet.buf != NULL)
                            {
                                packet.buflen     = packet.length;
                                packet.ringBuf    = NULL;
                                packet.ringBufLen = 0;
                                ret = 0;
                            }
                            else
                            {
                                packet.buflen     = 0;
                                packet.ringBuf    = NULL;
                                packet.ringBufLen = 0;
                                ret = -1;
                            }
                        }
                        if(ret<0 || (packet.buflen+packet.ringBufLen)<packet.length)
                        {
                            logi("waiting for stream buffer.");
                            //* no buffer, try to wait sometime.
                            ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 200);
                            if(ret == 0)    //* new message come, quit loop to process.
                                goto process_message;
                        }
                        else
                            break;  //* get buffer ok.
                    }
                }
                else
                {
                    //* request buffer from text decoder.
                    //* TODO.
                    loge("do not support subtitle yet, abort().");
                    abort();
                }

                //* 3. read data to buffer and submit.
                ret = CdxParserRead(demux->pParser, &packet);
                if(ret == 0)
                {
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_DATA_PACKET, &packet);

                    if((!CONFIG_DISABLE_VIDEO && ePlayerMediaType == MEDIA_TYPE_VIDEO) ||
                       (!CONFIG_DISABLE_AUDIO && ePlayerMediaType == MEDIA_TYPE_AUDIO) ||
                       (!CONFIG_DISABLE_SUBTITLE && ePlayerMediaType == MEDIA_TYPE_SUBTITLE))
                    {
                        streamDataInfo.pData        = (char*)packet.buf;
                        streamDataInfo.nLength      = packet.length;
                        streamDataInfo.nPts         = packet.pts;
                        streamDataInfo.nPcr         = -1;
                        streamDataInfo.bIsFirstPart = 1;
                        streamDataInfo.bIsLastPart  = 1;
                        PlayerSubmitStreamData(demux->pPlayer, &streamDataInfo,
                                                ePlayerMediaType, nStreamIndex);
                    }
                    else
                    {
                        //* skip the media data.
                        free(packet.buf);
                    }

                    //* post a read message to continue the reading job after message processed.
                    setMessage(&newMsg, DEMUX_COMMAND_READ);
                    AwMessageQueuePostMessage(demux->mq, &newMsg);
                }
                else
                {
                    logw("read data from parser return fail.");
                    if(demux->bStopping == 0 && demux->bSeeking == 0)
                    {
                        int err = CdxParserGetStatus(demux->pParser);

                        if(err == PSR_IO_ERR)
                        {
                            demux->bIOError = 1;
                            demux->callback(demux->pUserData, DEMUX_NOTIFY_IOERROR, 0);
                        }
                        else
                        {
                            demux->bEOS = 1;
                            demux->callback(demux->pUserData, DEMUX_NOTIFY_EOS, 0);
                            demux->eStatus = DEMUX_STATUS_COMPLETE;
                        }
                    }
                }

                continue;
            }
        }
        else
        {
            logw("unknow message with id %d, ignore.", msg.messageId);
        }
    }

    return NULL;
}


static void* CacheThread(void* arg)
{
    AwMessage         msg;
    int               ret;
    sem_t*            pReplySem;
    int*              pReplyValue;
    DemuxCompContext* demux;
    int               eCacheStatus;

    demux = (DemuxCompContext*)arg;
    eCacheStatus = DEMUX_STATUS_STOPPED;

    while(1)
    {
        if(AwMessageQueueGetMessage(demux->mqCache, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }

cache_process_message:
        pReplySem   = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == DEMUX_COMMAND_START)
        {
            logv("cache thread process message DEMUX_COMMAND_START.");

            eCacheStatus = DEMUX_STATUS_STARTED;
            setMessage(&msg, DEMUX_COMMAND_READ);
            AwMessageQueuePostMessage(demux->mqCache, &msg);
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end DEMUX_COMMAND_START
        else if(msg.messageId == DEMUX_COMMAND_PAUSE || msg.messageId == DEMUX_COMMAND_STOP)
        {
            logv("cache thread process message DEMUX_COMMAND_PAUSE or DEMUX_COMMAND_STOP.");

            eCacheStatus = DEMUX_STATUS_STOPPED;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end DEMUX_COMMAND_STOP.
        else if(msg.messageId == DEMUX_COMMAND_QUIT)
        {
            logv("cache thread process message DEMUX_COMMAND_QUIT.");
            StreamCacheFlushAll(demux->pCache);
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);

            break;  //* quit the thread.

        } //* end DEMUX_COMMAND_QUIT.
        else if(msg.messageId == DEMUX_COMMAND_SEEK)
        {
            logv("cache thread process message DEMUX_COMMAND_SEEK.");

            StreamCacheFlushAll(demux->pCache);

            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        }
        else if(msg.messageId == DEMUX_COMMAND_READ)
        {
            if(eCacheStatus != DEMUX_STATUS_STARTED)
                continue;

            logi("cache thread process message DEMUX_COMMAND_READ.");

            if(StreamCacheOverflow(demux->pCache))
            {
                //* wait some time for cache buffer.
                ret = AwMessageQueueTryGetMessage(demux->mqCache, &msg, 200);
                if(ret == 0)    //* new message come, quit loop to process.
                    goto cache_process_message;

                //* post a read message to continue the reading job after message processed.
                setMessage(&msg, DEMUX_COMMAND_READ);
                AwMessageQueuePostMessage(demux->mqCache, &msg);
            }
            else
            {
                //**************************************************************
                //* read data directly from parser.
                //**************************************************************

                CdxPacketT packet;
                CacheNode  node;

                memset(&packet, 0, sizeof(CdxPacketT));
                memset(&node, 0, sizeof(CacheNode));

                //* 1. get data type.
                if(CdxParserPrefetch(demux->pParser, &packet) != 0)
                {
                    logw("prefetch fail.");
                    if(demux->bStopping == 0 && demux->bSeeking == 0)
                    {
                        int err = CdxParserGetStatus(demux->pParser);

                        if(err == PSR_IO_ERR)
                            demux->bIOError = 1;
                        else
                            demux->bEOS = 1;
                    }

                    continue;
                }

                //* 2. request cache buffer.
                while(1)
                {
                    node.pData = (unsigned char*)malloc(packet.length);
                    if(node.pData == NULL)
                    {
                        logw("allocate memory for cache node fail, waiting for memory.");
                        //* no free memory, try to wait sometime.
                        ret = AwMessageQueueTryGetMessage(demux->mqCache, &msg, 200);
                        if(ret == 0)    //* new message come, quit loop to process.
                            goto cache_process_message;
                    }
                    else
                    {
                        packet.buf        = node.pData;
                        packet.buflen     = packet.length;
                        packet.ringBuf    = NULL;
                        packet.ringBufLen = 0;
                        break;
                    }
                }

                //* 3. read data to buffer and submit.
                ret = CdxParserRead(demux->pParser, &packet);
                if(ret == 0)
                {
                    node.pNext        = NULL;
                    node.nLength      = packet.length;
                    node.eMediaType   = packet.type;
                    node.nStreamIndex = packet.streamIndex;
                    node.nFlags       = packet.flags;
                    node.nPts         = packet.pts;
                    node.nPcr         = -1;
                    node.bIsFirstPart = 1;
                    node.bIsLastPart  = 1;

                    StreamCacheAddOneFrame(demux->pCache, &node);

                    //* post a read message to continue the reading job after message processed.
                    setMessage(&msg, DEMUX_COMMAND_READ);
                    AwMessageQueuePostMessage(demux->mqCache, &msg);
                }
                else
                {
                    logw("read data from parser return fail.");
                    if(node.pData != NULL)
                        free(node.pData);

                    if(demux->bStopping == 0 && demux->bSeeking == 0)
                    {
                        int err = CdxParserGetStatus(demux->pParser);

                        if(err == PSR_IO_ERR)
                            demux->bIOError = 1;
                        else
                            demux->bEOS = 1;
                    }
                }
                continue;
            }   //* end if(StreamCacheOverflow(demux->pCache)) {} else {}
        }   //* end DEMUX_COMMAND_READ.
    }   //* end while(1).

    return NULL;
}


static void clearDataSourceFields(CdxDataSourceT* source)
{
    CdxHttpHeaderFieldsT* pHttpHeaders;
    int                   i;
    int                   nHeaderSize;

    if(source->uri != NULL)
    {
        free(source->uri);
        source->uri = NULL;
    }

    if(source->extraDataType == EXTRA_DATA_HTTP_HEADER &&
       source->extraData != NULL)
    {
        pHttpHeaders = (CdxHttpHeaderFieldsT*)source->extraData;
        nHeaderSize  = pHttpHeaders->num;

        for(i=0; i<nHeaderSize; i++)
        {
            if(pHttpHeaders->pHttpHeader[i].key != NULL)
                free((void*)pHttpHeaders->pHttpHeader[i].key);
            if(pHttpHeaders->pHttpHeader[i].val != NULL)
                free((void*)pHttpHeaders->pHttpHeader[i].val);
        }

        free(pHttpHeaders->pHttpHeader);
        free(pHttpHeaders);
        source->extraData = NULL;
        source->extraDataType = EXTRA_DATA_UNKNOWN;
    }

    return;
}


static int setDataSourceFields(CdxDataSourceT* source, char* uri, map<string,string>* pHeaders)
{
    CdxHttpHeaderFieldsT* pHttpHeaders;
    int                   i;
    int                   nHeaderSize;

    clearDataSourceFields(source);

    if(uri != NULL)
    {
        //* check whether ths uri has a scheme.
        if(strstr(uri, "://") != NULL)
        {
            source->uri = strdup(uri);
            if(source->uri == NULL)
            {
                loge("can not dump string of uri.");
                return -1;
            }
        }
        else
        {
            source->uri  = (char*)malloc(strlen(uri)+8);
            if(source->uri == NULL)
            {
                loge("can not dump string of uri.");
                return -1;
            }
            sprintf(source->uri, "file://%s", uri);
        }

        if(pHeaders != NULL && (!strncasecmp("http://", uri, 7) ||
            !strncasecmp("https://", uri, 8)))
        {
            string key;
            string value;
            char*  str;
            map<string, string>::iterator it;

            it = pHeaders->find(string("x-hide-urls-from-log"));
            if(it != pHeaders->end())
                pHeaders->erase(it);

            nHeaderSize = pHeaders->size();
            if(nHeaderSize > 0)
            {
                pHttpHeaders = (CdxHttpHeaderFieldsT*)malloc(sizeof(CdxHttpHeaderFieldsT));
                if(pHttpHeaders == NULL)
                {
                    loge("can not malloc memory for http header.");
                    clearDataSourceFields(source);
                    return -1;
                }
                memset(pHttpHeaders, 0, sizeof(CdxHttpHeaderFieldsT));
                pHttpHeaders->num = nHeaderSize;

                pHttpHeaders->pHttpHeader = (CdxHttpHeaderFieldT*)malloc(
                            sizeof(CdxHttpHeaderFieldT)*nHeaderSize);
                if(pHttpHeaders->pHttpHeader == NULL)
                {
                    loge("can not malloc memory for http header.");
                    free(pHttpHeaders);
                    clearDataSourceFields(source);
                    return -1;
                }

                source->extraData = (void*)pHttpHeaders;
                source->extraDataType = EXTRA_DATA_HTTP_HEADER;

                i = 0;
                for(it=pHeaders->begin(); it!=pHeaders->end(); ++it)
                {
                    key   = it->first;
                    value = it->second;
                    str = (char*)key.c_str();

                    if(str != NULL)
                    {
                        pHttpHeaders->pHttpHeader[i].key = (const char*)strdup(str);
                        if(pHttpHeaders->pHttpHeader[i].key == NULL)
                        {
                            loge("can not dump string of http header.");
                            clearDataSourceFields(source);
                            return -1;
                        }
                    }
                    else
                        pHttpHeaders->pHttpHeader[i].key = NULL;

                    str = (char*)value.c_str();
                    if(str != NULL)
                    {
                        pHttpHeaders->pHttpHeader[i].val = (const char*)strdup(str);
                        if(pHttpHeaders->pHttpHeader[i].val == NULL)
                        {
                            loge("can not dump string of http header.");
                            clearDataSourceFields(source);
                            return -1;
                        }
                    }
                    else
                        pHttpHeaders->pHttpHeader[i].val = NULL;

                    i++;
                }
            }
        }
    }

    return 0;
}


static int setMediaInfo(MediaInfo* pMediaInfo, CdxMediaInfoT* pInfoFromParser)
{
    int                 i;
    int                 nStreamCount;
    VideoStreamInfo*    pVideoStreamInfo;
    AudioStreamInfo*    pAudioStreamInfo;
    SubtitleStreamInfo* pSubtitleStreamInfo;
    int                 nCodecSpecificDataLen;
    char*               pCodecSpecificData;

    clearMediaInfo(pMediaInfo);

    pMediaInfo->nDurationMs = pInfoFromParser->program[0].duration;
    pMediaInfo->nFileSize   = pInfoFromParser->fileSize;
    pMediaInfo->bSeekable   = pInfoFromParser->bSeekable;    //* TODO, parser should give this flag.

    pMediaInfo->albumsz = pInfoFromParser->albumsz;
    pMediaInfo->albumCharEncode = pInfoFromParser->albumCharEncode;
    memcpy(pMediaInfo->album, pInfoFromParser->album, pMediaInfo->albumsz);

    pMediaInfo->authorsz = pInfoFromParser->authorsz;
    pMediaInfo->authorCharEncode = pInfoFromParser->authorCharEncode;
    memcpy(pMediaInfo->author, pInfoFromParser->author, pMediaInfo->authorsz);

    pMediaInfo->genresz = pInfoFromParser->genresz;
    pMediaInfo->genreCharEncode = pInfoFromParser->genreCharEncode;
    memcpy(pMediaInfo->genre, pInfoFromParser->genre, pMediaInfo->genresz);

    pMediaInfo->titlesz = pInfoFromParser->titlesz;
    pMediaInfo->titleCharEncode = pInfoFromParser->titleCharEncode;
    memcpy(pMediaInfo->title, pInfoFromParser->title, pMediaInfo->titlesz);

    nStreamCount = pInfoFromParser->program[0].videoNum;
    logv("video stream count = %d", nStreamCount);
    if(nStreamCount > 0)
    {
        pVideoStreamInfo = (VideoStreamInfo*)malloc(sizeof(VideoStreamInfo)*nStreamCount);
        if(pVideoStreamInfo == NULL)
        {
            loge("can not alloc memory for media info.");
            return -1;
        }
        memset(pVideoStreamInfo, 0, sizeof(VideoStreamInfo)*nStreamCount);

        pMediaInfo->pVideoStreamInfo = pVideoStreamInfo;

        for(i=0; i<nStreamCount; i++)
        {
            pVideoStreamInfo = &pMediaInfo->pVideoStreamInfo[i];
            memcpy(pVideoStreamInfo, &pInfoFromParser->program[0].video[i],
                        sizeof(VideoStreamInfo));

            pCodecSpecificData    = pVideoStreamInfo->pCodecSpecificData;
            nCodecSpecificDataLen = pVideoStreamInfo->nCodecSpecificDataLen;
            pVideoStreamInfo->pCodecSpecificData = NULL;
            pVideoStreamInfo->nCodecSpecificDataLen = 0;

            if(pCodecSpecificData != NULL && nCodecSpecificDataLen > 0)
            {
                pVideoStreamInfo->pCodecSpecificData = (char*)malloc(nCodecSpecificDataLen);
                if(pVideoStreamInfo->pCodecSpecificData == NULL)
                {
                    loge("can not alloc memory for media info.");
                    clearMediaInfo(pMediaInfo);
                    return -1;
                }

                memcpy(pVideoStreamInfo->pCodecSpecificData, pCodecSpecificData,
                            nCodecSpecificDataLen);
                pVideoStreamInfo->nCodecSpecificDataLen = nCodecSpecificDataLen;
            }

            logv("the %dth video stream info.", i);
            logv("    codec: %d.", pVideoStreamInfo->eCodecFormat);
            logv("    width: %d.", pVideoStreamInfo->nWidth);
            logv("    height: %d.", pVideoStreamInfo->nHeight);
            logv("    frame rate: %d.", pVideoStreamInfo->nFrameRate);
            logv("    aspect ratio: %d.", pVideoStreamInfo->nAspectRatio);
            logv("    is 3D: %s.", pVideoStreamInfo->bIs3DStream ? "true" : "false");
            logv("    codec specific data size: %d.", pVideoStreamInfo->nCodecSpecificDataLen);
        }

        pMediaInfo->nVideoStreamNum = nStreamCount;
    }

    //* copy audio stream info.
    nStreamCount = pInfoFromParser->program[0].audioNum;
    if(nStreamCount > 0)
    {
        pAudioStreamInfo = (AudioStreamInfo*)malloc(sizeof(AudioStreamInfo)*nStreamCount);
        if(pAudioStreamInfo == NULL)
        {
            clearMediaInfo(pMediaInfo);
            loge("can not alloc memory for media info.");
            return -1;
        }
        memset(pAudioStreamInfo, 0, sizeof(AudioStreamInfo)*nStreamCount);
        pMediaInfo->pAudioStreamInfo = pAudioStreamInfo;

        for(i=0; i<nStreamCount; i++)
        {
            pAudioStreamInfo = &pMediaInfo->pAudioStreamInfo[i];
            memcpy(pAudioStreamInfo, &pInfoFromParser->program[0].audio[i],
                        sizeof(AudioStreamInfo));

            pCodecSpecificData    = pAudioStreamInfo->pCodecSpecificData;
            nCodecSpecificDataLen = pAudioStreamInfo->nCodecSpecificDataLen;
            pAudioStreamInfo->pCodecSpecificData = NULL;
            pAudioStreamInfo->nCodecSpecificDataLen = 0;

            if(pCodecSpecificData != NULL && nCodecSpecificDataLen > 0)
            {
                pAudioStreamInfo->pCodecSpecificData = (char*)malloc(nCodecSpecificDataLen);
                if(pAudioStreamInfo->pCodecSpecificData == NULL)
                {
                    loge("can not alloc memory for media info.");
                    clearMediaInfo(pMediaInfo);
                    return -1;
                }

                memcpy(pAudioStreamInfo->pCodecSpecificData, pCodecSpecificData,
                                nCodecSpecificDataLen);
                pAudioStreamInfo->nCodecSpecificDataLen = nCodecSpecificDataLen;
            }
        }

        pMediaInfo->nAudioStreamNum = nStreamCount;
    }

    //* copy subtitle stream info.
    nStreamCount = pInfoFromParser->program[0].subtitleNum;
    if(nStreamCount > 0)
    {
        pSubtitleStreamInfo = (SubtitleStreamInfo*)malloc(sizeof(SubtitleStreamInfo)*nStreamCount);
        if(pSubtitleStreamInfo == NULL)
        {
            clearMediaInfo(pMediaInfo);
            loge("can not alloc memory for media info.");
            return -1;
        }
        memset(pSubtitleStreamInfo, 0, sizeof(SubtitleStreamInfo)*nStreamCount);
        pMediaInfo->pSubtitleStreamInfo = pSubtitleStreamInfo;

        for(i=0; i<nStreamCount; i++)
        {
            pSubtitleStreamInfo = &pMediaInfo->pSubtitleStreamInfo[i];
            memcpy(pSubtitleStreamInfo, &pInfoFromParser->program[0].subtitle[i],
                        sizeof(SubtitleStreamInfo));
            pSubtitleStreamInfo->bExternal = 0;
            pSubtitleStreamInfo->pUrl      = NULL;
            pSubtitleStreamInfo->fd        = -1;
            pSubtitleStreamInfo->fdSub     = -1;
        }

        pMediaInfo->nSubtitleStreamNum = nStreamCount;
    }

    return 0;
}


static void clearMediaInfo(MediaInfo* pMediaInfo)
{
    int                 i;
    VideoStreamInfo*    pVideoStreamInfo;
    AudioStreamInfo*    pAudioStreamInfo;

    if(pMediaInfo->nVideoStreamNum > 0)
    {
        for(i=0; i<pMediaInfo->nVideoStreamNum; i++)
        {
            pVideoStreamInfo = &pMediaInfo->pVideoStreamInfo[i];
            if(pVideoStreamInfo->pCodecSpecificData != NULL &&
               pVideoStreamInfo->nCodecSpecificDataLen > 0)
            {
                free(pVideoStreamInfo->pCodecSpecificData);
                pVideoStreamInfo->pCodecSpecificData = NULL;
                pVideoStreamInfo->nCodecSpecificDataLen = 0;
            }
        }

        free(pMediaInfo->pVideoStreamInfo);
        pMediaInfo->pVideoStreamInfo = NULL;
        pMediaInfo->nVideoStreamNum = 0;
    }


    if(pMediaInfo->nAudioStreamNum > 0)
    {
        for(i=0; i<pMediaInfo->nAudioStreamNum; i++)
        {
            pAudioStreamInfo = &pMediaInfo->pAudioStreamInfo[i];
            if(pAudioStreamInfo->pCodecSpecificData != NULL &&
               pAudioStreamInfo->nCodecSpecificDataLen > 0)
            {
                free(pAudioStreamInfo->pCodecSpecificData);
                pAudioStreamInfo->pCodecSpecificData = NULL;
                pAudioStreamInfo->nCodecSpecificDataLen = 0;
            }
        }

        free(pMediaInfo->pAudioStreamInfo);
        pMediaInfo->pAudioStreamInfo = NULL;
        pMediaInfo->nAudioStreamNum = 0;
    }


    if(pMediaInfo->nSubtitleStreamNum > 0)
    {
        free(pMediaInfo->pSubtitleStreamInfo);
        pMediaInfo->pSubtitleStreamInfo = NULL;
        pMediaInfo->nSubtitleStreamNum = 0;
    }

    pMediaInfo->nFileSize      = 0;
    pMediaInfo->nDurationMs    = 0;
    pMediaInfo->eContainerType = CONTAINER_TYPE_UNKNOWN;
    pMediaInfo->bSeekable      = 0;

    return;
}


static int PlayerBufferOverflow(Player* p)
{
    int bVideoOverflow;
    int bAudioOverflow;

    int     nPictureNum;
    int     nFrameDuration;
    int     nPcmDataSize;
    int     nSampleRate;
    int     nChannelCount;
    int     nBitsPerSample;
    int     nStreamDataSize;
    int     nBitrate;
    int64_t nVideoCacheTime;
    int64_t nAudioCacheTime;

    bVideoOverflow = 1;
    bAudioOverflow = 1;

    if(PlayerHasVideo(p))
    {
        nPictureNum     = PlayerGetValidPictureNum(p);
        nFrameDuration  = PlayerGetVideoFrameDuration(p);
        nStreamDataSize = PlayerGetVideoStreamDataSize(p);
        nBitrate        = PlayerGetVideoBitrate(p);

        nVideoCacheTime = nPictureNum*nFrameDuration;

        if(nBitrate > 0)
            nVideoCacheTime += ((int64_t)nStreamDataSize)*8*1000*1000/nBitrate;

        if(nVideoCacheTime <= 2000000)   //* cache more than 2 seconds of data.
            bVideoOverflow = 0;

        logi("picNum = %d, frameDuration = %d, dataSize = %d, bitrate = %d, bVideoOverflow = %d",
            nPictureNum, nFrameDuration, nStreamDataSize, nBitrate, bVideoOverflow);
    }

    if(PlayerHasAudio(p))
    {
        nPcmDataSize    = PlayerGetAudioPcmDataSize(p);
        nStreamDataSize = PlayerGetAudioStreamDataSize(p);
        nBitrate        = PlayerGetAudioBitrate(p);
        PlayerGetAudioParam(p, &nSampleRate, &nChannelCount, &nBitsPerSample);

        nAudioCacheTime = 0;

        if(nSampleRate != 0 && nChannelCount != 0 && nBitsPerSample != 0)
        {
            nAudioCacheTime +=
                ((int64_t)nPcmDataSize)*8*1000*1000/(nSampleRate*nChannelCount*nBitsPerSample);
        }

        if(nBitrate > 0)
            nAudioCacheTime += ((int64_t)nStreamDataSize)*8*1000*1000/nBitrate;

        if(nAudioCacheTime <= 2000000)   //* cache more than 2 seconds of data.
            bAudioOverflow = 0;

        logi("nPcmDataSize = %d, nStreamDataSize = %d, nBitrate = %d, \
                nAudioCacheTime = %lld, bAudioOverflow = %d",
            nPcmDataSize, nStreamDataSize, nBitrate, nAudioCacheTime, bAudioOverflow);
    }

    return bVideoOverflow && bAudioOverflow;
}


static int PlayerBufferUnderflow(Player* p)
{
    int bVideoUnderflow;
    int bAudioUnderFlow;

    bVideoUnderflow = 0;
    bAudioUnderFlow = 0;

    if(PlayerHasVideo(p))
    {
        int nPictureNum;
        int nStreamFrameNum;

        nPictureNum = PlayerGetValidPictureNum(p);
        nStreamFrameNum = PlayerGetVideoStreamFrameNum(p);
        if(nPictureNum == 0 && nStreamFrameNum == 0)
            bVideoUnderflow = 1;

        logi("nPictureNum = %d, nStreamFrameNum = %d, bVideoUnderflow = %d",
            nPictureNum, nStreamFrameNum, bVideoUnderflow);
    }

    if(PlayerHasAudio(p))
    {
        int nStreamDataSize;
        int nPcmDataSize;
        int nCacheTime;

        nStreamDataSize = PlayerGetAudioStreamDataSize(p);
        nPcmDataSize    = PlayerGetAudioPcmDataSize(p);
        nCacheTime      = 0;
        //if(nCacheTime == 0 && nPcmDataSize == 0 && nStreamDataSize == 0){
        if(nCacheTime == 0 && (nPcmDataSize + nStreamDataSize < 8000)){
			logd("nPcmDataSize = %d,nStreamDataSize = %d",nPcmDataSize,nStreamDataSize);
            bAudioUnderFlow = 1;
		}

        logv("nStreamDataSize = %d, nPcmDataSize = %d, nCacheTime = %d, bAudioUnderFlow = %d",
            nStreamDataSize, nPcmDataSize, nCacheTime, bAudioUnderFlow);
    }

    return bVideoUnderflow | bAudioUnderFlow;
}
