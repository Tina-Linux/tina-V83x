#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "cdx_config.h"
#include "log.h"
#include "CdxParser.h"
#include "vdecoder.h"
#include "memoryAdapter.h"

#define DEBUG_TIME_INFO 0
#define DEBUG_COST_DRAM_ENABLE 0

#define DRAM_COST_MAX_THREAD_NUM 8
#define FRAME_COUNT 64
#define DISPLAY_PICTUER_LIST_NUM 5
#define DISPLAY_HOLDING_BUFFERS 2
#define DISPLAY_HOLDING_NUM 0
#define DISPLAY_COST_TIME 2

#define DEMO_PARSER_MAX_STREAM_NUM 1024
#define DEMO_FILE_NAME_LEN (2*1024)

#define DEMO_PARSER_ERROR (1 << 0)
#define DEMO_DECODER_ERROR (1 << 1)
#define DEMO_DISPLAY_ERROR (1 << 2)
#define DEMO_DECODE_FINISH (1 << 3)
#define DEMO_PARSER_EXIT   (1 << 5)
#define DEMO_DECODER_EXIT  (1 << 6)
#define DEMO_DISPLAY_EXIT  (1 << 7)
#define DEMO_ERROR	(DEMO_PARSER_ERROR | DEMO_DECODER_ERROR | DEMO_DISPLAY_ERROR)
#define DEMO_EXIT	(DEMO_ERROR | DEMO_DECODE_FINISH)

typedef struct MultiThreadCtx
{
	pthread_rwlock_t rwrock;
	int nEndofStream;
	int loop;
	int state;
}MultiThreadCtx;

typedef struct DecDemo
{
	VideoDecoder *pVideoDec;
	CdxParserT *parser;
	CdxDataSourceT source;
	CdxMediaInfoT mediaInfo;
	MultiThreadCtx thread;
	long long totalTime;
	long long DurationTime;
	int  nDispFrameCount;
	int  nFinishNum;
	int  nDramCostThreadNum;
	int  nDecodeFrameCount;
	char *pInputFile;
	char *pOutputFile;
	pthread_mutex_t parserMutex;
	/* start to save yuv picture,
	 * when decoded picture order >=  nSavePictureStartNumber*/
	int  nSavePictureStartNumber;
	/* the saved picture number */
	int  nSavePictureNumber;
    struct ScMemOpsS* memops;
}DecDemo;

typedef struct display
{
	VideoPicture* picture;
	int flag;
	struct display *next;
}display;

typedef enum
{
    INPUT,
    HELP,
    DECODE_FRAME_NUM,
    SAVE_FRAME_START,
    SAVE_FRAME_NUM,
    SAVE_FRAME_FILE,
    COST_DRAM_THREAD_NUM,
    INVALID
}ARGUMENT_T;


typedef struct
{
    char Short[8];
    char Name[128];
    ARGUMENT_T argument;
    char Description[512];
}argument_t;

static const argument_t ArgumentMapping[] =
{
    { "-h",  "--help",    HELP,
        "Print this help" },
	{ "-i",  "--input",   INPUT,
		"Input file" },
	{ "-n",  "--decode_frame_num",   DECODE_FRAME_NUM,
		"After display n frames, decoder stop" },
	{ "-ss",  "--save_frame_start",  SAVE_FRAME_START,
		"After display ss frames, saving pictures begin" },
	{ "-sn",  "--save_frame_num",  SAVE_FRAME_NUM,
		"After sn frames saved, stop saving picture" },
	{ "-o",  "--save_frame_file",  SAVE_FRAME_FILE,
		"saving picture file" },
	{ "-cn",  "--cost_dram_thread_num",  COST_DRAM_THREAD_NUM,
		"create cn threads to cost dram, or disturb cpu, test decoder robust" },
};

static void PrintDemoUsage(void)
{
    int i = 0;
    int num = sizeof(ArgumentMapping) / sizeof(argument_t);
    logd("Usage:");
    while(i < num)
    {
        logd("%s %-32s  %s", ArgumentMapping[i].Short, ArgumentMapping[i].Name,
			ArgumentMapping[i].Description);
        i++;
    }
}

