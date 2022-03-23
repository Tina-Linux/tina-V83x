
#include "log.h"

#include "player_i.h"
#include "audioDecComponent.h"
#include "videoDecComponent.h"
#include "subtitleDecComponent.h"
#include "audioRenderComponent.h"
#include "videoRenderComponent.h"
#include "subtitleRenderComponent.h"
#include "messageQueue.h"
#include "avtimer.h"
#include "bitrateEstimater.h"
#include "framerateEstimater.h"

#define CONFIG_DISABLE_VIDEO    (0)
#define CONFIG_DISABLE_AUDIO    (0)
#define CONFIG_DISABLE_SUBTITLE (0)

#define CdxAbs(a) ((a)>0 ? (a) : -(a))
#if(CONFIG_DTV==OPTION_DTV_YES)
#define	IS_DTMB_STREAM	(1)
#else
#define	IS_DTMB_STREAM	(0)
#endif

typedef struct PlayerContext
{
    VideoStreamInfo*            pVideoStreamInfo;

    AudioStreamInfo*            pAudioStreamInfo;

    SubtitleStreamInfo*         pSubtitleStreamInfo;

    int                         nAudioStreamNum;
    int                         nAudioStreamSelected;
    int                         nSubtitleStreamNum;
    int                         nSubtitleStreamSelected;
    void*                       pNativeWindow;
    void*                       pAudioSink;

    VideoRenderComp*            pVideoRender;
    AudioRenderComp*            pAudioRender;
    SubtitleRenderComp*         pSubtitleRender;
    VideoDecComp*               pVideoDecComp;
    AudioDecComp*               pAudioDecComp;
    SubtitleDecComp*            pSubtitleDecComp;
    AvTimer*                    pAvTimer;

    BitrateEstimater*           pVideoBitrateEstimater;
    BitrateEstimater*           pAudioBitrateEstimater;
    FramerateEstimater*         pVideoFramerateEstimater;

    enum EPLAYERSTATUS          eStatus;

    PlayerCallback              callback;
    void*                       pUserData;

    //* buffer for crashed media stream.
#if CONFIG_DISABLE_VIDEO
    void*                       pVideoStreamBuffer;
    int                         nVideoStreamBufferSize;
#endif
#if CONFIG_DISABLE_AUDIO
    void*                       pAudioStreamBuffer;
    int                         nAudioStreamBufferSize;
#endif
#if CONFIG_DISABLE_SUBTITLE
    void*                       pSubtitleStreamBuffer;
    int                         nSubtitleStreamBufferSize;
#endif

#if DROP_3D_SECOND_VIDEO_STREAM
    void*                       pVideoSecondStreamBuffer;
    int                         nVideoSecondStreamBufferSize;
#endif

    int                         bProcessingCommand;
    int64_t                     nFirstVideoRenderPts;
    int64_t                     nFirstAudioRenderPts;
    int64_t                     nTimeBase;
    int64_t                     nTimeOffset;
    pthread_mutex_t             timerMutex;
    int64_t                     nLastTimeTimerAdjusted;
    int onResetNotSync;
    int64_t ptsBeforeReset;

    int64_t                     nPreVideoPts;
    int64_t                     nPreAudioPts;
    int                         nUnsyncVideoFrameCnt;
    int64_t                     nLastInputPts;          //* for pts loop back detect in subtitle pts notify process.
    int64_t						nFirstPts;

    int                         bStreamEosSet;
    int                         bVideoRenderEosReceived;
    int                         bAudioRenderEosReceived;
    pthread_mutex_t             eosMutex;

    int                         bInFastMode;

    VConfig                     sVideoConfig;

    int                         sVideoCropWindow[4];

    //* audio decoder mutex, currently for synchronization of the switch audio
    //* operation and the demux sending job.
    pthread_mutex_t             audioDecoderMutex;
    pthread_mutex_t             subtitleDecoderMutex;

    int                         bVideoCrash;
    int                         bAudioCrash;
    int                         bSubtitleCrash;

    //*
    int                         bUnSurpportVideoFlag;
    void*                       pUnSurpportVideoBuffer;
    int                         nUnSurpportVideoBufferSize;

	float						volume;
	int                         bDiscardAudio;
	LayerControlOpsT*           pLayerControlOps;
	SoundControlOpsT*           pSoundControlOps;
}PlayerContext;


static int CallbackProcess(void* pSelf, int eMessageId, void* param);
static int PlayerInitialVideo(PlayerContext* p);
static int PlayerInitialAudio(PlayerContext* p);
static int PlayerInitialSubtitle(PlayerContext* p);


Player* PlayerCreate(void)
{
    PlayerContext* p;

    AddVDPlugin();
    p = (PlayerContext*)malloc(sizeof(PlayerContext));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }
    memset(p, 0, sizeof(*p));
	//p->volume = -1.0;
	p->volume = 0.0;
	p->nTimeOffset = 0;
	p->nTimeBase = -1;
    p->eStatus = PLAYER_STATUS_STOPPED;
    p->pAvTimer = AvTimerCreate();
    p->onResetNotSync = 0;
    pthread_mutex_init(&p->eosMutex, NULL);
    pthread_mutex_init(&p->timerMutex, NULL);
    pthread_mutex_init(&p->audioDecoderMutex, NULL);
    pthread_mutex_init(&p->subtitleDecoderMutex, NULL);

#if CONFIG_DISABLE_VIDEO
    p->nVideoStreamBufferSize = 256*1024;
    p->pVideoStreamBuffer = malloc(p->nVideoStreamBufferSize);
    if(p->pVideoStreamBuffer == NULL)
    {
        loge("allocate memory fail.");
        return NULL;
    }
#endif
#if CONFIG_DISABLE_AUDIO
    p->nAudioStreamBufferSize = 128*1024;
    p->pAudioStreamBuffer = malloc(p->nAudioStreamBufferSize);
    if(p->pAudioStreamBuffer == NULL)
    {
        loge("allocate memory fail.");
        return NULL;
    }
#endif
#if CONFIG_DISABLE_SUBTITLE
    p->nSubtitleStreamBufferSize = 256*1024;
    p->pSubtitleStreamBuffer = malloc(p->nSubtitleStreamBufferSize);
    if(p->pSubtitleStreamBuffer == NULL)
    {
        loge("allocate memory fail.");
        return NULL;
    }
#endif

#if DROP_3D_SECOND_VIDEO_STREAM
    p->nVideoSecondStreamBufferSize = 256*1024;
    p->pVideoSecondStreamBuffer     = malloc(p->nVideoSecondStreamBufferSize);
    if(p->pVideoSecondStreamBuffer == NULL)
    {
        loge("allocate memory fail.");
        return NULL;
    }
#endif

    p->sVideoConfig.bDispErrorFrame = 1;   // set the default value of the bDispErrorFrame is 1
    return (Player*)p;
}


int PlayerSetControlOps(Player* pl, LayerControlOpsT* layerOps, SoundControlOpsT* soundOps)
{
	PlayerContext* p;

    p = (PlayerContext*)pl;
    p->pLayerControlOps = layerOps;
    p->pSoundControlOps = soundOps;
	logd("=== set player control ops");

    return 0;
}

void PlayerDestroy(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    PlayerClear(pl);

    if(p->pAvTimer != NULL)
    {
        AvTimerDestroy(p->pAvTimer);
        p->pAvTimer = NULL;
    }

    pthread_mutex_destroy(&p->eosMutex);
    pthread_mutex_destroy(&p->timerMutex);
    pthread_mutex_destroy(&p->audioDecoderMutex);
    pthread_mutex_destroy(&p->subtitleDecoderMutex);


#if CONFIG_DISABLE_VIDEO
    if(p->pVideoStreamBuffer != NULL)
    {
        free(p->pVideoStreamBuffer);
        p->pVideoStreamBuffer = NULL;
    }
#endif
#if CONFIG_DISABLE_AUDIO
    if(p->pAudioStreamBuffer != NULL)
    {
        free(p->pAudioStreamBuffer);
        p->pAudioStreamBuffer = NULL;
    }
#endif
#if CONFIG_DISABLE_SUBTITLE
    if(p->pSubtitleStreamBuffer != NULL)
    {
        free(p->pSubtitleStreamBuffer);
        p->pSubtitleStreamBuffer = NULL;
    }
#endif

#if DROP_3D_SECOND_VIDEO_STREAM
    if(p->pVideoSecondStreamBuffer != NULL)
    {
        free(p->pVideoSecondStreamBuffer);
        p->pVideoSecondStreamBuffer = NULL;
    }
#endif

    if(p->pUnSurpportVideoBuffer != NULL)
    {
        free(p->pUnSurpportVideoBuffer);
        p->pUnSurpportVideoBuffer = NULL;
    }

    free(p);

    return;
}


int PlayerSetVideoStreamInfo(Player* pl, VideoStreamInfo* pStreamInfo)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

#if CONFIG_DISABLE_VIDEO
    return 0;
#else
    if(p->pVideoStreamInfo != NULL)
    {
        if(p->pVideoStreamInfo->pCodecSpecificData != NULL &&
           p->pVideoStreamInfo->nCodecSpecificDataLen > 0)
        {
            free(p->pVideoStreamInfo->pCodecSpecificData);
            p->pVideoStreamInfo->pCodecSpecificData = NULL;
            p->pVideoStreamInfo->nCodecSpecificDataLen = 0;
        }

        free(p->pVideoStreamInfo);
        p->pVideoStreamInfo = NULL;
    }

    if(pStreamInfo)
    {
        p->pVideoStreamInfo = (VideoStreamInfo*)malloc(sizeof(VideoStreamInfo));
        if(p->pVideoStreamInfo == NULL)
        {
            loge("malloc memory fail.");
            return -1;
        }

        memcpy(p->pVideoStreamInfo, pStreamInfo, sizeof(VideoStreamInfo));
        if(pStreamInfo->pCodecSpecificData != NULL &&
           pStreamInfo->nCodecSpecificDataLen > 0)
        {
            p->pVideoStreamInfo->pCodecSpecificData = (char*)malloc(pStreamInfo->nCodecSpecificDataLen);
            if(p->pVideoStreamInfo->pCodecSpecificData == NULL)
            {
                loge("malloc memory fail.");
                free(p->pVideoStreamInfo);
                p->pVideoStreamInfo = NULL;
                return -1;
            }
            memcpy(p->pVideoStreamInfo->pCodecSpecificData,
                   pStreamInfo->pCodecSpecificData,
                   pStreamInfo->nCodecSpecificDataLen);
        }
    }

    if(PlayerInitialVideo(p) == 0)
    {
        if(p->eStatus != PLAYER_STATUS_STOPPED)
        {
            VideoRenderCompStart(p->pVideoRender);
            if(p->eStatus == PLAYER_STATUS_PAUSED)
                VideoRenderCompPause(p->pVideoRender);
            VideoDecCompStart(p->pVideoDecComp);
            if(p->eStatus == PLAYER_STATUS_PAUSED)
                VideoDecCompPause(p->pVideoDecComp);
        }
        return 0;
    }
    else
    {
        return -1;
    }
#endif
}


int PlayerSetAudioStreamInfo(Player* pl, AudioStreamInfo* pStreamInfo, int nStreamNum, int nDefaultStream)
{
    PlayerContext* p;
    int            i;

    p = (PlayerContext*)pl;

    if(nStreamNum == 0)
    {
        loge("have no audio stream, wouldn't init audio player");
        return 0;
    }

#if CONFIG_DISABLE_AUDIO
    return 0;
#else
    if(p->pAudioStreamInfo != NULL && p->nAudioStreamNum > 0)
    {
        for(i=0; i<p->nAudioStreamNum; i++)
        {
            if(p->pAudioStreamInfo[i].pCodecSpecificData != NULL &&
               p->pAudioStreamInfo[i].nCodecSpecificDataLen > 0)
            {
                free(p->pAudioStreamInfo[i].pCodecSpecificData);
                p->pAudioStreamInfo[i].pCodecSpecificData = NULL;
                p->pAudioStreamInfo[i].nCodecSpecificDataLen = 0;
            }
        }

        free(p->pAudioStreamInfo);
        p->pAudioStreamInfo = NULL;
    }

    p->nAudioStreamNum = 0;
    p->nAudioStreamSelected = 0;

    if(pStreamInfo != NULL && nStreamNum > 0)
    {
        p->pAudioStreamInfo = (AudioStreamInfo*)malloc(sizeof(AudioStreamInfo) * nStreamNum);
        if(p->pAudioStreamInfo == NULL)
        {
            loge("malloc memory fail.");
            return -1;
        }

        memcpy(p->pAudioStreamInfo, pStreamInfo, sizeof(AudioStreamInfo)*nStreamNum);

        for(i=0; i<nStreamNum; i++)
        {
            if(pStreamInfo[i].pCodecSpecificData != NULL &&
               pStreamInfo[i].nCodecSpecificDataLen > 0)
            {
                p->pAudioStreamInfo[i].pCodecSpecificData = (char*)malloc(pStreamInfo[i].nCodecSpecificDataLen);
                if(p->pAudioStreamInfo[i].pCodecSpecificData == NULL)
                {
                    loge("malloc memory fail.");
                    break;
                }
                memcpy(p->pAudioStreamInfo[i].pCodecSpecificData,
                       pStreamInfo[i].pCodecSpecificData,
                       pStreamInfo[i].nCodecSpecificDataLen);
            }
        }

        if(i != nStreamNum)
        {
            i--;
            for(; i>=0; i--)
            {
                if(p->pAudioStreamInfo[i].pCodecSpecificData != NULL &&
                   p->pAudioStreamInfo[i].nCodecSpecificDataLen > 0)
                {
                    free(p->pAudioStreamInfo[i].pCodecSpecificData);
                    p->pAudioStreamInfo[i].pCodecSpecificData = NULL;
                    p->pAudioStreamInfo[i].nCodecSpecificDataLen = 0;
                }
            }

            free(p->pAudioStreamInfo);
            p->pAudioStreamInfo = NULL;
            return -1;
        }

        p->nAudioStreamSelected = nDefaultStream;
        p->nAudioStreamNum = nStreamNum;
    }

    if(PlayerInitialAudio(p) == 0)
    {
        if(p->eStatus != PLAYER_STATUS_STOPPED)
        {
            AudioRenderCompStart(p->pAudioRender);
            if(p->eStatus == PLAYER_STATUS_PAUSED)
                AudioRenderCompPause(p->pAudioRender);
            AudioDecCompStart(p->pAudioDecComp);
            if(p->eStatus == PLAYER_STATUS_PAUSED)
                AudioDecCompPause(p->pAudioDecComp);
        }
        return 0;
    }
    else
    {
        return -1;
    }
#endif
}


int PlayerAddAudioStream(Player* pl, AudioStreamInfo* pStreamInfo)
{
    PlayerContext*   p;
    AudioStreamInfo* newArr;
    int              nStreamCount;
    int              ret;

    p = (PlayerContext*)pl;

    if(p->pAudioStreamInfo != NULL && p->nAudioStreamNum > 0)
    {
        nStreamCount = p->nAudioStreamNum + 1;
        newArr = (AudioStreamInfo*)malloc(sizeof(AudioStreamInfo)*nStreamCount);
        if(newArr == NULL)
        {
            loge("malloc memory fail.");
            return -1;
        }

        memcpy(newArr, p->pAudioStreamInfo, p->nAudioStreamNum*sizeof(AudioStreamInfo));
        memcpy(&newArr[nStreamCount-1], pStreamInfo, sizeof(AudioStreamInfo));
        if(pStreamInfo->pCodecSpecificData != NULL && pStreamInfo->nCodecSpecificDataLen > 0)
        {
            newArr[nStreamCount-1].pCodecSpecificData = (char*)malloc(pStreamInfo->nCodecSpecificDataLen);
            if(newArr[nStreamCount-1].pCodecSpecificData == NULL)
            {
                loge("malloc memory fail.");
                free(newArr);
                return -1;
            }
            memcpy(newArr[nStreamCount-1].pCodecSpecificData,
                   pStreamInfo->pCodecSpecificData,
                   pStreamInfo->nCodecSpecificDataLen);
        }

        if(p->pAudioDecComp != NULL)
        {
            if(AudioDecCompGetStatus(p->pAudioDecComp) == PLAYER_STATUS_STARTED)
            {
                AudioDecCompPause(p->pAudioDecComp);
                ret = AudioDecCompAddAudioStream(p->pAudioDecComp, &newArr[nStreamCount-1]);
                AudioDecCompStart(p->pAudioDecComp);
            }
            else
                ret = AudioDecCompAddAudioStream(p->pAudioDecComp, &newArr[nStreamCount-1]);

            if(ret != 0)
            {
                if(newArr[nStreamCount-1].pCodecSpecificData != NULL &&
                   newArr[nStreamCount-1].nCodecSpecificDataLen != 0)
                {
                    free(newArr[nStreamCount-1].pCodecSpecificData);
                    newArr[nStreamCount-1].nCodecSpecificDataLen = 0;
                    newArr[nStreamCount-1].pCodecSpecificData = NULL;
                }
                free(newArr);
                return -1;
            }
        }

        free(p->pAudioStreamInfo);
        p->pAudioStreamInfo = newArr;
        p->nAudioStreamNum  = nStreamCount;

        return 0;
    }
    else
    {
        return PlayerSetAudioStreamInfo(pl, pStreamInfo, 1, 0);
    }
}


