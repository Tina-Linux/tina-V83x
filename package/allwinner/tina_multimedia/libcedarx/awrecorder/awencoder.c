/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : awencoder.cpp
 * Description : encoder
 * History :
 *
 */

#include "awencoder.h"
#include "AudioEncodeComponent.h"
#include "VideoEncodeComponent.h"
#include "EncDataComponent.h"
#include "vencoder.h"
#include "aencoder.h"
#include "AwMessageQueue.h"
#include "CdxMuxer.h"

//* player status.
enum AwEncoderStatus
{
    AWRECORDER_STATUS_IDLE        = 0,
    AWRECORDER_STATUS_INITIALIZED  = 1,
     AWRECORDER_STATUS_PREPARING    = 2,
     AWRECORDER_STATUS_PREPARED    = 3,
     AWRECORDER_STATUS_STARTED     = 4,
     AWRECORDER_STATUS_PAUSED      = 5,
     AWRECORDER_STATUS_STOPPED     = 6,
     AWRECORDER_STATUS_COMPLETE    = 7,
     AWRECORDER_STATUS_ERROR       = 8,
};

//* command id.
static const int AWENCODER_COMMAND_SET_SOURCE    = 0x101;
static const int AWENCODER_COMMAND_SET_SURFACE   = 0x102;
static const int AWENCODER_COMMAND_SET_AUDIOSINK = 0x103;
static const int AWENCODER_COMMAND_PREPARE       = 0x104;
static const int AWENCODER_COMMAND_START         = 0x105;
static const int AWENCODER_COMMAND_STOP          = 0x106;
static const int AWENCODER_COMMAND_PAUSE         = 0x107;
static const int AWENCODER_COMMAND_RESET         = 0x108;
static const int AWENCODER_COMMAND_QUIT          = 0x109;

typedef struct AwEncoderContext
{
    EncDataComp          *pEncData;
    AudioEncodeComp    *pAudioEncode;
    VideoEncodeComp    *pVideoEncode;

    VideoEncodeConfig      *pVideoConfig;
    AudioEncodeConfig      *pAudioConfig;

    pthread_t           mThreadId;
    int                 mStatus;
    int                 mThreadCreated;

    EncoderNotifyCallback   mCallback;
    void*               mUserData;

    void *              mApp;
    EncDataCallBackOps *mEncDataCallBackOps;

}AwEncoderContext;

static int ComponentCallbackProcess(void* pSelf, int eMessageId, void* param)
{
    AwEncoderContext* p;

    p = (AwEncoderContext*)pSelf;
	VencThumbInfo *thumb;

    switch(eMessageId)
    {
        case VIDEO_ENCODE_NOTIFY_ERROR:
        case VIDEO_ENCODE_NOTIFY_CRASH:
            if(p->mCallback)
                p->mCallback(p->mUserData, AWENCODER_VIDEO_ENCODER_NOTIFY_ERROR, param);
            break;

        case VIDEO_ENCODE_NOTIFY_RETURN_BUFFER:
        {
            int id = *((int*)param);
            logv("===== VIDEO_ENCODE_NOTIFY_RETURN_BUFFER: %d", id);
            if(p->mCallback)
                p->mCallback(p->mUserData, AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_BUFFER, &id);
            break;
        }

		case VIDEO_ENCODE_NOTIFY_THUMBNAIL_BUFFER:
            if(p->mCallback)
                p->mCallback(p->mUserData, AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_THUMBNAIL, param);
			break;
		case VIDEO_ENCODE_NOTIFY_THUMBNAIL_GETBUFFER:
            if(p->mCallback)
                p->mCallback(p->mUserData, AWENCODER_VIDEO_ENCODER_NOTIFY_THUMBNAIL_GETBUFFER, param);
			break;
        case VIDEO_ENCODE_NOTIFY_ENCODED_BUFFER:
        {
            break;
        }

        default:
            loge("unkown callback (%d)", eMessageId);
            break;
    }

    return 0;
}

