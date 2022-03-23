/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : xplayer.c
* Description : xplayer
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : first version
*
*/

#include "cdx_log.h"
#include "xplayer.h"
#include "demuxComponent.h"
#include <string.h>
#include <inttypes.h>

#include <version.h>

#include "player.h"
#include "demuxComponent.h"
#include "AwMessageQueue.h"
#ifndef ONLY_ENABLE_AUDIO
#include "layerControl.h"
#endif
#include "soundControl.h"

#ifndef ONLY_ENABLE_AUDIO
#include "subtitleControl.h"
#include "deinterlace.h"
#endif
#include "CdxParser.h"
#include <sys/time.h>
#include "CdxTime.h"

// pause then start to display, the cmcc apk call seekTo, it will flush the buffering cache(livemod)
// or seek some frames in cache ( vod) , it will discontinue
#define PAUSE_THEN_SEEK_BUG (1)

//* player status.
static const int XPLAYER_STATUS_IDLE        = 0;
static const int XPLAYER_STATUS_INITIALIZED = 1<<0;
static const int XPLAYER_STATUS_PREPARING   = 1<<1;
static const int XPLAYER_STATUS_PREPARED    = 1<<2;
static const int XPLAYER_STATUS_STARTED     = 1<<3;
static const int XPLAYER_STATUS_PAUSED      = 1<<4;
static const int XPLAYER_STATUS_STOPPED     = 1<<5;
static const int XPLAYER_STATUS_COMPLETE    = 1<<6;
static const int XPLAYER_STATUS_ERROR       = 1<<7;

//* command id.
static const int XPLAYER_COMMAND_SET_SOURCE    = 0x101;
static const int XPLAYER_COMMAND_SET_SURFACE   = 0x102;
static const int XPLAYER_COMMAND_SET_AUDIOSINK = 0x103;
static const int XPLAYER_COMMAND_PREPARE       = 0x104;
static const int XPLAYER_COMMAND_START         = 0x105;
static const int XPLAYER_COMMAND_STOP          = 0x106;
static const int XPLAYER_COMMAND_PAUSE         = 0x107;
static const int XPLAYER_COMMAND_RESET         = 0x108;
static const int XPLAYER_COMMAND_QUIT          = 0x109;
static const int XPLAYER_COMMAND_SEEK          = 0x10a;
static const int XPLAYER_COMMAND_RESETURL      = 0x10b;
static const int XPLAYER_COMMAND_SETSPEED      = 0x10c;
static const int XPLAYER_COMMAND_SET_SUBCTRL   = 0x10d;
static const int XPLAYER_COMMAND_SET_DI        = 0x10e;
static const int XPLAYER_COMMAND_SET_PLAYRATE  = 0x10f;

typedef struct PlayerContext
{
    AwMessageQueue*     mMessageQueue;
    Player*             mPlayer;
    DemuxComp*          mDemux;
    pthread_t           mThreadId;
    int                 mThreadCreated;
    uid_t               mUID;             //* no use.

    //* data source.
    char*               mSourceUrl;       //* file path or network stream url.
    CdxStreamT*         mSourceStream;    //* outside streaming source like miracast.

    int                 mSourceFd;        //* file descriptor.
    int64_t             mSourceFdOffset;
    int64_t             mSourceFdLength;
#ifndef ONLY_ENABLE_AUDIO
    //for setting scale down
    //* bSetScaleDown = 0 means do not set scale down.
    //* bSetScaleDown = 1 means scaling when the resolution is larger than 2k.
    //* here the case of the VR9 call bSetScaleDown = 1.
    //* bSetScaleDown = 2 means scaling at any resolution.
    int                 bSetScaleDown;
#endif
    //* media information.
    MediaInfo*          mMediaInfo;
#ifndef ONLY_ENABLE_AUDIO
    //* text codec format of the subtitle, used to transform subtitle text to
    //* utf8 when the subtitle text codec format is unknown.
    char                mDefaultTextFormat[32];

    //* whether enable subtitle show.
    int                 mIsSubtitleDisable;
#endif
    //* file descriptor of .idx file of index+sub subtitle.
    //* we save the .idx file's fd here because application set .idx file and .sub file
    //* seperately, we need to wait for the .sub file's fd, see
    //* INVOKE_ID_ADD_EXTERNAL_SOURCE_FD command in invoke() method.
    int                 mIndexFileHasBeenSet;
#ifndef ONLY_ENABLE_AUDIO
    int                 mIndexFileFdOfIndexSubtitle;
#endif
    //* for status and synchronize control.
    int                 mStatus;
    pthread_mutex_t     mMutexMediaInfo;    //* for media info protection.
    //* for mStatus protection in start/stop/pause operation and complete/seek finish callback.
    pthread_mutex_t     mMutexStatus;
    sem_t               mSemSetDataSource;
    sem_t               mSemPrepare;
    sem_t               mSemStart;
    sem_t               mSemStop;
    sem_t               mSemPause;
    sem_t               mSemQuit;
    sem_t               mSemReset;
    sem_t               mSemSeek;
#ifndef ONLY_ENABLE_AUDIO
    sem_t               mSemSetSurface;
#endif
    sem_t               mSemSetAudioSink;
    sem_t               mSemPrepareFinish;//* for signal prepare finish, used in prepare().
#ifndef ONLY_ENABLE_AUDIO
    sem_t               mSemSetSpeed;
    sem_t               mSemSetSubCtrl;
    sem_t               mSemSetDeinterlace;
#endif
    sem_t               mSemSetPlayBackSettings;

    //* status control.
    int                 mSetDataSourceReply;
    int                 mPrepareReply;
    int                 mStartReply;
    int                 mStopReply;
    int                 mPauseReply;
    int                 mResetReply;
    int                 mSeekReply;
#ifndef ONLY_ENABLE_AUDIO
    int                 mSetSurfaceReply;
#endif
    int                 mSetAudioSinkReply;
    int                 mPrepareFinishResult;   //* save the prepare result for prepare().
#ifndef ONLY_ENABLE_AUDIO
    int                 mSetSpeedReply;
    int                 mSetSubCtrlReply;
    int                 mSetDeinterlaceReply;
#endif
    int                 mSetPlayBackSettingsReply;

    int                 mPrepareSync;   //* synchroized prarare() call, don't call back to user.
    int                 mSeeking;
    int                 mSeekByInner;

    //* use to check whether seek callback is for current seek operation or previous.
    int                 mSeekTime;
    SeekModeType        mSeekModeType;
    int                 mSeekSync;  //* internal seek, don't call back to user.
    int                 mLoop;
#ifndef ONLY_ENABLE_AUDIO
    int                 mKeepLastFrame;
    int                 mVideoSizeWidth;  //* use to record videoSize which had send to app
    int                 mVideoSizeHeight;
#endif
    enum AwApplicationType mApplicationType;

    void*               mHTTPService;
#ifndef ONLY_ENABLE_AUDIO
    //* record the id of subtitle which is displaying
    //* we set the Nums to 64 .(32 may be not enough)
    unsigned int        mSubtitleDisplayIds[64];
    int                 mSubtitleDisplayIdsUpdateIndex;
#endif
    //* save the currentSelectTrackIndex;
    int                 mCurrentSelectTrackIndex;
    int                 mRawOccupyFlag;

    int                 mLivemode;
    int                 mPauseLivemode;
    int                 mbIsDiagnose;
    int64_t             mPauseTimeStamp;    //us
    int64_t             mShiftTimeStamp;    //us
#ifndef ONLY_ENABLE_AUDIO
    int                 mDisplayRatio;
#endif
    int64_t             mCurShiftTimeStamp; //us
    int64_t             mTimeShiftDuration; //ms

    int                 mSeekTobug;

    // the cmcc player should change pause state when buffer start, to fix getposition bug
    int                 mDemuxNotifyPause;
    int64_t             mDemuxPauseTimeStamp;

    //*

    int                 mPreSeekTimeMs;
    int                 mTimeMsBeforeStop;  //softdetector
#ifndef ONLY_ENABLE_AUDIO
    int                 mSpeed;
    int                 mbFast;
    int                 mFastTime;
    int                 mSpeedThreadRuning;
    pthread_t           mSpeedThreadId;

    int                 mScaledownFlag;

    int                 mWidthScaledownRatio;
    int                 mHeightScaledownRatio;

    int                 mVideoRotateDegree;
#endif
    XPlayerNotifyCallback mCallback;
    void*                  pUser;
    AwBufferingSettings* pBuffering;
}PlayerContext;

static void* XPlayerThread(void* arg);
static int ShiftTimeMode(int Shiftedms, char *buf);
static void clearMediaInfo(XPlayer* p);
static int callbackProcess(void *player, int messageId, void* param);

struct AwMessage {
    AWMESSAGE_COMMON_MEMBERS
    uintptr_t params[8];
};

XPlayer* XPlayerCreate()
{
    logd("XPlayerCreate.");
    XPlayer* mPriData;
    LogVersionInfo();
    mPriData = (PlayerContext*)malloc(sizeof(PlayerContext));
    memset(mPriData,0x00,sizeof(PlayerContext));

    mPriData->mUID            = -1;
    mPriData->mSourceUrl      = NULL;
    mPriData->mSourceFd       = -1;
    mPriData->mSourceFdOffset = 0;
    mPriData->mSourceFdLength = 0;
    mPriData->mSourceStream   = NULL;
    mPriData->mStatus         = XPLAYER_STATUS_IDLE;
    mPriData->mSeeking        = 0;
    mPriData->mSeekByInner    = 0;
    mPriData->mSeekSync       = 0;
    mPriData->mLoop           = 0;
#ifndef ONLY_ENABLE_AUDIO
    mPriData->mKeepLastFrame  = 0;
#endif
    mPriData->mMediaInfo      = NULL;
    mPriData->mMessageQueue   = NULL;
#ifndef ONLY_ENABLE_AUDIO
    mPriData->mVideoSizeWidth = 0;
    mPriData->mVideoSizeHeight= 0;
    mPriData->mScaledownFlag =0;
    mPriData->mWidthScaledownRatio = 0;
    mPriData->mHeightScaledownRatio = 0;
#endif
    mPriData->mCurrentSelectTrackIndex = -1;

    mPriData->mDemuxNotifyPause = 0;

#if    PAUSE_THEN_SEEK_BUG
    mPriData->mSeekTobug = 0;
#endif

    pthread_mutex_init(&mPriData->mMutexMediaInfo, NULL);
    pthread_mutex_init(&mPriData->mMutexStatus, NULL);
    sem_init(&mPriData->mSemSetDataSource, 0, 0);
    sem_init(&mPriData->mSemPrepare, 0, 0);
    sem_init(&mPriData->mSemStart, 0, 0);
    sem_init(&mPriData->mSemStop, 0, 0);
    sem_init(&mPriData->mSemPause, 0, 0);
    sem_init(&mPriData->mSemReset, 0, 0);
    sem_init(&mPriData->mSemQuit, 0, 0);
    sem_init(&mPriData->mSemSeek, 0, 0);
#ifndef ONLY_ENABLE_AUDIO
    sem_init(&mPriData->mSemSetSurface, 0, 0);
#endif
    sem_init(&mPriData->mSemSetAudioSink, 0, 0);
#ifndef ONLY_ENABLE_AUDIO
    sem_init(&mPriData->mSemSetSubCtrl, 0, 0);
    sem_init(&mPriData->mSemSetDeinterlace, 0, 0);
#endif
    sem_init(&mPriData->mSemPrepareFinish, 0, 0); //* for signal prepare finish, used in prepare().
#ifndef ONLY_ENABLE_AUDIO
    sem_init(&mPriData->mSemSetSpeed, 0, 0);
#endif
    sem_init(&mPriData->mSemSetPlayBackSettings, 0, 0);

    mPriData->mMessageQueue = AwMessageQueueCreate(64, "XPlayer");
    mPriData->mPlayer       = PlayerCreate();
    mPriData->mDemux        = DemuxCompCreate();

    if(mPriData->mPlayer != NULL)
        PlayerSetCallback(mPriData->mPlayer, callbackProcess, (void*)mPriData);

    if(mPriData->mDemux != NULL)
    {
        DemuxCompSetCallback(mPriData->mDemux, callbackProcess, (void*)mPriData);
        DemuxCompSetPlayer(mPriData->mDemux, mPriData->mPlayer);
    }

    if(pthread_create(&mPriData->mThreadId, NULL, XPlayerThread, mPriData) == 0)
        mPriData->mThreadCreated = 1;
    else
        mPriData->mThreadCreated = 0;
#ifndef ONLY_ENABLE_AUDIO
    strcpy(mPriData->mDefaultTextFormat, "GBK");
    mPriData->mIndexFileFdOfIndexSubtitle = -1;
#endif
    mPriData->mIndexFileHasBeenSet = 0;
#ifndef ONLY_ENABLE_AUDIO
    memset(mPriData->mSubtitleDisplayIds,0xff,64*sizeof(unsigned int));
    mPriData->mSubtitleDisplayIdsUpdateIndex = 0;
#endif
    mPriData->mApplicationType = APP_DEFAULT;
    mPriData->mRawOccupyFlag = 0;
#ifndef ONLY_ENABLE_AUDIO
    mPriData->mSpeed = 1;
    mPriData->mbFast = 0;
    mPriData->mFastTime = 0;
    mPriData->mSpeedThreadRuning = 0;
#endif
    return mPriData;
}


void XPlayerDestroy(XPlayer* p)
{
    AwMessage msg;
    XPlayer* mPriData;
    int param_occupy[3]  = {1,0,0};
    int param_release[3] = {0,0,0};
    logw("XPlayerDestroy");

    mPriData = (XPlayer*)p;
    if(mPriData->mThreadCreated)
    {
        void* status;

        XPlayerReset(p);    //* stop demux and player.

        //* send a quit message to quit the main thread.
        memset(&msg, 0, sizeof(AwMessage));
        msg.messageId = XPLAYER_COMMAND_QUIT;
        msg.params[0] = (uintptr_t)&mPriData->mSemQuit;
        AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
        SemTimedWait(&mPriData->mSemQuit, -1);
        pthread_join(mPriData->mThreadId, &status);
    }
#ifndef ONLY_ENABLE_AUDIO
    if (mPriData->mSpeedThreadRuning == 1)
    {
        void* status;
        mPriData->mSpeedThreadRuning = 0;
        pthread_join(mPriData->mSpeedThreadId,&status);
    }
#endif
    if(mPriData->mDemux != NULL)
        DemuxCompDestroy(mPriData->mDemux);

    if(mPriData->mPlayer != NULL)
        PlayerDestroy(mPriData->mPlayer);

    if(mPriData->mMessageQueue != NULL)
    {
        AwMessageQueueDestroy(mPriData->mMessageQueue);
        mPriData->mMessageQueue = NULL;
    }

    pthread_mutex_destroy(&mPriData->mMutexMediaInfo);
    pthread_mutex_destroy(&mPriData->mMutexStatus);
    sem_destroy(&mPriData->mSemSetDataSource);
    sem_destroy(&mPriData->mSemPrepare);
    sem_destroy(&mPriData->mSemStart);
    sem_destroy(&mPriData->mSemStop);
    sem_destroy(&mPriData->mSemPause);
    sem_destroy(&mPriData->mSemReset);
    sem_destroy(&mPriData->mSemQuit);
    sem_destroy(&mPriData->mSemSeek);
#ifndef ONLY_ENABLE_AUDIO
    sem_destroy(&mPriData->mSemSetSurface);
#endif
    sem_destroy(&mPriData->mSemSetAudioSink);
    sem_destroy(&mPriData->mSemPrepareFinish);
#ifndef ONLY_ENABLE_AUDIO
    sem_destroy(&mPriData->mSemSetSpeed);
    sem_destroy(&mPriData->mSemSetSubCtrl);
    sem_destroy(&mPriData->mSemSetDeinterlace);
#endif
    sem_destroy(&mPriData->mSemSetPlayBackSettings);

    if(mPriData->mMediaInfo != NULL)
        clearMediaInfo(p);

    if(mPriData->mSourceUrl != NULL)
        free(mPriData->mSourceUrl);

    if(mPriData->mSourceFd != -1)
        close(mPriData->mSourceFd);

#ifndef ONLY_ENABLE_AUDIO
    if(mPriData->mIndexFileFdOfIndexSubtitle != -1)
        close(mPriData->mIndexFileFdOfIndexSubtitle);
#endif

    free(mPriData);
}

