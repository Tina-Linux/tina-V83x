/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : demuxComponent.cpp
* Description : stream control and video stream demux
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : first version
*
*/

//#define CONFIG_LOG_LEVEL 4
#define  LOG_TAG "demuxComponent"
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>
#include <strings.h>

#include "demuxComponent.h"
#include "AwMessageQueue.h"
#include "CdxParser.h"          //* parser library in "LIBRARY/DEMUX/PARSER/include/"
#include "CdxStream.h"          //* parser library in "LIBRARY/DEMUX/STREAM/include/"
#include "player.h"             //* player library in "LIBRARY/PLAYER/"
#include "cache.h"
#include "cdx_log.h"
#include "xplayer.h"
#include "iniparserapi.h"

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
static const int DEMUX_COMMAND_SET_SOURCE                = 0x101;
static const int DEMUX_COMMAND_PREPARE                   = 0x104;
static const int DEMUX_COMMAND_START                     = 0x105;
static const int DEMUX_COMMAND_PAUSE                     = 0x106;
static const int DEMUX_COMMAND_STOP                      = 0x107;
static const int DEMUX_COMMAND_QUIT                      = 0x109;
static const int DEMUX_COMMAND_SEEK                      = 0x10a;
static const int DEMUX_COMMAND_CLEAR                     = 0x10b;
static const int DEMUX_COMMAND_CANCEL_PREPARE            = 0x10c;
static const int DEMUX_COMMAND_CANCEL_SEEK               = 0x10d;
static const int DEMUX_COMMAND_READ                      = 0x10e;
static const int DEMUX_COMMAND_NOTIFY_FIRST_FRAME_SHOWED = 0x10f;
static const int DEMUX_COMMAND_VIDEO_STREAM_CHANGE       = 0x110;
static const int DEMUX_COMMAND_AUDIO_STREAM_CHANGE       = 0x111;
static const int DEMUX_COMMAND_RESET_URL                 = 0x112;

#define kUseSecureInputBuffers 256 //copy from wvm/ExtractorWrapper.cpp
#define PREPARE_TRY_TIMES 1

struct AwMessage {
    AWMESSAGE_COMMON_MEMBERS
    uintptr_t params[8];
};

typedef struct DemuxCompContext_t
{
    int                         eStatus;
    int                         bLiveStream;        //* live streaming from network.
    int                         bFileStream;        //* local media file.
    int                         bVodStream;         //* vod stream from network.

    //* data source.
    int                         nSourceType;        //* url or fd or IStreamSource.
    CdxDataSourceT              source;
    MediaInfo                   mediaInfo;

    pthread_t                   threadId;
    AwMessageQueue*             mq;

    CdxParserT*                 pParser;
    CdxStreamT*                 pStream;
    Player*                     pPlayer;
    DemuxCallback               callback;
    void*                       pUserData;

    pthread_mutex_t             mutex;
    sem_t                       semSetDataSource;
    sem_t                       semStart;
    sem_t                       semStop;
    sem_t                       semQuit;
    sem_t                       semClear;
    sem_t                       semCancelPrepare;
    sem_t                       semCancelSeek;
    sem_t                       semStreamChange;

    int                         nSetDataSourceReply;
    int                         nStartReply;
    int                         nStopReply;

    pthread_t                   cacheThreadId;
    AwMessageQueue*             mqCache;
    sem_t                       semCache;
    int                         nCacheReply;
    StreamCache*                pCache;
    int                         bBufferring;
    int                         bEOS;
    int                         bIOError;

    struct ParserCacheStateS    sCacheState;        //* for buffering status update notify.
    int64_t                     nCacheStatReportIntervalUs;
    enum ECACHEPOLICY           eCachePolicy;

    SeekModeType                nSeekModeType;

    int                         bCancelPrepare;
    int                         bCancelSeek;
    int                         bSeeking;
    int                         bStopping;

    int                         bBufferStartNotified;
    int                         bNeedPausePlayerNotified;

    char                        shiftedTimeUrl[4096];
    int                         mLivemode;

    int                         bNetDisConnectNotified;
    int                         bDisconnet;

    pthread_mutex_t             lock;                      //for break blocking operation
    pthread_cond_t              cond;

    int                         statusCode;

    // Todo: using atomic if necessary
    int nStartPlayCacheSize;        //default 512*1024;
    int nCacheBufferSize;           //default 20*1024*1024;
    int nCacheBufferSizeLive;       //default 20*1024*1024;
    int nStartPlayCacheTime;        //default 6000; // 6 seconds.
    int nMaxStartPlayChacheSize;    //default 150*1024*1024;
    int nMaxCacheBufferSize;        //default 200*1024*1024;

    //* for timeshift to keep the last seqNum
    int timeShiftLastSeqNum;
    //* for hls discontinuity tag
    int bDiscontinue;

    int mStreamPrepared; //* default false, true if once prepared and playbacked;
    struct HdcpOpsS*             pHDCPOps;
    AwBufferingSettings*         pBuffering;
}DemuxCompContext;

struct BufferInfo {
    uint32_t size;
    void * buffer;
};

static void* DemuxThread(void* arg);
static void* CacheThread(void* arg);

static int setDataSourceFields(DemuxCompContext *demuxHdr,
                CdxDataSourceT* source, void* pHTTPServer, char* uri,
                CdxKeyedVectorT* pHeader);

static void clearDataSourceFields(CdxDataSourceT* source);

static int setMediaInfo(MediaInfo* pMediaInfo, CdxMediaInfoT* pInfoFromParser);
static void clearMediaInfo(MediaInfo* pMediaInfo);

static int PlayerBufferOverflow(Player* p);
static int PlayerBufferUnderflow(Player* p);

static int GetCacheState(DemuxCompContext* demux);
static int DemuxCompAdjustCacheParamsWithBitrate(DemuxCompContext* demux, int nBitrate);
static int DemuxCompUseDefaultCacheParams(DemuxCompContext* demux);
static void AdjustCacheParams(DemuxCompContext* demux);

static void NotifyCacheState(DemuxCompContext* demux);
static void PostReadMessage(AwMessageQueue* mq);
static int64_t GetSysTime();
static int ShiftTimeMode(int Shiftedms, char *buf);

/*
        char *dirPath = (char *)((int *)param)[0];
        int *pDirId = (int *)((int *)param)[1];
*/
static int DemuxCBOpenDir(void *cbhdr, const char *name, void **pDirHdr)
{
    DemuxCompContext *d = (DemuxCompContext *)cbhdr;
    uintptr_t msgParam[4];
    int dirId = -1;

    msgParam[0] = (uintptr_t)name;
    msgParam[1] = (uintptr_t)&dirId;

    d->callback(d->pUserData, DEMUX_IOREQ_OPENDIR, msgParam);

    uintptr_t nTmpDirId = dirId;
    *pDirHdr = (void *)nTmpDirId;

    return (dirId == -1) ? -1 : 0;
}

/*
        int dirId = ((int *)param)[0];
        int *pRet = (int *)((int *)param)[1];
        char *buf = (char *)((int *)param)[2];
        int bufLen = ((int *)param)[3];
*/
static int DemuxCBReadDir(void *cbhdr, void *dirHdr, char *dname, int dnameLen)
{
    DemuxCompContext *d = (DemuxCompContext *)cbhdr;
    uintptr_t msgParam[4];
    int ret = -1;
    //int dirId = (int)dirHdr;

    msgParam[0] = (uintptr_t)dirHdr;
    msgParam[1] = (uintptr_t)&ret;
    msgParam[2] = (uintptr_t)dname;
    msgParam[3] = dnameLen;

    d->callback(d->pUserData, DEMUX_IOREQ_READDIR, msgParam);
    return ret;
}

/*
        int dirId = ((int *)param)[0];
        int *pRet = (int *)((int *)param)[1];
*/
static int DemuxCBCloseDir(void *cbhdr, void *dirHdr)
{
    DemuxCompContext *d = (DemuxCompContext *)cbhdr;
    uintptr_t msgParam[4];
    int ret;
    //int dirId = (int)dirHdr;

    msgParam[0] = (uintptr_t)dirHdr;
    msgParam[1] = (uintptr_t)&ret;

    d->callback(d->pUserData, DEMUX_IOREQ_CLOSEDIR, msgParam);
    return ret;
}

/*
        char *filePath = (char *)((int *)param)[0];
        int *pFd = (int *)((int *)param)[1];
*/
static int DemuxCBOpenFile(void *cbhdr, const char *pathname, int flags)
{
    DemuxCompContext *d = (DemuxCompContext *)cbhdr;
    uintptr_t msgParam[4];
    int fd = -1;

    CEDARX_UNUSE(flags);

    msgParam[0] = (uintptr_t)pathname;
    msgParam[1] = (uintptr_t)&fd;

    d->callback(d->pUserData, DEMUX_IOREQ_OPEN, msgParam);
    return fd;
}

/*
        char *filePath = (char *)((int *)param)[0];
        int mode = ((int *)param)[1];
        int *pRet = (int *)((int *)param)[2];
*/
static int DemuxCBAccessFile(void *cbhdr, const char *pathname, int mode)
{
    DemuxCompContext *d = (DemuxCompContext *)cbhdr;
    uintptr_t msgParam[4];
    int ret = -1;

    msgParam[0] = (uintptr_t)pathname;
    msgParam[1] = mode;
    msgParam[2] = (uintptr_t)&ret;

    d->callback(d->pUserData, DEMUX_IOREQ_ACCESS, msgParam);

    return ret;
}

static struct IoOperationS demuxIoOps =
{
    .openDir = DemuxCBOpenDir,
    .readDir = DemuxCBReadDir,
    .closeDir = DemuxCBCloseDir,
    .openFile = DemuxCBOpenFile,
    .accessFile = DemuxCBAccessFile
};
static int ParserCallbackProcess(void* pUserData, int eMessageId, void* param)
{
    DemuxCompContext *demux = (DemuxCompContext *)pUserData;

    switch(eMessageId)
    {
#ifndef ONLY_ENABLE_AUDIO
        case PARSER_NOTIFY_VIDEO_STREAM_CHANGE:
        {
            CdxMediaInfoT parserMediaInfo;
            memset(&parserMediaInfo,0,sizeof(CdxMediaInfoT));
            CdxParserGetMediaInfo(demux->pParser, &parserMediaInfo);
            setMediaInfo(&demux->mediaInfo, &parserMediaInfo);
            demux->mediaInfo.eContainerType = (enum ECONTAINER)demux->pParser->type;

            //* if have cacheThread, we should make the demuxThread do callback()
            //* or will cause some async bug
            if(demux->cacheThreadId != 0)
            {
                AwMessage msg;

                msg.messageId = DEMUX_COMMAND_VIDEO_STREAM_CHANGE;
                msg.params[0] = (uintptr_t)&demux->semStreamChange;
                msg.params[1] = 0;
                msg.params[2] = ((int*)param)[0];
                AwMessageQueuePostMessage(demux->mq, &msg);

                SemTimedWait(&demux->semStreamChange, -1);
            }
            else
            {
                int nMsg = DEMUX_VIDEO_STREAM_CHANGE;
                demux->callback(demux->pUserData, nMsg, param);
            }
            break;
        }
#endif
        case PARSER_NOTIFY_AUDIO_STREAM_CHANGE:
        {
            CdxMediaInfoT parserMediaInfo;
            memset(&parserMediaInfo,0,sizeof(CdxMediaInfoT));
            CdxParserGetMediaInfo(demux->pParser, &parserMediaInfo);
            setMediaInfo(&demux->mediaInfo, &parserMediaInfo);
            demux->mediaInfo.eContainerType = (enum ECONTAINER)demux->pParser->type;

            //* if have cacheThread, we should make the demuxThread do callback()
            //* or will cause some async bug
            if(demux->cacheThreadId != 0)
            {
                AwMessage msg1;
                msg1.messageId = DEMUX_COMMAND_AUDIO_STREAM_CHANGE;
                msg1.params[0] = (uintptr_t)&demux->semStreamChange;
		msg1.params[1] = 0;
                AwMessageQueuePostMessage(demux->mq, &msg1);

                SemTimedWait(&demux->semStreamChange, -1);
            }
            else
            {
                int nMsg = DEMUX_AUDIO_STREAM_CHANGE;
                demux->callback(demux->pUserData, nMsg, param);
            }
            break;
        }
        case STREAM_EVT_DOWNLOAD_START:
        {
            DownloadObject cur;
            if(!param)
            {
                //this message is sent by http stream, it is the first m3u8 file; or m3u8 file.
                //cur.uri = demux->source.uri;
                //cur.seqNum = -1;
                //cur.statusCode = 0;

                //int nMsg = DEMUX_NOTIFY_HLS_DOWNLOAD_START;
                //demux->callback(demux->pUserData, nMsg, (void*)&cur);
                logd("sent by http stream");
            }
            else
            {
                // this message is sent by hls parser
                int nMsg = DEMUX_NOTIFY_HLS_DOWNLOAD_START;
                demux->callback(demux->pUserData, nMsg, param);
            }
            break;
        }
        case STREAM_EVT_DOWNLOAD_ERROR:
        {
            DownloadObject cur;
            char url[1024];

            if(demux->pParser)
            {
                if(CdxParserControl(demux->pParser, CDX_PSR_CMD_GET_URL, (void*)url))
                {
                    loge("get  url failed");
                }
            }
            cur.uri = url;
            cur.statusCode = *((int*)param);

            demux->statusCode = cur.statusCode;
            int nMsg = DEMUX_NOTIFY_HLS_DOWNLOAD_ERROR;
            demux->callback(demux->pUserData, nMsg, (void*)&cur);
            logd("demux: STREAM_EVT_DOWNLOAD_ERROR, statusCode(%d)", cur.statusCode);
            break;
        }
        case STREAM_EVT_DOWNLOAD_END:
        {
            ExtraDataContainerT *extradata = (ExtraDataContainerT *)param;
            if(EXTRA_DATA_HTTP == extradata->extraDataType)
            {
                //* this message is sent by http stream, it is the first m3u8 file, not callback.
            }
            else if(EXTRA_DATA_HLS == extradata->extraDataType)
            {
                int nMsg = DEMUX_NOTIFY_HLS_DOWNLOAD_END;
                param = (DownloadObject *)extradata->extraData;
                demux->callback(demux->pUserData, nMsg, param);
            }

            break;
        }
        case STREAM_EVT_NET_DISCONNECT:
        {
            int flag = *(int*)param;
            if(demux->bDisconnet == 0 && flag == 1)
            {
                CDX_LOGD("STREAM_EVT_NET_DISCONNECT, param = %d", flag);
                demux->bDisconnet = 1;
            }
            if(demux->bDisconnet == 1 && flag == 0)
            {
                CDX_LOGD("reconnect success");
                demux->bDisconnet = 0;
                if(demux->mLivemode == 1)
                {
                    StreamCacheFlushAll(demux->pCache);
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_RESET_PLAYER, NULL);
                }
            }
            break;
        }
        case STREAM_EVT_CMCC_LOG_RECORD:
        {
            demux->callback(demux->pUserData, DEMUX_NOTIFY_LOG_RECORDER, param);
            break;
        }
        case PARSER_NOTIFY_TIMESHIFT_END_INFO:
        {
            TimeShiftEndInfoT *info = (TimeShiftEndInfoT *)param;
            logd("PARSER_NOTIFY_TIMESHIFT_END_INFO seqnum:%d, duration:%lld ms",
                        info->timeShiftLastSeqNum, info->timeShiftDuration);
            demux->timeShiftLastSeqNum = info->timeShiftLastSeqNum;
            cdx_int64 dur = info->timeShiftDuration;
            demux->callback(demux->pUserData, DEMUX_NOTIFY_TIMESHIFT_DURATION, (void*)&dur);
            break;
        }
        case PARSER_NOTIFY_HLS_DISCONTINUITY:
        {
            demux->bDiscontinue = 1;
            break;
        }
	case PARSER_NOTIFY_HLS_GET_STREAM_CACHE_UNDERFLOW_FLAG:
	{
	    int *dataUnderflowFlag = (int *)param;
	    *dataUnderflowFlag = StreamCacheUnderflow(demux->pCache);

	    if(demux->bNeedPausePlayerNotified == 1 && *dataUnderflowFlag == 0)
	    {
		logd("when the audio stream changes, detect data remains, notify BUFFER_END.");
		demux->bBufferring = 0;
		if(demux->bBufferStartNotified == 1)
		{
		    unsigned char url[1024];
		    if(demux->pParser)
		    {
			if(CdxParserControl(demux->pParser,
						CDX_PSR_CMD_GET_URL, (void*)url))
			{
				loge("get  url failed");
			}
		    }
		    demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_END, (void*)url);
		    demux->bBufferStartNotified = 0;
		}
		demux->callback(demux->pUserData, DEMUX_NOTIFY_RESUME_PLAYER, NULL);
		demux->bNeedPausePlayerNotified = 0;
	    }
	    break;
	}
        default:
        {
            logw("ignore demux callback message, eMessageId = 0x%x.", eMessageId);
            return -1;
        }
    }

    return 0;
}

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

    d->nStartPlayCacheSize     = GetConfigParamterInt("start_play_cache_size",512)*1024;
    d->nCacheBufferSize        = GetConfigParamterInt("cache_buffer_size",20*1024)*1024;
    d->nCacheBufferSizeLive    = GetConfigParamterInt("cache_buffer_size_live",20*1024)*1024;
    d->nStartPlayCacheTime     = 0; // we will set it later
    d->nMaxStartPlayChacheSize = GetConfigParamterInt("max_start_play_chache_size",150*1024)*1024;
    d->nMaxCacheBufferSize     = GetConfigParamterInt("max_cache_buffer_size",200*1024)*1024;

    logv("StartPlayCacheSize(%d)",d->nStartPlayCacheSize);
    logv("CacheBufferSize(%d)",d->nCacheBufferSize);
    logv("StartPlayCacheTime(%d)",d->nStartPlayCacheTime);
    logv("MaxStartPlayChacheSize(%d)",d->nMaxStartPlayChacheSize);
    logv("MaxCacheBufferSize(%d)",d->nMaxCacheBufferSize);

    d->nCacheStatReportIntervalUs = 1000000;
    d->mStreamPrepared = 0;

    pthread_mutex_init(&d->mutex, NULL);
    sem_init(&d->semSetDataSource, 0, 0);
    sem_init(&d->semStart, 0, 0);
    sem_init(&d->semStop, 0, 0);
    sem_init(&d->semQuit, 0, 0);
    sem_init(&d->semClear, 0, 0);
    sem_init(&d->semCancelPrepare, 0, 0);
    sem_init(&d->semCancelSeek, 0, 0);
    sem_init(&d->semCache, 0, 0);
    sem_init(&d->semStreamChange, 0, 0);
    pthread_mutex_init(&d->lock, NULL);
    pthread_cond_init(&d->cond, NULL);

    d->mq = AwMessageQueueCreate(64, "demuxComp");
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
        sem_destroy(&d->semStreamChange);
        free(d);
        return NULL;
    }

    d->mqCache = AwMessageQueueCreate(64, "DemuxComp");
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
        sem_destroy(&d->semStreamChange);
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
        sem_destroy(&d->semCache);
        sem_destroy(&d->semStreamChange);
        free(d);
        return NULL;
    }

    d->pCache = StreamCacheCreate();
    d->pBuffering = NULL;

    return (DemuxComp*)d;
}


