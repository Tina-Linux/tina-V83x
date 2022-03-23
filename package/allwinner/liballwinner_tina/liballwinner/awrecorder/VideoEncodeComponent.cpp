//#define LOG_NDEBUG 0
#define LOG_TAG "VideoEncoderComponent"

#include "log.h"
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

#if (CONFIG_CHIP == OPTION_CHIP_T2 || CONFIG_CHIP == OPTION_CHIP_R8)
#define NEED_CONV_NV12 1
#else
#define NEED_CONV_NV12 0
#endif

typedef struct SoftFrameRateCtrl
{
    int capture;
    int total;
    int count;
    bool enable;
} SoftFrameRateCtrl;


typedef struct EncodeCompContex
{
	pthread_t               mThreadId;
	int                     mThreadCreated;
	AwMessageQueue*		        mMessageQueue;

	int                     mStatus;

	int                     mUseAllocInputBuffer;

	int				    mStartReply;
	int				    mStopReply;
	int				    mPauseReply;
	int				    mResetReply;

	sem_t                   mSemStart;
	sem_t                   mSemStop;
	sem_t                   mSemPause;
	sem_t                   mSemReset;
	sem_t                   mSemQuit;

	VideoEncoder*           pEncoder;
    VENC_CODEC_TYPE         mEncodeType;
    int                     mFrameRate; // *1000
    int                     mBitRate;
    int                     mSrcFrameRate;
    int                     mDesOutWidth;       //destination encode video width and height
    int                     mDesOutHeight;
    int                     mInputWidth;
    int                     mInputHeight;
    VENC_PIXEL_FMT          mInputYuvFormat;

    VencHeaderData          mPpsInfo;    //ENCEXTRADATAINFO_t //spspps info.

    VideoEncodeCallback     mCallback;
    void*                   mUserData;

    int                     mEos;

    SoftFrameRateCtrl       mFrameRateCtrl;

    struct ScMemOpsS*       memops;
#if NEED_CONV_NV12
    unsigned char *uv_tmp_buffer;
#endif
}EncodeCompContex;

#if NEED_CONV_NV12
static int yu12_nv12(unsigned int width, unsigned int height, unsigned char *addr_uv, unsigned char *addr_tmp_uv)
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
			loge("fatal error, audio decode component post message fail.");
			abort();
		}

		return;
	}
}