ARGUMENT_T GetArgument(char *name)
{
    int i = 0;
    int num = sizeof(ArgumentMapping) / sizeof(argument_t);
    while(i < num)
    {
        if((0 == strcmp(ArgumentMapping[i].Name, name)) ||
			((0 == strcmp(ArgumentMapping[i].Short, name)) &&
			 (0 != strcmp(ArgumentMapping[i].Short, "--"))))
        {
            return ArgumentMapping[i].argument;
        }
        i++;
    }
    return INVALID;
}


void ParseArgument(DecDemo *Decoder, char *argument, char *value)
{
    ARGUMENT_T arg;
//    int len = strlen(value);
    int len = 0;
    if(len > DEMO_FILE_NAME_LEN)
	return;
    arg = GetArgument(argument);
    switch(arg)
    {
        case HELP:
		PrintDemoUsage();
            exit(-1);
        case INPUT:
		sprintf(Decoder->pInputFile, "file://");
		sscanf(value, "%2048s", Decoder->pInputFile + 7);
		logd(" get input file: %s ", Decoder->pInputFile);
            break;
        case DECODE_FRAME_NUM:
		sscanf(value, "%d", &Decoder->nFinishNum);
            break;
        case SAVE_FRAME_START:
		sscanf(value, "%d", &Decoder->nSavePictureStartNumber);
            break;
        case SAVE_FRAME_NUM:
		sscanf(value, "%d", &Decoder->nSavePictureNumber);
            break;
        case COST_DRAM_THREAD_NUM:
		sscanf(value, "%d", &Decoder->nDramCostThreadNum);
            break;
        case SAVE_FRAME_FILE:
		sscanf(value, "%2048s", Decoder->pOutputFile);
		logd(" get output file: %s ", Decoder->pOutputFile);
            break;
        case INVALID:
        default:
            logd("unknowed argument :  %s", argument);
            break;
    }
}

static long long GetNowUs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000ll + tv.tv_usec;
}


