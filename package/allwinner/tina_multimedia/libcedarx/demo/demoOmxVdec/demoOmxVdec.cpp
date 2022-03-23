#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "config.h"
#include "cdx_log.h"
#include "CdxParser.h"
#include "vdecoder.h"
#include "OmxCodec.h"
//#include "layerControl.h"
#include <errno.h>

#define DEMO_PARSER_ERROR (1 << 0)
#define DEMO_DECODER_ERROR (1 << 1)
#define DEMO_DISPLAY_ERROR (1 << 2)
#define DEMO_DECODE_FINISH (1 << 3)
#define DEMO_PARSER_EXIT   (1 << 5)
#define DEMO_DECODER_EXIT  (1 << 6)
#define DEMO_DISPLAY_EXIT  (1 << 7)
#define DEMO_ERROR    ((DEMO_PARSER_ERROR | DEMO_DECODER_ERROR) | DEMO_DISPLAY_ERROR)

typedef struct MultiThreadCtx
{
    pthread_rwlock_t rwrock;
    int nEndofStream;
    int loop;
    int state;
} MultiThreadCtx;

typedef struct DecDemo
{
    AudioDecoder *pAudioDec;
    CdxParserT *parser;
    void* codecCtx;
    //LayerCtrl* lc;
    CdxDataSourceT source;
    CdxMediaInfoT mediaInfo;
    MultiThreadCtx thread;
    int nWidth;
    int nHeight;
    int eCodecFormat;
    char* componentname;
    char* role;
    long long totalTime;
    long long DurationTime;
    int  nDispFrameCount;
    int  nFinishNum;
    int  nDecodeFrameCount;
    pthread_mutex_t parserMutex;
} DecDemo;

typedef struct display
{
    VideoPicture* picture;
    int flag;
    struct display *next;
} display;

static int initDecoder(const char* url, DecDemo *Decoder)
{
    int nRet;
    int bForceExit = 0;
    char tmpUrl[1024];
    struct CdxProgramS *program;
    CdxStreamT *stream = NULL;
    OMX_BUFFERHEADERTYPE* pInputBuffer;


    memset(&Decoder->mediaInfo, 0, sizeof(CdxMediaInfoT));
    if (strlen(url) > 1023)
    {
        logd("url is too long");
        return -1;
    }
    memset(&Decoder->source, 0, sizeof(CdxDataSourceT));
    strcpy(tmpUrl, url);
    Decoder->source.uri = tmpUrl;

    nRet = CdxParserPrepare(&Decoder->source, 0,&Decoder->parserMutex,
                            &bForceExit, &Decoder->parser, &stream, NULL, NULL);
    if(nRet < 0 || Decoder->parser == NULL)
    {
        loge("decoder open parser error");
        return -1;
    }
    nRet = CdxParserGetMediaInfo(Decoder->parser, &Decoder->mediaInfo);
    if(nRet != 0)
    {
        loge("decoder parser get media info error");
        return -1;
    }

    program = &(Decoder->mediaInfo.program[Decoder->mediaInfo.programIndex]);
    Decoder->nWidth = program->video[0].nWidth;
    Decoder->nHeight = program->video[0].nHeight;
    Decoder->eCodecFormat = program->video[0].eCodecFormat;
    logd("eCodecFormat: 0x%x",Decoder->eCodecFormat);
    logd("nFrameRate: %d",program->video[0].nFrameRate);

    switch(Decoder->eCodecFormat)
    {
    case VIDEO_CODEC_FORMAT_H264:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.avc";
        Decoder->role = (char*)"video_decoder.avc";
        break;
    case VIDEO_CODEC_FORMAT_MPEG1:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.mpeg1";
        Decoder->role = (char*)"video_decoder.mpeg1";
        break;
    case VIDEO_CODEC_FORMAT_MPEG2:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.mpeg2";
        Decoder->role = (char*)"video_decoder.mpeg2";
        break;
    case VIDEO_CODEC_FORMAT_MJPEG:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.mjpeg";
        Decoder->role = (char*)"video_decoder.mjpeg";
        break;
    case VIDEO_CODEC_FORMAT_MPEG4:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.mpeg4";
        Decoder->role = (char*)"video_decoder.mpeg4";
        break;
    case VIDEO_CODEC_FORMAT_MSMPEG4V1:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.msmpeg4v1";
        Decoder->role = (char*)"video_decoder.msmpeg4v1";
        break;
    case VIDEO_CODEC_FORMAT_MSMPEG4V2:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.msmpeg4v2";
        Decoder->role = (char*)"video_decoder.msmpeg4v2";
        break;
    case VIDEO_CODEC_FORMAT_DIVX3:
    case VIDEO_CODEC_FORMAT_DIVX4:
    case VIDEO_CODEC_FORMAT_DIVX5:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.divx";
        Decoder->role = (char*)"video_decoder.divx";
        break;
    case VIDEO_CODEC_FORMAT_XVID:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.xvid";
        Decoder->role = (char*)"video_decoder.xvid";
        break;
    case VIDEO_CODEC_FORMAT_H263:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.h263";
        Decoder->role = (char*)"video_decoder.h263";
        break;
    case VIDEO_CODEC_FORMAT_SORENSSON_H263:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.s263";
        Decoder->role = (char*)"video_decoder.s263";
        break;
    case VIDEO_CODEC_FORMAT_RXG2:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.rxg2";
        Decoder->role = (char*)"video_decoder.rxg2";
        break;
    case VIDEO_CODEC_FORMAT_WMV1:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.wmv1";
        Decoder->role = (char*)"video_decoder.wmv1";
        break;
    case VIDEO_CODEC_FORMAT_WMV2:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.wmv2";
        Decoder->role = (char*)"video_decoder.wmv2";
        break;
    case VIDEO_CODEC_FORMAT_WMV3:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.vc1";
        Decoder->role = (char*)"video_decoder.vc1";
        break;
    case VIDEO_CODEC_FORMAT_VP6:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.vp6";
        Decoder->role = (char*)"video_decoder.vp6";
        break;
    case VIDEO_CODEC_FORMAT_VP8:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.vp8";
        Decoder->role = (char*)"video_decoder.vp8";
        break;
    case VIDEO_CODEC_FORMAT_VP9:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.vp9";
        Decoder->role = (char*)"video_decoder.vp9";
        break;
    case VIDEO_CODEC_FORMAT_H265:
        Decoder->componentname = (char*)"OMX.allwinner.video.decoder.hevc";
        Decoder->role = (char*)"video_decoder.hevc";
        break;
    default:
    {
        loge("unsupported pixel format.");
        return -1;
        break;
    }
    }
    Decoder->codecCtx = OmxCodecCreate(Decoder->componentname, Decoder->role);

    //  Omx Codec Configure.
    OmxCodecConfigure(Decoder->codecCtx, OMX_FALSE, Decoder->nWidth,
                      Decoder->nHeight, OMX_COLOR_FormatYUV420Planar);
    OmxCodecStart(Decoder->codecCtx);
    pInputBuffer = dequeneInputBuffer(Decoder->codecCtx);
    pInputBuffer->nFilledLen = program->video[0].nCodecSpecificDataLen;
    memcpy(pInputBuffer->pBuffer, program->video[0].pCodecSpecificData, pInputBuffer->nFilledLen);
    pInputBuffer->nTimeStamp = 0;
    pInputBuffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG;
    pInputBuffer->nOffset = 0;
    queneInputBuffer(Decoder->codecCtx, pInputBuffer);
    pthread_rwlock_init(&Decoder->thread.rwrock, NULL);
    return 0;
}