int PlayerGetAudioStreamCnt(Player* pl)
{
    PlayerContext* p;
    p = (PlayerContext*)pl;
    return p->nAudioStreamNum;
}


int PlayerCanSupportAudioStream(Player* pl, AudioStreamInfo* pStreamInfo)
{
    PlayerContext* p;

    CEDARX_UNUSE(pStreamInfo);

    p = (PlayerContext*)pl;
    return 1;
}

int PlayerCanSupportVideoStream(Player* pl, VideoStreamInfo* pStreamInfo)
{
    PlayerContext* p;

	CEDARX_UNUSE(pStreamInfo);

    p = (PlayerContext*)pl;
    return 1;
}

int PlayerSetDiscardAudio(Player* pl, int f)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    p->bDiscardAudio = f;

    return 0;
}


int PlayerSetWindow(Player* pl, void* pNativeWindow)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    p->pNativeWindow = pNativeWindow;

    if(p->pVideoRender != NULL)
    {
        return VideoRenderCompSetWindow(p->pVideoRender, pNativeWindow);
    }

    return 0;
}


int PlayerSetAudioSink(Player* pl, void* pAudioSink)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    p->pAudioSink = pAudioSink;
    if(p->pAudioRender != NULL)
    {
        return AudioRenderCompSetAudioSink(p->pAudioRender, pAudioSink);
    }

    return 0;
}


int PlayerSetCallback(Player* pl, PlayerCallback callback, void* pUserData)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    p->callback  = callback;
    p->pUserData = pUserData;
    return 0;
}


int PlayerHasVideo(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pVideoRender != NULL)
        return 1;
    else
        return 0;
}


int PlayerHasAudio(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pAudioRender != NULL)
        return 1;
    else
        return 0;
}



int PlayerStart(Player* pl)
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;

    logd("player start");

    if(p->eStatus == PLAYER_STATUS_STARTED)
    {
        loge("invalid start operation, player already in started status.");
        return -1;
    }

    p->bProcessingCommand = 1;

    if(p->eStatus == PLAYER_STATUS_PAUSED)
    {
        if(p->bInFastMode)
        {
            //* set timer to the max value to make the audio render to discard all data.
            p->pAvTimer->SetTime(p->pAvTimer, 0x7000000000000000LL);
        }
        else
        {
            if(p->nTimeBase != -1)
                p->pAvTimer->Start(p->pAvTimer);
        }

        //* resume components.
        if(p->pVideoDecComp != NULL)
            VideoDecCompStart(p->pVideoDecComp);
        if(p->pAudioDecComp != NULL)
            AudioDecCompStart(p->pAudioDecComp);
        if(p->pSubtitleDecComp != NULL)
            SubtitleDecCompStart(p->pSubtitleDecComp);
        if(p->pVideoRender != NULL)
            VideoRenderCompStart(p->pVideoRender);
        if(p->pAudioRender != NULL)
            AudioRenderCompStart(p->pAudioRender);
        if(p->pSubtitleRender != NULL)
            SubtitleRenderCompStart(p->pSubtitleRender);
    }
    else
    {
        if((p->pVideoDecComp == NULL || p->pVideoRender == NULL) &&
           (p->pAudioDecComp == NULL || p->pAudioRender == NULL))
        {
            loge("neither video nor audio stream exits.");
            return -1;
        }

        p->nFirstVideoRenderPts    = -1;
        p->nFirstAudioRenderPts    = -1;
        p->bStreamEosSet           = 0;
        p->bVideoRenderEosReceived = 0;
        p->bAudioRenderEosReceived = 0;
        p->nLastTimeTimerAdjusted  = 0;
        p->nPreVideoPts            = -1;
        p->nPreAudioPts            = -1;
        p->nUnsyncVideoFrameCnt    = 0;
        p->nLastInputPts           = -1;

        //* nTimeBase==-1 means timer will be started at the first pts callback.
        p->nTimeBase               = -1;

        if(p->pVideoBitrateEstimater)
            BitrateEstimaterReset(p->pVideoBitrateEstimater);
        if(p->pAudioBitrateEstimater)
            BitrateEstimaterReset(p->pAudioBitrateEstimater);
        if(p->pVideoFramerateEstimater)
            FramerateEstimaterReset(p->pVideoFramerateEstimater);

        p->pAvTimer->Stop(p->pAvTimer);
        if(p->bInFastMode)
        {
            //* set timer to a big value(but not easy to loop back) to make the audio render to discard all data.
            p->pAvTimer->SetTime(p->pAvTimer, 0x7000000000000000LL);
        }
        else
            p->pAvTimer->SetTime(p->pAvTimer, 0);
        p->pAvTimer->SetSpeed(p->pAvTimer, 1000);

        //* start components.
        //* must start the decoder components first, because render component thread will call methods like
        //* AudioDecCompRequestPcmData() which need access audio or subtitle decoders inside decoder components,
        //* if we start the render components first, decoders are not created yet and this will cause the
        //* render thread crash when calling methods like AudioDecCompRequestPcmData().
        if(p->pVideoDecComp != NULL)
            VideoDecCompStart(p->pVideoDecComp);
        if(p->pAudioDecComp != NULL)
            AudioDecCompStart(p->pAudioDecComp);
        if(p->pSubtitleDecComp != NULL)
            SubtitleDecCompStart(p->pSubtitleDecComp);
        if(p->pVideoRender != NULL)
            VideoRenderCompStart(p->pVideoRender);
        if(p->pAudioRender != NULL)
            AudioRenderCompStart(p->pAudioRender);
        if(p->pSubtitleRender != NULL)
            SubtitleRenderCompStart(p->pSubtitleRender);
    }

    p->bProcessingCommand = 0;
    p->eStatus = PLAYER_STATUS_STARTED;

    return 0;
}


int PlayerStop(Player* pl)      //* media stream information is still kept by the player.
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;

    logi("player stop");

    if(p->eStatus == PLAYER_STATUS_STOPPED)
    {
        loge("invalid stop operation, player already in stopped status.");
        return -1;
    }

    p->bProcessingCommand = 1;

    //* stop components.
    //* must stop the render components first, because render component thread will call methods like
    //* AudioDecCompRequestPcmData() which need access audio or subtitle decoders inside decoder components,
    //* if we stop the decoder components first, decoders are destroyed and this will cause the render
    //* thread crash when calling methods like AudioDecCompRequestPcmData().
    if(p->pSubtitleRender != NULL)
        SubtitleRenderCompStop(p->pSubtitleRender);
    if(p->pAudioRender != NULL)
        AudioRenderCompStop(p->pAudioRender);
    if(p->pVideoRender != NULL)
        VideoRenderCompStop(p->pVideoRender);
    if(p->pSubtitleDecComp != NULL)
        SubtitleDecCompStop(p->pSubtitleDecComp);
    if(p->pAudioDecComp != NULL)
        AudioDecCompStop(p->pAudioDecComp);
    if(p->pVideoDecComp != NULL)
        VideoDecCompStop(p->pVideoDecComp);

    p->pAvTimer->Stop(p->pAvTimer);
    p->pAvTimer->SetTime(p->pAvTimer, 0);
	p->nTimeBase = -1;
	p->nTimeOffset = 0;
    p->bProcessingCommand = 0;
    p->bInFastMode        = 0;
    p->eStatus = PLAYER_STATUS_STOPPED;
    p->bVideoCrash = 0;
    p->bAudioCrash = 0;
    p->bSubtitleCrash = 0;
    p->bDiscardAudio  = 0;

    return 0;
}


int PlayerPause(Player* pl)
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;

    if(p->eStatus != PLAYER_STATUS_STARTED)
    {
        loge("invalid pause operation, player not in started status.");
        return -1;
    }

    p->bProcessingCommand = 1;

    //* pause components.
    if(p->pVideoRender != NULL)
        VideoRenderCompPause(p->pVideoRender);
    if(p->pAudioRender != NULL)
        AudioRenderCompPause(p->pAudioRender);
    if(p->pSubtitleRender != NULL)
        SubtitleRenderCompPause(p->pSubtitleRender);
    if(p->pVideoDecComp != NULL)
        VideoDecCompPause(p->pVideoDecComp);
    if(p->pAudioDecComp != NULL)
        AudioDecCompPause(p->pAudioDecComp);
    if(p->pSubtitleDecComp != NULL)
        SubtitleDecCompPause(p->pSubtitleDecComp);

    p->pAvTimer->Stop(p->pAvTimer);

    p->bProcessingCommand = 0;
    p->eStatus = PLAYER_STATUS_PAUSED;

    return 0;
}


enum EPLAYERSTATUS PlayerGetStatus(Player* pl)
{
    PlayerContext* p;
    int              ret;
    p = (PlayerContext*)pl;
    return p->eStatus;
}


int PlayerReset(Player* pl, int64_t nSeekTimeUs)     //* for seek operation, mute be called under paused status.
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;
	//* nTimeBase==-1 means timer will be started at the first pts callback.
    p->nTimeBase               = -1;
	p->nTimeOffset             = nSeekTimeUs;
	logd("in PlayerReset():p->nTimeOffset = %lld us",p->nTimeOffset);
    if(p->eStatus == PLAYER_STATUS_STOPPED)
    {
        loge("invalid reset operation, should be called under pause status.");
        return -1;
    }

    p->bProcessingCommand = 1;

    if(p->eStatus == PLAYER_STATUS_STARTED)
    {
        //* pause components.
        if(p->pVideoRender != NULL)
            VideoRenderCompPause(p->pVideoRender);
        if(p->pAudioRender != NULL)
            AudioRenderCompPause(p->pAudioRender);
        if(p->pSubtitleRender != NULL)
            SubtitleRenderCompPause(p->pSubtitleRender);
        if(p->pVideoDecComp != NULL)
            VideoDecCompPause(p->pVideoDecComp);
        if(p->pAudioDecComp != NULL)
            AudioDecCompPause(p->pAudioDecComp);
        if(p->pSubtitleDecComp != NULL)
            SubtitleDecCompPause(p->pSubtitleDecComp);
    }

    //* reset components.
    if(p->pVideoDecComp != NULL)
        VideoDecCompReset(p->pVideoDecComp);
    if(p->pAudioDecComp != NULL)
        AudioDecCompReset(p->pAudioDecComp, nSeekTimeUs);
    if(p->pSubtitleDecComp != NULL)
        SubtitleDecCompReset(p->pSubtitleDecComp, nSeekTimeUs);
    if(p->pVideoRender != NULL)
        VideoRenderCompReset(p->pVideoRender);
    if(p->pAudioRender != NULL)
        AudioRenderCompReset(p->pAudioRender);
    if(p->pSubtitleRender != NULL)
        SubtitleRenderCompReset(p->pSubtitleRender);

    p->pAvTimer->Stop(p->pAvTimer);
    p->pAvTimer->SetTime(p->pAvTimer, 0);

    p->nFirstVideoRenderPts    = -1;
    p->nFirstAudioRenderPts    = -1;
    p->bStreamEosSet           = 0;
    p->bVideoRenderEosReceived = 0;
    p->bAudioRenderEosReceived = 0;
    p->nLastTimeTimerAdjusted  = 0;
    p->nPreVideoPts            = -1;
    p->nPreAudioPts            = -1;
    p->nUnsyncVideoFrameCnt    = 0;
    p->nLastInputPts           = -1;
    p->bVideoCrash             = 0;
    p->bAudioCrash             = 0;
    p->bSubtitleCrash          = 0;

    if(p->pVideoBitrateEstimater)
        BitrateEstimaterReset(p->pVideoBitrateEstimater);
    if(p->pAudioBitrateEstimater)
        BitrateEstimaterReset(p->pAudioBitrateEstimater);
    if(p->pVideoFramerateEstimater)
        FramerateEstimaterReset(p->pVideoFramerateEstimater);

    p->bProcessingCommand = 0;

    if(p->eStatus == PLAYER_STATUS_STARTED)
    {
        //* start components.
        //* must start the decoder components first, because render component thread will call methods like
        //* AudioDecCompRequestPcmData() which need access audio or subtitle decoders inside decoder components,
        //* if we start the render components first, decoders are not created yet and this will cause the
        //* render thread crash when calling methods like AudioDecCompRequestPcmData().
        if(p->pVideoRender != NULL)
            VideoRenderCompStart(p->pVideoRender);
        if(p->pAudioRender != NULL)
            AudioRenderCompStart(p->pAudioRender);
        if(p->pSubtitleRender != NULL)
            SubtitleRenderCompStart(p->pSubtitleRender);
        if(p->pVideoDecComp != NULL)
            VideoDecCompStart(p->pVideoDecComp);
        if(p->pAudioDecComp != NULL)
            AudioDecCompStart(p->pAudioDecComp);
        if(p->pSubtitleDecComp != NULL)
            SubtitleDecCompStart(p->pSubtitleDecComp);
    }

    return 0;
}


int PlayerClear(Player* pl)     //* must be called under stopped status, all stream information cleared.
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        loge("invalid clear operation, player is not in stopped status.");
        return -1;
    }

    p->bProcessingCommand      = 1;
    p->nFirstVideoRenderPts    = -1;
    p->nFirstAudioRenderPts    = -1;
    p->bStreamEosSet           = 0;
    p->bVideoRenderEosReceived = 0;
    p->bAudioRenderEosReceived = 0;
    p->bVideoCrash             = 0;
    p->bAudioCrash             = 0;
    p->bSubtitleCrash          = 0;
    p->sVideoCropWindow[0]     = 0;
    p->sVideoCropWindow[1]     = 0;
    p->sVideoCropWindow[2]     = 0;
    p->sVideoCropWindow[3]     = 0;

    if(p->pVideoRender != NULL)
    {
        VideoRenderCompDestroy(p->pVideoRender);
        p->pVideoRender = NULL;
    }

    if(p->pAudioRender != NULL)
    {
        AudioRenderCompDestroy(p->pAudioRender);
        p->pAudioRender = NULL;
    }

    if(p->pSubtitleRender != NULL)
    {
        SubtitleRenderCompDestroy(p->pSubtitleRender);
        p->pSubtitleRender = NULL;
    }

    if(p->pVideoDecComp != NULL)
    {
        VideoDecCompDestroy(p->pVideoDecComp);
        p->pVideoDecComp = NULL;
    }

    if(p->pAudioDecComp != NULL)
    {
        AudioDecCompDestroy(p->pAudioDecComp);
        p->pAudioDecComp = NULL;
    }

    if(p->pSubtitleDecComp != NULL)
    {
        SubtitleDecCompDestroy(p->pSubtitleDecComp);
        p->pSubtitleDecComp = NULL;
    }

    p->bProcessingCommand = 0;

    if(p->pVideoBitrateEstimater != NULL)
    {
        BitrateEstimaterDestroy(p->pVideoBitrateEstimater);
        p->pVideoBitrateEstimater = NULL;
    }

    if(p->pAudioBitrateEstimater != NULL)
    {
        BitrateEstimaterDestroy(p->pAudioBitrateEstimater);
        p->pAudioBitrateEstimater = NULL;
    }

    if(p->pVideoFramerateEstimater)
    {
        FramerateEstimaterDestroy(p->pVideoFramerateEstimater);
        p->pVideoFramerateEstimater = NULL;
    }

    if(p->pAvTimer != NULL)
    {
        p->pAvTimer->Stop(p->pAvTimer);
        p->pAvTimer->SetTime(p->pAvTimer, 0);
    }

    if(p->pVideoStreamInfo != NULL)
    {
        if(p->pVideoStreamInfo->pCodecSpecificData != NULL &&
           p->pVideoStreamInfo->nCodecSpecificDataLen > 0)
        {
            free(p->pVideoStreamInfo->pCodecSpecificData);
            p->pVideoStreamInfo->pCodecSpecificData = NULL;
            p->pVideoStreamInfo->nCodecSpecificDataLen = 0;
        }

        free(p->pVideoStreamInfo);
        p->pVideoStreamInfo = NULL;
    }

    if(p->pAudioStreamInfo != NULL && p->nAudioStreamNum > 0)
    {
        int i;
        for(i=0; i<p->nAudioStreamNum; i++)
        {
            if(p->pAudioStreamInfo[i].pCodecSpecificData != NULL &&
               p->pAudioStreamInfo[i].nCodecSpecificDataLen > 0)
            {
                free(p->pAudioStreamInfo[i].pCodecSpecificData);
                p->pAudioStreamInfo[i].pCodecSpecificData = NULL;
                p->pAudioStreamInfo[i].nCodecSpecificDataLen = 0;
            }
        }

        free(p->pAudioStreamInfo);
        p->pAudioStreamInfo = NULL;
    }

    if(p->pSubtitleStreamInfo != NULL && p->nSubtitleStreamNum > 0)
    {
        int i;
        for(i=0; i<p->nSubtitleStreamNum; i++)
        {
            if(p->pSubtitleStreamInfo[i].pUrl != NULL)
            {
                free(p->pSubtitleStreamInfo[i].pUrl);
                p->pSubtitleStreamInfo[i].pUrl = NULL;
            }
            if(p->pSubtitleStreamInfo[i].pCodecSpecificData != NULL &&
               p->pSubtitleStreamInfo[i].nCodecSpecificDataLen > 0)
            {
                free(p->pSubtitleStreamInfo[i].pCodecSpecificData);
                p->pSubtitleStreamInfo[i].pCodecSpecificData = NULL;
                p->pSubtitleStreamInfo[i].nCodecSpecificDataLen = 0;
            }
        }

        free(p->pSubtitleStreamInfo);
        p->pSubtitleStreamInfo = NULL;
    }

    return 0;
}