void DemuxCompDestroy(DemuxComp* d)
{
    void* status;

    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    //* lock mutex to sync with the message process thread when processing SEEK message.
    //* when processing the SEEK message, we clear the Parser's force stop flag by calling
    //* CdxParserClrForceStop(). if we're playing a network stream and processing
    //* seek message, when user cancel the seek message, we should set the parser's
    //* force stop flag by calling CdxParserForceStop() to
    //* prevend from blocking at the network io.
    pthread_mutex_lock(&demux->mutex);
    demux->bStopping = 1;
    pthread_mutex_unlock(&demux->mutex);
    //* send a quit message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_QUIT;
    msg.params[0] = (uintptr_t)&demux->semQuit;
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semQuit, -1);

    pthread_join(demux->threadId, &status);

    StreamCacheDestroy(demux->pCache);

    if(demux->mq != NULL)
        AwMessageQueueDestroy(demux->mq);

    if(demux->mqCache != NULL)
        AwMessageQueueDestroy(demux->mqCache);

    pthread_mutex_destroy(&demux->mutex);
    pthread_mutex_destroy(&demux->lock);
    pthread_cond_destroy(&demux->cond);
    sem_destroy(&demux->semSetDataSource);
    sem_destroy(&demux->semStart);
    sem_destroy(&demux->semStop);
    sem_destroy(&demux->semQuit);
    sem_destroy(&demux->semClear);
    sem_destroy(&demux->semCancelPrepare);
    sem_destroy(&demux->semCancelSeek);
    sem_destroy(&demux->semCache);
    sem_destroy(&demux->semStreamChange);
    free(demux);

    return;
}

void DemuxCompClear(DemuxComp* d)  //* clear the data source, like just created.
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    //* lock mutex to sync with the message process thread when processing SEEK message.
    //* when processing the SEEK message, we clear the Parser's force stop flag by calling
    //* CdxParserClrForceStop(). if we're playing a network stream and processing
    //* seek message, when user cancel the seek message, we should set the parser's
    //* force stop flag by calling CdxParserForceStop() to
    //* prevend from blocking at the network io.
    pthread_mutex_lock(&demux->mutex);
    demux->bStopping = 1;
    pthread_mutex_unlock(&demux->mutex);
    //* send clear message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_CLEAR;
    msg.params[0] = (uintptr_t)&demux->semClear;
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semClear, -1);

    return;
}

int DemuxCompSetUrlSource(DemuxComp* d, void* pHTTPServer,
            const char* pUrl, const CdxKeyedVectorT* pHeaders)
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    msg.messageId = DEMUX_COMMAND_SET_SOURCE;
    msg.params[0] = (uintptr_t)&demux->semSetDataSource;
    msg.params[1] = (uintptr_t)&demux->nSetDataSourceReply;
    msg.params[2] = SOURCE_TYPE_URL;
    msg.params[3] = (uintptr_t)pUrl;
    msg.params[4] = (uintptr_t)pHeaders;
    msg.params[5] = (uintptr_t)pHTTPServer;

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
    msg.messageId = DEMUX_COMMAND_SET_SOURCE;
    msg.params[0] = (uintptr_t)&demux->semSetDataSource;
    msg.params[1] = (uintptr_t)&demux->nSetDataSourceReply;
    msg.params[2] = SOURCE_TYPE_FD;
    msg.params[3] = fd;                              //* params[3] = fd.
    msg.params[4] = (uintptr_t)(nOffset>>32);        //* params[4] = high 32 bits of offset.
    msg.params[5] = (uintptr_t)(nOffset & 0xffffffff); //* params[5] = low 32 bits of offset.
    msg.params[6] = (uintptr_t)(nLength>>32);        //* params[6] = high 32 bits of length.
    msg.params[7] = (uintptr_t)(nLength & 0xffffffff); //* params[7] = low 32 bits of length.
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semSetDataSource, -1);

    return demux->nSetDataSourceReply;
}


int DemuxCompSetStreamSource(DemuxComp* d, const char* pUri)
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    //* send a set data source message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_SET_SOURCE;
    msg.params[0] = (uintptr_t)&demux->semSetDataSource;
    msg.params[1] = (uintptr_t)&demux->nSetDataSourceReply;
    msg.params[2] = SOURCE_TYPE_ISTREAMSOURCE;
    msg.params[3] = (uintptr_t)pUri;           //* params[3] = uri of IStreamSource.

    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semSetDataSource, -1);

    return demux->nSetDataSourceReply;
}

int DemuxCompSetMediaDataSource(DemuxComp* d, const char* pUri)
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    //* send a set data source message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_SET_SOURCE;
    msg.params[0] = (uintptr_t)&demux->semSetDataSource;
    msg.params[1] = (uintptr_t)&demux->nSetDataSourceReply;
    msg.params[2] = SOURCE_TYPE_DATASOURCE;
    msg.params[3] = (uintptr_t)pUri;           //* params[3] = uri of MediaDataSource.

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


int DemuxCompSetHdcpOps(DemuxComp* d, struct HdcpOpsS* pHdcp)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    demux->pHDCPOps= pHdcp;
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

    //* clear this flag, you can set this flag to make the parser quit preparing.
    demux->bCancelPrepare = 0;
    demux->eStatus = DEMUX_STATUS_PREPARING;

    //* send a prepare message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_PREPARE;
    AwMessageQueuePostMessage(demux->mq, &msg);
    return 0;
}

//* should call back DEMUX_PREPARE_FINISH message.
int DemuxCompCancelPrepare(DemuxComp* d)
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
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_CANCEL_PREPARE;
    msg.params[0] = (uintptr_t)&demux->semCancelPrepare;
    msg.params[1] = 0;      //* no reply.

    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semCancelPrepare, -1);
    return 0;
}
#ifndef ONLY_ENABLE_AUDIO
int DemuxProbeH265RefPictureNumber(char* pDataBuf, int nDataLen)
{
    return probeH265RefPictureNumber((cdx_uint8 *)pDataBuf, (cdx_uint32)nDataLen);
}
#endif
MediaInfo* DemuxCompGetMediaInfo(DemuxComp* d)
{
    DemuxCompContext*   demux;
    MediaInfo*          mi;
    MediaInfo*          myMediaInfo;
    int                 i;
#ifndef ONLY_ENABLE_AUDIO
    VideoStreamInfo*    pVideoStreamInfo;
#endif
    AudioStreamInfo*    pAudioStreamInfo;
#ifndef ONLY_ENABLE_AUDIO
    SubtitleStreamInfo* pSubtitleStreamInfo;
#endif
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
    mi->nBitrate       = myMediaInfo->nBitrate;
    mi->eContainerType = myMediaInfo->eContainerType;
    mi->bSeekable      = myMediaInfo->bSeekable;
    mi->nFirstPts      = myMediaInfo->nFirstPts;
    memcpy(mi->cRotation,myMediaInfo->cRotation,4*sizeof(unsigned char));
#ifndef ONLY_ENABLE_AUDIO
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
            pVideoStreamInfo->bSecureStreamFlag =
                        myMediaInfo->pVideoStreamInfo[i].bSecureStreamFlag;

            if(pCodecSpecificData != NULL && nCodecSpecificDataLen > 0)
            {
                pVideoStreamInfo->pCodecSpecificData = (char*)malloc(nCodecSpecificDataLen);
                if(pVideoStreamInfo->pCodecSpecificData == NULL)
                {
                    loge("can not alloc memory for media info.");
                    clearMediaInfo(mi);
                    return NULL;
                }

                memcpy(pVideoStreamInfo->pCodecSpecificData, pCodecSpecificData,
                                nCodecSpecificDataLen);
                pVideoStreamInfo->nCodecSpecificDataLen = nCodecSpecificDataLen;
            }
            if (demux->pParser->type == CDX_PARSER_TS ||
                demux->pParser->type == CDX_PARSER_BD ||
                demux->pParser->type == CDX_PARSER_HLS)
            {
                pVideoStreamInfo->bIsFramePackage = 0; /* stream package */
            }
            else
            {
                pVideoStreamInfo->bIsFramePackage = 1; /* frame package */
            }
        }

        mi->nVideoStreamNum = myMediaInfo->nVideoStreamNum;
    }

    logv("video stream num = %d, video stream info = %p",
                mi->nVideoStreamNum, mi->pVideoStreamInfo);
#endif
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
#ifndef ONLY_ENABLE_AUDIO
    if(myMediaInfo->nSubtitleStreamNum > 0)
    {
        pSubtitleStreamInfo = (SubtitleStreamInfo*)malloc
                (sizeof(SubtitleStreamInfo)*myMediaInfo->nSubtitleStreamNum);
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
#endif
    return mi;
}


int DemuxCompStart(DemuxComp* d)
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    if(demux->eStatus == DEMUX_STATUS_STARTED && demux->bBufferring != 1)
    {
        logi("demux component already in started status.");
        return 0;
    }

    if(demux->eStatus == DEMUX_STATUS_COMPLETE)
    {
        logv("demux component is in complete status.");
        return 0;
    }

    if(pthread_equal(pthread_self(), demux->threadId)
        || pthread_equal(pthread_self(), demux->cacheThreadId))
    {
        //* called from demux callback to awplayer.
        if(demux->bSeeking)
        {
            demux->eStatus = DEMUX_STATUS_STARTED;
        }
        return 0;
    }

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_START;
    msg.params[0] = (uintptr_t)&demux->semStart;
    msg.params[1] = (uintptr_t)&demux->nStartReply;

    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semStart, -1);
    return demux->nStartReply;
}


int DemuxCompStop(DemuxComp* d)    //* close the data source, must call prepare again to restart.
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    //* lock mutex to sync with the message process thread when processing SEEK message.
    //* when processing the SEEK message, we clear the Parser's force stop flag by calling
    //* CdxParserClrForceStop(). if we're playing a network stream and processing
    //* seek message, when user cancel the seek message, we should set the parser's
    //* force stop flag by calling CdxParserForceStop() to
    //* prevend from blocking at the network io.
    pthread_mutex_lock(&demux->mutex);
    demux->bStopping = 1;
    pthread_mutex_unlock(&demux->mutex);
    if(demux->pParser != NULL)
        CdxParserForceStop(demux->pParser); //* quit from reading or seeking.
    else if(demux->pStream != NULL)
        CdxStreamForceStop(demux->pStream);

    //* send a stop message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_STOP;
    msg.params[0] = (uintptr_t)&demux->semStop;
    msg.params[1] = (uintptr_t)&demux->nStopReply;

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


int DemuxCompSeekTo(DemuxComp* d, int nSeekTimeMs, SeekModeType nSeekModeType)
{
    AwMessage msg;
    DemuxCompContext* demux;
    logd("+++++ DemuxCompSeekTo");

    demux = (DemuxCompContext*)d;

    demux->bCancelSeek = 0;
    demux->bSeeking = 1;
    if(demux->pParser != NULL && demux->eStatus == DEMUX_STATUS_STARTED)
        CdxParserForceStop(demux->pParser); //* quit from reading.

    demux->nSeekModeType = nSeekModeType;
    //* send a seek message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_SEEK;
    msg.params[0] = 0;
    msg.params[1] = 0;
    msg.params[2] = nSeekTimeMs;
    AwMessageQueuePostMessage(demux->mq, &msg);
    return 0;
}

int DemuxCompSeekToResetUrl(DemuxComp* d, int nSeekTimeMs)
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;
    logd("+++++ DemuxCompSeekToResetUrl");

    demux->bCancelSeek = 0;
    demux->bSeeking = 1;
    if(demux->pParser != NULL && demux->eStatus == DEMUX_STATUS_STARTED)
        CdxParserForceStop(demux->pParser); //* quit from reading.

    //* send a resetUrl message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_RESET_URL;
    msg.params[0] = 0;
    msg.params[1] = 0;
    msg.params[2] = nSeekTimeMs;
    AwMessageQueuePostMessage(demux->mq, &msg);
    return 0;
}