static int initDecoder(DecDemo *Decoder)
{
	int nRet, i;
	int bForceExit = 0;
	VConfig             VideoConf;
	VideoStreamInfo     VideoInfo;
	struct CdxProgramS *program;
	CdxStreamT *stream = NULL;

	memset(&VideoConf, 0, sizeof(VConfig));
	memset(&VideoInfo, 0, sizeof(VideoStreamInfo));
	memset(&Decoder->mediaInfo, 0, sizeof(CdxMediaInfoT));
    memset(&Decoder->source, 0, sizeof(CdxDataSourceT));

    Decoder->memops = MemAdapterGetOpsS();
    if(Decoder->memops == NULL)
    {
        loge("memops is NULL");
        return -1;
    }
    CdcMemOpen(Decoder->memops);

	logv(" before strcpy(tmpUrl, url) ");
	Decoder->source.uri = Decoder->pInputFile;
	logv(" before CdxParserPrepare() %s", Decoder->source.uri);
	pthread_mutex_init(&Decoder->parserMutex, NULL);
	nRet = CdxParserPrepare(&Decoder->source, 0, &Decoder->parserMutex,
			&bForceExit, &Decoder->parser, &stream, NULL, NULL);
	if(nRet < 0 || Decoder->parser == NULL)
	{
		loge(" decoder open parser error nRet = %d, Decoder->parser: %p", nRet, Decoder->parser);
		return -1;
	}
	logv(" before CdxParserGetMediaInfo() ");
	nRet = CdxParserGetMediaInfo(Decoder->parser, &Decoder->mediaInfo);
	if(nRet != 0)
	{
		loge(" decoder parser get media info error ");
		return -1;
	}
	logv(" before CreateVideoDecoder() ");
	Decoder->pVideoDec = CreateVideoDecoder();
	if(Decoder->pVideoDec == NULL)
	{
		loge(" decoder demom CreateVideoDecoder() error ");
		return -1;
	}
	logv(" before InitializeVideoDecoder() ");
	program = &(Decoder->mediaInfo.program[Decoder->mediaInfo.programIndex]);
	for (i = 0; i < 1; i++)
	{
		VideoStreamInfo *vp = &VideoInfo;
		vp->eCodecFormat = program->video[i].eCodecFormat;
		vp->nWidth = program->video[i].nWidth;
		vp->nHeight = program->video[i].nHeight;
		vp->nFrameRate = program->video[i].nFrameRate;
		vp->nFrameDuration = program->video[i].nFrameDuration;
		vp->nAspectRatio = program->video[i].nAspectRatio;
		vp->bIs3DStream = program->video[i].bIs3DStream;
		vp->nCodecSpecificDataLen = program->video[i].nCodecSpecificDataLen;
		vp->pCodecSpecificData = program->video[i].pCodecSpecificData;
	}
	VideoConf.eOutputPixelFormat  = PIXEL_FORMAT_YV12;

    VideoConf.nDeInterlaceHoldingFrameBufferNum = NUM_OF_PICTURES_KEEPPED_BY_DEINTERLACE;
    VideoConf.nDisplayHoldingFrameBufferNum = NUM_OF_PICTURES_KEEP_IN_LIST;
    VideoConf.nRotateHoldingFrameBufferNum = NUM_OF_PICTURES_KEEPPED_BY_ROTATE;
    VideoConf.nDecodeSmoothFrameBufferNum = NUM_OF_PICTURES_FOR_EXTRA_SMOOTH;
    VideoConf.memops = Decoder->memops;

	nRet = InitializeVideoDecoder(Decoder->pVideoDec, &VideoInfo, &VideoConf);
	logv(" after InitializeVideoDecoder() ");
	if(nRet != 0)
	{
		loge("decoder demom initialize video decoder fail.");
		DestroyVideoDecoder(Decoder->pVideoDec);
		Decoder->pVideoDec = NULL;
	}

	pthread_rwlock_init(&Decoder->thread.rwrock, NULL);

	logd(" initDecoder OK ");
    return 0;
}

static char copyPicture(VideoPicture* pPicture)
{
#if 1
	return 0;
#else
	int nWidth, nHeight;
	int i, j;
	char *pDst, *pSrc, ret;
	ret = 0;
	nHeight = pPicture->nHeight;
	nWidth = pPicture->nWidth;
	pDst = malloc(nWidth * nHeight);
	if(pDst == NULL)
		return ret;
	pSrc = pPicture->pData0;
	for(j = 0; j < nHeight; j++)
		for( i = 0; i < nWidth; i++)
		{
			pDst[j + nHeight + i] = pSrc[j + nHeight + i];
			pDst[i] = pSrc[j + nHeight + i] - pSrc[i];
		}

	pSrc = pPicture->pData1;
	nHeight = pPicture->nHeight / 2;
	nWidth = pPicture->nWidth / 2;
	for(j = 0; j < nHeight; j++)
		for( i = 0; i < nWidth; i++)
		{
			pDst[j + nHeight + i] = pSrc[j + nHeight + i];
			pDst[i] = pSrc[j + nHeight + i] - pSrc[i];
		}

	pSrc = pPicture->pData2;
	nHeight = pPicture->nHeight / 2;
	nWidth = pPicture->nWidth / 2;
	for(j = 0; j < nHeight; j++)
		for( i = 0; i < nWidth; i++)
		{
			pDst[j + nHeight + i] = pSrc[j + nHeight + i];
			pDst[i] = pSrc[j + nHeight + i] - pSrc[i];
		}
	ret = pDst[i];
	free(pDst);
	return ret;
#endif
}