int PlayerSetEos(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pVideoDecComp != NULL)
        VideoDecCompSetEOS(p->pVideoDecComp);
    if(p->pAudioDecComp != NULL)
        AudioDecCompSetEOS(p->pAudioDecComp);
    if(p->pSubtitleDecComp != NULL)
        SubtitleDecCompSetEOS(p->pSubtitleDecComp);

    p->bStreamEosSet = 1;

    return 0;
}

int PlayerSetFirstPts(Player* pl, int64_t nFirstPts)
{
	PlayerContext* p;

    p = (PlayerContext*)pl;
	p->nFirstPts = nFirstPts;
	if(p->pSubtitleRender != NULL)
		SubtitleRenderCompSetVideoOrAudioFirstPts(p->pSubtitleRender,p->nFirstPts);

	return 0;
}
int PlayerRequestStreamBuffer(Player*         pl,
                              int             nRequireSize,
                              void**          ppBuf,
                              int*            pBufSize,
                              void**          ppRingBuf,
                              int*            pRingBufSize,
                              enum EMEDIATYPE eMediaType,
                              int             nStreamIndex)
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;

    logi("request stream buffer, eMediaType = %d, require size = %d", eMediaType, nRequireSize);

    *ppBuf          = NULL;
    *pBufSize       = 0;
    *ppRingBuf      = NULL;
    *pRingBufSize   = 0;

    if(p->eStatus == PLAYER_STATUS_STOPPED)
    {
        loge("can not request stream buffer at stopped status.");
        return -1;
    }

    if(eMediaType == MEDIA_TYPE_VIDEO)
    {
        if(p->bUnSurpportVideoFlag == 1)
        {
            if(nRequireSize > p->nUnSurpportVideoBufferSize)
                nRequireSize = p->nUnSurpportVideoBufferSize;
            *pBufSize = nRequireSize;
            *ppBuf = p->pUnSurpportVideoBuffer;
            return 0;
        }

#if CONFIG_DISABLE_VIDEO
        if(nRequireSize > p->nVideoStreamBufferSize)
            nRequireSize = p->nVideoStreamBufferSize;
        *pBufSize = nRequireSize;
        *ppBuf = p->pVideoStreamBuffer;
        return 0;
#else

        #if DROP_3D_SECOND_VIDEO_STREAM
        if(nStreamIndex == 1)
        {
            if(nRequireSize > p->nVideoSecondStreamBufferSize)
                nRequireSize = p->nVideoSecondStreamBufferSize;
            *pBufSize = nRequireSize;
            *ppBuf    = p->pVideoSecondStreamBuffer;
            return 0;
        }
        #endif

        if(p->pVideoDecComp == NULL)
        {
            loge("no video decode component, can not request video stream buffer.");
            return -1;
        }

        ret = VideoDecCompRequestStreamBuffer(p->pVideoDecComp,
                                              nRequireSize,
                                              (char**)ppBuf,
                                              pBufSize,
                                              (char**)ppRingBuf,
                                              pRingBufSize,
                                              nStreamIndex);

        return ret;
#endif
    }
    else if(eMediaType == MEDIA_TYPE_AUDIO)
    {
#if CONFIG_DISABLE_AUDIO
        if(nRequireSize > p->nAudioStreamBufferSize)
            nRequireSize = p->nAudioStreamBufferSize;
        *pBufSize = nRequireSize;
        *ppBuf = p->pAudioStreamBuffer;
        return 0;
#else
        pthread_mutex_lock(&p->audioDecoderMutex);

        if(p->pAudioDecComp == NULL)
        {
            loge("no audio decode component, can not request audio stream buffer.");
            pthread_mutex_unlock(&p->audioDecoderMutex);
            return -1;
        }

        ret = AudioDecCompRequestStreamBuffer(p->pAudioDecComp,
                                              nRequireSize,
                                              (char**)ppBuf,
                                              pBufSize,
                                              (char**)ppRingBuf,
                                              pRingBufSize,
                                              nStreamIndex);

        pthread_mutex_unlock(&p->audioDecoderMutex);
        return ret;
#endif
    }
    else
    {
#if CONFIG_DISABLE_SUBTITLE
        if(nRequireSize > p->nSubtitleStreamBufferSize)
            nRequireSize = p->nSubtitleStreamBufferSize;
        *pBufSize = nRequireSize;
        *ppBuf = p->pSubtitleStreamBuffer;
        return 0;
#else
        pthread_mutex_lock(&p->subtitleDecoderMutex);

        if(p->pSubtitleDecComp == NULL)
        {
            loge("no subtitle decode component, can not request subtitle stream buffer.");
            pthread_mutex_unlock(&p->subtitleDecoderMutex);
            return -1;
        }

        ret = SubtitleDecCompRequestStreamBuffer(p->pSubtitleDecComp,
                                                 nRequireSize,
                                                 (char**)ppBuf,
                                                 pBufSize,
                                                 nStreamIndex);

        pthread_mutex_unlock(&p->subtitleDecoderMutex);
        return ret;
#endif
    }
}


int PlayerSubmitStreamData(Player*              pl,
                           MediaStreamDataInfo* pDataInfo,
                           enum EMEDIATYPE      eMediaType,
                           int                  nStreamIndex)
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;

    logi("submit stream data, eMediaType = %d", eMediaType);

    if(p->eStatus == PLAYER_STATUS_STOPPED)
    {
        loge("can not submit stream data at stopped status.");
        return -1;
    }

    if(pDataInfo->nPts != -1)
        p->nLastInputPts = pDataInfo->nPts; //* save the last input pts.

    if(eMediaType == MEDIA_TYPE_VIDEO)
    {
        if(p->bUnSurpportVideoFlag == 1)
            return 0;

#if CONFIG_DISABLE_VIDEO
        return 0;
#else

        #if DROP_3D_SECOND_VIDEO_STREAM
            if(nStreamIndex == 1)
                return 0;
        #endif
        VideoStreamDataInfo dataInfo;

        if(p->pVideoDecComp == NULL)
        {
            loge("no video decode component, can not submit video stream data.");
            return -1;
        }

        logi("submit video data, pts = %lld ms, curTime = %lld ms", pDataInfo->nPts/1000,
            p->pAvTimer->GetTime(p->pAvTimer)/1000);

        if(nStreamIndex == 0 && p->pVideoBitrateEstimater)
            BitrateEstimaterUpdate(p->pVideoBitrateEstimater, pDataInfo->nPts, pDataInfo->nLength);

        //* process the video resolution change
        if(pDataInfo->nStreamChangeFlag == 1)
        {
            if(pDataInfo->nStreamChangeNum != 1)
            {
                loge("the videoNum is not 1,");
                abort();
            }

            VideoStreamInfo* pVideoChangeInfo = (VideoStreamInfo*)pDataInfo->pStreamInfo;
            VideoStreamInfo* pVideoStreamInfo   = (VideoStreamInfo*)malloc(sizeof(VideoStreamInfo));
            memset(pVideoStreamInfo,0,sizeof(VideoStreamInfo));

            pVideoStreamInfo->eCodecFormat    = pVideoChangeInfo->eCodecFormat;
            pVideoStreamInfo->nWidth          = pVideoChangeInfo->nWidth;
            pVideoStreamInfo->nHeight         = pVideoChangeInfo->nHeight;
            pVideoStreamInfo->nFrameRate      = pVideoChangeInfo->nFrameRate;
            pVideoStreamInfo->nFrameDuration  = pVideoChangeInfo->nFrameDuration;
            pVideoStreamInfo->nAspectRatio    = pVideoChangeInfo->nAspectRatio;
            pVideoStreamInfo->bIs3DStream     = pVideoChangeInfo->bIs3DStream;
            pVideoStreamInfo->bIsFramePackage = pVideoChangeInfo->bIsFramePackage;

            if(pVideoChangeInfo->nCodecSpecificDataLen > 0)
            {
                pVideoStreamInfo->pCodecSpecificData = (char*)malloc(pVideoChangeInfo->nCodecSpecificDataLen);
                if(pVideoStreamInfo->pCodecSpecificData == NULL)
                {
                    loge("malloc pCodecSpecificData failed");
                    free(pVideoChangeInfo->pCodecSpecificData);
                    free(pVideoChangeInfo);
                    return -1;
                }

                memcpy(pVideoStreamInfo->pCodecSpecificData,pVideoChangeInfo->pCodecSpecificData,
                       pVideoChangeInfo->nCodecSpecificDataLen);
                pVideoStreamInfo->nCodecSpecificDataLen = pVideoChangeInfo->nCodecSpecificDataLen;

                free(pVideoChangeInfo->pCodecSpecificData);
                pVideoChangeInfo->pCodecSpecificData = NULL;
            }
            else
            {
                pVideoStreamInfo->pCodecSpecificData    = NULL;
                pVideoStreamInfo->nCodecSpecificDataLen = 0;
            }

            free(pVideoChangeInfo);
            pDataInfo->pStreamInfo = NULL;

            dataInfo.bVideoInfoFlag = 1;
            dataInfo.pVideoInfo     = pVideoStreamInfo;
        }
        else
        {
            dataInfo.pVideoInfo     = NULL;
            dataInfo.bVideoInfoFlag = 0;
        }

        dataInfo.pData        = pDataInfo->pData;
        dataInfo.nLength      = pDataInfo->nLength;
        dataInfo.nPts         = pDataInfo->nPts;
        dataInfo.nPcr         = pDataInfo->nPcr;
        dataInfo.bIsFirstPart = pDataInfo->bIsFirstPart;
        dataInfo.bIsLastPart  = pDataInfo->bIsLastPart;
        dataInfo.nStreamIndex = nStreamIndex;

        ret = VideoDecCompSubmitStreamData(p->pVideoDecComp, &dataInfo, nStreamIndex);
        return ret;
#endif
    }
    else if(eMediaType == MEDIA_TYPE_AUDIO)
    {
#if CONFIG_DISABLE_AUDIO
        return 0;
#else
        AudioStreamDataInfo dataInfo;

        pthread_mutex_lock(&p->audioDecoderMutex);

        if(p->pAudioDecComp == NULL)
        {
            loge("no audio decode component, can not submit audio stream data.");
            pthread_mutex_unlock(&p->audioDecoderMutex);
            return -1;
        }

        logi("submit audio data, pts = %lld ms, curTime = %lld ms", pDataInfo->nPts/1000,
            p->pAvTimer->GetTime(p->pAvTimer)/1000);

        if(nStreamIndex == p->nAudioStreamSelected)
            BitrateEstimaterUpdate(p->pAudioBitrateEstimater, pDataInfo->nPts, pDataInfo->nLength);

        //* process the video resolution change.
        //* now, we just free the buffer , not to process it really
        if(pDataInfo->nStreamChangeFlag == 1)
        {
            AudioStreamInfo* pAudioChangeInfo = (AudioStreamInfo*)pDataInfo->pStreamInfo;
            for(int i = 0; i < pDataInfo->nStreamChangeNum; i++)
            {
                if(pAudioChangeInfo[i].pCodecSpecificData != NULL)
                    free(pAudioChangeInfo[i].pCodecSpecificData);
            }
            free(pAudioChangeInfo);
        }


        dataInfo.pData        = pDataInfo->pData;
        dataInfo.nLength      = pDataInfo->nLength;
        dataInfo.nPts         = pDataInfo->nPts;
        dataInfo.nPcr         = pDataInfo->nPcr;
        dataInfo.nPcr         = pDataInfo->nPcr;
        dataInfo.bIsFirstPart = pDataInfo->bIsFirstPart;
        dataInfo.bIsLastPart  = pDataInfo->bIsLastPart;

        ret = AudioDecCompSubmitStreamData(p->pAudioDecComp, &dataInfo, nStreamIndex);
        pthread_mutex_unlock(&p->audioDecoderMutex);

        return ret;
#endif
    }
    else
    {
#if CONFIG_DISABLE_SUBTITLE
        return 0;
#else
        SubtitleStreamDataInfo dataInfo;

        pthread_mutex_lock(&p->subtitleDecoderMutex);

        if(p->pSubtitleDecComp == NULL)
        {
            loge("no subtitle decode component, can not submit subtitle stream data.");
            pthread_mutex_unlock(&p->subtitleDecoderMutex);
            return -1;
        }

        logi("submit subtitle data, pts = %lld ms, curTime = %lld ms", pDataInfo->nPts/1000,
            p->pAvTimer->GetTime(p->pAvTimer)/1000);

        dataInfo.pData        = pDataInfo->pData;
        dataInfo.nLength      = pDataInfo->nLength;
        dataInfo.nPts         = pDataInfo->nPts;
        dataInfo.nPcr         = pDataInfo->nPcr;
        dataInfo.nPcr         = pDataInfo->nPcr;
        dataInfo.nDuration    = pDataInfo->nDuration;

        ret = SubtitleDecCompSubmitStreamData(p->pSubtitleDecComp, &dataInfo, nStreamIndex);
        pthread_mutex_unlock(&p->subtitleDecoderMutex);

        return ret;
#endif
    }
}

int64_t PlayerGetPositionCMCC(Player* pl)    //* current time position in us.
{
    PlayerContext* p;
    int64_t        nCurTime;
    int64_t        nCurPosition;

    p = (PlayerContext*)pl;

    if(p->bInFastMode)
        return p->nPreVideoPts;

    if(p->nTimeBase != -1)
    {
        nCurTime = p->pAvTimer->GetTime(p->pAvTimer);
        nCurPosition = p->nTimeOffset - nCurTime + p->nTimeBase ;
    }
    else
    {
        //* waiting timer to be start at the first pts callback.
        //* current timer value is not valid.
        nCurPosition = p->nTimeOffset;
    }
    return nCurPosition;
}

int64_t PlayerGetPosition(Player* pl)    //* current time position in us.
{
    PlayerContext* p;
    int64_t        nCurTime;
    int64_t        nCurPosition;

    p = (PlayerContext*)pl;

    if(p->bInFastMode)
        return p->nPreVideoPts;

    if(p->nTimeBase != -1)
    {
        nCurTime = p->pAvTimer->GetTime(p->pAvTimer);
        nCurPosition = nCurTime - p->nTimeBase + p->nTimeOffset;
    }
    else
    {
        //* waiting timer to be start at the first pts callback.
        //* current timer value is not valid.
        nCurPosition = p->nTimeOffset;
    }
	logd("in PlayerGetPosition,nCurPosition = %lld us , p->nTimeBase = %d",nCurPosition,p->nTimeBase);
    return nCurPosition;
}


