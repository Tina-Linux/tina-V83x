
#include "awplayer.h"
#include "log.h"
#include <string.h>
#include "demoConfig.h"

//* player status.
static const int AWPLAYER_STATUS_IDLE        = 0;
static const int AWPLAYER_STATUS_INITIALIZED = 1<<0;
static const int AWPLAYER_STATUS_PREPARING   = 1<<1;
static const int AWPLAYER_STATUS_PREPARED    = 1<<2;
static const int AWPLAYER_STATUS_STARTED     = 1<<3;
static const int AWPLAYER_STATUS_PAUSED      = 1<<4;
static const int AWPLAYER_STATUS_STOPPED     = 1<<5;
static const int AWPLAYER_STATUS_COMPLETE    = 1<<6;
static const int AWPLAYER_STATUS_ERROR       = 1<<7;

//* callback message id.
static const int AWPLAYER_MESSAGE_DEMUX_PREPARED            = 0x101;
static const int AWPLAYER_MESSAGE_DEMUX_EOS                 = 0x102;
static const int AWPLAYER_MESSAGE_DEMUX_IOERROR             = 0x103;
static const int AWPLAYER_MESSAGE_DEMUX_SEEK_FINISH         = 0x104;
static const int AWPLAYER_MESSAGE_DEMUX_CACHE_REPORT        = 0x105;
static const int AWPLAYER_MESSAGE_DEMUX_BUFFER_START        = 0x106;
static const int AWPLAYER_MESSAGE_DEMUX_BUFFER_END          = 0x107;
static const int AWPLAYER_MESSAGE_PLAYER_EOS                = 0x201;
static const int AWPLAYER_MESSAGE_PLAYER_FIRST_PICTURE      = 0x202;
static const int AWPLAYER_MESSAGE_PLAYER_SUBTITLE_AVAILABLE = 0x203;
static const int AWPLAYER_MESSAGE_PLAYER_SUBTITLE_EXPIRED   = 0x204;

//* command id.
static const int AWPLAYER_COMMAND_SET_SOURCE    = 0x101;
static const int AWPLAYER_COMMAND_SET_SURFACE   = 0x102;
static const int AWPLAYER_COMMAND_SET_AUDIOSINK = 0x103;
static const int AWPLAYER_COMMAND_PREPARE       = 0x104;
static const int AWPLAYER_COMMAND_START         = 0x105;
static const int AWPLAYER_COMMAND_STOP          = 0x106;
static const int AWPLAYER_COMMAND_PAUSE         = 0x107;
static const int AWPLAYER_COMMAND_RESET         = 0x108;
static const int AWPLAYER_COMMAND_QUIT          = 0x109;
static const int AWPLAYER_COMMAND_SEEK          = 0x10a;

static void* AwPlayerThread(void* arg);
static int DemuxCallbackProcess(void* pUserData, int eMessageId, void* param);
static int PlayerCallbackProcess(void* pUserData, int eMessageId, void* param);

AwPlayer::AwPlayer()
{
    logv("awplayer construct.");

    mSourceUrl      = NULL;
    mStatus         = AWPLAYER_STATUS_IDLE;
    mSeeking        = 0;
    mSeekSync       = 0;
    mLoop           = 0;
    mMediaInfo      = NULL;
    mMessageQueue   = NULL;

    pthread_mutex_init(&mMutex, NULL);
    sem_init(&mSemSetDataSource, 0, 0);
    sem_init(&mSemPrepare, 0, 0);
    sem_init(&mSemStart, 0, 0);
    sem_init(&mSemStop, 0, 0);
    sem_init(&mSemPause, 0, 0);
    sem_init(&mSemReset, 0, 0);
    sem_init(&mSemQuit, 0, 0);
    sem_init(&mSemSeek, 0, 0);

    sem_init(&mSemPrepareFinish, 0, 0); //* for signal prepare finish, used in prepare().

    mMessageQueue = AwMessageQueueCreate(64);
    mPlayer       = PlayerCreate();
    mDemux        = DemuxCompCreate();

    if(mPlayer != NULL)
        PlayerSetCallback(mPlayer, PlayerCallbackProcess, (void*)this);

    if(mDemux != NULL)
    {
        DemuxCompSetCallback(mDemux, DemuxCallbackProcess, (void*)this);
        DemuxCompSetPlayer(mDemux, mPlayer);
    }

    if(pthread_create(&mThreadId, NULL, AwPlayerThread, this) == 0)
        mThreadCreated = 1;
    else
        mThreadCreated = 0;
}


AwPlayer::~AwPlayer()
{
    AwMessage msg;
    logv("~AwPlayer");

    if(mThreadCreated)
    {
        void* status;

        reset();    //* stop demux and player.

        //* send a quit message to quit the main thread.
        setMessage(&msg, AWPLAYER_COMMAND_QUIT, (uintptr_t)&mSemQuit);
        AwMessageQueuePostMessage(mMessageQueue, &msg);
        SemTimedWait(&mSemQuit, -1);
        pthread_join(mThreadId, &status);
    }

    if(mDemux != NULL)
        DemuxCompDestroy(mDemux);

    if(mPlayer != NULL)
        PlayerDestroy(mPlayer);

    if(mMessageQueue != NULL)
        AwMessageQueueDestroy(mMessageQueue);

    pthread_mutex_destroy(&mMutex);
    sem_destroy(&mSemSetDataSource);
    sem_destroy(&mSemPrepare);
    sem_destroy(&mSemStart);
    sem_destroy(&mSemStop);
    sem_destroy(&mSemPause);
    sem_destroy(&mSemReset);
    sem_destroy(&mSemQuit);
    sem_destroy(&mSemSeek);
    sem_destroy(&mSemPrepareFinish);

    if(mMediaInfo != NULL)
        clearMediaInfo();

    if(mSourceUrl != NULL)
        free(mSourceUrl);
}


