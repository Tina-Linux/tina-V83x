#define LOG_TAG "tplayer"
#include "tlog.h"
#include "tplayer.h"
#include "tsound_ctrl.h"
#ifndef ONLY_ENABLE_AUDIO
#include "tdisp_ctrl.h"
#endif
#include "tina_multimedia_version.h"

int CallbackFromXPlayer(void* pUserData, int msg, int ext1, void* param){
    TPlayer* tplayer = (TPlayer*)pUserData;
    TP_CHECK(tplayer);
    int appMsg = -1;
     switch(msg)
    {
        case AWPLAYER_MEDIA_PREPARED:
        {
            appMsg = TPLAYER_NOTIFY_PREPARED;
            break;
        }

        case AWPLAYER_MEDIA_PLAYBACK_COMPLETE:
        {
            appMsg = TPLAYER_NOTIFY_PLAYBACK_COMPLETE;
            break;
        }

        case AWPLAYER_MEDIA_SEEK_COMPLETE:
        {
            appMsg = TPLAYER_NOTIFY_SEEK_COMPLETE;
            break;
        }

        case AWPLAYER_MEDIA_ERROR:
        {
            appMsg = TPLAYER_NOTIFY_MEDIA_ERROR;
            switch(ext1)
            {
                case AW_MEDIA_ERROR_UNKNOWN:
                {
                    ext1 = TPLAYER_MEDIA_ERROR_UNKNOWN;
                    break;
                }
                case AW_MEDIA_ERROR_UNSUPPORTED:
                {
                    ext1 = TPLAYER_MEDIA_ERROR_UNSUPPORTED;
                    break;
                }
                case AW_MEDIA_ERROR_IO:
                {
                    ext1 = TPLAYER_MEDIA_ERROR_IO;
                    break;
                }
            }
            break;
        }
        case AWPLAYER_MEDIA_INFO:
        {
            switch(ext1)
            {
                case AW_MEDIA_INFO_NOT_SEEKABLE:
                {
                    appMsg = TPLAYER_NOTIFY_NOT_SEEKABLE;
                    break;
                }
                case AW_MEDIA_INFO_BUFFERING_START:
                {
                    appMsg = TPLAYER_NOTIFY_BUFFER_START;
                    break;
                }
                case AW_MEDIA_INFO_BUFFERING_END:
                {
                    appMsg = TPLAYER_NOTIFY_BUFFER_END;
                    break;
                }
            }
            break;
        }
#ifndef ONLY_ENABLE_AUDIO
        case AWPLAYER_MEDIA_SET_VIDEO_SIZE:
        {
            int w, h;
            w   = ((int*)param)[0];   //* read positon to total file size.
            h  = ((int*)param)[1];   //* cache buffer fullness.
            TLOGD("video width = %d,height = %d",w,h);
            printf("*****tplayer:video width = %d,height = %d\n",w,h);
            appMsg = TPLAYER_NOTIFY_MEDIA_VIDEO_SIZE;
            break;
        }

        case AWPLAYER_MEDIA_DECODED_VIDEO_SIZE:
        {
            int w, h;
            w   = ((int*)param)[0];   //real decoded video width
            h  = ((int*)param)[1];   //real decoded video height
            TLOGD("video decoded width = %d,height = %d",w,h);
            printf("*****tplayer:video decoded width = %d,height = %d\n",w,h);
            appMsg = TPLAYER_NOTYFY_DECODED_VIDEO_SIZE;
            break;
        }
#endif

        default:
        {
            //printf("warning: unknown callback from xplayer.\n");
            break;
        }
    }
    if(appMsg != -1){
        tplayer->mNotifier(tplayer->mUserData,appMsg,ext1,param);
    }
    return 0;
}

#ifndef ONLY_ENABLE_AUDIO
static int SubCallbackProcess(void* pUser, int eMessageId, void* param)
{
    TPlayer* tplayer = (TPlayer*)pUser;
    TP_CHECK(tplayer);
    TP_CHECK(param);
    //TLOGD("SubCallbackProcess   ");
    int msg = -1;
    switch(eMessageId)
    {
        case SUBCTRL_SUBTITLE_AVAILABLE:
            msg = TPLAYER_NOTIFY_SUBTITLE_FRAME;
            break;
        case SUBCTRL_SUBTITLE_EXPIRED:

            break;
        default:
            TLOGE("the msg is not support");
            break;
    }

    if(tplayer->mNotifier){
        if(msg != -1)
            tplayer->mNotifier(tplayer->mUserData,msg,0,param);
    }
    return 0;

}