int64_t PlayerGetPts(Player* pl)    //* current pts in us.
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    if(p->bInFastMode)
        return p->nPreVideoPts;

    if(p->nTimeBase != -1)
    {
        return p->pAvTimer->GetTime(p->pAvTimer);
    }
    else
    {
        //* waiting timer to be start at the first pts callback.
        //* current timer value is not valid.
        return p->nTimeOffset;
    }
}


int PlayerSet3DMode(Player* pl, enum EPICTURE3DMODE ePicture3DMode, enum EDISPLAY3DMODE eDisplay3DMode)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    if(p->pVideoRender != NULL)
    {
        return VideoRenderSet3DMode(p->pVideoRender, ePicture3DMode, eDisplay3DMode);
    }
    else
    {
        loge("no video render, can not set 3D mode.");
        return -1;
    }
}


int PlayerGet3DMode(Player* pl, enum EPICTURE3DMODE* ePicture3DMode, enum EDISPLAY3DMODE* eDisplay3DMode)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    if(p->pVideoRender != NULL)
    {
        return VideoRenderGet3DMode(p->pVideoRender, ePicture3DMode, eDisplay3DMode);
    }
    else
    {
        loge("no video render, can not get 3D mode.");
        return -1;
    }
}


int PlayerConfigVideoScaleDownRatio(Player* pl, int nHorizonRatio, int nVerticalRatio)
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;

    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        loge("player not in stopped status, can not config video scale down ratio.");
        return -1;
    }

    if(nHorizonRatio != 0 || nVerticalRatio != 0)
        p->sVideoConfig.bScaleDownEn  = 1;
    else
        p->sVideoConfig.bScaleDownEn  = 0;
    p->sVideoConfig.nHorizonScaleDownRatio  = nHorizonRatio;
    p->sVideoConfig.nVerticalScaleDownRatio = nVerticalRatio;
    return 0;
}


int PlayerConfigVideoRotateDegree(Player* pl, int nDegree)
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;

    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        loge("player not in stopped status, can not config video scale down ratio.");
        return -1;
    }
    if(nDegree != 0)
        p->sVideoConfig.bRotationEn  = 1;
    else
        p->sVideoConfig.bRotationEn  = 0;
    p->sVideoConfig.nRotateDegree = nDegree;
    return 0;
}


int PlayerConfigVideoDeinterlace(Player* pl, int bDeinterlace)
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;

    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        loge("player not in stopped status, can not config video deinterlace flag.");
        return -1;
    }

    p->sVideoConfig.bSupportMaf = bDeinterlace;
    return 0;
}

int PlayerConfigTvStreamFlag(Player* pl, int bFlag)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        loge("player not in stopped status, can not config video deinterlace flag.");
        return -1;
    }

    p->sVideoConfig.bIsTvStream = bFlag;
    return 0;
}

int PlayerConfigDispErrorFrame(Player* pl, int bDispErrorFrame)
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;

    p->sVideoConfig.bDispErrorFrame = bDispErrorFrame;
    return 0;
}

int PlayerConfigDropLaytedFrame(Player* pl, int bDropLaytedFrame)
{
	PlayerContext* p;
	p = (PlayerContext*)pl;

    logd("PlayerConfigDropLaytedFrame\n");
	if(p->pVideoDecComp != NULL)
	{
		logd("VideoDecCompSetDropDelayFrames\n");
		return VideoDecCompSetDropDelayFrames(p->pVideoDecComp, bDropLaytedFrame); //* only treat the major video stream.
	}
	return 0;
}

int PlayerConfigSetMemoryThresh(Player* pl, int nMemoryThresh)
{
	PlayerContext* p;
	p = (PlayerContext*)pl;


	if(p->pVideoDecComp != NULL)
	{

		return VideoDecCompSetMemortThresh(p->pVideoDecComp, nMemoryThresh); //* only treat the major video stream.
	}
	return 0;
}
int PlayerGetVideoBitrate(Player* pl)   //* estimated by pts and stream data size.
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pVideoBitrateEstimater != NULL)
    {
        return BitrateEstimaterGetBitrate(p->pVideoBitrateEstimater);
    }
    else
        return 0;
}


//* estimated by pts.
int PlayerGetVideoFrameRate(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pVideoFramerateEstimater != NULL)
    {
        return FramerateEstimaterGetFramerate(p->pVideoFramerateEstimater);
    }
    else
        return 0;
}


//* estimated by pts.
int PlayerGetVideoFrameDuration(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pVideoFramerateEstimater != NULL)
    {
        return FramerateEstimaterGetFrameDuration(p->pVideoFramerateEstimater);
    }
    else
        return 0;
}


//* how much video stream data has been buffered.
int PlayerGetVideoStreamDataSize(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pVideoDecComp != NULL)
        return VideoDecCompStreamDataSize(p->pVideoDecComp, 0); //* only treat the major video stream.
    else
        return 0;
}

int PlayerGetVideoStreamBufferSize(Player* pl)
{
	PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pVideoDecComp != NULL)
        return VideoDecCompStreamBufferSize(p->pVideoDecComp, 0); //* only treat the major video stream.
    else
        return 0;
}


//* how many stream frame in buffer.
int PlayerGetVideoStreamFrameNum(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pVideoDecComp != NULL)
        return VideoDecCompStreamFrameNum(p->pVideoDecComp, 0); //* only treat the major video stream.
    else
        return 0;
}


//* how many picture has been decoded and waiting to show.
int PlayerGetValidPictureNum(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pVideoDecComp != NULL)
        return VideoDecCompValidPictureNum(p->pVideoDecComp, 0); //* only treat the major video stream.
    else
        return 0;
}


int PlayerGetAudioBitrate(Player* pl)  //* estimated by pts and stream data size.
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pAudioBitrateEstimater != NULL)
    {
        return BitrateEstimaterGetBitrate(p->pAudioBitrateEstimater);
    }
    else
        return 0;
}


//* get audio sample rate, channel count and how many bits per sample of pcm data.
int PlayerGetAudioParam(Player* pl, int* pSampleRate, int* pChannelCount, int* pBitsPerSample)
{
    PlayerContext* p;
    unsigned int   nSampleRate;
    unsigned int   nChannelCount;
    unsigned int   nBitRate;

    p = (PlayerContext*)pl;
    if(p->pAudioDecComp != NULL &&
        AudioDecCompGetAudioSampleRate(p->pAudioDecComp, &nSampleRate, &nChannelCount, &nBitRate) == 0)
    {
        *pSampleRate    = nSampleRate;
        *pChannelCount  = nChannelCount;
        *pBitsPerSample = 16;
        return 0;
    }
    else
    {
        *pSampleRate    = 0;
        *pChannelCount  = 0;
        *pBitsPerSample = 0;
        return -1;
    }
}


//* how much audio stream data has been buffered.
int PlayerGetAudioStreamDataSize(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pAudioDecComp != NULL)
        return AudioDecCompStreamDataSize(p->pAudioDecComp, p->nAudioStreamSelected);
    else
        return 0;
}


//* how many stream frame in buffer.
int PlayerGetAudioStreamFrameNum(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pAudioDecComp != NULL)
        return AudioDecCompStreamFrameNum(p->pAudioDecComp, p->nAudioStreamSelected);
    else
        return 0;
}


//* how much audio pcm data has been decoded to the pcm buffer.
int PlayerGetAudioPcmDataSize(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pAudioDecComp != NULL)
        return AudioDecCompPcmDataSize(p->pAudioDecComp, p->nAudioStreamSelected);
    else
        return 0;
}


//* how much audio pcm data cached in audio device.
int PlayerGetAudioCacheTimeInSoundDevice(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pAudioRender != NULL)
        return AudioRenderCompCacheTimeUs(p->pAudioRender);
    else
        return 0;
}
//hkw for IPTV
int PlayerStopAudio(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pAudioRender == NULL || p->pAudioDecComp == NULL)
    {
        logw("no audio decoder or render, switch audio fail.");
        return -1;
    }

    pthread_mutex_lock(&p->audioDecoderMutex);

    p->bAudioCrash = 0;

    //* 1. stop the audio render.
    AudioRenderCompStop(p->pAudioRender);

    //* 2. stop the audio decoder.
    AudioDecCompStop(p->pAudioDecComp);
    pthread_mutex_unlock(&p->audioDecoderMutex);

    return 0;
}
int PlayerStartAudio(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pAudioRender == NULL || p->pAudioDecComp == NULL)
    {
        logw("no audio decoder or render, switch audio fail.");
        return -1;
    }

    pthread_mutex_lock(&p->audioDecoderMutex);

    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        //* 4. start the audio decoder and render.
        AudioDecCompStart(p->pAudioDecComp);
        AudioRenderCompStart(p->pAudioRender);

        if(p->eStatus == PLAYER_STATUS_PAUSED)
        {
            //* 5. pause the audio decoder and render.
            AudioRenderCompPause(p->pAudioRender);
            AudioDecCompPause(p->pAudioDecComp);
        }
    }

    if(p->bStreamEosSet)
    {
        p->bAudioRenderEosReceived = 0;
        AudioDecCompSetEOS(p->pAudioDecComp);
    }

    pthread_mutex_unlock(&p->audioDecoderMutex);

    return 0;
}


int PlayerSwitchAudio(Player* pl, int nStreamIndex)
{
    PlayerContext* p;
    int            nAudioStreamCount;

    p = (PlayerContext*)pl;
    if(p->pAudioRender == NULL || p->pAudioDecComp == NULL)
    {
        logw("no audio decoder or render, switch audio fail.");
        return -1;
    }

    if(nStreamIndex < 0)
    {
        logw("invalid audio stream index %d, switch audio stream fail.", nStreamIndex);
        return -1;
    }

    nAudioStreamCount = AudioDecCompGetAudioStreamCnt(p->pAudioDecComp);
    if(nAudioStreamCount <= nStreamIndex)
    {
        logw("only %d audio streams, can not switch to audio stream %d.", nAudioStreamCount, nStreamIndex);
        return -1;
    }

    if(AudioDecCompCurrentStreamIndex(p->pAudioDecComp) == nStreamIndex)
    {
        logw("selected audio stream is the one being played, do nothing for switch audio.");
        return 0;
    }

    pthread_mutex_lock(&p->audioDecoderMutex);

    p->bAudioCrash = 0;

    //* 1. stop the audio render.
    AudioRenderCompStop(p->pAudioRender);

    //* 2. stop the audio decoder.
    AudioDecCompStop(p->pAudioDecComp);

    //* 3. audio decoder switch audio stream.
    AudioDecCompSwitchStream(p->pAudioDecComp, nStreamIndex);
    p->nAudioStreamSelected = nStreamIndex;

    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        //* 4. start the audio decoder and render.
        AudioDecCompStart(p->pAudioDecComp);
        AudioRenderCompStart(p->pAudioRender);

        if(p->eStatus == PLAYER_STATUS_PAUSED)
        {
            //* 5. pause the audio decoder and render.
            AudioRenderCompPause(p->pAudioRender);
            AudioDecCompPause(p->pAudioDecComp);
        }
    }

    if(p->bStreamEosSet)
    {
        p->bAudioRenderEosReceived = 0;
        AudioDecCompSetEOS(p->pAudioDecComp);
    }

    pthread_mutex_unlock(&p->audioDecoderMutex);

    return 0;
}


//* get audio balance, return 1 means left channel, 2 means right channel, 3 means stereo.
int PlayerGetAudioBalance(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    if(p->pAudioRender != NULL)
        return AudioRenderCompGetAudioOutBalance(p->pAudioRender);
    else
        return 3;
}


//* set audio balance, 1 means left channel, 2 means right channel, 3 means stereo.
int PlayerSetAudioBalance(Player* pl, int nAudioBalance)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    if(p->pAudioRender != NULL)
        return AudioRenderCompSetAudioOutBalance(p->pAudioRender, nAudioBalance);
    else
        return -1;
}


int PlayerGetAudioStreamInfo(Player* pl, int* pStreamNum, AudioStreamInfo** ppStreamInfo)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    *pStreamNum = p->nAudioStreamNum;
    *ppStreamInfo = p->pAudioStreamInfo;
    return 0;
}


int PlayerSetAudioMute(Player* pl, int bMute)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    if(p->pAudioRender != NULL)
        return AudioRenderCompSetAudioMute(p->pAudioRender, bMute);
    else
        return -1;
}


int PlayerGetAudioMuteFlag(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    if(p->pAudioRender != NULL)
        return AudioRenderCompGetAudioMuteFlag(p->pAudioRender);
    else
        return -1;
}

int PlayerSetAudioForceWriteToDeviceFlag(Player* pl, int bForceFlag)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    logd("*** PlayerSetAudioForceWriteToDeviceFlag, pAudioRender = %p, flag = %d",
         p->pAudioRender,bForceFlag);
    if(p->pAudioRender != NULL)
        return AudioRenderCompSetForceWriteToDeviceFlag(p->pAudioRender,bForceFlag);
    else
        return -1;
}

//*****************************************************************************//
//* TODO.
//*****************************************************************************//

//* set video window.
int PlayerSetVideoRegion(Player* pl, int x, int y, int nWidth, int nHeight)
{
    CEDARX_UNUSE(pl);
	CEDARX_UNUSE(x);
    CEDARX_UNUSE(y);
    CEDARX_UNUSE(nWidth);
	CEDARX_UNUSE(nHeight);

    return 0;
}


//* set picture display ratio, full stream, letter box or other modes.
int PlayerSetDisplayRatio(Player* pl, enum EDISPLAYRATIO eDisplayRatio)
{
	CEDARX_UNUSE(pl);
	CEDARX_UNUSE(eDisplayRatio);
    return 0;
}

//*****************************************************************************//
//* TODO end.
//*****************************************************************************//

int PlayerSetSubtitleStreamInfo(Player* pl, SubtitleStreamInfo* pStreamInfo, int nStreamNum, int nDefaultStream)
{
    PlayerContext* p;
    int            i;

    p = (PlayerContext*)pl;

    if(nStreamNum == 0)
    {
        loge("have no subtitle stream, wouldn't init audio player");
        return 0;
    }

#if CONFIG_DISABLE_SUBTITLE
    return 0;
#else
    if(p->pSubtitleStreamInfo != NULL && p->nSubtitleStreamNum > 0)
    {
        for(i=0; i<p->nSubtitleStreamNum; i++)
        {
            if(p->pSubtitleStreamInfo[i].pUrl != NULL)
            {
                free(p->pSubtitleStreamInfo[i].pUrl);
                p->pSubtitleStreamInfo[i].pUrl = NULL;
            }
            if(p->pSubtitleStreamInfo[i].pCodecSpecificData)
            {
		free(p->pSubtitleStreamInfo[i].pCodecSpecificData);
		p->pSubtitleStreamInfo[i].pCodecSpecificData = NULL;
		p->pSubtitleStreamInfo[i].nCodecSpecificDataLen = 0;
            }
        }

        free(p->pSubtitleStreamInfo);
        p->pSubtitleStreamInfo = NULL;
    }

    p->nSubtitleStreamNum = 0;
    p->nSubtitleStreamSelected = 0;

    if(pStreamInfo != NULL && nStreamNum > 0)
    {
        p->pSubtitleStreamInfo = (SubtitleStreamInfo*)malloc(sizeof(SubtitleStreamInfo) * nStreamNum);
        if(p->pSubtitleStreamInfo == NULL)
        {
            loge("malloc memory fail.");
            return -1;
        }

        memcpy(p->pSubtitleStreamInfo, pStreamInfo, sizeof(SubtitleStreamInfo)*nStreamNum);

        for(i=0; i<nStreamNum; i++)
        {
            if(pStreamInfo[i].pUrl != NULL)
            {
                p->pSubtitleStreamInfo[i].pUrl = strdup(pStreamInfo[i].pUrl);
                if(p->pSubtitleStreamInfo[i].pUrl == NULL)
                {
                    loge("malloc memory fail.");
                    break;
                }
            }

            if(pStreamInfo[i].pCodecSpecificData != NULL &&
               pStreamInfo[i].nCodecSpecificDataLen > 0)
            {
		p->pSubtitleStreamInfo[i].nCodecSpecificDataLen = pStreamInfo[i].nCodecSpecificDataLen;
                p->pSubtitleStreamInfo[i].pCodecSpecificData = (char*)malloc(pStreamInfo[i].nCodecSpecificDataLen);
                if(p->pSubtitleStreamInfo[i].pCodecSpecificData == NULL)
                {
                    loge("malloc memory fail.");
                    break;
                }
                memcpy(p->pSubtitleStreamInfo[i].pCodecSpecificData,
                       pStreamInfo[i].pCodecSpecificData,
                       pStreamInfo[i].nCodecSpecificDataLen);
            }
        }

        if(i != nStreamNum)
        {
            i--;
            for(; i>=0; i--)
            {
                if(p->pSubtitleStreamInfo[i].pUrl != NULL)
                {
                    free(p->pSubtitleStreamInfo[i].pUrl);
                    p->pSubtitleStreamInfo[i].pUrl = NULL;
                }
                if(p->pSubtitleStreamInfo[i].pCodecSpecificData)
	            {
			free(p->pSubtitleStreamInfo[i].pCodecSpecificData);
			p->pSubtitleStreamInfo[i].pCodecSpecificData = NULL;
			p->pSubtitleStreamInfo[i].nCodecSpecificDataLen = 0;
	            }
            }

            free(p->pSubtitleStreamInfo);
            p->pSubtitleStreamInfo = NULL;
            return -1;
        }

        p->nSubtitleStreamSelected = nDefaultStream;
        p->nSubtitleStreamNum = nStreamNum;
    }

    if(PlayerInitialSubtitle(p) == 0)
    {
        if(p->eStatus != PLAYER_STATUS_STOPPED)
        {
            SubtitleRenderCompStart(p->pSubtitleRender);
            if(p->eStatus == PLAYER_STATUS_PAUSED)
                SubtitleRenderCompPause(p->pSubtitleRender);
            SubtitleDecCompStart(p->pSubtitleDecComp);
            if(p->eStatus == PLAYER_STATUS_PAUSED)
                SubtitleDecCompPause(p->pSubtitleDecComp);
        }
        return 0;
    }
    else
    {
        return -1;
    }
#endif
}


