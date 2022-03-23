/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : VideoEncodeComponent.h
 * Description : VideoEncodeComponent
 * History :
 *
 */

#ifndef __ENCODER_COMP_H__
#define __ENCODER_COMP_H__

#include <semaphore.h>
#include <pthread.h>

#include "vencoder.h"
#include "AwMessageQueue.h"
#include "awencoder.h"

enum VIDEOENCODERNOTIFY
{
    VIDEO_ENCODE_NOTIFY_ERROR,
    VIDEO_ENCODE_NOTIFY_CRASH,
    VIDEO_ENCODE_NOTIFY_RETURN_BUFFER,
    VIDEO_ENCODE_NOTIFY_ENCODED_BUFFER,
    VIDEO_ENCODE_NOTIFY_THUMBNAIL_BUFFER,
    VIDEO_ENCODE_NOTIFY_THUMBNAIL_GETBUFFER,
};

typedef void* VideoEncodeComp;

typedef int (*VideoEncodeCallback)(void* pUserData, int eMessageId, void* param);

VideoEncodeComp* VideoEncodeCompCreate();
int VideoEncodeCompInit(VideoEncodeComp* p, VideoEncodeConfig* config);
void VideoEncodeCompDestory(VideoEncodeComp* p);

int VideoEncodeCompStart(VideoEncodeComp *p);
int VideoEncodeCompStop(VideoEncodeComp *p);
int VideoEncodeCompReset(VideoEncodeComp* p)  ;

int VideoEncodeCompSetFrameRate(VideoEncodeComp *p, int framerate);
int VideoEncodeCompSetBitRate(VideoEncodeComp *p, int bitrate);
int VideoEncodeCompScaleDownRatio(VideoEncodeComp* v, int32_t ratio);

int VideoEncodeCompInputBuffer(VideoEncodeComp* v, VideoInputBuffer *buf);
int VideoEncodeCompRequestVideoFrame(VideoEncodeComp *p, VencOutputBuffer *pBuffer);
int VideoEncodeCompReturnVideoFrame(VideoEncodeComp *v, VencOutputBuffer* outputBuffer);
int VideoEncodeCompSetCallback(VideoEncodeComp *p, VideoEncodeCallback notifier, void* pUserData);
int VideoEncodeCompGetExtradata(VideoEncodeComp *p, unsigned char** buf, unsigned int* length);
int VideoEncodeCompSetThumbNailAddr(VideoEncodeComp *v,VencThumbInfo *addr);
int VideoEncodeCompSetThumbNailEnable(VideoEncodeComp *v,int32_t enable);

#endif