static void addPictureToList(VideoDecoder *pVideoDec, display *pDisList,
		display **h, display **r, VideoPicture* pPicture)
{
	int i;
	display *node = NULL;
	display *DisHeader = *h;
	display *DisRear = *r;
	if(pPicture == NULL || pDisList == NULL)
	{
		loge(" add picuter to list error");
		return;
	}
	for(i = 0; i < DISPLAY_PICTUER_LIST_NUM; i++)
	{
		if(pDisList[i].flag == 0)
		{
			node = &pDisList[i];
			node->flag = 1;
			node->picture = pPicture;
			break;
		}
	}
	if(DisHeader == NULL && DisRear == NULL)
	{
		DisHeader = DisRear = node;
	}
	else
	{
		DisRear->next = node;
		node->next = NULL;
		DisRear = node;
	}

	i = 1;
	node = DisHeader;
	while(node != NULL && node->next != NULL)
	{
		i += 1;
		node = node->next;
	}
	if(i >= DISPLAY_HOLDING_BUFFERS)
	{
		node = DisHeader;
		DisHeader = DisHeader->next;
		node->next = NULL;
		node->flag = 0;
		copyPicture(node->picture);
		usleep(DISPLAY_COST_TIME * 1000); /* displaying one picture use some time */
		logv(" display one picture. pts: %lld ", node->picture->nPts);
		ReturnPicture(pVideoDec, node->picture);
	}
	*h = DisHeader;
	*r = DisRear;
}

static int ReturnAllPicture(VideoDecoder *pVideoDec, display *DisHeader)
{
	int num = 0;
	while(DisHeader)
	{
		ReturnPicture(pVideoDec, DisHeader->picture);
		DisHeader->flag = 0;
		DisHeader = DisHeader->next;
		num += 1;
	}
	return num;
}

void* H265DecodeThread(void* param)
{
	DecDemo  *pVideoDecoder;
	VideoDecoder *pVideoDec;
	int nRet, nStreamNum, i, state;
	int nEndOfStream;

	pVideoDecoder = (DecDemo *)param;
	nEndOfStream = 0;

	pVideoDec = pVideoDecoder->pVideoDec;
	logv(" H265DecodeThread(), thread created ");

	i = 0;
	nStreamNum = VideoStreamFrameNum(pVideoDec, 0);
	while(nStreamNum < 200)
	{
		usleep(2*1000);
		i++;
		if(i > 100)
			break;
		nStreamNum = VideoStreamFrameNum(pVideoDec, 0);
	}
	logv(" data trunk number: %d, i = %d ", nStreamNum, i);

	while(1)
	{
		/* step 1: get stream data */
		usleep(50);
		pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
		nEndOfStream = pVideoDecoder->thread.nEndofStream;
		state = pVideoDecoder->thread.state;
		pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);
		if(state & DEMO_EXIT)
		{
			if(state & DEMO_ERROR)
			{
				loge(" decoer thread recieve an error singnal,  exit..... ");
			}
			if(state & DEMO_DECODE_FINISH)
			{
				logd(" decoer thread recieve a finish singnal,  exit.....  ");
			}
			break;
		}

		logv(" H265DecodeThread(), DecodeVideoStream() start .... ");
		nRet = DecodeVideoStream(pVideoDec, nEndOfStream /*eos*/,
				0/*key frame only*/, 0/*drop b frame*/,
				0/*current time*/);
//		logd(" ------- decoderThread. one frame cost time: %lld ", deltaTime);

		if(nEndOfStream == 1 && nRet == VDECODE_RESULT_NO_BITSTREAM)
		{
			logd(" decoer thread finish decoding.  exit..... ");
			break;
		}
		if(nRet == VDECODE_RESULT_KEYFRAME_DECODED ||
				nRet == VDECODE_RESULT_FRAME_DECODED)
			pVideoDecoder->nDecodeFrameCount++;

		if(nRet < 0)
		{
			loge(" decoder return error. decoder exit ");
			pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
			pVideoDecoder->thread.state |= DEMO_DECODER_ERROR;
			pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);
			break;
		}
	}

	pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
	pVideoDecoder->thread.state |= DEMO_DECODER_EXIT;
	pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);

	logd(" decoder thread exit.... ");
	pthread_exit(NULL);
	return 0;
}