int XPlayerConfig(XPlayer* p, const XPlayerConfig_t *config)
{
    p->mLivemode = config->livemode;
    p->mApplicationType = config->appType;
    return 0;
}

int XPlayerInitCheck(XPlayer* p)
{
    logv("initCheck");
    PlayerContext* mPriData = (PlayerContext*)p;
    if(mPriData->mPlayer == NULL || mPriData->mDemux == NULL || mPriData->mThreadCreated == 0)
    {
        loge("initCheck() fail, XPlayer::mplayer = %p, XPlayer::mDemux = %p",
                mPriData->mPlayer, mPriData->mDemux);
        return -1;
    }
    else
        return 0;
}


int XPlayerSetUID(XPlayer* p, int nUid)
{
    (void)nUid;
    (void)p;
    logv("setUID(), uid = %d", nUid);
    //mPriData->mUID = uid;
    return 0;
}

int XPlayerSetHdcpOps(XPlayer* p, struct HdcpOpsS* pHdcp)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    if(mPriData->mStatus != XPLAYER_STATUS_IDLE &&
        mPriData->mStatus != XPLAYER_STATUS_INITIALIZED)
    {
        logw("set hdcp ops incorrect. status: %d", mPriData->mStatus);
        return -1;
    }

    if(p->mDemux)
        DemuxCompSetHdcpOps(p->mDemux, pHdcp);
    return 0;
}

#ifndef ONLY_ENABLE_AUDIO
int XPlayerSetScaleDown(XPlayer* p,int bSetScaleDown)
{
    if(p != NULL)
    {
       p->bSetScaleDown = bSetScaleDown;
       logd("XPlayer set scale down.p->bSetScaleDown:%d",p->bSetScaleDown);
       return 0;
    }
    else
    {
        loge("xplayer set scale down error!");
        return -1;
    }
}
#endif

int XPlayerSetDataSourceUrl(XPlayer* p,
                            const char* pUrl,
                            void* httpService,
                            const CdxKeyedVectorT* pHeaders)
{
    AwMessage msg;
    int ret;
    PlayerContext* mPriData = (PlayerContext*)p;

    if(pUrl == NULL)
    {
        loge("setDataSource(url), url=NULL");
        return -1;
    }

    logd("setDataSource(url), url='%s'", pUrl);

    DemuxCompSetLiveMode(mPriData->mDemux, p->mLivemode);

    mPriData->mHTTPService = httpService;

#ifndef ONLY_ENABLE_AUDIO
    PlayerConfigDispErrorFrame(mPriData->mPlayer, 0);
#endif

    //* send a set data source message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_SET_SOURCE;
    msg.params[0] = (uintptr_t)&mPriData->mSemSetDataSource;
    msg.params[1] = (uintptr_t)&mPriData->mSetDataSourceReply;
    msg.params[2] = SOURCE_TYPE_URL;
    msg.params[3] = (uintptr_t)pUrl;
    msg.params[4] = (uintptr_t)pHeaders;
    msg.params[5] = (uintptr_t)(mPriData->mHTTPService);

    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemSetDataSource, -1);

    if(mPriData->mSetDataSourceReply != 0)
        return mPriData->mSetDataSourceReply;

    //* for local file, I think we should ack like file descriptor source,
    //* so here we call prepare().
    //* local file list: 'bdmv://---',  '/---'
    if (strncmp(pUrl, "bdmv://", 7) == 0 || strncmp(pUrl, "file://", 7) == 0 || pUrl[0] == '/')
    {
        //* for local file source set as a file descriptor,
        //* the application will call invoke() by command INVOKE_ID_GET_TRACK_INFO
        //* to get track info, so we need call prepare() here to obtain media information before
        //* application call prepareAsync().
        //* here I think for local file source set as a url, we should ack the same as the file
        //* descriptor case.
        ret = XPlayerPrepare(p);
        if (ret != 0)
        {
            loge("prepare failure, ret(%d)", ret);
        }
        return ret;
    }
    else
        return 0;
}


//* Warning: The filedescriptor passed into this method will only be valid until
//* the method returns, if you want to keep it, dup it!
int XPlayerSetDataSourceFd(XPlayer* p, int fd, int64_t offset, int64_t length)
{
    AwMessage msg;
    int ret = 0;
    PlayerContext* mPriData = (PlayerContext*)p;
    logv("setDataSource(fd), fd=%d", fd);
#ifndef ONLY_ENABLE_AUDIO
    PlayerConfigDispErrorFrame(mPriData->mPlayer, 0);
#endif
    //* send a set data source message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_SET_SOURCE;
    msg.params[0] = (uintptr_t)&mPriData->mSemSetDataSource;
    msg.params[1] = (uintptr_t)&mPriData->mSetDataSourceReply;
    msg.params[2] = SOURCE_TYPE_FD;
    msg.params[3] = fd;
    msg.params[4] = (uintptr_t)(offset>>32);        //* params[4] = high 32 bits of offset.
    msg.params[5] = (uintptr_t)(offset & 0xffffffff);//* params[5] = low 32 bits of offset.
    msg.params[6] = (uintptr_t)(length>>32);     //* params[6] = high 32 bits of length.
    msg.params[7] = (uintptr_t)(length & 0xffffffff);//* params[7] = low 32 bits of length.
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemSetDataSource, -1);

    if(mPriData->mSetDataSourceReply != 0)
        return mPriData->mSetDataSourceReply;

#if !defined(CONF_SET_SCALEDOWN_MODE)
    //* for local files, the application will call invoke() by command INVOKE_ID_GET_TRACK_INFO
    //* to get track info, so we need call prepare() here to obtain media information before
    //* application call prepareAsync().
    ret = XPlayerPrepare(p);
    if (ret != 0)
    {
        loge("prepare failure, ret(%d)", ret);
    }
#endif
    return ret;
}

int XPlayerSetDataSourceStream(XPlayer* p, const char* streamStr)
{
    AwMessage msg;
    logd("setDataSource(IStreamSource)");

    PlayerContext* mPriData = (PlayerContext*)p;
#ifndef ONLY_ENABLE_AUDIO
    PlayerConfigDispErrorFrame(mPriData->mPlayer, 1);
#endif
    //* send a set data source message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_SET_SOURCE;
    msg.params[0] = (uintptr_t)&mPriData->mSemSetDataSource;
    msg.params[1] = (uintptr_t)&mPriData->mSetDataSourceReply;
    msg.params[2] = SOURCE_TYPE_ISTREAMSOURCE;
    msg.params[3] = (uintptr_t)streamStr;
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemSetDataSource, -1);
    return mPriData->mSetDataSourceReply;
}

int XPlayerSetMediaDataSource(XPlayer* p, const char* streamStr)
{
    AwMessage msg;
    logd("setDataSource(MediaDataSource)");

    PlayerContext* mPriData = (PlayerContext*)p;
#ifndef ONLY_ENABLE_AUDIO
    PlayerConfigDispErrorFrame(mPriData->mPlayer, 1);
#endif
    //* send a set data source message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_SET_SOURCE;
    msg.params[0] = (uintptr_t)&mPriData->mSemSetDataSource;
    msg.params[1] = (uintptr_t)&mPriData->mSetDataSourceReply;
    msg.params[2] = SOURCE_TYPE_DATASOURCE;
    msg.params[3] = (uintptr_t)streamStr;
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemSetDataSource, -1);
    return mPriData->mSetDataSourceReply;
}

#ifndef ONLY_ENABLE_AUDIO
//* set video layer control ops
int XPlayerSetVideoSurfaceTexture(XPlayer* p, const LayerCtrl* surfaceTexture)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logd("setVideoSurfaceTexture, surface = %p", surfaceTexture);

    //* send a set surface message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_SET_SURFACE;
    msg.params[0] = (uintptr_t)&mPriData->mSemSetSurface;
    msg.params[1] = (uintptr_t)&mPriData->mSetSurfaceReply;
    msg.params[2] = (uintptr_t)surfaceTexture;
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemSetSurface, -1);

    return mPriData->mSetSurfaceReply;
}
#endif

void XPlayerSetAudioSink(XPlayer* p, const SoundCtrl* audioSink)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logv("setAudioSink");

    //* send a set audio sink message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_SET_AUDIOSINK;
    msg.params[0] = (uintptr_t)&mPriData->mSemSetAudioSink;
    msg.params[1] = (uintptr_t)&mPriData->mSetAudioSinkReply;
    msg.params[2] = (uintptr_t)audioSink;
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemSetAudioSink, -1);

    return;
}

int XPlayerSetPlaybackSettings(XPlayer* p,const XAudioPlaybackRate *rate)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    //* send a set set playback message.
    AwMessage msg;
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_SET_PLAYRATE;
    msg.params[0] = (uintptr_t)&mPriData->mSemSetPlayBackSettings;
    msg.params[1] = (uintptr_t)&mPriData->mSetPlayBackSettingsReply;
    msg.params[2] = (uintptr_t)rate;
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemSetPlayBackSettings, -1);

    if (rate->mSpeed == 0.f && mPriData->mStatus == XPLAYER_STATUS_STARTED)
    {
       logd("PlayerPause");
       XPlayerPause(p);
    }
    else if (rate->mSpeed != 0.f && (mPriData->mStatus == XPLAYER_STATUS_PAUSED
           || mPriData->mStatus == XPLAYER_STATUS_PREPARED
           || mPriData->mStatus == XPLAYER_STATUS_COMPLETE))
    {
       XPlayerStart(p);
    }

    return mPriData->mSetPlayBackSettingsReply;
}

int XPlayerGetPlaybackSettings(XPlayer* p,XAudioPlaybackRate *rate)
{
    logd("XPlayerGetPlaybackSettings");

    PlayerContext* mPriData = (PlayerContext*)p;
    int ret = PlayerGetPlayBackSettings(mPriData->mPlayer,rate);

    if (ret == 0)
    {
        if (mPriData->mStatus == XPLAYER_STATUS_PAUSED)
        {
            rate->mSpeed = 0.f;
        }
    }
    return ret;
}

#ifndef ONLY_ENABLE_AUDIO
void XPlayerSetSubCtrl(XPlayer* p, const SubCtrl* subctrl)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logv("setSubRender");
    //* send a set audio sink message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_SET_SUBCTRL;
    msg.params[0] = (uintptr_t)&mPriData->mSemSetSubCtrl;
    msg.params[1] = (uintptr_t)&mPriData->mSetSubCtrlReply;
    msg.params[2] = (uintptr_t)subctrl;
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemSetSubCtrl, -1);

    return;
}

void XPlayerSetDeinterlace(XPlayer* p, const Deinterlace* di)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logd("set deinterlace");
    //* send a set audio sink message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_SET_DI;
    msg.params[0] = (uintptr_t)&mPriData->mSemSetDeinterlace;
    msg.params[1] = (uintptr_t)&mPriData->mSetDeinterlaceReply;
    msg.params[2] = (uintptr_t)di;
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemSetDeinterlace, -1);

    return;
}
#endif

int XPlayerSetNotifyCallback(XPlayer* p,
                                    XPlayerNotifyCallback notifier,
                                    void* pUserData)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    mPriData->mCallback = notifier;
    mPriData->pUser = pUserData;

    return 0;
}

int XPlayerPrepareAsync(XPlayer* p)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logd("prepareAsync");

    //* send a prepare.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_PREPARE;
    msg.params[0] = (uintptr_t)&mPriData->mSemPrepare;
    msg.params[1] = (uintptr_t)&mPriData->mPrepareReply;
    msg.params[2] = 0;
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemPrepare, -1);

    return mPriData->mPrepareReply;
}


int XPlayerPrepare(XPlayer* p)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logd("prepare");

    //* clear the mSemPrepareFinish semaphore.
    while(sem_trywait(&mPriData->mSemPrepareFinish) == 0);

    //* send a prepare message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_PREPARE;
    msg.params[0] = (uintptr_t)&mPriData->mSemPrepare;
    msg.params[1] = (uintptr_t)&mPriData->mPrepareReply;
    msg.params[2] = 1;          //* params[2] = mPrepareSync.
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemPrepare, -1);

    if(mPriData->mPrepareReply == 0)
    {
        //* wait for the prepare finish.
        SemTimedWait(&mPriData->mSemPrepareFinish, -1);
        return mPriData->mPrepareFinishResult;
    }

    //* call DemuxCompPrepareAsync() fail, or status error.
    return mPriData->mPrepareReply;
}

int XPlayerStart(XPlayer* p)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logd("start");

    // in washu cmcc for livemode 1, we should call seekTo when paused longer than 30s
    mPriData->mLivemode = DemuxCompGetLiveMode(mPriData->mDemux);
    if(mPriData->mStatus == XPLAYER_STATUS_PAUSED &&
            mPriData->mLivemode == 1 &&
            mPriData->mApplicationType == APP_CMCC_WASU)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        int shiftTimeMs = (tv.tv_sec * 1000000ll + tv.tv_usec - mPriData->mPauseTimeStamp)/1000;
        if(shiftTimeMs > 30*1000 && !mPriData->mSeekTobug && (mPriData->mPauseLivemode!=2))
        {
            logd("pause time longer than 30s in livemode 1, seekTo(%d)", shiftTimeMs);
            XPlayerSeekTo(p, shiftTimeMs, mPriData->mSeekModeType);
        }
    }

#if (PAUSE_THEN_SEEK_BUG)
    if(mPriData->mSeekTobug && mPriData->mApplicationType == APP_CMCC_WASU)
    {
        mPriData->mSeekTobug = 0;
        mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_SEEK_COMPLETE, 0, 0);
    }
#endif

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_START;
    msg.params[0] = (uintptr_t)&mPriData->mSemStart;
    msg.params[1] = (uintptr_t)&mPriData->mStartReply;
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemStart, -1);
    return mPriData->mStartReply;
}


int XPlayerStop(XPlayer* p)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logd("stop");
    XPlayerGetCurrentPosition(mPriData, &mPriData->mTimeMsBeforeStop);  //softdetector

    //* send a stop message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_STOP;
    msg.params[0] = (uintptr_t)&mPriData->mSemStop;
    msg.params[1] = (uintptr_t)&mPriData->mStopReply;
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemStop, -1);

    logd("=== stop end");
    return mPriData->mStopReply;
}