int DemuxCompCancelSeek(DemuxComp* d)  //* should not call back DEMUX_SEEK_FINISH message.
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    //* lock mutex to sync with the message process thread when processing SEEK message.
    //* when processing the SEEK message, we clear the Parser's force stop flag by calling
    //* CdxParserClrForceStop(). if we're playing a network stream and processing
    //* seek message, when user cancel the seek message, we should set the parser's
    //* force stop flag by calling CdxParserForceStop() to
    //* prevend from blocking at the network io.
    pthread_mutex_lock(&demux->mutex);
    demux->bCancelSeek = 1;
    pthread_mutex_unlock(&demux->mutex);

    if(demux->pParser != NULL)
        CdxParserForceStop(demux->pParser);

    //* send a prepare.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_CANCEL_SEEK;
    msg.params[0] = (uintptr_t)&demux->semCancelSeek;
    msg.params[1] = 0; //* no reply.
    AwMessageQueuePostMessage(demux->mq, &msg);
    SemTimedWait(&demux->semCancelSeek, -1);
    return 0;
}


int DemuxCompNotifyFirstFrameShowed(DemuxComp* d)   //* notify video first frame showed.
{
    AwMessage msg;
    DemuxCompContext* demux;

    demux = (DemuxCompContext*)d;

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = DEMUX_COMMAND_NOTIFY_FIRST_FRAME_SHOWED;
    AwMessageQueuePostMessage(demux->mq, &msg);
    return 0;
}

int DemuxCompSetCacheStatReportInterval(DemuxComp* d, int ms)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    demux->nCacheStatReportIntervalUs = ms*1000;
    return 0;
}


int DemuxCompSetCachePolicy(DemuxComp*          d,
                            enum ECACHEPOLICY   eCachePolicy,
                            int                 nStartPlaySize,
                            int                 nStartPlayTimeMs,
                            int                 nMaxBufferSize)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;

    if(demux->eStatus == DEMUX_STATUS_IDLE ||
       demux->eStatus == DEMUX_STATUS_INITIALIZED ||
       demux->eStatus == DEMUX_STATUS_PREPARING)
    {
        loge("not prepared yet, can not set cache parameter.");
        return -1;
    }

    if(demux->bFileStream)
    {
        //* no need to set cache parameter for file stream.
        return -1;
    }

    if (nStartPlaySize > 0)
        demux->nStartPlayCacheSize = nStartPlaySize;
    if (nStartPlayTimeMs > 0)
        demux->nStartPlayCacheTime = nStartPlayTimeMs;
    if (nMaxBufferSize > 0 && nMaxBufferSize <= demux->nMaxCacheBufferSize)
    {
        demux->nCacheBufferSize = nMaxBufferSize;
        demux->nCacheBufferSizeLive = nMaxBufferSize;
    }

    switch(eCachePolicy)
    {
        case CACHE_POLICY_SMOOTH:
        {
            demux->eCachePolicy = CACHE_POLICY_SMOOTH;
            StreamCacheSetSize(demux->pCache, demux->nCacheBufferSize,
                    demux->nCacheBufferSize);
            break;
        }
        case CACHE_POLICY_ADAPTIVE:
        {
            DemuxCompUseDefaultCacheParams(demux);
            demux->eCachePolicy = CACHE_POLICY_ADAPTIVE;
            break;
        }
        default:
        {
            loge("no such policy");
            abort();
        }
    }

    return 0;
}

int DemuxCompGetLiveMode(DemuxComp* d)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    return demux->mLivemode;
}

int DemuxCompSetLiveMode(DemuxComp* d, int liveMode)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    demux->mLivemode = liveMode;
    return 0;
}

int DemuxCompGetCacheSize(DemuxComp* d)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    return StreamCacheGetSize(demux->pCache);
}

static int DemuxCompUseDefaultCacheParams(DemuxCompContext* demux)
{
    int               nStartPlaySize;
    int               nMaxBufferSize;
    int               nCacheTimeMs;
    int               nBitrate;

    nCacheTimeMs = demux->nStartPlayCacheTime;
    if(nCacheTimeMs > demux->mediaInfo.nDurationMs)
        nCacheTimeMs = demux->mediaInfo.nDurationMs;

    if(demux->mediaInfo.nFileSize <= 0)
    {
        if(demux->mediaInfo.nBitrate > 0)
            nBitrate = demux->mediaInfo.nBitrate;
        else
            nBitrate = 200*1024;    //* default vod stream bitrate is 200 kbits.
    }
    else
    {
        if(demux->mediaInfo.nDurationMs != 0)
            nBitrate = (int)(demux->mediaInfo.nFileSize*8*1000/demux->mediaInfo.nDurationMs);
        else
            nBitrate = 200*1024;    //* default vod stream bitrate is 200 kbits.
    }

    if(nBitrate <= 64*1024)
        nBitrate = 64*1024;
    if(nBitrate > 20*1024*1024)
        nBitrate = 20*1024*1024;

    nStartPlaySize = (int)(nBitrate * (int64_t)nCacheTimeMs / (8*1000));

    if(nStartPlaySize < demux->nStartPlayCacheSize)
        nStartPlaySize = demux->nStartPlayCacheSize;
    if(nStartPlaySize > demux->nMaxStartPlayChacheSize)
        nStartPlaySize = demux->nMaxStartPlayChacheSize;

    nMaxBufferSize = nStartPlaySize*4/3;
    if (demux->bLiveStream)
    {
        if (nMaxBufferSize < demux->nCacheBufferSizeLive)
            nMaxBufferSize = demux->nCacheBufferSizeLive;
    }
    else
    {
        if (nMaxBufferSize < demux->nCacheBufferSize)
            nMaxBufferSize = demux->nCacheBufferSize;
    }
    if(nMaxBufferSize > demux->nMaxCacheBufferSize)
        nMaxBufferSize = demux->nMaxCacheBufferSize;

    StreamCacheSetSize(demux->pCache, nStartPlaySize, nMaxBufferSize);
    return 0;
}

int DemuxCompSetBufferingSettings(DemuxComp* d, AwBufferingSettings* pBuffering)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    if(pBuffering == NULL)
    {
        loge("Buffering is null. please check it.");
        return -1;
    }
    demux->pBuffering = pBuffering; //* Only BufferingSettings would assign a non-null pointer to it
    if (demux->bLiveStream)
    {
        if (demux->pBuffering->mRebufferingWatermarkHighKB < demux->nCacheBufferSizeLive / 1024)
            demux->pBuffering->mRebufferingWatermarkHighKB = demux->nCacheBufferSizeLive / 1024;
    }
    else
    {
        if (demux->pBuffering->mRebufferingWatermarkHighKB < demux->nCacheBufferSize / 1024)
            demux->pBuffering->mRebufferingWatermarkHighKB = demux->nCacheBufferSize / 1024;
    }
    if(demux->pBuffering->mRebufferingWatermarkHighKB > demux->nMaxCacheBufferSize / 1024)
        demux->pBuffering->mRebufferingWatermarkHighKB = demux->nMaxCacheBufferSize / 1024;
    StreamSetBufferingSettings(demux->pCache, demux->pBuffering);
    return 0;
}
int DemuxCompGetBufferingSettings(DemuxComp* d, AwBufferingSettings* pBuffering)
{
    DemuxCompContext* demux;
    demux = (DemuxCompContext*)d;
    if(demux == NULL || pBuffering == NULL)
    {
        loge("Buffering is null. please check it.");
        return -1;
    }
    if(demux->pBuffering == NULL)
    {
        pBuffering->mInitialBufferingMode = AW_BUFFERING_MODE_SIZE_ONLY;
        pBuffering->mRebufferingMode = AW_BUFFERING_MODE_SIZE_ONLY;
        pBuffering->mInitialWatermarkKB = 0;
        pBuffering->mInitialWatermarkMs = -1;
        pBuffering->mRebufferingWatermarkHighKB = GetConfigParamterInt("cache_buffer_size",20*1024);
        pBuffering->mRebufferingWatermarkLowKB = GetConfigParamterInt("start_play_cache_size",512);
        pBuffering->mRebufferingWatermarkHighMs = -1;
        pBuffering->mRebufferingWatermarkLowMs = -1;
    }
    else
    {
        memcpy(pBuffering, demux->pBuffering, sizeof(AwBufferingSettings));
    }
    return 0;
}

static void AdjustCacheParams(DemuxCompContext* demux)
{
    int nBitrate = 0;
    if(demux->eCachePolicy != CACHE_POLICY_ADAPTIVE)
        return;

    nBitrate = demux->mediaInfo.nBitrate;
    if (nBitrate <= 0)
#ifndef ONLY_ENABLE_AUDIO
        nBitrate = PlayerGetVideoBitrate(demux->pPlayer) +
#endif
            PlayerGetAudioBitrate(demux->pPlayer);

    logd("bitrate = %d", nBitrate);
    if(nBitrate > 0)
    {
        if(nBitrate <= 64*1024)
            nBitrate = 64*1024;
        else if(nBitrate > 20*1024*1024)
            nBitrate = 20*1024*1024;
        DemuxCompAdjustCacheParamsWithBitrate(demux, nBitrate);
    }
}