static void savePicture(VideoPicture* pPicture, char *file)
{
	int nSizeY, nSizeUV, nSize;
	FILE *fp = NULL;
	char *pData = NULL;

	if(pPicture == NULL)
	{
		loge(" demo decoder save picture error, picture pointer equals NULL ");
		return;
	}
	fp = fopen(file, "ab");
	if(fp == NULL)
	{
		loge(" demo decoder save picture error, open file fail ");
		return;
	}
	nSizeY = pPicture->nWidth * pPicture->nHeight;
	nSizeUV = nSizeY >> 2;
	logd(" save picture to file: %s, size: %dx%d, top: %d, bottom: %d, left: %d, right: %d",
			file, pPicture->nWidth, pPicture->nHeight,
			pPicture->nTopOffset, pPicture->nBottomOffset, pPicture->nLeftOffset, pPicture->nRightOffset);
	/* save y */
	pData = pPicture->pData0;
	nSize = nSizeY;
	fwrite(pData, 1, nSize, fp);

	/* save u */
	pData = pPicture->pData0 + nSizeY + nSizeUV;
	nSize = nSizeUV;
	fwrite(pData, 1, nSize, fp);

	/* save v */
	pData = pPicture->pData0 + nSizeY;
	nSize = nSizeUV;
	fwrite(pData, 1, nSize, fp);

	fclose(fp);
}

void *displayPictureThreadFunc(void* param)
{
	DecDemo  *pVideoDecoder;
	VideoDecoder *pVideoDec;
	VideoPicture* pPicture;
	display *DisHeader, *DisRear, *pDisList;
	long long nTime, DurationTime;
	int state, nDispFrameCount, nValidPicNum;

	pVideoDecoder = (DecDemo *)param;

	DisHeader = NULL;
	DisRear = NULL;
	nDispFrameCount = 0;
	nValidPicNum = 0;
	DurationTime = 0;
	pVideoDec = pVideoDecoder->pVideoDec;

	pDisList = calloc(DISPLAY_PICTUER_LIST_NUM, sizeof(display));
	if(pDisList == NULL)
	{
		pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
		pVideoDecoder->thread.state |= DEMO_DISPLAY_ERROR;
		pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);
		logd(" display thread calloc memory fial ");
		goto display_thread_exit;
	}
	logd(" display thread starting..... ");
	pVideoDecoder->nDispFrameCount = 0;
	nTime = GetNowUs();
	while(1)
	{
		usleep(100);
		pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
		state = pVideoDecoder->thread.state;
		pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);
		if(state & DEMO_EXIT)
		{
			loge(" display thread recieve an error singnal,  exit..... ");
			break;
		}
		pPicture = RequestPicture(pVideoDec, 0/*the major stream*/);
		if(pPicture != NULL)
		{
			logv(" decoder get one picture, size: %dx%d ", pPicture->nWidth, pPicture->nHeight);
			logd(" picture size: %dx%d ", pPicture->nWidth, pPicture->nHeight);
			nTime = GetNowUs() - nTime;
			DurationTime += nTime;
			nDispFrameCount += 1;

			if(nDispFrameCount >= pVideoDecoder->nSavePictureStartNumber &&
					nDispFrameCount < (pVideoDecoder->nSavePictureStartNumber + pVideoDecoder->nSavePictureNumber))
			{
				logd(" saving picture number: %d ", nDispFrameCount);
				savePicture(pPicture, pVideoDecoder->pOutputFile);
			}

			if(nDispFrameCount >= pVideoDecoder->nFinishNum)
			{
				loge(" display thread get enungh frame, exit ...");
				pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
				pVideoDecoder->thread.state |= DEMO_DECODE_FINISH;
				pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);
				break;
			}
#if DEBUG_TIME_INFO
			if(nDispFrameCount >= FRAME_COUNT)
			{
				float fps, avg;
				pVideoDecoder->nDispFrameCount += nDispFrameCount;
				pVideoDecoder->DurationTime += DurationTime;
				fps = (float)(DurationTime / 1000);
				fps = (nDispFrameCount * 1000) / fps;
				avg = (float)(pVideoDecoder->DurationTime / 1000);
				avg = (pVideoDecoder->nDispFrameCount * 1000) / avg;

				loge(" decoder speed info. current speed: %.2f, average speed: %.2f ", fps, avg);
				DurationTime = 0;
				nDispFrameCount = 0;
			}