int XPlayerPause(XPlayer* p)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logd("pause");

    mPriData->mLivemode = DemuxCompGetLiveMode(mPriData->mDemux);
    mPriData->mPauseLivemode = mPriData->mLivemode;
    if(mPriData->mLivemode == 1)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        mPriData->mPauseTimeStamp = tv.tv_sec * 1000000ll + tv.tv_usec;
        logd("livemode1, get current position inmPauseTimeStamp = %" PRId64 "",
                mPriData->mPauseTimeStamp);
    }
    else if(mPriData->mLivemode == 2)
    {
        int msec;
        XPlayerGetCurrentPosition(p, &msec);
        logd("get current position in pause: %d", msec);

        struct timeval tv;
        gettimeofday(&tv, NULL);
        mPriData->mPauseTimeStamp = tv.tv_sec * 1000000ll + tv.tv_usec - (int64_t)msec*1000;
        logd("get current position in pause, nowUs = %ld, mPauseTimeStamp = %" PRId64 "",
                tv.tv_sec * 1000000 + tv.tv_usec, mPriData->mPauseTimeStamp);
    }

    //* send a pause message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_PAUSE;
    msg.params[0] = (uintptr_t)&mPriData->mSemPause;
    msg.params[1] = (uintptr_t)&mPriData->mPauseReply;
    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemPause, -1);
    return mPriData->mPauseReply;
}

int XPlayerSeekTo(XPlayer* p, int nSeekTimeMs, SeekModeType nSeekModeType)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logd("seekTo [%dms]", nSeekTimeMs);
    mPriData->mSeekModeType = nSeekModeType;

    mPriData->mLivemode = DemuxCompGetLiveMode(mPriData->mDemux);

#if (PAUSE_THEN_SEEK_BUG)
    if((mPriData->mStatus == XPLAYER_STATUS_PAUSED
        /*|| mPriData->mStatus == XPLAYER_STATUS_STARTED */) &&
        mPriData->mApplicationType == APP_CMCC_WASU)
    {
        int pos;
        XPlayerGetCurrentPosition(p, &pos);
        int diff = pos - nSeekTimeMs;
        logd("diff = %d", diff);
        if(-1000 < diff && diff < 1000)
        {
            if(mPriData->mLivemode == 1 && mPriData->mStatus == XPLAYER_STATUS_PAUSED)
            {
                // if livemod1, we should change it to livemode2
                struct timeval tv;
                gettimeofday(&tv, NULL);
                DemuxCompSetLiveMode(mPriData->mDemux, 2);
                mPriData->mShiftTimeStamp = tv.tv_sec * 1000000ll + tv.tv_usec - nSeekTimeMs*1000ll;
            }
            mPriData->mSeekTobug = 1;
            return XPlayerStart(p);
        }
    }
#endif

    if(mPriData->mLivemode == 1 || mPriData->mLivemode == 2)
    {
        logd("++++++ reset url");
        struct timeval tv;
        gettimeofday(&tv, NULL);
        mPriData->mShiftTimeStamp = tv.tv_sec * 1000000ll + tv.tv_usec;
        mPriData->mPreSeekTimeMs = nSeekTimeMs + 20000; //* for timeshift

        //* send a start message.
        memset(&msg, 0, sizeof(AwMessage));
        msg.messageId = XPLAYER_COMMAND_RESETURL;
        msg.params[0] = (uintptr_t)&mPriData->mSemSeek;
        msg.params[1] = (uintptr_t)&mPriData->mSeekReply;
        msg.params[2] = nSeekTimeMs;  //* params[2] = mSeekTime.
        //* params[3] = mSeekSync.
        AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
        SemTimedWait(&mPriData->mSemSeek, -1);
    }
    else
    {
        logd("seek");
        mPriData->mSeekByInner = 0;
        //* send a start message.
        memset(&msg, 0, sizeof(AwMessage));
        msg.messageId = XPLAYER_COMMAND_SEEK;
        msg.params[0] = (uintptr_t)&mPriData->mSemSeek;
        msg.params[1] = (uintptr_t)&mPriData->mSeekReply;
        msg.params[2] = nSeekTimeMs;  //* params[2] = mSeekTime.
        AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
        SemTimedWait(&mPriData->mSemSeek, -1);

    }

    return mPriData->mSeekReply;
}

int XPlayerReset(XPlayer* p)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logw("reset...");
    XPlayerGetCurrentPosition(mPriData, &mPriData->mTimeMsBeforeStop);  //softdetector

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_RESET;
    msg.params[0] = (uintptr_t)&mPriData->mSemReset;
    msg.params[1] = (uintptr_t)&mPriData->mResetReply;

    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemReset, -1);
    return mPriData->mResetReply;
}

#ifndef ONLY_ENABLE_AUDIO
int XPlayerSetSpeed(XPlayer* p, int nSpeed)
{
    AwMessage msg;
    PlayerContext* mPriData = (PlayerContext*)p;

    logd("XPlayerSetSpeed %d",nSpeed);
    mPriData->mSpeed = nSpeed;
    //PlayerSetSpeed(mPriData->mPlayer, nSpeed);

    if(nSpeed == 1)
    {
        mPriData->mbFast = 0;
        mPriData->mSpeed = 1;
        mPriData->mFastTime = 0;
        XPlayerStart(p);
        PlayerSetDiscardAudio(mPriData->mPlayer, 0);
        return 0;
    }

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = XPLAYER_COMMAND_SETSPEED;
    msg.params[0] = (uintptr_t)&mPriData->mSemSetSpeed;
    msg.params[1] = (uintptr_t)&mPriData->mSetSpeedReply;
    msg.params[2] = nSpeed;

    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
    SemTimedWait(&mPriData->mSemSetSpeed, -1);

    return mPriData->mSetSpeedReply;
}
#endif

int XPlayerIsPlaying(XPlayer* p)
{
    logi("isPlaying");
    PlayerContext* mPriData = (PlayerContext*)p;
    if(mPriData->mStatus == XPLAYER_STATUS_STARTED ||
        (mPriData->mStatus == XPLAYER_STATUS_COMPLETE && mPriData->mLoop != 0))
        return 1;

    return 0;
}

MediaInfo* XPlayerGetMediaInfo(XPlayer* p)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    if(mPriData->mStatus == XPLAYER_STATUS_PREPARING ||
       mPriData->mStatus == XPLAYER_STATUS_INITIALIZED||
       mPriData->mStatus == XPLAYER_STATUS_IDLE )
    {
        loge("cannot get mediainfo in this status");
        return NULL;
    }

    return mPriData->mMediaInfo;
}

int XPlayerGetCurrentPosition(XPlayer* p, int* msec)
{
    int64_t nPositionUs;
    PlayerContext* mPriData = (PlayerContext*)p;

    logv("getCurrentPosition");

    if(mPriData->mStatus == XPLAYER_STATUS_PREPARED ||
       mPriData->mStatus == XPLAYER_STATUS_STARTED  ||
       mPriData->mStatus == XPLAYER_STATUS_PAUSED   ||
       mPriData->mStatus == XPLAYER_STATUS_COMPLETE)
    {
        if(mPriData->mSeeking != 0)
        {
            *msec = mPriData->mSeekTime;
            return 0;
        }

        mPriData->mLivemode = DemuxCompGetLiveMode(mPriData->mDemux);
        if(mPriData->mLivemode == 1)
        {
            if(mPriData->mStatus == XPLAYER_STATUS_PAUSED)
            {
                struct timeval tv;
                gettimeofday(&tv, NULL);
                *msec = (tv.tv_sec * 1000000ll + tv.tv_usec - mPriData->mPauseTimeStamp)/1000;
            }
            else if(mPriData->mDemuxNotifyPause)
            {
                struct timeval tv;
                gettimeofday(&tv, NULL);
                *msec = (tv.tv_sec * 1000000ll + tv.tv_usec - mPriData->mDemuxPauseTimeStamp)/1000;
            }
            else
            {
                *msec = 0;
            }

            if(*msec < 0) *msec = 0;

            return 0;
        }
        else if(mPriData->mLivemode == 2)
        {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            if(mPriData->mStatus == XPLAYER_STATUS_PAUSED)
            {
                *msec = (tv.tv_sec * 1000000 + tv.tv_usec - mPriData->mPauseTimeStamp)/1000;
                logd("livemode2, nowUs = %ld, mPauseTimeStamp = %" PRId64 "",
                        tv.tv_sec * 1000000 + tv.tv_usec, mPriData->mPauseTimeStamp);
            }
            else
            {
                //cmcc livemode2 only support ts
                nPositionUs = PlayerGetPositionCMCC(mPriData->mPlayer);
                logd("-- nPositionUs = %" PRId64 "", nPositionUs);
                *msec = (tv.tv_sec * 1000000LL + tv.tv_usec -
                        mPriData->mShiftTimeStamp + nPositionUs)/1000;
            }

            if(*msec < 0)
            {
                logd("positon < 0 ,check it!!");
                *msec = 0;
            }
            logd("livemode = 2, getCurrentPosition %d", *msec);

            return 0;
        }

        //* in complete status, the prepare() method maybe called and it change the media info.
        pthread_mutex_lock(&mPriData->mMutexMediaInfo);
        if(mPriData->mMediaInfo != NULL)
        {
            //* ts stream's pts is not started at 0.
            if(mPriData->mMediaInfo->eContainerType == CONTAINER_TYPE_TS ||
               mPriData->mMediaInfo->eContainerType == CONTAINER_TYPE_BD ||
               mPriData->mMediaInfo->eContainerType == CONTAINER_TYPE_HLS)
                nPositionUs = PlayerGetPosition(mPriData->mPlayer);
            else  //* generally, stream pts is started at 0 except ts stream.
                nPositionUs = PlayerGetPts(mPriData->mPlayer);
            *msec = (nPositionUs + 500)/1000;
            pthread_mutex_unlock(&mPriData->mMutexMediaInfo);

            return 0;
        }
        else
        {
            loge("getCurrentPosition() fail, mMediaInfo==NULL.");
            *msec = 0;
            pthread_mutex_unlock(&mPriData->mMutexMediaInfo);
            return 0;
        }
    }
    else
    {
        *msec = 0;

        if(mPriData->mStatus == XPLAYER_COMMAND_STOP && mPriData->mTimeMsBeforeStop >= 0)
            *msec = mPriData->mTimeMsBeforeStop;

        if(mPriData->mStatus == XPLAYER_STATUS_ERROR)
            return -1;
        else
            return 0;
    }

    return -1;
}


int XPlayerGetDuration(XPlayer* p, int *msec)
{
    logv("getDuration");
    PlayerContext* mPriData = (PlayerContext*)p;

    if(mPriData->mStatus == XPLAYER_STATUS_PREPARED ||
       mPriData->mStatus == XPLAYER_STATUS_STARTED  ||
       mPriData->mStatus == XPLAYER_STATUS_PAUSED   ||
       mPriData->mStatus == XPLAYER_STATUS_STOPPED  ||
       mPriData->mStatus == XPLAYER_STATUS_COMPLETE)
    {
        //* in complete status, the prepare() method maybe called and it change the media info.
        pthread_mutex_lock(&mPriData->mMutexMediaInfo);
        if(mPriData->mMediaInfo != NULL)
            *msec = mPriData->mMediaInfo->nDurationMs;
        else
        {
            loge("getCurrentPosition() fail, mPriData->mMediaInfo==NULL.");
            *msec = 0;
        }
        pthread_mutex_unlock(&mPriData->mMutexMediaInfo);
        return 0;
    }

    loge("invalid getDuration() call, player not in valid status.");
    return -1;
}


int XPlayerSetLooping(XPlayer* p, int loop)
{
    logd("setLooping");
    PlayerContext* mPriData = (PlayerContext*)p;

    if(mPriData->mStatus == XPLAYER_STATUS_ERROR)
        return -1;

    mPriData->mLoop = loop;
    return 0;
}

#ifndef ONLY_ENABLE_AUDIO
int updateVideoInfo(XPlayer* p)
{
    //* get media information.
    MediaInfo*          mi;
    VideoStreamInfo*    vi;
    int                 ret;
    PlayerContext* mPriData = (PlayerContext*)p;

    pthread_mutex_lock(&mPriData->mMutexMediaInfo);

    mi = DemuxCompGetMediaInfo(mPriData->mDemux);
    if(mi == NULL)
    {
        loge("can not get media info from demux.");
        pthread_mutex_unlock(&mPriData->mMutexMediaInfo);
        return -1;
    }
    clearMediaInfo(p);
    mPriData->mMediaInfo = mi;

#if !AWPLAYER_CONFIG_DISABLE_VIDEO
    if(mi->pVideoStreamInfo != NULL)
    {
#if !defined(CONF_3D_ENABLE)
            if(mi->pVideoStreamInfo->bIs3DStream == 1)
                mi->pVideoStreamInfo->bIs3DStream = 0;
#endif

        ret = PlayerSetVideoStreamInfo(mPriData->mPlayer, mi->pVideoStreamInfo);
        if(ret != 0)
        {
            logw("PlayerSetVideoStreamInfo() fail, video stream not supported.");
        }
    }
#endif

    pthread_mutex_unlock(&mPriData->mMutexMediaInfo);
    return 0;
}
#endif