int PlayerAddSubtitleStream(Player* pl, SubtitleStreamInfo* pStreamInfo)
{
    PlayerContext*      p;
    SubtitleStreamInfo* newArr;
    int                 nStreamCount;
    int                 ret;

    p = (PlayerContext*)pl;

    if(p->pSubtitleStreamInfo != NULL && p->nSubtitleStreamNum > 0)
    {
        nStreamCount = p->nSubtitleStreamNum + 1;
        newArr = (SubtitleStreamInfo*)malloc(sizeof(SubtitleStreamInfo)*nStreamCount);
        if(newArr == NULL)
        {
            loge("malloc memory fail.");
            return -1;
        }

        memcpy(newArr, p->pSubtitleStreamInfo, p->nSubtitleStreamNum*sizeof(SubtitleStreamInfo));
        memcpy(&newArr[nStreamCount-1], pStreamInfo, sizeof(SubtitleStreamInfo));
        if(pStreamInfo->pUrl != NULL)
        {
            newArr[nStreamCount-1].pUrl = strdup(pStreamInfo->pUrl);
            if(newArr[nStreamCount-1].pUrl == NULL)
            {
                loge("malloc memory fail.");
                free(newArr);
                return -1;
            }
        }
        if(pStreamInfo->pCodecSpecificData != NULL && pStreamInfo->nCodecSpecificDataLen > 0)
        {
		newArr[nStreamCount-1].nCodecSpecificDataLen = pStreamInfo->nCodecSpecificDataLen;
		newArr[nStreamCount-1].pCodecSpecificData = (char*)malloc(pStreamInfo->nCodecSpecificDataLen);
            if(newArr[nStreamCount-1].pCodecSpecificData == NULL)
            {
                loge("malloc memory fail.");
                free(newArr);
                return -1;
            }
            memcpy(newArr[nStreamCount-1].pCodecSpecificData,
                   pStreamInfo->pCodecSpecificData,
                   pStreamInfo->nCodecSpecificDataLen);
        }

        if(p->pSubtitleDecComp != NULL)
        {
            if(SubtitleDecCompGetStatus(p->pSubtitleDecComp) == PLAYER_STATUS_STARTED)
            {
                SubtitleDecCompPause(p->pSubtitleDecComp);
                ret = SubtitleDecCompAddSubtitleStream(p->pSubtitleDecComp, &newArr[nStreamCount-1]);
                SubtitleDecCompStart(p->pSubtitleDecComp);
            }
            else
                ret = SubtitleDecCompAddSubtitleStream(p->pSubtitleDecComp, &newArr[nStreamCount-1]);

            if(ret != 0)
            {
                if(newArr[nStreamCount-1].pUrl != NULL)
                {
                    free(newArr[nStreamCount-1].pUrl);
                    newArr[nStreamCount-1].pUrl = NULL;
                }
                if(pStreamInfo->pCodecSpecificData != NULL && pStreamInfo->nCodecSpecificDataLen > 0)
		        {
				free(newArr[nStreamCount-1].pCodecSpecificData);
		            newArr[nStreamCount-1].pCodecSpecificData = NULL;
		            newArr[nStreamCount-1].nCodecSpecificDataLen = 0;
		        }
                free(newArr);
                return -1;
            }
        }

        free(p->pSubtitleStreamInfo);
        p->pSubtitleStreamInfo = newArr;
        p->nSubtitleStreamNum  = nStreamCount;

        return 0;
    }
    else
    {
        return PlayerSetSubtitleStreamInfo(pl, pStreamInfo, 1, 0);
    }
}


int PlayerCanSupportSubtitleStream(Player* pl, SubtitleStreamInfo* pStreamInfo)
{
    PlayerContext* p;

	CEDARX_UNUSE(pStreamInfo);
    p = (PlayerContext*)pl;
    return 1;
}


int PlayerGetSubtitleStreamCnt(Player* pl)
{
    PlayerContext* p;
    p = (PlayerContext*)pl;
    return p->nSubtitleStreamNum;
}


int PlayerGetSubtitleStreamInfo(Player* pl, int* pStreamNum, SubtitleStreamInfo** ppStreamInfo)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    *pStreamNum = p->nSubtitleStreamNum;
    *ppStreamInfo = p->pSubtitleStreamInfo;
    return 0;
}

//* stream info array is allocated inside, user need to free the memory by calling free(*ppStreamInfo);
int PlayerProbeSubtitleStreamInfo(const char*          strFileName,
                                  int*                 pStreamNum,
                                  SubtitleStreamInfo** ppStreamInfo)
{
    return ProbeSubtitleStream(strFileName, ppStreamInfo, pStreamNum);
}

//* stream info array is allocated inside, user need to free the memory by calling free(*ppStreamInfo);
int PlayerProbeSubtitleStreamInfoFd(int fd, int offset, int len, int* pStreamNum, SubtitleStreamInfo** ppStreamInfo)
{
    return ProbeSubtitleStreamFd(fd, offset, len, ppStreamInfo, pStreamNum);
}

int PlayerSwitchSubtitle(Player* pl, int nStreamIndex)
{
    PlayerContext* p;
    int            nSubtitleStreamCount;

    p = (PlayerContext*)pl;
    if(p->pSubtitleRender == NULL || p->pSubtitleDecComp == NULL)
    {
        logw("no subtitle decoder or render, switch subtitle fail.");
        return -1;
    }

    if(nStreamIndex < 0)
    {
        logw("invalid subtitle stream index %d, switch subtitle stream fail.", nStreamIndex);
        return -1;
    }

    nSubtitleStreamCount = SubtitleDecCompGetSubtitleStreamCnt(p->pSubtitleDecComp);
    if(nSubtitleStreamCount <= nStreamIndex)
    {
        logw("only %d subtitle streams, can not switch to subtitle stream %d.", nSubtitleStreamCount, nStreamIndex);
        return -1;
    }

    if(SubtitleDecCompCurrentStreamIndex(p->pSubtitleDecComp) == nStreamIndex)
    {
        logw("selected subtitle stream is the one being played, do nothing for switch subtitle.");
        return 0;
    }

    pthread_mutex_lock(&p->subtitleDecoderMutex);

    p->bSubtitleCrash = 0;

    //* 1. stop the subtitle render.
    SubtitleRenderCompStop(p->pSubtitleRender);

    //* 2. stop the subtitle decoder.
    SubtitleDecCompStop(p->pSubtitleDecComp);

    //* 3. subtitle decoder switch subtitle stream.
    SubtitleDecCompSwitchStream(p->pSubtitleDecComp, nStreamIndex);
    p->nSubtitleStreamSelected = nStreamIndex;

    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        //* 4. start the subtitle decoder and render.
        SubtitleDecCompStart(p->pSubtitleDecComp);
        SubtitleRenderCompStart(p->pSubtitleRender);

        if(p->eStatus == PLAYER_STATUS_PAUSED)
        {
            //* 5. pause the subtitle decoder and render.
            SubtitleRenderCompPause(p->pSubtitleRender);
            SubtitleDecCompPause(p->pSubtitleDecComp);
        }
    }

#if( CONFIG_ALI_YUNOS == OPTION_ALI_YUNOS_YES)
	SubtitleRenderCompSwitchStream(p->pSubtitleRender,nStreamIndex);
#endif

    if(p->bStreamEosSet)
    {
        SubtitleDecCompSetEOS(p->pAudioDecComp);
    }

    pthread_mutex_unlock(&p->subtitleDecoderMutex);

    return 0;
}


//* adjust subtitle show time.
int PlayerSetSubtitleShowTimeAdjustment(Player* pl, int nTimeMs)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pSubtitleRender != NULL)
        return SubtitleRenderSetShowTimeAdjustment(p->pSubtitleRender, nTimeMs);
    else
        return -1;
}

//* get the adjustment of subtitle show time, in unit of ms.
int PlayerGetSubtitleShowTimeAdjustment(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;

    if(p->pSubtitleRender != NULL)
        return SubtitleRenderGetShowTimeAdjustment(p->pSubtitleRender);
    else
        return -1;
}


//* show video layer.
int PlayerVideoShow(Player* pl)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pVideoRender != NULL)
        return VideoRenderVideoHide(p->pVideoRender, 0);
    else
        return -1;
}


//* Hide video layer.
int PlayerVideoHide(Player* pl)
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;
    if(p->pVideoRender != NULL)
        return VideoRenderVideoHide(p->pVideoRender, 1);
    else
        return -1;
}

//* set whether keep last picture when player stopped.
//* the last picture is hold in default.
int PlayerSetHoldLastPicture(Player* pl, int bHold)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    if(p->pVideoRender != NULL)
        return VideoRenderSetHoldLastPicture(p->pVideoRender, bHold);
    else
        return -1;
}

int PlayerGetPictureSize(Player* pl, int* pWidth, int* pHeight)
{
    PlayerContext*   p;
    VideoStreamInfo  vsi;
    int              ret;

    p = (PlayerContext*)pl;

    if(p->pVideoDecComp != NULL)
    {
        ret = VideoDecCompGetVideoStreamInfo(p->pVideoDecComp, &vsi);
        if(ret < 0)
            return -1;

        *pWidth  = vsi.nWidth;
        *pHeight = vsi.nHeight;
        return 0;
    }
    else
        return -1;
}

int PlayerGetPictureCrop(Player* pl, int* pLeftOff, int* pTopOff, int* pCropWidth, int* pCropHeight)
{
    PlayerContext*   p;
    p = (PlayerContext*)pl;
    *pLeftOff    = p->sVideoCropWindow[0];
    *pTopOff     = p->sVideoCropWindow[1];
    *pCropWidth  = p->sVideoCropWindow[2];
    *pCropHeight = p->sVideoCropWindow[3];
    return 0;
}


//* set player playback in fast mode.
//* in fast mode, video is showed directly without any synchronization, and
//* audio is discard.
int PlayerFast(Player* pl, int bDecodeKeyframeOnly)
{
    PlayerContext* p;
    int            ret;

    p = (PlayerContext*)pl;

	PlayerPause(pl);
    PlayerReset(pl);
    if(p->pVideoDecComp && bDecodeKeyframeOnly)
        VideoDecCompSetDecodeKeyFrameOnly(p->pVideoDecComp, 1);
    p->bInFastMode = 1;
    PlayerStart(pl);
    return 0;
}

//* return from fast mode, the player is set to started status when return.
int PlayerStopFast(Player* pl)
{
    PlayerContext*     p;
    enum EPLAYERSTATUS eStatus;

    p = (PlayerContext*)pl;
	eStatus = p->eStatus;
    PlayerPause(pl);
    if(p->pVideoDecComp)
        VideoDecCompSetDecodeKeyFrameOnly(p->pVideoDecComp, 0);
    PlayerReset(pl);
    p->bInFastMode = 0;
    if(eStatus == PLAYER_STATUS_STARTED)
	    PlayerStart(pl);

    return 0;
}