static int DemuxCompAdjustCacheParamsWithBitrate(DemuxCompContext* demux, int nBitrate)
{
    int nStartPlaySize;
    int nMaxBufferSize;
    int nCacheTimeMs = demux->nStartPlayCacheTime;

    if(demux->mediaInfo.nDurationMs > 0 &&
            nCacheTimeMs > demux->mediaInfo.nDurationMs)
        nCacheTimeMs = demux->mediaInfo.nDurationMs;

    nStartPlaySize = (int)(nBitrate * (int64_t)nCacheTimeMs / (8*1000));

    if(nStartPlaySize < demux->nStartPlayCacheSize)
        nStartPlaySize = demux->nStartPlayCacheSize;
    if(nStartPlaySize > demux->nMaxStartPlayChacheSize)
        nStartPlaySize = demux->nMaxStartPlayChacheSize;

    nMaxBufferSize = nStartPlaySize*4/3;
    if (demux->bLiveStream)
    {
        if (nMaxBufferSize < demux->nCacheBufferSizeLive)
            nMaxBufferSize = demux->nCacheBufferSizeLive;
    }
    else
    {
        if (nMaxBufferSize < demux->nCacheBufferSize)
            nMaxBufferSize = demux->nCacheBufferSize;
    }
    if(nMaxBufferSize > demux->nMaxCacheBufferSize)
        nMaxBufferSize = demux->nMaxCacheBufferSize;

    StreamCacheSetSize(demux->pCache, nStartPlaySize, nMaxBufferSize);
    return 0;
}
#ifndef ONLY_ENABLE_AUDIO
static int setVideoStreamDataInfo(MediaStreamDataInfo *streamDataInfo, struct VideoInfo* pVideoinfo)
{
    logv("****pVideoinfo = %p, pVideoinfo->videoNum = %d",pVideoinfo, pVideoinfo->videoNum);
    size_t size = sizeof(VideoStreamInfo)*pVideoinfo->videoNum;
    VideoStreamInfo *pVideoStreamInfo =
                                    (VideoStreamInfo*)calloc(1, size);
    if(pVideoStreamInfo == NULL)
    {
        loge("malloc video stream info fail, size=%d.", (int)size);
        return -1;
    }

    int i;
    for(i = 0; i < pVideoinfo->videoNum; i++)
    {
        memcpy(&pVideoStreamInfo[i], &pVideoinfo->video[i],
                sizeof(VideoStreamInfo));
        if (!pVideoinfo->video[i].pCodecSpecificData)
            continue;
        size = pVideoinfo->video[i].nCodecSpecificDataLen;
        pVideoStreamInfo[i].pCodecSpecificData = (char*)malloc(size);
        if(pVideoStreamInfo[i].pCodecSpecificData == NULL)
        {
            loge("malloc video specific data fail, size=%d", (int)size);
            int j;
            for(j = 0; j < i; j++)
            {
                if(pVideoStreamInfo[j].pCodecSpecificData)
                {
                    free(pVideoStreamInfo[j].pCodecSpecificData);
                    pVideoStreamInfo[j].pCodecSpecificData = NULL;
                }
            }
            free(pVideoStreamInfo);
            return -1;
        }
        memcpy(pVideoStreamInfo[i].pCodecSpecificData,
               pVideoinfo->video[i].pCodecSpecificData,
               size);
        pVideoStreamInfo[i].nCodecSpecificDataLen = size;
    }

    streamDataInfo->pStreamInfo = pVideoStreamInfo;
    streamDataInfo->nStreamChangeFlag = 1;
    streamDataInfo->nStreamChangeNum  = pVideoinfo->videoNum;

    return 0;
}
#endif
static int setAudioStreamDataInfo(MediaStreamDataInfo *streamDataInfo, struct AudioInfo* pAudioinfo)
{
    size_t size = sizeof(AudioStreamInfo)*pAudioinfo->audioNum;
    AudioStreamInfo *pAudioStreamInfo =
                                (AudioStreamInfo*)calloc(1, size);
    if(pAudioStreamInfo == NULL)
    {
        loge("malloc audio stream info fail, size=%d", (int)size);
        return -1;
    }

    int i;
    for(i = 0; i < pAudioinfo->audioNum; i++)
    {
        memcpy(&pAudioStreamInfo[i], &pAudioinfo->audio[i],
            sizeof(AudioStreamInfo));
        if(!pAudioinfo->audio[i].pCodecSpecificData)
            continue;
        size = pAudioinfo->audio[i].nCodecSpecificDataLen;
        pAudioStreamInfo[i].pCodecSpecificData = (char*)calloc(1, size);
        if(pAudioStreamInfo[i].pCodecSpecificData == NULL)
        {
            loge("malloc audio specific data fail, size=%d", (int)size);
            int j;
            for(j = 0; j < i; j++)
            {
                if(pAudioStreamInfo[j].pCodecSpecificData)
                {
                    free(pAudioStreamInfo[j].pCodecSpecificData);
                    pAudioStreamInfo[j].pCodecSpecificData = NULL;
                }
            }
            free(pAudioStreamInfo);
            return -1;
        }
        memcpy(pAudioStreamInfo[i].pCodecSpecificData,
               pAudioinfo->audio[i].pCodecSpecificData,
               size);
        pAudioStreamInfo[i].nCodecSpecificDataLen = size;
    }

    streamDataInfo->pStreamInfo = pAudioStreamInfo;
    streamDataInfo->nStreamChangeFlag = 1;
    streamDataInfo->nStreamChangeNum  = pAudioinfo->audioNum;

    return 0;
}
#ifndef ONLY_ENABLE_AUDIO
static int setCacheNodeVideo(CdxPacketT *packet, CacheNode *node)
{
    struct VideoInfo *info = (struct VideoInfo *)calloc(1, sizeof(*info));
    if(!info)
    {
        loge("malloc fail, size=%d.", (int)sizeof(*info));
        return -1;
    }
    memcpy(info, packet->info, sizeof(struct VideoInfo));

    int i;
    for(i = 0; i < info->videoNum; i++)
    {
        size_t size = info->video[i].nCodecSpecificDataLen;
        if(!info->video[i].pCodecSpecificData || !size)
            continue;
        info->video[i].pCodecSpecificData = (char *)malloc(size);
        if(!info->video[i].pCodecSpecificData)
        {
            loge("malloc fail, size=%d.", (int)size);
            int j;
            for(j = 0; j < i; j++)
            {
                if(info->video[j].pCodecSpecificData)
                {
                    free(info->video[j].pCodecSpecificData);
                    info->video[j].pCodecSpecificData = NULL;
                }
            }
            free(info);
            return -1;
        }
        memcpy(info->video[i].pCodecSpecificData,
            ((struct VideoInfo *)packet->info)->video[i].pCodecSpecificData,
            size);
    }

    CdxAtomicSet(&info->ref, 0);
    CdxAtomicInc(&info->ref);
    node->info        = info;
    node->infoVersion = 1;

    return 0;
}
#endif
static int setCacheNodeAudio(CdxPacketT *packet, CacheNode *node)
{
    struct AudioInfo *info = (struct AudioInfo *)calloc(1, sizeof(*info));
    if(!info)
    {
        loge("malloc fail, size=%d.", (int)sizeof(*info));
        return -1;
    }
    memcpy(info, packet->info, sizeof(struct AudioInfo));

    int i;
    for(i = 0; i < info->audioNum; i++)
    {
        size_t size = info->audio[i].nCodecSpecificDataLen;
        if(!info->audio[i].pCodecSpecificData || !size)
            continue;

        info->audio[i].pCodecSpecificData = (char *)malloc(size);
        if(!info->audio[i].pCodecSpecificData)
        {
            loge("malloc fail, size=%d.", (int)size);
            int j;
            for(j = 0; j < i; j++)
            {
                if(info->audio[j].pCodecSpecificData)
                {
                    free(info->audio[j].pCodecSpecificData);
                    info->audio[j].pCodecSpecificData = NULL;
                }
            }
            free(info);
            return -1;
        }
        memcpy(info->audio[i].pCodecSpecificData,
            ((struct AudioInfo *)packet->info)->audio[i].pCodecSpecificData,
            size);
    }

    CdxAtomicSet(&info->ref, 0);
    CdxAtomicInc(&info->ref);
    node->info        = info;
    node->infoVersion = 1;

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
    int64_t           nLastCacheStateNotifyTimeUs;
    int64_t           nLastBufferingStartTimeUs;
    int64_t           nCacheStartPlaySizeIntervalUs;
    int64_t           nCurTimeUs;
#ifndef ONLY_ENABLE_AUDIO
    int               bVideoFirstFrameShowed;
    int               nVideoInfoVersion = 0;
#endif
    int               nAudioInfoVersion = 0;

    //* Declares below is to avoid stack overflow by goto statement.
    CdxPacketT          packet;
    enum EMEDIATYPE     ePlayerMediaType;
    MediaStreamDataInfo streamDataInfo;
    int                 nStreamIndex;

    demux = (DemuxCompContext*)arg;
    nLastCacheStateNotifyTimeUs         = 0;
    nLastBufferingStartTimeUs           = 0;
    nCacheStartPlaySizeIntervalUs       = 2000000;   //* adjust start play size every 2 seconds.
#ifndef ONLY_ENABLE_AUDIO
    bVideoFirstFrameShowed              = 0;
#endif
    demux->timeShiftLastSeqNum          =-1;

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
                CdxKeyedVectorT* pHeaders;

                uri = (char*)msg.params[3];
                pHeaders = (CdxKeyedVectorT*)msg.params[4];

                void* pHTTPServer;
                pHTTPServer = (void*)msg.params[5];

                if(setDataSourceFields(demux, &demux->source, pHTTPServer, uri, pHeaders) == 0)
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

                memset(&demux->source, 0x00, sizeof(demux->source));
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
            else if(demux->nSourceType == SOURCE_TYPE_DATASOURCE)
            {
                //* data source of MediaDataSource interface.
                clearDataSourceFields(&demux->source);
                memset(&demux->source, 0x00, sizeof(demux->source));

                char *uri = (char*)msg.params[3];
                demux->source.uri = strdup(uri);
                if(demux->source.uri != NULL)
                {
                    demux->eStatus = DEMUX_STATUS_INITIALIZED;
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                }
                else
                {
                    loge("can not dump string to represent interface.");
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
                clearDataSourceFields(&demux->source);
                memset(&demux->source, 0x00, sizeof(demux->source));

                char *uri = (char*)msg.params[3];
                demux->source.uri = strdup(uri);
                if(demux->source.uri != NULL)
                {
                    demux->eStatus = DEMUX_STATUS_INITIALIZED;
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                }
                else
                {
                    loge("can not dump string to represent interface.");
                    demux->eStatus = DEMUX_STATUS_IDLE;
                    if(pReplyValue != NULL)
                        *pReplyValue = -1;
                }
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }
        } //* end DEMUX_COMMAND_SET_SOURCE.
        else if(msg.messageId == DEMUX_COMMAND_PREPARE)
        {
            int flags = 0, ret = 0;
            int tryTimes = 0;

            logd("process message DEMUX_COMMAND_PREPARE.");

__prepareBegin:
            if(demux->pParser)
            {
                //* should not run here, pParser should be NULL under INITIALIZED or STOPPED status.
                logw("demux->pParser != NULL when DEMUX_COMMAND_PREPARE message received.");
                CdxParserClose(demux->pParser);
                demux->pParser = NULL;
                demux->pStream = NULL;
            }
            else if(demux->pStream)
            {
                CdxStreamClose(demux->pStream);
                demux->pStream = NULL;
            }

            if(demux->nSourceType == SOURCE_TYPE_ISTREAMSOURCE)
            {
                flags |= MIRACST;
            }
#if AWPLAYER_CONFIG_DISABLE_SUBTITLE
            flags |= DISABLE_SUBTITLE;
#endif
#if AWPLAYER_CONFIG_DISABLE_AUDIO
            flags |= DISABLE_AUDIO;
#endif
#if AWPLAYER_CONFIG_DISABLE_VIDEO
            flags |= DISABLE_VIDEO;
#endif
#if !AWPLAYER_CONFIG_DISALBE_MULTI_AUDIO
            flags |= MUTIL_AUDIO;
#endif

            struct CallBack cb;
            cb.callback = ParserCallbackProcess;
            cb.pUserData = (void *)demux;
/*
            static struct HdcpOpsS hdcpOps;
            hdcpOps.init = HDCP_Init;
            hdcpOps.deinit = HDCP_Deinit;
            hdcpOps.decrypt = HDCP_Decrypt;
*/
            ContorlTask parserContorlTask, *contorlTask;
            ContorlTask contorlTask1;
            parserContorlTask.cmd = CDX_PSR_CMD_SET_CALLBACK;
            parserContorlTask.param = (void *)&cb;
            parserContorlTask.next = NULL;
            contorlTask = &parserContorlTask;

            if(demux->pHDCPOps)
            {
                contorlTask1.cmd = CDX_PSR_CMD_SET_HDCP;
                contorlTask1.param = (void *)demux->pHDCPOps;
                contorlTask1.next = NULL;
                if(flags & MIRACST)
                {
                    contorlTask->next = &contorlTask1;
                    contorlTask = contorlTask->next;
                }
            }
//
            if(/*demux->mLivemode == 1 ||*/ demux->mLivemode == 2)
            {
                flags |= CMCC_TIME_SHIFT;
            }

            logd("=== prepare msg");

            ret = CdxParserPrepare(&demux->source, flags, &demux->mutex, &demux->bCancelPrepare,
                &demux->pParser, &demux->pStream, &parserContorlTask, &parserContorlTask);
            if(ret < 0)
            {
                if(demux->bCancelPrepare)
                {
                    logd("cancel prepare.");
                    goto _endTask;
                }

                if (strncasecmp(demux->source.uri, "http://", 7) != 0)
                {
                    logd("source.uri(%s)", demux->source.uri);
                    goto _endTask;
                }

                tryTimes++;
                if(tryTimes >= PREPARE_TRY_TIMES)
                {
                    logw("try %d times, exit.", tryTimes);
                    goto _endTask;
                }

                logw("parser prepare failed, try again.");

                usleep(100*1000);
                goto __prepareBegin;
                //goto _endTask;
            }
            if(CdxParserControl(demux->pParser, CDX_PSR_CMD_GET_REDIRECT_URL,
                            (void*)demux->shiftedTimeUrl))
            {
                //CDX_PSR_CMD_GET_REDIRECT_URL just for cmcc RESET_URL case
                logd("get redirect url failed");
            }
            logd("--- demux->shiftedTimeUrl = %s", demux->shiftedTimeUrl);

            CdxMediaInfoT parserMediaInfo;
            memset(&parserMediaInfo,0,sizeof(CdxMediaInfoT));
            CdxParserGetMediaInfo(demux->pParser, &parserMediaInfo);
            setMediaInfo(&demux->mediaInfo, &parserMediaInfo);
            demux->mediaInfo.eContainerType = (enum ECONTAINER)demux->pParser->type;

            {
                // record playmode, duration info.
                char cmccLog[4096];
                memset(cmccLog, 0, 4096);
                if(demux->mLivemode == 1 || demux->mLivemode == 2)
                    sprintf(cmccLog, "[info][%s %s %d]playmode: live, duration: %lldms",
                    LOG_TAG, __FUNCTION__, __LINE__, (long long int)demux->mediaInfo.nDurationMs);
                else
                    sprintf(cmccLog, "[info][%s %s %d]playmode: vod, duration: %lldms",
                    LOG_TAG, __FUNCTION__, __LINE__, (long long int)demux->mediaInfo.nDurationMs);
                demux->callback(demux->pUserData, DEMUX_NOTIFY_LOG_RECORDER, (void*)cmccLog);

                // record width, height info.
                memset(cmccLog, 0, 4096);
#ifndef ONLY_ENABLE_AUDIO
                if(parserMediaInfo.program[0].videoNum > 0)
                {
                    sprintf(cmccLog, "[info][%s %s %d]Video frame width: %d, height: %d",
                            LOG_TAG, __FUNCTION__, __LINE__,
                            demux->mediaInfo.pVideoStreamInfo[0].nWidth,
                            demux->mediaInfo.pVideoStreamInfo[0].nHeight);
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_LOG_RECORDER, (void*)cmccLog);
                }
#endif
                // record Video codec, Audio codec info.
                memset(cmccLog, 0, 4096);
#ifndef ONLY_ENABLE_AUDIO
                if(parserMediaInfo.program[0].audioNum > 0 &&
                    parserMediaInfo.program[0].videoNum > 0)
                {
                    sprintf(cmccLog, "[info][%s %s %d]Video codec: %d\n"
                        "       Audio codec: %d", LOG_TAG, __FUNCTION__, __LINE__,
                        demux->mediaInfo.pVideoStreamInfo[0].eCodecFormat,
                        demux->mediaInfo.pAudioStreamInfo[0].eCodecFormat);
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_LOG_RECORDER, (void*)cmccLog);
                }
#else
                if(parserMediaInfo.program[0].audioNum > 0)
                {
                    sprintf(cmccLog, "[info][%s %s %d]\n"
                        "       Audio codec: %d", LOG_TAG, __FUNCTION__, __LINE__,
                        demux->mediaInfo.pAudioStreamInfo[0].eCodecFormat);
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_LOG_RECORDER, (void*)cmccLog);
                }
#endif
                // record start decoder info.
                memset(cmccLog, 0, 4096);
                sprintf(cmccLog, "[info][%s %s %d]start decoder", LOG_TAG, __FUNCTION__, __LINE__);
                demux->callback(demux->pUserData, DEMUX_NOTIFY_LOG_RECORDER, (void*)cmccLog);
            }

            demux->bEOS     = 0;
            demux->bIOError = 0;

            if(demux->nSourceType == SOURCE_TYPE_URL)
            {
                if (!strncasecmp(demux->source.uri, "file://", 7) ||
                    !strncasecmp(demux->source.uri, "bdmv://", 7) || demux->source.uri[0] == '/')
                    demux->bFileStream = 1;
                else if(demux->mediaInfo.nDurationMs <= 0)
                    demux->bLiveStream = 1;
                else
                    demux->bVodStream = 1;
            }
            else if(demux->nSourceType == SOURCE_TYPE_FD)
                demux->bFileStream = 1;
            else
                demux->bFileStream = 1;//demux->bLiveStream = 1;
                //* treat IStreamSource(miracast) as a live stream.//demux->bFileStream = 1;

            //* if the parser is wvm , we should not start cache thread,
            //* because wvm need secure buffer.
            if(demux->bFileStream == 0 && demux->pParser->type != CDX_PARSER_WVM )
            {
#ifndef ONLY_ENABLE_AUDIO
                if((demux->mediaInfo.nVideoStreamNum > 0 && !(demux->mediaInfo.pVideoStreamInfo->bSecureStreamFlag))
                  ||(demux->mediaInfo.nVideoStreamNum <= 0 && demux->mediaInfo.nAudioStreamNum > 0))
                {
                    if(pthread_create(&demux->cacheThreadId, NULL, CacheThread, (void*)demux) == 0){
                        //* send a fetch message to start the cache loop.
                        memset(&newMsg, 0, sizeof(AwMessage));
                        newMsg.messageId = DEMUX_COMMAND_START;
                        AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                    }
                    else
                        demux->cacheThreadId = 0;
                }
#else
                if(demux->mediaInfo.nAudioStreamNum > 0)
                {
                    if(pthread_create(&demux->cacheThreadId, NULL, CacheThread, (void*)demux) == 0){
                        //* send a fetch message to start the cache loop.
                        memset(&newMsg, 0, sizeof(AwMessage));
                        newMsg.messageId = DEMUX_COMMAND_START;
                        AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                    }
                    else
                        demux->cacheThreadId = 0;
                }
#endif
            }

            //* set player and media format info to cache for seek processing.
            StreamCacheSetPlayer(demux->pCache, demux->pPlayer);
 #ifndef ONLY_ENABLE_AUDIO
            if(demux->mediaInfo.nVideoStreamNum > 0)
                StreamCacheSetMediaFormat(demux->pCache,
                       demux->pParser->type,
                       (enum EVIDEOCODECFORMAT)demux->mediaInfo.pVideoStreamInfo->eCodecFormat,
                       demux->mediaInfo.nBitrate);
            else
#endif
                StreamCacheSetMediaFormat(demux->pCache,
                                          demux->pParser->type,
 #ifndef ONLY_ENABLE_AUDIO
                                          VIDEO_CODEC_FORMAT_UNKNOWN,
#else
                                          0,
#endif
                                          demux->mediaInfo.nBitrate);
            demux->eStatus = DEMUX_STATUS_PREPARED;
            if(!demux->pBuffering && demux->bFileStream == 0)
            {
                DemuxCompSetCachePolicy((DemuxComp*)demux, CACHE_POLICY_ADAPTIVE, 0, 0, 0);
            }

#if defined(CONF_CMCC)
            if(demux->cacheThreadId != 0) //* cache some data before start player.
            {
                while(1)
                {
                    if(StreamCacheDataEnough(demux->pCache) || demux->bEOS || demux->bIOError)
                    {
                        logd("totSize(%d), startSize(%d)", demux->pCache->nDataSize,
                                    demux->pCache->nStartPlaySize);
                        if(demux->bEOS)
                        {
                            //* android.media.cts wait for Buffering Update Notify,
                            //* before MediaPlayer.start
                            NotifyCacheState(demux);
                        }
                        break;
                    }
                    else
                    {
                        //* wait for 10ms if no message come.
                        ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 10);
                        if(ret == 0)    //* new message come, quit loop to process.
                        {
                            goto process_message;
                        }
                    }
                }
            }
#endif
            if(demux->bFileStream == 0)
                demux->nStartPlayCacheTime = GetConfigParamterInt("start_play_cache_time",6)*1000;

            demux->callback(demux->pUserData, DEMUX_NOTIFY_PREPARED, 0);
            continue;

