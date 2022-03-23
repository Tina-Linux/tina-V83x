/*************************************************************************
    > File Name: decodertest.c
    > Author:
    > Mail:
    > Created Time: 2017/10/15 14:20:17
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <cdx_log.h>
#include "cdx_config.h"
#include "CdxParser.h"
#include "vdecoder.h"
#include "memoryAdapter.h"
//#include <openssl/sha.h>

#define ALIGN_16B(x) (((x) + (15)) & ~(15))

static unsigned char decoderlog = 1;
#define decoderLog if(decoderlog) \
                        printf

#define DEMO_PARSER_MAX_STREAM_NUM 1024

#define DEMO_PARSER_ERROR  (1 << 0)
#define DEMO_DECODER_ERROR (1 << 1)
#define DEMO_DISPLAY_ERROR (1 << 2)
#define DEMO_DECODE_FINISH (1 << 3)
#define DEMO_PARSER_EXIT   (1 << 5)
#define DEMO_DECODER_EXIT  (1 << 6)
#define DEMO_DISPLAY_EXIT  (1 << 7)
#define DEMO_ERROR    (DEMO_PARSER_ERROR | DEMO_DECODER_ERROR | DEMO_DISPLAY_ERROR)
#define DEMO_EXIT     (DEMO_ERROR | DEMO_DECODE_FINISH)

typedef struct MultiThreadCtx
{
    pthread_rwlock_t rwrock;
    int nEndofStream;
    int loop;
    int state;
}MultiThreadCtx;

typedef struct DecDemo
{
	int ScaleDownEn;
    VideoDecoder *pVideoDec;
    CdxParserT *parser;
    CdxDataSourceT source;
    CdxMediaInfoT mediaInfo;
    MultiThreadCtx thread;
    long long totalTime;
    long long DurationTime;
    int  nDecodeFrameCount;
    char *pInputFile;
    char *pOutputFile;
    enum EPIXELFORMAT decode_format;
    pthread_mutex_t parserMutex;
    /* start to save yuv picture,
    * when decoded picture order >=  nSavePictureStartNumber
    */
    int  nSavePictureStartNumber;
    /* the saved picture number */
    int  nSavePictureNumber;
    struct ScMemOpsS* memops;
}DecDemo;

static long long GetNowUs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000ll + tv.tv_usec;
}

int save_frame_to_file(void *str,void *start,int length)
{
    FILE *fp = NULL;

    fp = fopen(str, "wb+");
    if(!fp){
        printf(" Open file error\n");
        return -1;
    }

    if(fwrite(start, length, 1, fp)){
        fclose(fp);
        return 0;
    }else{
        printf(" Write file fail (%s)\n",strerror(errno));
        fclose(fp);
        return -1;
    }

    return 0;
}

void show_help(void)
{
    printf("***********************************************************\n");
    printf(" please enter following instructoins!\n");
    printf(" decodertest argv[1] argv[2] argv[3] argv[4] argv[5]\n");
    printf(" argv[1]: input data path\n");
    printf(" argv[2]: out data path\n");
	printf(" argv[3]: ScaleDownEnable---0:disable 1:enable\n");
    printf(" argv[4]: out data format---NV21 YV12 MB420\n");
    printf(" argv[5]: save picture start number\n");
    printf(" argv[6]: save frame number\n");
    printf("***********************************************************\n");
    printf(" for instance : decodertest /tmp/source.mp4 /tmp 0 NV21 2 10\n");
    printf("***********************************************************\n");
}

enum EPIXELFORMAT return_format(char *short_name)
{
    if (strcmp(short_name, "YV12") == 0) {
        return PIXEL_FORMAT_YV12;
    } else if (strcmp(short_name, "NV21") == 0) {
        return PIXEL_FORMAT_NV21;
	} else if (strcmp(short_name, "MB420") == 0) {
        return PIXEL_FORMAT_YUV_MB32_420;
    } else {
        return 0;
    }
}