int AwPlayer::initCheck()
{
    logv("initCheck");

    if(mPlayer == NULL || mDemux == NULL || mThreadCreated == 0 || mNotifier == NULL)
    {
        loge("initCheck() fail, AwPlayer::mplayer = %p, AwPlayer::mDemux = %p", mPlayer, mDemux);
        return -1;
    }
    else
        return 0;
}


int AwPlayer::setNotifyCallback(NotifyCallback notifier, void* pUserData)
{
    mNotifier = notifier;
    mUserData = pUserData;
    return 0;
}


#if CONFIG_OS == OPTION_OS_ANDROID
status_t AwPlayer::setDataSource(const char* pUrl, const KeyedVector<String8, String8>* pHeaders)
#else
int AwPlayer::setDataSource(const char* pUrl, const map<string, string>* pHeaders)
#endif
{
    AwMessage msg;

    if(pUrl == NULL)
    {
        loge("setDataSource(url), url=NULL");
        return -1;
    }

    logv("setDataSource(url), url=%s", pUrl);

    //* send a set data source message.
    setMessage(&msg,
               AWPLAYER_COMMAND_SET_SOURCE,             //* message id.
               (uintptr_t)&mSemSetDataSource,        //* params[0] = &mSemSetDataSource.
               (uintptr_t)&mSetDataSourceReply,      //* params[1] = &mSetDataSourceReply.
               SOURCE_TYPE_URL,                         //* params[2] = SOURCE_TYPE_URL.
               (uintptr_t)pUrl,                      //* params[3] = pUrl.
               (uintptr_t)pHeaders);                 //* params[4] = KeyedVector<String8, String8>* pHeaders;
    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemSetDataSource, -1);

    return mSetDataSourceReply;
}


int AwPlayer::prepareAsync()
{
    AwMessage msg;

    logv("prepareAsync");

    //* send a prepare.
    setMessage(&msg,
               AWPLAYER_COMMAND_PREPARE,        //* message id.
               (uintptr_t)&mSemPrepare,      //* params[0] = &mSemPrepare.
               (uintptr_t)&mPrepareReply,    //* params[1] = &mPrepareReply.
               0);                              //* params[2] = mPrepareSync.
    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemPrepare, -1);
    return mPrepareReply;
}


int AwPlayer::prepare()
{
    AwMessage msg;

    logv("prepare");

    //* clear the mSemPrepareFinish semaphore.
    while(sem_trywait(&mSemPrepareFinish) == 0);

    //* send a prepare message.
    setMessage(&msg,
               AWPLAYER_COMMAND_PREPARE,        //* message id.
               (uintptr_t)&mSemPrepare,      //* params[0] = &mSemPrepare.
               (uintptr_t)&mPrepareReply,    //* params[1] = &mPrepareReply.
               1);                              //* params[2] = mPrepareSync.
    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemPrepare, -1);

    if(mPrepareReply == 0)
    {
        //* wait for the prepare finish.
        SemTimedWait(&mSemPrepareFinish, -1);
        return mPrepareFinishResult;
    }
    else
        return mPrepareReply; //* call DemuxCompPrepareAsync() fail, or status error.
}


int AwPlayer::start()
{
    AwMessage msg;

    logv("start");

    //* send a start message.
    setMessage(&msg,
               AWPLAYER_COMMAND_START,        //* message id.
               (uintptr_t)&mSemStart,      //* params[0] = &mSemStart.
               (uintptr_t)&mStartReply);   //* params[1] = &mStartReply.
    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemStart, -1);
    return mStartReply;
}


int AwPlayer::stop()
{
    AwMessage msg;

    logv("stop");

    //* send a stop message.
    setMessage(&msg,
               AWPLAYER_COMMAND_STOP,        //* message id.
               (uintptr_t)&mSemStop,      //* params[0] = &mSemStop.
               (uintptr_t)&mStopReply);   //* params[1] = &mStopReply.
    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemStop, -1);
    return mStopReply;
}


int AwPlayer::pause()
{
    AwMessage msg;

    logv("pause");

    //* send a pause message.
    setMessage(&msg,
               AWPLAYER_COMMAND_PAUSE,        //* message id.
               (uintptr_t)&mSemPause,      //* params[0] = &mSemPause.
               (uintptr_t)&mPauseReply);   //* params[1] = &mPauseReply.
    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemPause, -1);
    return mPauseReply;
}


int AwPlayer::seekTo(int msec)
{
    AwMessage msg;

    logv("seekTo");

    //* send a start message.
    setMessage(&msg,
               AWPLAYER_COMMAND_SEEK,        //* message id.
               (uintptr_t)&mSemSeek,      //* params[0] = &mSemSeek.
               (uintptr_t)&mSeekReply,    //* params[1] = &mSeekReply.
               msec,                         //* params[2] = mSeekTime.
               0);                           //* params[3] = mSeekSync.
    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemSeek, -1);
    return mSeekReply;
}


