#ifndef TPLAYER_H
#define TAPLAYER_H

#include <stdbool.h>
#include <xplayer.h>

#ifdef __cplusplus
extern "C" {
#endif

//#ifndef __cplusplus
//    typedef int bool;
//#endif

/**
  *The response of state change notices APP what current state of player.
  */
typedef enum TplayerNotifyAppType
{
    TPLAYER_NOTIFY_PREPARED			          = 0,
    TPLAYER_NOTIFY_PLAYBACK_COMPLETE       = 1,
    TPLAYER_NOTIFY_SEEK_COMPLETE		 = 2,
    TPLAYER_NOTIFY_MEDIA_ERROR			 = 3,
    TPLAYER_NOTIFY_NOT_SEEKABLE			 = 4,
    TPLAYER_NOTIFY_BUFFER_START			 = 5, /*this means no enough data to play*/
    TPLAYER_NOTIFY_BUFFER_END			 = 6, /*this means got enough data to play*/
    TPLAYER_NOTIFY_DOWNLOAD_START		 = 7,//not support now
    TPLAYER_NOTIFY_DOWNLOAD_END	         = 8,//not support now
    TPLAYER_NOTIFY_DOWNLOAD_ERROR           = 9,//not support now
    TPLAYER_NOTIFY_MEDIA_VIDEO_SIZE		 = 10, /*notified while video size changed*/
    TPLAYER_NOTIFY_VIDEO_FRAME                    = 11,//notify the decoded video frame
    TPLAYER_NOTIFY_AUDIO_FRAME                    = 12,//notify the decoded audio frame
    TPLAYER_NOTIFY_SUBTITLE_FRAME               = 13,//notify the decoded subtitle frame
    TPLAYER_NOTYFY_DECODED_VIDEO_SIZE      =14,//notify the decoded video size
}TplayerNotifyAppType;

typedef enum TplayerMediaErrorType
{
    TPLAYER_MEDIA_ERROR_UNKNOWN			= 1,
    TPLAYER_MEDIA_ERROR_OUT_OF_MEMORY	= 2,//not support now
    TPLAYER_MEDIA_ERROR_IO				= 3,
    TPLAYER_MEDIA_ERROR_UNSUPPORTED	= 4,
    TPLAYER_MEDIA_ERROR_TIMED_OUT		= 5,//not support now
}TplayerMediaErrorType;

typedef enum TplayerType
{
    CEDARX_PLAYER			= 0,
    AUDIO_PLAYER		= 1,
}TplayerType;

typedef enum TplayerVideoScaleDownType
{
    TPLAYER_VIDEO_SCALE_DOWN_1      = 0, /*no scale down*/
    TPLAYER_VIDEO_SCALE_DOWN_2      = 1, /*scale down 1/2*/
    TPLAYER_VIDEO_SCALE_DOWN_4      = 2, /*scale down 1/4*/
    TPLAYER_VIDEO_SCALE_DOWN_8      = 3, /*scale down 1/8*/
}TplayerVideoScaleDownType;

typedef enum TplayerVideoRotateType
{
    TPLAYER_VIDEO_ROTATE_DEGREE_0       = 0, /*do not rotate*/
    TPLAYER_VIDEO_ROTATE_DEGREE_90       = 90, /*rotate 90 degree clockwise*/
    TPLAYER_VIDEO_ROTATE_DEGREE_180      = 180, /*rotate 180 degree clockwise*/
    TPLAYER_VIDEO_ROTATE_DEGREE_270      = 270, /*rotate 270 degree clockwise*/
}TplayerVideoRotateType;

typedef enum TplayerPlaySpeedType
{
    PLAY_SPEED_FAST_FORWARD_16  =  0,  /*fast forward 16 times*/
    PLAY_SPEED_FAST_FORWARD_8   =  1,   /*fast forward 8 times*/
    PLAY_SPEED_FAST_FORWARD_4   =  2,   /*fast forward 4 times*/
    PLAY_SPEED_FAST_FORWARD_2   =  3,   /*fast forward 2 times*/
    PLAY_SPEED_1		                        =  4,   /*normal play*/
    PLAY_SPEED_FAST_BACKWARD_2   =  5, /*fast backward 2 times*/
    PLAY_SPEED_FAST_BACKWARD_4   =  6, /*fast backward  4 times*/
    PLAY_SPEED_FAST_BACKWARD_8   =  7, /*fast backward  8 times*/
    PLAY_SPEED_FAST_BACKWARD_16  =  8, /*fast backward  16 times*/
}TplayerPlaySpeedType;

typedef int (*TPlayerNotifyCallback)(void* pUser,
                        int msg, int ext1, void* para);

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
    unsigned char* pData;
    unsigned int   nSize;
    unsigned int samplerate;
    unsigned int channels;
    int accuracy;
} AudioPcmData;

typedef struct TPlayerContext
{
    XPlayer*					mXPlayer;
    TplayerType                 mPlayerType;
    void*						mUserData;
    TPlayerNotifyCallback		mNotifier;
    int				mVolume;
    MediaInfo*			mMediaInfo;
    bool						mMuteFlag;
#ifndef ONLY_ENABLE_AUDIO
    bool						mHideSubFlag;
#endif
    SoundCtrl*			mSoundCtrl;
#ifndef ONLY_ENABLE_AUDIO
    LayerCtrl*					mLayerCtrl;
    Deinterlace*				mDeinterlace;
    SubCtrl*					mSubCtrl;
#endif
    bool				mDebugFlag;
}TPlayer;