int updateAudioInfo(XPlayer* p)
{
    //* get media information.
    MediaInfo*          mi;
    AudioStreamInfo*    vi;
    int                 ret;
    PlayerContext* mPriData = (PlayerContext*)p;

    pthread_mutex_lock(&mPriData->mMutexMediaInfo);

    mi = DemuxCompGetMediaInfo(mPriData->mDemux);
    if(mi == NULL)
    {
        loge("can not get media info from demux.");
        pthread_mutex_unlock(&mPriData->mMutexMediaInfo);
        return -1;
    }
    clearMediaInfo(p);
    mPriData->mMediaInfo = mi;

#if !AWPLAYER_CONFIG_DISABLE_AUDIO
    if(mi->pAudioStreamInfo != NULL)
    {
        int                 i;
        int                 nDefaultAudioIndex;

        nDefaultAudioIndex = -1;
        for(i=0; i<mi->nAudioStreamNum; i++)
        {
            if(PlayerCanSupportAudioStream(mPriData->mPlayer, &mi->pAudioStreamInfo[i]))
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

        //* 1. stop audio
        PlayerStopAudio(mPriData->mPlayer);
        //* 2. set stream info
        ret = PlayerSetAudioStreamInfo(mPriData->mPlayer, mi->pAudioStreamInfo,
                                        mi->nAudioStreamNum, nDefaultAudioIndex);
        if(ret != 0)
        {
            logw("PlayerSetAudioStreamInfo() fail, audio stream not supported.");
        }
    }
#endif
    pthread_mutex_unlock(&mPriData->mMutexMediaInfo);
    return 0;
}

#ifndef ONLY_ENABLE_AUDIO
/*
 *ret 1 means need to scale down
 *    0 is do nothing
 *    -1 is unknown error
 * */
/*A80              general 4K -> do nothing (0)*/
/*H3/H8            general 4K -> Scale down (1)*/
/*H3            H265 4K    -> do nothing (0)*/
int getScaledownFlag(const MediaInfo *mi,int bSetScaleDown)
{
    int ret = 0; /* default not scaledown */

    if(bSetScaleDown == 1)
    {
        if(mi->pVideoStreamInfo->nWidth > WIDTH_1080P || mi->pVideoStreamInfo->nHeight > HEIGHT_1080P)
        {
            logd("###source file larger than 2k,scale down.");
            ret = 1;
        }
    }
    else if(bSetScaleDown == 2)
    {
        logd("set scale down in any resolution.");
        ret = 1;
    }

    if(mi->pVideoStreamInfo->nWidth >= WIDTH_4K || mi->pVideoStreamInfo->nHeight >= HEIGHT_4K)
    {
        if (mi->pVideoStreamInfo->eCodecFormat == VIDEO_CODEC_FORMAT_H265)
        {
#if !defined(CONF_H265_4K_P2P) /* only H3/H5 support H.265 p2p */
            ret = 1;
#endif
        }
        else if (mi->pVideoStreamInfo->eCodecFormat == VIDEO_CODEC_FORMAT_H264)
        {
#if !defined(CONF_H264_4K_P2P)  /* only A80 support H.264 p2p */
            ret = 1;
#endif
        }
        else if (mi->pVideoStreamInfo->eCodecFormat == VIDEO_CODEC_FORMAT_VP9)
        {
            ret = 0;
        }
        else /* all other codec format should scaledown for 4K size */
        {
            ret = 1;
        }
    }

    return ret;
}

int XPlayerSetScaleDownRatio(XPlayer* p,int widthRatio,int heightRatio){
    PlayerContext* mPriData = (PlayerContext*)p;
    if((mPriData != NULL)&& (mPriData->mStatus==XPLAYER_STATUS_IDLE
        || mPriData->mStatus==XPLAYER_STATUS_INITIALIZED
        || mPriData->mStatus==XPLAYER_STATUS_PREPARING) ){
        mPriData->mWidthScaledownRatio = widthRatio;
        mPriData->mHeightScaledownRatio = heightRatio;
        return 0;
    }else{
        loge("XPlayerSetScaleDownRatio err,mPriData->mStatus = %d",mPriData->mStatus);
        return -1;
    }
}

/*
 *ret 0 means supported, -1 is unsupported.
 * */
int checkVideoSupported(const MediaInfo *mi)
{
    int ret = 0;    //default is supported

    /*A80/H8          H265 4K unsupported*/
    /*H3/H5            H265 4K supported*/
    if (mi->pVideoStreamInfo->nWidth * mi->pVideoStreamInfo->nHeight >= PIXELS_4K)
    {
#if !defined(CONF_H265_4K)
        if (mi->pVideoStreamInfo->eCodecFormat == VIDEO_CODEC_FORMAT_H265)
        {
            loge("Not support H265 4K video !!");
            ret = -1;
        }
#endif
        if (mi->pVideoStreamInfo->eCodecFormat == VIDEO_CODEC_FORMAT_WMV3)
        {
            loge("Not support VC1(WMV3) 4K video !!");
            ret = -1;
        }
    }
    else if (mi->pVideoStreamInfo->nWidth > WIDTH_1080P ||
            mi->pVideoStreamInfo->nHeight > HEIGHT_1080P)
    {
        /* for all chip, WMV1/WMV2/VP6 specs unsupported,  play effect too bad... */
        if (mi->pVideoStreamInfo->eCodecFormat == VIDEO_CODEC_FORMAT_WMV1
                || mi->pVideoStreamInfo->eCodecFormat == VIDEO_CODEC_FORMAT_WMV2
                || mi->pVideoStreamInfo->eCodecFormat == VIDEO_CODEC_FORMAT_VP6)
        {
            loge("Not support WMV1/WMV2/VP6 1080P video !!");
            ret = -1;
        }
    }
    else /* < 1080P */
    {
        /* we support this*/
    }

    logv("check video support ret = [%d]", ret);
    return ret;
}
#endif

static int initializePlayer(XPlayer* p)
{
    //* get media information.
    MediaInfo*          mi;
#ifndef ONLY_ENABLE_AUDIO
    VideoStreamInfo*    vi;
#endif
    AudioStreamInfo*    ai;
#ifndef ONLY_ENABLE_AUDIO
    SubtitleStreamInfo* si;
#endif
    int                 i;
    int                 nDefaultAudioIndex;
#ifndef ONLY_ENABLE_AUDIO
    int                 nDefaultSubtitleIndex;
#endif
    int                 ret;
    PlayerContext* mPriData = (PlayerContext*)p;

    pthread_mutex_lock(&mPriData->mMutexMediaInfo);

    mi = DemuxCompGetMediaInfo(mPriData->mDemux);
    if(mi == NULL)
    {
        loge("can not get media info from demux.");
        pthread_mutex_unlock(&mPriData->mMutexMediaInfo);
        return -1;
    }
#ifndef ONLY_ENABLE_AUDIO
    if(mi->nBitrate > 0)
        PlayerSetVideoAvgByteRate(mPriData->mPlayer, mi->nBitrate/8);
#endif
    PlayerSetFirstPts(mPriData->mPlayer,mi->nFirstPts);
    mPriData->mMediaInfo = mi;
#ifndef ONLY_ENABLE_AUDIO
    if(mi->pVideoStreamInfo != NULL)
    {
        /*detect if support*/
        if(checkVideoSupported(mi))
        {
            loge("this video is outof specs, unsupported.");
            return -1;
        }
        if(mPriData->mWidthScaledownRatio == 0 && mPriData->mHeightScaledownRatio == 0){
            /*if the app not set scaledownRatio,use the default*/
	        /*check if need scaledown*/
	        if(getScaledownFlag(mi,mPriData->bSetScaleDown)==1) /*1 means need to scale down*/
	        {
	            ret = PlayerConfigVideoScaleDownRatio(mPriData->mPlayer, 1, 1);
	            if(ret != 0)
	            {
	                logw("PlayerConfigVideoScaleDownRatio() fail, ret = %d",ret);
	            }
	            else
	                mPriData->mScaledownFlag = 1;
	        }
        }else{
            ret = PlayerConfigVideoScaleDownRatio(mPriData->mPlayer,mPriData->mWidthScaledownRatio,mPriData->mHeightScaledownRatio);
            if(ret != 0)
            {
                logw("PlayerConfigVideoScaleDownRatio() fail, ret = %d",ret);
            }
            else
                mPriData->mScaledownFlag = 1;
        }
    }

    //* initialize the player.
#if !AWPLAYER_CONFIG_DISABLE_VIDEO
    if(mi->pVideoStreamInfo != NULL)
    {
#if !defined(CONF_3D_ENABLE)
            if(mi->pVideoStreamInfo->bIs3DStream == 1)
                mi->pVideoStreamInfo->bIs3DStream = 0;
#endif
        //* set the rotation
        int nRotateDegree;
        int nRotation = atoi((const char*)mPriData->mMediaInfo->cRotation);

        if(nRotation == 0)
            nRotateDegree = 0;
        else if(nRotation == 90)
            nRotateDegree = 1;
        else if(nRotation == 180)
            nRotateDegree = 2;
        else if(nRotation == 270)
            nRotateDegree = 3;
        else
            nRotateDegree = 0;
        /*if the rotate degree is set by app,use this degree instead*/
        logd("mVideoRotateDegree = %d",mPriData->mVideoRotateDegree);
        if(mPriData->mVideoRotateDegree == 0)
            nRotateDegree = 0;
        else if(mPriData->mVideoRotateDegree == 90)
            nRotateDegree = 1;
        else if(mPriData->mVideoRotateDegree == 180)
            nRotateDegree = 2;
        else if(mPriData->mVideoRotateDegree == 270)
            nRotateDegree = 3;
        logd("rotate degree level = %d",nRotateDegree);
        ret = PlayerConfigVideoRotateDegree(mPriData->mPlayer, nRotateDegree);
        if(ret != 0)
        {
            logw("PlayerConfigVideoRotateDegree() fail, ret = %d",ret);
        }

        //* set the video streamInfo
        ret = PlayerSetVideoStreamInfo(mPriData->mPlayer, mi->pVideoStreamInfo);
        if(ret != 0)
        {
            logw("PlayerSetVideoStreamInfo() fail, video stream not supported.");
        }

        ret = PlayerConfigDropDelayFrame(mPriData->mPlayer, 1);
        if(ret != 0)
        {
            logw("PlayerConfigDropDelayFrame() fail.");
        }
    }

#endif

#endif

#if !AWPLAYER_CONFIG_DISABLE_AUDIO
    if(mi->pAudioStreamInfo != NULL)
    {
        nDefaultAudioIndex = -1;
        for(i=0; i<mi->nAudioStreamNum; i++)
        {
            if(PlayerCanSupportAudioStream(mPriData->mPlayer, &mi->pAudioStreamInfo[i]))
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

        ret = PlayerSetAudioStreamInfo(mPriData->mPlayer, mi->pAudioStreamInfo,
                                        mi->nAudioStreamNum, nDefaultAudioIndex);
        if(ret != 0)
        {
            logw("PlayerSetAudioStreamInfo() fail, audio stream not supported.");
        }
    }
#endif

#ifndef ONLY_ENABLE_AUDIO
    if(PlayerHasVideo(mPriData->mPlayer) == 0 && PlayerHasAudio(mPriData->mPlayer) == 0)
    {
        loge("neither video nor audio stream can be played.");
        pthread_mutex_unlock(&mPriData->mMutexMediaInfo);
        return -1;
    }
#else
    if(PlayerHasAudio(mPriData->mPlayer) == 0)
    {
        loge("audio stream can not be played.");
        pthread_mutex_unlock(&mPriData->mMutexMediaInfo);
        return -1;
    }
#endif

#ifndef ONLY_ENABLE_AUDIO
#if !AWPLAYER_CONFIG_DISABLE_SUBTITLE
    //* set subtitle stream to the text decoder.
    if(mi->pSubtitleStreamInfo != NULL)
    {
        nDefaultSubtitleIndex = -1;
        for(i=0; i<mi->nSubtitleStreamNum; i++)
        {
            if(PlayerCanSupportSubtitleStream(mPriData->mPlayer, &mi->pSubtitleStreamInfo[i]))
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

        ret = PlayerSetSubtitleStreamInfo(mPriData->mPlayer,
                    mi->pSubtitleStreamInfo, mi->nSubtitleStreamNum, nDefaultSubtitleIndex);
        if(ret != 0)
        {
            logw("PlayerSetSubtitleStreamInfo() fail, subtitle stream not supported.");
        }
    }
#endif
#endif
    //* report not seekable.
    if(mi->bSeekable == 0)
        mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_INFO,
                        AW_MEDIA_INFO_NOT_SEEKABLE, 0);

    pthread_mutex_unlock(&mPriData->mMutexMediaInfo);

    return 0;
}

static void clearMediaInfo(XPlayer* p)
{
    int                 i;
#ifndef ONLY_ENABLE_AUDIO
    VideoStreamInfo*    v;
#endif
    AudioStreamInfo*    a;
#ifndef ONLY_ENABLE_AUDIO
    SubtitleStreamInfo* s;
#endif
    PlayerContext* mPriData = (PlayerContext*)p;

    if(mPriData->mMediaInfo != NULL)
    {
#ifndef ONLY_ENABLE_AUDIO
        //* free video stream info.
        if(mPriData->mMediaInfo->pVideoStreamInfo != NULL)
        {
            for(i=0; i<mPriData->mMediaInfo->nVideoStreamNum; i++)
            {
                v = &mPriData->mMediaInfo->pVideoStreamInfo[i];
                if(v->pCodecSpecificData != NULL && v->nCodecSpecificDataLen > 0)
                    free(v->pCodecSpecificData);
            }
            free(mPriData->mMediaInfo->pVideoStreamInfo);
            mPriData->mMediaInfo->pVideoStreamInfo = NULL;
        }
#endif
        //* free audio stream info.
        if(mPriData->mMediaInfo->pAudioStreamInfo != NULL)
        {
            for(i=0; i<mPriData->mMediaInfo->nAudioStreamNum; i++)
            {
                a = &mPriData->mMediaInfo->pAudioStreamInfo[i];
                if(a->pCodecSpecificData != NULL && a->nCodecSpecificDataLen > 0)
                    free(a->pCodecSpecificData);
            }
            free(mPriData->mMediaInfo->pAudioStreamInfo);
            mPriData->mMediaInfo->pAudioStreamInfo = NULL;
        }
#ifndef ONLY_ENABLE_AUDIO
        //* free subtitle stream info.
        if(mPriData->mMediaInfo->pSubtitleStreamInfo != NULL)
        {
            for(i=0; i<mPriData->mMediaInfo->nSubtitleStreamNum; i++)
            {
                s = &mPriData->mMediaInfo->pSubtitleStreamInfo[i];
                if(s->pUrl != NULL)
                {
                    free(s->pUrl);
                    s->pUrl = NULL;
                }
                if(s->fd >= 0)
                {
                    close(s->fd);
                    s->fd = -1;
                }
                if(s->fdSub >= 0)
                {
                    close(s->fdSub);
                    s->fdSub = -1;
                }
            }
            free(mPriData->mMediaInfo->pSubtitleStreamInfo);
            mPriData->mMediaInfo->pSubtitleStreamInfo = NULL;
        }
#endif
        //* free the media info.
        free(mPriData->mMediaInfo);
        mPriData->mMediaInfo = NULL;
    }

    return;
}

#ifndef ONLY_ENABLE_AUDIO
static void* sendSpeedCmdThread(void* arg)
{
    PlayerContext* mPriData = (PlayerContext*)arg;
    mPriData->mSpeedThreadRuning = 1;
    int needPauseNext =0;
    logd("sendSpeedCmdThread start");

    while(mPriData->mSpeedThreadRuning)
    {
        if (mPriData->mbFast == 1)
        {
            sleep(1);

            if (mPriData->mSpeedThreadRuning != 1)
                break;

            if(mPriData->mStatus == XPLAYER_STATUS_PAUSED ||
               mPriData->mStatus == XPLAYER_STATUS_STOPPED ||
               mPriData->mStatus == XPLAYER_STATUS_COMPLETE ||
               mPriData->mStatus == XPLAYER_STATUS_ERROR ||
               mPriData->mStatus == XPLAYER_STATUS_IDLE)
            {
                mPriData->mbFast = 0;
                continue;
            }

            if ( needPauseNext == 1 || mPriData->mSpeed == 0 )
            {
                needPauseNext = 0;
                AwMessage msg;
                //* send a pause message.
                memset(&msg, 0, sizeof(AwMessage));
                msg.messageId = XPLAYER_COMMAND_PAUSE;
                msg.params[0] = (uintptr_t)&mPriData->mSemPause;
                msg.params[1] = (uintptr_t)&mPriData->mPauseReply;
                AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
                continue;
            }

            mPriData->mFastTime += mPriData->mSpeed*1000;
            if (mPriData->mFastTime < 0)
            {
                mPriData->mFastTime = 0;
                needPauseNext = 1;
            }
            else
                needPauseNext = 0;

            if(mPriData->mFastTime >= 0 && mPriData->mbFast == 1 )
            {
                AwMessage msg;
                //* send a seek message.
                memset(&msg, 0, sizeof(AwMessage));
                msg.messageId = XPLAYER_COMMAND_SEEK;
                msg.params[0] = (uintptr_t)&mPriData->mSemSeek;
                msg.params[1] = (uintptr_t)&mPriData->mSeekReply;
                msg.params[2] = mPriData->mFastTime;     //* params[2] = mSeekTime.
                msg.params[3] = 0;                      //* params[3] = mSeekSync.
                logd("speed mode seek, mFastTime: %d, mSpeed: %d",
                     mPriData->mFastTime, mPriData->mSpeed);
                AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
            }

            if(mPriData->mStatus == XPLAYER_STATUS_PAUSED ||
               mPriData->mStatus == XPLAYER_STATUS_STOPPED ||
               mPriData->mStatus == XPLAYER_STATUS_COMPLETE ||
               mPriData->mStatus == XPLAYER_STATUS_ERROR ||
               mPriData->mStatus == XPLAYER_STATUS_IDLE )
            {
                mPriData->mbFast = 0;
                continue;
            }
        }
        else
        {
            usleep(1000);
        }
    }
    return 0;
}
#endif

static void* XPlayerThread(void* arg)
{
    AwMessage            msg;
    int                  ret;
    sem_t*               pReplySem;
    int*                 pReplyValue;
    PlayerContext* mPriData = (PlayerContext*)arg;

    while(1)
    {
        if(AwMessageQueueGetMessage(mPriData->mMessageQueue, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }

        pReplySem   = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == XPLAYER_COMMAND_SET_SOURCE)
        {
            logi("process message XPLAYER_COMMAND_SET_SOURCE.");
            //* check status.
            if(mPriData->mStatus != XPLAYER_STATUS_IDLE &&
                    mPriData->mStatus != XPLAYER_STATUS_INITIALIZED)
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
                CdxKeyedVectorT* pHeaders;
                //* data source is a url string.
                if(mPriData->mSourceUrl != NULL)
                    free(mPriData->mSourceUrl);
                mPriData->mSourceUrl = strdup((char*)msg.params[3]);
                pHeaders   = (CdxKeyedVectorT*) msg.params[4];

                ret = DemuxCompSetUrlSource(mPriData->mDemux,
                                (void*)msg.params[5], mPriData->mSourceUrl, pHeaders);
                if(ret == 0)
                {
                    mPriData->mStatus = XPLAYER_STATUS_INITIALIZED;
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                }
                else
                {
                    loge("DemuxCompSetUrlSource() return fail.");
                    mPriData->mStatus = XPLAYER_STATUS_IDLE;
                    free(mPriData->mSourceUrl);
                    mPriData->mSourceUrl = NULL;
                    if(pReplyValue != NULL)
                        *pReplyValue = -1;
                }
            }
            else if((int)msg.params[2] == SOURCE_TYPE_FD)
            {
                //* data source is a file descriptor.
                int     fd;
                int64_t nOffset;
                int64_t nLength;

                fd = msg.params[3];
                nOffset = msg.params[4];
                nOffset<<=32;
                nOffset |= msg.params[5];
                nLength = msg.params[6];
                nLength<<=32;
                nLength |= msg.params[7];

                if(mPriData->mSourceFd != -1)
                    close(mPriData->mSourceFd);

                mPriData->mSourceFd       = dup(fd);
                mPriData->mSourceFdOffset = nOffset;
                mPriData->mSourceFdLength = nLength;
                ret = DemuxCompSetFdSource(mPriData->mDemux,
                                mPriData->mSourceFd, mPriData->mSourceFdOffset,
                                mPriData->mSourceFdLength);
                if(ret == 0)
                {
                    mPriData->mStatus = XPLAYER_STATUS_INITIALIZED;
                    if(pReplyValue != NULL)
                        *pReplyValue = (int)0;
                }
                else
                {
                    loge("DemuxCompSetFdSource() return fail.");
                    mPriData->mStatus = XPLAYER_STATUS_IDLE;
                    close(mPriData->mSourceFd);
                    mPriData->mSourceFd = -1;
                    mPriData->mSourceFdOffset = 0;
                    mPriData->mSourceFdLength = 0;
                    if(pReplyValue != NULL)
                        *pReplyValue = (int)-1;
                }
            }
            else if((int)msg.params[2] == SOURCE_TYPE_DATASOURCE)
            {
                //* data source is a mediadatasource interface.
                char *uri = (char *)msg.params[3];
                int ret;
                void *handle;
                ret = sscanf(uri, "datasource://%p", &handle);
                if (ret != 1)
                {
                    CDX_LOGE("sscanf failure...(%s)", uri);
                    mPriData->mSourceStream = NULL;
                }
                else
                {
                    mPriData->mSourceStream = (CdxStreamT *)handle;
                }
                ret = DemuxCompSetMediaDataSource(mPriData->mDemux, uri);
                if(ret == 0)
                {
                    mPriData->mStatus = XPLAYER_STATUS_INITIALIZED;
                    if(pReplyValue != NULL)
                        *pReplyValue = (int)0;
                }
                else
                {
                    loge("DemuxCompSetStreamSource() return fail.");
                    mPriData->mStatus = XPLAYER_STATUS_IDLE;
                    if(mPriData->mSourceStream)
                    {
                        CdxStreamClose(mPriData->mSourceStream);
                        mPriData->mSourceStream = NULL;
                    }
                    if(pReplyValue != NULL)
                        *pReplyValue = (int)-1;
                }
            }
            else
            {
                //* data source is a IStreamSource interface.
                char *uri = (char *)msg.params[3];
                int ret;
                void *handle;
                ret = sscanf(uri, "customer://%p", &handle);
                if (ret != 1)
                {
                    CDX_LOGE("sscanf failure...(%s)", uri);
                    mPriData->mSourceStream = NULL;
                }
                else
                {
                    mPriData->mSourceStream = (CdxStreamT *)handle;
                }
                ret = DemuxCompSetStreamSource(mPriData->mDemux, uri);
                if(ret == 0)
                {
                    mPriData->mStatus = XPLAYER_STATUS_INITIALIZED;
                    if(pReplyValue != NULL)
                        *pReplyValue = (int)0;
                }
                else
                {
                    loge("DemuxCompSetStreamSource() return fail.");
                    mPriData->mStatus = XPLAYER_STATUS_IDLE;
                    if(mPriData->mSourceStream)
                    {
                        CdxStreamClose(mPriData->mSourceStream);
                        mPriData->mSourceStream = NULL;
                    }
                    if(pReplyValue != NULL)
                        *pReplyValue = (int)-1;
                }
            }

            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end XPLAYER_COMMAND_SET_SOURCE.
#ifndef ONLY_ENABLE_AUDIO
        else if(msg.messageId == XPLAYER_COMMAND_SET_SURFACE)
        {
            LayerCtrl*   lc;

            logd("process message XPLAYER_COMMAND_SET_SURFACE.");

            //* set native window before delete the old one.
            //* because the player's render thread may use the old surface
            //* before it receive the new surface.
            lc = (LayerCtrl*)msg.params[2];

            ret = PlayerSetWindow(mPriData->mPlayer, lc);
            if(ret == 0)
            {
                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
            }
            else
            {
                loge("PlayerSetWindow() return fail.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)-1;
            }

            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end XPLAYER_COMMAND_SET_SURFACE.
#endif
        else if(msg.messageId == XPLAYER_COMMAND_SET_AUDIOSINK)
        {
            void* audioSink;

            logv("process message XPLAYER_COMMAND_SET_AUDIOSINK.");

            audioSink = (SoundCtrl*)msg.params[2];
            PlayerSetAudioSink(mPriData->mPlayer, audioSink);

            //* super class MediaPlayerInterface has mAudioSink.
            //MediaPlayerInterface::setAudioSink(audioSink);

            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end XPLAYER_COMMAND_SET_AUDIOSINK.
        else if (msg.messageId == XPLAYER_COMMAND_SET_PLAYRATE)
        {
            XAudioPlaybackRate *rate;
            rate = (XAudioPlaybackRate*)msg.params[2];

            //do set playback setting in playback
            int ret = PlayerSetPlayBackSettings(mPriData->mPlayer,rate);

            if(pReplyValue != NULL)
                *pReplyValue = ret;
            if(pReplySem != NULL)
                sem_post(pReplySem);

            continue;
        }//* end XPLAYER_COMMAND_SET_PLAYRATE.
#ifndef ONLY_ENABLE_AUDIO
        else if(msg.messageId == XPLAYER_COMMAND_SET_SUBCTRL)
        {
            SubCtrl* subctrl;

            logd("==== process message XPLAYER_COMMAND_SET_SUBCTRL.");

            subctrl = (SubCtrl*)msg.params[2];
            PlayerSetSubCtrl(mPriData->mPlayer, subctrl);

            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end XPLAYER_COMMAND_SET_SUBCTRL.
        else if(msg.messageId == XPLAYER_COMMAND_SET_DI)
        {
            Deinterlace* di;

            logd("==== process message XPLAYER_COMMAND_SET_SUBCTRL.");

            di = (Deinterlace*)msg.params[2];
            PlayerSetDeinterlace(mPriData->mPlayer, di);

            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end XPLAYER_COMMAND_SET_DI
#endif
        else if(msg.messageId == XPLAYER_COMMAND_PREPARE)
        {
            logd("process message XPLAYER_COMMAND_PREPARE. mPriData->mStatus: %d",
                  mPriData->mStatus);
            if(mPriData->mStatus == XPLAYER_STATUS_PREPARED)
            {
                //* for data source set by fd(file descriptor), the prepare() method
                //* is called in setDataSource(), so the player is already in PREPARED
                //* status, here we just notify a prepared message.
#ifndef ONLY_ENABLE_AUDIO
                //* when app call prepareAsync(), we callback video-size to app here,
                if(mPriData->mMediaInfo->pVideoStreamInfo != NULL)
                {
                    if(mPriData->mMediaInfo->pVideoStreamInfo->nWidth !=
                                mPriData->mVideoSizeWidth
                       && mPriData->mMediaInfo->pVideoStreamInfo->nHeight !=
                                mPriData->mVideoSizeHeight)
                    {
                        int nRotation;

                        nRotation = atoi((const char*)mPriData->mMediaInfo->cRotation);
                        if((nRotation%180)==0)//* when the rotation is 0 and 180
                        {
                            mPriData->mVideoSizeWidth  =
                                mPriData->mMediaInfo->pVideoStreamInfo->nWidth;
                            mPriData->mVideoSizeHeight =
                                mPriData->mMediaInfo->pVideoStreamInfo->nHeight;
                        }
                        else
                        {
                            //* when the rotation is 90 and 270,
                            //* we should exchange nHeight and nwidth
                            mPriData->mVideoSizeWidth  =
                                mPriData->mMediaInfo->pVideoStreamInfo->nHeight;
                            mPriData->mVideoSizeHeight =
                                mPriData->mMediaInfo->pVideoStreamInfo->nWidth;
                        }

                        logi("xxxxxxxxxx video size: width = %d, height = %d",
                                mPriData->mVideoSizeWidth, mPriData->mVideoSizeHeight);

                        int size[2];
                        size[0] = mPriData->mVideoSizeWidth;
                        size[1] = mPriData->mVideoSizeHeight;
                        mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_SET_VIDEO_SIZE,
                                    0,
                                    size);
                    }
                }
#endif
                mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_PREPARED, 0, 0);

                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                if(1 == msg.params[2]){
                    logd("let XPlayerPrepare return");
                    mPriData->mPrepareFinishResult = 0;
                    sem_post(&mPriData->mSemPrepareFinish);
                }
                continue;
            }

            if(mPriData->mStatus != XPLAYER_STATUS_INITIALIZED &&
                    mPriData->mStatus != XPLAYER_STATUS_STOPPED)
            {
                logd("invalid prepareAsync() call, player not in initialized or stopped status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)-1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            mPriData->mStatus = XPLAYER_STATUS_PREPARING;
            mPriData->mPrepareSync = msg.params[2];
            ret = DemuxCompPrepareAsync(mPriData->mDemux);
            if(ret != 0)
            {
                loge("DemuxCompPrepareAsync return fail immediately.");
                mPriData->mStatus = XPLAYER_STATUS_IDLE;
                if(pReplyValue != NULL)
                    *pReplyValue = (int)-1;
            }
            else
            {
                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
            }

            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end XPLAYER_COMMAND_PREPARE.
#ifndef ONLY_ENABLE_AUDIO
        else if(msg.messageId == XPLAYER_COMMAND_SETSPEED)
        {
            logd("process message XPLAYER_COMMAND_SETSPEED.");
            if(mPriData->mStatus != XPLAYER_STATUS_PREPARED &&
               mPriData->mStatus != XPLAYER_STATUS_STARTED )
            {
                logd("invalid start() call, player not in prepared, \
                        started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)-1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            if(mPriData->mMediaInfo == NULL || mPriData->mMediaInfo->bSeekable == 0)
            {
                if(mPriData->mMediaInfo == NULL)
                {
                    loge("setspeed fail because mMediaInfo == NULL.");
                    if(pReplyValue != NULL)
                        *pReplyValue = -1;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;
                }
                else
                {
                    loge("media not seekable. cannot fast");
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;
                }
            }

            if(mPriData->mSeeking)
            {
                DemuxCompCancelSeek(mPriData->mDemux);
                mPriData->mSeeking = 0;
            }

            //* protect mSeeking and mSeekTime from being changed by the seek finish callback.
            pthread_mutex_lock(&mPriData->mMutexStatus);

            int curTime;
            XPlayerGetCurrentPosition(mPriData, &curTime);

            mPriData->mSeeking  = 1;
            mPriData->mbFast = 1;
            mPriData->mFastTime = curTime + mPriData->mSpeed*1000;
            mPriData->mSeekTime = mPriData->mFastTime;
            mPriData->mSeekSync = 0;
            pthread_mutex_unlock(&mPriData->mMutexStatus);

            //PlayerFastForeward(mPriData->mPlayer);
            PlayerSetDiscardAudio(mPriData->mPlayer, 1);

            if(PlayerGetStatus(mPriData->mPlayer) == PLAYER_STATUS_STOPPED)
            {
                //* if in prepared status, the player is in stopped status,
                //* this will make the player not record the nSeekTime at PlayerReset() operation
                //* called at seek finish callback.
                PlayerStart(mPriData->mPlayer,0,0);
            }

            PlayerPause(mPriData->mPlayer);
            DemuxCompSeekTo(mPriData->mDemux, mPriData->mSeekTime, mPriData->mSeekModeType);

            if (mPriData->mSpeedThreadRuning == 0)
            {
                int ret = pthread_create(&mPriData->mSpeedThreadId, NULL,
                    sendSpeedCmdThread, (void*)mPriData );
                if (ret != 0)
                {
                    loge("create sendSpeedCmdThread error!");
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;
                }
            }

            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        }//* end XPLAYER_COMMAND_SETSPEED.
#endif
        else if(msg.messageId == XPLAYER_COMMAND_START)
        {
            logd("process message XPLAYER_COMMAND_START.");
            if(mPriData->mStatus != XPLAYER_STATUS_PREPARED &&
               mPriData->mStatus != XPLAYER_STATUS_STARTED  &&
               mPriData->mStatus != XPLAYER_STATUS_PAUSED   &&
               mPriData->mStatus != XPLAYER_STATUS_COMPLETE)
            {
                logd("invalid start() call, player not in prepared, \
                    started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)-1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }
#ifndef ONLY_ENABLE_AUDIO
            if(mPriData->mbFast == 1)
            {
                PlayerSetDiscardAudio(mPriData->mPlayer, 0);
                mPriData->mbFast = 0;
                mPriData->mSpeed = 1;
                mPriData->mFastTime = 0;
            }
#endif
            //* synchronize with the seek or complete callback and the status may be changed.
            pthread_mutex_lock(&mPriData->mMutexStatus);

            if(mPriData->mStatus == XPLAYER_STATUS_STARTED)
            {
                if(PlayerGetStatus(mPriData->mPlayer) == PLAYER_STATUS_PAUSED
                        && mPriData->mSeeking == 0)
                {
                    //* player is paused for buffering, start it.
                    //* see XPLAYER_MESSAGE_DEMUX_PAUSE_PLAYER callback message.
                    PlayerStart(mPriData->mPlayer,0,0);
                    DemuxCompStart(mPriData->mDemux);
                }
                pthread_mutex_unlock(&mPriData->mMutexStatus);
                logv("player already in started status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            if(mPriData->mSeeking)
            {
                //* player and demux will be started at the seek callback.
                mPriData->mStatus = XPLAYER_STATUS_STARTED;
                pthread_mutex_unlock(&mPriData->mMutexStatus);

                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            //* for complete status, we seek to the begin of the file.
            if(mPriData->mStatus == XPLAYER_STATUS_COMPLETE)
            {
                AwMessage newMsg;

                if(mPriData->mMediaInfo->bSeekable)
                {
                    if(mPriData->mSeeking)
                    {
                        DemuxCompCancelSeek(mPriData->mDemux);
                        mPriData->mSeeking = 0;
                    }

                    mPriData->mSeeking  = 1;
                    mPriData->mSeekTime = 0;
                    mPriData->mSeekSync = 0;
                    mPriData->mSeekByInner = 1;
                    logd("seekTo %.2f secs", mPriData->mSeekTime / 1E3);

                    PlayerPause(mPriData->mPlayer);
                    DemuxCompSeekTo(mPriData->mDemux, mPriData->mSeekTime, mPriData->mSeekModeType);

                    //* player and demux will be started at the seek callback.
                    mPriData->mStatus = XPLAYER_STATUS_STARTED;
                    pthread_mutex_unlock(&mPriData->mMutexStatus);

                    if(pReplyValue != NULL)
                        *pReplyValue = (int)0;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;
                }
                else
                {
                    //* post a stop message.
                    memset(&newMsg, 0, sizeof(AwMessage));
                    newMsg.messageId = XPLAYER_COMMAND_STOP;
                    AwMessageQueuePostMessage(mPriData->mMessageQueue, &newMsg);

                    //* post a prepare message.
                    memset(&newMsg, 0, sizeof(AwMessage));
                    newMsg.messageId = XPLAYER_COMMAND_PREPARE;
                    newMsg.params[0] = 0;
                    newMsg.params[1] = 0;
                    newMsg.params[2] = 1;   //* params[2] = mPrepareSync.
                    AwMessageQueuePostMessage(mPriData->mMessageQueue, &newMsg);

                    //* post a start message.
                    memset(&newMsg, 0, sizeof(AwMessage));
                    newMsg.messageId = XPLAYER_COMMAND_START;
                    AwMessageQueuePostMessage(mPriData->mMessageQueue, &newMsg);

                    //* should I reply 0 to the user at this moment?
                    //* or just set the semaphore and reply variable to the start message to
                    //* make it reply when start message done?
                    pthread_mutex_unlock(&mPriData->mMutexStatus);
                    if(pReplyValue != NULL)
                        *pReplyValue = (int)0;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;
                }
            }

            pthread_mutex_unlock(&mPriData->mMutexStatus);

#ifndef ONLY_ENABLE_AUDIO
            if(mPriData->mApplicationType == APP_STREAMING)
            {
                PlayerFast(mPriData->mPlayer, 0);
            }
            else
#endif
            {
                PlayerStart(mPriData->mPlayer,0,0);
            }

            DemuxCompStart(mPriData->mDemux);
            mPriData->mStatus = XPLAYER_STATUS_STARTED;
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;

        } //* end XPLAYER_COMMAND_START.
        else if(msg.messageId == XPLAYER_COMMAND_STOP)
        {
            logv("process message XPLAYER_COMMAND_STOP.");
            if(mPriData->mStatus != XPLAYER_STATUS_PREPARED &&
               mPriData->mStatus != XPLAYER_STATUS_STARTED  &&
               mPriData->mStatus != XPLAYER_STATUS_PAUSED   &&
               mPriData->mStatus != XPLAYER_STATUS_COMPLETE &&
               mPriData->mStatus != XPLAYER_STATUS_STOPPED)
            {
                logd("invalid stop() call, player not in prepared, paused, \
                        started, stopped or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)-1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            if(mPriData->mStatus == XPLAYER_STATUS_STOPPED)
            {
                logv("player already in stopped status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            //* the prepare callback may happen at this moment.
            //* so the mStatus may be changed to PREPARED asynchronizely.
            if(mPriData->mStatus == XPLAYER_STATUS_PREPARING)
            {
                logw("stop() called at preparing status, cancel demux prepare.");
                DemuxCompCancelPrepare(mPriData->mDemux);
            }

            if(mPriData->mSeeking)
            {
                DemuxCompCancelSeek(mPriData->mDemux);
                mPriData->mSeeking = 0;
            }

            DemuxCompStop(mPriData->mDemux);
            PlayerStop(mPriData->mPlayer);
            PlayerClear(mPriData->mPlayer);               //* clear all media information in player.
#ifndef ONLY_ENABLE_AUDIO
            //*clear the mSubtitleDisplayIds
            memset(mPriData->mSubtitleDisplayIds,0xff,64*sizeof(unsigned int));
            mPriData->mSubtitleDisplayIdsUpdateIndex = 0;
#endif
            mPriData->mStatus  = XPLAYER_STATUS_STOPPED;
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end XPLAYER_COMMAND_STOP.
        else if(msg.messageId == XPLAYER_COMMAND_PAUSE)
        {
            logv("process message XPLAYER_COMMAND_PAUSE.");
            if(mPriData->mStatus != XPLAYER_STATUS_STARTED  &&
               mPriData->mStatus != XPLAYER_STATUS_PAUSED   &&
               mPriData->mStatus != XPLAYER_STATUS_COMPLETE)
            {
                logd("invalid pause() call, player not in started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)-1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            if(mPriData->mStatus == XPLAYER_STATUS_PAUSED)
            {
                logv("player already in paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            //* sync with the seek, complete or pause_player/resume_player call back.
            pthread_mutex_lock(&mPriData->mMutexStatus);

            if(mPriData->mSeeking)
            {
                //* player and demux will be paused at the seek callback.
                mPriData->mStatus = XPLAYER_STATUS_PAUSED;
                pthread_mutex_unlock(&mPriData->mMutexStatus);

                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            PlayerPause(mPriData->mPlayer);
            mPriData->mStatus = XPLAYER_STATUS_PAUSED;

            //* sync with the seek, complete or pause_player/resume_player call back.
            pthread_mutex_unlock(&mPriData->mMutexStatus);

            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end XPLAYER_COMMAND_PAUSE.
        else if(msg.messageId == XPLAYER_COMMAND_RESET)
        {
            logv("process message XPLAYER_COMMAND_RESET.");
             //* the prepare callback may happen at this moment.
            //* so the mStatus may be changed to PREPARED asynchronizely.
            if(mPriData->mStatus == XPLAYER_STATUS_PREPARING)
            {
                logw("reset() called at preparing status, cancel demux prepare.");
                DemuxCompCancelPrepare(mPriData->mDemux);
            }

            if(mPriData->mSeeking)
            {
                DemuxCompCancelSeek(mPriData->mDemux);
                mPriData->mSeeking = 0;
            }

            //* stop and clear the demux.
            //* this will stop the seeking if demux is currently processing seeking message.
            //* it will clear the data source keep inside, this is important for the IStreamSource.
            DemuxCompStop(mPriData->mDemux);
            DemuxCompClear(mPriData->mDemux);

            //* stop and clear the player.
            PlayerStop(mPriData->mPlayer);
            PlayerClear(mPriData->mPlayer);   //* it will clear media info config to the player.
#ifndef ONLY_ENABLE_AUDIO
            //* clear suface.
            if(mPriData->mKeepLastFrame == 0)
            {

            }
#endif
            //* clear data source.
            if(mPriData->mSourceUrl != NULL)
            {
                free(mPriData->mSourceUrl);
                mPriData->mSourceUrl = NULL;
            }
            if(mPriData->mSourceFd != -1)
            {
                close(mPriData->mSourceFd);
                mPriData->mSourceFd = -1;
                mPriData->mSourceFdOffset = 0;
                mPriData->mSourceFdLength = 0;
            }
            mPriData->mSourceStream = NULL;

            //* clear media info.
            clearMediaInfo(mPriData);

            //* clear loop setting.
            mPriData->mLoop   = 0;
#ifndef ONLY_ENABLE_AUDIO
            //* clear the mSubtitleDisplayIds
            memset(mPriData->mSubtitleDisplayIds,0xff,64*sizeof(unsigned int));
            mPriData->mSubtitleDisplayIdsUpdateIndex = 0;
#endif
            //* set status to IDLE.
            mPriData->mStatus = XPLAYER_STATUS_IDLE;

            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        }
        else if(msg.messageId == XPLAYER_COMMAND_SEEK)
        {
            logd("process message XPLAYER_COMMAND_SEEK.");
            if(mPriData->mStatus != XPLAYER_STATUS_PREPARED &&
               mPriData->mStatus != XPLAYER_STATUS_STARTED  &&
               mPriData->mStatus != XPLAYER_STATUS_PAUSED   &&
               mPriData->mStatus != XPLAYER_STATUS_COMPLETE)
            {
                logd("invalid seekTo() call, player not in prepared, \
                            started, paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)-1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            //* the application will call seekTo() when player is in complete status.
            //* after seekTo(), the player should still stay on complete status until
            //* application call start().
            //* cts test requires this implement.
            if(mPriData->mStatus == XPLAYER_STATUS_COMPLETE)
            {
                //* protect mSeeking and mSeekTime from being changed by the seek finish callback.
                pthread_mutex_lock(&mPriData->mMutexStatus);
                mPriData->mSeekTime = msg.params[2];
                pthread_mutex_unlock(&mPriData->mMutexStatus);
                mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_SEEK_COMPLETE, 0, 0);
                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            if(mPriData->mMediaInfo == NULL || mPriData->mMediaInfo->bSeekable == 0)
            {
                if(mPriData->mMediaInfo == NULL)
                {
                    loge("seekTo fail because mMediaInfo == NULL.");
                    mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_SEEK_COMPLETE, 0, 0);
                    if(pReplyValue != NULL)
                        *pReplyValue = -1;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;
                }
                else
                {
                    loge("media not seekable.");
                    mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_SEEK_COMPLETE, 0, 0);
                    if(pReplyValue != NULL)
                        *pReplyValue = 0;
                    if(pReplySem != NULL)
                        sem_post(pReplySem);
                    continue;
                }
            }

            if(mPriData->mSeeking)
            {
                DemuxCompCancelSeek(mPriData->mDemux);
                mPriData->mSeeking = 0;
            }

            //* protect mSeeking and mSeekTime from being changed by the seek finish callback.
            pthread_mutex_lock(&mPriData->mMutexStatus);
            mPriData->mSeeking  = 1;
            mPriData->mSeekTime = msg.params[2];
            mPriData->mSeekSync = msg.params[3];
            logv("seekTo %.2f secs", mPriData->mSeekTime / 1E3);
            pthread_mutex_unlock(&mPriData->mMutexStatus);

            if(PlayerGetStatus(mPriData->mPlayer) == PLAYER_STATUS_STOPPED)
            {
                //* if in prepared status, the player is in stopped status,
                //* this will make the player not record the nSeekTime at PlayerReset() operation
                //* called at seek finish callback.
                PlayerStart(mPriData->mPlayer,0,0);
            }
            PlayerPause(mPriData->mPlayer);
            DemuxCompSeekTo(mPriData->mDemux, mPriData->mSeekTime, mPriData->mSeekModeType);

            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end XPLAYER_COMMAND_SEEK.
        else if(msg.messageId == XPLAYER_COMMAND_RESETURL)
        {
            logd("process message XPLAYER_COMMAND_RESETURL.");
            if(mPriData->mStatus != XPLAYER_STATUS_PREPARED &&
               mPriData->mStatus != XPLAYER_STATUS_STARTED  &&
               mPriData->mStatus != XPLAYER_STATUS_PAUSED   &&
               mPriData->mStatus != XPLAYER_STATUS_COMPLETE)
            {
                logd("invalid seekTo() call, player not in prepared, started, \
                        paused or complete status.");
                if(pReplyValue != NULL)
                    *pReplyValue = (int)-1;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            if(mPriData->mSeeking)
            {
                DemuxCompCancelSeek(mPriData->mDemux);
                mPriData->mSeeking = 0;
            }

            //* protect mSeeking and mSeekTime from being changed by the seek finish callback.
            pthread_mutex_lock(&mPriData->mMutexStatus);
            mPriData->mSeeking  = 1;
            mPriData->mSeekTime = msg.params[2];
            mPriData->mSeekSync = msg.params[3];
            logd("seekTo %.2f secs", mPriData->mSeekTime / 1E3);
            pthread_mutex_unlock(&mPriData->mMutexStatus);

            if(PlayerGetStatus(mPriData->mPlayer) == PLAYER_STATUS_STOPPED)
            {
                //* if in prepared status, the player is in stopped status,
                //* this will make the player not record the nSeekTime at PlayerReset() operation
                //* called at seek finish callback.
                PlayerStart(mPriData->mPlayer,0,0);
            }
            PlayerPause(mPriData->mPlayer);
            DemuxCompSeekToResetUrl(mPriData->mDemux, mPriData->mSeekTime);

            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        }
        else if(msg.messageId == XPLAYER_COMMAND_QUIT)
        {
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            break;  //* break the thread.
        } //* end XPLAYER_COMMAND_QUIT.
        else
        {
            logw("unknow message with id %d, ignore.", msg.messageId);
        }
    }

    return 0;
}

#ifndef ONLY_ENABLE_AUDIO
int XPlayerSwitchSubtitle(XPlayer* p, int nStreamIndex)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    if(mPriData->mPlayer == NULL)
    {
        return -1;
    }

    return PlayerSwitchSubtitle(mPriData->mPlayer, nStreamIndex);
}
#endif

int XPlayerSwitchAudio(XPlayer* p, int nStreamIndex)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    if(mPriData->mPlayer == NULL)
    {
        return -1;
    }

    return PlayerSwitchAudio(mPriData->mPlayer, nStreamIndex);
}

#ifndef ONLY_ENABLE_AUDIO
static int setSubtitleStream(PlayerContext* mPriData, int nStreamNum,
                                  SubtitleStreamInfo* pNewStreamInfo)
{
    int i;
    SubtitleStreamInfo* pStreamInfo;
    if(pNewStreamInfo == NULL || nStreamNum == 0)
    {
        loge("PlayerProbeSubtitleStreamInfo failed!");
        return -1;
    }

    pthread_mutex_lock(&mPriData->mMutexMediaInfo);

    //* set reference video size.
    if(mPriData->mMediaInfo->pVideoStreamInfo != NULL)
    {
        for(i=0; i<nStreamNum; i++)
        {
            if(pNewStreamInfo[i].nReferenceVideoFrameRate == 0)
                pNewStreamInfo[i].nReferenceVideoFrameRate =
                    mPriData->mMediaInfo->pVideoStreamInfo->nFrameRate;
            if(pNewStreamInfo[i].nReferenceVideoHeight == 0 ||
               pNewStreamInfo[i].nReferenceVideoWidth == 0)
            {
                pNewStreamInfo[i].nReferenceVideoHeight =
                    mPriData->mMediaInfo->pVideoStreamInfo->nHeight;
                pNewStreamInfo[i].nReferenceVideoWidth  =
                    mPriData->mMediaInfo->pVideoStreamInfo->nWidth;
            }
        }
    }

    //* add stream info to the mMediaInfo,
    //* put external subtitle streams ahead of the embedded subtitle streams.
    if(mPriData->mMediaInfo->pSubtitleStreamInfo == NULL)
    {
        mPriData->mMediaInfo->pSubtitleStreamInfo = pNewStreamInfo;
        mPriData->mMediaInfo->nSubtitleStreamNum  = nStreamNum;
        pNewStreamInfo = NULL;
        nStreamNum = 0;
    }
    else
    {
        pStreamInfo = (SubtitleStreamInfo*)malloc(sizeof(SubtitleStreamInfo)*
                (mPriData->mMediaInfo->nSubtitleStreamNum + nStreamNum));
        if(pStreamInfo == NULL)
        {
            loge("invode::INVOKE_ID_ADD_EXTERNAL_SOURCE fail, can not malloc memory.");
            for(i=0; i<nStreamNum; i++)
            {
                if(pNewStreamInfo[i].pUrl != NULL)
                {
                    free(pNewStreamInfo[i].pUrl);
                    pNewStreamInfo[i].pUrl = NULL;
                }

                if(pNewStreamInfo[i].fd >= 0)
                {
                    close(pNewStreamInfo[i].fd);
                    pNewStreamInfo[i].fd = -1;
                }

                if(pNewStreamInfo[i].fdSub >= 0)
                {
                    close(pNewStreamInfo[i].fdSub);
                    pNewStreamInfo[i].fdSub = -1;
                }
            }

            free(pNewStreamInfo);
            pthread_mutex_unlock(&mPriData->mMutexMediaInfo);
            return -1;
        }

        //* make the internal subtitle in front of external subtitle
        memcpy(&pStreamInfo[mPriData->mMediaInfo->nSubtitleStreamNum],
                pNewStreamInfo, sizeof(SubtitleStreamInfo)*nStreamNum);
        memcpy(pStreamInfo, mPriData->mMediaInfo->pSubtitleStreamInfo,
                sizeof(SubtitleStreamInfo)*mPriData->mMediaInfo->nSubtitleStreamNum);

        free(mPriData->mMediaInfo->pSubtitleStreamInfo);
        mPriData->mMediaInfo->pSubtitleStreamInfo = pStreamInfo;
        mPriData->mMediaInfo->nSubtitleStreamNum += nStreamNum;

        free(pNewStreamInfo);
        pNewStreamInfo = NULL;
        nStreamNum = 0;
    }

    //* re-arrange the stream index.
    for(i=0; i<mPriData->mMediaInfo->nSubtitleStreamNum; i++)
        mPriData->mMediaInfo->pSubtitleStreamInfo[i].nStreamIndex = i;

    //* set subtitle stream info to player again.
    //* here mMediaInfo != NULL, so initializePlayer() had been called.
    if(mPriData->mPlayer != NULL)
    {
        int nDefaultSubtitleIndex = -1;
        for(i=0; i<mPriData->mMediaInfo->nSubtitleStreamNum; i++)
        {
            if(PlayerCanSupportSubtitleStream(mPriData->mPlayer,
                    &mPriData->mMediaInfo->pSubtitleStreamInfo[i]))
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

        if(0 != PlayerSetSubtitleStreamInfo(mPriData->mPlayer,
                                            mPriData->mMediaInfo->pSubtitleStreamInfo,
                                            mPriData->mMediaInfo->nSubtitleStreamNum,
                                            nDefaultSubtitleIndex))
        {
            logw("PlayerSetSubtitleStreamInfo() fail, subtitle stream not supported.");
        }
    }

    pthread_mutex_unlock(&mPriData->mMutexMediaInfo);

    return 0;
}


int XPlayerSetExternalSubUrl(XPlayer* p, const char* fileName)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    SubtitleStreamInfo* pStreamInfo;
    SubtitleStreamInfo* pNewStreamInfo = NULL;
    int                 nStreamNum = 0;
    int i;

    if(mPriData->mStatus != XPLAYER_STATUS_PREPARED || mPriData->mMediaInfo == NULL)
    {
        loge("can not add external text source, player not in prepared status \
            or there is no media stream.");
        return -1;
    }

    //* probe subtitle info
    PlayerProbeSubtitleStreamInfo(fileName, &nStreamNum, &pNewStreamInfo);

    if(pNewStreamInfo == NULL || nStreamNum == 0)
    {
        loge("PlayerProbeSubtitleStreamInfo failed!");
        return -1;
    }

    return setSubtitleStream(mPriData, nStreamNum, pNewStreamInfo);
}


int XPlayerSetExternalSubFd(XPlayer* p, int fd, int64_t offset, int64_t len, int fdSub)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    SubtitleStreamInfo* pStreamInfo;
    SubtitleStreamInfo* pNewStreamInfo = NULL;
    int                 nStreamNum = 0;
    int i;

    if(mPriData->mStatus != XPLAYER_STATUS_PREPARED || mPriData->mMediaInfo == NULL)
    {
        loge("can not add external text source, player not in prepared status \
            or there is no media stream.");
        return -1;
    }

    //* probe subtitle info
    PlayerProbeSubtitleStreamInfoFd(fd, offset, len, &nStreamNum, &pNewStreamInfo);

    if(nStreamNum > 0 && pNewStreamInfo[0].eCodecFormat == SUBTITLE_CODEC_IDXSUB)
    {
        if(fdSub >= 0)
        {
            //* for index+sub subtitle,
            //* we set the .sub file's descriptor to pNewStreamInfo[i].fdSub.
            for(i=0; i<nStreamNum; i++)
                pNewStreamInfo[i].fdSub = dup(fdSub);
        }
        else
        {
            loge("index sub subtitle stream without .sub file fd.");
            for(i=0; i<nStreamNum; i++)
            {
                if(pNewStreamInfo[i].fd >= 0)
                {
                    close(pNewStreamInfo[i].fd);
                    pNewStreamInfo[i].fd = -1;
                }
            }
            free(pNewStreamInfo);
            pNewStreamInfo = NULL;
            nStreamNum = 0;
        }
    }

    //* fdSub is the file descriptor of .sub file of a index+sub subtitle.
    if(fdSub >= 0)
    {
        //* close the file descriptor of .idx file, we dup it when
        //* INVOKE_ID_ADD_EXTERNAL_SOURCE_FD is called to set the .idx file
        if(fd >= 0)
            close(fd);
    }

    if(pNewStreamInfo == NULL || nStreamNum == 0)
    {
        loge("PlayerProbeSubtitleStreamInfo failed!");
        return -1;
    }

    return setSubtitleStream(mPriData, nStreamNum, pNewStreamInfo);
}

int XPlayerSetSubCharset(XPlayer* p, const char* strFormat)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    if(strFormat != NULL && (strlen(strFormat) < 31))
    {
        strcpy(mPriData->mDefaultTextFormat, strFormat);
    }
    else
    {
        strcpy(mPriData->mDefaultTextFormat, "UTF-8");
    }

    return 0;
}

int XPlayerGetSubCharset(XPlayer* p, char *charset)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    if(mPriData->mPlayer == NULL)
    {
        return -1;
    }

    strcpy(charset, mPriData->mDefaultTextFormat);

    return 0;
}

int XPlayerSetSubDelay(XPlayer* p, int nTimeMs)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    if(mPriData->mPlayer!=NULL)
        return PlayerSetSubtitleShowTimeAdjustment(mPriData->mPlayer, nTimeMs);
    else
        return -1;
}

int XPlayerGetSubDelay(XPlayer* p)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    if(mPriData->mPlayer!=NULL)
        return PlayerGetSubtitleShowTimeAdjustment(mPriData->mPlayer);
    else
        return -1;
}
#endif

int XPlayerGetBitrate(XPlayer* p)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    if(mPriData->mPlayer!=NULL)
#ifndef ONLY_ENABLE_AUDIO
        return PlayerGetVideoBitrate(mPriData->mPlayer) +
               PlayerGetAudioBitrate(mPriData->mPlayer);
#else
        return PlayerGetAudioBitrate(mPriData->mPlayer);
#endif
    else
        return -1;
}

int XPlayerGetCacheSize(XPlayer* p)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    if(mPriData->mPlayer!=NULL)
        return DemuxCompGetCacheSize(mPriData->mDemux);
    else
        return -1;
}
int XPlayerSetBufferingSettings(XPlayer* p, AwBufferingSettings* pBuffering)
{
    if(pBuffering == NULL)
    {
        return -1;
    }
    PlayerContext* mPriData = (PlayerContext*)p;
    mPriData->pBuffering = pBuffering;
    DemuxCompSetBufferingSettings(mPriData->mDemux, mPriData->pBuffering);
    return 0;
}
int XPlayerGetBufferingSettings(XPlayer* p, AwBufferingSettings* pBuffering)
{
    if(p == NULL || pBuffering == NULL)
    {
        logd("input is NULL. Please check it.");
        return -1;
    }
    PlayerContext* mPriData = (PlayerContext*)p;
    return DemuxCompGetBufferingSettings(mPriData->mDemux, pBuffering);
}

#ifndef ONLY_ENABLE_AUDIO
int XPlayerGetVideoDispFramerate(XPlayer* p,float* dispFramerate){
    PlayerContext* mPriData = (PlayerContext*)p;
    return PlayerGetVideoDispFramerate(mPriData->mPlayer,dispFramerate);
}

int XPlayerSetHoldLastPicture(XPlayer* p,int bHoldFlag){
    PlayerContext* mPriData = (PlayerContext*)p;
    return PlayerSetHoldLastPicture(mPriData->mPlayer,bHoldFlag);
}

int XPlayerSetRotateDegree(XPlayer* p,int rotateDegree){
    PlayerContext* mPriData = (PlayerContext*)p;
    mPriData->mVideoRotateDegree = rotateDegree;
    return 0;
}
#endif

static int callbackProcess(void *p, int messageId, void* param)
{
    PlayerContext* mPriData = (PlayerContext*)p;
    switch(messageId)
    {
        case DEMUX_NOTIFY_PREPARED:
        {
            uintptr_t tmpPtr = (uintptr_t)param;
            int err = tmpPtr;
            if(err != 0)
            {
                //* demux prepare return fail.
                //* notify a media error event.
                mPriData->mStatus = XPLAYER_STATUS_ERROR;
                if(mPriData->mPrepareSync == 0)
                {
                    if(err == DEMUX_ERROR_IO)
                    {
                        mPriData->mCallback(mPriData->pUser,
                            AWPLAYER_MEDIA_ERROR, AW_MEDIA_ERROR_IO, 0);
                    }
                    else if(err == DEMUX_ERROR_USER_CANCEL)
                    {
                        logd("preparing canceled by user");
                    }
                    else
                    {
                        mPriData->mCallback(mPriData->pUser,
                            AWPLAYER_MEDIA_ERROR, AW_MEDIA_ERROR_UNKNOWN, 0);
                    }
                }
                else
                {
                    if(err == DEMUX_ERROR_IO)
                        mPriData->mPrepareFinishResult = AW_MEDIA_ERROR_IO;
                    else
                        mPriData->mPrepareFinishResult = -1;
                    sem_post(&mPriData->mSemPrepareFinish);
                }
            }
            else
            {
                //* demux prepare success, initialize the player.
                if(initializePlayer(p) == 0)
                {
                    //* initialize player success, notify a prepared event.
                    mPriData->mStatus = XPLAYER_STATUS_PREPARED;
                    if(mPriData->mPrepareSync == 0)
                    {
#ifndef ONLY_ENABLE_AUDIO
                        if(mPriData->mMediaInfo->pVideoStreamInfo!=NULL)
                        {
                            if(mPriData->mMediaInfo->pVideoStreamInfo->nWidth !=
                                        mPriData->mVideoSizeWidth
                               && mPriData->mMediaInfo->pVideoStreamInfo->nHeight !=
                                        mPriData->mVideoSizeHeight)
                            {
                                int nRotation;

                                nRotation = atoi((const char*)mPriData->mMediaInfo->cRotation);
                                if((nRotation%180)==0)//* when the rotation is 0 and 180
                                {
                                    mPriData->mVideoSizeWidth  =
                                            mPriData->mMediaInfo->pVideoStreamInfo->nWidth;
                                    mPriData->mVideoSizeHeight =
                                            mPriData->mMediaInfo->pVideoStreamInfo->nHeight;
                                }
                                else
                                {
                                    //* when the rotation is 90 and 270,
                                    //* we should exchange nHeight and nwidth
                                    mPriData->mVideoSizeWidth  =
                                            mPriData->mMediaInfo->pVideoStreamInfo->nHeight;
                                    mPriData->mVideoSizeHeight =
                                            mPriData->mMediaInfo->pVideoStreamInfo->nWidth;
                                }
                                logi("xxxxxxxxxx video size: width = %d, height = %d",
                                        mPriData->mVideoSizeWidth, mPriData->mVideoSizeHeight);
                                int size[2];
                                size[0] = mPriData->mVideoSizeWidth;
                                size[1] = mPriData->mVideoSizeHeight;
                                mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_SET_VIDEO_SIZE,
                                                    0, size);
                            }
                            else
                            {
                                //此时mVideoSizeWidth必是0
                                logi("xxxxxxxxxx video size: width = 0, height = 0");
                                mPriData->mCallback(mPriData->pUser,
                                            AWPLAYER_MEDIA_SET_VIDEO_SIZE, 0, (int []){0, 0});
                            }
                        }
                        else
                        {
                            //此时mVideoSizeWidth必是0
                            logi("xxxxxxxxxx video size: width = 0, height = 0");
                            mPriData->mCallback(mPriData->pUser,
                                            AWPLAYER_MEDIA_SET_VIDEO_SIZE, 0, (int []){0, 0});
                        }
#endif
                        mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_PREPARED, 0, 0);
                    }
                    else
                    {
                        mPriData->mPrepareFinishResult = 0;
                        sem_post(&mPriData->mSemPrepareFinish);
                    }
                }
                else
                {
                    //* initialize player fail, notify a media error event.
                    mPriData->mStatus = XPLAYER_STATUS_ERROR;
                    if(mPriData->mPrepareSync == 0)
                        mPriData->mCallback(mPriData->pUser,
                            AWPLAYER_MEDIA_ERROR, DEMUX_ERROR_UNKNOWN, 0);
                    else
                    {
                        mPriData->mPrepareFinishResult = -1;
                        sem_post(&mPriData->mSemPrepareFinish);
                    }
                }
            }

            break;
        }

        case DEMUX_NOTIFY_EOS:
        {
            if(mPriData->mLivemode == 2) //* timeshift
            {
                int seekTimeMs = 0;
                struct timeval tv;
                gettimeofday(&tv, NULL);
                mPriData->mCurShiftTimeStamp = tv.tv_sec * 1000000ll + tv.tv_usec;
                logd("mCurShiftTimeStamp=%" PRId64 " ms", mPriData->mCurShiftTimeStamp/1000);
                seekTimeMs = (mPriData->mCurShiftTimeStamp - mPriData->mShiftTimeStamp)/1000
                                - mPriData->mTimeShiftDuration + mPriData->mPreSeekTimeMs;
                logd("===== seekTimeMs=%d ms, mPriData->mCurShiftTimeStamp=%" PRId64 " us,"
                       "mPriData->mShiftTimeStamp=%" PRId64 " us,"
                       " mPriData->mTimeShiftDuration=%" PRId64 " ms, mPreSeekTimeMs=%d ms",
                        seekTimeMs,
                        mPriData->mCurShiftTimeStamp, mPriData->mShiftTimeStamp,
                        mPriData->mTimeShiftDuration, mPriData->mPreSeekTimeMs);
                XPlayerSeekTo(p, seekTimeMs, mPriData->mSeekModeType);
            }
            else
            {
                logw("eos...");
                PlayerSetEos(mPriData->mPlayer);
            }
            break;
        }

        case DEMUX_NOTIFY_IOERROR:
        {
            loge("io error...");
            //* should we report a MEDIA_INFO event of "MEDIA_INFO_NETWORK_ERROR" and
            //* try reconnect for sometimes before a MEDIA_ERROR_IO event reported ?

            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_ERROR, AW_MEDIA_ERROR_IO, 0);
            break;
        }

        case DEMUX_NOTIFY_CACHE_STAT:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_BUFFERING_UPDATE,
                            0,  param);
            break;
        }

        case DEMUX_NOTIFY_BUFFER_START:
        {
#ifndef ONLY_ENABLE_AUDIO
            logd("MEDIA_INFO_BUFFERING_START,mDisplayRatio = %d",mPriData->mDisplayRatio);
#endif
            // for live mode 1, getposition when network broken
            mPriData->mDemuxNotifyPause = 1;
            struct timeval tv;
            gettimeofday(&tv, NULL);
            mPriData->mDemuxPauseTimeStamp = tv.tv_sec * 1000000ll + tv.tv_usec;

            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_INFO,
                            AW_MEDIA_INFO_BUFFERING_START, param);
            break;
        }

        case DEMUX_NOTIFY_BUFFER_END:
        {
            logd("MEDIA_INFO_BUFFERING_END ...");

            mPriData->mDemuxNotifyPause = 0;
            mPriData->mDemuxPauseTimeStamp = 0;
            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_INFO,
                            AW_MEDIA_INFO_BUFFERING_END, 0);
            break;
        }

        case DEMUX_NOTIFY_PAUSE_PLAYER:
        {
            logd("XPLAYER_MESSAGE_DEMUX_PAUSE_PLAYER, mStatus = %d", mPriData->mStatus);
            //* be careful to check whether there is any player callback lock the mMutexStatus,
            //* if so, the PlayerPause() call may fall into dead lock if the player
            //* callback is requesting mMutexStatus.
            //* currently we do not lock mMutexStatus in any player callback.
            pthread_mutex_lock(&mPriData->mMutexStatus);
            if(mPriData->mStatus == XPLAYER_STATUS_STARTED)
                PlayerPause(mPriData->mPlayer);

            pthread_mutex_unlock(&mPriData->mMutexStatus);
            break;
        }

        case DEMUX_NOTIFY_RESUME_PLAYER:
        {
            //* be careful to check whether there is any player callback lock the mMutexStatus,
            //* if so, the PlayerPause() call may fall into dead lock if the player
            //* callback is requesting mMutexStatus.
            //* currently we do not lock mMutexStatus in any player callback.
            pthread_mutex_lock(&mPriData->mMutexStatus);
            if (mPriData->mStatus == XPLAYER_STATUS_STARTED /*&& bBuffering == false*/)
            {
                PlayerStart(mPriData->mPlayer,0,0);
            }
            pthread_mutex_unlock(&mPriData->mMutexStatus);
            break;
        }

#if !(defined(CONF_NEW_BDMV_STREAM))

        case DEMUX_IOREQ_ACCESS:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_EXTEND_MEDIA_INFO,
                                AW_EX_IOREQ_ACCESS, param);
            break;
        }

        case DEMUX_IOREQ_OPEN:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_EXTEND_MEDIA_INFO,
                                AW_EX_IOREQ_OPEN, param);
            break;
        }

        case DEMUX_IOREQ_OPENDIR:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_EXTEND_MEDIA_INFO,
                                AW_EX_IOREQ_OPENDIR, param);
            break;
        }

        case DEMUX_IOREQ_READDIR:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_EXTEND_MEDIA_INFO,
                                AW_EX_IOREQ_READDIR, param);
            break;
        }

        case DEMUX_IOREQ_CLOSEDIR:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_EXTEND_MEDIA_INFO,
                                AW_EX_IOREQ_CLOSEDIR, param);
            break;
        }

#endif

        case PLAYBACK_NOTIFY_EOS:
        {
            mPriData->mStatus = XPLAYER_STATUS_COMPLETE;
            if(mPriData->mLoop == 0)
            {
                logd("player notify eos.");
                mPriData->mSeekTime = 0; //* clear the seek flag.
#ifndef ONLY_ENABLE_AUDIO
                mPriData->mbFast = 0;
                mPriData->mSpeed = 1;
                mPriData->mFastTime = 0;
#endif
                mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_PLAYBACK_COMPLETE, 0, 0);
            }
            else
            {
                AwMessage msg;

                logv("player notify eos, loop is set, send start command.");
                mPriData->mSeekTime = 0;  //* seek to the file start and replay.
                //* send a start message.
                memset(&msg, 0, sizeof(AwMessage));
                msg.messageId = XPLAYER_COMMAND_START;
                AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
            }
            break;
        }

#ifndef ONLY_ENABLE_AUDIO
        case PLAYBACK_NOTIFY_FIRST_PICTURE:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_INFO,
                                                 AW_MEDIA_INFO_RENDERING_START, 0);
            DemuxCompNotifyFirstFrameShowed(mPriData->mDemux);

            break;
        }
#endif

        case DEMUX_NOTIFY_RESET_PLAYER:
        {
            logd("DEMUX_NOTIFY_RESET_PLAYER");
            mPriData->mLivemode = DemuxCompGetLiveMode(mPriData->mDemux);
            if(mPriData->mLivemode == 1)
            {
                mPriData->mSeekTime = 0;
            }
            pthread_mutex_lock(&mPriData->mMutexStatus);
            PlayerReset(mPriData->mPlayer, ((int64_t)mPriData->mSeekTime)*1000);
            pthread_mutex_unlock(&mPriData->mMutexStatus);
            break;
        }
        case DEMUX_NOTIFY_SEEK_FINISH:
        {
            logd("DEMUX_NOTIFY_SEEK_FINISH");
            int seekResult;
            int nSeekTimeMs;
            int nFinalSeekTimeMs;

            //* be careful to check whether there is any player callback lock the mMutexStatus,
            //* if so, the PlayerPause() call may fall into dead lock if the player
            //* callback is requesting mMutexStatus.
            //* currently we do not lock mMutexStatus in any player callback.
            pthread_mutex_lock(&mPriData->mMutexStatus);

            seekResult       = ((int*)param)[0];
            nSeekTimeMs      = ((int*)param)[1];
            nFinalSeekTimeMs = ((int*)param)[2];

            int bSeekClosest = 0;
            if(mPriData->mSeekModeType == AW_SEEK_CLOSEST)
            {
                bSeekClosest = 1;
            }

            if (seekResult == 0)
            {
                PlayerReset(mPriData->mPlayer, ((int64_t)nFinalSeekTimeMs)*1000);

                if (nSeekTimeMs == mPriData->mSeekTime)
                {
                    mPriData->mSeeking = 0;
                    if (mPriData->mStatus == XPLAYER_STATUS_STARTED)
                    {
                        PlayerStart(mPriData->mPlayer,bSeekClosest,nSeekTimeMs);
                        DemuxCompStart(mPriData->mDemux);

                        //check if seek call by xplayer inner case
                        //inner case seek no need to callback app
                        if (mPriData->mSeekByInner == 1)
                        {
                            mPriData->mSeekByInner = 0;
                            pthread_mutex_unlock(&mPriData->mMutexStatus);
                            break;
                        }
                    }
                }
                else
                {
                    logv("seek time not match, there may be another seek operation happening.");
                }
                pthread_mutex_unlock(&mPriData->mMutexStatus);
                mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_SEEK_COMPLETE, 0, 0);
            }
            else if(seekResult == DEMUX_ERROR_USER_CANCEL)
            {
                // do nothing , do not start player
                pthread_mutex_unlock(&mPriData->mMutexStatus);
                mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_SEEK_COMPLETE, 0, 0);
            }
            else
            {
                pthread_mutex_unlock(&mPriData->mMutexStatus);

                mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_ERROR, AW_MEDIA_ERROR_IO, 0);
            }
            break;
        }