int AwPlayer::reset()
{
    AwMessage msg;

    logv("reset");

    //* send a start message.
    setMessage(&msg,
               AWPLAYER_COMMAND_RESET,       //* message id.
               (uintptr_t)&mSemReset,     //* params[0] = &mSemReset.
               (uintptr_t)&mResetReply);  //* params[1] = &mResetReply.
    AwMessageQueuePostMessage(mMessageQueue, &msg);
    SemTimedWait(&mSemReset, -1);
    return mResetReply;
}


int AwPlayer::isPlaying()
{
    logv("isPlaying");
    if(mStatus == AWPLAYER_STATUS_STARTED || mStatus == AWPLAYER_STATUS_COMPLETE)
        return 1;
    else
        return 0;
}


int AwPlayer::getCurrentPosition(int* msec)
{
    int64_t nPositionUs;

    logv("getCurrentPosition");

    if(mStatus == AWPLAYER_STATUS_PREPARED ||
       mStatus == AWPLAYER_STATUS_STARTED  ||
       mStatus == AWPLAYER_STATUS_PAUSED   ||
       mStatus == AWPLAYER_STATUS_COMPLETE)
    {
        if(mSeeking != 0)
        {
            *msec = mSeekTime;
            return 0;
        }

        pthread_mutex_lock(&mMutex);    //* in complete status, the prepare() method maybe called
        if(mMediaInfo != NULL)
        {
            if(mMediaInfo->eContainerType == CONTAINER_TYPE_TS || mMediaInfo->eContainerType == CONTAINER_TYPE_BD)
                nPositionUs = PlayerGetPosition(mPlayer); //* ts stream's pts is not started at 0.
            else
                nPositionUs = PlayerGetPts(mPlayer);      //* generally, stream pts is started at 0 except ts stream.
            *msec = (nPositionUs + 500)/1000;
            pthread_mutex_unlock(&mMutex);
            return 0;
        }
        else
        {
            loge("getCurrentPosition() fail, mMediaInfo==NULL.");
            *msec = 0;
            pthread_mutex_unlock(&mMutex);
            return 0;
        }
    }
    else
    {
        *msec = 0;
        if(mStatus == AWPLAYER_STATUS_ERROR)
            return -1;
        else
            return 0;
    }
}


int AwPlayer::getDuration(int *msec)
{
    logv("getDuration");

    if(mStatus == AWPLAYER_STATUS_PREPARED ||
       mStatus == AWPLAYER_STATUS_STARTED  ||
       mStatus == AWPLAYER_STATUS_PAUSED   ||
       mStatus == AWPLAYER_STATUS_STOPPED  ||
       mStatus == AWPLAYER_STATUS_COMPLETE)
    {
        pthread_mutex_lock(&mMutex);    //* in complete status, the prepare() method maybe called and
        if(mMediaInfo != NULL)
            *msec = mMediaInfo->nDurationMs;
        else
        {
            loge("getCurrentPosition() fail, mMediaInfo==NULL.");
            *msec = 0;
        }
        pthread_mutex_unlock(&mMutex);
        return 0;
    }
    else
    {
        loge("invalid getDuration() call, player not in valid status.");
        return -1;
    }
}


int AwPlayer::setLooping(int loop)
{
    logv("setLooping");

    if(mStatus == AWPLAYER_STATUS_ERROR)
        return -1;

    mLoop = loop;
    return 0;
}


int AwPlayer::initializePlayer()
{
    //* get media information.
    MediaInfo*          mi;
    int                 i;
    int                 nDefaultAudioIndex;
    int                 nDefaultSubtitleIndex;
    int                 ret;

    pthread_mutex_lock(&mMutex);

    mi = DemuxCompGetMediaInfo(mDemux);
    if(mi == NULL)
    {
        loge("can not get media info from demux.");
        pthread_mutex_unlock(&mMutex);
        return -1;
    }

    mMediaInfo = mi;

    //* initialize the player.
#if !DEMO_CONFIG_DISABLE_VIDEO
    if(mi->pVideoStreamInfo != NULL)
    {
        ret = PlayerSetVideoStreamInfo(mPlayer, mi->pVideoStreamInfo);
        if(ret != 0)
        {
            logw("PlayerSetVideoStreamInfo() fail, video stream not supported.");
        }
    }
#endif

#if !DEMO_CONFIG_DISABLE_AUDIO
    if(mi->pAudioStreamInfo != NULL)
    {
        nDefaultAudioIndex = -1;
        for(i=0; i<mi->nAudioStreamNum; i++)
        {
            if(PlayerCanSupportAudioStream(mPlayer, &mi->pAudioStreamInfo[i]))
            {
                nDefaultAudioIndex = i;
                break;
            }
        }

        if(nDefaultAudioIndex < 0)
        {
            logw("no audio stream supported.");
            nDefaultAudioIndex = 0;
        }

        ret = PlayerSetAudioStreamInfo(mPlayer, mi->pAudioStreamInfo, mi->nAudioStreamNum, nDefaultAudioIndex);
        if(ret != 0)
        {
            logw("PlayerSetAudioStreamInfo() fail, audio stream not supported.");
        }
    }
#endif

    if(PlayerHasVideo(mPlayer) == 0 && PlayerHasAudio(mPlayer) == 0)
    {
        loge("neither video nor audio stream can be played.");
        pthread_mutex_unlock(&mMutex);
        return -1;
    }

#if !DEMO_CONFIG_DISABLE_SUBTITLE
    //* set subtitle stream to the text decoder.
    if(mi->pSubtitleStreamInfo != NULL)
    {
        nDefaultSubtitleIndex = -1;
        for(i=0; i<mi->nSubtitleStreamNum; i++)
        {
            if(PlayerCanSupportSubtitleStream(mPlayer, &mi->pSubtitleStreamInfo[i]))
            {
                nDefaultSubtitleIndex = i;
                break;
            }
        }

        if(nDefaultSubtitleIndex < 0)
        {
            logw("no subtitle stream supported.");
            nDefaultSubtitleIndex = 0;
        }

        ret = PlayerSetSubtitleStreamInfo(mPlayer, mi->pSubtitleStreamInfo, mi->nSubtitleStreamNum, nDefaultSubtitleIndex);
        if(ret != 0)
        {
            logw("PlayerSetSubtitleStreamInfo() fail, subtitle stream not supported.");
        }
    }
#else
    (void)nDefaultSubtitleIndex;
#endif

    //* report not seekable.
    if(mi->bSeekable == 0)
    {
        if(mNotifier != NULL)
            mNotifier(mUserData, NOTIFY_NOT_SEEKABLE, 0, NULL);
    }

#if CONFIG_OS == OPTION_OS_LINUX
    //* on linux, outside program do not set a video layer,
    //* here we let the player create a video layer by itself.
#if !DEMO_CONFIG_DISABLE_VIDEO
    PlayerSetWindow(mPlayer, NULL);
#endif
#endif

    pthread_mutex_unlock(&mMutex);
    return 0;
}