static void* encodeThread(void* pArg)
{
	EncodeCompContex   *p = (EncodeCompContex*)pArg;
	AwMessage			 msg;
	int				 ret;
	sem_t*				 pReplySem;
	int*					pReplyValue;
	VencInputBuffer inputBufferReturn;

	while(1)
	{
		if(AwMessageQueueGetMessage(p->mMessageQueue, &msg) < 0)
		{
			loge("get message fail.");
			continue;
		}

		pReplySem	= (sem_t*)msg.params[0];
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
			if(p->mUseAllocInputBuffer == 0)
			{
				//* check used buffer, return it
				if(0==AlreadyUsedInputBuffer(p->pEncoder, &inputBufferReturn))
				{
					logv("return this buf(%lu)", inputBufferReturn.nID);
					p->mCallback(p->mUserData, VIDEO_ENCODE_NOTIFY_RETURN_BUFFER, &inputBufferReturn.nID);
				}
			}

			ret = VideoEncodeOneFrame(p->pEncoder);
			if(ret == VENC_RESULT_ERROR)
			{
				loge("encode error");
				p->mCallback(p->mUserData, VIDEO_ENCODE_NOTIFY_ERROR, NULL);
			}
			else if(ret == VENC_RESULT_NO_FRAME_BUFFER)
			{
				logv("encode VENC_RESULT_NO_FRAME_BUFFER");
				usleep(10*1000); // it is not a good idea to do this
				PostEncodeMessage(p->mMessageQueue);
			    continue;
			}
			else if (ret == VENC_RESULT_BITSTREAM_IS_FULL)
			{
				logv("encode VENC_RESULT_BITSTREAM_IS_FULL");
				usleep(10*1000); // it is not a good idea to do this
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
			continue;	//* break the thread.
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
            PostEncodeMessage(p->mMessageQueue);   //* post a decode message to decode the first picture.

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
    p->mInputYuvFormat = config->nInputYuvFormat;
    if(p->mEncodeType != VENC_CODEC_H264 && p->mEncodeType != VENC_CODEC_JPEG)
    {
	loge("connot support this video type, p->mEncodeType(%d)", p->mEncodeType);
        return -1;
    }

    //* create an encoder.
    p->pEncoder = VideoEncCreate(p->mEncodeType);
	if(p->pEncoder == NULL)
	{
        return -1;
	}

	//* init video encoder
    VencBaseConfig baseConfig;

    memset(&baseConfig, 0 ,sizeof(VencBaseConfig));
    baseConfig.nInputWidth  = config->nSrcWidth;
    baseConfig.nInputHeight = config->nSrcHeight;
    baseConfig.nStride      = config->nSrcWidth;
    baseConfig.nDstWidth    = p->mDesOutWidth;
    baseConfig.nDstHeight   = p->mDesOutHeight;
    baseConfig.eInputFormat = config->nInputYuvFormat;
	logd("input yuv format = %d",baseConfig.eInputFormat);
    baseConfig.memops = p->memops;

    memset(&bufferParam, 0 ,sizeof(VencAllocateBufferParam));
    bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
	bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;

    if((baseConfig.eInputFormat>=0)&&(baseConfig.eInputFormat<=3))
    {
        bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
        bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;
    }
    else if((baseConfig.eInputFormat>=4)&&(baseConfig.eInputFormat<=7))
    {
        bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
        bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight;
    }
    else if((baseConfig.eInputFormat>=8)&&(baseConfig.eInputFormat<=11))
    {
        bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight*2;
        bufferParam.nSizeC = 0;
    }
    else if((baseConfig.eInputFormat>=12)&&(baseConfig.eInputFormat<=15))
    {
        bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight*4;
        bufferParam.nSizeC = 0;
    }

#if(CONFIG_CHIP == OPTION_CHIP_C500 )
	bufferParam.nBufferNum = 2;
#else
	bufferParam.nBufferNum = 6;
#endif

	//for jpeg
	if(p->mEncodeType == VENC_CODEC_JPEG)
	{
		EXIFInfo exifinfo;
		exifinfo.ThumbWidth = 176;
		exifinfo.ThumbHeight = 144;

		strcpy((char*)exifinfo.CameraMake,		"allwinner make test");
		strcpy((char*)exifinfo.CameraModel,		"allwinner model test");
		strcpy((char*)exifinfo.DateTime,		"2014:02:21 10:54:05");
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
	        h264Param.nBitrate = 150000;	//* 400 kbits.
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
	#if(CONFIG_CHIP == OPTION_CHIP_C500 )
	int vbvSize = 2*1024*1024;
	#else
	int vbvSize = 4*1024*1024;
	#endif

	VideoEncSetParameter(p->pEncoder, VENC_IndexParamSetVbvSize, &vbvSize);

	VideoEncInit(p->pEncoder, &baseConfig);
	if(p->mEncodeType == VENC_CODEC_H264)
	{
		VideoEncGetParameter(p->pEncoder, VENC_IndexParamH264SPSPPS, &p->mPpsInfo);    //VencHeaderData    spsppsInfo;
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

    sem_init(&p->mSemStart, 0, 0);
    sem_init(&p->mSemStop, 0, 0);
    sem_init(&p->mSemPause, 0, 0);
    sem_init(&p->mSemQuit, 0, 0);
    sem_init(&p->mSemReset, 0, 0);
	p->mMessageQueue = AwMessageQueueCreate(4);

	// use the buffer malloc in this component
	p->mUseAllocInputBuffer = 1;

	p->memops = MemAdapterGetOpsS();
	if(p->memops == NULL)
	{
		loge("MemAdapterGetOpsS failed");
	}
	CdcMemOpen(p->memops);

    if(pthread_create(&p->mThreadId, NULL, encodeThread, p) == 0)
		p->mThreadCreated = 1;
	else
		p->mThreadCreated = 0;

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
        setMessage(&msg, ENC_COMMAND_QUIT, (uintptr_t)&p->mSemQuit);
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

    AwMessageQueueDestroy(p->mMessageQueue);

    free(p);
}

int VideoEncodeCompGetExtradata(VideoEncodeComp *v, unsigned char** buf, unsigned int* length)
{
	EncodeCompContex* p;
	p = (EncodeCompContex*)v;
	*length = 0;
	int ret;
	if(p->mEncodeType == VENC_CODEC_H264)
	{
		ret = VideoEncGetParameter(p->pEncoder, VENC_IndexParamH264SPSPPS, &p->mPpsInfo);    //VencHeaderData    spsppsInfo;
        if(ret == 0)
        {
		*length = p->mPpsInfo.nLength;
		*buf = p->mPpsInfo.pBuffer;
		}
		return ret;
	}

	return 0;
}



int VideoEncodeCompSetFrameRate(VideoEncodeComp* v,int32_t framerate)
{
	EncodeCompContex* p;
	p = (EncodeCompContex*)v;
    p->mFrameRate = framerate;
	if (setFrameRateControl(p->mFrameRate/1000, p->mSrcFrameRate/1000, &p->mFrameRateCtrl.capture, &p->mFrameRateCtrl.total) >= 0)
	{
		p->mFrameRateCtrl.count = 0;
		p->mFrameRateCtrl.enable = true;
	}
	else
	{
		p->mFrameRateCtrl.enable = false;
	}
    return 0;
}

int VideoEncodeCompSetBitRate(VideoEncodeComp* v, int32_t bitrate)
{
	EncodeCompContex *p;
	p = (EncodeCompContex *)v;

    p->mBitRate = bitrate;
    if (p->pEncoder != NULL)
    {
        int ret = VideoEncSetParameter(p->pEncoder, VENC_IndexParamBitrate, (void*)&bitrate);
        if (ret != 0)
        {
            loge("<F:%s, L:%d> fatal error! VideoEncSetParameter(VENC_IndexParamBitrate) fail[%d]", __FUNCTION__, __LINE__, ret);
        }
        //int threshold = p->mDesOutWidth * p->mDesOutHeight;
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
	setMessage(&msg,
			   ENC_COMMAND_START,		//* message id.
			   (uintptr_t)&p->mSemStart,	   //* params[0] = &mSemStart.
			   (uintptr_t)&p->mStartReply);   //* params[1] = &mStartReply.
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
	setMessage(&msg,
			   ENC_COMMAND_STOP,		//* message id.
			   (uintptr_t)&p->mSemStop,	   //* params[0] = &mSemStart.
			   (uintptr_t)&p->mStopReply);   //* params[1] = &mStartReply.
	AwMessageQueuePostMessage(p->mMessageQueue, &msg);
	SemTimedWait(&p->mSemStop, -1);
	return (int)p->mStopReply;
}

int VideoEncodeCompPause(VideoEncodeComp* v)
{
	EncodeCompContex* p;
	AwMessage msg;

	p = (EncodeCompContex*)v;
	logd("pause");

	setMessage(&msg,
	           ENC_COMMAND_PAUSE,
	           (uintptr_t)&p->mSemPause,
	           (uintptr_t)&p->mPauseReply);
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

	setMessage(&msg,
	           ENC_COMMAND_RESET,
	           (uintptr_t)&p->mSemReset,
	           (uintptr_t)&p->mResetReply);
	AwMessageQueuePostMessage(p->mMessageQueue, &msg);

    SemTimedWait(&p->mSemReset, -1);
	return (int)p->mResetReply;
}

int VideoEncodeCompInputBuffer(VideoEncodeComp* v, VideoInputBuffer *buf)
{
	EncodeCompContex* p;
	int ret;
	p = (EncodeCompContex*)v;

	if(buf == NULL )
	{
		if((p->mUseAllocInputBuffer && buf->pAddrPhyC == NULL && buf->pAddrPhyY == NULL) ||
			(!p->mUseAllocInputBuffer && buf->pData == NULL))
		{
			loge("input buffer failed");
			return -1;
		}
	}

	unsigned int sizeY = p->mInputWidth*p->mInputHeight;
	VencInputBuffer inputBuffer, inputBufferReturn;

	memset(&inputBuffer, 0x00, sizeof(VencInputBuffer));

	if(p->mUseAllocInputBuffer)
	{
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

        // YUV420
        if((p->mInputYuvFormat >= 0) && (p->mInputYuvFormat <= 3))
        {
            memcpy(inputBuffer.pAddrVirY, buf->pData, sizeY);
            memcpy(inputBuffer.pAddrVirC, buf->pData+sizeY, sizeY/2);
        }
        else if((p->mInputYuvFormat >= 4) && (p->mInputYuvFormat <= 7))
        {
            // YUV422
            memcpy(inputBuffer.pAddrVirY, buf->pData, sizeY);
            memcpy(inputBuffer.pAddrVirC, buf->pData+sizeY, sizeY);
        }
        else if((p->mInputYuvFormat >= 8) && (p->mInputYuvFormat <= 15))
        {
            memcpy(inputBuffer.pAddrVirY, buf->pData, buf->nLen);
        }

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

        if((p->mInputYuvFormat > 7) && (p->mInputYuvFormat < 12))
        {
            inputBuffer.pAddrPhyY = buf->pAddrPhyY;
        }
        else
        {
            inputBuffer.pAddrPhyY = buf->pAddrPhyY;
            inputBuffer.pAddrPhyC = buf->pAddrPhyC;
        }

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

	return 0;
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