_endTask:
            demux->eStatus = DEMUX_STATUS_INITIALIZED;
            if(demux->bCancelPrepare)
            {
                demux->callback(demux->pUserData,
                        DEMUX_NOTIFY_PREPARED, (void*)DEMUX_ERROR_USER_CANCEL);
            }
            else
            {
                loge("DEMUX_ERROR_IO");
                demux->callback(demux->pUserData, DEMUX_NOTIFY_PREPARED, (void*)DEMUX_ERROR_IO);
            }
            continue;
        } //* end DEMUX_COMMAND_PREPARE.
        else if(msg.messageId == DEMUX_COMMAND_START)
        {
            logd("process message DEMUX_COMMAND_START.");

            if(demux->bBufferring)
            {
                //* user press the start button when buffering.
                //* start to play even the data is not enough.
                if(demux->bBufferStartNotified == 1)
                {
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_END, NULL);
                    demux->bBufferStartNotified = 0;

                }
                demux->bNeedPausePlayerNotified = 0;
                demux->bBufferring = 0;
            }

            if(demux->eStatus == DEMUX_STATUS_STARTED)
            {
                logi("demux already in started status.");
                //* send a read message to start the read loop.
                PostReadMessage(demux->mq);
                if(pReplyValue != NULL)
                    *pReplyValue = 0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            if(demux->eStatus != DEMUX_STATUS_PREPARED)
            {
                loge("demux not in prepared status when DEMUX_COMMAND_START message received.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }
#ifndef ONLY_ENABLE_AUDIO
            //* in case of bVideoFirstFrameShowed == 0, demux will continue to send data to player
            //* until video first frame showed.
            //* here we set this flag to let the demux don't send data at buffering status.
            if(demux->pPlayer == NULL || PlayerHasVideo(demux->pPlayer) == 0)
                bVideoFirstFrameShowed = 1;
#endif
            demux->eStatus = DEMUX_STATUS_STARTED;
            //* send a read message to start the read loop.
            PostReadMessage(demux->mq);
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
                memset(&newMsg, 0, sizeof(AwMessage));
                newMsg.messageId = DEMUX_COMMAND_QUIT;
                newMsg.params[0] = (uintptr_t)&demux->semCache;
                AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                SemTimedWait(&demux->semCache, -1);
                pthread_join(demux->cacheThreadId, &status);
                demux->cacheThreadId = 0;
                StreamCacheFlushAll(demux->pCache);
                if(demux->bBufferStartNotified == 1)
                {
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_END, NULL);
                    demux->bBufferStartNotified = 0;
                    demux->bNeedPausePlayerNotified = 0;
                }
            }

            if(demux->pParser != NULL)
            {
                CdxParserClose(demux->pParser);
                demux->pParser = NULL;
                demux->pStream = NULL;
            }
            else if(demux->pStream)
            {
                CdxStreamClose(demux->pStream);
                demux->pStream = NULL;
            }
#ifndef ONLY_ENABLE_AUDIO
            bVideoFirstFrameShowed = 0;
#endif
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
            logv("process message DEMUX_COMMAND_QUIT or DEMUX_COMMAND_CLEAR.");

            //* stop the cache thread if it is not stopped yet.
            if(demux->cacheThreadId != 0)
            {
                void* status;
                memset(&newMsg, 0, sizeof(AwMessage));
                newMsg.messageId = DEMUX_COMMAND_QUIT;
                newMsg.params[0] = (uintptr_t)&demux->semCache;
                AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                SemTimedWait(&demux->semCache, -1);
                pthread_join(demux->cacheThreadId, &status);
                demux->cacheThreadId = 0;

                if(demux->bBufferStartNotified == 1)
                {
                    unsigned char url[1024];
                    if(demux->pParser)
                    {
                        if(CdxParserControl(demux->pParser, CDX_PSR_CMD_GET_URL, (void*)url))
                        {
                            loge("get  url failed");
                        }
                    }
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_END, NULL);
                    demux->bBufferStartNotified = 0;
                    demux->bNeedPausePlayerNotified = 0;
                }
            }

            //mediaInfo need to free here
            clearMediaInfo(&demux->mediaInfo);

            if(demux->pParser != NULL)
            {
                CdxParserClose(demux->pParser);
                demux->pParser = NULL;
                demux->pStream = NULL;
            }
            else if(demux->pStream)
            {
                CdxStreamClose(demux->pStream);
                demux->pStream = NULL;
            }

            clearDataSourceFields(&demux->source);
            demux->eStatus = DEMUX_STATUS_IDLE;
            demux->bStopping = 0;
#ifndef ONLY_ENABLE_AUDIO
            bVideoFirstFrameShowed = 0;
#endif
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
            int nSeekTimeMs;
            int nFinalSeekTimeMs;
            int params[3];

            logd("process message DEMUX_COMMAND_SEEK.");
#ifndef ONLY_ENABLE_AUDIO
            bVideoFirstFrameShowed = 0;
#endif
            nSeekTimeMs = msg.params[2];
            if(demux->pParser != NULL)
            {
                if(demux->cacheThreadId != 0)
                {
                    memset(&newMsg, 0, sizeof(AwMessage));
                    newMsg.messageId = DEMUX_COMMAND_PAUSE;
                    newMsg.params[0] = (uintptr_t)&demux->semCache;
                    AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                    SemTimedWait(&demux->semCache, -1);
                    int64_t nSeekTimeUs = nSeekTimeMs*1000LL;
                    int64_t nSeekTimeFinal = StreamCacheSeekTo(demux->pCache, nSeekTimeUs);
                    int n = 0;

                    demux->bEOS     = 0;
                    demux->bIOError = 0;
                    if(nSeekTimeFinal < 0)
                    {
                        StreamCacheFlushAll(demux->pCache);
                        demux->nCacheReply = -1;

                        memset(&newMsg, 0, sizeof(AwMessage));
                        newMsg.messageId = DEMUX_COMMAND_SEEK;
                        newMsg.params[0] = (uintptr_t)&demux->semCache;
                        newMsg.params[1] = (uintptr_t)&demux->nCacheReply;
                        newMsg.params[2] = (uintptr_t)nSeekTimeMs;
                        AwMessageQueuePostMessage(demux->mqCache, &newMsg);

                        if(demux->bBufferStartNotified == 1)
                        {
                            demux->callback(demux->pUserData,
                                    DEMUX_NOTIFY_BUFFER_END, NULL);
                            demux->bBufferStartNotified = 0;
                        }

                        //* wait seek complete
                        while(demux->nCacheReply)
                        {
                            if(demux->bCancelSeek)
                            {
                                break;
                            }

                            ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 50);
                            if(ret == 0)    //* new message come, quit loop to process.
                            {
                                goto process_message;
                            }

                            n++;
                            if(n>=40 && !demux->bBufferStartNotified)
                            {
                                logd("++++++++++ send buffer Start, in demux thread");
                                unsigned char url[1024];
                                if(demux->pParser)
                                {
                                    if(CdxParserControl(demux->pParser,
                                            CDX_PSR_CMD_GET_URL, (void*)url))
                                    {
                                        loge("get  url failed");
                                    }
                                }

                                demux->callback(demux->pUserData,
                                        DEMUX_NOTIFY_BUFFER_START,
                                        (void*)&demux->pCache->nStartPlaySize);
                                demux->bBufferStartNotified = 1;
                                demux->bBufferring = 1;
                            }
                        }

                    }
                    else
                    {
                        if(demux->bBufferStartNotified == 1)
                        {
                            unsigned char url[1024];
                            if(demux->pParser)
                            {
                                if(CdxParserControl(demux->pParser,
                                        CDX_PSR_CMD_GET_URL, (void*)url))
                                {
                                    loge("get  url failed");
                                }
                            }
                            demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_END, NULL);
                            demux->bBufferStartNotified = 0;
                        }

                        //* since DemuxCompSeekTo use CdxParserForceStop, clear it here.
                        pthread_mutex_lock(&demux->mutex);
                        if(demux->bCancelSeek == 0 && demux->bStopping == 0)
                        {
                            logv("CdxParserClrForceStop");
                            ret = CdxParserClrForceStop(demux->pParser);
                            if(ret < 0)
                            {
                                logw("CdxParserClrForceStop fail, ret(%d)", ret);
                            }
                        }
                        pthread_mutex_unlock(&demux->mutex);

                        params[0] = 0;
                        params[1] = nSeekTimeMs;
                        params[2] = nSeekTimeFinal/1000;
                        demux->callback(demux->pUserData, DEMUX_NOTIFY_SEEK_FINISH, (void*)params);
                        demux->bSeeking = 0;

                        memset(&newMsg, 0, sizeof(AwMessage));
                        newMsg.messageId = DEMUX_COMMAND_START;
                        newMsg.params[0] = (uintptr_t)&demux->semCache;
                        AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                        SemTimedWait(&demux->semCache, -1);
                    }

                    if(demux->eStatus == DEMUX_STATUS_COMPLETE)
                        demux->eStatus = DEMUX_STATUS_STARTED;

                    if(demux->eStatus == DEMUX_STATUS_STARTED)
                        PostReadMessage(demux->mq); //* send read message to start reading loop.
                    demux->bBufferring = 0;
                    demux->bNeedPausePlayerNotified = 0;
                    continue;
                }
                else
                {

                    //* lock the mutex to sync with the DemuxCompCancelSeek()
                    //* operation or DEmuxCompStop() operation.
                    //* when user are requesting to quit the seek operation,
                    //* we should not clear the parser's force
                    //* stop flag, other wise the parser may blocked at a network io operation.
                    pthread_mutex_lock(&demux->mutex);
                    if(demux->bCancelSeek == 0 && demux->bStopping == 0)
                    {
                        ret = CdxParserClrForceStop(demux->pParser);
                        if(ret < 0)
                        {
                            logw("CdxParserClrForceStop fail, ret(%d)", ret);
                        }
                    }
                    pthread_mutex_unlock(&demux->mutex);

                    if(CdxParserSeekTo(demux->pParser, ((int64_t)nSeekTimeMs)*1000, demux->nSeekModeType) >= 0)
                    {
                        nFinalSeekTimeMs = nSeekTimeMs;
                        ret = 0;
                    }
                    else
                        ret = -1;

                    if(ret == 0)
                    {
                        params[0] = 0;
                        params[1] = nSeekTimeMs;
                        params[2] = nFinalSeekTimeMs;
                        demux->callback(demux->pUserData, DEMUX_NOTIFY_SEEK_FINISH, (void*)params);

                        demux->bSeeking = 0;
                        demux->bEOS     = 0;
                        demux->bIOError = 0;

                        if(demux->eStatus == DEMUX_STATUS_COMPLETE)
                            demux->eStatus = DEMUX_STATUS_STARTED;

                        if(demux->eStatus == DEMUX_STATUS_STARTED)
                            PostReadMessage(demux->mq); //* send read message to start reading loop.

                        if(pReplyValue != NULL)
                            *pReplyValue = 0;
                    }
                    else
                    {
                        loge("CdxParserSeekTo() return fail, demux->bCancelSeek:%d",
                                    demux->bCancelSeek);
                        //* set to complete status to stop reading.
                        demux->eStatus = DEMUX_STATUS_COMPLETE;
                        demux->bSeeking = 0;
                        if(demux->bCancelSeek == 1 || demux->bStopping == 1)
                            params[0] = DEMUX_ERROR_USER_CANCEL;
                        else
                            params[0] = DEMUX_ERROR_IO;
                        params[1] = nSeekTimeMs;
                        demux->callback(demux->pUserData, DEMUX_NOTIFY_SEEK_FINISH, (void*)params);

                        if(pReplyValue != NULL)
                            *pReplyValue = -1;
                    }
                    demux->bBufferring = 0;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;

                }
            }
            else
            {
                params[0] = DEMUX_ERROR_UNKNOWN;
                params[1] = nSeekTimeMs;
                demux->bSeeking = 0;
                demux->callback(demux->pUserData,
                        DEMUX_NOTIFY_SEEK_FINISH, (void*)params);

                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }
        }
        else if(msg.messageId == DEMUX_COMMAND_RESET_URL)
        {
            //* resetUrl in demux thread, send a message to cache thread
            //* and then wait the cache thread finish resetUrl message
            int nSeekTimeMs;
            int nFinalSeekTimeMs;
            int params[3];
            int64_t startTime , nowTime;

            struct timeval tv;
            gettimeofday(&tv, NULL);
            startTime = tv.tv_sec * 1000000ll + tv.tv_usec;

            logd("process message DEMUX_COMMAND_RESET_URL.");
#ifndef ONLY_ENABLE_AUDIO
            bVideoFirstFrameShowed = 0;
#endif
            nSeekTimeMs = msg.params[2];

            char *pLiveModeType = NULL;
            char *pStartime = NULL;
            char TimeStr[512] = {0};
            pLiveModeType = strstr(demux->shiftedTimeUrl, "livemode=");//check livemode here

            //get the shifted time format string here
            if ((ShiftTimeMode(nSeekTimeMs, TimeStr)) == 2)
            {
                //* convert live to pltv
                char cmccLog[4096] = "";
                sprintf(cmccLog, "[info][%s %s %d]convert to pltv",
                        LOG_TAG, __FUNCTION__, __LINE__);
                demux->callback(demux->pUserData,
                            DEMUX_NOTIFY_LOG_RECORDER, (void*)cmccLog);

                demux->mLivemode = 2;
                pLiveModeType[9] = '2';//get shifted event with livemode1, will change to livemode2
                pStartime = strstr(demux->shiftedTimeUrl, "&starttime=");
                if (pStartime != NULL)
                {
                    sprintf(pStartime, "&starttime=%s", TimeStr);//generate the shifted url
                }
                else
                {
                    char tmpShiftedTimeUrl[4096];
                    snprintf(tmpShiftedTimeUrl,sizeof(tmpShiftedTimeUrl), "%s",demux->shiftedTimeUrl);
                    snprintf(demux->shiftedTimeUrl,sizeof(demux->shiftedTimeUrl), "%s&starttime=%s",tmpShiftedTimeUrl, TimeStr);
                }
            }
            else
            {

                //* convert pltv to live
                char cmccLog[4096] = "";
                sprintf(cmccLog, "[info][%s %s %d]convert to live",
                        LOG_TAG, __FUNCTION__, __LINE__);
                demux->callback(demux->pUserData, DEMUX_NOTIFY_LOG_RECORDER, (void*)cmccLog);

                demux->mLivemode = 1;
                pLiveModeType[9] = '1';
                pStartime = strstr(demux->shiftedTimeUrl, "&starttime=");
                if (pStartime != NULL)
                {
                    pStartime[0] = '\0';
                }
            }

            logd("ShiftedTimeUrl = %s", demux->shiftedTimeUrl);

            if(demux->pParser != NULL)
            {

                CdxParserClose(demux->pParser);
                demux->pParser = NULL;
                demux->pStream = NULL;
            }
            else if(demux->pStream != NULL)
            {
                CdxStreamClose(demux->pStream);
                demux->pStream = NULL;
            }

            //* send reset_url message to cache thread, and wait parserOpen
            //if(demux->cacheThreadId != 0)
            {
                memset(&newMsg, 0, sizeof(AwMessage));
                newMsg.messageId = DEMUX_COMMAND_PAUSE;
                newMsg.params[0] = (uintptr_t)&demux->semCache;
                AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                SemTimedWait(&demux->semCache, -1);
                StreamCacheFlushAll(demux->pCache);

                memset(&newMsg, 0, sizeof(AwMessage));
                newMsg.messageId = DEMUX_COMMAND_RESET_URL;
                newMsg.params[0] = (uintptr_t)&demux->semCache;
                newMsg.params[1] = (uintptr_t)&demux->nCacheReply;
                newMsg.params[2] = (uintptr_t)nSeekTimeMs;
                AwMessageQueuePostMessage(demux->mqCache, &newMsg);

                if(demux->bBufferStartNotified == 1)
                {
                    demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_END, NULL);
                    demux->bBufferStartNotified = 0;
                }

                // blocking here, wait for parserPrepare in cache thread
                while(demux->pParser == NULL)
                {
                    if(demux->bCancelSeek)
                    {
                        break;
                    }
                    //ret = SemTimedWait(&demux->semCache, 50);
                    usleep(50*1000); // sleep 50ms

                    gettimeofday(&tv, NULL);
                    nowTime = tv.tv_sec * 1000000ll + tv.tv_usec;
                    // after 2s, send bufferStart
                    if((nowTime-startTime > 2000000) && !demux->bBufferStartNotified)
                    {
                        logd("++++++++++ send buffer Start, in demux thread");
                        demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_START,
                                (void*)&demux->pCache->nStartPlaySize);
                        demux->bBufferStartNotified = 1;
                        demux->bBufferring = 1;
                    }
                }
                SemTimedWait(&demux->semCache, -1); // wait DEMUX_COMMAND_RESET_URL in cache thread
            }

            nFinalSeekTimeMs = nSeekTimeMs;
            ret = demux->nCacheReply;
            logd("demux thread: ret = %d", ret);
            if(ret == 0)
            {

                demux->bSeeking = 0;
                demux->bEOS     = 0;
                demux->bIOError = 0;

                //* send a start message to the cache thread.
                if(demux->cacheThreadId != 0)
                {
                    memset(&newMsg, 0, sizeof(AwMessage));
                    newMsg.messageId = DEMUX_COMMAND_START;
                    newMsg.params[0] = (uintptr_t)&demux->semCache;
                    AwMessageQueuePostMessage(demux->mqCache, &newMsg);
                    SemTimedWait(&demux->semCache, -1);
                }

                if(demux->eStatus == DEMUX_STATUS_COMPLETE)
                    demux->eStatus = DEMUX_STATUS_STARTED;


                // *blocking here to buffering data
                StreamCacheSetSize(demux->pCache, demux->nStartPlayCacheSize,
                        demux->nCacheBufferSize);
                while(!StreamCacheDataEnough(demux->pCache))
                {
                    usleep(20*1000);

                    gettimeofday(&tv, NULL);
                    nowTime = tv.tv_sec * 1000000ll + tv.tv_usec;
                     // after 2s, send bufferStart
                    if((nowTime-startTime > 2000000) && !demux->bBufferStartNotified)
                    {
                        logd("++++++++++ send buffer Start, in demux thread");
                        demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_START,
                                    (void*)&demux->pCache->nStartPlaySize);
                        demux->bBufferStartNotified = 1;
                        demux->bBufferring = 1;
                    }

                    if(demux->bEOS || demux->bIOError)
                    {
                        loge("eos in livemode2, error");
                    }
                    if(demux->bCancelSeek)
                    {
                        break;
                    }
                }

                if(demux->eStatus == DEMUX_STATUS_STARTED)
                    PostReadMessage(demux->mq); //* send read message to start reading loop.

                params[0] = 0;
                params[1] = nSeekTimeMs;
                params[2] = nFinalSeekTimeMs;
                demux->callback(demux->pUserData, DEMUX_NOTIFY_SEEK_FINISH, (void*)params);

                if(pReplyValue != NULL)
                    *pReplyValue = 0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }
            else
            {

                loge("CdxParserResetUrl() return fail, bCancelSeek = %d", demux->bCancelSeek);
                demux->eStatus = DEMUX_STATUS_COMPLETE; //* set to complete status to stop reading.
                demux->bSeeking = 0;
                //if(demux->bCancelSeek == 1 || demux->bStopping == 1)
                //* we cannot set it to DEMUX_ERROR_USER_CANCEL in cmcc, it will error out
                    params[0] = DEMUX_ERROR_USER_CANCEL;
                //else
                //    params[0] = DEMUX_ERROR_IO;
                params[1] = nSeekTimeMs;
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

            pthread_mutex_lock(&demux->lock);
            while(demux->bSeeking)
            {
                pthread_cond_wait(&demux->cond, &demux->lock);
            }
            pthread_mutex_unlock(&demux->lock);
            demux->bCancelSeek = 0;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        }
        else if(msg.messageId == DEMUX_COMMAND_NOTIFY_FIRST_FRAME_SHOWED)
        {
#ifndef ONLY_ENABLE_AUDIO
            bVideoFirstFrameShowed = 1;
#endif
            if(demux->eStatus == DEMUX_STATUS_STARTED)
                PostReadMessage(demux->mq);

            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        }