void *displayPictureThreadFunc(void* param)
{
    DecDemo  *pVideoDecoder = (DecDemo *)param;
    int state, nDispFrameCount;
    OMX_BUFFERHEADERTYPE* pBuffer;

    nDispFrameCount = 0;
    //pVideoDecoder->lc = LayerCreate();
    //VideoPicture* pBuf = (VideoPicture*)malloc(sizeof(VideoPicture));
    //pBuf->nWidth = pVideoDecoder->nWidth;
    //pBuf->nHeight= pVideoDecoder->nHeight;
    //pBuf->ePixelFormat = PIXEL_FORMAT_YV12;

    logd("ePixelFormat: 0x%x",pVideoDecoder->eCodecFormat);
    logd("Display Thread Starting.....");

    while(1)
    {
        usleep(100);
        pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
        state = pVideoDecoder->thread.state;
        pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);

#if 0
        if(state & DEMO_PARSER_EXIT)
        {
            break;
        }
#endif

        if(state & DEMO_ERROR)
        {
            loge("display thread recieve an error singnal,  exit.....");
            goto diplay_exit;
        }

        if(nDispFrameCount >= pVideoDecoder->nFinishNum-1 &&
          (pVideoDecoder->thread.state & DEMO_PARSER_EXIT) )
        {
            loge("decoder get enungh frame, exit ...");
            pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
            pVideoDecoder->thread.state |= DEMO_DECODE_FINISH;
            pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);
            goto diplay_exit;
        }

        pBuffer =  dequeneOutputBuffer(pVideoDecoder->codecCtx);

        if (pBuffer == NULL)
            continue;

        //pBuf->pData0 =  (char *)pBuffer->pBuffer;
        //pBuf->pData1 = (pBuf->pData0 + pBuf->nWidth*pBuf->nHeight);
        //pBuf->pData2 = (pBuf->pData1 + pBuf->nWidth*pBuf->nHeight/4);

        if(pBuffer->pBuffer != NULL)
        {
            logv("decoder get one picture");
            nDispFrameCount++;
            logd("nGetFrameCount=%d,nFinishNum=%d",nDispFrameCount,pVideoDecoder->nFinishNum);
            // omxLayerQueueBuffer(pVideoDecoder->lc,pBuf,1);
            queneOutputBuffer(pVideoDecoder->codecCtx, pBuffer);
        }
    }

    pVideoDecoder->nDispFrameCount += nDispFrameCount;
    pthread_rwlock_wrlock(&pVideoDecoder->thread.rwrock);
    pVideoDecoder->thread.state |= DEMO_DISPLAY_EXIT;
    pthread_rwlock_unlock(&pVideoDecoder->thread.rwrock);
