#define LOG_TAG "recoderdemo"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>

#include <cdx_config.h>
#include <log.h>

#include <CdxQueue.h>
#include <AwPool.h>
#include <CdxBinary.h>
#include <CdxMuxer.h>

#include "memoryAdapter.h"
#include "awencoder.h"

#include "RecoderWriter.h"

#define SAVE_VIDEO_FRAME (0)
#define AUDIO_INPUT (1)
#define VIDEO_INPUT (1)

static const int STATUS_IDEL   = 0;

FILE* inputPCM = NULL;
FILE* inputYUV = NULL;

char pcm_path[128] = {0};
char yuv_path[128] = {0};

int videoEos = 0;
int audioEos = 0;


typedef struct DemoRecoderContext
{
	AwEncoder*       mAwEncoder;
	int callbackData;
	pthread_mutex_t mMutex;

    VideoEncodeConfig videoConfig;
    AudioEncodeConfig audioConfig;


	CdxMuxerT* pMuxer;
	int muxType;
	char pUrl[1024];
	CdxWriterT* pStream;
	char*       pOutUrl;

    unsigned char* extractDataBuff;
    unsigned int extractDataLength;

    pthread_t muxerThreadId ;
    pthread_t audioDataThreadId ;
    pthread_t videoDataThreadId ;
    AwPoolT *pool;
    CdxQueueT *dataQueue;
    int exitFlag ;
	unsigned char* pAddrPhyY;
	unsigned char* pAddrPhyC;
	int     bUsed;

	FILE* fpSaveVideoFrame;
}DemoRecoderContext;

//* a notify callback for AwEncorder.
void NotifyCallbackForAwEncorder(void* pUserData, int msg, void* param)
{
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)pUserData;

    switch(msg)
    {
		case AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_BUFFER:
		{
			int id = *((int*)param);
			if(id == 0)
			{
				pDemoRecoder->bUsed = 0;
				//printf("---- pDemoRecoder->bUsed: %d , %p, pDemoRecoder: %p\n", pDemoRecoder->bUsed, &pDemoRecoder->bUsed, pDemoRecoder);
			}
			break;
		}
        default:
        {
            printf("warning: unknown callback from AwRecorder.\n");
            break;
        }
    }

    return ;
}

int onVideoDataEnc(void *app,CdxMuxerPacketT *buff)
{
    CdxMuxerPacketT *packet = NULL;
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)app;
    if (!buff)
        return 0;

    packet = (CdxMuxerPacketT*)malloc(sizeof(CdxMuxerPacketT));
    packet->buflen = buff->buflen;
    packet->length = buff->length;
    packet->buf = malloc(buff->buflen);
    memcpy(packet->buf, buff->buf, packet->buflen);
    packet->pts = buff->pts;
    packet->type = buff->type;
    packet->streamIndex  = buff->streamIndex;
    packet->duration = buff->duration;

#if SAVE_VIDEO_FRAME
	if(pDemoRecoder->fpSaveVideoFrame)
	{
		fwrite(packet->buf, 1, packet->buflen, pDemoRecoder->fpSaveVideoFrame);
	}
#endif

    CdxQueuePush(pDemoRecoder->dataQueue,packet);
    return 0;
}
int onAudioDataEnc(void *app,CdxMuxerPacketT *buff)
{
    CdxMuxerPacketT *packet = NULL;
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)app;
    if (!buff)
        return 0;
    packet = (CdxMuxerPacketT*)malloc(sizeof(CdxMuxerPacketT));
    packet->buflen = buff->buflen;
    packet->length = buff->length;
    packet->buf = malloc(buff->buflen);
    memcpy(packet->buf, buff->buf, packet->buflen);
    packet->pts = buff->pts;
    packet->type = buff->type;
    packet->streamIndex = buff->streamIndex;

    CdxQueuePush(pDemoRecoder->dataQueue,packet);

    return 0;
}