#ifndef ONLY_ENABLE_AUDIO
        case PLAYBACK_NOTIFY_SUBTITLE_ITEM_AVAILABLE:
        {

            //logd("subtitle available. id = %d, pSubtitleItem = %p",nSubtitleId,pSubtitleItem);

            break;
        }

        case PLAYBACK_NOTIFY_SUBTITLE_ITEM_EXPIRED:
        {
            logd("subtitle expired.");
            unsigned int nSubtitleId;
            int i;

            break;
        }

        case PLAYBACK_NOTIFY_VIDEO_SIZE:
        {
            int nWidth  = ((int*)param)[0];
            int nHeight = ((int*)param)[1];
            logd("*************decoded nWidth = %d,nHeight = %d********",nWidth,nHeight);
            int decodedSize[2];
            decodedSize[0] = nWidth;
            decodedSize[1] = nHeight;
            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_DECODED_VIDEO_SIZE,
                                        0, decodedSize);
            //* if had scale down, we should zoom the widht and height
            //* here the case of the VR9 we should not zoom the width and height
            if(mPriData->mScaledownFlag == 1 && mPriData->bSetScaleDown != 1)
            {
                nWidth  = 2*nWidth;
                nHeight = 2*nHeight;
            }

            //if(nWidth != mVideoSizeWidth || nHeight != mVideoSizeHeight)
            {
                logi("xxxxxxxxxx video size : width = %d, height = %d", nWidth, nHeight);

                mPriData->mVideoSizeWidth  = nWidth;
                mPriData->mVideoSizeHeight = nHeight;
                int size[2];
                size[0] = mPriData->mVideoSizeWidth;
                size[1] = mPriData->mVideoSizeHeight;
                mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_SET_VIDEO_SIZE,
                                        0, size);
            }
            break;
        }

        case PLAYBACK_NOTIFY_AUDIORAWPLAY:
        {
            break;
        }
        case PLAYBACK_NOTIFY_VIDEO_CROP:
            //* TODO
            break;

        case PLAYBACK_NOTIFY_VIDEO_UNSUPPORTED:
            if(mPriData->mMediaInfo->nVideoStreamNum < 2)
            {
                mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_ERROR,
                                        AW_MEDIA_ERROR_UNSUPPORTED, 0);
            }

            break;