void AwPlayer::clearMediaInfo()
{
    int                 i;
    VideoStreamInfo*    v;
    AudioStreamInfo*    a;
    SubtitleStreamInfo* s;

    if(mMediaInfo != NULL)
    {
        //* free video stream info.
        if(mMediaInfo->pVideoStreamInfo != NULL)
        {
            for(i=0; i<mMediaInfo->nVideoStreamNum; i++)
            {
                v = &mMediaInfo->pVideoStreamInfo[i];
                if(v->pCodecSpecificData != NULL && v->nCodecSpecificDataLen > 0)
                    free(v->pCodecSpecificData);
            }
            free(mMediaInfo->pVideoStreamInfo);
            mMediaInfo->pVideoStreamInfo = NULL;
        }

        //* free audio stream info.
        if(mMediaInfo->pAudioStreamInfo != NULL)
        {
            for(i=0; i<mMediaInfo->nAudioStreamNum; i++)
            {
                a = &mMediaInfo->pAudioStreamInfo[i];
                if(a->pCodecSpecificData != NULL && a->nCodecSpecificDataLen > 0)
                    free(a->pCodecSpecificData);
            }
            free(mMediaInfo->pAudioStreamInfo);
            mMediaInfo->pAudioStreamInfo = NULL;
        }

        //* free subtitle stream info.
        if(mMediaInfo->pSubtitleStreamInfo != NULL)
        {
            for(i=0; i<mMediaInfo->nSubtitleStreamNum; i++)
            {
                s = &mMediaInfo->pSubtitleStreamInfo[i];
                if(s->pUrl != NULL)
                {
                    free(s->pUrl);
                    s->pUrl = NULL;
                }
                if(s->fd >= 0)
                {
                    ::close(s->fd);
                    s->fd = -1;
                }
                if(s->fdSub >= 0)
                {
                    ::close(s->fdSub);
                    s->fdSub = -1;
                }
            }
            free(mMediaInfo->pSubtitleStreamInfo);
            mMediaInfo->pSubtitleStreamInfo = NULL;
        }

        //* free the media info.
        free(mMediaInfo);
        mMediaInfo = NULL;
    }

    return;
}