//this function is callback by layer
static int LayerCallbackVideoframe(void* pUser,void* param){
    TPlayer* tplayer = (TPlayer*)pUser;
    VideoPicture *pic = (VideoPicture*)param;
    TP_CHECK(tplayer);
    TP_CHECK(param);
    //TLOGD("LayerCallbackVideoframe");
    VideoPicData data;
    memset(&data, 0x00, sizeof(VideoPicData));
    data.nPts = pic->nPts;
    data.nWidth = pic->nWidth;
    data.nHeight = pic->nHeight;
    data.ePixelFormat = pic->ePixelFormat;
    data.nBottomOffset = pic->nBottomOffset;
    data.nLeftOffset = pic->nLeftOffset;
    data.nRightOffset = pic->nRightOffset;
    data.nTopOffset = pic->nTopOffset;
    data.pData0 = pic->pData0;
    data.pData1 = pic->pData1;
    data.pData2 = pic->pData2;
    data.phyCBufAddr = pic->phyCBufAddr;
    data.phyYBufAddr = pic->phyYBufAddr;
    if(tplayer->mNotifier)
        tplayer->mNotifier(tplayer->mUserData,TPLAYER_NOTIFY_VIDEO_FRAME,0,&data);
    return 0;
}
#endif

//this function is callback by soundctrl
static int SoundctrlCallbackAudioframe(void* pUser,void* param){
    TPlayer* tplayer = (TPlayer*)pUser;
    SoundPcmData *pcmData = (SoundPcmData*)param;
    TP_CHECK(tplayer);
    TP_CHECK(param);
    //TLOGD("SoundctrlCallbackAudioframe");
    AudioPcmData data;
    memset(&data, 0x00, sizeof(AudioPcmData));
    data.pData = pcmData->pData;
    data.nSize = pcmData->nSize;
    data.samplerate = pcmData->samplerate;
    data.channels = pcmData->channels;
    data.accuracy = pcmData->accuracy;
    if(tplayer->mNotifier)
        tplayer->mNotifier(tplayer->mUserData,TPLAYER_NOTIFY_AUDIO_FRAME,0,&data);
    return 0;
}