#endif

        case PLAYBACK_NOTIFY_AUDIO_UNSUPPORTED:
#ifndef ONLY_ENABLE_AUDIO
            if(mPriData->mMediaInfo->nVideoStreamNum == 0)
#endif
            {
                mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_ERROR,
                                        AW_MEDIA_ERROR_UNSUPPORTED, 0);
            }
            //* TODO
            break;

#ifndef ONLY_ENABLE_AUDIO
        case PLAYBACK_NOTIFY_SUBTITLE_UNSUPPORTED:
            //* TODO
            break;

        case DEMUX_VIDEO_STREAM_CHANGE:
            logd("update video info");
            int a_change  = ((int*)param)[0];
            if(!a_change)
                updateVideoInfo(p);
            else
            {
                updateVideoInfo(p);
                updateAudioInfo(p);
            }
            break;
#endif

        case DEMUX_AUDIO_STREAM_CHANGE:
            logd("update audio info");
            updateAudioInfo(p);
            break;

#ifndef ONLY_ENABLE_AUDIO
        case PLAYBACK_NOTIFY_VIDEO_RENDER_FRAME:
#if 0
            if(mPriData->mbFast)
            {
                logd("==== video key frame in fast mode, mFastTime: %d, mSpeed: %d",
                            mPriData->mFastTime, mPriData->mSpeed);
                if(mPriData->mSpeed == 0)
                {
                    break;
                }

                int sleepTime = (mPriData->mSpeed > 0) ? 2000000/mPriData->mSpeed :
                            (-2000000/mPriData->mSpeed);
                //usleep(sleepTime);

                mPriData->mFastTime += mPriData->mSpeed*1000;
                if(mPriData->mFastTime > 0 && mPriData->mbFast)
                {
                    AwMessage msg;

                    //* send a seek message.
                    memset(&msg, 0, sizeof(AwMessage));
                    msg.messageId = XPLAYER_COMMAND_SEEK;
                    msg.params[0] = (uintptr_t)&mPriData->mSemSeek;
                    msg.params[1] = (uintptr_t)&mPriData->mSeekReply;
                    msg.params[2] = mPriData->mFastTime;     //* params[2] = mSeekTime.
                    msg.params[3] = 0;                      //* params[3] = mSeekSync.

                    AwMessageQueuePostMessage(mPriData->mMessageQueue, &msg);
                }
                else if(mPriData->mFastTime <= 0)
                {
                    PlayerSetDiscardAudio(mPriData->mPlayer, 0);
                    mPriData->mbFast = 0;
                }
            }