#endif
			nTime = GetNowUs();
			addPictureToList(pVideoDec, pDisList, &DisHeader, &DisRear, pPicture);
		}
		else
		{
			pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
			state = pVideoDecoder->thread.state;
			pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);
			if(state & DEMO_DECODER_EXIT)
			{
				nValidPicNum = ValidPictureNum(pVideoDec, 0);
				logv(" display thread find that decode thread had exit ");
				if(nValidPicNum <= 0)
					break;
			}
		}
	}

	pVideoDecoder->nDispFrameCount += nDispFrameCount;
	pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
	pVideoDecoder->thread.state |= DEMO_DISPLAY_EXIT;
	pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);
	ReturnAllPicture(pVideoDec, DisHeader);
	logd(" display thread exit....disp frame num: %d ", pVideoDecoder->nDispFrameCount);
display_thread_exit:
	if(pDisList)
		free(pDisList);
	pthread_exit(NULL);
	return 0;
}

void *parserThreadFunc(void* param)
{
	DecDemo *pDec;
	CdxParserT *parser;
	VideoDecoder *pVideoDec;
	int nRet, nStreamNum, state;
	int nValidSize;
	int nRequestDataSize, trytime;
	unsigned char *buf;
	VideoStreamDataInfo  dataInfo;
	CdxPacketT packet;

	buf = malloc(1024*1024);
	if(buf == NULL)
	{
		loge(" parser thread malloc error ");
		goto parser_exit;
	}
	pDec = (DecDemo *)param;
	pVideoDec = pDec->pVideoDec;
	parser = pDec->parser;
	memset(&packet, 0, sizeof(packet));
	logv(" parserThreadFunc(), thread created ! ");
	state = 0;
	trytime = 0;
	while (0 == CdxParserPrefetch(parser, &packet))
	{
		usleep(50);
		nValidSize = VideoStreamBufferSize(pVideoDec, 0) - VideoStreamDataSize(pVideoDec, 0);
		nRequestDataSize = packet.length;

		pthread_rwlock_wrlock(&pDec->thread.rwrock);
		state = pDec->thread.state;
		pthread_rwlock_unlock(&pDec->thread.rwrock);
		if(state & DEMO_EXIT)
		{
			loge(" hevc parser receive other thread error. exit flag ");
			goto parser_exit;
		}
		if(trytime >= 2000)
		{
			loge("  parser thread trytime >= 2000, maybe some error happen ");
			pthread_rwlock_wrlock(&pDec->thread.rwrock);
			pDec->thread.state |= DEMO_PARSER_ERROR;
			pthread_rwlock_unlock(&pDec->thread.rwrock);
			goto parser_exit;
		}
		if (packet.type == CDX_MEDIA_VIDEO && ((packet.flags&MINOR_STREAM)==0))
		{
			if(nRequestDataSize > nValidSize)
			{
				usleep(50 * 1000);
				trytime++;
				continue;
			}

			nRet = RequestVideoStreamBuffer(pVideoDec,
											nRequestDataSize,
											(char**)&packet.buf,
											&packet.buflen,
											(char**)&packet.ringBuf,
											&packet.ringBufLen,
											0);
			if(nRet != 0)
			{
				logw(" RequestVideoStreamBuffer fail. request size: %d, valid size: %d ",
						nRequestDataSize, nValidSize);
				usleep(50*1000);
				continue;
			}
			if(packet.buflen + packet.ringBufLen < nRequestDataSize)
			{
				loge(" RequestVideoStreamBuffer fail, require size is too small ");
				pthread_rwlock_wrlock(&pDec->thread.rwrock);
				pDec->thread.state |= DEMO_PARSER_ERROR;
				pthread_rwlock_unlock(&pDec->thread.rwrock);
				goto parser_exit;
			}
		}
		else
		{
			packet.buf = buf;
			packet.buflen = packet.length;
			CdxParserRead(parser, &packet);
			continue;
		}
		trytime = 0;
		nStreamNum = VideoStreamFrameNum(pVideoDec, 0);
		if(nStreamNum > DEMO_PARSER_MAX_STREAM_NUM)
		{
			usleep(50*1000);
		}
		nRet = CdxParserRead(parser, &packet);
		if(nRet != 0)
		{
			loge(" parser thread read video data error ");
			pthread_rwlock_wrlock(&pDec->thread.rwrock);
			pDec->thread.state |= DEMO_PARSER_ERROR;
			pthread_rwlock_unlock(&pDec->thread.rwrock);
			goto parser_exit;
		}
		memset(&dataInfo, 0, sizeof(VideoStreamDataInfo));
		dataInfo.pData		  = packet.buf;
		dataInfo.nLength	  = packet.length;
		dataInfo.nPts		  = packet.pts;
		dataInfo.nPcr		  = packet.pcr;
		dataInfo.bIsFirstPart = (!!(packet.flags & FIRST_PART));
		dataInfo.bIsLastPart = (!!(packet.flags & LAST_PART));
		nRet = SubmitVideoStreamData(pVideoDec , &dataInfo, 0);
		if(nRet != 0)
		{
			loge(" parser thread  SubmitVideoStreamData() error ");
			pthread_rwlock_wrlock(&pDec->thread.rwrock);
			pDec->thread.state |= DEMO_PARSER_ERROR;
			pthread_rwlock_unlock(&pDec->thread.rwrock);
			goto parser_exit;
		}
	}

	pthread_rwlock_wrlock(&pDec->thread.rwrock);
	pDec->thread.nEndofStream = 1;
	pDec->thread.state |= DEMO_PARSER_EXIT;
	pthread_rwlock_unlock(&pDec->thread.rwrock);

parser_exit:
	if(buf)
		free(buf);
	logv(" parser exit..... ");
	pthread_exit(NULL);
	return 0;
}