static int CallbackProcess(void* pSelf, int eMessageId, void* param)
{
    PlayerContext* p;

    p = (PlayerContext*)pSelf;

    switch(eMessageId)
    {
        case PLAYER_VIDEO_DECODER_NOTIFY_EOS:
        {
            if(p->pVideoRender != NULL)
                VideoRenderCompSetEOS(p->pVideoRender);
            return 0;
        }

        case PLAYER_AUDIO_DECODER_NOTIFY_EOS:
        {
            if(p->pAudioRender != NULL)
                AudioRenderCompSetEOS(p->pAudioRender);
            return 0;
        }

        case PLAYER_SUBTITLE_DECODER_NOTIFY_EOS:
        {
            if(p->pSubtitleRender != NULL)
                SubtitleRenderCompSetEOS(p->pSubtitleRender);
            return 0;
        }

        case PLAYER_VIDEO_RENDER_NOTIFY_EOS:
        {
            pthread_mutex_lock(&p->eosMutex);
            p->bVideoRenderEosReceived = 1;
            if(p->bAudioRenderEosReceived == 1 || PlayerHasAudio((Player*)p) == 0 || p->bAudioCrash == 1)
            {
                //*When recive eos, we should stop the avTimer,
                //*or the PlayerGetPts() function will return wrong values.
                if(p->pAvTimer != NULL)
                    p->pAvTimer->Stop(p->pAvTimer);

                if(p->callback != NULL)
                    p->callback(p->pUserData, PLAYER_NOTIFY_EOS, NULL);
            }
            pthread_mutex_unlock(&p->eosMutex);
            return 0;
        }

        case PLAYER_AUDIO_RENDER_NOTIFY_EOS:
        {
            pthread_mutex_lock(&p->eosMutex);
            p->bAudioRenderEosReceived = 1;
            if(p->bVideoRenderEosReceived == 1 || PlayerHasVideo((Player*)p) == 0 || p->bVideoCrash == 1)
            {
                //*When recive eos, we should stop the avTimer,
                //*or the PlayerGetPts() function will return wrong values.
                if(p->pAvTimer != NULL)
                    p->pAvTimer->Stop(p->pAvTimer);

                if(p->callback != NULL)
                    p->callback(p->pUserData, PLAYER_NOTIFY_EOS, NULL);
            }
            pthread_mutex_unlock(&p->eosMutex);
            return 0;
        }

        case PLAYER_SUBTITLE_RENDER_NOTIFY_EOS:
        {
            //* unconcerned.
            return 0;
        }

        case PLAYER_VIDEO_RENDER_NOTIFY_VIDEO_SIZE:
        {
            if(p->callback != NULL)
            {
		p->callback(p->pUserData, PLAYER_NOTIFY_VIDEO_SIZE, param);
                p->callback(p->pUserData, PLAYER_NOTIFY_FIRST_PICTURE, NULL);
            }
		return 0;
        }

        case PLAYER_VIDEO_RENDER_NOTIFY_VIDEO_CROP:
        {
            p->sVideoCropWindow[0] = ((int*)param)[0];
            p->sVideoCropWindow[1] = ((int*)param)[1];
            p->sVideoCropWindow[2] = ((int*)param)[2];
            p->sVideoCropWindow[3] = ((int*)param)[3];

            if(p->callback != NULL) {
		p->callback(p->pUserData, PLAYER_NOTIFY_VIDEO_CROP, param);
            }

		return 0;
        }

        case PLAYER_VIDEO_RENDER_NOTIFY_VIDEO_BUFFER:
        {
		if(p->callback)
		{
			p->callback(p->pUserData, PLAYER_NOTIFY_VIDEO_PIC_DATA, param);
		}
		return 0;
        }

        case PLAYER_AUDIO_RENDER_NOTIFY_AUDIO_BUFFER:
        {
		if(p->callback)
		{
			p->callback(p->pUserData, PLAYER_NOTIFY_AUDIO_PCM_DATA, param);
		}
		return 0;
        }

        case PLAYER_VIDEO_RENDER_NOTIFY_FIRST_PICTURE:
        {
            p->nFirstVideoRenderPts = *((int64_t*)param);

            logd("first video pts = %lld", p->nFirstVideoRenderPts);

            if(p->bInFastMode)
            {
                //* in fast mode, video is showed without any synchronization,
                //* audio data is discard if overtime.
                p->nPreVideoPts = p->nFirstVideoRenderPts;
                return 0;
            }

            if(PlayerHasAudio((Player*)p) == 0)
            {
                pthread_mutex_lock(&p->timerMutex);
                p->nTimeBase = p->nFirstVideoRenderPts;
                p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);
                p->pAvTimer->Start(p->pAvTimer);
                pthread_mutex_unlock(&p->timerMutex);
                logd("no audio, set time to %lld and start timer.", p->nTimeBase);
            }
            else
            {
                int nWaitTime = 0;

                while(p->pAvTimer->GetStatus(p->pAvTimer) != TIMER_STATUS_START)
                {
                    //* wait for audio first frame.
                    if(p->bProcessingCommand || nWaitTime >= CONFIG_MAX_WAIT_TIME_FOR_SYNC || p->bAudioCrash || p->bAudioRenderEosReceived || p->bDiscardAudio)
                    {
                        //* need process new command, or wait too long.
                        logw("break audio video first sync.");
                        break;
                    }

#if CONFIG_USE_LARGER_PTS_AS_START_TIME
                    //* if the video first pts is smaller (too much) than the audio first pts, discard
                    //* this frame to catch up the audio first frame.
                    if(p->nFirstAudioRenderPts != -1)
                    {
                        if(p->nFirstAudioRenderPts > (p->nFirstVideoRenderPts + 50000))
                            return TIMER_DROP_VIDEO_DATA;
                    }
#endif
                    usleep(10*1000);
                    nWaitTime += 10000;
                }

                if(p->bProcessingCommand && nWaitTime < CONFIG_MAX_WAIT_TIME_FOR_SYNC && p->bAudioCrash == 0)
                {
                    //* the waiting process for first audio video sync is broken by new command.
                    //* tell the video render notify the first pts message later.
                    p->nFirstVideoRenderPts = -1;
                    return TIMER_NEED_NOTIFY_AGAIN;
                }

                pthread_mutex_lock(&p->timerMutex);
                if(p->pAvTimer->GetStatus(p->pAvTimer) != TIMER_STATUS_START)
                {
                    p->nTimeBase = p->nFirstVideoRenderPts;
                    p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);
                    p->pAvTimer->Start(p->pAvTimer);
                }
                pthread_mutex_unlock(&p->timerMutex);
            }

            p->nPreVideoPts = p->nFirstVideoRenderPts;
            return 0;
        }

        case PLAYER_AUDIO_RENDER_NOTIFY_FIRST_FRAME:
        {
		if(p->bDiscardAudio)
                return TIMER_DROP_AUDIO_DATA;

            if(p->bInFastMode)
            {
                //* discard audio in fast mode for IPTV
                if(p->bDiscardAudio)
			return TIMER_DROP_AUDIO_DATA;

                //* in fast mode, video is showed without any synchronization,
                //* audio data is discard if overtime.
                int64_t nAudioPts = *((int64_t*)param);
                //if(p->nPreVideoPts == -1 || nAudioPts < (p->nPreVideoPts - 150*1000))
                //{
                //	logw("TIMER_DROP_AUDIO_DATA");
                //    return TIMER_DROP_AUDIO_DATA;
                //}
                //else
                {
                    p->nFirstAudioRenderPts = *((int64_t*)param);
                    return 0;
                }
            }

            p->nFirstAudioRenderPts = *((int64_t*)param);

            logd("first audio pts = %lld", p->nFirstAudioRenderPts);

            if(PlayerHasVideo((Player*)p) == 0)
            {
                pthread_mutex_lock(&p->timerMutex);
                p->nTimeBase = p->nFirstAudioRenderPts;
                p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);
                p->pAvTimer->Start(p->pAvTimer);
                pthread_mutex_unlock(&p->timerMutex);
            }
            else
            {
                int nWaitTime = 0;
                int eTimerStatus;

                while(p->nFirstVideoRenderPts == -1)
                {
                    if(p->bProcessingCommand || nWaitTime >= CONFIG_MAX_WAIT_TIME_FOR_SYNC || p->bVideoCrash || p->bVideoRenderEosReceived)
                    {
                        //* need process new command, or wait too long.
                        logw("break audio video first sync.");
                        break;
                    }

                    usleep(10*1000);
                    nWaitTime += 10000;
                }

                if(p->bProcessingCommand && nWaitTime < CONFIG_MAX_WAIT_TIME_FOR_SYNC && p->bVideoCrash == 0)
                {
                    //* the waiting process for first audio video sync is broken by new command.
                    //* tell the audio render notify the first pts message later.
                    p->nFirstAudioRenderPts = -1;
                    return TIMER_NEED_NOTIFY_AGAIN;
                }

                pthread_mutex_lock(&p->timerMutex);
                eTimerStatus = p->pAvTimer->GetStatus(p->pAvTimer);
                pthread_mutex_unlock(&p->timerMutex);

                if(eTimerStatus != TIMER_STATUS_START)    //* timer not started yet.
                {
                    if(p->nFirstVideoRenderPts != -1)
                    {
                        if(p->nFirstVideoRenderPts <= p->nFirstAudioRenderPts)
                        {
                            if((p->nFirstAudioRenderPts - p->nFirstVideoRenderPts) <=
                                    CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP)
                            {
                                //* time difference seems to be normal. (less than 'threshold' seconds.)

#if CONFIG_USE_LARGER_PTS_AS_START_TIME
                                //* wait some time for the video first pts run to nFirstAudioRenderPts
                                //* by discarding video first picture.
                                nWaitTime = 0;
                                while(p->nFirstAudioRenderPts > (p->nFirstVideoRenderPts + 50000))
                                {
                                    if(p->bProcessingCommand || nWaitTime >= CONFIG_MAX_WAIT_TIME_FOR_SYNC)
                                    {
                                        //* need process new command, or wait too long, break waiting.
                                        logw("break audio first frame waiting for clock.");
                                        break;
                                    }

                                    usleep(50000);
                                    nWaitTime += 50000;
                                }

                                //* set smaller as the start time.
                                pthread_mutex_lock(&p->timerMutex);
                                if(p->nFirstVideoRenderPts <= p->nFirstAudioRenderPts)
                                    p->nTimeBase = p->nFirstVideoRenderPts;
                                else
                                    p->nTimeBase = p->nFirstAudioRenderPts;
                                p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);
                                p->pAvTimer->Start(p->pAvTimer);
                                pthread_mutex_unlock(&p->timerMutex);
#else
                                //* start timer and let audio wait sometime for clock run to nFirstAudioRenderPts.
                                pthread_mutex_lock(&p->timerMutex);
                                p->nTimeBase = p->nFirstVideoRenderPts;
                                p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);
                                p->pAvTimer->Start(p->pAvTimer);
                                pthread_mutex_unlock(&p->timerMutex);

                                nWaitTime = 0;
                                while(p->nFirstAudioRenderPts >
                                        (p->pAvTimer->GetTime(p->pAvTimer) + 50000))
                                {
                                    if(p->bProcessingCommand || nWaitTime >= CONFIG_MAX_WAIT_TIME_FOR_SYNC)
                                    {
                                        //* need process new command, or wait too long, break waiting.
                                        logw("break audio first frame waiting for clock.");
                                        break;
                                    }

                                    usleep(50000);
                                    nWaitTime += 50000;
                                }
#endif  //* end of CONFIG_USE_LARGER_PTS_AS_START_TIME
                            }
                            else
                            {
                                //* time difference is more than 2 seconds, to prevend from pts jump
                                //* case, we set the base time to the audio first pts and let the
                                //* synchronize job to be done at pts call back process later.
                                logw("nFirstAudioRenderPts = %lld, nFirstVideoRenderPts == %lld, difference too big.", \
                                        p->nFirstAudioRenderPts, p->nFirstVideoRenderPts);

                                //* reset the timer value to audio pts.
                                p->nTimeBase = p->nFirstAudioRenderPts;
                                pthread_mutex_lock(&p->timerMutex);
                                p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);
                                p->pAvTimer->Start(p->pAvTimer);
                                pthread_mutex_unlock(&p->timerMutex);

                                //* What is the effect if time difference is more than 2 seconds at
                                //* the file start? It seems that video display will flush
                                //* quickly to catch the timer.
                                //* Any method to resolve this problem except enlarge the 2 seconds
                                //* limits? Oh, Live is so hard to me.
                            }
                        }
                        else    //* p->nFirstVideoRenderPts > p->nFirstAudioRenderPts
                        {
                            //* audio first pts is smaller,
                            //* if CONFIG_USE_LARGER_PTS_AS_START_TIME is set,
                            //* choose video first pts as the base time, discard audio data before first picture;
                            //* else
                            //* choose the audio first pts as base time.

                            if((p->nFirstVideoRenderPts - p->nFirstAudioRenderPts) <=
                                    CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP)
                            {
                                //* time difference seems to be normal.
#if CONFIG_USE_LARGER_PTS_AS_START_TIME
                                //* audio discard some data to follow the first picture pts.
                                if(p->nFirstVideoRenderPts - p->nFirstAudioRenderPts > 50000)
                                {
                                    //* difference is more than 50ms.
                                    //* do not start the timer, audio first frame callback will to come again after
                                    //* some data is dropped.
                                    return TIMER_DROP_AUDIO_DATA;
                                }
                                else
                                {
                                    //* difference is less than 50ms, start the timer.
                                    pthread_mutex_lock(&p->timerMutex);
                                    p->nTimeBase = p->nFirstVideoRenderPts;
                                    p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);
                                    p->pAvTimer->Start(p->pAvTimer);
                                    pthread_mutex_unlock(&p->timerMutex);
                                }
#else
                                //* choose the audio first pts as the base time.
                                pthread_mutex_lock(&p->timerMutex);
                                p->nTimeBase = p->nFirstAudioRenderPts;
                                p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);
                                p->pAvTimer->Start(p->pAvTimer);
                                pthread_mutex_unlock(&p->timerMutex);
