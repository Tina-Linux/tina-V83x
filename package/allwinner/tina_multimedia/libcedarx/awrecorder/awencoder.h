/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : awencoder.h
 * Description : encoder
 * History :
 *
 */

#ifndef __AW_RECORDER_H__
#define __AW_RECORDER_H__

#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <vencoder.h>
#include <CdxMuxer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* AwEncoder;

typedef enum AwEncoderIndexType
{
    AwEncoder_SetFrameRate = 0x0,
    AwEncoder_SetBitRate,
	AwEncoder_SetScaleDownRatio,
	AwEncoder_SetThumbNailEnable,
	AwEncoder_SetThumbNailAddr,
}AwEncoderIndexType;

enum ERECORDERINTERNALNOTIFY  //*
{
    AWENCODER_VIDEO_ENCODER_NOTIFY_STREAM_UNDERFLOW        = 32,
    AWENCODER_VIDEO_ENCODER_NOTIFY_CRASH,
    AWENCODER_VIDEO_ENCODER_NOTIFY_EOS,
    AWENCODER_VIDEO_ENCODER_NOTIFY_OUT_BUFFER,
    AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_BUFFER,
    AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_THUMBNAIL,
    AWENCODER_VIDEO_ENCODER_NOTIFY_THUMBNAIL_GETBUFFER,
    AWENCODER_VIDEO_ENCODER_NOTIFY_ERROR,

    AWENCODER_AUDIO_ENCODER_NOTIFY_STREAM_UNDERFLOW        = 96,
    AWENCODER_AUDIO_ENCODER_NOTIFY_CRASH,
    AWENCODER_AUDIO_ENCODER_NOTIFY_EOS,
    AWENCODER_AUDIO_ENCODER_NOTIFY_OUT_BUFFER,

    AWENCODER_MUXER_NOTIFY_FIRST_FRAME              = 128,
    AWENCODER_MUXER_NOTIFY_EOS,
    AWENCODER_MUXER_NOTIFY_PTS_AND_CACHETIME,
};

typedef enum VIDEO_PIXEL_FMT {
    VIDEO_PIXEL_YUV420_NV12,//NV12
    VIDEO_PIXEL_YUV420_NV21,//NV21
    VIDEO_PIXEL_YUV420_YU12,//YU12
    VIDEO_PIXEL_YVU420_YV12,//YV12
    VIDEO_PIXEL_YUV422SP,
    VIDEO_PIXEL_YVU422SP,
    VIDEO_PIXEL_YUV422P,
    VIDEO_PIXEL_YVU422P,
    VIDEO_PIXEL_YUYV422,
    VIDEO_PIXEL_UYVY422,
    VIDEO_PIXEL_YVYU422,
    VIDEO_PIXEL_VYUY422,
    VIDEO_PIXEL_ARGB,
    VIDEO_PIXEL_RGBA,
    VIDEO_PIXEL_ABGR,
    VIDEO_PIXEL_BGRA,
    VIDEO_PIXEL_YUV420_MB32,//MB32_420
    VIDEO_PIXEL_TILE_128X32,
}VIDEO_ENCODER_PIXEL_FORMAT;


typedef enum VIDEO_ENCODE_TYPE
{
    VIDEO_ENCODE_H264,
    VIDEO_ENCODE_JPEG,
}VIDEO_ENCODE_TYPE;

typedef enum AUDIO_ENCODE_TYPE
{
     AUDIO_ENCODE_AAC_TYPE,
     AUDIO_ENCODE_LPCM_TYPE,    //only used by mpeg2ts
     AUDIO_ENCODE_PCM_TYPE,
     AUDIO_ENCODE_MP3_TYPE,
}AUDIO_ENCODE_TYPE;

typedef struct AudioEncodeConfig
{
    AUDIO_ENCODE_TYPE     nType;
    int                    nInSamplerate;   //那?豕?fs
    int                 nInChan;         //那?豕?pcm chan 1:mon 2:stereo
    int                 nBitrate;        //bs
    int                 nSamplerBits;    //only for 16bits
    int                 nOutSamplerate;  //那?3?fs,now OutSamplerate must equal InSamplerate
    int                 nOutChan;        //㊣角??那?3? chan
    //for aac: 0:add head,1:raw data;
    //for pcm: 2:mpegTs pcm(big endian),
    //other: common pcm(little endian)
    int                 nFrameStyle;
}AudioEncodeConfig;

typedef struct VideoEncodeConfig
{
    VIDEO_ENCODE_TYPE nType;
    int             nFrameRate;
    int             nBitRate;
    int             nOutWidth;
    int             nOutHeight;
    int             nSrcFrameRate;
    int             nSrcWidth;
    int             nSrcHeight;
    int             bUsePhyBuf;
	VIDEO_ENCODER_PIXEL_FORMAT nInputYuvFormat;
	E_ISP_SCALER_RATIO ratio;
}VideoEncodeConfig;

typedef struct VideoInputBuffer
{
    unsigned char*  pData;            //virtual data buff
    int             nLen;            //data len
    long long       nPts;            //pts

    unsigned long   nID;
    unsigned char*  pAddrPhyY;
    unsigned char*  pAddrPhyC;
}VideoInputBuffer;

typedef struct AudioInputBuffer
{
    char   *pData;            //data buff
    int             nLen;            //data len
    long long       nPts;            //pts
}AudioInputBuffer;

typedef struct VideoOutBuffer
{
    unsigned int   nSize0;
    unsigned int   nSize1;
    unsigned char* pData0;
    unsigned char* pData1;
}VideoOutBuffer;

typedef struct AudioEncOutBuffer
{
    char       *pBuf;
    unsigned int        len;
    long long  pts;
    int        bufId;
}AudioEncOutBuffer;

typedef struct EncDataCallBackOps
{
    int (*onVideoDataEnc)(void *app,CdxMuxerPacketT *buff);
    int (*onAudioDataEnc)(void *app,CdxMuxerPacketT *buff);
} EncDataCallBackOps;

typedef void (*EncoderNotifyCallback)(void* pUserData, int msg, void* param);

AwEncoder*     AwEncoderCreate(void * app);
void           AwEncoderDestory(AwEncoder* p);
int            AwEncoderInit(AwEncoder* p,
                                  VideoEncodeConfig *videoConfig,
                                  AudioEncodeConfig *audioConfig,
                                  EncDataCallBackOps *ops);

int            AwEncoderGetExtradata(AwEncoder* v, unsigned char** buf, unsigned int* length);
int            AwEncoderSetParamete(AwEncoder* p, AwEncoderIndexType nIndexType, void* para);
int            AwEncoderStart(AwEncoder* p);
int            AwEncoderStop(AwEncoder* p);
int            AwEncoderReset(AwEncoder* p);

int            AwEncoderWriteYUVdata(AwEncoder* p, VideoInputBuffer* buf);
int            AwEncoderWritePCMdata(AwEncoder* p, AudioInputBuffer* buf);

int            AwEncoderSetNotifyCallback(AwEncoder* p,
                                                    EncoderNotifyCallback notifier,
                                                    void* pUserData);

#ifdef __cplusplus
}
#endif

#endif /* __RECORDER_H__ */