TPlayer* TPlayerCreate(TplayerType type){
    TPlayer* mPrivateData = (TPlayer*)malloc(sizeof(TPlayer));
    if(mPrivateData == NULL){
        TLOGE("malloc TPlayer fail");
        return NULL;
    }
    LogTinaVersionInfo();
    TLOGD("TPlayerCreate");
    memset(mPrivateData,0x00,sizeof(TPlayer));
    mPrivateData->mPlayerType = type;//default is CEDARX_PLAYER
    mPrivateData->mUserData = NULL;
    mPrivateData->mNotifier = NULL;
    mPrivateData->mVolume = 20;//default is 20,which is the origin pcm
    mPrivateData->mMuteFlag = 0;
    if(type == CEDARX_PLAYER){
#ifndef ONLY_ENABLE_AUDIO
        mPrivateData->mHideSubFlag = 0;
#endif
        mPrivateData->mDebugFlag = 0;
        mPrivateData->mXPlayer = XPlayerCreate();
        if(mPrivateData->mXPlayer == NULL){
            TLOGE("XPlayerCreate fail");
            free(mPrivateData);
            mPrivateData = NULL;
            return NULL;
        }
        int checkRet = XPlayerInitCheck(mPrivateData->mXPlayer);
        if(checkRet == -1){
            TLOGE("the player init check fail");
            XPlayerDestroy(mPrivateData->mXPlayer);
            mPrivateData->mXPlayer = NULL;
            free(mPrivateData);
            mPrivateData = NULL;
            return NULL;
        }
        mPrivateData->mMediaInfo = NULL;
        mPrivateData->mSoundCtrl = TSoundDeviceCreate(SoundctrlCallbackAudioframe,mPrivateData);
        if(mPrivateData->mSoundCtrl == NULL){
            //if the mSoundCtrl is null,do not set it to xplayer
            TLOGE("sound device create fail,mPrivateData->mSoundCtrl is null");
            XPlayerDestroy(mPrivateData->mXPlayer);
            mPrivateData->mXPlayer = NULL;
            free(mPrivateData);
            mPrivateData = NULL;
            return NULL;
        }else{
            XPlayerSetAudioSink(mPrivateData->mXPlayer,mPrivateData->mSoundCtrl);
        }
#ifndef ONLY_ENABLE_AUDIO
        mPrivateData->mLayerCtrl = LayerCreate(LayerCallbackVideoframe,mPrivateData);
        TP_LOG_CHECK(mPrivateData->mXPlayer,"TPlayer create layer failed!\n");

        mPrivateData->mSubCtrl = SubtitleCreate(SubCallbackProcess, mPrivateData);
        TP_LOG_CHECK(mPrivateData->mXPlayer,"TPlayer create subtitle ctrl failed!\n");

        mPrivateData->mDeinterlace = DeinterlaceCreate();
        TP_LOG_CHECK(mPrivateData->mXPlayer,"TPlayer create deinterlace ctrl failed!\n");

        XPlayerSetVideoSurfaceTexture(mPrivateData->mXPlayer, (void*)mPrivateData->mLayerCtrl);
        XPlayerSetSubCtrl(mPrivateData->mXPlayer, (void*)mPrivateData->mSubCtrl);
        XPlayerSetDeinterlace(mPrivateData->mXPlayer, (void*)mPrivateData->mDeinterlace);
#endif
    }else if(type == AUDIO_PLAYER){
        mPrivateData->mDebugFlag = 0;
        mPrivateData->mXPlayer = XPlayerCreate();
        if(mPrivateData->mXPlayer == NULL){
            TLOGE("XPlayerCreate fail");
            free(mPrivateData);
            mPrivateData = NULL;
            return NULL;
        }
        int checkRet = XPlayerInitCheck(mPrivateData->mXPlayer);
        if(checkRet == -1){
            TLOGE("the player init check fail");
            XPlayerDestroy(mPrivateData->mXPlayer);
            mPrivateData->mXPlayer = NULL;
            free(mPrivateData);
            mPrivateData = NULL;
            return NULL;
        }
        mPrivateData->mMediaInfo = NULL;
        mPrivateData->mSoundCtrl = TSoundDeviceCreate(SoundctrlCallbackAudioframe,mPrivateData);
        if(mPrivateData->mSoundCtrl == NULL){
            //if the mSoundCtrl is null,do not set it to xplayer
            TLOGE("sound device create fail,mPrivateData->mSoundCtrl is null");
            XPlayerDestroy(mPrivateData->mXPlayer);
            mPrivateData->mXPlayer = NULL;
            free(mPrivateData);
            mPrivateData = NULL;
            return NULL;
        }else{
            XPlayerSetAudioSink(mPrivateData->mXPlayer,mPrivateData->mSoundCtrl);
        }
    }else{
        TLOGE("the player type:%d is not support",type);
        free(mPrivateData);
        mPrivateData = NULL;
        return NULL;
    }
    return mPrivateData;
}

void TPlayerDestroy(TPlayer* p){
    TP_CHECK(p);
    if(p->mXPlayer != NULL){
	XPlayerDestroy(p->mXPlayer);
        p->mXPlayer = NULL;
    }
    free(p);
}

int TPlayerSetDebugFlag(TPlayer* p, bool debugFlag){
    TP_CHECK(p);
    p->mDebugFlag = debugFlag;
    return 0;
}

int TPlayerSetNotifyCallback(TPlayer* p,
                                    TPlayerNotifyCallback notifier,
                                    void* pUserData){
    int ret = -1;
    TP_CHECK(p);
    p->mNotifier = notifier;
    p->mUserData = pUserData;
    ret = XPlayerSetNotifyCallback(p->mXPlayer, CallbackFromXPlayer, (void*)p);
    return ret;
}

int TPlayerSetDataSource(TPlayer* p, const char* pUrl, const CdxKeyedVectorT* pHeaders){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    void* httpService = NULL;
    return XPlayerSetDataSourceUrl(p->mXPlayer,pUrl,httpService,pHeaders);
}