/*
 * DRAMcostTHread() thread make cpu do some other thing, cost DRAM bandwidth
 * */
void *DRAMcostTHread(void *arg)
{
#define MEMORY_STRIDE 1920
#define MEMORY_NUM 8
#define MEMORY_BLOCK (1080*MEMORY_STRIDE)
#define MEMORY_SIZE (MEMORY_BLOCK*MEMORY_NUM)

	DecDemo *pDec;
	//int bExitFlag = 0;
	int i, j;
	int state;
	char *pSrc, *pDst, *p;

	pSrc = NULL;
	pDst = NULL;
	pDec = (DecDemo *)arg;

	pSrc = (char *)malloc(MEMORY_SIZE);
	if(pSrc == NULL)
	{
		logd(" DRAMcostTHread malloc fail ....pSrc ");
		goto DRAM_cost_exit;
	}
	pDst = (char *)malloc(MEMORY_SIZE);
	if(pDst == NULL)
	{
		logd(" DRAMcostTHread malloc fail ....pDst ");
		goto DRAM_cost_exit;
	}
	logd(" DRAM memory copy thread created .... ");
	while(1)
	{
		char *s, *d;
		usleep(100);
		pthread_rwlock_wrlock(&pDec->thread.rwrock);
		//bExitFlag = pDec->thread.nEndofStream ;
		state = pDec->thread.state;
		pthread_rwlock_unlock(&pDec->thread.rwrock);
		if(state & DEMO_DECODER_EXIT)
		{
			logd(" DRAM COST THREAD EIXT..... ");
			break;
		}

		for(j = 0; j < MEMORY_NUM; j++)
		{
			s = pSrc + j*MEMORY_BLOCK;
			d = pDst + j*MEMORY_BLOCK;
			for(i = 0; i < 1080; i++)
			{
				int k;
				p = s;
				for(k = 0; k < MEMORY_NUM*10 && k*j < MEMORY_STRIDE; k++)
					p[k*j] = rand()%128;
				memcpy(d, s, MEMORY_STRIDE);
				s += MEMORY_STRIDE;
				d += MEMORY_STRIDE;
			}
		}
	}
DRAM_cost_exit:
	if(pSrc)
		free(pSrc);
	if(pDst)
		free(pDst);
	pthread_exit(NULL);
	return 0;
}


