
#ifndef TINAPLAYER_H
#define TINAPLAYER_H

#include <semaphore.h>
#include <pthread.h>
#include <string>
#include <map>
#include <asm/types.h>

using namespace std;

namespace aw{

enum TINA_NOTIFY_APP_TYPE
{
	TINA_NOTIFY_NOT_SEEKABLE        = 1,
	TINA_NOTIFY_ERROR               = 2,
	TINA_NOTIFY_PREPARED            = 3,
	TINA_NOTIFY_BUFFERRING_UPDATE   = 4,
	TINA_NOTIFY_PLAYBACK_COMPLETE   = 5,
	TINA_NOTIFY_RENDERING_START     = 6,
	TINA_NOTIFY_SEEK_COMPLETE       = 7,
	TINA_NOTIFY_BUFFER_START		= 8,
	TINA_NOTIFY_BUFFER_END			= 9,
	TINA_NOTIFY_VIDEO_PACKET        = 10, //the video packet data demux from parser
	TINA_NOTIFY_AUDIO_PACKET        = 11, //the audiopacket data demux from parser
	TINA_NOTIFY_VIDEO_FRAME         = 12, //the video pic after decoding
	TINA_NOTIFY_AUDIO_FRAME         = 13, //the audio pcm data after decoding
};

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

    VIDEO_PIXEL_FORMAT_RGBA				= 10,
    VIDEO_PIXEL_FORMAT_ARGB				= 11,
    VIDEO_PIXEL_FORMAT_ABGR				= 12,
    VIDEO_PIXEL_FORMAT_BGRA				= 13,

    VIDEO_PIXEL_FORMAT_YUYV				= 14,
    VIDEO_PIXEL_FORMAT_YVYU				= 15,
    VIDEO_PIXEL_FORMAT_UYVY				= 16,
    VIDEO_PIXEL_FORMAT_VYUY				= 17,

    VIDEO_PIXEL_FORMAT_PLANARUV_422		= 18,
    VIDEO_PIXEL_FORMAT_PLANARVU_422		= 19,
    VIDEO_PIXEL_FORMAT_PLANARUV_444		= 20,
    VIDEO_PIXEL_FORMAT_PLANARVU_444		= 21,

    VIDEO_PIXEL_FORMAT_MIN = VIDEO_PIXEL_FORMAT_DEFAULT,
    VIDEO_PIXEL_FORMAT_MAX = VIDEO_PIXEL_FORMAT_PLANARVU_444,
};

typedef void (*NotifyCallback)(void* pUserData, int msg, int param0, void* param1);

typedef __s64 int64_t;

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
	int64_t		nPts;
	int		ePixelFormat;
	int		nWidth;
	int		nHeight;
	int		nLineStride;
    int		    nTopOffset;
    int			nLeftOffset;
    int			nBottomOffset;
    int			nRightOffset;
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


class TinaPlayer{
	public:
		int mLoop;
		TinaPlayer();
		~TinaPlayer();
		int initCheck();
		int setNotifyCallback(NotifyCallback notifier, void* pUserData);
		int setDataSource(const char* pUrl, const map<string, string>* pHeaders);
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
		int setVolume(int volume);
		int getVolume();
		void callbackToApp(int msg, int param0, void* param1);
		int setVideoOutputScaleRatio(int horizonScaleDownRatio,int verticalScaleDownRatio);
		int                 mVideoFrameNum;
		int                 mAudioFrameNum;
	private:

		void initSoundControlOpsT();
		void* mPlayer;
		NotifyCallback		mNotifier;
		void*				mUserData;
		int                 mVolume;
	};
}

#endif