diplay_exit:
    //LayerRelease(pVideoDecoder->lc,0);
    //if(pBuf)
   //     free(pBuf);
    logd("Display Exit.....");
    pthread_exit(NULL);
    return 0;

}

void *parserThreadFunc(void* param)
{
    DecDemo *pDec = (DecDemo *)param;;
    CdxParserT *parser;
    int nRet,   state;
    unsigned char *buf = (unsigned char *)malloc(1024*1024);
    CdxPacketT packet;
    OMX_BUFFERHEADERTYPE* pBuffer;

    if(buf == NULL)
    {
        loge("parser thread malloc error");
        goto parser_exit;
    }

    parser = pDec->parser;
    memset(&packet, 0, sizeof(packet));

    state = 0;
    pDec->nFinishNum = 0;
    nRet = CdxParserPrefetch(parser, &packet);
    while (nRet == 0)
    {
        usleep(30000);
        pthread_rwlock_wrlock(&pDec->thread.rwrock);
        state = pDec->thread.state;
        pthread_rwlock_unlock(&pDec->thread.rwrock);
        if(state & DEMO_ERROR)
        {
            loge("parser receive other thread error. exit flag");
            goto parser_exit;
        }
        if ((packet.type == CDX_MEDIA_VIDEO ) && ((packet.flags & MINOR_STREAM)==0))
        {

            pBuffer =  dequeneInputBuffer(pDec->codecCtx);

            if (pBuffer == NULL)
            {
                nRet = CdxParserPrefetch(parser, &packet);
                continue;
            }


            pBuffer->nFilledLen = packet.length;
            logd("packet.length is %d",packet.length);
            pBuffer->nOffset = 0;
            pBuffer->nFlags = 0;

            packet.buf = pBuffer->pBuffer;
            packet.buflen = packet.length;
        }
        else
        {
            packet.buf = buf;
            packet.buflen = packet.length;
            CdxParserRead(parser, &packet);
            nRet = CdxParserPrefetch(parser, &packet);
            continue;
        }

        nRet = CdxParserRead(parser, &packet);
        if(nRet != 0)
        {
            loge("parser thread read video data error");
            pthread_rwlock_wrlock(&pDec->thread.rwrock);
            pDec->thread.state |= DEMO_PARSER_ERROR;
            pthread_rwlock_unlock(&pDec->thread.rwrock);
            goto parser_exit;
        }
        pDec->nFinishNum ++;

        queneInputBuffer(pDec->codecCtx, pBuffer);
        nRet = CdxParserPrefetch(parser, &packet);
    }

    pthread_rwlock_wrlock(&pDec->thread.rwrock);
    pDec->thread.nEndofStream = 1;
    pDec->thread.state |= DEMO_PARSER_EXIT;
    pthread_rwlock_unlock(&pDec->thread.rwrock);
parser_exit:
    if(buf)
        free(buf);

    pthread_rwlock_wrlock(&pDec->thread.rwrock);
    pDec->thread.nEndofStream = 1;
    pDec->thread.state |= DEMO_PARSER_EXIT;
    pthread_rwlock_unlock(&pDec->thread.rwrock);

    logd("parser exit.....");
    pthread_exit(NULL);
    return 0;
}

void DemoHelpInfo(void)
{
    logd("==== demo help start =====");
    logd("input arg0 = executed file; arg1 = file:///mnt;");
    logd("./demoOmxVdec file:///mnt/xxx.mp4");
    logd("==== demo help end ======");
}

int main(int argc, char** argv)
{
    int nRet = 0;
    pthread_t tparser, tdisplay;
    DecDemo  Decoder;

    DemoHelpInfo();

    memset(&Decoder, 0, sizeof(DecDemo));
    nRet = initDecoder(argv[1], &Decoder);

    if(argc < 2)
    {
        logd("no stream path, arg error");
        return 0;
    }

    if(nRet != 0)
    {
        loge("decoder demom initDecoder error");
        return 0;
    }

    logd("decoder file: %s", argv[1]);
    pthread_create(&tparser, NULL, parserThreadFunc, (void*)(&Decoder));
    pthread_create(&tdisplay, NULL, displayPictureThreadFunc, (void*)(&Decoder));
    pthread_join(tparser, (void**)&nRet);
    pthread_join(tdisplay, (void**)&nRet);
    OmxCodecStop(Decoder.codecCtx);
    CdxParserClose(Decoder.parser);
    logd("demo decoder exit successful");

    return 0;
}