AwEncoder*    AwEncoderCreate(void * app)
{
    AwEncoderContext* p;

    p = (AwEncoderContext*)malloc(sizeof(AwEncoderContext));
    if(!p)
    {
        loge("malloc failed");
        return NULL;
    }
    memset(p, 0x00, sizeof(AwEncoderContext));

    p->mApp = app;

    return (AwEncoder*)p;
}

void     AwEncoderDestory(AwEncoder* v)
{
    AwEncoderContext* p = (AwEncoderContext*)v;

    if(p->pEncData)
    {
        EncDataCompDestory(p->pEncData);
    }

    if(p->pAudioEncode)
    {
        AudioEncodeCompDestory(p->pAudioEncode);
    }

    if(p->pVideoEncode)
    {
        VideoEncodeCompDestory(p->pVideoEncode);
    }

    free(p);
}

int AwEncoderInit(AwEncoder* v,
                        VideoEncodeConfig *videoConfig,
                        AudioEncodeConfig *audioConfig,
                        EncDataCallBackOps *ops)
{
    AwEncoderContext* p = (AwEncoderContext*)v;

    if(videoConfig)
    {
        p->pVideoConfig = videoConfig;
    }

    if(audioConfig)
    {
        p->pAudioConfig = audioConfig;
    }

    p->mEncDataCallBackOps = ops;
    return 0;
}

int AwEncoderGetExtradata(AwEncoder* v, unsigned char** buf, unsigned int* length)
{
    AwEncoderContext* p = (AwEncoderContext*)v;
    if (p && p->pVideoEncode)
    {
        return VideoEncodeCompGetExtradata(p->pVideoEncode, buf, length);
    }
    else
    {
        loge("GetExtradata fail!");
        return -1;
    }
}

int AwEncoderSetParamete(AwEncoder* v, AwEncoderIndexType nIndexType, void* para)
{
    AwEncoderContext* p = (AwEncoderContext*)v;
    int ret = 0;
    switch(nIndexType)
    {
        case AwEncoder_SetFrameRate:
            logd("set frame rate:%d",*(int32_t *)para);
            ret = VideoEncodeCompSetFrameRate(p->pVideoEncode,*(int32_t *)para);
            break;
        case AwEncoder_SetBitRate:
            logd("set bit rate:%d",*(int32_t *)para);
            ret = VideoEncodeCompSetBitRate(p->pVideoEncode,*(int32_t *)para);
            break;
		case AwEncoder_SetScaleDownRatio:
            logd("set scale down ratio:%d",*(int32_t *)para);
            ret = VideoEncodeCompScaleDownRatio(p->pVideoEncode,*(int32_t *)para);
			break;
		case AwEncoder_SetThumbNailEnable:
			ret = VideoEncodeCompSetThumbNailEnable(p->pVideoEncode,*(int32_t *)para);
			break;
		case AwEncoder_SetThumbNailAddr:
			ret = VideoEncodeCompSetThumbNailAddr(p->pVideoEncode,(VencThumbInfo *)para);
			break;
        default:
            logw("the %d index is not support",nIndexType);
            break;
    }
    return ret;
}

