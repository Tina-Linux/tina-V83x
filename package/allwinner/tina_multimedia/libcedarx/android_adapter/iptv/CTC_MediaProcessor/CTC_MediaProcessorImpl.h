/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CTC_MediaProcessorImpl.h
 * Description : IPTV API
 * History :
 *
 */

#ifndef _CTC_MEDIAPROCESSORIMPL_H_
#define _CTC_MEDIAPROCESSORIMPL_H_

#include "CTC_MediaProcessor.h"
#include "player.h"
#include "tsdemux.h"
#include "media/AudioSystem.h"
#include <jni.h>
#define SwitchAudioSeamless (1)

#define DEBUG_WRITE_DATA_SPEED (0)

using namespace android;

/**
 * @CTC_MediaprocessorImpl
 */
class CTC_MediaProcessorImpl : public CTC_MediaProcessor
{
public:
    CTC_MediaProcessorImpl();
    virtual ~CTC_MediaProcessorImpl();
    virtual void InitVideo(PVIDEO_PARA_T pVideoPara);
    virtual void InitAudio(PAUDIO_PARA_T pAudioPara);
    virtual bool StartPlay();
    virtual bool Pause();
    virtual bool Resume();
    virtual bool Fast();
    virtual bool StopFast();
    virtual bool Stop();
    virtual bool Seek();
#if NewInterface
    virtual int WriteData(unsigned char* pBuffer, unsigned int nSize);
#else
    virtual int    GetWriteBuffer(IPTV_PLAYER_STREAMTYPE_e type,
                    unsigned char** pBuffer, unsigned int *nSize);
    virtual int WriteData(IPTV_PLAYER_STREAMTYPE_e type, unsigned char* pBuffer,
                    unsigned int nSize, uint64_t timestamp);
#endif

#if NewInterface
    virtual bool SetRatio(int nRatio);
#else
    virtual void SetContentMode(IPTV_PLAYER_CONTENTMODE_e contentMode);
#endif

#if NewInterface
    virtual void SwitchAudioTrack(int pid);
#else
    virtual void SwitchAudioTrack(int pid, PAUDIO_PARA_T pAudioPara);
#endif
    virtual void SwitchSubtitle(int pid);
    virtual void SetProperty(int nType, int nSub, int nValue);
    virtual long GetCurrentPlayTime();
    virtual void leaveChannel();
    virtual int  GetAbendNum();
    virtual bool IsSoftFit();
    virtual void SetEPGSize(int w, int h);
    virtual bool SetVolume(int volume);
    virtual int GetVolume();

    virtual int    GetPlayMode();
    virtual void GetVideoPixels(int& width, int& height);
    virtual int SetVideoWindow(int x,int y,int width,int height);
    virtual int VideoShow();
    virtual int VideoHide();
    virtual int GetAudioBalance();
    virtual bool SetAudioBalance(int nAudioBalance);
    virtual void SetSurface(Surface *pSurface);
    virtual void playerback_register_evt_cb(IPTV_PLAYER_EVT_CB pfunc, void *handler);
#if !NewInterface
    virtual void SetStopMode(bool bHoldLastPic);
    virtual bool GetIsEos();
    virtual int GetBufferStatus(long *totalsize, long *datasize);
#endif

    //* process callback from player.
    virtual int ProcessCallback(int eMessageId, void* param);

    virtual int QueryBuffer(Player* pl, int nRequireSize, int MediaType);

    virtual int RequestBuffer(unsigned char** ppBuf, unsigned int* pSize, int MediaType);

    virtual int UpdateData(int64_t pts, int nDataSize, int bIsFirst, int bIsLast, int MediaType);

    virtual int OpenTsDemux(void *mDemux, int nPid, int MediaType, int Format);

    virtual int CloseTsDemux(void *mDemux, int nPid);

    virtual int SetAudioInfo(PAUDIO_PARA_T pAudioPara);

    virtual int SetVideoInfo(PVIDEO_PARA_T pVideoPara);

    static void* WriteDataThread(void* pArg);
private:
    int setVolumeJNI(int index);
    int getVolumeJNI();
private:
    IPTV_PLAYER_EVT_CB  m_callBackFunc;
    void*               m_eventHandler;
    sp<ANativeWindow>   mNativeWindow;
    sp<Surface>         mSurface;
    Player*             mPlayer;
    void*               mDemux;
    bool                mIsInited;
    bool                mIsEos;
    bool                mHoldLastPicture;
    void*               mVideoStreamBuf;
    unsigned int        mVideoStreamBufSize;
    void*               mAudioStreamBuf;
    unsigned int        mAudioStreamBufSize;
    void*               mTsStreamBuf;
    unsigned int        mTsStreamBufSize;

    unsigned short      mVideoPids;
    unsigned short      mAudioPids[32];
    int                 mAudioTrackCount;
    int                 mCurrentAudioTrack;

    mutable Mutex         mLock;
    VIDEO_PARA_T        mVideoPara;
    AUDIO_PARA_T        mAudioPara;
    int                 mLeftBytes;
    char*                  mVideoBufForTsDemux;
    char*                  mAudioBufForTsDemux;

    bool                mIsQuitedRequest;
    bool                mIsQuited;

    IPTV_PLAYER_STREAMTYPE_e mStreamType;

    int                 mVideoX;
    int                 mVideoY;
    int                 mVideoWidth;
    int                 mVideoHeight;
    bool                mIsSetVideoPosition;

    pthread_t           mWriteDataThreadId;

    bool                mSeekRequest;
    int                 mSeekReply;
    pthread_cond_t      mCond;
    pthread_mutex_t     mSeekMutex;

    JNIEnv*             mEnv;

    int                 mPlayFlag;
    int                 mStart;

    int                 bBufferring;

    unsigned char*      mDataBuffer;
    int                 mValidDataSize;
    unsigned int        mDataBufferSize;
    unsigned char*      mDataWritePtr;
    unsigned char*      mDataReadPtr;
    pthread_mutex_t     mBufMutex;


    int                 isH265;
    int                 mFirstStartPlay;
    int                 mFirstCache;
    int64_t             mStartCacheTime;


#if    DEBUG_WRITE_DATA_SPEED
    int64_t mTotalSize;
    int64_t mNowtime;
    int64_t mPrintTime;
    int     mFirst;
#endif
};

#endif
