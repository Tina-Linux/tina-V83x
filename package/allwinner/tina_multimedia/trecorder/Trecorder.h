#ifndef _TRECORDER_H_
#define _TRECORDER_H_

#include "dataqueue.h"

typedef enum
{
    T_CAMERA      = 0x01,
    T_AUDIO       = 0x02,
    T_DECODER     = 0x04,
    T_ENCODER     = 0x08,
    T_SCREEN      = 0x10,
    T_MUXER       = 0x20,
    T_CUSTOM      = 0x40,
}TmoduleName;

typedef struct
{
	int x;
	int y;
	int width;
	int height;
}TdispRect;

typedef enum
{
	T_CAMERA_FRONT,
	T_CAMERA_BACK,
	T_CAMERA_DISABLE,
	T_CAMERA_END,
}TcameraIndex;

typedef enum
{
	T_AUDIO_MIC0,
	T_AUDIO_MIC1,
	T_AUDIO_MIC_DISABLE,
	T_AUDIO_MIC_END,
}TmicIndex;

typedef enum
{
	T_DISP_LAYER0,
	T_DISP_LAYER1,
	T_DISP_LAYER_DISABLE,
	T_DISP_LAYER_END,
}TdispIndex;

typedef enum
{
	T_ROTATION_ANGLE_0 = 0,
	T_ROTATION_ANGLE_90 = 1,
	T_ROTATION_ANGLE_180 = 2,
	T_ROTATION_ANGLE_270 = 3
}TrotateDegree;


typedef enum
{
	T_OUTPUT_TS,
	T_OUTPUT_MOV,
	T_OUTPUT_JPG,
	T_OUTPUT_AAC,
	T_OUTPUT_MP3,
	T_OUTPUT_END,
}ToutputFormat;

typedef enum
{
	T_VIDEO_H264,
	T_VIDEO_MJPEG,
	T_VIDEO_END,
}TvideoEncodeFormat;

typedef enum
{
	T_AUDIO_PCM,
	T_AUDIO_AAC,
	T_AUDIO_MP3,
	T_AUDIO_LPCM,
	T_AUDIO_END,
}TaudioEncodeFormat;

typedef enum
{
	T_CAPTURE_JPG,
	T_CAPTURE_END,
}TcaptureFormat;

typedef enum
{
    T_CAMERA_YUV420SP,//NV12
    T_CAMERA_YVU420SP,//NV21
    T_CAMERA_YUV420P,//YU12
    T_CAMERA_YVU420P,//YV12
    T_CAMERA_YUV422SP,
    T_CAMERA_YVU422SP,
    T_CAMERA_YUV422P,
    T_CAMERA_YVU422P,
    T_CAMERA_YUYV422,
    T_CAMERA_UYVY422,
    T_CAMERA_YVYU422,
    T_CAMERA_VYUY422,
    T_CAMERA_ARGB,
    T_CAMERA_RGBA,
    T_CAMERA_ABGR,
    T_CAMERA_BGRA,
    T_CAMERA_MJPEG,
	T_CAMERA_H264,
    T_CAMERA_TILE_32X32,//MB32_420
    T_CAMERA_TILE_128X32,
}TcameraFormat;

typedef enum
{
	T_MIC_PCM,
	T_MIC_END,
}TmicFormat;

typedef enum
{
	T_PREVIEW,
	T_RECORD,
	T_ALL,
}TFlags;

typedef enum
{
	T_ROUTE_ISP,
	T_ROUTE_VE,
	T_ROUTE_CAMERA,
}TRoute;

typedef enum TrecorderCallbackMsg
{
    T_RECORD_ONE_FILE_COMPLETE			          = 0,
    T_RECORD_ONE_AAC_FILE_COMPLETE,
}TrecorderCallbackMsg;

typedef enum
{
	T_PREVIEW_ZORDER_TOP,
	T_PREVIEW_ZORDER_MIDDLE,
	T_PREVIEW_ZORDER_BOTTOM,
}TZorder;