char *return_shortname(enum EPIXELFORMAT output_format)
{
    if(output_format == PIXEL_FORMAT_YV12){
        return "YV12";
    }else if(output_format == PIXEL_FORMAT_NV12){
        return "NV12";
    }else if(output_format == PIXEL_FORMAT_NV21){
        return "NV21";
    }else if(output_format == PIXEL_FORMAT_YUV_MB32_420){
        return "MB420";
    }else{
        return NULL;
    }
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
    if(Decoder->memops == NULL){
        printf(" memops is NULL\n");
        return -1;
    }
    CdcMemOpen(Decoder->memops);
    decoderLog(" before strcpy(tmpUrl, url)\n");
    Decoder->source.uri = Decoder->pInputFile;
    decoderLog(" before CdxParserPrepare() %s\n", Decoder->source.uri);
    pthread_mutex_init(&Decoder->parserMutex, NULL);
    nRet = CdxParserPrepare(&Decoder->source, 0, &Decoder->parserMutex,
                            &bForceExit, &Decoder->parser, &stream, NULL, NULL);
    if(nRet < 0 || Decoder->parser == NULL){
        printf(" decoder open parser error nRet = %d, Decoder->parser: %p\n", nRet, Decoder->parser);
        return -1;
    }
    decoderLog(" before CdxParserGetMediaInfo()\n");
    nRet = CdxParserGetMediaInfo(Decoder->parser, &Decoder->mediaInfo);
    if(nRet != 0){
        printf(" decoder parser get media info error\n");
        return -1;
    }
    decoderLog(" before CreateVideoDecoder()\n");
    Decoder->pVideoDec = CreateVideoDecoder();
    if(Decoder->pVideoDec == NULL){
        printf(" decoder demom CreateVideoDecoder() error\n");
        return -1;
    }
    decoderLog(" before InitializeVideoDecoder()\n");
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

	if(Decoder->ScaleDownEn > 0){
		VideoConf.nVbvBufferSize = 1*1024*1024;
		VideoConf.bScaleDownEn = 1;
		VideoConf.nHorizonScaleDownRatio = 1;
		VideoConf.nVerticalScaleDownRatio = 1;
	}

	VideoConf.eOutputPixelFormat  = Decoder->decode_format;

    VideoConf.nDeInterlaceHoldingFrameBufferNum = GetConfigParamterInt("pic_4di_num", 2);
    VideoConf.nDisplayHoldingFrameBufferNum = GetConfigParamterInt("pic_4list_num", 3);
    VideoConf.nRotateHoldingFrameBufferNum = GetConfigParamterInt("pic_4rotate_num", 0);
    VideoConf.nDecodeSmoothFrameBufferNum = GetConfigParamterInt("pic_4smooth_num", 3);
    VideoConf.memops = Decoder->memops;
    nRet = InitializeVideoDecoder(Decoder->pVideoDec, &VideoInfo, &VideoConf);
    decoderLog(" after InitializeVideoDecoder()\n");
    if(nRet != 0){
        printf(" decoder demom initialize video decoder fail\n");
        DestroyVideoDecoder(Decoder->pVideoDec);
        Decoder->pVideoDec = NULL;
        return -1;
    }

    pthread_rwlock_init(&Decoder->thread.rwrock, NULL);

    decoderLog(" initDecoder OK\n");
    return 0;
}