void* MuxerThread(void *param)
{
    //int ret = 0;
    int i =0;
    CdxMuxerPacketT *mPacket = NULL;
	DemoRecoderContext *p = (DemoRecoderContext*)param;
	RecoderWriterT *rw = NULL;
#if FS_WRITER
	CdxFsCacheMemInfo fs_cache_mem;
	int fs_mode = FSWRITEMODE_DIRECT;
    int fs_cache_size = 512 * 1024;
#endif
    logd("MuxerThread");


	if(p->pUrl)
	{
        if ((p->pStream = CdxWriterCreat()) == NULL)
        {
            loge("CdxWriterCreat() failed");
            return 0;
        }
        rw = (RecoderWriterT*)p->pStream;
        rw->file_mode = FD_FILE_MODE;
        strcpy(rw->file_path, p->pUrl);
        RWOpen(p->pStream);
		p->pMuxer = CdxMuxerCreate(p->muxType, p->pStream);
		if(p->pMuxer == NULL)
		{
			loge("CdxMuxerCreate failed");
			return 0;
		}
		logd("MuxerThread init ok");
	}

	CdxMuxerMediaInfoT mediainfo;
    memset(&mediainfo, 0, sizeof(CdxMuxerMediaInfoT));
	switch (p->audioConfig.nType)
	{
		case AUDIO_ENCODE_PCM_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_PCM_TYPE;
			break;
		case AUDIO_ENCODE_AAC_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_AAC_TYPE;
			break;
		case AUDIO_ENCODE_MP3_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_MP3_TYPE;
			break;
		case AUDIO_ENCODE_LPCM_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_LPCM_TYPE;
			break;
		default:
			loge("unlown audio type(%d)", p->audioConfig.nType);
			break;
	}
#if AUDIO_INPUT
    mediainfo.audioNum = 1;
#endif
#if VIDEO_INPUT
    mediainfo.videoNum = 1;
#endif
    if(p->muxType == CDX_MUXER_AAC)
    {
	mediainfo.videoNum = 0;
    }

	mediainfo.audio.nAvgBitrate = p->audioConfig.nBitrate;
	mediainfo.audio.nBitsPerSample = p->audioConfig.nSamplerBits;
	mediainfo.audio.nChannelNum = p->audioConfig.nOutChan;
	mediainfo.audio.nMaxBitRate = p->audioConfig.nBitrate;
	mediainfo.audio.nSampleRate = p->audioConfig.nOutSamplerate;
	mediainfo.audio.nSampleCntPerFrame = 1024; // aac

	if(p->videoConfig.nType == VIDEO_ENCODE_H264)
		mediainfo.video.eCodeType = VENC_CODEC_H264;
	else if(p->videoConfig.nType == VIDEO_ENCODE_JPEG)
		mediainfo.video.eCodeType = VENC_CODEC_JPEG;
	else
	{
		loge("cannot suppot this video type");
		return 0;
	}

	mediainfo.video.nWidth    = p->videoConfig.nOutWidth;
	mediainfo.video.nHeight   = p->videoConfig.nOutHeight;
	mediainfo.video.nFrameRate = p->videoConfig.nFrameRate;

	logd("******************* mux mediainfo *****************************");
	logd("videoNum                   : %d", mediainfo.videoNum);
	logd("audioNum                   : %d", mediainfo.audioNum);
	logd("videoTYpe                  : %d", mediainfo.video.eCodeType);
	logd("framerate                  : %d", mediainfo.video.nFrameRate);
	logd("width                      : %d", mediainfo.video.nWidth);
	logd("height                     : %d", mediainfo.video.nHeight);
	logd("**************************************************************");

	if(p->pMuxer)
	{
		CdxMuxerSetMediaInfo(p->pMuxer, &mediainfo);
#if FS_WRITER
        memset(&fs_cache_mem, 0, sizeof(CdxFsCacheMemInfo));
        /*
        fs_cache_mem.m_cache_size = 512 * 1024;
        fs_cache_mem.mp_cache = (cdx_int8*)malloc(fs_cache_mem.m_cache_size);
        if (fs_cache_mem.mp_cache == NULL)
        {
            loge("fs_cache_mem.mp_cache malloc failed\n");
            return NULL;
        }
        CdxMuxerControl(p->pMuxer, SET_CACHE_MEM, &fs_cache_mem);
        fs_mode = FSWRITEMODE_CACHETHREAD;
        */

        fs_cache_size = 512 * 1024;
        CdxMuxerControl(p->pMuxer, SET_FS_SIMPLE_CACHE_SIZE, &fs_cache_size);
        fs_mode = FSWRITEMODE_SIMPLECACHE;

        //fs_mode = FSWRITEMODE_DIRECT;
        CdxMuxerControl(p->pMuxer, SET_FS_WRITE_MODE, &fs_mode);
#endif
    }

    logd("extractDataLength %d",p->extractDataLength);
    if(p->extractDataLength > 0 && p->pMuxer)
    {
        logd("demo WriteExtraData");
        if(p->pMuxer)
            CdxMuxerWriteExtraData(p->pMuxer, p->extractDataBuff, p->extractDataLength, 0);
    }

    if(p->pMuxer)
	{
	    logd("write head");
		CdxMuxerWriteHeader(p->pMuxer);
	}

#if AUDIO_INPUT && VIDEO_INPUT
    while ((audioEos==0) || (videoEos==0))
#elif VIDEO_INPUT
    while(videoEos==0)
#elif AUDIO_INPUT
    while(audioEos==0)
#endif
    {
        while (!CdxQueueEmpty(p->dataQueue))
        {
            mPacket = CdxQueuePop(p->dataQueue);
            i++;
            if(p->pMuxer)
            {
                if(CdxMuxerWritePacket(p->pMuxer, mPacket) < 0)
                {
                    loge("+++++++ CdxMuxerWritePacket failed");
                }
            }

            free(mPacket->buf);
            free(mPacket);

        }
        usleep(1*1000);

    }

    if(p->pMuxer)
    {
        logd("write trailer");
        CdxMuxerWriteTrailer(p->pMuxer);
    }

    logd("CdxMuxerClose");
    if(p->pMuxer)
    {
		CdxMuxerClose(p->pMuxer);
        p->pMuxer = NULL;
    }
    if(p->pStream)
    {
        RWClose(p->pStream);
        CdxWriterDestroy(p->pStream);
        p->pStream = NULL;
        rw = NULL;
    }

#if FS_WRITER
    if(fs_cache_mem.mp_cache)
    {
        free(fs_cache_mem.mp_cache);
        fs_cache_mem.mp_cache = NULL;
    }
#endif
    logd("MuxerThread has finish!");
	p->exitFlag  = 1;

    return 0;
}