/* int TPlayerSetDataSourceFd(TPlayer* p, int fd, int64_t nOffset, int64_t nLength);*/

int TPlayerPrepare(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerPrepare(p->mXPlayer);
}

int TPlayerPrepareAsync(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerPrepareAsync(p->mXPlayer);
}

int TPlayerStart(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerStart(p->mXPlayer);
}

int TPlayerPause(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerPause(p->mXPlayer);
}

int TPlayerStop(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerStop(p->mXPlayer);
}

int TPlayerReset(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerReset(p->mXPlayer);
}

int TPlayerSeekTo(TPlayer* p, int nSeekTimeMs){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerSeekTo(p->mXPlayer,nSeekTimeMs,AW_SEEK_PREVIOUS_SYNC);
}

bool TPlayerIsPlaying(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return (bool)XPlayerIsPlaying(p->mXPlayer);
}

int TPlayerGetCurrentPosition(TPlayer* p, int* msec){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerGetCurrentPosition(p->mXPlayer,msec);
}

int TPlayerGetDuration(TPlayer* p, int* msec){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerGetDuration(p->mXPlayer,msec);
}

MediaInfo*  TPlayerGetMediaInfo(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
     MediaInfo* mi = XPlayerGetMediaInfo(p->mXPlayer);
     p->mMediaInfo = mi;
     return mi;
}

int TPlayerSetLooping(TPlayer* p, bool bLoop){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    if(bLoop)
        return XPlayerSetLooping(p->mXPlayer,1);
    else
        return XPlayerSetLooping(p->mXPlayer,0);
}

//this interface should be called before prepare
int TPlayerSetScaleDownRatio(TPlayer* p, TplayerVideoScaleDownType nHorizonScaleDown, TplayerVideoScaleDownType nVerticalScaleDown){
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetScaleDownRatio() function can not use");
    return -1;
#else
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    int horizonScaleDown = 0;
    int verticalScaleDown = 0;
    switch (nHorizonScaleDown)
    {
        case TPLAYER_VIDEO_SCALE_DOWN_1:
        {
            horizonScaleDown = 0;
            break;
        }
        case TPLAYER_VIDEO_SCALE_DOWN_2:
        {
            horizonScaleDown = 1;
            break;
        }
        case TPLAYER_VIDEO_SCALE_DOWN_4:
        {
            horizonScaleDown = 2;
            break;
        }
        case TPLAYER_VIDEO_SCALE_DOWN_8:
        {
            horizonScaleDown = 3;
            break;
        }
        default:
        {
            TLOGE("the nHorizonScaleDown value is wrong,nHorizonScaleDown = %d",nHorizonScaleDown);
            break;
        }
    }
    switch (nVerticalScaleDown)
    {
        case TPLAYER_VIDEO_SCALE_DOWN_1:
        {
            verticalScaleDown = 0;
            break;
        }
        case TPLAYER_VIDEO_SCALE_DOWN_2:
        {
            verticalScaleDown = 1;
            break;
        }
        case TPLAYER_VIDEO_SCALE_DOWN_4:
        {
            verticalScaleDown = 2;
            break;
        }
        case TPLAYER_VIDEO_SCALE_DOWN_8:
        {
            verticalScaleDown = 3;
            break;
        }
        default:
        {
            TLOGE("the nVerticalScaleDown value is wrong,nVerticalScaleDown = %d",nVerticalScaleDown);
            break;
        }
    }
    return XPlayerSetScaleDownRatio(p->mXPlayer,horizonScaleDown,verticalScaleDown);
#endif
}

//the xplayer do not supply this interface,and this interface should be called before prepare status
int TPlayerSetRotate(TPlayer* p, TplayerVideoRotateType rotate){
#ifdef ONLY_ENABLE_AUDIO
      logw("only enable audio,TPlayerSetRotate() function can not use");
      return -1;
#else
    return XPlayerSetRotateDegree(p->mXPlayer,(int)rotate);
#endif
}

