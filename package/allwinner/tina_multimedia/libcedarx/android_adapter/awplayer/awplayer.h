/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : awplayer.h
* Description : mediaplayer adapter for operator
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : first version
*
*/

#ifndef AW_PLAYER_H
#define AW_PLAYER_H

#include <semaphore.h>
#include <pthread.h>
#include <media/MediaPlayerInterface.h>
#if (CONFIG_ANDROID_MAJOR_VER >= 8)
#include <media/BufferingSettings.h>
#endif

#if (CONFIG_ANDROID_MAJOR_VER >= 5)
#include <media/IMediaHTTPService.h>
#endif

using namespace android;

class AwPlayer : public MediaPlayerInterface
{
public:
    AwPlayer();
    virtual ~AwPlayer();

    virtual status_t    initCheck();

    virtual status_t    setUID(uid_t nUid);

#if (CONF_ANDROID_MAJOR_VER >= 5)
    virtual status_t    setDataSource(const sp<IMediaHTTPService> &httpService,
                        const char* pUrl, const KeyedVector<String8, String8>* pHeaders);
#else
    virtual status_t    setDataSource(const char* pUrl,
                        const KeyedVector<String8, String8>* pHeaders);
#endif
    virtual status_t    setDataSource(int fd, int64_t nOffset, int64_t nLength);
    virtual status_t    setDataSource(const sp<IStreamSource>& source);
#if (CONF_ANDROID_MAJOR_VER >= 6)
    virtual status_t    setDataSource(const sp<DataSource> &dataSource);
#endif
#if !((CONF_ANDROID_MAJOR_VER == 4)&&(CONF_ANDROID_SUB_VER == 2))
    //* android 4.4 use IGraphicBufferProducer instead of ISurfaceTexture in android 4.2.
    virtual status_t    setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer);
#else
    virtual status_t    setVideoSurfaceTexture(const sp<ISurfaceTexture>& surfaceTexture);
#endif

    virtual status_t    prepare();
    virtual status_t    prepareAsync();
    virtual status_t    start();
    virtual status_t    stop();
    virtual status_t    pause();
    virtual bool        isPlaying();

#if (CONF_ANDROID_MAJOR_VER >= 8)
    virtual status_t    seekTo(int nSeekTimeMs, MediaPlayerSeekMode mode);
#else
    virtual status_t    seekTo(int nSeekTimeMs);
#endif
    virtual status_t    setSpeed(int nSpeed);

    virtual status_t    getCurrentPosition(int* msec);
    virtual status_t    getDuration(int* msec);
    virtual status_t    reset();
    virtual status_t    setLooping(int bLoop);

    virtual player_type playerType();   //* return AW_PLAYER

    virtual status_t    invoke(const Parcel &request, Parcel *reply);
    virtual void        setAudioSink(const sp<AudioSink>& audioSink);

#if (CONF_ANDROID_MAJOR_VER > 5)
    virtual status_t    setPlaybackSettings(const AudioPlaybackRate &rate);
    virtual status_t    getPlaybackSettings(AudioPlaybackRate *rate);
#endif

    virtual status_t    setParameter(int key, const Parcel& request);
    virtual status_t    getParameter(int key, Parcel* reply);

    virtual status_t    getMetadata(const media::Metadata::Filter& ids, Parcel* records);

    //* this method setSubCharset(const char* charset) is added by allwinner.
    virtual status_t    setSubCharset(const char* charset);
    virtual status_t    getSubCharset(char *charset);
    virtual status_t    setSubDelay(int nTimeMs);
    virtual int         getSubDelay();
    virtual status_t    callbackProcess(int messageId, int ext1, void* param);
#if (CONF_ANDROID_MAJOR_VER >= 8)
    virtual status_t    setBufferingSettings(const BufferingSettings& buffering);
    virtual status_t    getBufferingSettings(BufferingSettings* pBuffering);
#endif

private:

    AwPlayer(const AwPlayer&);
    AwPlayer &operator=(const AwPlayer&);

#if (CONF_ANDROID_MAJOR_VER >= 8)
    void updateMetrics(const char *where);
#endif

    struct PlayerPriData *mPriData;

    static struct Instance {
        int count;
        pthread_mutex_t lock;
    } instance;
};


#endif  // AWPLAYER