int AwEncoderStart(AwEncoder* v)
{
    AwEncoderContext *p;
    int ret;

    p = (AwEncoderContext*)v;

    if(p->pAudioConfig)
    {
        p->pAudioEncode = AudioEncodeCompCreate();
        if(p->pAudioEncode)
        {
            AudioEncodeCompInit(p->pAudioEncode, p->pAudioConfig);
        }
        else
        {
            loge("create audio encoder failed");
        }
    }

    if(p->pVideoConfig)
    {
        logd("++++ VideoEncodeCompCreate");
        p->pVideoEncode = VideoEncodeCompCreate();
        if(p->pVideoEncode)
        {
            VideoEncodeCompInit(p->pVideoEncode, p->pVideoConfig);
        }
        else
        {
            loge("create video encoder failed");

        }
    }

    p->pEncData = EncDataCompCreate(p->mApp);
    if(!p->pEncData)
    {
        loge("EncDataCompCreate failed");
        return -1;
    }

    if(p->pVideoEncode)
        EncDataCompSetVideoEncodeComp(p->pEncData,p->pVideoEncode);
    if(p->pAudioEncode)
        EncDataCompSetAudioEncodeComp(p->pEncData,p->pAudioEncode);

    ret = EncDataCompInit(p->pEncData, p->pVideoConfig, p->pAudioConfig, p->mEncDataCallBackOps);
    if(ret < 0)
    {
        loge("EncDataCompInit failed");
        return -1;
    }

    if(p->pAudioEncode)
    {
        AudioEncodeCompSetCallback(p->pAudioEncode, ComponentCallbackProcess, p);
        AudioEncodeCompStart(p->pAudioEncode);
    }

    if(p->pVideoEncode)
    {
        VideoEncodeCompSetCallback(p->pVideoEncode, ComponentCallbackProcess, p);
        VideoEncodeCompStart(p->pVideoEncode);
    }

    if(p->pEncData)
    {
        EncDataCompSetCallback(p->pEncData, ComponentCallbackProcess, p);
        EncDataCompStart(p->pEncData);
    }

    return 0;
}

int AwEncoderStop(AwEncoder* v)
{
    AwEncoderContext *p;

    p = (AwEncoderContext*)v;

    int ret = 0;

    if(p->pEncData)
    {
        ret = EncDataCompStop(p->pEncData);
        if(ret < 0){
            loge("EncDataCompStop fail");
            return -1;
        }
    }

    if(p->pAudioEncode)
    {
        int ret = AudioEncodeCompStop(p->pAudioEncode);
        if(ret < 0){
            loge("AudioEncodeCompStop fail");
            return -1;
        }
    }

    if(p->pVideoEncode)
    {
        int ret = VideoEncodeCompStop(p->pVideoEncode);
        if(ret < 0){
            loge("VideoEncodeCompStop fail");
            return -1;
        }
    }

    return 0;
}

int AwEncoderReset(AwEncoder* v)
{
    AwEncoderContext *p;

    p = (AwEncoderContext*)v;

    int ret = 0;

    if(p->pEncData)
    {
        ret = EncDataCompReset(p->pEncData);
        if(ret < 0){
            loge("EncDataCompReset fail");
            return -1;
        }
    }

    if(p->pAudioEncode)
    {
        ret = AudioEncodeCompReset(p->pAudioEncode);
        if(ret < 0){
            loge("AudioEncodeCompReset fail");
            return -1;
        }
    }

    if(p->pVideoEncode)
    {
        ret = VideoEncodeCompReset(p->pVideoEncode);
        if(ret < 0){
            loge("VideoEncodeCompReset fail");
            return -1;
        }
    }

    p->pVideoConfig = NULL;
    p->pAudioConfig = NULL;

    return 0;
}

int AwEncoderWriteYUVdata(AwEncoder* v, VideoInputBuffer* buf)
{
    AwEncoderContext *p;
    int ret;

    p = (AwEncoderContext*)v;

    if(!p->pVideoEncode)
    {
        loge("video encoder not create");
        return -1;
    }
    ret = VideoEncodeCompInputBuffer(p->pVideoEncode, buf);
    if(ret < 0)
    {
        loge("input video data failed");
        return -1;
    }

    return 0;
}

int AwEncoderWritePCMdata(AwEncoder* v, AudioInputBuffer* buf)
{
    AwEncoderContext *p;
    int ret;

    p = (AwEncoderContext*)v;

    if(!p->pAudioEncode)
    {
        loge("audio encoder not create");
        return -1;
    }
    ret = AudioEncodeCompInputBuffer(p->pAudioEncode, buf);
    if(ret < 0)
    {
        loge("input audio data failed");
        return -1;
    }

    return 0;
}

int   AwEncoderSetNotifyCallback(AwEncoder* v, EncoderNotifyCallback notifier, void* pUserData)
{
    AwEncoderContext *p = (AwEncoderContext*)v;

    p->mCallback = notifier;
    p->mUserData = pUserData;
    return 0;
}