void* AudioInputThread(void *param)
{
    int ret = 0;
    //int i =0;
	DemoRecoderContext *p = (DemoRecoderContext*)param;

    logd("AudioInputThread");
    int num = 0;
    int size2 = 0;
    int64_t audioPts = 0;

    AudioInputBuffer audioInputBuffer;
    memset(&audioInputBuffer, 0x00, sizeof(AudioInputBuffer));
    audioInputBuffer.nLen = 1024*4; //176400;
    audioInputBuffer.pData = (char*)malloc(audioInputBuffer.nLen);

	while(num<1024)
	{
		ret = -1;
		if(!audioEos)
		{
		    if (inputPCM == NULL)
		    {
		        printf("before fread, inputPCM is NULL\n");
                break;
		    }
			size2 = fread(audioInputBuffer.pData, 1, audioInputBuffer.nLen, inputPCM);
			if(size2 < audioInputBuffer.nLen)
			{
				logd("read error");
				audioEos = 1;
			}

			while(ret)
			{
				audioInputBuffer.nPts = audioPts;
				ret = AwEncoderWritePCMdata(p->mAwEncoder,&audioInputBuffer);
				//logd("=== WritePCMdata audioPts : %lld", audioPts);

				usleep(10*1000);
			}
			usleep(20*1000);
			audioPts += 23;
		}
		num ++;
	}
	logd("audio read data finish!");
	audioEos = 1;

    if(audioInputBuffer.pData)
        free(audioInputBuffer.pData);

    return 0;
}