void *parserThreadFunc(void *param)
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
    if(buf == NULL){
        printf(" parser thread malloc error\n");
        goto parser_exit;
    }
    pDec = (DecDemo *)param;
    pVideoDec = pDec->pVideoDec;
    parser = pDec->parser;
    memset(&packet, 0, sizeof(packet));
    decoderLog(" parserThreadFunc(), thread created\n");
    state = 0;
    trytime = 0;

    /* fetch before read parse */
    while (0 == CdxParserPrefetch(parser, &packet))
    {
        usleep(50);

        /* get encoder buffer valid size */
        nValidSize = VideoStreamBufferSize(pVideoDec, 0) - VideoStreamDataSize(pVideoDec, 0);
        nRequestDataSize = packet.length;

        pthread_rwlock_wrlock(&pDec->thread.rwrock);
        state = pDec->thread.state;
        pthread_rwlock_unlock(&pDec->thread.rwrock);
        if(state & DEMO_EXIT){
            printf(" hevc parser receive other thread error. exit flag\n");
            goto parser_exit;
        }else if(state & DEMO_DECODER_EXIT){
            printf(" decoder thread finish. parserThreadFunc() exit flag\n");
            break;
        }

        if(trytime >= 2000){
            printf(" parser thread trytime >= 2000, maybe some error happen\n");
            pthread_rwlock_wrlock(&pDec->thread.rwrock);
            pDec->thread.state |= DEMO_PARSER_ERROR;
            pthread_rwlock_unlock(&pDec->thread.rwrock);
            goto parser_exit;
        }

        if (packet.type == CDX_MEDIA_VIDEO && ((packet.flags&MINOR_STREAM)==0)){
            if(nRequestDataSize > nValidSize){
                usleep(50 * 1000);
                trytime++;
                continue;
            }

            /* get encoder buf */
            nRet = RequestVideoStreamBuffer(pVideoDec,nRequestDataSize,(char**)&packet.buf,
                                           &packet.buflen, (char**)&packet.ringBuf, &packet.ringBufLen, 0);
            if(nRet != 0){
                printf(" RequestVideoStreamBuffer fail. request size: %d, valid size: %d\n", nRequestDataSize, nValidSize);
                usleep(50*1000);
                continue;
            }

            if(packet.buflen + packet.ringBufLen < nRequestDataSize){
                printf(" RequestVideoStreamBuffer fail, require size is too small\n");
                pthread_rwlock_wrlock(&pDec->thread.rwrock);
                pDec->thread.state |= DEMO_PARSER_ERROR;
                pthread_rwlock_unlock(&pDec->thread.rwrock);
                goto parser_exit;
            }
        }
        else{
            packet.buf = buf;
            packet.buflen = packet.length;
            CdxParserRead(parser, &packet);
            continue;
        }

        /* get undecode data buffer number */
        trytime = 0;
        nStreamNum = VideoStreamFrameNum(pVideoDec, 0);
        if(nStreamNum > DEMO_PARSER_MAX_STREAM_NUM){
            usleep(50*1000);
        }

        nRet = CdxParserRead(parser, &packet);
        if(nRet != 0){
            printf(" parser thread read video data error\n");
            pthread_rwlock_wrlock(&pDec->thread.rwrock);
            pDec->thread.state |= DEMO_PARSER_ERROR;
            pthread_rwlock_unlock(&pDec->thread.rwrock);
            goto parser_exit;
        }

        /* video data to decoder */
        memset(&dataInfo, 0, sizeof(VideoStreamDataInfo));
        dataInfo.pData      = packet.buf;
        dataInfo.nLength    = packet.length;
        dataInfo.nPts       = packet.pts;
        dataInfo.nPcr       = packet.pcr;
        dataInfo.bIsFirstPart = (!!(packet.flags & FIRST_PART));
        dataInfo.bIsLastPart = (!!(packet.flags & LAST_PART));
        nRet = SubmitVideoStreamData(pVideoDec , &dataInfo, 0);
        if(nRet != 0){
            printf(" parser thread  SubmitVideoStreamData() error\n");
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
    decoderLog(" parser exit.....\n");
    pthread_exit(NULL);
    return 0;
}

void* DecodeThread(void* param)
{
    DecDemo  *pVideoDecoder;
    VideoDecoder *pVideoDec;
    int nRet, nStreamNum, i, state;
    int nEndOfStream;
	int validNum = 0;
	VideoPicture *videoPicture = NULL;
	int dataLen = 0;
	char *decodeData = NULL;
	char savePath[128];

    pVideoDecoder = (DecDemo *)param;
    nEndOfStream = 0;

    pVideoDec = pVideoDecoder->pVideoDec;
    decoderLog(" DecodeThread(), thread created\n");

	/* get undecode data buffer number */
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
    decoderLog(" data trunk number: %d, i = %d\n", nStreamNum, i);

	decoderLog(" DecodeThread() Decode Video Stream start ....\n");
	pVideoDecoder->nDecodeFrameCount = 0;
    while(1)
    {
        /* get stream data */
        usleep(50);
        pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
        nEndOfStream = pVideoDecoder->thread.nEndofStream;
        state = pVideoDecoder->thread.state;
        pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);
        if(state & DEMO_EXIT){
            if(state & DEMO_ERROR)
                printf(" decoer thread recieve an error singnal,  exit.....\n");

            if(state & DEMO_DECODE_FINISH)
                printf(" decoer thread recieve a finish singnal,  exit.....\n");

            break;
        }

		/* decode stream data */
        nRet = DecodeVideoStream(pVideoDec, nEndOfStream /*eos*/,
                0/*key frame only*/, 0/*drop b frame*/,
                0/*current time*/);

        if(nEndOfStream == 1 && nRet == VDECODE_RESULT_NO_BITSTREAM){
            printf(" decoer thread finish decoding.  exit.....\n");
            break;
        }

        if(nRet == VDECODE_RESULT_KEYFRAME_DECODED ||
				nRet == VDECODE_RESULT_FRAME_DECODED){
            /* get decode data */
			validNum = ValidPictureNum(pVideoDec, 0);

			if(validNum >= 0 &&
                    pVideoDecoder->nSavePictureStartNumber <= pVideoDecoder->nDecodeFrameCount){

                int saveNum = 0;
                videoPicture = RequestPicture(pVideoDec, 0);
                if (videoPicture == NULL) {
                    printf(" RequestPicture fail\n");
                    continue;
                }

				dataLen = videoPicture->nWidth*videoPicture->nHeight*3/2;
                decodeData = (char *)malloc(dataLen);
                if(decodeData == NULL){
                    printf(" malloc decode data error\n");
                    return -1;
                }
                memcpy(decodeData, videoPicture->pData0, videoPicture->nWidth*videoPicture->nHeight);
                memcpy(decodeData + videoPicture->nWidth*videoPicture->nHeight, videoPicture->pData1,videoPicture->nWidth*videoPicture->nHeight/2);

                /* save picture data */
                saveNum = pVideoDecoder->nDecodeFrameCount - pVideoDecoder->nSavePictureStartNumber;
                memset(savePath, 0, sizeof(savePath));
                sprintf(savePath, "%s/decode%s_%d_%d_%d.yuv", pVideoDecoder->pOutputFile,return_shortname(videoPicture->ePixelFormat),
                                                                videoPicture->nWidth, videoPicture->nHeight, saveNum);
                save_frame_to_file(savePath, decodeData, dataLen);
                decoderLog(" save %s finish\n",savePath);

                /* return picture */
                ReturnPicture(pVideoDec, videoPicture);

				free(decodeData);

                if(saveNum >= pVideoDecoder->nSavePictureNumber - 1){
                    decoderLog(" decoder thread finish\n");
                    break;
                }
            }else if(validNum >= 0){
                videoPicture = RequestPicture(pVideoDec, 0);
                if (videoPicture == NULL) {
                    printf(" RequestPicture fail\n");
                    continue;
                }
                ReturnPicture(pVideoDec, videoPicture);
            }

            pVideoDecoder->nDecodeFrameCount++;
        }
        if(nRet < 0){
            printf(" decoder return error. decoder exit\n");
            pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
            pVideoDecoder->thread.state |= DEMO_DECODER_ERROR;
            pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);
            break;
        }
    }

    pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
    pVideoDecoder->thread.state |= DEMO_DECODER_EXIT;
    pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);

    decoderLog(" decoder thread exit....\n");
    pthread_exit(NULL);
    return 0;
}