int AwPlayer::mainThread()
{
    AwMessage            msg;
    int                  ret;
    sem_t*               pReplySem;
    int*                 pReplyValue;

    while(1)
    {
        if(AwMessageQueueGetMessage(mMessageQueue, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }

        pReplySem   = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == AWPLAYER_COMMAND_SET_SOURCE)
        {
            logv("process message AWPLAYER_COMMAND_SET_SOURCE.");
            //* check status.
            if(mStatus != AWPLAYER_STATUS_IDLE && mStatus != AWPLAYER_STATUS_INITIALIZED)
            {
                loge("invalid setDataSource() operation, player not in IDLE or INITIALIZED status");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                if(pReplySem != NULL)
		            sem_post(pReplySem);
		        continue;
            }

            if((int)msg.params[2] == SOURCE_TYPE_URL)
            {
#if CONFIG_OS == OPTION_OS_ANDROID
                KeyedVector<String8, String8>* pHeaders;
#else
                map<string, string>* pHeaders;
#endif
                //* data source is a url string.
                if(mSourceUrl != NULL)
                    free(mSourceUrl);
                mSourceUrl = strdup((char*)msg.params[3]);
#if CONFIG_OS == OPTION_OS_ANDROID
                pHeaders   = (KeyedVector<String8, String8>*) msg.params[4];
#else
                pHeaders   = (map<string, string>*) msg.params[4];
#endif
                ret = DemuxCompSetUrlSource(mDemux, mSourceUrl, pHeaders);
                if(ret == 0)
                {
                    mStatus = AWPLAYER_STATUS_INITIALIZED;
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                }
                else
                {
                    loge("DemuxCompSetUrlSource() return fail.");
                    mStatus = AWPLAYER_STATUS_IDLE;
                    free(mSourceUrl);
                    mSourceUrl = NULL;
                    if(pReplyValue != NULL)
                        *pReplyValue = -1;
                }
            }
            else
            {
                //* support for fd source is delete.
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
            }

            if(pReplySem != NULL)
		        sem_post(pReplySem);
		    continue;
        } //* end AWPLAYER_COMMAND_SET_SOURCE.
        else if(msg.messageId == AWPLAYER_COMMAND_PREPARE)
        {
            logv("process message AWPLAYER_COMMAND_PREPARE.");

            if(mStatus != AWPLAYER_STATUS_INITIALIZED && mStatus != AWPLAYER_STATUS_STOPPED)
            {
                logd("invalid prepareAsync() call, player not in initialized or stopped status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                if(pReplySem != NULL)
		            sem_post(pReplySem);
		        continue;
            }

            mStatus = AWPLAYER_STATUS_PREPARING;
            mPrepareSync = msg.params[2];
            ret = DemuxCompPrepareAsync(mDemux);
            if(ret != 0)
            {
                loge("DemuxCompPrepareAsync return fail immediately.");
                mStatus = AWPLAYER_STATUS_IDLE;
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
            }
            else
            {
                if(pReplyValue != NULL)
                    *pReplyValue = 0;
            }

            if(pReplySem != NULL)
		        sem_post(pReplySem);
		    continue;
        } //* end AWPLAYER_COMMAND_PREPARE.
        else if(msg.messageId == AWPLAYER_COMMAND_START)
        {
            logv("process message AWPLAYER_COMMAND_START.");
            if(mStatus != AWPLAYER_STATUS_PREPARED &&
               mStatus != AWPLAYER_STATUS_STARTED  &&
               mStatus != AWPLAYER_STATUS_PAUSED   &&
               mStatus != AWPLAYER_STATUS_COMPLETE)
            {
                logd("invalid start() call, player not in prepared, started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                if(pReplySem != NULL)
		            sem_post(pReplySem);
		        continue;
            }

            if(mStatus == AWPLAYER_STATUS_STARTED)
            {
                logv("player already in started status.");
                if(pReplyValue != NULL)
                    *pReplyValue = 0;
                if(pReplySem != NULL)
		            sem_post(pReplySem);
		        continue;
            }

            pthread_mutex_lock(&mMutex);    //* synchronize with the seek or complete callback.
            if(mSeeking)
            {
                mStatus = AWPLAYER_STATUS_STARTED;  //* player and demux will be started at the seek callback.
                pthread_mutex_unlock(&mMutex);

                if(pReplyValue != NULL)
                    *pReplyValue = 0;
                if(pReplySem != NULL)
		            sem_post(pReplySem);
		        continue;
            }

            //* for complete status, we seek to the begin of the file.
            if(mStatus == AWPLAYER_STATUS_COMPLETE)
            {
                AwMessage newMsg;

                if(mMediaInfo->bSeekable)
                {
                    setMessage(&newMsg,
                               AWPLAYER_COMMAND_SEEK,   //* message id.
                               0,                       //* params[0] = &mSemSeek, internal message, do not post.
                               0,                       //* params[1] = &mSeekReply, internal message, do not set reply.
                               0,                       //* params[2] = mSeekTime(ms).
                               1);                      //* params[3] = mSeekSync.
                    AwMessageQueuePostMessage(mMessageQueue, &newMsg);

                    pthread_mutex_unlock(&mMutex);
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                    if(pReplySem != NULL)
		                sem_post(pReplySem);
		            continue;
                }
                else
                {
                    //* post a stop message.
                    setMessage(&newMsg,
                               AWPLAYER_COMMAND_STOP,   //* message id.
                               0,                       //* params[0] = &mSemStop, internal message, do not post.
                               0);                      //* params[1] = &mStopReply, internal message, do not reply.
                    AwMessageQueuePostMessage(mMessageQueue, &newMsg);

                    //* post a prepare message.
                    setMessage(&newMsg,
                               AWPLAYER_COMMAND_PREPARE,    //* message id.
                               0,                           //* params[0] = &mSemPrepare, internal message, do not post.
                               0,                           //* params[1] = &mPrepareReply, internal message, do not reply.
                               1);                          //* params[2] = mPrepareSync.
                    AwMessageQueuePostMessage(mMessageQueue, &newMsg);

                    //* post a start message.
                    setMessage(&newMsg,
                               AWPLAYER_COMMAND_START,      //* message id.
                               0,                           //* params[0] = &mSemStart, internal message, do not post.
                               0);                          //* params[1] = &mStartReply, internal message, do not reply.
                    AwMessageQueuePostMessage(mMessageQueue, &newMsg);

                    //* should I reply OK to the user at this moment?
                    //* or just set the semaphore and reply variable to the start message to
                    //* make it reply when start message done?
                    pthread_mutex_unlock(&mMutex);
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                    if(pReplySem != NULL)
		                sem_post(pReplySem);
		            continue;
                }
            }

            pthread_mutex_unlock(&mMutex);

            PlayerStart(mPlayer);
            DemuxCompStart(mDemux);
            mStatus = AWPLAYER_STATUS_STARTED;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
		        sem_post(pReplySem);
		    continue;

        } //* end AWPLAYER_COMMAND_START.
        else if(msg.messageId == AWPLAYER_COMMAND_STOP)
        {
            logv("process message AWPLAYER_COMMAND_STOP.");
            if(mStatus != AWPLAYER_STATUS_PREPARED &&
               mStatus != AWPLAYER_STATUS_STARTED  &&
               mStatus != AWPLAYER_STATUS_PAUSED   &&
               mStatus != AWPLAYER_STATUS_COMPLETE &&
               mStatus != AWPLAYER_STATUS_STOPPED)
            {
                logd("invalid stop() call, player not in prepared, paused, started, stopped or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                if(pReplySem != NULL)
		            sem_post(pReplySem);
		        continue;
            }

            if(mStatus == AWPLAYER_STATUS_STOPPED)
            {
                logv("player already in stopped status.");
                if(pReplyValue != NULL)
                    *pReplyValue = 0;
                if(pReplySem != NULL)
		            sem_post(pReplySem);
		        continue;
            }

            if(mStatus == AWPLAYER_STATUS_PREPARING)    //* the prepare callback may happen at this moment.
            {                                           //* so the mStatus may be changed to PREPARED asynchronizely.
                logw("stop() called at preparing status, cancel demux prepare.");
                DemuxCompCancelPrepare(mDemux);
            }

            if(mSeeking)
            {
                DemuxCompCancelSeek(mDemux);
                mSeeking = 0;
            }

            DemuxCompStop(mDemux);
            PlayerStop(mPlayer);
            PlayerClear(mPlayer);               //* clear all media information in player.
            mStatus  = AWPLAYER_STATUS_STOPPED;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
		        sem_post(pReplySem);
		    continue;
        } //* end AWPLAYER_COMMAND_STOP.
        else if(msg.messageId == AWPLAYER_COMMAND_PAUSE)
        {
            logv("process message AWPLAYER_COMMAND_PAUSE.");
            if(mStatus != AWPLAYER_STATUS_STARTED  &&
               mStatus != AWPLAYER_STATUS_PAUSED   &&
               mStatus != AWPLAYER_STATUS_COMPLETE)
            {
                logd("invalid pause() call, player not in started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                if(pReplySem != NULL)
		            sem_post(pReplySem);
		        continue;
            }

            if(mStatus == AWPLAYER_STATUS_PAUSED || mStatus == AWPLAYER_STATUS_COMPLETE)
            {
                logv("player already in paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = 0;
                if(pReplySem != NULL)
		            sem_post(pReplySem);
		        continue;
            }

            pthread_mutex_lock(&mMutex);        //* synchronize with the seek callback.

            if(mSeeking)
            {
                mStatus = AWPLAYER_STATUS_PAUSED;  //* player and demux will be paused at the seek callback.
                pthread_mutex_unlock(&mMutex);

                if(pReplyValue != NULL)
                    *pReplyValue = 0;
                if(pReplySem != NULL)
		            sem_post(pReplySem);
		        continue;
            }

            pthread_mutex_unlock(&mMutex);  //* sync with the seek or complete call back.

            PlayerPause(mPlayer);

            mStatus = AWPLAYER_STATUS_PAUSED;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
		        sem_post(pReplySem);
		    continue;
        } //* end AWPLAYER_COMMAND_PAUSE.
        else if(msg.messageId == AWPLAYER_COMMAND_RESET)
        {
            logv("process message AWPLAYER_COMMAND_RESET.");
            if(mStatus == AWPLAYER_STATUS_PREPARING)    //* the prepare callback may happen at this moment.
            {                                           //* so the mStatus may be changed to PREPARED asynchronizely.
                logw("reset() called at preparing status, cancel demux prepare.");
                DemuxCompCancelPrepare(mDemux);
            }

            if(mSeeking)
            {
                DemuxCompCancelSeek(mDemux);
                mSeeking = 0;
            }

            //* stop and clear the demux.
            DemuxCompStop(mDemux);  //* this will stop the seeking if demux is currently processing seeking message.
            DemuxCompClear(mDemux); //* it will clear the data source keep inside, this is important for the IStreamSource.

            //* stop and clear the player.
            PlayerStop(mPlayer);
            PlayerClear(mPlayer);   //* it will clear media info config to the player.

            //* clear data source.
            if(mSourceUrl != NULL)
            {
                free(mSourceUrl);
                mSourceUrl = NULL;
            }

            //* clear media info.
            clearMediaInfo();

            //* clear loop setting.
            mLoop   = 0;

            //* set status to IDLE.
            mStatus = AWPLAYER_STATUS_IDLE;

            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
		        sem_post(pReplySem);
		    continue;
        }
        else if(msg.messageId == AWPLAYER_COMMAND_SEEK)
        {
            logv("process message AWPLAYER_COMMAND_SEEK.");
            if(mStatus != AWPLAYER_STATUS_PREPARED &&
               mStatus != AWPLAYER_STATUS_STARTED  &&
               mStatus != AWPLAYER_STATUS_PAUSED   &&
               mStatus != AWPLAYER_STATUS_COMPLETE)
            {
                logd("invalid seekTo() call, player not in prepared, started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                if(pReplySem != NULL)
		            sem_post(pReplySem);
		        continue;
            }

            if(mMediaInfo == NULL || mMediaInfo->bSeekable == 0)
            {
                if(mMediaInfo == NULL)
                {
                    loge("seekTo fail because mMediaInfo == NULL.");
                    if(pReplyValue != NULL)
                        *pReplyValue = -1;
                    if(pReplySem != NULL)
		                sem_post(pReplySem);
		            continue;
                }
                else
                {
                    loge("media not seekable.");
                    if(pReplyValue != NULL)
                        *pReplyValue = -1;
                    if(pReplySem != NULL)
		                sem_post(pReplySem);
		            continue;
                }
            }

            if(mSeeking)
            {
                DemuxCompCancelSeek(mDemux);
                mSeeking = 0;
            }

            pthread_mutex_lock(&mMutex);
            mSeeking  = 1;
            mSeekTime = msg.params[2];
            mSeekSync = msg.params[3];
            logv("seekTo %.2f secs", mSeekTime / 1E3);
            pthread_mutex_unlock(&mMutex);

            //* if in prepared status, the player is not started yet,
            //* so PlayerPause() will return fail, it dosn't matter.
            //* player will be reset at the seek complete callback.
            PlayerPause(mPlayer);
            DemuxCompSeekTo(mDemux, mSeekTime);

            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
		        sem_post(pReplySem);
		    continue;
        } //* end AWPLAYER_COMMAND_SEEK.
        else if(msg.messageId == AWPLAYER_COMMAND_QUIT)
        {
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            if(pReplySem != NULL)
		        sem_post(pReplySem);
		    break;  //* break the thread.
        }
        else
        {
            logw("unknow message with id %d, ignore.", msg.messageId);
        }
    }

    return 0;
}


int AwPlayer::callbackProcess(int messageId, void* param)
{
    switch(messageId)
    {
        case AWPLAYER_MESSAGE_DEMUX_PREPARED:
        {
            uintptr_t tmpPtr = (uintptr_t)param;
			int err = tmpPtr;
            if(err != 0)
            {
                //* demux prepare return fail.
                //* notify a media error event.
                mStatus = AWPLAYER_STATUS_ERROR;
                if(mPrepareSync == 0)
                {
                    if(err == DEMUX_ERROR_IO)
                        mNotifier(mUserData, NOTIFY_ERROR, NOTIFY_ERROR_TYPE_IO, NULL);
                    else
                        mNotifier(mUserData, NOTIFY_ERROR, NOTIFY_ERROR_TYPE_UNKNOWN, NULL);
                }
                else
                {
                    mPrepareFinishResult = -1;
                    sem_post(&mSemPrepareFinish);
                }
            }
            else
            {
                //* demux prepare success, initialize the player.
                if(initializePlayer() == 0)
                {
                    //* initialize player success, notify a prepared event.
                    mStatus = AWPLAYER_STATUS_PREPARED;
                    if(mPrepareSync == 0)
                        mNotifier(mUserData, NOTIFY_PREPARED, 0, NULL);
                    else
                    {
                        mPrepareFinishResult = 0;
                        sem_post(&mSemPrepareFinish);
                    }
                }
                else
                {
                    //* initialize player fail, notify a media error event.
                    mStatus = AWPLAYER_STATUS_ERROR;
                    if(mPrepareSync == 0)
                        mNotifier(mUserData, NOTIFY_ERROR, NOTIFY_ERROR_TYPE_UNKNOWN, NULL);
                    else
                    {
                        mPrepareFinishResult = -1;
                        sem_post(&mSemPrepareFinish);
                    }
                }
            }

            break;
        }

        case AWPLAYER_MESSAGE_DEMUX_EOS:
        {
            PlayerSetEos(mPlayer);
            break;
        }

        case AWPLAYER_MESSAGE_DEMUX_IOERROR:
        {
            //* should we report a MEDIA_INFO event of "MEDIA_INFO_NETWORK_ERROR" and
            //* try reconnect for sometimes before a NOTIFY_ERROR_TYPE_IO event reported ?
            mNotifier(mUserData, NOTIFY_ERROR, NOTIFY_ERROR_TYPE_IO, NULL);
            break;
        }

        case AWPLAYER_MESSAGE_DEMUX_CACHE_REPORT:
        {
            int nTotalPercentage;
            int nBufferPercentage;

            nTotalPercentage  = ((int*)param)[0];   //* read positon to total file size.
            nBufferPercentage = ((int*)param)[1];   //* cache data size to start play cache size.
            mNotifier(mUserData, NOTIFY_BUFFERRING_UPDATE, nBufferPercentage<<16 | nTotalPercentage, NULL);
            break;
        }

        case AWPLAYER_MESSAGE_DEMUX_BUFFER_START:
        {
            //* TODO
            break;
        }

        case AWPLAYER_MESSAGE_DEMUX_BUFFER_END:
        {
            //* TODO
            break;
        }

        case AWPLAYER_MESSAGE_PLAYER_EOS:
        {
            mStatus = AWPLAYER_STATUS_COMPLETE;
            if(mLoop == 0)
            {
                logv("player notify eos.");
                mNotifier(mUserData, NOTIFY_PLAYBACK_COMPLETE, 0, NULL);
            }
            else
            {
                AwMessage msg;

                logv("player notify eos, loop is set, send start command.");
                //* send a start message.
                setMessage(&msg,
                           AWPLAYER_COMMAND_START,      //* message id.
                           0,                           //* params[0] = &mSemStart, internal message, do not post message.
                           0);                          //* params[1] = &mStartReply, internal message, do not reply.
                AwMessageQueuePostMessage(mMessageQueue, &msg);
            }
            break;
        }

        case AWPLAYER_MESSAGE_PLAYER_FIRST_PICTURE:
        {
            mNotifier(mUserData, NOTIFY_RENDERING_START, 0, NULL);
            break;
        }

        case AWPLAYER_MESSAGE_DEMUX_SEEK_FINISH:
        {
            int seekResult;
            int seekTimeMs;

            pthread_mutex_lock(&mMutex);    //* be careful to check whether there is any player callback lock the mutex,
                                            //* if so, the PlayerPause() call may fall into dead lock if the player
                                            //* callback is requesting mMutex.
                                            //* currently we do not lock mMutex in any player callback.

            seekResult = ((int*)param)[0];
            seekTimeMs = ((int*)param)[1];

            if(seekResult == 0)
            {
                PlayerReset(mPlayer, ((int64_t)seekTimeMs)*1000);

                if(seekTimeMs == mSeekTime)
                {
                    mSeeking = 0;
                    if(mStatus == AWPLAYER_STATUS_STARTED || mStatus == AWPLAYER_STATUS_COMPLETE)
                    {
                        PlayerStart(mPlayer);
                        DemuxCompStart(mDemux);
                        if(mStatus == AWPLAYER_STATUS_COMPLETE)
                            mStatus = AWPLAYER_STATUS_STARTED;
                    }
                }
                else
                {
                    logv("seek time not match, there may be another seek operation happening.");
                }
            }

            pthread_mutex_unlock(&mMutex);
            mNotifier(mUserData, NOTIFY_SEEK_COMPLETE, 0, NULL);
            break;
        }

        case AWPLAYER_MESSAGE_PLAYER_SUBTITLE_AVAILABLE:
        {
            //* skip subtitle.
            break;
        }

        case AWPLAYER_MESSAGE_PLAYER_SUBTITLE_EXPIRED:
        {
            //* skip subtitle.
            break;
        }

        default:
        {
            logw("message 0x%x not handled.", messageId);
            break;
        }
    }

    return 0;
}


static void* AwPlayerThread(void* arg)
{
    AwPlayer* me = (AwPlayer*)arg;
    me->mainThread();
    return NULL;
}


static int DemuxCallbackProcess(void* pUserData, int eMessageId, void* param)
{
    int       msg;
    AwPlayer* p;

    switch(eMessageId)
    {
        case DEMUX_NOTIFY_PREPARED:
            msg = AWPLAYER_MESSAGE_DEMUX_PREPARED;
            break;
        case DEMUX_NOTIFY_EOS:
            msg = AWPLAYER_MESSAGE_DEMUX_EOS;
            break;
        case DEMUX_NOTIFY_IOERROR:
            msg = AWPLAYER_MESSAGE_DEMUX_IOERROR;
            break;
        case DEMUX_NOTIFY_SEEK_FINISH:
            msg = AWPLAYER_MESSAGE_DEMUX_SEEK_FINISH;
            break;
        case DEMUX_NOTIFY_CACHE_STAT:
            msg = AWPLAYER_MESSAGE_DEMUX_CACHE_REPORT;
            break;
        case DEMUX_NOTIFY_BUFFER_START:
            msg = AWPLAYER_MESSAGE_DEMUX_BUFFER_START;
            break;
        case DEMUX_NOTIFY_BUFFER_END:
            msg = AWPLAYER_MESSAGE_DEMUX_BUFFER_END;
            break;
        default:
            logw("ignore demux callback message, eMessageId = 0x%x.", eMessageId);
            return -1;
    }

    p = (AwPlayer*)pUserData;
    p->callbackProcess(msg, param);

    return 0;
}


static int PlayerCallbackProcess(void* pUserData, int eMessageId, void* param)
{
    int       msg;
    AwPlayer* p;

    switch(eMessageId)
    {
        case PLAYER_NOTIFY_EOS:
            msg = AWPLAYER_MESSAGE_PLAYER_EOS;
            break;
        case PLAYER_NOTIFY_FIRST_PICTURE:
            msg = AWPLAYER_MESSAGE_PLAYER_FIRST_PICTURE;
            break;

        case PLAYER_NOTIFY_SUBTITLE_ITEM_AVAILABLE:
            msg = AWPLAYER_MESSAGE_PLAYER_SUBTITLE_AVAILABLE;
            break;

        case PLAYER_NOTIFY_SUBTITLE_ITEM_EXPIRED:
            msg = AWPLAYER_MESSAGE_PLAYER_SUBTITLE_EXPIRED;
            break;

        case PLAYER_NOTIFY_VIDEO_SIZE:              //* TODO
        case PLAYER_NOTIFY_VIDEO_CROP:              //* TODO
        default:
            logw("ignore player callback message, eMessageId = 0x%x.", eMessageId);
            return -1;
    }

    p = (AwPlayer*)pUserData;
    p->callbackProcess(msg, param);

    return 0;
}
