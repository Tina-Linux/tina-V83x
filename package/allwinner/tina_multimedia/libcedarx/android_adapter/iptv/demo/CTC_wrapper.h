/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CTC_wrapper.h
 * Description : CTC_wrapper
 * History :
 *
 */


#ifndef CTC_WRAPPER_H
#define CTC_WRAPPER_H

#include <semaphore.h>
#include <pthread.h>
#include "cdx_config.h"             //* configuration file in "LiBRARY/"
#include "player.h"             //* player library in "LIBRARY/PLAYER/"
#include "mediaInfo.h"
#include "AwMessageQueue.h"
#include <CTC_MediaProcessor.h>

#define NOTIFY_NOT_SEEKABLE         1
#define NOTIFY_ERROR                2
#define NOTIFY_PREPARED             3
#define NOTIFY_BUFFERRING_UPDATE    4
#define NOTIFY_PLAYBACK_COMPLETE    5
#define NOTIFY_RENDERING_START      6
#define NOTIFY_SEEK_COMPLETE        7

#define NOTIFY_ERROR_TYPE_UNKNOWN   0x100   //* for param0 when notify a NOTIFY_ERROR message.
#define NOTIFY_ERROR_TYPE_IO        0x101   //* for param0 when notify a NOTIFY_ERROR message.

#define ClearMediaInfo 0

using namespace android;
typedef void (*NotifyCallback)(void* pUserData, int msg, int param0, void* param1);

class CTC_Wrapper
{
public:
    CTC_Wrapper();
    virtual ~CTC_Wrapper();

    virtual status_t    initCheck();

    virtual status_t    setDataSource(const char* pUrl,
                        const KeyedVector<String8, String8>* pHeaders);

    virtual status_t    setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer);

    virtual status_t    prepare();
    virtual status_t    prepareAsync();
    virtual status_t    start();
    virtual status_t    stop();
    virtual status_t    pause();
    virtual bool        isPlaying();
    virtual status_t    seekTo(int nSeekTimeMs);

    virtual status_t    getCurrentPosition(int* msec);
    virtual status_t    getDuration(int* msec);
    virtual status_t    reset();
    virtual status_t    fast();
    virtual status_t    stopFast();
    virtual status_t    resume();

    virtual status_t    mainThread();
    int setNotifyCallback(NotifyCallback notifier, void* pUserData);

    virtual status_t    setVideoWindow(int x, int y, int width, int height);
    virtual status_t    show();
    virtual status_t    hide();
    virtual status_t    getVideoPixel(int& width, int& height);
    virtual status_t    setVideoRadio(int radio);
    CdxMediaInfoT* GetMediaInfo();

    int SetVolume(int volume);

    int GetVolume();

    int SwitchAudio(int nStreamIndex);
#if ClearMediaInfo
private:
    virtual void        clearMediaInfo();
#endif

private:
    AwMessageQueue*     mMessageQueue;
    pthread_t           mThreadId;
    int                 mThreadCreated;

    //* data source.
    char*               mSourceUrl;       //* file path or network stream url.

    CdxDataSourceT              mSource;
    CdxParserT *pParser;
    CdxStreamT *pStream;
    cdx_bool bCancelPrepare;
    pthread_mutex_t mutex;
    MediaInfo                   mediaInfo;

    CTC_MediaProcessor *mediaProcessor;

    //* media information.
    CdxMediaInfoT         parserMediaInfo;
    MediaInfo*          mMediaInfo;
    NotifyCallback      mNotifier;
    void*               mUserData;

    //* surface.
    sp<IGraphicBufferProducer> mGraphicBufferProducer;
    sp<ANativeWindow>   mNativeWindow;

    sp<Surface> surface;

    //* for status and synchronize control.
    int                 mStatus;
    pthread_mutex_t     mMutexMediaInfo;    //* for media info protection.
    pthread_mutex_t     mMutexStatus;
    sem_t               mSemSetDataSource;
    sem_t               mSemPrepare;
    sem_t               mSemStart;
    sem_t               mSemStop;
    sem_t               mSemPause;
    sem_t               mSemQuit;
    sem_t               mSemReset;
    sem_t               mSemSeek;
    sem_t               mSemSetSurface;
    sem_t               mSemSetAudioSink;
    sem_t               mSemPrepareFinish;      //* for signal prepare finish, used in prepare().

    sem_t               mSemHide;

    //* status control.
    int                 mSetDataSourceReply;
    int                 mPrepareReply;
    int                 mStartReply;
    int                 mStopReply;
    int                 mPauseReply;
    int                 mResetReply;
    int                 mSeekReply;
    int                 mSetSurfaceReply;
    int                 mSetAudioSinkReply;
    int                 mPrepareFinishResult;   //* save the prepare result for prepare().

    int                 mPrepareSync;   //* synchroized prarare() call, don't call back to user.
    int                 mSeeking;
    int                    mStopping;
    int                 mSeekTime;
    int                 mSeekSync;  //* internal seek, don't call back to user.
    int                 mLoop;
    int                 mKeepLastFrame;
    int                 mVideoSizeWidth;  //* use to record videoSize which had send to app
    int                 mVideoSizeHeight;

    //* record the id of subtitle which is displaying
    //* we set the Nums to 64 .(32 may be not enough)
    unsigned int        mSubtitleDisplayIds[64];
    int                 mSubtitleDisplayIdsUpdateIndex;

    //* save the currentSelectTrackIndex;
    int                 mCurrentSelectAudioTrack;
    int                 mCurrentSelectSubtitleTrack;

    bool                bIsStreamEos;
    bool                bIOError;

    CTC_Wrapper(const CTC_Wrapper&);
    CTC_Wrapper &operator=(const CTC_Wrapper&);
};

void ShowMediaInfo(CdxMediaInfoT *mediaInfo);

#endif  // CTC_WRAPPER_H