void* VideoInputThread(void *param)
{
	DemoRecoderContext *p = (DemoRecoderContext*)param;

    logd("VideoInputThread");
    struct ScMemOpsS* memops = NULL;

    VideoInputBuffer videoInputBuffer;
    int sizeY = p->videoConfig.nSrcHeight* p->videoConfig.nSrcWidth;

    if(p->videoConfig.bUsePhyBuf)
    {
        memops = MemAdapterGetOpsS();
	    if(memops == NULL)
	    {
	        loge("memops is NULL");
		    return NULL;
	    }
	    CdcMemOpen(memops);
	p->pAddrPhyY = CdcMemPalloc(memops, sizeY);
		p->pAddrPhyC = CdcMemPalloc(memops, sizeY/2);
		printf("==== palloc demoRecoder.pAddrPhyY: %p\n", p->pAddrPhyY);

		videoInputBuffer.nID = 0;
		fread(p->pAddrPhyY, 1, sizeY,  inputYUV);
		fread(p->pAddrPhyC, 1, sizeY/2, inputYUV);
		CdcMemFlushCache(memops, p->pAddrPhyY, sizeY);
		CdcMemFlushCache(memops, p->pAddrPhyC, sizeY/2);

		videoInputBuffer.nID = 0;
		videoInputBuffer.pAddrPhyY = CdcMemGetPhysicAddressCpu(memops, p->pAddrPhyY);
		videoInputBuffer.pAddrPhyC = CdcMemGetPhysicAddressCpu(memops, p->pAddrPhyC);

    }
    else
    {
	    memset(&videoInputBuffer, 0x00, sizeof(VideoInputBuffer));
	    videoInputBuffer.nLen = p->videoConfig.nSrcHeight* p->videoConfig.nSrcWidth *3/2;
	    videoInputBuffer.pData = (unsigned char*)malloc(videoInputBuffer.nLen);
    }

    unsigned int size1;
    long long videoPts = 0;

	int ret = -1;
	int num = 0;

	while(num<100)
	{
		ret = -1;

		if(p->videoConfig.bUsePhyBuf)
		{
			while(1)
			{
				if(p->bUsed == 0)
				{
					break;
				}
				//printf("==== wait buf return, demoRecoder.bUsed: %d \n", demoRecoder.bUsed);
				usleep(10000);
			}

			videoInputBuffer.nID = 0;
			fread(p->pAddrPhyY, 1, sizeY,  inputYUV);
			fread(p->pAddrPhyC, 1, sizeY/2, inputYUV);
			CdcMemFlushCache(memops, p->pAddrPhyY, sizeY);
			CdcMemFlushCache(memops, p->pAddrPhyC, sizeY/2);


			videoInputBuffer.nID = 0;
			videoInputBuffer.pAddrPhyY = CdcMemGetPhysicAddressCpu(memops, p->pAddrPhyY);
			videoInputBuffer.pAddrPhyC = CdcMemGetPhysicAddressCpu(memops, p->pAddrPhyC);

		}
		else
		{
			size1 = fread(videoInputBuffer.pData, 1, videoInputBuffer.nLen, inputYUV);
		if(size1 < videoInputBuffer.nLen)
		{
			logd("read error");
			videoEos = 1;
		}
		}

	while(ret < 0)
		{
		    videoInputBuffer.nPts = videoPts;
		    p->bUsed = 1;
		    //printf("==== writeYUV used: %d", demoRecoder.bUsed);
			ret = AwEncoderWriteYUVdata(p->mAwEncoder,&videoInputBuffer);

			usleep(10*1000);
		}
		usleep(29*1000);
		videoPts += 30;
		num ++;
	}

    logd("video read data finish!");

	videoEos = 1;

	printf("==== freee demoRecoder.pAddrPhyY: %p\n", p->pAddrPhyY);
	if(p->pAddrPhyY)
	{
		CdcMemPfree(memops, p->pAddrPhyY);
	}
	printf("==== freee demoRecoder.pAddrPhyY  end\n");

    if(p->pAddrPhyC)
	{
		CdcMemPfree(memops, p->pAddrPhyC);
	}
    if (memops)
    {
	    CdcMemClose(memops);
	    memops = NULL;
    }

    if (inputYUV)
    {
        fclose(inputYUV);
        inputYUV = NULL;
    }
    return 0;
}