void DemoHelpInfo(void)
{
	logd(" ==== CedarX linux decoder demo help start ===== ");
	logd(" -h or --help to show the demo usage");
	logd(" demo created by zouwenhuan, allwinnertech/AL3 ");
	logd(" email: zouwenhuan@allwinnertech.com ");
	logd(" ===== CedarX linux decoder demo help end ====== ");
}

int main(int argc, char** argv)
{
	int nRet = 0;
	int i, nDramCostThreadNum;
	char *pInputFile;
	char *pOutputFile;
	pthread_t tdecoder, tparser, tdisplay;
	pthread_t dram[DRAM_COST_MAX_THREAD_NUM];
	DecDemo  Decoder;
	long long endTime;
    AddVDPlugin();

	DemoHelpInfo();

	pInputFile = calloc(DEMO_FILE_NAME_LEN, 1);
	if(pInputFile == NULL)
	{
		loge(" input file. calloc memory fail. ");
		return 0;
	}
	pOutputFile = calloc(DEMO_FILE_NAME_LEN, 1);
	if(pOutputFile == NULL)
	{
		loge(" output file. calloc memory fail. ");
		free(pInputFile);
		return 0;
	}
	memset(&Decoder, 0, sizeof(DecDemo));
	Decoder.nFinishNum = 0x7fffffff;
	Decoder.nDramCostThreadNum = 0;
	Decoder.pInputFile = NULL;
	Decoder.nSavePictureNumber = 0;
	Decoder.nSavePictureStartNumber = 0xffffff;
	Decoder.pInputFile = pInputFile;
	Decoder.pOutputFile = pOutputFile;
    if(argc >= 2)
    {
        for(i = 1; i < (int)argc; i += 2)
        {
			ParseArgument(&Decoder, argv[i], argv[i + 1]);
        }
    }
    else
    {
	logd(" we need more arguments ");
	PrintDemoUsage();
	return 0;
    }

	nRet = initDecoder(&Decoder);
	if(nRet != 0)
	{
		loge(" decoder demom initDecoder error ");
		return 0;
	}
	logd("decoder file: %s", Decoder.pInputFile);


	Decoder.totalTime = GetNowUs();

	nDramCostThreadNum = Decoder.nDramCostThreadNum;

	pthread_create(&tparser, NULL, parserThreadFunc, (void*)(&Decoder));
	pthread_create(&tdecoder, NULL, H265DecodeThread, (void*)(&Decoder));
	pthread_create(&tdisplay, NULL, displayPictureThreadFunc, (void*)(&Decoder));
	for(i = 0; i < nDramCostThreadNum; i++)
	{
		pthread_create(&dram[i], NULL, DRAMcostTHread, (void*)(&Decoder));
		logd(" creat dram memory copy thread[%d] ", i);
	}

	pthread_join(tparser, (void**)&nRet);
	pthread_join(tdecoder, (void**)&nRet);
	pthread_join(tdisplay, (void**)&nRet);
	for(i = 0; i < nDramCostThreadNum; i++)
		pthread_join(dram[i], (void**)&nRet);

	endTime = GetNowUs();
	Decoder.totalTime = endTime - Decoder.totalTime;
	logd(" demoDecoder finish.decode frame: %d, display frame: %d, cost %lld s ",
			Decoder.nDecodeFrameCount, Decoder.nDispFrameCount, Decoder.totalTime/(1000*1000));
	pthread_mutex_destroy(&Decoder.parserMutex);
	CdxParserClose(Decoder.parser);
	logv(" after CdxParserClose()");
	DestroyVideoDecoder(Decoder.pVideoDec);
	if(pInputFile != NULL)
		free(pInputFile);
	if(pOutputFile != NULL)
		free(pOutputFile);
	logd(" demo decoder exit successful");
    CdcMemClose(Decoder.memops);
	return 0;
}
