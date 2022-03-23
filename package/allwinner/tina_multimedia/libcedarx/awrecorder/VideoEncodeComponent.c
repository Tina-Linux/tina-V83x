/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : VideoEncodeComponent.cpp
 * Description : VideoEncodeComponent
 * History :
 *
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "VideoEncoderComponent"

#include <cdx_log.h>
#include <stdlib.h>
#include <unistd.h>
#include "VideoEncodeComponent.h"
#include "memoryAdapter.h"

//* demux status, same with the awplayer.
static const int ENC_STATUS_IDLE        = 0;   //* parsing and sending data.
static const int ENC_STATUS_STARTED     = 1;   //* parsing and sending data.
static const int ENC_STATUS_PAUSED      = 1<<1;   //* sending job paused.
static const int ENC_STATUS_STOPPED     = 1<<2;   //* parser closed.

//* command id in encode thread
static const int ENC_COMMAND_ENCODE        = 0x101;
static const int ENC_COMMAND_START         = 0x104;
static const int ENC_COMMAND_STOP          = 0x106;
static const int ENC_COMMAND_PAUSE         = 0x107;
static const int ENC_COMMAND_RESET         = 0x108;
static const int ENC_COMMAND_QUIT          = 0x109;

//* for T2 & R8
//#define NEED_CONV_NV12 1
#define NEED_CONV_NV12 0

#define ALIGN_16B(x) (((x) + (15)) & ~(15))

typedef struct SoftFrameRateCtrl
{
    int capture;
    int total;
    int count;
    int enable;
} SoftFrameRateCtrl;

struct AwMessage {
    AWMESSAGE_COMMON_MEMBERS
    uintptr_t params[4];
};

typedef struct EncodeCompContex
{
    pthread_t               mThreadId;
    int                     mThreadCreated;
    AwMessageQueue*             mMessageQueue;

    int                     mStatus;

    int                     mUseAllocInputBuffer;

    int                     mStartReply;
    int                     mStopReply;
    int                     mPauseReply;
    int                     mResetReply;

    sem_t                   mSemStart;
    sem_t                   mSemStop;
    sem_t                   mSemPause;
    sem_t                   mSemReset;
    sem_t                   mSemQuit;
    sem_t                   mSemWaitData;

    VideoEncoder*           pEncoder;
    VENC_CODEC_TYPE         mEncodeType;
    int                     mFrameRate; // *1000
    int                     mBitRate;
    int                     mSrcFrameRate;
    int                     mDesOutWidth;       //destination encode video width and height
    int                     mDesOutHeight;
    int                     mInputWidth;
    int                     mInputHeight;

    VencHeaderData          mPpsInfo;    //ENCEXTRADATAINFO_t //spspps info.

    VideoEncodeCallback     mCallback;
    void*                   mUserData;

    int                     mEos;

    SoftFrameRateCtrl       mFrameRateCtrl;

    struct ScMemOpsS*       memops;
    int              mInputBufSizeY;
    int              mInputBufSizeC;
#if NEED_CONV_NV12
    unsigned char *uv_tmp_buffer;
#endif
	int thumbNailEnable;
	VencThumbInfo thInfo;
}EncodeCompContex;

#if NEED_CONV_NV12
static int yu12_nv12(unsigned int width, unsigned int height,
                    unsigned char *addr_uv, unsigned char *addr_tmp_uv)
{
    unsigned int i, chroma_bytes;
    unsigned char *u_addr = NULL;
    unsigned char *v_addr = NULL;
    unsigned char *tmp_addr = NULL;

    chroma_bytes = width*height/4;

    u_addr = addr_uv;
    v_addr = addr_uv + chroma_bytes;
    tmp_addr = addr_tmp_uv;

    for(i=0; i<chroma_bytes; i++)
    {
        *(tmp_addr++) = *(u_addr++);
        *(tmp_addr++) = *(v_addr++);
    }

    memcpy(addr_uv, addr_tmp_uv, chroma_bytes*2);

    return 0;
}
#endif

