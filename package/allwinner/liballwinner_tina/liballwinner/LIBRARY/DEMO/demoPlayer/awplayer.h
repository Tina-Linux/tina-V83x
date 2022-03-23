
#ifndef AWPLAYER_H
#define AWPLAYER_H

#include <semaphore.h>
#include <pthread.h>
#include "cdx_config.h"             //* configuration file in "LiBRARY/"
#include "player.h"             //* player library in "LIBRARY/PLAYER/"
#include "mediaInfo.h"
#include "demuxComponent.h"
#include "awMessageQueue.h"

#define NOTIFY_NOT_SEEKABLE         1
#define NOTIFY_ERROR                2
#define NOTIFY_PREPARED             3
#define NOTIFY_BUFFERRING_UPDATE    4
#define NOTIFY_PLAYBACK_COMPLETE    5
#define NOTIFY_RENDERING_START      6
#define NOTIFY_SEEK_COMPLETE        7

#define NOTIFY_ERROR_TYPE_UNKNOWN   0x100   //* for param0 when notify a NOTIFY_ERROR message.
#define NOTIFY_ERROR_TYPE_IO        0x101   //* for param0 when notify a NOTIFY_ERROR message.

typedef void (*NotifyCallback)(void* pUserData, int msg, int param0, void* param1);

class AwPlayer
{
public:
    AwPlayer();
    ~AwPlayer();

    int initCheck();
    int setNotifyCallback(NotifyCallback notifier, void* pUserData);
#if CONFIG_OS == OPTION_OS_ANDROID
    int setDataSource(const char* pUrl, const KeyedVector<String8, String8>* pHeaders);
#else
    int setDataSource(const char* pUrl, const map<string, string>* pHeaders);
#endif
    int prepare();
    int prepareAsync();
    int start();
    int stop();
    int pause();
    int isPlaying();
    int seekTo(int msec);

    int getCurrentPosition(int* msec);
    int getDuration(int* msec);
    int reset();
    int setLooping(int bLoop);

    int callbackProcess(int messageId, void* param);
    int mainThread();

private:
    int  initializePlayer();
    void clearMediaInfo();

private:
    AwMessageQueue*     mMessageQueue;
    Player*             mPlayer;
    DemuxComp*          mDemux;
    pthread_t           mThreadId;
    int                 mThreadCreated;

    //* data source.
    char*               mSourceUrl;       //* file path or network stream url.

    //* media information.
    MediaInfo*          mMediaInfo;
    NotifyCallback      mNotifier;
    void*               mUserData;

	//* for status and synchronize control.
	int                 mStatus;
	pthread_mutex_t     mMutex;
	sem_t               mSemSetDataSource;
	sem_t               mSemPrepare;
	sem_t               mSemStart;
	sem_t               mSemStop;
	sem_t               mSemPause;
	sem_t               mSemQuit;
	sem_t               mSemReset;
	sem_t               mSemSeek;
	sem_t               mSemPrepareFinish;      //* for signal prepare finish, used in prepare().

	//* status control.
	int                 mSetDataSourceReply;
	int                 mPrepareReply;
    int                 mStartReply;
    int                 mStopReply;
    int                 mPauseReply;
    int                 mResetReply;
    int                 mSeekReply;
	int                 mPrepareFinishResult;   //* save the prepare result for prepare().

	int                 mPrepareSync;   //* synchroized prarare() call, don't call back to user.
	int                 mSeeking;
	int                 mSeekTime;  //* use to check whether seek callback is for current seek operation or previous.
	int                 mSeekSync;  //* internal seek, don't call back to user.
	int                 mLoop;

    AwPlayer(const AwPlayer&);
    AwPlayer &operator=(const AwPlayer&);
};

#endif  // AWPLAYER