#endif
                            }
                            else    //* time difference is more than threshold to judge pts jump.
                            {
                                //* time difference is unnormal, to prevend from pts jump case,
                                //* we set the base time to the audio first pts and let the
                                //* synchronize job to be done at pts call back process later.

                                logw("nFirstAudioRenderPts = %lld, nFirstVideoRenderPts == %lld, difference too big.", \
                                        p->nFirstAudioRenderPts, p->nFirstVideoRenderPts);

                                pthread_mutex_lock(&p->timerMutex);
                                p->nTimeBase = p->nFirstAudioRenderPts;
                                p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);
                                p->pAvTimer->Start(p->pAvTimer);
                                pthread_mutex_unlock(&p->timerMutex);

                                //* What is the effect if time difference is more than 'theashold' seconds at
                                //* the file begin? It seems that video display will stuff
                                //* for several seconds to wait the timer.
                                //* Any method to resolve this problem except enlarge the 'theashold' seconds
                                //* limits? I hate those guys who make media files with media
                                //* streams interleaved so bad, don't let me have a gun!
                            }
                        }
                    }
                    else    //* p->nFirstVideoRenderPts == -1
                    {
                        //* wait for video first frame too long, start timer anyway.
                        pthread_mutex_lock(&p->timerMutex);
                        p->nTimeBase = p->nFirstAudioRenderPts;
                        p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);
                        p->pAvTimer->Start(p->pAvTimer);
                        pthread_mutex_unlock(&p->timerMutex);
                    }
                }
                else
                {
                    //* in case when video thread wait for audio first frame too long at its first frame call
                    //* back process, the timer was force started even first audio frame not received yet.
                    //* another case that the timer is in start status before audio first frame received is
                    //* that audio stream was just switched.

                    int64_t nCurTime;

                    nCurTime = p->pAvTimer->GetTime(p->pAvTimer);
                    if(nCurTime <= p->nFirstAudioRenderPts)
                    {
                        //* audio wait sometime for clock run to nFirstAudioRenderPts.
                        nWaitTime = 0;
                        if(p->nFirstAudioRenderPts - nCurTime <=
                                CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP)
                        {
                            //* time difference seems to be normal. (less than 2 seconds.)
                            //* audio wait sometime for clock run to nFirstAudioRenderPts.
                            while(p->nFirstAudioRenderPts >
                                  (p->pAvTimer->GetTime(p->pAvTimer) + 50000))
                            {
                                if(p->bProcessingCommand || nWaitTime >= CONFIG_MAX_WAIT_TIME_FOR_SYNC)
                                {
                                    //* need process new command, or wait too long, break waiting.
                                    logw("break audio first frame waiting for clock.");
                                    break;
                                }

                                usleep(50000);
                                nWaitTime += 50000;
                            }
                        }
                        else
                        {
                            //* time difference is more than 'threshold' seconds, to prevend from pts jump
                            //* case, we set the base time to the audio first pts and let the
                            //* synchronize job to be done at pts call back process later.
                            logw("nFirstAudioRenderPts = %lld, nFirstVideoRenderPts == %lld, difference too big.", \
                                    p->nFirstAudioRenderPts, p->nFirstVideoRenderPts);

                            //* reset the timer value to audio pts.
                            p->nTimeBase = p->nFirstAudioRenderPts;
                            pthread_mutex_lock(&p->timerMutex);
                            p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);
                            pthread_mutex_unlock(&p->timerMutex);

                            //* What is the effect if time difference is more than 2 seconds at
                            //* audio stream switch case? It seems that video display will flush
                            //* quickly to catch the timer. So we should control audio stream
                            //* carefully for multi audio stream at the audioDecComponent.cpp module.
                        }
                    }
                    else    //* curTime > p->nFirstAudioRenderPts
                    {
                        //* audio first pts is smaller, we should discard some audio data
                        //* to catch up the timer.
                        if(nCurTime - p->nFirstAudioRenderPts <= CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP)
                        {
                            //* time difference seems to be normal. (less than 'threshold' seconds)
                            //* audio discard some data to catch up the timer.
                            if(p->nFirstAudioRenderPts + 50000 < nCurTime)
                            {
                                //* difference is more than 50ms.
                                //* discard audio data.
                                return TIMER_DROP_AUDIO_DATA;
                            }
                        }
                        else    //* time difference is more than 2 seconds.
                        {
                            //* time difference is unnormal, to prevend from pts jump case,
                            //* we set the base time to the audio first pts and let the
                            //* synchronize job to be done at pts call back process later.

                            logw("timer had been started, current time = %lld, \
                                  nFirstVideoRenderPts == %lld, difference too big.", \
                                    nCurTime, p->nFirstVideoRenderPts);

                            p->nTimeBase = p->nFirstAudioRenderPts;
                            p->pAvTimer->SetTime(p->pAvTimer, p->nTimeBase);

                            //* What is the effect if time difference is more than 2 seconds at
                            //* audio stream switch case? It seems that video display will stuff
                            //* for several seconds to wait the timer. So we should control audio
                            //* stream carefully for multi audio stream at the audioDecComponent.cpp
                            //* module.
                        }
                    }
                }
            }

            p->nPreAudioPts = p->nFirstAudioRenderPts;
            p->nLastTimeTimerAdjusted = p->nFirstAudioRenderPts;
            return 0;
        }

        case PLAYER_VIDEO_RENDER_NOTIFY_PICTURE_PTS:
        {
            int64_t nVideoPts;
            int64_t nCurTime;
            int64_t nTimeDiff;
            int     nWaitTimeMs;

            if(p->bInFastMode)
            {
                //* in fast mode, video is showed without any synchronization,
                //* audio data is discard if overtime.
                p->nPreVideoPts = *((int64_t*)param);
                return 0;
            }

            nVideoPts = *((int64_t*)param);
            if(p->pVideoFramerateEstimater != NULL)
                FramerateEstimaterUpdate(p->pVideoFramerateEstimater, nVideoPts);

            nCurTime  = p->pAvTimer->GetTime(p->pAvTimer);
            nTimeDiff = nVideoPts - nCurTime;

            logv("notify video pts = %lld ms, curTime = %lld ms, diff = %lld ms",
                    nVideoPts/1000, nCurTime/1000, nTimeDiff/1000);

			if (p->onResetNotSync)
			{
				if (CdxAbs(nVideoPts - p->ptsBeforeReset) <= 2000000)
				{
					nWaitTimeMs = 0;
					p->nPreVideoPts = nVideoPts;
					return nWaitTimeMs;
				}
				else
				{
					p->onResetNotSync = 0;
				}
			}

			if(!IS_DTMB_STREAM || (PlayerHasAudio((Player*)p) && (p->bAudioCrash == 0)))
			{
	            if(CdxAbs(nTimeDiff) <= CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP)
	            {
	                //* time difference seems to be normal, video render thread will
	                //* wait to sync the timer if nTimeDiff>0 or hurry up to catch the
	                //* timer if nTimeDiff<0.
	                p->nUnsyncVideoFrameCnt = 0;
	                nWaitTimeMs = (int)(nTimeDiff/1000);
	            }
	            else
	            {
	                //* time difference unnormal.
	                if(PlayerHasAudio((Player*)p) == 0)
	                {
	                    //* if no audio, just reset the timer, it should be a pts jump event.
	                    if(p->nUnsyncVideoFrameCnt > 2)
	                    {
	                        nWaitTimeMs = 0;
	                        p->pAvTimer->SetTime(p->pAvTimer, nVideoPts);
	                    }
	                    else
	                    {
	                        nWaitTimeMs = 40;
	                        p->nUnsyncVideoFrameCnt++;
	                    }
	                }
	                else
	                {
	                    //* it maybe a pts jump event,
	                    //* if one of the video pts or audio pts jump first and the other
	                    //* not jump yet, the video pts will get a big difference to the timer,
	                    //* we just go for several frames for a while to see whether the other
	                    //* guy (video pts or audio pts) will jump also.
	                    //* here we go ahead for 30 frames at maximum.
	                    if(p->nUnsyncVideoFrameCnt > 30)
	                    {
	                        //* we have go for more than 30 frames and the pts still not synchronized,
	                        //* it seems not a pts jump event, it seem like a pts error or packet lost,
	                        //* we force the video to hurry up(nWaitTimeMs<0) or wait(nWaitTime>0) to
	                        //* sync to the timer.
	                        //* (but for save, don't wait too long for a frame, 2 seconds)
	                        if(nTimeDiff < -CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP)
	                            nWaitTimeMs = -2000;
	                        else
	                            nWaitTimeMs = 2000;
	                    }
	                    else
	                    {
	                        //* wait for one frame duration.
	                        if(p->pVideoFramerateEstimater != NULL)
	                            nWaitTimeMs = FramerateEstimaterGetFrameDuration(p->pVideoFramerateEstimater)/1000;
	                        else
	                            nWaitTimeMs = 40;
	                        p->nUnsyncVideoFrameCnt++;
	                    }

						//add by xuqi, for IPTV
	                    nWaitTimeMs = 0;
	                }
	                logv("%d video frames not sync, force sync. vpts(%.3f) jztime(%.3f)",
	                        p->nUnsyncVideoFrameCnt, nVideoPts/1000000.0, nCurTime/1000000.0);
	            }
			}
			else
			{
	            if(CdxAbs(nTimeDiff) <= CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP_DTMB)
	            {
	                p->nUnsyncVideoFrameCnt = 0;
	                nWaitTimeMs = (int)(nTimeDiff/1000);
	            }
	            else
	            {
	                //* time difference unnormal.
	                if(PlayerHasAudio((Player*)p) == 0 || (PlayerHasAudio((Player*)p) == 1 && p->bAudioCrash == 1))
	                {
	                    if(p->nUnsyncVideoFrameCnt > 2)
	                    {
	                        nWaitTimeMs = 0;
	                        p->pAvTimer->SetTime(p->pAvTimer, nVideoPts);
	                    }
	                    else
	                    {
	                        nWaitTimeMs = 40;
	                        p->nUnsyncVideoFrameCnt++;
	                    }
	                }
	                else
	                {
	                    if(p->nUnsyncVideoFrameCnt > 30)
	                    {
	                        if(nTimeDiff < -CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP)
	                            nWaitTimeMs = -2000;
	                        else
	                            nWaitTimeMs = 2000;
	                    }
	                    else
	                    {
	                        //* wait for one frame duration.
	                        if(p->pVideoFramerateEstimater != NULL)
	                            nWaitTimeMs = FramerateEstimaterGetFrameDuration(p->pVideoFramerateEstimater)/1000;
	                        else
	                            nWaitTimeMs = 40;
	                        p->nUnsyncVideoFrameCnt++;
	                    }

	                    nWaitTimeMs = 0;
	                }
	                logv("%d video frames not sync, force sync. vpts(%.3f) jztime(%.3f)",
	                        p->nUnsyncVideoFrameCnt, nVideoPts/1000000.0, nCurTime/1000000.0);
	            }
			}

            p->nPreVideoPts = nVideoPts;

            return nWaitTimeMs;
        }

        case PLAYER_AUDIO_RENDER_NOTIFY_PTS_AND_CACHETIME:
        {
            int64_t nAudioPts;
            int64_t nCachedTimeInSoundDevice;
            int64_t nTimeDiff;
            int64_t nCurTime;
            int64_t nCurAudioTime;


            pthread_mutex_lock(&p->timerMutex);

            nAudioPts = ((int64_t*)param)[0];
            nCachedTimeInSoundDevice = ((int64_t*)param)[1];
            nCurTime = p->pAvTimer->GetTime(p->pAvTimer);
            nCurAudioTime = nAudioPts - nCachedTimeInSoundDevice;
            nTimeDiff = nCurAudioTime - nCurTime;

            if(p->bDiscardAudio)
            {
		pthread_mutex_unlock(&p->timerMutex);
                return TIMER_DROP_AUDIO_DATA;
            }

            if(p->bInFastMode)
            {
                p->nPreAudioPts = nAudioPts;
                pthread_mutex_unlock(&p->timerMutex);

                //* discard audio in fast mode for IPTV
                if(p->bDiscardAudio)
			return TIMER_DROP_AUDIO_DATA;

                //* in fast mode, video is showed without any synchronization,
                //* audio data is discard if overtime.
                //if(p->nPreAudioPts < (p->nPreVideoPts - 200*1000))
                //{
                //	logw("TIMER_DROP_AUDIO_DATA");
                //    return TIMER_DROP_AUDIO_DATA;
                //}
                //else
                {
                    return 0;
                }

            }

            logv("notify audio pts = %lld ms, curTime = %lld ms, diff = %lld ms, cacheTime = %lld ms",
                nAudioPts/1000, nCurTime/1000, nTimeDiff/1000, nCachedTimeInSoundDevice/1000);

            //* check whether audio pts jump.
            if(CdxAbs(nTimeDiff) > CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP)
            {
                //* Time difference is unnormal, there may be a pts jump event in the streams.
                //* Any way, we just adjust the time base to keep the play position goes normal.
                if((nAudioPts > p->nPreAudioPts + CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP) ||
                   (nAudioPts + CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP < p->nPreAudioPts))
                {
                    int64_t nCurPosition;

                    logw("audio pts jump, maybe pts loop back or data lost.");
                    //* update timebase.
                    nCurPosition = nCurTime - p->nTimeBase;
                    p->nTimeBase = nCurAudioTime - nCurPosition;
                }
            }

            if(CdxAbs(nTimeDiff) > 100000)   //* difference is more than 100 ms.
            {
                //* time difference is too big, we can not adjust the timer speed to make it
                //* become synchronize in a short time,
                //* so, just reset the timer, this will make the video display flush or stuff.
                logw("reset the timer to %.3f, time difference is %.3f",
                    nCurAudioTime/1000000.0, nTimeDiff/1000000.0);
                p->pAvTimer->SetTime(p->pAvTimer, nCurAudioTime);
                p->nLastTimeTimerAdjusted = nAudioPts;

				//for IPTV
                if(CdxAbs(nTimeDiff) > 2000000)
                {
	                p->onResetNotSync = 1;
					p->ptsBeforeReset = nCurTime;
				}
            }
            else
            {
                if((nAudioPts > p->nLastTimeTimerAdjusted + 1000000) ||
                   (p->nLastTimeTimerAdjusted > nAudioPts))
                {
                    //* adjust timer speed to make the timer follow audio.
                    if(nTimeDiff > 50000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 1006);
                    else if(nTimeDiff > 40000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 1005);
                    else if(nTimeDiff > 30000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 1004);
                    else if(nTimeDiff > 20000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 1003);
                    else if(nTimeDiff > 10000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 1002);
                    else if(nTimeDiff > 5000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 1001);
                    else if(nTimeDiff < -50000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 994);
                    else if(nTimeDiff < -40000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 995);
                    else if(nTimeDiff < -30000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 996);
                    else if(nTimeDiff < -20000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 997);
                    else if(nTimeDiff < -10000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 998);
                    else if(nTimeDiff < -5000)
                        p->pAvTimer->SetSpeed(p->pAvTimer, 999);
                    else
                        p->pAvTimer->SetSpeed(p->pAvTimer, 1000);

                    p->nLastTimeTimerAdjusted = nAudioPts;
                }
            }

            pthread_mutex_unlock(&p->timerMutex);
            p->nPreAudioPts = nAudioPts;

            return 0;
        }

        case PLAYER_SUBTITLE_RENDER_NOTIFY_ITEM_PTS_AND_DURATION:
        {
            //* check whether a subtitle item is time to show or expired.
            int64_t nPts;
            int64_t nEndTime;
            int64_t nDuration;
            int64_t nCurTime;
            int64_t nCurInputPts;
            int     eTimerStatus;
            int     nStreamIndex;
            int     bExternal;
            int     bHasBeenShowed;
            int     nWaitTimeMs;

            nPts      = ((int64_t*)param)[0];
            nDuration = ((int64_t*)param)[1];
            nEndTime  = nPts + nDuration;

            bHasBeenShowed = (int)((int64_t*)param)[2];
            nCurTime = p->pAvTimer->GetTime(p->pAvTimer);

            if(p->bInFastMode)
            {
                //* in fast mode, video is showed without any synchronization,
                //* audio and subtitle data is discard.
                return -1;  //* means discard subtitle item.
            }

            eTimerStatus = p->pAvTimer->GetStatus(p->pAvTimer);
            if(eTimerStatus != TIMER_STATUS_START)
                return 100; //* not started yet, wait 100ms for first audio and video frame arrive.

            if(nCurTime >= nPts && nCurTime < nEndTime)
                return 0;   //* show this item.

            nStreamIndex = SubtitleDecCompCurrentStreamIndex(p->pSubtitleDecComp);
            bExternal = SubtitleDecCompIsExternalSubtitle(p->pSubtitleDecComp, nStreamIndex);

            if(nCurTime < nPts)
            {
                if(bHasBeenShowed)
                {
                    logw("subtitle item had been showed but now pts seems not match, may be timer loop back.");
                    return -1;  //* means discard subtitle item.
                }
                else if(bExternal)
                {
                    //* when (nPts-nCurTime) less than 1000 us, the nWaitTimeMs will be 0,
                    //* player.cpp will return 0 to subtitleRender.
                    //* if return 0, subtitleRender will send the subItem to display(it is not the time to the subItem to display),
                    //* and when check the subItem whether had expired , it will return -1 because (nCurTime < nPts) and (bHasBeenShowed==1)
                    //* so we will see that the subItem will disappear.
                    //* here we enlarge 999 to avoid the unexpected wrong.
                    nWaitTimeMs = (int)((nPts-nCurTime+999)/1000);
                    return nWaitTimeMs;
                }
                else
                {
                    if(nPts-nCurTime > CONFIG_THREASHOLD_OF_PTS_DIFFERENCE_TO_JUDGE_PTS_jUMP_FOR_SUBTITLE)
                    {
                        //* time difference more than 'threshold_subtitle' second, it should not happen in embedded subtitle
                        //* except it is a pts loop back or
                        //* it is a bad interleave file(subtitle is put ahead the video and audio too much).
                        //* check whether it is a pts loop back case,
                        //* if it is, the subtitle is expired, should drop it.

                        //* if it is cased by a clock loop back,
                        //* we should have nPts > nLastInputPts > nCurTime;
                        //* (except user submit too much stream of m3u8/dash source, in which case pts always loop back in a short time.)
                        if(nPts > p->nLastInputPts)
                        {
                            //* PTS loop back like this:
                            //*    small         big small          big
                            //* pts: |------------|    |-------------|
                            //*                 |        |     |
                            //*                nPts      |     |
                            //*                       curTime  |
                            //*                           nLastInputPts
                            return -1;  //* means discard subtitle item.
                        }
                        else
                        {
                            //* to prevent from subtitle stream stuff cased by invalid judgement,
                            //* check whether the subtitle stream buffer is full,
                            //* if so, discard this subtitle item.
                            int nStreamDataSize;
                            int nStreamBufferSize;
                            int nFullness;

                            nStreamDataSize = SubtitleDecCompStreamBufferSize(p->pSubtitleDecComp, nStreamIndex);
                            nStreamBufferSize = SubtitleDecCompStreamDataSize(p->pSubtitleDecComp, nStreamIndex);
                            nFullness = nStreamDataSize*100/nStreamBufferSize;
                            if(nFullness >= 90)
                                return -1;  //* means discard subtitle item.
                        }
                    }

                    //* when (nPts-nCurTime) less than 1000 us, the nWaitTimeMs will be 0,
                    //* player.cpp will return 0 to subtitleRender.
                    //* if return 0, subtitleRender will send the subItem to display(it is not the time to the subItem to display),
                    //* and when check the subItem whether had expired , it will return -1 because (nCurTime < nPts) and (bHasBeenShowed==1)
                    //* so we will see that the subItem will disappear.
                    //* here we enlarge 999 to avoid the unexpected wrong.
                    nWaitTimeMs = (int)((nPts-nCurTime+999)/1000);
                    return nWaitTimeMs;
                }
            }
            else
            {
                //* nCurTime >= nEndTime.
                if(bHasBeenShowed)
                    return -1;  //* means discard subtitle item.
                else if(bExternal)
                {
                    return -1;  //* means discard subtitle item.
                }
                else
                {
                    if(nCurTime-nEndTime > 2000000) //* here we set the threshold as 2 seconds.
                    {
                        //* time difference more than 'threshold_subtitle' second, check whether it is a loop back case,
                        //* if it is, wait for the timer loop back.

                        //* if it is cased by a clock loop back,
                        //* we should have nCurTime > nLastInputPts > nPts; otherwise it
                        //* should be a normal case that the subtitle is expired. we should have nLastInputPts >= nCurTime > nEndTime;
                        if(p->nLastInputPts >= nCurTime)
                        {
                            return -1;
                        }
                        else
                        {
                            //* PTS loop back like this:
                            //*    small         big small          big
                            //* pts: |------------|    |-------------|
                            //*                 |        |     |
                            //*              curTime     |     |
                            //*                         nPts   |
                            //*                           nLastInputPts

                            //* to prevent from subtitle stream stuff cased by invalid judgement,
                            //* check whether the subtitle stream buffer is full,
                            //* if so, discard this subtitle item.
                            int nStreamDataSize;
                            int nStreamBufferSize;
                            int nFullness;
                            nStreamDataSize = SubtitleDecCompStreamBufferSize(p->pSubtitleDecComp, nStreamIndex);
                            nStreamBufferSize = SubtitleDecCompStreamDataSize(p->pSubtitleDecComp, nStreamIndex);
                            nFullness = nStreamDataSize*100/nStreamBufferSize;
                            if(nFullness >= 90)
                                return -1;  //* means discard subtitle item.

                            return 100;  //* wait 100ms for the timer loop back.
                        }
                    }

                    return -1;  //* means discard subtitle item.
                }
            }

            break;
        }

        case PLAYER_SUBTITLE_RENDER_NOTIFY_ITEM_AVAILABLE:
        {
            //* notify to draw a subtitle item.
            //* subtitle_id   = ((unsigned int*)param)[0];
            //* pSubtitleItem = (SubtitleItem*)((unsigned int*)param)[1];
            if(p->callback)
                p->callback(p->pUserData, PLAYER_NOTIFY_SUBTITLE_ITEM_AVAILABLE, param);
            return 0;
        }

        case PLAYER_SUBTITLE_RENDER_NOTIFY_ITEM_EXPIRED:
        {
            //* notify to clear a subtitle item.
            //* subtitle_id = (unsigned int)param;
            if(p->callback)
                p->callback(p->pUserData, PLAYER_NOTIFY_SUBTITLE_ITEM_EXPIRED, param);
            return 0;
        }

        case PLAYER_VIDEO_DECODER_NOTIFY_STREAM_UNDERFLOW:
        case PLAYER_AUDIO_DECODER_NOTIFY_STREAM_UNDERFLOW:
        case PLAYER_SUBTITLE_DECODER_NOTIFY_STREAM_UNDERFLOW:
            break;

        case PLAYER_VIDEO_DECODER_NOTIFY_CRASH:

            pthread_mutex_lock(&p->eosMutex);

            p->bVideoCrash = 1;
            if(p->callback != NULL)
                p->callback(p->pUserData, PLAYER_NOTIFY_VIDEO_UNSUPPORTED, NULL);

            if(p->bAudioRenderEosReceived == 1 || PlayerHasAudio((Player*)p) == 0 || p->bAudioCrash == 1)
            {
                if(p->callback != NULL)
                    p->callback(p->pUserData, PLAYER_NOTIFY_EOS, NULL);
            }
            pthread_mutex_unlock(&p->eosMutex);
            break;

        case PLAYER_VIDEO_RENDER_NOTIFY_VIDEO_FRAME:
            if(p->callback != NULL)
            {
                logv("===== notify render key frame in fast mode");
                p->callback(p->pUserData, PLAYER_NOTIFY_VIDEO_RENDER_FRAME, NULL);
            }
            break;

        case PLAYER_AUDIO_DECODER_NOTIFY_CRASH:

            pthread_mutex_lock(&p->eosMutex);

            p->bAudioCrash = 1;
            if(p->callback != NULL)
                p->callback(p->pUserData, PLAYER_NOTIFY_AUDIO_UNSUPPORTED, NULL);

            if(p->bVideoRenderEosReceived == 1 || PlayerHasVideo((Player*)p) == 0 || p->bVideoCrash == 1)
            {
                if(p->callback != NULL)
                    p->callback(p->pUserData, PLAYER_NOTIFY_EOS, NULL);
            }
            pthread_mutex_unlock(&p->eosMutex);
            break;

        case PLAYER_SUBTITLE_DECODER_NOTIFY_CRASH:

            pthread_mutex_lock(&p->eosMutex);

            p->bSubtitleCrash = 1;
            if(p->callback != NULL)
                p->callback(p->pUserData, PLAYER_NOTIFY_SUBTITLE_UNSUPPORTED, NULL);

            pthread_mutex_unlock(&p->eosMutex);
            break;

        case PLAYER_AUDIO_DECODER_NOTIFY_AUDIORAWPLAY:
			loge("%s,1",__func__);
            if(p->callback != NULL)
                p->callback(p->pUserData, PLAYER_NOTIFY_AUDIORAWPLAY, param);
			loge("%s,2",__func__);
			break;

        case PLAYER_VIDEO_DECODER_NOTIFY_SET_SECURE_BUFFER_COUNT:

            if(p->callback != NULL)
                p->callback(p->pUserData, PLAYER_NOTIFY_SET_SECURE_BUFFER_COUNT, param);
            break;

        case PLAYER_VIDEO_DECODER_NOTIFY_SET_SECURE_BUFFERS:

            if(p->callback != NULL)
                p->callback(p->pUserData, PLAYER_NOTIFY_SET_SECURE_BUFFERS, param);
            break;

        case PLAYER_AUDIO_RENDER_NOTIFY_AUDIO_INFO:

            if(p->callback != NULL)
            {
				int nAudioInfo[4];
				nAudioInfo[0] = p->nAudioStreamSelected;
				nAudioInfo[1] = ((int *)param)[0];
				nAudioInfo[2] = ((int *)param)[1];
				nAudioInfo[3] = ((int *)param)[2] > 0 ? ((int *)param)[2] : 320*1000;
                p->callback(p->pUserData, PLAYER_NOTIFY_AUDIO_INFO, (void *)nAudioInfo);
            }
            break;

        default:
            break;
    }

    return 0;
}