//* the main method.
int main(int argc, char *argv[])
{
    DemoRecoderContext demoRecoder;

    EncDataCallBackOps mEncDataCallBackOps;
    CdxMuxerPacketT *mPacket = NULL;

    mEncDataCallBackOps.onAudioDataEnc = onAudioDataEnc;
    mEncDataCallBackOps.onVideoDataEnc = onVideoDataEnc;

    printf("\n");
    printf("******************************************************************************************\n");
    printf("* This program implements a simple recoder.\n");
    printf("* Inplemented by Allwinner ALD-AL3 department.\n");
    printf("******************************************************************************************\n");

    if(argc < 6)
    {
	    printf("run failed \n");
	    printf("./recoderdemo argv[1] to argv[5] \n");
	    printf(" argv[1]: video yuv data file \n");
	    printf(" argv[2]: audio pcm file \n");
	    printf(" argv[3]: video enc type: 0:H264 1:JPEG \n");
	    printf(" argv[4]: audio enc type: 0:AAC 2:PCM 3:MP3 \n");
	    printf(" argv[5]: mux type: 0:MP4 1:TS 3:AAC 4:MP3 \n");
	    return -1;
    }
#if VIDEO_INPUT
    inputYUV = fopen(argv[1], "rb");
    logd("fopen inputYUV == %p\n", inputYUV);
    if(inputYUV == NULL)
    {
	printf("open yuv file failed");
	return -1;
    }
#endif
#if AUDIO_INPUT
    inputPCM = fopen(argv[2], "rb");
    logd("fopen inputPCM == %p\n", inputPCM);
    if(inputPCM == NULL)
    {
	printf("open pcm file failed");
	return -1;
    }
#endif

    //* create a demoRecoder.
    memset(&demoRecoder, 0, sizeof(DemoRecoderContext));

    demoRecoder.pool = AwPoolCreate(NULL);
    demoRecoder.dataQueue = CdxQueueCreate(demoRecoder.pool);
#if SAVE_VIDEO_FRAME
    demoRecoder.fpSaveVideoFrame = fopen("/mnt/UDISK/video.dat", "wb");
    if(demoRecoder.fpSaveVideoFrame == NULL)
    {
	printf("open file /mnt/UDISK/video.dat failed, errno(%d)\n", errno);
    }
#endif
	//VideoEncodeConfig videoConfig;
	memset(&demoRecoder.videoConfig, 0x00, sizeof(VideoEncodeConfig));
	demoRecoder.videoConfig.nType       = atoi(argv[3]);
	demoRecoder.videoConfig.nInputYuvFormat = VENC_PIXEL_YUV420P;
	demoRecoder.videoConfig.nFrameRate  = 30;
	demoRecoder.videoConfig.nOutHeight  = 720;
	demoRecoder.videoConfig.nOutWidth   = 1280;
	demoRecoder.videoConfig.nSrcHeight  = 720;
	demoRecoder.videoConfig.nSrcWidth   = 1280;
	demoRecoder.videoConfig.nBitRate    = 3*1000*1000;
	demoRecoder.videoConfig.bUsePhyBuf  = 1;

	//AudioEncodeConfig audioConfig;
	demoRecoder.audioConfig.nType = atoi(argv[4]);
	demoRecoder.audioConfig.nInChan = 2;
	demoRecoder.audioConfig.nInSamplerate = 44100;
	demoRecoder.audioConfig.nOutChan = 2;
	demoRecoder.audioConfig.nOutSamplerate = 44100;
	demoRecoder.audioConfig.nSamplerBits = 16;


    demoRecoder.muxType = atoi(argv[5]);

    if(demoRecoder.muxType == CDX_MUXER_TS && demoRecoder.audioConfig.nType == AUDIO_ENCODE_PCM_TYPE)
    {
	demoRecoder.audioConfig.nFrameStyle = 2;
    }

    if(demoRecoder.muxType == CDX_MUXER_TS && demoRecoder.audioConfig.nType == AUDIO_ENCODE_AAC_TYPE)
    {
	demoRecoder.audioConfig.nFrameStyle = 1;
    }

    if(demoRecoder.muxType == CDX_MUXER_AAC)
    {
	demoRecoder.audioConfig.nType = AUDIO_ENCODE_AAC_TYPE;
	demoRecoder.audioConfig.nFrameStyle = 0;
    }

    if(demoRecoder.muxType == CDX_MUXER_MP3)
    {
	demoRecoder.audioConfig.nType = AUDIO_ENCODE_MP3_TYPE;
    }

    pthread_mutex_init(&demoRecoder.mMutex, NULL);

	demoRecoder.mAwEncoder = AwEncoderCreate(&demoRecoder);
    if(demoRecoder.mAwEncoder == NULL)
    {
        printf("can not create AwRecorder, quit.\n");
        exit(-1);
    }

    //* set callback to recoder.
    AwEncoderSetNotifyCallback(demoRecoder.mAwEncoder,NotifyCallbackForAwEncorder,&(demoRecoder));

	if(demoRecoder.muxType == CDX_MUXER_AAC || demoRecoder.muxType == CDX_MUXER_MP3)
	{
	AwEncoderInit(demoRecoder.mAwEncoder, NULL, &demoRecoder.audioConfig,&mEncDataCallBackOps);
	videoEos = 1;
    }
    else
    {
	AwEncoderInit(demoRecoder.mAwEncoder, &demoRecoder.videoConfig, &demoRecoder.audioConfig,&mEncDataCallBackOps);
    }
    //AwEncoderInit(demoRecoder.mAwEncoder, &demoRecoder.videoConfig, NULL,&mEncDataCallBackOps);

    switch(demoRecoder.muxType)
    {
        case CDX_MUXER_AAC:
            sprintf(demoRecoder.pUrl,  "/mnt/UDISK/save.aac");
            break;
        case CDX_MUXER_MP3:
            sprintf(demoRecoder.pUrl,  "/mnt/UDISK/save.mp3");
            break;
        case CDX_MUXER_TS:
            sprintf(demoRecoder.pUrl,  "/mnt/UDISK/save.ts");
            break;
        default:
            sprintf(demoRecoder.pUrl,  "/mnt/UDISK/save.mp4");
            break;
     }

    AwEncoderStart(demoRecoder.mAwEncoder);

    AwEncoderGetExtradata(demoRecoder.mAwEncoder,&demoRecoder.extractDataBuff,&demoRecoder.extractDataLength);
#if SAVE_VIDEO_FRAME
    if(demoRecoder.fpSaveVideoFrame)
    {
	fwrite(demoRecoder.extractDataBuff, 1, demoRecoder.extractDataLength, demoRecoder.fpSaveVideoFrame);
    }
#endif

#if AUDIO_INPUT
    pthread_create(&demoRecoder.audioDataThreadId, NULL, AudioInputThread, &demoRecoder);
#endif
#if VIDEO_INPUT
    if((demoRecoder.muxType != CDX_MUXER_AAC) && (demoRecoder.muxType != CDX_MUXER_MP3))
    {
	pthread_create(&demoRecoder.videoDataThreadId, NULL, VideoInputThread, &demoRecoder);
    }
#endif

    pthread_create(&demoRecoder.muxerThreadId, NULL, MuxerThread, &demoRecoder);


    while (demoRecoder.exitFlag == 0)
    {
        logd("wait MuxerThread finish!");
        usleep(1000*1000);
    }

	if(demoRecoder.muxerThreadId)
	pthread_join(demoRecoder.muxerThreadId,     NULL);
#if AUDIO_INPUT
    if(demoRecoder.audioDataThreadId)
	pthread_join(demoRecoder.audioDataThreadId, NULL);
#endif
#if VIDEO_INPUT
    if(demoRecoder.videoDataThreadId)
	pthread_join(demoRecoder.videoDataThreadId, NULL);
#endif

	printf("destroy AwRecorder.\n");
    while (!CdxQueueEmpty(demoRecoder.dataQueue))
    {
        logd("free a packet");
        mPacket = CdxQueuePop(demoRecoder.dataQueue);
        free(mPacket->buf);
        free(mPacket);
    }
    CdxQueueDestroy(demoRecoder.dataQueue);
    AwPoolDestroy(demoRecoder.pool);

	if(demoRecoder.mAwEncoder != NULL)
	{
		AwEncoderStop(demoRecoder.mAwEncoder);
	    AwEncoderDestory(demoRecoder.mAwEncoder);
	    demoRecoder.mAwEncoder = NULL;
	}

    pthread_mutex_destroy(&demoRecoder.mMutex);
#if SAVE_VIDEO_FRAME
    if(demoRecoder.fpSaveVideoFrame)
	fclose(demoRecoder.fpSaveVideoFrame);
#endif

#if VIDEO_INPUT
    if (inputYUV)
    {
        fclose(inputYUV);
        inputYUV = NULL;
    }
#endif
#if AUDIO_INPUT
    if (inputPCM)
    {
         fclose(inputPCM);
         inputPCM = NULL;
    }
#endif

    printf("\n");
    printf("******************************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("******************************************************************************************\n");
    printf("\n");

	return 0;
}