//this interface may have bug
int TPlayerSetSpeed(TPlayer* p, TplayerPlaySpeedType nSpeed){
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetSpeed() function can not use");
    return -1;
#else
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    int speed = 1;
    switch (nSpeed)
    {
        case PLAY_SPEED_FAST_FORWARD_16:
        {
            speed = 16;
            break;
        }
        case PLAY_SPEED_FAST_FORWARD_8:
        {
            speed = 8;
            break;
        }
        case PLAY_SPEED_FAST_FORWARD_4:
        {
            speed = 4;
            break;
        }
        case PLAY_SPEED_FAST_FORWARD_2:
        {
            speed = 2;
            break;
        }
        case PLAY_SPEED_1:
        {
            speed = 1;
            break;
        }
        case PLAY_SPEED_FAST_BACKWARD_2:
        {
            speed = -2;
            break;
        }
        case PLAY_SPEED_FAST_BACKWARD_4:
        {
            speed = -4;
            break;
        }
        case PLAY_SPEED_FAST_BACKWARD_8:
        {
            speed = -8;
            break;
        }
        case PLAY_SPEED_FAST_BACKWARD_16:
        {
            speed = -16;
            break;
        }
        default:
        {
            TLOGE("the nSpeed value is wrong,nSpeed = %d",nSpeed);
            break;
        }
    }
    return XPlayerSetSpeed(p->mXPlayer,speed);
#endif
}

/* the following  reference to mediaplayer */
//int TPlayerGetVideoWidth(TPlayer* p, int* width); /* media info has include, no need*/
//int TPlayerGetVideoHeight(TPlayer* p, int* height); /*media info has include, no need*/
//int TPlayerGetSubtitleStreamNum(TPlayer* p, int* nStreamNum);/* media info has include, no need*/
//int TPlayerGetAudioStreamNum(TPlayer* p, int* nStreamNum);/* media info has include, no need*/

/**
 *  Set volume with specified value.
 *  @param[in] volume Specified volume value to be set.
 *  @return TRUE if success. Otherwise, return FALSE.
 */
int TPlayerSetVolume(TPlayer* p, int volume){
    TP_CHECK(p);

    if(p->mSoundCtrl){
        if(volume > 40){
            TLOGE("the volume(%d) is larger than the largest volume(40),set it to 40",volume);
            volume = 40;
        }else if(volume < 0){
            TLOGE("the volume(%d) is smaller than 0,set it to 0",volume);
            volume =0;
        }
        p->mVolume = volume;
        volume -= 20;
        return TSoundDeviceSetVolume(p->mSoundCtrl,volume);
    }else{
        TLOGE("p->mSoundCtrl is NULL,can not set the volume");
        return -1;
    }
}

int TPlayerGetVolume(TPlayer* p){
    TP_CHECK(p);
    return p->mVolume;
}

int TPlayerSetAudioMute(TPlayer* p ,bool mute){
    TP_CHECK(p);
    if(p->mSoundCtrl){
        if(mute){
            int muteVolume = -20;
            return TSoundDeviceSetVolume(p->mSoundCtrl,muteVolume);
        }else{
            return TSoundDeviceSetVolume(p->mSoundCtrl,p->mVolume-20);
        }
    }else{
        TLOGE("p->mSoundCtrl is NULL,can not set the volume mute");
        return -1;
    }
}

int TPlayerSetExternalSubUrl(TPlayer* p, const char* filePath){
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetExternalSubUrl() function can not use");
    return -1;
#else
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    TP_CHECK(filePath);
    return XPlayerSetExternalSubUrl(p->mXPlayer,filePath);
#endif
}

int TPlayerSetExternalSubFd(TPlayer* p, int fd, int64_t offset, int64_t len, int fdSub){
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetExternalSubFd() function can not use");
    return -1;
#else
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerSetExternalSubFd(p->mXPlayer,fd,offset,len,fdSub);
#endif
}

int TPlayerGetSubDelay(TPlayer* p){
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerGetSubDelay() function can not use");
    return -1;
#else
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerGetSubDelay(p->mXPlayer);
#endif
}

int TPlayerSwitchAudio(TPlayer* p, int nStreamIndex){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    if(p->mMediaInfo != NULL){
        if(nStreamIndex >= 0 && nStreamIndex<p->mMediaInfo->nAudioStreamNum){
            return XPlayerSwitchAudio(p->mXPlayer,nStreamIndex);
        }else{
            TLOGE("the nStreamIndex is wrong,can not switch audio.  nStreamIndex = %d",nStreamIndex);
        }
    }
    return -1;
}