#ifndef ONLY_ENABLE_AUDIO
        else if(msg.messageId == DEMUX_COMMAND_VIDEO_STREAM_CHANGE)
        {
            logv("process message DEMUX_COMMAND_VIDEO_STREAM_CHANGE!");

            int nMsg =  DEMUX_VIDEO_STREAM_CHANGE;
            int params[2];

            params[0] = (int)msg.params[2];

            demux->callback(demux->pUserData, nMsg, (void*)params);

            PostReadMessage(demux->mq);
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);

            continue;
        } //* end DEMUX_COMMAND_VIDEO_STREAM_CHANGE.
#endif
        else if(msg.messageId == DEMUX_COMMAND_AUDIO_STREAM_CHANGE)
        {
            logv("process message DEMUX_COMMAND_AUDIO_STREAM_CHANGE!");

            int nMsg =  DEMUX_AUDIO_STREAM_CHANGE;
            demux->callback(demux->pUserData, nMsg, NULL);

            PostReadMessage(demux->mq);
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);

            continue;
        } //* end DEMUX_COMMAND_AUDIO_STREAM_CHANGE.
        else if(msg.messageId == DEMUX_COMMAND_READ)
        {
            logv("process message DEMUX_COMMAND_READ.");

            if(demux->eStatus != DEMUX_STATUS_STARTED)
            {
                logw("demux component not in started status, ignore read message.");
                continue;
            }
#ifndef ONLY_ENABLE_AUDIO
            if (GetConfigParamterInt("show_1th_frame_quick", 0) != 1)
                bVideoFirstFrameShowed = 1;//* cmcc not show first frame immediately
#endif
            if(demux->cacheThreadId != 0)
            {
                if(demux->bFileStream == 0)
                {
                    nCurTimeUs = GetSysTime();

                    //**************************************************************
                    //* notify the cache status.
                    //**************************************************************
                    if(nCurTimeUs >= (nLastCacheStateNotifyTimeUs +
                        demux->nCacheStatReportIntervalUs) ||
                        nCurTimeUs < nLastCacheStateNotifyTimeUs)
                    {
                        NotifyCacheState(demux);
                        nLastCacheStateNotifyTimeUs = nCurTimeUs;
                    }
                }

                //**************************************************************
                //* read data from cache.
                //**************************************************************
                if(demux->bBufferring)
                {
                    //* the player is paused and caching stream data.
                    //* check whether data in cache is enough for play.
                    if(StreamCacheDataEnough(demux->pCache) || demux->bEOS || demux->bIOError)
                    {
                        logd("detect data enough, notify BUFFER_END.");
                        demux->bBufferring = 0;
                        if(demux->bBufferStartNotified == 1)
                        {
                            unsigned char url[1024];
                            if(demux->pParser)
                            {
                                if(CdxParserControl(demux->pParser,
                                    CDX_PSR_CMD_GET_URL, (void*)url))
                                {
                                    loge("get  url failed");
                                }
                            }
                            demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_END, (void*)url);
                            demux->bBufferStartNotified = 0;
                        }
                        demux->callback(demux->pUserData, DEMUX_NOTIFY_RESUME_PLAYER, NULL);
                        demux->bNeedPausePlayerNotified = 0;
                    }
                    else
                    {
                        if(demux->bBufferStartNotified == 0)
                        {
                            nCurTimeUs = GetSysTime();
                            if(nCurTimeUs > (nLastBufferingStartTimeUs + 2000000) ||
                               nCurTimeUs < nLastBufferingStartTimeUs)
                            {
                                //* had been paused for buffering for more than 2 seconds,
                                //* notify buffer start message to let the application know.
                                logd("after 2s, notify BUFFER_START.");
                                unsigned char url[1024];
                                if(demux->pParser)
                                {
                                    if(CdxParserControl(demux->pParser,
                                            CDX_PSR_CMD_GET_URL, (void*)url))
                                    {
                                        loge("get  url failed");
                                    }
                                }
                                logd("-- bufferStart url = %s", url);
                                demux->callback(demux->pUserData, DEMUX_NOTIFY_BUFFER_START,
                                            (void*)&demux->pCache->nStartPlaySize);
                                demux->bBufferStartNotified = 1;
                            }
                        }
#ifndef ONLY_ENABLE_AUDIO
                        if(bVideoFirstFrameShowed == 1)
                        {
                            //* wait some time for caching.
                            ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 10);
                            if(ret == 0)    //* new message come, quit loop to process.
                                goto process_message;

                            //* post a read message to continue the reading job.
                            PostReadMessage(demux->mq);
                            continue;
                        }
#endif
                    }
                }

                if(demux->pBuffering && !demux->mStreamPrepared)
                {
                    int ret_caching = StreamBufferingPrepared(demux->pCache);
                    if(ret_caching)
                    {
                        demux->mStreamPrepared = 1;
                    }
                }
                if(demux->pBuffering && !demux->mStreamPrepared)
                {
                    ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 10);
                    if(ret == 0)
                        goto process_message;
                    PostReadMessage(demux->mq);
                    continue;
                }
                //* check whether cache underflow.
                if(StreamCacheUnderflow(demux->pCache))
                {
                    logv("detect cache data underflow.");
                    //* cache underflow, if not eos, we need to notify pausing,
                    //* otherwise we need to notify complete.
                    if(demux->bEOS)
                    {
                        //* end of stream, notify complete.
                        logi("detect eos, notify EOS.");
                        demux->callback(demux->pUserData, DEMUX_NOTIFY_EOS, 0);
                        demux->eStatus = DEMUX_STATUS_COMPLETE;
                        continue;
                    }
                    else if(demux->bIOError)
                    {
                        logi("detect io error, notify IOERROR.");
                        //* end of stream, notify complete.
                        demux->callback(demux->pUserData, DEMUX_NOTIFY_IOERROR, 0);
                        continue;
                    }
                    else
                    {
                        //* no data in cache, check whether player hold enough data,
                        //* if not, we need to notify pausing to wait for caching
                        //* more data for player.
                        // in cmcc, we should skip the next code PlayerBufferUnderflow
                        if(PlayerBufferUnderflow(demux->pPlayer) &&
                            demux->bNeedPausePlayerNotified == 0
#ifndef ONLY_ENABLE_AUDIO
                            && bVideoFirstFrameShowed == 1
#endif
                            )
                        {
                            logd("detect player data underflow, notify PAUSE_PLAYER. ");
                            if(demux->bDiscontinue)
                            {
                                demux->bDiscontinue = 0;
                                if(CdxParserControl(demux->pParser,
                                        CDX_PSR_CMD_SET_HLS_STREAM_FORMAT_CHANGE, NULL))
                                {
                                    loge("***hls set stream format change fail");
                                }
                            }
                            else
                            {
                                demux->bBufferring = 1;
                                nLastBufferingStartTimeUs = GetSysTime();
                                demux->callback(demux->pUserData, DEMUX_NOTIFY_PAUSE_PLAYER, NULL);
                                demux->bNeedPausePlayerNotified = 1;

                                //**************************************************************
                                //* adjust cache params.
                                //**************************************************************
                                AdjustCacheParams(demux);
                            }
                        }

                        //* wait some time for caching.
                        ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 10);
                        if(ret == 0)    //* new message come, quit loop to process.
                            goto process_message;

                        //* post a read message to continue the reading job.
                        PostReadMessage(demux->mq);
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
                        PostReadMessage(demux->mq);
                        continue;
                    }
                    else
                    {
                        //*************************************
                        //* send data from cache to player.
                        //*************************************
                        CacheNode*          node;
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
                            loge("Cache underflow");
                            PostReadMessage(demux->mq);
                            continue;
                        }

                        //********************************
                        //* 2. request buffer from player.
                        //********************************
#ifndef ONLY_ENABLE_AUDIO
                        if(node->eMediaType == CDX_MEDIA_VIDEO)
                        {
                            ePlayerMediaType = MEDIA_TYPE_VIDEO;
                            nStreamIndex     = (node->nFlags&MINOR_STREAM)==0 ? 0 : 1;
                        }
                        else
#endif
                        if(node->eMediaType == CDX_MEDIA_AUDIO)
                        {
                            ePlayerMediaType = MEDIA_TYPE_AUDIO;
                            nStreamIndex     = node->nStreamIndex;
                        }
#ifndef ONLY_ENABLE_AUDIO
                        else if(node->eMediaType == CDX_MEDIA_SUBTITLE)
                        {
                            ePlayerMediaType = MEDIA_TYPE_SUBTITLE;
                            nStreamIndex     = node->nStreamIndex;
                        }
