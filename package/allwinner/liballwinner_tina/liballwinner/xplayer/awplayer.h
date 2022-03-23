/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : awplayer.h
 * Description : player
 * History :
 *
 */


#ifndef AWPLAYER_H
#define AWPLAYER_H

#include <semaphore.h>
#include <pthread.h>
#include <string>
#include <map>
using namespace std;

#include "cdx_config.h"             //* configuration file in "LiBRARY/"
//#include "player.h"             //* player library in "LIBRARY/PLAYER/"
//#include "mediaInfo.h"
//#include "demuxComponent.h"
//#include "awMessageQueue.h"
#include "mediaInfo.h"
#include <AwTypes.h>
#include "soundControl.h"
#include "layerControl.h"

#define NOTIFY_NOT_SEEKABLE         1
#define NOTIFY_ERROR                2
#define NOTIFY_PREPARED             3
#define NOTIFY_BUFFERRING_UPDATE    4

#define NOTIFY_PLAYBACK_COMPLETE    5
#define NOTIFY_RENDERING_START      6
#define NOTIFY_SEEK_COMPLETE        7

#define NOTIFY_BUFFER_START            8
#define NOTIFY_BUFFER_END            9

#define NOTIFY_VIDEO_PACKET   10 //the video packet data demux from parser
#define NOTIFY_AUDIO_PACKET   11 //the audiopacket data demux from parser

#define NOTIFY_VIDEO_FRAME    12 //the video pic after decoding
#define NOTIFY_AUDIO_FRAME    13 //the audio pcm data after decoding

#define NOTIFY_ERROR_TYPE_UNKNOWN   0x100   //* for param0 when notify a NOTIFY_ERROR message.
#define NOTIFY_ERROR_TYPE_IO        0x101   //* for param0 when notify a NOTIFY_ERROR message.

enum EVIDEOPIXELFORMAT
{
    VIDEO_PIXEL_FORMAT_DEFAULT            = 0,

    VIDEO_PIXEL_FORMAT_YUV_PLANER_420     = 1,
    VIDEO_PIXEL_FORMAT_YUV_PLANER_422     = 2,
    VIDEO_PIXEL_FORMAT_YUV_PLANER_444     = 3,

    VIDEO_PIXEL_FORMAT_YV12               = 4,
    VIDEO_PIXEL_FORMAT_NV21               = 5,
    VIDEO_PIXEL_FORMAT_NV12               = 6,
    VIDEO_PIXEL_FORMAT_YUV_MB32_420       = 7,
    VIDEO_PIXEL_FORMAT_YUV_MB32_422       = 8,
    VIDEO_PIXEL_FORMAT_YUV_MB32_444       = 9,

    VIDEO_PIXEL_FORMAT_RGBA                = 10,
    VIDEO_PIXEL_FORMAT_ARGB                = 11,
    VIDEO_PIXEL_FORMAT_ABGR                = 12,
    VIDEO_PIXEL_FORMAT_BGRA                = 13,

    VIDEO_PIXEL_FORMAT_YUYV                = 14,
    VIDEO_PIXEL_FORMAT_YVYU                = 15,
    VIDEO_PIXEL_FORMAT_UYVY                = 16,
    VIDEO_PIXEL_FORMAT_VYUY                = 17,

    VIDEO_PIXEL_FORMAT_PLANARUV_422        = 18,
    VIDEO_PIXEL_FORMAT_PLANARVU_422        = 19,
    VIDEO_PIXEL_FORMAT_PLANARUV_444        = 20,
    VIDEO_PIXEL_FORMAT_PLANARVU_444        = 21,

    VIDEO_PIXEL_FORMAT_MIN = VIDEO_PIXEL_FORMAT_DEFAULT,
    VIDEO_PIXEL_FORMAT_MAX = VIDEO_PIXEL_FORMAT_PLANARVU_444,
};

typedef struct DemuxData
{
    int64_t        nPts;
    unsigned int   nSize0;
    unsigned int   nSize1;
    unsigned char* pData0;
    unsigned char* pData1;
}DemuxData;

typedef struct VideoPicData
{
    int64_t            nPts;
    int                ePixelFormat;
    int                nWidth;
    int                nHeight;
    int                nLineStride;
    int             nTopOffset;
    int             nLeftOffset;
    int             nBottomOffset;
    int             nRightOffset;
    char*  pData0;
    char*  pData1;
    char*  pData2;
    unsigned long phyYBufAddr;
    unsigned long phyCBufAddr;
}VideoPicData;

typedef struct AudioPcmData
{
    unsigned int   nSize;
    unsigned char* pData;
}AudioPcmData;

typedef void (*NotifyCallback)(void* pUserData, int msg, int param0, void* param1);

class AwPlayer
{
public:
    AwPlayer();
    ~AwPlayer();

    int initCheck();
    int setNotifyCallback(NotifyCallback notifier, void* pUserData);
    int setControlOps(LayerControlOpsT* pLayerCtlOps, SoundControlOpsT* pSoundCtlOps);
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
    int setSpeed(int mSpeed);
    MediaInfo* getMediaInfo();

    int getCurrentPosition(int* msec);
    int getDuration(int* msec);
    int reset();
    int setLooping(int bLoop);

    int callbackProcess(int messageId, void* param);

    int setVideoOutputScaleRatio(int horizonScaleDownRatio,int verticalScaleDownRatio);

    int mainThread();

    int setVolume(int volume);

    void *resData;

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
    sem_t               mSemSetVolume;
    sem_t               mSemSetSpeed;

    //* status control.
    int                 mSetDataSourceReply;
    int                 mPrepareReply;
    int                 mStartReply;
    int                 mStopReply;
    int                 mPauseReply;
    int                 mResetReply;
    int                 mSeekReply;
    int                 mPrepareFinishResult;   //* save the prepare result for prepare().
    int                 mSetVolumeReply;
    int                 mSetSpeedReply;

    int                 mPrepareSync;   //* synchroized prarare() call, don't call back to user.
    int                 mSeeking;
    int                 mSeekTime;
    int                 mSeekSync;  //* internal seek, don't call back to user.
    int                 mLoop;
    int                 mVolume;

	int                 mSpeed;
	int                 mbFast;
	int                 mFastTime;

    AwPlayer(const AwPlayer&);
    AwPlayer &operator=(const AwPlayer&);
};

#endif  // AWPLAYER
