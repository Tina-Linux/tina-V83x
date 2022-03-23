#ifndef _TINA_RECORDER_H_
#define _TINA_RECORDER_H_

#include <pthread.h>
#include <semaphore.h>
#include <videoOutPort.h>
#include "CdxMuxer.h"
#include "audioInPort.h"
#include "Video_input_port.h"
#include "videoDecorder.h"
#include "dataqueue.h"


#define ENCODER_SAVE_NUM 3

#define DIRTY_BACKGROUND_BYTES      (4*1024*1024)

#define MUXER_MoreVIDEO1080P_QUEUE_MbSIZE (4)
#define MUXER_LessVIDEO1080P_QUEUE_MbSIZE (3)
#define MUXER_AUDIO_QUEUE_KbSIZE (12)

#define MUXER_NEED_KEYFRAME 1
#define MUXER_OK_KEYFRAME 2

#define MAX_PATH_LEN 128
#define MAX_URL_LEN 128

typedef enum
{
	TR_OUTPUT_TS,
	TR_OUTPUT_MOV,
	TR_OUTPUT_JPG,
	TR_OUTPUT_AAC,
	TR_OUTPUT_MP3,
	TR_OUTPUT_END
}outputFormat;

typedef enum
{
	TR_VIDEO_H264,
	TR_VIDEO_MJPEG,
	TR_VIDEO_END
}videoEncodeFormat;

typedef enum
{
	TR_CAPTURE_JPG,
	TR_CAPTURE_END
}captureFormat;

typedef struct
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
}dispRect;

typedef struct
{
    char capturePath[MAX_PATH_LEN];
    captureFormat capFormat ;
    int captureWidth;
    int captureHeight;
}captureConfig;


typedef enum
{
	FLAGS_PREVIEW,
	FLAGS_RECORDING,
	FLAGS_ALL
}TinaFlags;

typedef enum
{
	TINA_RECORD_STATUS_INIT,
	TINA_RECORD_STATUS_INITIALIZED,
	TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED,
	TINA_RECORD_STATUS_PREPARED,
	TINA_RECORD_STATUS_RECORDING,
	TINA_RECORD_STATUS_RELEASED
}tinaRecordStatus;

typedef struct
{
	int (*init)(void *hdl,int enable,char *url);
	int (*writeData)(void *hdl,void **data,int size,dataParam *para);
	int (*getUrl)(void *hdl,char *url);
	int (*setUrl)(void *hdl,char *url);
	int (*setEnable)(void *hdl,int enable);
	int (*getEnable)(void *hdl);

	int enable;
	char url[MAX_URL_LEN];
}rOutPort;

typedef enum
{
	RENDER_STATUS_LOOP,
	RENDER_STATUS_PAUSE,
	RENDER_STATUS_RECONFIG,
	RENDER_STATUS_STOP
}renderStatus;

typedef enum TinaRecorderCallbackMsg
{
    TINA_RECORD_ONE_FILE_COMPLETE			          = 0,
    TINA_RECORD_ONE_AAC_FILE_COMPLETE,
}TinaRecorderCallbackMsg;

typedef int (*TinaRecorderCallback)(void* pUserData,int msgType,void* param);

typedef struct
{
	int (*setVideoInPort)(void *hdl,vInPort *vPort);
	int (*setAudioInPort)(void *hdl,aInPort *aPort);
	int (*setDispOutPort)(void *hdl,dispOutPort *dispPort);
	int (*setOutput)(void *hdl,rOutPort *output);

	int (*start)(void *hdl,int flags);
	int (*stop)(void *hdl,int flags);
	int (*release)(void *hdl);
	int (*prepare)(void *hdl);
	int (*reset)(void *hdl);
	int (*deflink)(void *hdl);
    void (*moduleDefInit)(void *hdl);
	int (*resume)(void *hdl,int flags);

	int (*setVideoEncodeSize)(void *hdl,int width,int height);
	int (*setVideoEncodeBitRate)(void *hdl,int bitrate);
	int (*setOutputFormat)(void *hdl,int format);
	int (*setCallback)(void *hdl,TinaRecorderCallback callback,void* pUserData);
	int (*setEncodeVideoFormat)(void *hdl,int format);
	int (*setEncodeAudioFormat)(void *hdl,int format);
	int (*setEncodeFramerate)(void *hdl,int framerate);
	int (*setVEScaleDownRatio)(void *hdl,int ratio);

	int (*getRecorderTime)(void *hdl);
	int (*captureCurrent)(void *hdl,captureConfig *capConfig);
	int (*setMaxRecordTime)(void *hdl,int ms);
	int (*setParameter)(void *hdl,int cmd,int parameter);

	int (*changeOutputPath)(void *hdl,char* path);
	int (*setAudioMute)(void *hdl,int muteFlag);

	int status;
    int startFlags;

	vInPort vport;
	aInPort aport;
    decoder_handle decoderport;
	dispOutPort dispport;
	rOutPort outport;
	void *EncoderContext;

    pthread_t videoId;
    pthread_t audioId;
    pthread_t decoderId;
    pthread_t dirdispId;

    int mAudioUserSetting;
    int mCameraIndex;
	dataParam param;
    int use_wm;
	int wm_width;
	int wm_height;
    int mCaptureFlag;
    captureConfig mCaptureConfig;

    videoZorder zorder;
    int dispSrcWidth;
    int dispSrcHeight;

    int encoderExternalEnable;
    pixelFormat encodeVinputFormat;
	videoEncodeFormat encodeVFormat;
	audioEncodeFormat encodeAFormat;
	int encodeWidth;
	int encodeHeight;
	int encodeBitrate;
	int encodeFramerate;
    int encodeUsePhy;

    pthread_mutex_t mEncoderPacketLock;
    struct modulePacket *encoderpacket[ENCODER_SAVE_NUM];

    int rotateAngel;
    int scaleDownRatio;
    renderBuf rBuf;
    pthread_mutex_t mScaleCallbackLock;

	outputFormat outputFormat;
	TinaRecorderCallback callback;
    void *pUserData;
	int maxRecordTime;
    char mOutputPath[MAX_PATH_LEN];

    int mNeedMuxerVideoFlag;
    long long mBaseVideoPts;
    int mAudioMuteFlag;

    int mTotalAudioPktBufSize;
    int mAudioPktBufSizeCount;
    int mNeedAudioPktBufSpace;
    pthread_mutex_t mAudioPktBufLock;
    sem_t mAudioPktBufSem;

    int mTotalVideoPktBufSize;
    int mVideoPktBufSizeCount;
    int mNeedVideoPktBufSpace;
    pthread_mutex_t mVideoPktBufLock;
    sem_t mVideoPktBufSem;

    /***** dao add *****/
    int moduleLinkFlag;
    int modulePortInitFlag;
    struct moduleAttach cameraPort; /* save camera link */
    struct moduleAttach *videoPort; /* save video link */
    struct moduleAttach audioPort;
    struct moduleAttach screenPort;
    struct moduleAttach muxerPort;
    struct moduleAttach encoderPort;
    struct moduleAttach decoderPort;
}TinaRecorder;

TinaRecorder *CreateTinaRecorder();
int DestroyTinaRecorder(TinaRecorder *hdl);

#endif