int TPlayerSetSubDelay(TPlayer* p, int nTimeMs)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetSubDelay() function can not use");
    return -1;
#else
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
	return XPlayerSetSubDelay(p->mXPlayer, nTimeMs);
#endif
}

int TPlayerGetSubCharset(TPlayer* p, char *charset)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerGetSubCharset() function can not use");
    return -1;
#else
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
	if(NULL == charset) {
		TLOGE("charset invalid!\n");
		return -1;
	}
	return XPlayerGetSubCharset(p->mXPlayer, charset);
#endif
}

int TPlayerSetSubCharset(TPlayer* p, const char* strFormat)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetSubCharset() function can not use");
    return -1;
#else
    TP_CHECK(p);
	TP_CHECK(p->mXPlayer);

	if(NULL == strFormat) {
		TLOGE("strFormat invalid!\n");
		return -1;
	}
	return TPlayerSetSubCharset(p->mXPlayer, strFormat);
#endif
}

int TPlayerSwitchSubtitle(TPlayer* p, int nStreamIndex)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSwitchSubtitle() function can not use");
    return -1;
#else
    TP_CHECK(p);
	TP_CHECK(p->mXPlayer);

	if(nStreamIndex < 0)
		return -1;
	return XPlayerSwitchSubtitle(p->mXPlayer, nStreamIndex);
#endif
}

void TPlayerSetSubtitleDisplay(TPlayer* p, bool onoff)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetSubtitleDisplay() function can not use");
#else
    TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mSubCtrl);
	SubtitleDisplayOnoff(p->mSubCtrl, onoff);
#endif
}

void TPlayerSetVideoDisplay(TPlayer* p, bool onoff)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetSubtitleDisplay() function can not use");
#else
    TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);
	return LayerDisplayOnoff(p->mLayerCtrl, onoff);
#endif
}

void TPlayerSetDisplayRect(TPlayer* p, int x, int y, unsigned int width, unsigned int height)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetDisplayRect() function can not use");
#else
    TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);
	return LayerSetDisplayRect(p->mLayerCtrl, x, y, width, height);
#endif
}
void TPlayerSetSrcRect(TPlayer* p, int x, int y, unsigned int width, unsigned int height)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetSrcRect() function can not use");
#else
    TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);
	return LayerSetSrcRect(p->mLayerCtrl, x, y, width, height);
#endif
}

void TPlayerSetBrightness(TPlayer* p, unsigned int grade)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetBrightness() function can not use");
#else
    TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);
	LayerSetControl(p->mLayerCtrl, LAYER_CMD_SET_BRIGHTNESS, grade);
#endif
}

void TPlayerSetContrast(TPlayer* p, unsigned int grade)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetBrightness() function can not use");
#else
    TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);
	LayerSetControl(p->mLayerCtrl, LAYER_CMD_SET_CONTRAST, grade);
#endif
}
void TPlayerSetHue(TPlayer* p, unsigned int grade)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetHue() function can not use");
#else
    TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);
	LayerSetControl(p->mLayerCtrl, LAYER_CMD_SET_HUE, grade);
#endif
}

void TPlayerSetSaturation(TPlayer* p, unsigned int grade)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetSaturation() function can not use");
#else
    TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);
	LayerSetControl(p->mLayerCtrl, LAYER_CMD_SET_SATURATION, grade);
#endif
}

void TPlayerSetEnhanceDefault(TPlayer* p)

{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetEnhanceDefault() function can not use");
#else
    TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);
	LayerSetControl(p->mLayerCtrl, LAYER_CMD_SET_VEDIO_ENHANCE_DEFAULT, 0);
#endif
}

int TPlayerGetVideoDispFramerate(TPlayer* p,float* dispFramerate)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerGetVideoDispFramerate() function can not use");
    return -1;
#else
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerGetVideoDispFramerate(p->mXPlayer, dispFramerate);
#endif
}

int TPlayerSetHoldLastPicture(TPlayer* p,int bHoldFlag)
{
#ifdef ONLY_ENABLE_AUDIO
    logw("only enable audio,TPlayerSetHoldLastPicture() function can not use");
    return -1;
#else
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerSetHoldLastPicture(p->mXPlayer, bHoldFlag);
#endif
}