static int setFrameRateControl(int reqFrameRate, int srcFrameRate, int *numerator, int *denominator)
{
    int i;
    int ratio;

    if (reqFrameRate >= srcFrameRate)
    {
        logv("reqFrameRate[%d] > srcFrameRate[%d]", reqFrameRate, srcFrameRate);
        *numerator = 0;
        *denominator = 0;
        return -1;
    }

    ratio = (int)((float)reqFrameRate / srcFrameRate * 1000);

    for (i = srcFrameRate; i > 1; --i)
    {
        if (ratio < (int)((float)1.0 / i * 1000))
        {
            break;
        }
    }

    if (i == srcFrameRate)
    {
        *numerator = 1;
        *denominator = i;
    }
    else if (i > 1)
    {
        int rt1 = (int)((float)1.0 / i * 1000);
        int rt2 = (int)((float)1.0 / (i+1) * 1000);
        *numerator = 1;
        if (ratio - rt2 > rt1 - ratio)
        {
            *denominator = i;
        }
        else
        {
            *denominator = i + 1;
        }
    }
    else
    {
        for (i = 2; i < srcFrameRate - 1; ++i)
        {
            if (ratio < (int)((float)i / (i+1) * 1000))
            {
                break;
            }
        }
        int rt1 = (int)((float)(i-1) / i * 1000);
        int rt2 = (int)((float)i / (i+1) * 1000);
        if (ratio - rt1 > rt2 - ratio)
        {
            *numerator = i;
            *denominator = i+1;
        }
        else
        {
            *numerator = i-1;
            *denominator = i;
        }
    }
    return 0;
}

static void PostEncodeMessage(AwMessageQueue* mq)
{
    if(AwMessageQueueGetCount(mq)<=0)
    {
        AwMessage msg;
        msg.messageId = ENC_COMMAND_ENCODE;
        msg.params[0] = msg.params[1] = msg.params[2] = msg.params[3] = 0;
        if(AwMessageQueuePostMessage(mq, &msg) != 0)
        {
            loge("fatal error, video encode component post message fail.");
            abort();
        }

        return;
    }
}