#endif
            break;
#endif
        case DEMUX_NOTIFY_HLS_DOWNLOAD_START:
        {
            logd("--- hls message, DEMUX_NOTIFY_HLS_DOWNLOAD_START");
            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_INFO,
                                                 AW_MEDIA_INFO_DOWNLOAD_START, param);
            break;
        }
        case DEMUX_NOTIFY_HLS_DOWNLOAD_END:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_INFO,
                                                 AW_MEDIA_INFO_DOWNLOAD_END, param);
            break;
        }
        case DEMUX_NOTIFY_HLS_DOWNLOAD_ERROR:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_INFO,
                                                 AW_MEDIA_INFO_DOWNLOAD_ERROR, param);

            break;
        }
        case DEMUX_NOTIFY_LOG_RECORDER:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_LOG_RECORDER, 0, param);
            break;
        }
        case DEMUX_NOTIFY_TIMESHIFT_DURATION:
        {
            if(mPriData->mLivemode == 2)
            {
                mPriData->mTimeShiftDuration = *(cdx_int64*)param;
                logd("xxxxxxxxx mPriData->mTimeShiftDuration=%" PRId64 "",
                                                 mPriData->mTimeShiftDuration);
            }
            break;
        }
        case DEMUX_NOTIFY_METADATA:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_META_DATA,
                            0,  param);
            break;
        }

        case PLAYBACK_RESET_BUFFER_FINISHED:
        {
            mPriData->mCallback(mPriData->pUser, AWPLAYER_MEDIA_RESET_BUFFER_FINISHED,0, NULL);
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