typedef struct
{
    char capturePath[128];
    TcaptureFormat captureFormat ;
    int captureWidth;
    int captureHeight;
}TCaptureConfig;


typedef int (*TRecorderCallback)(void* pUserData,int msgType,void* param);

typedef void * TrecorderHandle;

TrecorderHandle *CreateTRecorder();
int TRsetCamera(TrecorderHandle *hdl,int index);
int TRsetAudioSrc(TrecorderHandle *hdl,int index);
int TRsetPreview(TrecorderHandle *hdl,int layer);
int TRsetOutput(TrecorderHandle *hdl,char *url);

int TRstart(TrecorderHandle *hdl,int flags);
int TRstop(TrecorderHandle *hdl,int flags);
int TRrelease(TrecorderHandle *hdl);
int TRprepare(TrecorderHandle *hdl);
int TRreset(TrecorderHandle *hdl);
int TRdeinit(TrecorderHandle *hdl);
int TRresume(TrecorderHandle *hdl);

int TRsetVideoEncodeSize(TrecorderHandle *hdl,int width,int height);
int TRsetOutputFormat(TrecorderHandle *hdl,int format);
int TRsetRecorderCallback(TrecorderHandle *hdl,TRecorderCallback callback,void* pUserData);
int TRsetMaxRecordTimeMs(TrecorderHandle *hdl,int ms);
int TRsetVideoEncoderFormat(TrecorderHandle *hdl,int VFormat);
int TRsetAudioEncoderFormat(TrecorderHandle *hdl,int AFormat);
int TRsetEncoderBitRate(TrecorderHandle *hdl,int bitrate);//only for video
int TRsetRecorderEnable(TrecorderHandle *hdl,int enable);
int TRsetEncodeFramerate(TrecorderHandle *hdl,int framerate);//only for video
int TRsetVEScaleDownRatio(TrecorderHandle *hdl,int ratio);

int TRgetRecordTime(TrecorderHandle *hdl);
int TRCaptureCurrent(TrecorderHandle *hdl,TCaptureConfig *config);

int TRsetCameraInputFormat(TrecorderHandle *hdl,int format);
int TRsetCameraFramerate(TrecorderHandle *hdl,int framerate);
int TRsetCameraCaptureSize(TrecorderHandle *hdl,int width,int height);
int TRsetCameraEnableWM(TrecorderHandle *hdl,int width,int height,int enable);
int TRsetCameraDiscardRatio(TrecorderHandle *hdl,int ratio);
int TRsetCameraWaterMarkIndex(TrecorderHandle *hdl,int id);
int TRsetCameraEnable(TrecorderHandle *hdl,int enable);

int TRsetMICInputFormat(TrecorderHandle *hdl,int format);
int TRsetMICSampleRate(TrecorderHandle *hdl,int sampleRate);
int TRsetMICChannels(TrecorderHandle *hdl,int channels);
int TRsetMICSampleBits(TrecorderHandle *hdl,int samplebits);
int TRsetMICEnable(TrecorderHandle *hdl,int enable);

int TRsetPreviewSrcRect(TrecorderHandle *hdl,TdispRect *SrcRect);
int TRsetPreviewRect(TrecorderHandle *hdl,TdispRect *rect);
int TRsetPreviewRotate(TrecorderHandle *hdl,int angle);
int TRsetPreviewEnable(TrecorderHandle *hdl,int enable);
int TRsetPreviewRoute(TrecorderHandle *hdl,int route);
int TRsetPreviewZorder(TrecorderHandle *hdl,int zorder);

int TRchangeOutputPath(TrecorderHandle *hdl,char* path);
int TRsetAudioMute(TrecorderHandle *hdl,int muteFlag);

int TRmoduleLink(TrecorderHandle *hdl, TmoduleName *moduleName_1, TmoduleName *moduleName_2, ...);
int TRmoduleUnlink(TrecorderHandle *hdl, TmoduleName *moduleName_1, TmoduleName *moduleName_2, ...);

#endif