int main(int argc, char **argv)
{
    int nRet = 0;
    int i, nDramCostThreadNum;
    char pInputFile[128];
    char pOutputFile[128];
    pthread_t tparser, tdecoder;
    DecDemo Decoder;
    long long endTime;

	/* user setting */
    if(argc != 7){
        show_help();
        return -1;
    }

    memset(pInputFile, 0, sizeof(pInputFile));
    memset(pOutputFile, 0, sizeof(pOutputFile));
    if(strlen(argv[1] ) < 110){
        sprintf(pInputFile, "file://%s", argv[1]);
    }
    sprintf(pOutputFile, "%s", argv[2]);

    memset(&Decoder, 0, sizeof(DecDemo));
	Decoder.ScaleDownEn = atoi(argv[3]);
    Decoder.decode_format = return_format(argv[4]);
    if(Decoder.decode_format == 0){
        printf("*************************************************\n");
        printf(" Does not support %s\n", argv[4]);
        printf("*************************************************\n");
        return -1;
    }

    Decoder.nSavePictureStartNumber = atoi(argv[5]);
    Decoder.nSavePictureNumber = atoi(argv[6]);

    Decoder.pInputFile = pInputFile;
    Decoder.pOutputFile = pOutputFile;

    nRet = initDecoder(&Decoder);
    if(nRet != 0){
        decoderLog(" decoder demom initDecoder error\n");
        return -1;
    }
    decoderLog(" decoder input file: %s\n", Decoder.pInputFile);
    decoderLog(" decoder output directory: %s\n", Decoder.pOutputFile);
    decoderLog(" output pixel format: %s\n", return_shortname(Decoder.decode_format));

    Decoder.totalTime = GetNowUs();

    pthread_create(&tparser, NULL, parserThreadFunc, (void*)(&Decoder));
    pthread_create(&tdecoder, NULL, DecodeThread, (void*)(&Decoder));

    pthread_join(tparser, NULL);
    pthread_join(tdecoder, NULL);

    endTime = GetNowUs();
    Decoder.totalTime = endTime - Decoder.totalTime;
    decoderLog(" demoDecoder finish.decode frame: %d, cost %lld s\n",
            Decoder.nDecodeFrameCount, Decoder.totalTime/(1000*1000));
    pthread_mutex_destroy(&Decoder.parserMutex);
    CdxParserClose(Decoder.parser);
    decoderLog(" after CdxParserClose()\n");
    DestroyVideoDecoder(Decoder.pVideoDec);

	CdcMemClose(Decoder.memops);
}