static int PlayerInitialVideo(PlayerContext* p)
{
    if(p->pVideoRender != NULL)
    {
        VideoRenderCompDestroy(p->pVideoRender);
        p->pVideoRender = NULL;
    }

    if(p->pVideoDecComp != NULL)
    {
        VideoDecCompDestroy(p->pVideoDecComp);
        p->pVideoDecComp = NULL;
    }

    if(p->pVideoBitrateEstimater != NULL)
    {
        BitrateEstimaterDestroy(p->pVideoBitrateEstimater);
        p->pVideoBitrateEstimater = NULL;
    }

    if(p->pVideoFramerateEstimater != NULL)
    {
        FramerateEstimaterDestroy(p->pVideoFramerateEstimater);
        p->pVideoFramerateEstimater = NULL;
    }

    p->sVideoCropWindow[0] = 0;
    p->sVideoCropWindow[1] = 0;
    p->sVideoCropWindow[2] = 0;
    p->sVideoCropWindow[3] = 0;

    p->pVideoDecComp = VideoDecCompCreate();

#if (OUTPUT_PIXEL_FORMAT == OUTPUT_PIXEL_FORMAT_YV12)
	p->sVideoConfig.eOutputPixelFormat = PIXEL_FORMAT_YV12;
#elif (OUTPUT_PIXEL_FORMAT == OUTPUT_PIXEL_FORMAT_NV21)
    p->sVideoConfig.eOutputPixelFormat = PIXEL_FORMAT_NV21;
#elif (OUTPUT_PIXEL_FORMAT == OUTPUT_PIXEL_FORMAT_MB32)
    p->sVideoConfig.eOutputPixelFormat = PIXEL_FORMAT_YUV_MB32_420;
#endif

    //* We should set format to nv21 if it is 3D stream except chip-1639,
    //* Because new-display-double-stream need nv21 format
#if(NEW_DISPLAY_DOUBLE_STREAM_NEED_NV21 == 1 && USE_NEW_DISPLAY == 1)
    if(p->pVideoStreamInfo->bIs3DStream == 1)
        p->sVideoConfig.eOutputPixelFormat = PIXEL_FORMAT_NV21;
#endif

    p->sVideoConfig.bSecureosEn = p->pVideoStreamInfo->bSecureStreamFlagLevel1;

    p->sVideoConfig.nDisplayHoldingFrameBufferNum     = NUM_OF_PICTURES_KEEP_IN_LIST;
    p->sVideoConfig.nDeInterlaceHoldingFrameBufferNum = NUM_OF_PICTURES_KEEPPED_BY_DEINTERLACE;
    p->sVideoConfig.nRotateHoldingFrameBufferNum = NUM_OF_PICTURES_KEEPPED_BY_ROTATE;
    p->sVideoConfig.nDecodeSmoothFrameBufferNum = NUM_OF_PICTURES_FOR_EXTRA_SMOOTH;

#if (CONFIG_CHIP == OPTION_CHIP_C500)
	p->sVideoConfig.nVbvBufferSize = 2*1024*1024;
	p->sVideoConfig.nDeInterlaceHoldingFrameBufferNum = 0;
	p->sVideoConfig.nDisplayHoldingFrameBufferNum = 0;
	p->sVideoConfig.nRotateHoldingFrameBufferNum = 0;
	p->sVideoConfig.nDecodeSmoothFrameBufferNum = 1;
	logd("set vbv buf [2M] in c500");
#endif


    if(p->pVideoStreamInfo->nWidth >= (3840-256) || p->pVideoStreamInfo->nHeight >= (2160-256))
        p->sVideoConfig.nDecodeSmoothFrameBufferNum = 0;

    //* Because in the function of VideoDecCompSetVideoStreamInfo, we will call
    //* the callback function to do something, so we must setCallback before it.
    VideoDecCompSetCallback(p->pVideoDecComp, CallbackProcess, (void*)p);
    VideoDecCompSetTimer(p->pVideoDecComp, p->pAvTimer);

    p->bUnSurpportVideoFlag = 0;
    if(VideoDecCompSetVideoStreamInfo(p->pVideoDecComp, p->pVideoStreamInfo, &p->sVideoConfig) != 0)
    {
        loge("video stream is not supported.");

		if(p->callback != NULL)
		{
			p->callback(p->pUserData, PLAYER_NOTIFY_VIDEO_UNSUPPORTED, NULL);
		}

		return -1;
#if 0
        /* in this case the docoder did process, but audio and  subtitle will be work */

        p->bUnSurpportVideoFlag = 1;

        if(p->pUnSurpportVideoBuffer != NULL)
            free(p->pUnSurpportVideoBuffer);

        p->nUnSurpportVideoBufferSize = 256*1024;
        p->pUnSurpportVideoBuffer = malloc(p->nUnSurpportVideoBufferSize);
        if(p->pUnSurpportVideoBuffer == NULL)
        {
            loge("malloc buffer failed");
            return -1;
        }
#endif
    }

    p->pVideoRender = VideoRenderCompCreate();
    if(p->pVideoRender == NULL)
    {
        loge("create video render fail.");
        VideoDecCompDestroy(p->pVideoDecComp);
        p->pVideoDecComp = NULL;
        return -1;
    }

    if(p->pLayerControlOps)
    {
	VideoRenderCompSetLayerCtlOps(p->pVideoRender, p->pLayerControlOps);
    }

    //* set video stream info for videorender
    VideoRenderCompSetVideoStreamInfo(p->pVideoRender, p->pVideoStreamInfo);

    //* tall the render whether the output buffer is secure(should be protect by native window) or not
    VideoRenderCompSetProtecedFlag(p->pVideoRender,p->pVideoStreamInfo->bSecureStreamFlag);
    VideoRenderCompSetCallback(p->pVideoRender, CallbackProcess, (void*)p);
    VideoRenderCompSetTimer(p->pVideoRender, p->pAvTimer);
    VideoRenderCompSetDecodeComp(p->pVideoRender, p->pVideoDecComp);

#if(CONFIG_CMCC==OPTION_CMCC_YES)
	VideoRenderCompSetSyncFirstPictureFlag(p->pVideoRender, 1);
#else
	VideoRenderCompSetSyncFirstPictureFlag(p->pVideoRender, 0);
#endif
    if(p->pVideoStreamInfo->bIs3DStream)
        VideoRenderSet3DMode(p->pVideoRender, PICTURE_3D_MODE_TWO_SEPERATED_PICTURE, DISPLAY_3D_MODE_2D);
    if(p->pNativeWindow != NULL)
        VideoRenderCompSetWindow(p->pVideoRender, p->pNativeWindow);

    p->pVideoBitrateEstimater   = BitrateEstimaterCreate();
    p->pVideoFramerateEstimater = FramerateEstimaterCreate();

    VideoRenderCompSetFrameRateEstimater(p->pVideoRender, p->pVideoFramerateEstimater);

    return 0;
}


static int PlayerInitialAudio(PlayerContext* p)
{
    int ret;

    if(p->pAudioRender != NULL)
    {
        AudioRenderCompDestroy(p->pAudioRender);
        p->pAudioRender = NULL;
    }

    if(p->pAudioDecComp != NULL)
    {
        AudioDecCompDestroy(p->pAudioDecComp);
        p->pAudioDecComp = NULL;
    }

    if(p->pAudioBitrateEstimater != NULL)
    {
        BitrateEstimaterDestroy(p->pAudioBitrateEstimater);
        p->pAudioBitrateEstimater = NULL;
    }

    p->pAudioDecComp = AudioDecCompCreate();
    ret = AudioDecCompSetAudioStreamInfo(p->pAudioDecComp,
                                         p->pAudioStreamInfo,
                                         p->nAudioStreamNum,
                                         p->nAudioStreamSelected);

    if(ret != 0)
    {
        loge("selected audio stream is not supported, call SwitchAudio() to select another stream.");
        /* in this case the docoder did process, but video and  subtitle will be work */
    }

    AudioDecCompSetCallback(p->pAudioDecComp, CallbackProcess, (void*)p);
    AudioDecCompSetTimer(p->pAudioDecComp, p->pAvTimer);

    p->pAudioRender = AudioRenderCompCreate();
    if(p->pAudioRender == NULL)
    {
        loge("create audio render fail.");
        AudioDecCompDestroy(p->pAudioDecComp);
        p->pAudioDecComp = NULL;
        return -1;
    }

    if(p->pSoundControlOps)
    {
	AudioRenderSetSoundCtlOps(p->pAudioRender, p->pSoundControlOps);
    }

    AudioRenderCompSetVolume(p->pAudioRender, p->volume);
    AudioRenderCompSetCallback(p->pAudioRender, CallbackProcess, (void*)p);
    AudioRenderCompSetTimer(p->pAudioRender, p->pAvTimer);
    AudioRenderCompSetDecodeComp(p->pAudioRender, p->pAudioDecComp);
    if(p->pAudioSink != NULL)
        AudioRenderCompSetAudioSink(p->pAudioRender, p->pAudioSink);

    p->pAudioBitrateEstimater = BitrateEstimaterCreate();

    return 0;
}


static int PlayerInitialSubtitle(PlayerContext* p)
{
    int ret;

    if(p->pSubtitleRender != NULL)
    {
        SubtitleRenderCompDestroy(p->pSubtitleRender);
        p->pSubtitleRender = NULL;
    }

    if(p->pSubtitleDecComp != NULL)
    {
        SubtitleDecCompDestroy(p->pSubtitleDecComp);
        p->pSubtitleDecComp = NULL;
    }

    p->pSubtitleDecComp = SubtitleDecCompCreate();
    ret = SubtitleDecCompSetSubtitleStreamInfo(p->pSubtitleDecComp,
                                               p->pSubtitleStreamInfo,
                                               p->nSubtitleStreamNum,
                                               p->nSubtitleStreamSelected);

    if(ret != 0)
    {
        loge("selected subtitle stream is not supported, call SwitchSubtitle() to select another stream.");
        /* in this case the docoder did process, but video and  audio will be work */
    }

    SubtitleDecCompSetCallback(p->pSubtitleDecComp, CallbackProcess, (void*)p);
    SubtitleDecCompSetTimer(p->pSubtitleDecComp, p->pAvTimer);

    p->pSubtitleRender = SubtitleRenderCompCreate();
    if(p->pSubtitleRender == NULL)
    {
        loge("create subtitle render fail.");
        SubtitleDecCompDestroy(p->pSubtitleDecComp);
        p->pSubtitleDecComp = NULL;
        return -1;
    }
	SubtitleRenderCompSetVideoOrAudioFirstPts(p->pSubtitleRender, p->nFirstPts);
    SubtitleRenderCompSetCallback(p->pSubtitleRender, CallbackProcess, (void*)p);
    SubtitleRenderCompSetTimer(p->pSubtitleRender, p->pAvTimer);
    SubtitleRenderCompSetDecodeComp(p->pSubtitleRender, p->pSubtitleDecComp);

#if( CONFIG_ALI_YUNOS == OPTION_ALI_YUNOS_YES)
    ret = SubtitleRenderCompSetSubtitleStreamInfo(p->pSubtitleRender,
										p->pSubtitleStreamInfo,
										p->nSubtitleStreamNum,
										p->nSubtitleStreamSelected);
    if(ret != 0)
    {
        loge("Set Subtitle StreamInfo to RenderComp fail.");
    }
#endif

    return 0;
}
int PlayerSetVolume(Player* pl, float volume)
{
    PlayerContext* p;

    p = (PlayerContext*)pl;
    p->volume = volume;
    if(!p->pAudioRender)
    {
	logd("!p->pAudioRender");
        return -1;
    }
	return AudioRenderCompSetVolume(p->pAudioRender, p->volume);
}
int PlayerGetVolume(Player* pl, float *volume)
{
	logd("GetVolume");
    PlayerContext* p;
    p = (PlayerContext*)pl;
    *volume = p->volume;
    if(!p->pAudioRender)
    {
		logd("!p->pAudioRender");
        return 0;
    }
	return AudioRenderCompGetVolume(p->pAudioRender, volume);
}


//*****************************************************************************//
//*****************************************************************************//
//*****************************************************************************//

int PlayerConfigExtraScaleInfo(Player* pl, int nWidthTh, int nHeightTh, int nHorizontalScaleRatio, int nVerticalScaleRatio)
{
	PlayerContext* p;
	p = (PlayerContext*)pl;

	if(p->pVideoDecComp != NULL)
	{
		//return VideoDecCompSetDropDelayFrames(p->pVideoDecComp, bDropLaytedFrame); //* only treat the major video stream.
		return VideoDecCompSetExtraScaleInfo(p->pVideoDecComp,nWidthTh, nHeightTh, nHorizontalScaleRatio, nVerticalScaleRatio);
	}
	return 0;
}