#endif
                        else if(node->eMediaType == CDX_MEDIA_METADATA)
                        {
                            ePlayerMediaType = MEDIA_TYPE_METADATA;
                            nStreamIndex     = node->nStreamIndex;
                        }
                        else
                        {
                            loge("media type from parser not valid, should not run here, abort().");
                            abort();
                        }

                        while(1)
                        {
                            if(ePlayerMediaType != MEDIA_TYPE_METADATA)
                                ret = PlayerRequestStreamBuffer(demux->pPlayer,
                                                                node->nLength,
                                                                &pBuf0,
                                                                &nBufSize0,
                                                                &pBuf1,
                                                                &nBufSize1,
                                                                ePlayerMediaType,
                                                                nStreamIndex);
                            else
                            {
                                //* allocate a buffer to read uncare media data and skip it.
                                pBuf0 = malloc(node->nLength);
                                if(pBuf0 != NULL)
                                {
                                    nBufSize0 = node->nLength;
                                    pBuf1     = NULL;
                                    nBufSize1 = 0;
                                    ret = 0;
                                }
                                else
                                {
                                    nBufSize0 = 0;
                                    pBuf1     = NULL;
                                    nBufSize1 = 0;
                                    ret = -1;
                                }
                            }

                            if ((nBufSize0 + nBufSize1) < node->nLength)
                                ret = -1;

                            if(ret < 0)
                            {
                                //logi("waiting for stream buffer.");
                                //* no buffer, try to wait sometime.
                                ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 200);
                                if(ret == 0)    //* new message come, quit loop to process.
                                    goto process_message;

                                /* player is paused due to cache underflow, don't wait forever */
                                if (demux->bNeedPausePlayerNotified == 1)
                                    break;
                            }
                            else
                                break;  //* get buffer ok.
                        }

                        /* player is paused due to cache underflow, and audio
                         * stream is overflow. The funtion of 'show the first
                         * video frame in paused status after seek' won't work.
                         * Set bVideoFirstFrameShowed to one and wait until
                         * stream cache data enough.
                         */
                        if (ret < 0)
                        {
                            logd("don't wait for stream buffer in paused status");
#ifndef ONLY_ENABLE_AUDIO
                            bVideoFirstFrameShowed = 1;
#endif
                            PostReadMessage(demux->mq);
                            continue;
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

                        if(ePlayerMediaType != MEDIA_TYPE_METADATA)
                        {
                            streamDataInfo.pData        = (char*)pBuf0;
                            streamDataInfo.nLength      = node->nLength;
                            streamDataInfo.nPts         = node->nPts;
                            streamDataInfo.nPcr         = node->nPcr;
                            streamDataInfo.bIsFirstPart = 1;
                            streamDataInfo.bIsLastPart  = 1;
                            streamDataInfo.nDuration    = node->nDuration;
                            streamDataInfo.pStreamInfo  = NULL;
                            streamDataInfo.nStreamChangeFlag = 0;
                            streamDataInfo.nStreamChangeNum = 0;
#ifndef ONLY_ENABLE_AUDIO
                            if(ePlayerMediaType == MEDIA_TYPE_VIDEO && node->infoVersion == 1)
                            {
                                logd(" demux -- video stream info change: node->info = %p",node->info);
                                if(node->info)
                                {
                                    int ret;
                                    struct VideoInfo* pVideoinfo = (struct VideoInfo *)node->info;
                                    ret = setVideoStreamDataInfo(&streamDataInfo, pVideoinfo);
                                    if (ret < 0)
                                    {
                                        loge("setVideoStreamDataInfo fail.");
                                        abort();
                                    }
                                }
                                nVideoInfoVersion = node->infoVersion;
                            }
                            else
#endif
                            if(ePlayerMediaType == MEDIA_TYPE_AUDIO && node->infoVersion == 1)
                            {
                                logd(" demux -- audio stream info change: node->info = %p",node->info);
                                if(node->info)
                                {
                                    int ret;
                                    struct AudioInfo* pAudioinfo = (struct AudioInfo *)node->info;
                                    ret = setAudioStreamDataInfo(&streamDataInfo, pAudioinfo);
                                    if (ret < 0)
                                    {
                                        loge("setAudioStreamDataInfo fail.");
                                        abort();
                                    }
                                }
                                nAudioInfoVersion = node->infoVersion;
                            }
                            PlayerSubmitStreamData(demux->pPlayer, &streamDataInfo,
                                    ePlayerMediaType, nStreamIndex);

                            StreamCacheFlushOneFrame(demux->pCache);
                        }
                        else
                        {
                            //* skip the media data.
                            unsigned int params[4];
                            params[0] = (unsigned int)(node->nPts >> 32);//high 32bits
                            params[1] = (unsigned int)(node->nPts & 0xffffffff);//low 32bits
                            params[2] = (unsigned int)node->nLength;
                            params[3] = (unsigned int)pBuf0;
                            demux->callback(demux->pUserData, DEMUX_NOTIFY_METADATA, (void*)params);
                            free(pBuf0);
                            StreamCacheFlushOneFrame(demux->pCache);
                        }
                        //* post a read message to continue the reading job.
                        PostReadMessage(demux->mq);
                        continue;
                    }   //* end if(PlayerBufferOverflow(...)){}else {}
                }   //* end if(StreamCacheUnderflow(...)){}else {}
            }
            else
            {
                //**************************************************************
                //* read data directly from parser.
                //**************************************************************

                memset(&packet, 0x00, sizeof(CdxPacketT));
                //* if data in player is not too much, send it to player,
                //* otherwise don't read.
                if(PlayerBufferOverflow(demux->pPlayer))
                {
                    //* too much data in player, wait some time.
                    ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 200);
                    if(ret == 0)    //* new message come, quit loop to process.
                        goto process_message;

                    //* post a read message to continue the reading job.
                    PostReadMessage(demux->mq);
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
                        else if (err == PSR_USER_CANCEL) // TODO:
                        {
                            /* do noting */
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
#ifndef ONLY_ENABLE_AUDIO
                //* 2. request buffer from player.
                if(packet.type == CDX_MEDIA_VIDEO)
                {
                    ePlayerMediaType = MEDIA_TYPE_VIDEO;
                    nStreamIndex     = (packet.flags&MINOR_STREAM)==0 ? 0 : 1;
                }
                else
#endif
                if(packet.type == CDX_MEDIA_AUDIO)
                {
                    ePlayerMediaType = MEDIA_TYPE_AUDIO;
                    nStreamIndex     = packet.streamIndex;
                }
#ifndef ONLY_ENABLE_AUDIO
                else if(packet.type == CDX_MEDIA_SUBTITLE)
                {
                    ePlayerMediaType = MEDIA_TYPE_SUBTITLE;
                    nStreamIndex     = packet.streamIndex;
                }
#endif
                else if(packet.type == CDX_MEDIA_METADATA)
                {
                    ePlayerMediaType = MEDIA_TYPE_METADATA;
                    nStreamIndex     = packet.streamIndex;
                }
                else
                {
                    loge("media type from parser not valid, should not run here, abort().");
                    abort();
                }

                while(1)
                {
                    if(
#ifndef ONLY_ENABLE_AUDIO
                      (!AWPLAYER_CONFIG_DISABLE_VIDEO && ePlayerMediaType == MEDIA_TYPE_VIDEO) ||
#endif
                       (!AWPLAYER_CONFIG_DISABLE_AUDIO && ePlayerMediaType == MEDIA_TYPE_AUDIO)
#ifndef ONLY_ENABLE_AUDIO
                       ||(!AWPLAYER_CONFIG_DISABLE_SUBTITLE &&ePlayerMediaType == MEDIA_TYPE_SUBTITLE)
#endif
                       )
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
                        //logi("waiting for stream buffer.");
                        //* no buffer, try to wait sometime.
                        ret = AwMessageQueueTryGetMessage(demux->mq, &msg, 200);
                        if(ret == 0)    //* new message come, quit loop to process.
                            goto process_message;
                    }
                    else
                        break;  //* get buffer ok.
                }

                //* 3. read data to buffer and submit.
                ret = CdxParserRead(demux->pParser, &packet);
                if(ret == 0)
                {
                    if(
#ifndef ONLY_ENABLE_AUDIO
                       (!AWPLAYER_CONFIG_DISABLE_VIDEO && ePlayerMediaType == MEDIA_TYPE_VIDEO) ||
#endif
                       (!AWPLAYER_CONFIG_DISABLE_AUDIO && ePlayerMediaType == MEDIA_TYPE_AUDIO)
#ifndef ONLY_ENABLE_AUDIO
                       ||(!AWPLAYER_CONFIG_DISABLE_SUBTITLE &&ePlayerMediaType == MEDIA_TYPE_SUBTITLE)
#endif
                     )
                    {
                        streamDataInfo.pData        = (char*)packet.buf;
                        streamDataInfo.nLength      = packet.length;
                        streamDataInfo.nPts         = packet.pts;
                        streamDataInfo.nPcr         = packet.pcr;
                        streamDataInfo.bIsFirstPart = (!!(packet.flags & FIRST_PART));
                        streamDataInfo.bIsLastPart  = (!!(packet.flags & LAST_PART));
                        streamDataInfo.nDuration    = packet.duration;
                        streamDataInfo.pStreamInfo  = NULL;
                        streamDataInfo.nStreamChangeFlag = 0;
                        streamDataInfo.nStreamChangeNum = 0;
#ifndef ONLY_ENABLE_AUDIO
                        if(ePlayerMediaType == MEDIA_TYPE_VIDEO &&
                                packet.infoVersion != nVideoInfoVersion)
                        {
                            if(packet.info)
                            {
                                int ret;
                                struct VideoInfo* pVideoinfo = (struct VideoInfo *)packet.info;
                                ret = setVideoStreamDataInfo(&streamDataInfo, pVideoinfo);
                                if (ret < 0)
                                {
                                    loge("setVideoStreamDataInfo fail.");
                                    abort();
                                }
                            }
                            nVideoInfoVersion = packet.infoVersion;

                        }
                        else
#endif
                        if(ePlayerMediaType == MEDIA_TYPE_AUDIO &&
                                packet.infoVersion != nAudioInfoVersion)
                        {
                            if(packet.info)
                            {
                                int ret;
                                struct AudioInfo* pAudioinfo = (struct AudioInfo *)packet.info;
                                ret = setAudioStreamDataInfo(&streamDataInfo, pAudioinfo);
                                if (ret < 0)
                                {
                                    loge("setAudioStreamDataInfo fail.");
                                    abort();
                                }
                            }
                            nAudioInfoVersion = packet.infoVersion;

                        }
                        PlayerSubmitStreamData(demux->pPlayer, &streamDataInfo,
                                            ePlayerMediaType, nStreamIndex);
                    }
                    else
                    {
                        //* skip the media data.
                        free(packet.buf);
                    }

                    //* post a read message to continue the reading job after message processed.
                    PostReadMessage(demux->mq);
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
                        else if (err == PSR_USER_CANCEL) // TODO:
                        {
                            /* do noting */
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
#ifndef ONLY_ENABLE_AUDIO
    int               nVideoInfoVersion = 0;
#endif
    int               nAudioInfoVersion = 0;

    demux = (DemuxCompContext*)arg;
    eCacheStatus = DEMUX_STATUS_STOPPED;
    logd("CacheThread begin");
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
            PostReadMessage(demux->mqCache);
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

            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
                sem_post(pReplySem);

            break;  //* quit the thread.

        } //* end DEMUX_COMMAND_QUIT.
        else if(msg.messageId == DEMUX_COMMAND_SEEK)
        {
            int64_t nSeekTimeUs;
            int64_t nSeekTimeFinal;
            int params[3];

            logd("cache thread process message DEMUX_COMMAND_SEEK.");
            nSeekTimeUs = ((int64_t)msg.params[2])*1000;
            //* TODO
            //* for ts/bd/hls parser, we should map the nSeekTimeUs to pts.
            //* because pts in these parsers may be loopback or may not started with zero value.
            pthread_mutex_lock(&demux->mutex);
            if(demux->bCancelSeek == 0 && demux->bStopping == 0)
            {
                ret = CdxParserClrForceStop(demux->pParser);
                if(ret < 0)
                {
                    logw("CdxParserClrForceStop fail, ret(%d)", ret);
                }
            }
            pthread_mutex_unlock(&demux->mutex);

            logd("CdxParserSeekTo");
            if(CdxParserSeekTo(demux->pParser, nSeekTimeUs, demux->nSeekModeType) >= 0)
            {
                nSeekTimeFinal = nSeekTimeUs;
                ret = 0;
            }
            else
                ret = -1;


            if(ret == 0)
            {
                params[0] = 0;
                params[1] = nSeekTimeUs/1000;
                params[2] = nSeekTimeFinal/1000;
                demux->callback(demux->pUserData, DEMUX_NOTIFY_SEEK_FINISH, (void*)params);

                pthread_mutex_lock(&demux->lock);
                demux->bSeeking = 0;
                pthread_cond_signal(&demux->cond);
                pthread_mutex_unlock(&demux->lock);

                demux->bEOS     = 0;
                demux->bIOError = 0;


                eCacheStatus = DEMUX_STATUS_STARTED;
                PostReadMessage(demux->mqCache);
                //if(demux->eStatus == DEMUX_STATUS_STARTED)
                //    PostReadMessage(demux->mq); //* send read message to start reading loop.

                if(pReplyValue != NULL)
                    *pReplyValue = 0;
            }
            else
            {
                loge("CdxParserSeekTo() return fail, demux->bCancelSeek:%d", demux->bCancelSeek);
                if(demux->bCancelSeek == 1 || demux->bStopping == 1)
                    params[0] = DEMUX_ERROR_USER_CANCEL;
                else
                    params[0] = DEMUX_ERROR_IO;
                demux->eStatus = DEMUX_STATUS_COMPLETE; //* set to complete status to stop reading.

                pthread_mutex_lock(&demux->lock);
                demux->bSeeking = 0;
                pthread_cond_signal(&demux->cond);
                pthread_mutex_unlock(&demux->lock);

                params[1] = nSeekTimeUs/1000;
                params[2] = 0;

                demux->callback(demux->pUserData, DEMUX_NOTIFY_SEEK_FINISH, (void*)params);

                if(pReplyValue != NULL)
                    *pReplyValue = -1;
            }

            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        }
        else if(msg.messageId == DEMUX_COMMAND_RESET_URL)
        {
            int64_t nSeekTimeMs;
            int64_t nSeekTimeFinal;

            logv("cache thread process message DEMUX_COMMAND_RESET_URL.");
            nSeekTimeMs = msg.params[2];

            logd("ShiftedTimeUrl = %s", demux->shiftedTimeUrl);

            struct CallBack cb;
            cb.callback = ParserCallbackProcess;
            cb.pUserData = (void *)demux;

            ContorlTask parserContorlTask, *contorlTask;
            ContorlTask contorlTask1;
            parserContorlTask.cmd = CDX_PSR_CMD_SET_CALLBACK;
            parserContorlTask.param = (void *)&cb;
            parserContorlTask.next = NULL;
            contorlTask = &parserContorlTask;

            if(demux->timeShiftLastSeqNum >= 0)
            {
                contorlTask1.cmd = CDX_PSR_CMD_SET_TIMESHIFT_LAST_SEQNUM;
                contorlTask1.param = (void *)&demux->timeShiftLastSeqNum;
                contorlTask1.next = NULL;
                contorlTask->next = &contorlTask1;
                contorlTask = contorlTask->next;

           }

            CdxDataSourceT    source;
            bzero(&source, sizeof(source));
            source.extraData = NULL;

            demux->statusCode = 0;  // we should reset statusCode to 0 here

reopen:
            source.uri = demux->shiftedTimeUrl;

            int flags = 0;
            if(/*demux->mLivemode == 1 || */demux->mLivemode == 2)
            {
                flags |= CMCC_TIME_SHIFT;
            }

            do {
                if(demux->pParser)
                {
                    CdxParserClose(demux->pParser);
                    demux->pParser = NULL;
                    demux->pStream = NULL;
                }
                else if(demux->pStream)
                {
                    CdxStreamClose(demux->pStream);
                    demux->pStream = NULL;
                }
                ret = CdxParserPrepare(&source, flags, &demux->mutex, &demux->bCancelSeek,
                                &demux->pParser, &demux->pStream,
                                &parserContorlTask, &parserContorlTask);
                if(ret < 0)
                {
                    logd(" resetUrl failed, reconnect,demux->bCancelSeek(%d)", demux->bCancelSeek);
                    if(demux->bCancelSeek)
                    {
                        break;
                    }
                    uintptr_t statusCode = demux->statusCode;
                    logd("+++++ get status code: %zu", statusCode);

                    // if the url is not connectable (http status code 400, 500),
                    // change to livemode 1
                    if(statusCode >= 400)
                    {
                        char *pLiveModeType = NULL;
                        char *pStartime = NULL;
                        //check livemode here
                        pLiveModeType = strstr(demux->shiftedTimeUrl, "livemode=");
                        demux->mLivemode = 1;
                        pLiveModeType[9] = '1';
                        pStartime = strstr(demux->shiftedTimeUrl, "&starttime=");
                        if (pStartime != NULL)
                        {
                            pStartime[0] = '\0';
                        }
                        goto reopen;
                    }

                    usleep(20000);
                }
            } while(ret < 0);

            logd("cache thread: ret = %d", ret);
            if(ret == 0)
            {
                if(pReplyValue != NULL)
                    *pReplyValue = 0;
            }
            else
            {
                if(pReplyValue != NULL)
                {
                    if(demux->bCancelSeek)
                        *pReplyValue = -2;   // user_cancel
                    else
                        *pReplyValue = -1;
                }
            }

            if(pReplySem != NULL)
                sem_post(pReplySem);

            demux->timeShiftLastSeqNum = -1;
            continue;
        }
        else if(msg.messageId == DEMUX_COMMAND_READ)
        {
            if(eCacheStatus != DEMUX_STATUS_STARTED)
                continue;

            logv("cache thread process message DEMUX_COMMAND_READ.");

            if(StreamCacheOverflow(demux->pCache))
            {
                //* wait some time for cache buffer.
                ret = AwMessageQueueTryGetMessage(demux->mqCache, &msg, 200);
                if(ret == 0)    //* new message come, quit loop to process.
                    goto cache_process_message;

                //* post a read message to continue the reading job after message processed.
                PostReadMessage(demux->mqCache);
            }
            else
            {
                //**************************************************************
                //* read data directly from parser.
                //**************************************************************

                CdxPacketT packet;
                CacheNode  node;
                memset(&packet, 0x00, sizeof(CdxPacketT));

                //* 1. get data type.
                if(CdxParserPrefetch(demux->pParser, &packet) != 0)
                {
                    logw("prefetch fail.");
                    if(demux->bStopping == 0 && demux->bSeeking == 0)
                    {
                        int err = CdxParserGetStatus(demux->pParser);
                        logd("err = %d", err);
                        if(err == PSR_IO_ERR)
                        {
                            demux->bIOError = 1;
                        }
                        else if (err == PSR_USER_CANCEL) // TODO:
                        {
                            /* do noting */
                        }
                        else
                        {
                            demux->bEOS = 1;
                            NotifyCacheState(demux);
                        }
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
                    node.nPcr         = packet.pcr;
                    node.bIsFirstPart = 1;
                    node.bIsLastPart  = 1;
                    node.nDuration    = packet.duration;
                    node.info          = NULL;
                    node.infoVersion  = -1;
#ifndef ONLY_ENABLE_AUDIO
                    if(packet.type == CDX_MEDIA_VIDEO)
                    {
                        if(packet.infoVersion != nVideoInfoVersion && packet.info)
                        {
                            logd("cache -- video stream info change: "
                                    "preInfoVersion = %d, curInfoVersion = %d",
                                    nVideoInfoVersion, packet.infoVersion);
                            int ret;
                            ret = setCacheNodeVideo(&packet, &node);
                            if(ret < 0)
                            {
                                loge("setCacheNodeVideo fail.");
                                abort();
                            }
                        }
                        nVideoInfoVersion = packet.infoVersion;
                    }
                    else
#endif
                    if(packet.type == CDX_MEDIA_AUDIO)
                    {
                        if(packet.infoVersion != nAudioInfoVersion && packet.info)
                        {
                            logd("cache -- audio stream info change: "
                                    "preInfoVersion = %d, curInfoVersion = %d",
                                    nAudioInfoVersion, packet.infoVersion);

                            int ret;
                            ret = setCacheNodeAudio(&packet, &node);
                            if(ret < 0)
                            {
                                loge("setCacheNodeVideo fail.");
                                abort();
                            }
                        }
                        nAudioInfoVersion = packet.infoVersion;
                    }

                    StreamCacheAddOneFrame(demux->pCache, &node);

                    //* post a read message to continue the reading job after message processed.
                    PostReadMessage(demux->mqCache);
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
                        {
                            demux->bIOError = 1;
                        }
                        else if (err == PSR_USER_CANCEL) // TODO:
                        {
                            /* do noting */
                        }
                        else
                        {
                            demux->bEOS = 1;
                        }
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
    else if (source->extraDataType == EXTRA_DATA_RTSP &&
            source->extraData != NULL)
    {
        CdxKeyedVectorDestroy((CdxKeyedVectorT *)source->extraData);
        source->extraData = NULL;
        source->extraDataType = EXTRA_DATA_UNKNOWN;
    }

    return;
}

static int setDataSourceFields(DemuxCompContext *demuxDhr, CdxDataSourceT* source,
            void* pHTTPServer, char* uri, CdxKeyedVectorT* pHeaders)
{
    CdxHttpHeaderFieldsT* pHttpHeaders;
    int                   i;
    int                   nHeaderSize;

    clearDataSourceFields(source);

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

    source->pHTTPServer = pHTTPServer;

    if(pHeaders != NULL && (!strncasecmp("http://", uri, 7)
        || !strncasecmp("https://", uri, 8)))
    {
        const char* key;
        const char* value;

        //i = pHeaders->indexOfKey(String8("x-hide-urls-from-log"));
        //if(i >= 0)
        //    pHeaders->removeItemsAt(i);

        nHeaderSize = pHeaders->size;
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

            pHttpHeaders->pHttpHeader =
                (CdxHttpHeaderFieldT*)malloc(sizeof(CdxHttpHeaderFieldT)*nHeaderSize);
            if(pHttpHeaders->pHttpHeader == NULL)
            {
                loge("can not malloc memory for http header.");
                free(pHttpHeaders);
                clearDataSourceFields(source);
                return -1;
            }

            source->extraData = (void*)pHttpHeaders;
            source->extraDataType = EXTRA_DATA_HTTP_HEADER;

            for(i=0; i<nHeaderSize; i++)
            {
                key   = pHeaders->item[i].key;
                value = pHeaders->item[i].val;
                if(key != NULL)
                {
                    pHttpHeaders->pHttpHeader[i].key = (const char*)strdup(key);
                    if(pHttpHeaders->pHttpHeader[i].key == NULL)
                    {
                        loge("can not dump string of http header.");
                        clearDataSourceFields(source);
                        return -1;
                    }
                }
                else
                    pHttpHeaders->pHttpHeader[i].key = NULL;

                if(value != NULL)
                {
                    pHttpHeaders->pHttpHeader[i].val = (const char*)strdup(value);
                    if(pHttpHeaders->pHttpHeader[i].val == NULL)
                    {
                        loge("can not dump string of http header.");
                        clearDataSourceFields(source);
                        return -1;
                    }
                }
                else
                    pHttpHeaders->pHttpHeader[i].val = NULL;
            }
        }
    }
    else if (pHeaders != NULL && strncasecmp("rtsp://", uri, 7) == 0)
    {
        /* This block should be removed after cleaning up the code above to use
         * CdxKeyedVectorT
         */
        nHeaderSize = pHeaders->size;
        if (nHeaderSize <= 0)
            return 0;

        CdxKeyedVectorT *rtspHeader = CdxKeyedVectorCreate(nHeaderSize);
        if (rtspHeader == NULL)
        {
            logw("CdxKeyedVectorCreate() failed");
            clearDataSourceFields(source);
            return -1;
        }

        source->extraData = (void *)rtspHeader;
        source->extraDataType = EXTRA_DATA_RTSP;

        char* key;
        char* value;
        for (i = 0; i < nHeaderSize; i++)
        {
            key = pHeaders->item[i].key;
            value = pHeaders->item[i].val;
            if (CdxKeyedVectorAdd(rtspHeader, key, value) != 0)
            {
                clearDataSourceFields(source);
                return -1;
            }
        }
    }
    else if (strncasecmp("bdmv://", uri, 7) == 0)
    {
        logw("trace...");
        static struct BdmvExtraDataS bdmvED;
        bdmvED.ioCb = &demuxIoOps;
        bdmvED.cbHdr = (void *)demuxDhr;
        source->extraData = &bdmvED;
        source->extraDataType = EXTRA_DATA_BDMV;
    }

    return 0;
}


static int setMediaInfo(MediaInfo* pMediaInfo, CdxMediaInfoT* pInfoFromParser)
{
    int                 i;
    int                 nStreamCount;
#ifndef ONLY_ENABLE_AUDIO
    VideoStreamInfo*    pVideoStreamInfo = NULL;
#endif
    AudioStreamInfo*    pAudioStreamInfo = NULL;
#ifndef ONLY_ENABLE_AUDIO
    SubtitleStreamInfo* pSubtitleStreamInfo = NULL;
#endif
    int                 nCodecSpecificDataLen;
    char*               pCodecSpecificData = NULL;

    clearMediaInfo(pMediaInfo);

    pMediaInfo->nDurationMs = pInfoFromParser->program[0].duration;
    pMediaInfo->nFileSize   = pInfoFromParser->fileSize;
    pMediaInfo->nFirstPts   = pInfoFromParser->program[0].firstPts;
    if(pInfoFromParser->bitrate > 0)
        pMediaInfo->nBitrate = pInfoFromParser->bitrate;
    else if(pInfoFromParser->fileSize > 0 && pInfoFromParser->program[0].duration > 0)
        pMediaInfo->nBitrate =
            (int)((pInfoFromParser->fileSize*8*1000)/pInfoFromParser->program[0].duration);
    else
        pMediaInfo->nBitrate = 0;
    pMediaInfo->bSeekable   = pInfoFromParser->bSeekable;

    memcpy(pMediaInfo->cRotation,pInfoFromParser->rotate,4*sizeof(unsigned char));
#ifndef ONLY_ENABLE_AUDIO
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

            if(pInfoFromParser->program[0].flags & kUseSecureInputBuffers)
                pVideoStreamInfo->bSecureStreamFlag = 1;

            logv("the %dth video stream info.", i);
            logv("    codec: %d.", pVideoStreamInfo->eCodecFormat);
            logv("    width: %d.", pVideoStreamInfo->nWidth);
            logv("    height: %d.", pVideoStreamInfo->nHeight);
            logv("    frame rate: %d.", pVideoStreamInfo->nFrameRate);
            logv("    aspect ratio: %d.", pVideoStreamInfo->nAspectRatio);
            logv("    is 3D: %s.", pVideoStreamInfo->bIs3DStream ? "true" : "false");
            logv("    codec specific data size: %d.", pVideoStreamInfo->nCodecSpecificDataLen);
            logv("    bSecureStreamFlag : %d",pVideoStreamInfo->bSecureStreamFlag);
        }

        pMediaInfo->nVideoStreamNum = nStreamCount;
    }
#endif
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

#ifndef ONLY_ENABLE_AUDIO
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
#endif
    return 0;
}


static void clearMediaInfo(MediaInfo* pMediaInfo)
{
    int                 i;
#ifndef ONLY_ENABLE_AUDIO
    VideoStreamInfo*    pVideoStreamInfo;
#endif
    AudioStreamInfo*    pAudioStreamInfo;
#ifndef ONLY_ENABLE_AUDIO
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
#endif

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

#ifndef ONLY_ENABLE_AUDIO
    if(pMediaInfo->nSubtitleStreamNum > 0)
    {
        free(pMediaInfo->pSubtitleStreamInfo);
        pMediaInfo->pSubtitleStreamInfo = NULL;
        pMediaInfo->nSubtitleStreamNum = 0;
    }
#endif
    pMediaInfo->nFileSize      = 0;
    pMediaInfo->nDurationMs    = 0;
    pMediaInfo->eContainerType = CONTAINER_TYPE_UNKNOWN;
    pMediaInfo->bSeekable      = 0;

    return;
}


static int PlayerBufferOverflow(Player* p)
{
#ifndef ONLY_ENABLE_AUDIO
    int bVideoOverflow;
#endif
    int bAudioOverflow;
#ifndef ONLY_ENABLE_AUDIO
    int     nPictureNum;
    int     nFrameDuration;
#endif
    int     nPcmDataSize;
    int     nSampleRate;
    int     nChannelCount;
    int     nBitsPerSample;
    int     nStreamDataSize;
    int     nStreamBufferSize;
    int     nBitrate;
#ifndef ONLY_ENABLE_AUDIO
    int64_t nVideoCacheTime;
#endif
    int64_t nAudioCacheTime;
#ifndef ONLY_ENABLE_AUDIO
    bVideoOverflow = 1;
#endif
    bAudioOverflow = 1;

#ifndef ONLY_ENABLE_AUDIO
    if(PlayerHasVideo(p))
    {
        nPictureNum     = PlayerGetValidPictureNum(p);
        nFrameDuration  = PlayerGetVideoFrameDuration(p);
        nStreamDataSize = PlayerGetVideoStreamDataSize(p);
        nStreamBufferSize = PlayerGetVideoStreamBufferSize(p);
        nBitrate        = PlayerGetVideoBitrate(p);

        nVideoCacheTime = nPictureNum*nFrameDuration;

        if(nBitrate > 0)
            nVideoCacheTime += ((int64_t)nStreamDataSize)*8*1000*1000/nBitrate;

        //* cache more than 2 seconds of data.
        if( nVideoCacheTime <= 2000000 || nStreamDataSize<nStreamBufferSize/2 )
            bVideoOverflow = 0;

        logv("picNum = %d, frameDuration = %d, dataSize = %d, \
            bitrate = %d, bVideoOverflow = %d ,nStreamDataSize = %d ,nStreamBufferSize = %d",
            nPictureNum, nFrameDuration, nStreamDataSize, nBitrate,
            bVideoOverflow,nStreamDataSize,nStreamBufferSize);
    }
#endif

    if(PlayerHasAudio(p))
    {
        nPcmDataSize    = PlayerGetAudioPcmDataSize(p);
        nStreamDataSize = PlayerGetAudioStreamDataSize(p);
        nBitrate        = PlayerGetAudioBitrate(p);
        PlayerGetAudioParam(p, &nSampleRate, &nChannelCount, &nBitsPerSample);

        nAudioCacheTime = 0;

        if(nSampleRate != 0 && nChannelCount != 0 && nBitsPerSample != 0)
        {
            nAudioCacheTime += ((int64_t)nPcmDataSize)*8*1000*1000/
                    (nSampleRate*nChannelCount*nBitsPerSample);
        }

        if(nBitrate > 0)
            nAudioCacheTime += ((int64_t)nStreamDataSize)*8*1000*1000/nBitrate;

        if(nAudioCacheTime <= 3000000)   //* cache more than 2 seconds of data.
            bAudioOverflow = 0;

        logv("nPcmDataSize = %d, nStreamDataSize = %d,   \
            nBitrate = %d, nAudioCacheTime = %lld, bAudioOverflow = %d",
            nPcmDataSize, nStreamDataSize, nBitrate, nAudioCacheTime, bAudioOverflow);
    }
#ifndef ONLY_ENABLE_AUDIO
    return bVideoOverflow && bAudioOverflow;
#else
    return bAudioOverflow;
#endif
}


static int PlayerBufferUnderflow(Player* p)
{
#ifndef ONLY_ENABLE_AUDIO
    int bVideoUnderflow;
#endif
    int bAudioUnderFlow;
#ifndef ONLY_ENABLE_AUDIO
    bVideoUnderflow = 0;
#endif
    bAudioUnderFlow = 0;
#ifndef ONLY_ENABLE_AUDIO
    if(PlayerHasVideo(p))
    {
        int nPictureNum;
        int nStreamFrameNum;

        nPictureNum = PlayerGetValidPictureNum(p);
        nStreamFrameNum = PlayerGetVideoStreamFrameNum(p);
        if(nPictureNum + nStreamFrameNum < 15)
            bVideoUnderflow = 1;

        logv("nPictureNum = %d, nStreamFrameNum = %d, bVideoUnderflow = %d",
            nPictureNum, nStreamFrameNum, bVideoUnderflow);
    }
#endif
    if(PlayerHasAudio(p))
    {
        int nStreamDataSize;
        int nPcmDataSize;
        int nCacheTime;

        nStreamDataSize = PlayerGetAudioStreamDataSize(p);
        nPcmDataSize    = PlayerGetAudioPcmDataSize(p);
        nCacheTime      = 0;
        if(nCacheTime == 0 && (nPcmDataSize + nStreamDataSize < 8000))
            bAudioUnderFlow = 1;

        logv("nStreamDataSize = %d, nPcmDataSize = %d, nCacheTime = %d, bAudioUnderFlow = %d",
            nStreamDataSize, nPcmDataSize, nCacheTime, bAudioUnderFlow);
    }
#ifndef ONLY_ENABLE_AUDIO
    return bVideoUnderflow | bAudioUnderFlow;
#else
    return bAudioUnderFlow;
#endif

}


static int GetCacheState(DemuxCompContext* demux)
{
    if(CdxParserControl(demux->pParser, CDX_PSR_CMD_GET_CACHESTATE, &demux->sCacheState) == 0)
        return 0;
    else
        return -1;
}


static int64_t GetSysTime()
{
    int64_t time;
    struct timeval t;
    gettimeofday(&t, NULL);
    time = (int64_t)t.tv_sec * 1000000;
    time += t.tv_usec;
    return time;
}


static void NotifyCacheState(DemuxCompContext* demux)
{
    int nTotalPercentage;
    int nBufferPercentage;
    int nLoadingPercentage;
    int param[3];

    GetCacheState(demux);

    param[0] = nTotalPercentage   = demux->sCacheState.nPercentage;
    param[1] = nBufferPercentage  = StreamCacheGetBufferFullness(demux->pCache);
    param[2] = nLoadingPercentage = StreamCacheGetLoadingProgress(demux->pCache);

    logv("notify cache state, total percent = %d%%,  \
        buffer fullness = %d%%, loading progress = %d%%",
        nTotalPercentage, nBufferPercentage, nLoadingPercentage);
    demux->callback(demux->pUserData, DEMUX_NOTIFY_CACHE_STAT, (void*)param);
    return;
}


static void PostReadMessage(AwMessageQueue* mq)
{
    if(AwMessageQueueGetCount(mq)<=0)
    {
        AwMessage msg;
        memset(&msg, 0, sizeof(AwMessage));
        msg.messageId = DEMUX_COMMAND_READ;
        AwMessageQueuePostMessage(mq, &msg);
    }
    return;
}

static int ShiftTimeMode(int Shiftedms, char *buf)
{
    struct tm *p;
    time_t timep;
    time(&timep);

    //just get seconds shifted, at least 30s for shiftedtime function
    if (Shiftedms > 30 * 1000)
    {
        timep -= Shiftedms / 1000;
    }
    else
    {
        return 1;
    }

    //p = localtime(&timep);
    timep += 8*3600;
    p = gmtime(&timep);


    sprintf(buf, "%d%02d%02dT%02d%02d%02d.00Z",
        1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    logd("shifted formatTime str->[%s]\n", buf);

    return 2;
}