TPlayer* TPlayerCreate(TplayerType type);

void TPlayerDestroy(TPlayer* p);

int TPlayerSetDebugFlag(TPlayer* p, bool debugFlag);

int TPlayerSetNotifyCallback(TPlayer* p,
                                    TPlayerNotifyCallback notifier,
                                    void* pUserData);

int TPlayerSetDataSource(TPlayer* p, const char* pUrl, const CdxKeyedVectorT* pHeaders);

/* int TPlayerSetDataSourceFd(TPlayer* p, int fd, int64_t nOffset, int64_t nLength);*/

int TPlayerPrepare(TPlayer* p);

int TPlayerPrepareAsync(TPlayer* p);

int TPlayerStart(TPlayer* p);

int TPlayerPause(TPlayer* p);

int TPlayerStop(TPlayer* p);

int TPlayerReset(TPlayer* p);

int TPlayerSeekTo(TPlayer* p, int nSeekTimeMs);

bool TPlayerIsPlaying(TPlayer* p);

int TPlayerGetCurrentPosition(TPlayer* p, int* msec);

int TPlayerGetDuration(TPlayer* p, int* msec);

MediaInfo* TPlayerGetMediaInfo(TPlayer* p);

int TPlayerSetLooping(TPlayer* p, bool bLoop); /*needed ?*/

int TPlayerSetScaleDownRatio(TPlayer* p, TplayerVideoScaleDownType nHorizonScaleDown, TplayerVideoScaleDownType nVerticalScaleDown);

int TPlayerSetRotate(TPlayer* p, TplayerVideoRotateType rotate);

int TPlayerSetSpeed(TPlayer* p, TplayerPlaySpeedType nSpeed);

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
int TPlayerSetVolume(TPlayer* p, int volume);
int TPlayerGetVolume(TPlayer* p);
int TPlayerSetAudioMute(TPlayer* p ,bool mute);

int TPlayerSetExternalSubUrl(TPlayer* p, const char* filePath);
int TPlayerSetExternalSubFd(TPlayer* p, int fd, int64_t offset, int64_t len, int fdSub);
int TPlayerGetSubDelay(TPlayer* p);
int TPlayerSetSubDelay(TPlayer* p, int nTimeMs);
int TPlayerGetSubCharset(TPlayer* p, char *charset);
int TPlayerSetSubCharset(TPlayer* p, const char* strFormat);

int TPlayerSwitchAudio(TPlayer* p, int nStreamIndex);

/**
 *  Change subtitle stream with given track index.
 *  @param[in] trackIdx Track index. Player can get information of track index from #TPlayerGetMediaInfo.
 *  If track index is invalid, server just ignores the command.
 *  If the given track index is -1, server switch to next subtitle stream circularly.
 */

int TPlayerSwitchSubtitle(TPlayer* p,int nStreamIndex);

/**
 *  Set subtitle display on/off.
 *  @param[in] onoff Display or hide.
 */
void TPlayerSetSubtitleDisplay(TPlayer* p,bool onoff);

/**
 *  Set video display on/off.
 *  @param[in] onoff Display or hide.
 */
void TPlayerSetVideoDisplay(TPlayer* p,bool onoff);

/**
 *  To change display position and dimension.
 *  @param[in] x X position to display.
 *  @param[in] y Y position to display.
 *  @param[in] width Width to be scale.
 *  @param[in] height Height to be scale.
 */
void TPlayerSetDisplayRect(TPlayer* p,int x, int y, unsigned int width, unsigned int height);

/**
 *  To set source crop position and dimension.
 *  @param[in] x X position to display.
 *  @param[in] y Y position to display.
 *  @param[in] width Width to be scale.
 *  @param[in] height Height to be scale.
 */
void TPlayerSetSrcRect(TPlayer* p,int x, int y, unsigned int width, unsigned int height);
/**
 *  Set brightness of display.
 *  @param[in] grade Range: 0 - 100 Default: 50
 */
void TPlayerSetBrightness(TPlayer* p,unsigned int grade);

/**
 *  Set contrast of display.
 *  @param[in] grade Range: 0 - 100 Default: 50
 */
void TPlayerSetContrast(TPlayer* p,unsigned int grade);

/**
 *  Set hue of display.
 *  @param[in] grade Range: 0 - 100 Default: 50
 */
void TPlayerSetHue(TPlayer* p,unsigned int grade);

/**
 *  Set saturation of display.
 *  @param[in] grade Range: 0 - 100 Default: 50
 */
void TPlayerSetSaturation(TPlayer* p,unsigned int grade);

/**
 *  Set sharpness of display.
 *  @param[in] grade Range: 0 - 10 Default: 5
 */

/**
 *  Set brightness, contrast, hue, saturation and sharpness to default value.
 */
void TPlayerSetEnhanceDefault(TPlayer* p);

/*
    get the real display framerate
*/
int TPlayerGetVideoDispFramerate(TPlayer* p,float* dispFramerate);

int TPlayerSetHoldLastPicture(TPlayer* p,int bHoldFlag);


#ifdef __cplusplus
}
#endif

#endif