static void* VideoEncodeThread(void* pArg)
{
    EncodeCompContex   *p = (EncodeCompContex*)pArg;
    AwMessage             msg;
    int                  ret;
    sem_t*                 pReplySem;
    int*                     pReplyValue;
    VencInputBuffer inputBufferReturn;

    while(1)
    {
        if(AwMessageQueueGetMessage(p->mMessageQueue, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }

        pReplySem    = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == ENC_COMMAND_START)
        {
            if(p->mStatus == ENC_STATUS_STARTED)
            {
                loge("invalid start operation, already in started status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            if(p->mStatus == ENC_STATUS_PAUSED)
            {
                PostEncodeMessage(p->mMessageQueue);
                p->mStatus = ENC_STATUS_STARTED;
                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            PostEncodeMessage(p->mMessageQueue);
            // reset encoder ******
            p->mStatus = ENC_STATUS_STARTED;
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end ENC_COMMAND_START.
        else if(msg.messageId == ENC_COMMAND_ENCODE)
        {
            sem_wait(&p->mSemWaitData);
            ret = VideoEncodeOneFrame(p->pEncoder);
            if(ret == VENC_RESULT_ERROR)
            {
                loge("encode error");
                p->mCallback(p->mUserData, VIDEO_ENCODE_NOTIFY_ERROR, NULL);
                PostEncodeMessage(p->mMessageQueue);
                continue;
            }
            else if(ret == VENC_RESULT_NO_FRAME_BUFFER)
            {
                logd("p->mDesOutWidth = %d,encode VENC_RESULT_NO_FRAME_BUFFER",p->mDesOutWidth);
                usleep(5*1000); // it is not a good idea to do this
                PostEncodeMessage(p->mMessageQueue);
                continue;
            }
            else if (ret == VENC_RESULT_BITSTREAM_IS_FULL)
            {
                logd("p->mDesOutWidth = %d,encode VENC_RESULT_BITSTREAM_IS_FULL",p->mDesOutWidth);
                //usleep(5*1000); // it is not a good idea to do this
                //PostEncodeMessage(p->mMessageQueue);
                //continue;
                if(p->thumbNailEnable)
		{
			p->mCallback(p->mUserData, VIDEO_ENCODE_NOTIFY_THUMBNAIL_GETBUFFER,
				NULL);
		}

		if(p->thumbNailEnable && p->thInfo.nThumbSize != -1 && p->thInfo.pThumbBuf != NULL)
		{
			int ret;
			ret = VideoEncGetParameter(p->pEncoder, VENC_IndexParamGetThumbYUV, (void *)&p->thInfo);
                        p->mCallback(p->mUserData, VIDEO_ENCODE_NOTIFY_THUMBNAIL_BUFFER,&p->thInfo);
		}

		if(p->mUseAllocInputBuffer == 0)
		{
		    //* check used buffer, return it
		    if(0==AlreadyUsedInputBuffer(p->pEncoder, &inputBufferReturn))
		    {
		        logv("return this buf(%lu)", inputBufferReturn.nID);
		        p->mCallback(p->mUserData, VIDEO_ENCODE_NOTIFY_RETURN_BUFFER,
		                        &inputBufferReturn.nID);
		    }
		}
		PostEncodeMessage(p->mMessageQueue);
		continue;
            }
            else if(ret < 0)
            {
                loge(" encode crash");
                p->mCallback(p->mUserData, VIDEO_ENCODE_NOTIFY_CRASH, NULL);
                continue;
            }
            else
            {
                logv("encode ok");
				if(p->mUseAllocInputBuffer == 0)
				{
				    //* check used buffer, return it
				    if(0==AlreadyUsedInputBuffer(p->pEncoder, &inputBufferReturn))
				    {
				        logv("return this buf(%lu)", inputBufferReturn.nID);
				        p->mCallback(p->mUserData, VIDEO_ENCODE_NOTIFY_RETURN_BUFFER,
				                        &inputBufferReturn.nID);
				    }
				}
				if(p->thumbNailEnable)
				{
					p->mCallback(p->mUserData, VIDEO_ENCODE_NOTIFY_THUMBNAIL_GETBUFFER, NULL);
				}
				if(p->thumbNailEnable && p->thInfo.nThumbSize != -1 && p->thInfo.pThumbBuf != NULL)
				{
					int ret;
					ret = VideoEncGetParameter(p->pEncoder, VENC_IndexParamGetThumbYUV, (void *)&p->thInfo);
					p->mCallback(p->mUserData, VIDEO_ENCODE_NOTIFY_THUMBNAIL_BUFFER, &p->thInfo);
				}

				PostEncodeMessage(p->mMessageQueue);
				continue;
            }
        } //* end ENC_COMMAND_ENCODE.
        else if(msg.messageId == ENC_COMMAND_STOP)
        {
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;    //* break the thread.
        } //* end ENC_COMMAND_STOP.
        else if(msg.messageId == ENC_COMMAND_PAUSE)
        {
            if(p->mStatus == ENC_STATUS_PAUSED)
            {
                loge("invalid pause operation, already in pause status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }
            p->mStatus = ENC_STATUS_PAUSED;
            PostEncodeMessage(p->mMessageQueue);

            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
            continue;
        } //* end ENC_COMMAND_PAUSE.
        else if(msg.messageId == ENC_COMMAND_RESET)
        {
            VideoEncUnInit(p->pEncoder);
            VideoEncDestroy(p->pEncoder);
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
            continue;
        }
        else if(msg.messageId == ENC_COMMAND_QUIT)
        {
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            break;
        } //* end AWPLAYER_COMMAND_QUIT.
        else
        {
            logw("unknow message with id %d, ignore.", msg.messageId);
        }
    }

    return NULL;
}

static int setEncodeType(EncodeCompContex *p, VideoEncodeConfig* config)
{
    VencAllocateBufferParam bufferParam;
    //init decoder and encoder.
    if(config->nType == VIDEO_ENCODE_H264)
        p->mEncodeType = VENC_CODEC_H264;
    else if(config->nType == VIDEO_ENCODE_JPEG)
        p->mEncodeType = VENC_CODEC_JPEG;
    else
    {
        loge("not support this video encode type");
        return -1;
    }

    p->mFrameRate    = config->nFrameRate;
    p->mBitRate      = config->nBitRate;
    p->mDesOutWidth  = config->nOutWidth;
    p->mDesOutHeight = config->nOutHeight;
    p->mInputWidth   = config->nSrcWidth;
    p->mInputHeight  = config->nOutHeight;
    p->mSrcFrameRate = config->nSrcFrameRate;
    if(p->mEncodeType != VENC_CODEC_H264 && p->mEncodeType != VENC_CODEC_JPEG)
    {
        loge("connot support this video type, p->mEncodeType(%d)", p->mEncodeType);
        return -1;
    }

    //* create an encoder.
    p->pEncoder = VideoEncCreate(p->mEncodeType);
    if(p->pEncoder == NULL)
    {
        loge("VideoEncCreate fail");
        return -1;
    }

    //* init video encoder
    VencBaseConfig baseConfig;

    memset(&baseConfig, 0 ,sizeof(VencBaseConfig));
    baseConfig.nInputWidth  = config->nSrcWidth;
    baseConfig.nInputHeight = config->nSrcHeight;
    baseConfig.nStride      = ALIGN_16B(config->nSrcWidth);
    baseConfig.nDstWidth    = p->mDesOutWidth;
    baseConfig.nDstHeight   = p->mDesOutHeight;

    baseConfig.memops = p->memops;

    memset(&bufferParam, 0 ,sizeof(VencAllocateBufferParam));

    //bufferParam.nBufferNum = 2; // for C500
    bufferParam.nBufferNum = 6;

    switch(config->nInputYuvFormat)
    {
        case VIDEO_PIXEL_YUV420_NV12:
            baseConfig.eInputFormat = VENC_PIXEL_YUV420SP;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
            bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;
            break;
        case VIDEO_PIXEL_YUV420_NV21:
            baseConfig.eInputFormat = VENC_PIXEL_YVU420SP;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
            bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;
            break;
        case VIDEO_PIXEL_YUV420_YU12:
            baseConfig.eInputFormat = VENC_PIXEL_YUV420P;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
            bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;
            break;
        case VIDEO_PIXEL_YVU420_YV12:
            baseConfig.eInputFormat = VENC_PIXEL_YVU420P;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
            bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;
            break;
        case VIDEO_PIXEL_YUV422SP:
            baseConfig.eInputFormat = VENC_PIXEL_YUV422SP;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
            bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight;
            break;
        case VIDEO_PIXEL_YVU422SP:
            baseConfig.eInputFormat = VENC_PIXEL_YVU422SP;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
            bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight;
            break;
        case VIDEO_PIXEL_YUV422P:
            baseConfig.eInputFormat = VENC_PIXEL_YUV422P;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
            bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight;
            break;
        case VIDEO_PIXEL_YVU422P:
            baseConfig.eInputFormat = VENC_PIXEL_YVU422P;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
            bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight;
            break;
        case VIDEO_PIXEL_YUYV422:
            baseConfig.eInputFormat = VENC_PIXEL_YUYV422;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight*2;
            bufferParam.nSizeC = 0;
            break;
        case VIDEO_PIXEL_UYVY422:
            baseConfig.eInputFormat = VENC_PIXEL_UYVY422;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight*2;
            bufferParam.nSizeC = 0;
            break;
        case VIDEO_PIXEL_YVYU422:
            baseConfig.eInputFormat = VENC_PIXEL_YVYU422;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight*2;
            bufferParam.nSizeC = 0;
            break;
        case VIDEO_PIXEL_VYUY422:
            baseConfig.eInputFormat = VENC_PIXEL_VYUY422;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight*2;
            bufferParam.nSizeC = 0;
            break;
        case VIDEO_PIXEL_ARGB:
            baseConfig.eInputFormat = VENC_PIXEL_ARGB;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight*4;
            bufferParam.nSizeC = 0;
            break;
        case VIDEO_PIXEL_RGBA:
            baseConfig.eInputFormat = VENC_PIXEL_RGBA;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight*4;
            bufferParam.nSizeC = 0;
            break;
        case VIDEO_PIXEL_ABGR:
            baseConfig.eInputFormat = VENC_PIXEL_ABGR;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight*4;
            bufferParam.nSizeC = 0;
            break;
        case VIDEO_PIXEL_BGRA:
            baseConfig.eInputFormat = VENC_PIXEL_BGRA;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight*4;
            bufferParam.nSizeC = 0;
            break;
        case VIDEO_PIXEL_YUV420_MB32:
            baseConfig.eInputFormat = VENC_PIXEL_TILE_32X32;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
            bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;
            break;
        case VIDEO_PIXEL_TILE_128X32:
            baseConfig.eInputFormat = VENC_PIXEL_TILE_128X32;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
            bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;
            break;
        default:
            logw("the config->nInputYuvFormat is not support,use the default format:VENC_PIXEL_YUV420P");
            baseConfig.eInputFormat = VENC_PIXEL_YUV420P;
            bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
            bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;
            break;
    }

    //store the input buf sizeY and sizeC
    p->mInputBufSizeY = bufferParam.nSizeY;
    p->mInputBufSizeC = bufferParam.nSizeC;

	if(config->ratio != VENC_ISP_SCALER_0)
	{
		int ret;
		p->thumbNailEnable = 1;
		ret = VideoEncSetParameter(p->pEncoder, VENC_IndexParamSetThumbScaler, (void *)&config->ratio);
		p->thInfo.nThumbSize = -1;
		p->thInfo.pThumbBuf = NULL;

		printf("rett:%d\n",ret);
	}
    //for jpeg
    if(p->mEncodeType == VENC_CODEC_JPEG)
    {
        EXIFInfo exifinfo;
        exifinfo.ThumbWidth = 176;
        exifinfo.ThumbHeight = 144;

        strcpy((char*)exifinfo.CameraMake,        "allwinner make test");
        strcpy((char*)exifinfo.CameraModel,        "allwinner model test");
        strcpy((char*)exifinfo.DateTime,         "2014:02:21 10:54:05");
        strcpy((char*)exifinfo.gpsProcessingMethod,  "allwinner gps");

        exifinfo.Orientation = 0;

        exifinfo.ExposureTime.num = 2;
        exifinfo.ExposureTime.den = 1000;

        exifinfo.FNumber.num = 20;
        exifinfo.FNumber.den = 10;
        exifinfo.ISOSpeed = 50;

        exifinfo.ExposureBiasValue.num= -4;
        exifinfo.ExposureBiasValue.den= 1;

        exifinfo.MeteringMode = 1;
        exifinfo.FlashUsed = 0;

        exifinfo.FocalLength.num = 1400;
        exifinfo.FocalLength.den = 100;

        exifinfo.DigitalZoomRatio.num = 4;
        exifinfo.DigitalZoomRatio.den = 1;

        exifinfo.WhiteBalance = 1;
        exifinfo.ExposureMode = 1;

        exifinfo.enableGpsInfo = 1;

        exifinfo.gps_latitude = 23.2368;
        exifinfo.gps_longitude = 24.3244;
        exifinfo.gps_altitude = 1234.5;

        exifinfo.gps_timestamp = (long)time(NULL);

        strcpy((char*)exifinfo.CameraSerialNum,  "123456789");
        strcpy((char*)exifinfo.ImageName,  "exif-name-test");
        strcpy((char*)exifinfo.ImageDescription,  "exif-descriptor-test");

        int quality = 50;
        int jpeg_mode = 1;

        printf("quality:50\n");
        VideoEncSetParameter(p->pEncoder, VENC_IndexParamJpegExifInfo, &exifinfo);
        VideoEncSetParameter(p->pEncoder, VENC_IndexParamJpegQuality, &quality);
        VideoEncSetParameter(p->pEncoder, VENC_IndexParamJpegEncMode, &jpeg_mode);
    }
    else if(p->mEncodeType == VENC_CODEC_H264)
    {
        int value;
        VencH264Param h264Param;
        memset(&h264Param, 0, sizeof(VencH264Param));
        h264Param.bEntropyCodingCABAC = 1;
        if (p->mBitRate > 0)
        {
            h264Param.nBitrate = p->mBitRate;
        }
        else
        {
            h264Param.nBitrate = 150000;    //* 400 kbits.
        }
        h264Param.nCodingMode             = VENC_FRAME_CODING;
        h264Param.nMaxKeyInterval         = 15; //30; //mFrameRate / 1000;
        h264Param.sProfileLevel.nProfile  = VENC_H264ProfileMain;
        h264Param.sProfileLevel.nLevel    = VENC_H264Level31;
        h264Param.sQPRange.nMinqp = 10;
        h264Param.sQPRange.nMaxqp = 40;
        if(p->mFrameRate > 0)
        {
            h264Param.nFramerate = p->mFrameRate;
        }
        else
        {
            h264Param.nFramerate = 30;
        }

        VideoEncSetParameter(p->pEncoder, VENC_IndexParamH264Param, &h264Param);

        value = 0;
        VideoEncSetParameter(p->pEncoder, VENC_IndexParamIfilter, &value);

        value = 0; //degree
        VideoEncSetParameter(p->pEncoder, VENC_IndexParamRotation, &value);

        value = 0;
        VideoEncSetParameter(p->pEncoder, VENC_IndexParamSetPSkip, &value);

        //sAspectRatio.aspect_ratio_idc = 255;
        //sAspectRatio.sar_width = 4;
        //sAspectRatio.sar_height = 3;
        //VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264AspectRatio, &sAspectRatio);

        //value = 1;
        //VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264FastEnc, &value);
    }

    // set vbvSize in c500
    //int vbvSize = 2*1024*1024;
    int vbvSize = 4*1024*1024;
    if(baseConfig.nDstWidth>=1920){
        vbvSize = 4*1024*1024;
    }else{
        vbvSize = 2*1024*1024;
    }
    logd("baseConfig.nDstWidth = %d,vbv size = %d",baseConfig.nDstWidth,vbvSize);

    VideoEncSetParameter(p->pEncoder, VENC_IndexParamSetVbvSize, &vbvSize);

    if(p->mEncodeType == VENC_CODEC_H264){
        int vbvThreshold = (baseConfig.nDstWidth*baseConfig.nDstHeight)/4;
        VideoEncSetParameter(p->pEncoder, VENC_IndexParamSetFrameLenThreshold, &vbvThreshold);
    }

    VideoEncInit(p->pEncoder, &baseConfig);
    if(p->mEncodeType == VENC_CODEC_H264)
    {
         VideoEncGetParameter(p->pEncoder, VENC_IndexParamH264SPSPPS, &p->mPpsInfo);
    }

    if(config->bUsePhyBuf)
    {
        p->mUseAllocInputBuffer = 0;
    }

    if(p->mUseAllocInputBuffer)
    {
        AllocInputBuffer(p->pEncoder, &bufferParam);
    }

    return 0;
}

VideoEncodeComp* VideoEncodeCompCreate()
{
    EncodeCompContex* p;

    p = (EncodeCompContex*)malloc(sizeof(EncodeCompContex));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }
    memset(p, 0, sizeof(*p));


    p->mMessageQueue = AwMessageQueueCreate(4, "VideoEncodeComp");
    if(NULL == p->mMessageQueue)
    {
	logd("message queue create fail!\n");
        free(p);
        return NULL;
    }

    // use the buffer malloc in video decoder as default
    p->mUseAllocInputBuffer = 1;

    p->memops = MemAdapterGetOpsS();
    if(p->memops == NULL)
    {
        loge("MemAdapterGetOpsS failed");
        AwMessageQueueDestroy(p->mMessageQueue);
        free(p);
        return NULL;
    }
    CdcMemOpen(p->memops);

    sem_init(&p->mSemStart, 0, 0);
    sem_init(&p->mSemStop, 0, 0);
    sem_init(&p->mSemPause, 0, 0);
    sem_init(&p->mSemQuit, 0, 0);
    sem_init(&p->mSemReset, 0, 0);
    sem_init(&p->mSemWaitData, 0, 0);
    if(pthread_create(&p->mThreadId, NULL, VideoEncodeThread, p) != 0)
    {
        p->mThreadCreated = 0;
        sem_destroy(&p->mSemStart);
        sem_destroy(&p->mSemStop);
        sem_destroy(&p->mSemPause);
        sem_destroy(&p->mSemQuit);
        sem_destroy(&p->mSemReset);
        sem_destroy(&p->mSemWaitData);
        CdcMemClose(p->memops);
        AwMessageQueueDestroy(p->mMessageQueue);
        free(p);
        return NULL;
    }

    p->mThreadCreated = 1;
    return (VideoEncodeComp*)p;
}

int VideoEncodeCompInit(VideoEncodeComp* v, VideoEncodeConfig* config)
{
    EncodeCompContex* p;
    p = (EncodeCompContex*)v;

    if(setEncodeType(p, config))
    {
        loge("set encode type error");
        return -1;
    }

#if NEED_CONV_NV12
    p->uv_tmp_buffer = (unsigned char*)malloc(config->nSrcWidth*config->nSrcHeight/2);
#endif

    return 0;
}

void VideoEncodeCompDestory(VideoEncodeComp* v)
{
    logd("EncodeCompDestory");
    AwMessage msg;
    EncodeCompContex* p;
    p = (EncodeCompContex*)v;

    if(p->mThreadCreated)
    {
        void* status;

        //* send a quit message to quit the main thread.
        memset(&msg, 0, sizeof(AwMessage));
        msg.messageId = ENC_COMMAND_QUIT;
        msg.params[0] = (uintptr_t)&p->mSemQuit;
        msg.params[1] = msg.params[2] = msg.params[3] = 0;

        AwMessageQueuePostMessage(p->mMessageQueue, &msg);
        SemTimedWait(&p->mSemQuit, -1);
        pthread_join(p->mThreadId, &status);
    }

    if(p->pEncoder)
    {
        VideoEncUnInit(p->pEncoder);
        VideoEncDestroy(p->pEncoder);
        p->pEncoder = NULL;
    }

    if(p->memops)
    {
        CdcMemClose(p->memops);
    }

#if NEED_CONV_NV12
    if(p->uv_tmp_buffer)
    {
        free(p->uv_tmp_buffer);
        p->uv_tmp_buffer = NULL;
    }
#endif

    sem_destroy(&p->mSemStart);
    sem_destroy(&p->mSemStop);
    sem_destroy(&p->mSemPause);
    sem_destroy(&p->mSemQuit);
    sem_destroy(&p->mSemReset);
    sem_destroy(&p->mSemWaitData);

    AwMessageQueueDestroy(p->mMessageQueue);
    free(p);
}

int VideoEncodeCompGetExtradata(VideoEncodeComp *v, unsigned char** buf, unsigned int* length)
{
    EncodeCompContex* p;
    p = (EncodeCompContex*)v;
    *length = 0;
    if(p->mEncodeType == VENC_CODEC_H264)
    {
         VideoEncGetParameter(p->pEncoder, VENC_IndexParamH264SPSPPS, &p->mPpsInfo);
        *length = p->mPpsInfo.nLength;
        *buf = p->mPpsInfo.pBuffer;
    }

    return 0;
}

int VideoEncodeCompSetFrameRate(VideoEncodeComp* v,int32_t framerate)
{
    EncodeCompContex* p;
    p = (EncodeCompContex*)v;
    p->mFrameRate = framerate;

    int ret = -1;
    p->mFrameRate = framerate;
    if (p->pEncoder != NULL)
    {
        ret = VideoEncSetParameter(p->pEncoder, VENC_IndexParamFramerate, (void*)&framerate);
        if (ret != 0)
        {
            loge("VideoEncSetParameter :set VENC_IndexParamFramerate failed ");
        }
    }
    return ret;

    /*
    if (setFrameRateControl(p->mFrameRate/1000, p->mSrcFrameRate/1000,
                &p->mFrameRateCtrl.capture, &p->mFrameRateCtrl.total) >= 0)
    {
        p->mFrameRateCtrl.count = 0;
        p->mFrameRateCtrl.enable = 1;
    }
    else
    {
        p->mFrameRateCtrl.enable = 0;
    }
    return 0;
    */
}

int VideoEncodeCompSetBitRate(VideoEncodeComp* v, int32_t bitrate)
{
    EncodeCompContex *p;
    p = (EncodeCompContex *)v;
    int ret = -1;
    p->mBitRate = bitrate;
    if (p->pEncoder != NULL)
    {
        ret = VideoEncSetParameter(p->pEncoder, VENC_IndexParamBitrate, (void*)&bitrate);
        if (ret != 0)
        {
            loge("VideoEncSetParameter :set VENC_IndexParamBitrate failed ");
        }
    }
    return ret;
}

int VideoEncodeCompScaleDownRatio(VideoEncodeComp* v, int32_t ratio)
{
    EncodeCompContex *p;
    p = (EncodeCompContex *)v;
    int ret = -1;
    if (p->pEncoder != NULL)
    {
        ret = VideoEncSetParameter(p->pEncoder, VENC_IndexParamSetThumbScaler, (void*)&ratio);
        if (ret != 0)
        {
            loge("VideoEncSetParameter :set VENC_IndexParamBitrate failed ");
        }
    }
    return ret;
}

int VideoEncodeCompSetThumbNailAddr(VideoEncodeComp *v,VencThumbInfo *addr)
{
    EncodeCompContex *p;
    p = (EncodeCompContex *)v;
	if(p != NULL)
	{
		p->thInfo.pThumbBuf = addr->pThumbBuf;
		p->thInfo.nThumbSize = addr->nThumbSize;
	}
	else
	{
		printf("set thumb nail addr fail!\n");
		return -1;
	}
	return 0;
}

int VideoEncodeCompSetThumbNailEnable(VideoEncodeComp *v,int32_t enable)
{
    EncodeCompContex *p;
    p = (EncodeCompContex *)v;
	if(p != NULL)
	{
		p->thumbNailEnable = enable;
	}
	else
	{
		printf("set thumb enable fail!\n");
	}
	return 0;
}

int VideoEncodeCompStart(VideoEncodeComp* v)
{
    EncodeCompContex* p;
    AwMessage msg;

    p = (EncodeCompContex*)v;

    logd("start");

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = ENC_COMMAND_START;
    msg.params[0] = (uintptr_t)&p->mSemStart;
    msg.params[1] = (uintptr_t)&p->mStartReply;
    msg.params[2] = msg.params[3] = 0;

    AwMessageQueuePostMessage(p->mMessageQueue, &msg);
    SemTimedWait(&p->mSemStart, -1);
    return (int)p->mStartReply;
}

int VideoEncodeCompStop(VideoEncodeComp* v)
{
    EncodeCompContex* p;
    AwMessage msg;

    p = (EncodeCompContex*)v;
    logd("VideoEncodeCompStop");

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = ENC_COMMAND_STOP;
    msg.params[0] = (uintptr_t)&p->mSemStop;
    msg.params[1] = (uintptr_t)&p->mStopReply;
    msg.params[2] = msg.params[3] = 0;

    AwMessageQueuePostMessage(p->mMessageQueue, &msg);
    sem_post(&p->mSemWaitData);
    SemTimedWait(&p->mSemStop, -1);
    return (int)p->mStopReply;
}

int VideoEncodeCompPause(VideoEncodeComp* v)
{
    EncodeCompContex* p;
    AwMessage msg;

    p = (EncodeCompContex*)v;
    logd("pause");

    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = ENC_COMMAND_PAUSE;
    msg.params[0] = (uintptr_t)&p->mSemPause;
    msg.params[1] = (uintptr_t)&p->mPauseReply;
    msg.params[2] = msg.params[3] = 0;

    AwMessageQueuePostMessage(p->mMessageQueue, &msg);

    SemTimedWait(&p->mSemPause, -1);
    return (int)p->mPauseReply;
}

int VideoEncodeCompReset(VideoEncodeComp* v)
{
    EncodeCompContex* p;
    AwMessage msg;

    p = (EncodeCompContex*)v;
    logd("pause");

    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = ENC_COMMAND_RESET;
    msg.params[0] = (uintptr_t)&p->mSemReset;
    msg.params[1] = (uintptr_t)&p->mResetReply;
    msg.params[2] = msg.params[3] = 0;

    AwMessageQueuePostMessage(p->mMessageQueue, &msg);

    SemTimedWait(&p->mSemReset, -1);
    return (int)p->mResetReply;
}

int VideoEncodeCompInputBuffer(VideoEncodeComp* v, VideoInputBuffer *buf)
{
    EncodeCompContex* p;
    int ret;
    p = (EncodeCompContex*)v;

    if(buf != NULL )
    {
        if((!p->mUseAllocInputBuffer && buf->pAddrPhyC == NULL && buf->pAddrPhyY == NULL) ||
            (p->mUseAllocInputBuffer && buf->pData == NULL))
        {
            loge("input video buffer failed");
            return -1;
        }

        int sizeY = p->mInputBufSizeY;
        int sizeC = p->mInputBufSizeC;
        VencInputBuffer inputBuffer, inputBufferReturn;

        memset(&inputBuffer, 0x00, sizeof(VencInputBuffer));

        if(p->mUseAllocInputBuffer)//the input buf is allocated by video decoder
        {
            if(buf->nLen!= (p->mInputBufSizeY+p->mInputBufSizeC))
            {
                loge("input buf length not right,buf->nLen = %d",buf->nLen);
                return -1;
            }

            //* check used buffer, return it
            if(0==AlreadyUsedInputBuffer(p->pEncoder, &inputBufferReturn))
            {
                ReturnOneAllocInputBuffer(p->pEncoder, &inputBufferReturn);
            }

            ret = GetOneAllocInputBuffer(p->pEncoder, &inputBuffer);
            if(ret < 0)
            {
                loge("GetOneAllocInputBuffer failed");
                return -1;
            }

            inputBuffer.nPts = buf->nPts;
            inputBuffer.nFlag = 0;
            memcpy(inputBuffer.pAddrVirY, buf->pData, sizeY);
            if(sizeC > 0)
                memcpy(inputBuffer.pAddrVirC, buf->pData+sizeY, sizeC);

            inputBuffer.bEnableCorp = 0;
            inputBuffer.sCropInfo.nLeft =  240;
            inputBuffer.sCropInfo.nTop  =  240;
            inputBuffer.sCropInfo.nWidth  =  240;
            inputBuffer.sCropInfo.nHeight =  240;

#if NEED_CONV_NV12
            yu12_nv12(p->mInputWidth, p->mInputHeight, inputBuffer.pAddrVirC, p->uv_tmp_buffer);
#endif

            FlushCacheAllocInputBuffer(p->pEncoder, &inputBuffer);
        }
        else
        {
            inputBuffer.nID = buf->nID;
            inputBuffer.nPts = buf->nPts;
            inputBuffer.nFlag = 0;
            inputBuffer.pAddrPhyY = buf->pAddrPhyY;
            inputBuffer.pAddrPhyC = buf->pAddrPhyC;

            inputBuffer.bEnableCorp = 0;
            inputBuffer.sCropInfo.nLeft =  240;
            inputBuffer.sCropInfo.nTop  =  240;
            inputBuffer.sCropInfo.nWidth  =  240;
            inputBuffer.sCropInfo.nHeight =  240;
        }

        ret = AddOneInputBuffer(p->pEncoder, &inputBuffer);
        if(ret < 0)
        {
            loge(" AddOneInputBuffer failed");
            return -1;
        }

	sem_post(&p->mSemWaitData);
        return 0;

    }else{
        loge("the VideoInputBuffer is null");
        return -1;
    }

}

// we must copy the data in outputBuffer after this function called
int VideoEncodeCompRequestVideoFrame(VideoEncodeComp *v, VencOutputBuffer* outputBuffer)
{
    EncodeCompContex* p;
    int ret;
    p = (EncodeCompContex*)v;

    if(ValidBitstreamFrameNum(p->pEncoder) == 0)
    {
        //logw("valid frame num: 0, cannot get video frame");
        return -1;
    }

    ret = GetOneBitstreamFrame(p->pEncoder, outputBuffer);
    if(ret < 0)
    {
        loge("+++ GetOneBitstreamFrame failed");
        return -1;
    }

    if(p->mCallback)
    {
        p->mCallback(p->mUserData, VIDEO_ENCODE_NOTIFY_ENCODED_BUFFER, outputBuffer);
    }
    return 0;
}

int VideoEncodeCompReturnVideoFrame(VideoEncodeComp *v, VencOutputBuffer* outputBuffer)
{
    EncodeCompContex* p;
    int ret;

    p = (EncodeCompContex*)v;
    ret = FreeOneBitStreamFrame(p->pEncoder, outputBuffer);
    if(ret < 0)
    {
        loge("FreeOneBitStreamFrame fail");
        return -1;
    }
    return 0;
}

int VideoEncodeCompSetCallback(VideoEncodeComp *v, VideoEncodeCallback notifier, void* pUserData)
{
    EncodeCompContex* p;

    p = (EncodeCompContex*)v;

    p->mCallback = notifier;
    p->mUserData = pUserData;
    return 0;
}
