#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "config.h"
#include "cdx_log.h"
#include "OmxCodec.h"

#include <vdecoder.h>

#include <errno.h>


#define DEMO_PARSER_ERROR  (1 << 0)
#define DEMO_ENCODER_ERROR (1 << 1)
#define DEMO_DISPLAY_ERROR (1 << 2)
#define DEMO_ENCODE_FINISH (1 << 3)
#define DEMO_INTPUT_EXIT   (1 << 5)
#define DEMO_ENCODER_EXIT  (1 << 6)
#define DEMO_OUTPUT_EXIT   (1 << 7)
#define DEMO_ERROR    ((DEMO_PARSER_ERROR | DEMO_ENCODER_ERROR) | DEMO_DISPLAY_ERROR)

typedef struct MultiThreadCtx
{
    pthread_rwlock_t rwrock;
    int nEndofStream;
    int loop;
    int state;
} MultiThreadCtx;

typedef struct EecDemo
{
    void* codecCtx;
    const char *inputFile;
    const char *outputFile;
    MultiThreadCtx thread;
    int nWidth;
    int nHeight;
    int eCodecFormat;
    int nBitrate;
    int nFrameRate;
    char* componentname;
    char* role;
    int  nGetFrameCount;
    int  nFinishNum;
    int  nEncodeFrameCount;
} EecDemo;

int Translate420PlanarTo420SemiPlanar(
    unsigned char *YUVBuffer,
    int width,
    int height)
{
    int size = width * height;
    unsigned char *UBuffer = NULL, *VBuffer = NULL;
    unsigned char *pU = NULL, *pV = NULL, *pMove = NULL, *pEnd = NULL;
    int count;
    if(YUVBuffer == NULL)
    {
        printf("YUVBufrrer is NULL\n");
        return 1;
    }
    UBuffer = (unsigned char*)malloc(size / 4);
    if(UBuffer == NULL)
    {
        printf("malloc UBuffer error\n");
        return 1;
    }
    VBuffer = (unsigned char*)malloc(size / 4);

    if(VBuffer == NULL)
    {
        printf("malloc VBuffer error\n");
        free(UBuffer);
        return 1;
    }

    pU = YUVBuffer + size;
    memcpy(UBuffer, pU, size/4);
    pV = pU + size / 4;
    memcpy(VBuffer, pV, size/4);
    pMove = pU;
    pU = UBuffer;
    pV = VBuffer;
    pEnd=YUVBuffer+size*3/2;

    while(pMove != pEnd)
    {

        *pMove = *pV;
        pMove++;
        *pMove = *pU;
        pMove++;
        pU++;
        pV++;
    }

    pU = pV = pMove = NULL;
    free(UBuffer);
    free(VBuffer);
    UBuffer = VBuffer = NULL;
    return 0;
}


static int initEncoder(const char* inputUrl,const char* outputUrl, EecDemo *Encoder)
{
    int nRet;
    int bForceExit = 0;

    OMX_BUFFERHEADERTYPE* pInputBuffer;

    Encoder->inputFile = inputUrl;
    Encoder->outputFile = outputUrl;

    Encoder->nWidth = 1280;
    Encoder->nHeight = 720;
    Encoder->nFrameRate = 30;              //no use ?
    Encoder->nBitrate = 1024*1024;         //no use ?
    Encoder->eCodecFormat = VIDEO_CODEC_FORMAT_H264;
    logd("eCodecFormat: 0x%x",Encoder->eCodecFormat);

    switch(Encoder->eCodecFormat)
    {
        case VIDEO_CODEC_FORMAT_H264:
            Encoder->componentname = (char*)"OMX.allwinner.video.encoder.avc";
            Encoder->role = (char*)"video_encoder.avc";
            break;
        case VIDEO_CODEC_FORMAT_MJPEG:
            Encoder->componentname = (char*)"OMX.allwinner.video.encoder.mjpeg";
            Encoder->role = (char*)"video_encoder.mjpeg";
            break;
        default:
        {
            loge("unsupported pixel format.");
            return -1;
            break;
        }
    }
    Encoder->codecCtx = OmxCodecCreate(Encoder->componentname, Encoder->role);

    //  Omx Codec Configure.
    OmxCodecConfigure(Encoder->codecCtx, OMX_TRUE,Encoder->nWidth,Encoder->nHeight,
                      Encoder->nFrameRate,Encoder->nBitrate, OMX_COLOR_FormatYUV420Planar);
    OmxCodecStart(Encoder->codecCtx);
    pthread_rwlock_init(&Encoder->thread.rwrock, NULL);
    return 0;
}


void *OutputThreadFunc(void* param)
{
    EecDemo  *pVideoEncoder = (EecDemo *)param;
    int state, nGetFrameCount;
    OMX_BUFFERHEADERTYPE* pBuffer;
    FILE *fOutput = NULL;

    fOutput = fopen(pVideoEncoder->outputFile, "wb");
    if(fOutput == NULL)
    {
        printf( "open OutputFile failed\n");
        goto output_exit;
    }

    nGetFrameCount = 0;

    logd("Output Thread Starting.....");
    while(1)
    {
        usleep(100);
        pthread_rwlock_wrlock(&pVideoEncoder->thread.rwrock);
        state = pVideoEncoder->thread.state;
        pthread_rwlock_unlock(&pVideoEncoder->thread.rwrock);

#if 0
        if(state & DEMO_INTPUT_EXIT)
        {
            break;
        }
#endif

        if(state & DEMO_ERROR)
        {
            loge("display thread recieve an error singnal,  exit.....");
            goto output_exit;
        }

        if(nGetFrameCount >= pVideoEncoder->nFinishNum &&
           (pVideoEncoder->thread.state & DEMO_INTPUT_EXIT) )
        {
            loge("Encoder get enungh frame, exit ...");
            pthread_rwlock_wrlock(&pVideoEncoder->thread.rwrock);
            pVideoEncoder->thread.state |= DEMO_OUTPUT_EXIT;
            pthread_rwlock_unlock(&pVideoEncoder->thread.rwrock);
            goto output_exit;
        }

        pBuffer =  dequeneOutputBuffer(pVideoEncoder->codecCtx);

        if(pBuffer != NULL && pBuffer->pBuffer != NULL)
        {
            fwrite(pBuffer->pBuffer, 1, pBuffer->nFilledLen, fOutput);
            nGetFrameCount++;
            logd("nGetFrameCount=%d,nFinishNum=%d",nGetFrameCount,pVideoEncoder->nFinishNum);
            queneOutputBuffer(pVideoEncoder->codecCtx, pBuffer);
        }

    }

    pVideoEncoder->nGetFrameCount += nGetFrameCount;
    pthread_rwlock_wrlock(&pVideoEncoder->thread.rwrock);
    pVideoEncoder->thread.state |= DEMO_OUTPUT_EXIT;
    pthread_rwlock_unlock(&pVideoEncoder->thread.rwrock);
output_exit:

    if (fOutput)
    {
        fclose(fOutput);
        fOutput = NULL;
    }

    logd("Output Thread Exit.....");
    pthread_exit(0);
    return 0;

}

void *InputThreadFunc(void* param)
{
    EecDemo *pEnc = (EecDemo *)param;
    int nRet,   state;
    FILE *fOutput = NULL, *fInput = NULL;
    int isInputEnd = 0;
    int readSize = (pEnc->nWidth* pEnc->nHeight * 3) / 2;
    unsigned char *buf = (unsigned char *)malloc(readSize);

    fInput = fopen(pEnc->inputFile, "rb");

    if(fInput == NULL)
    {
        printf("open InputFile failed\n");
        goto intput_exit;
    }

    OMX_BUFFERHEADERTYPE* pBuffer;

    state = 0;
    pEnc->nFinishNum = 0;

    logd("Read intput data!");
    nRet = fread(buf, 1, readSize, fInput);

    while (1)
    {
        usleep(30*1000);

        if (nRet < readSize )
        {
            logd("data not engouh!");
            pthread_rwlock_wrlock(&pEnc->thread.rwrock);
            pEnc->thread.state |= DEMO_INTPUT_EXIT;
            pthread_rwlock_unlock(&pEnc->thread.rwrock);
            goto intput_exit;
        }

        if(state & DEMO_ERROR)
        {
            loge("intput thread receive other thread error. exit flag");
            goto intput_exit;
        }

        pthread_rwlock_wrlock(&pEnc->thread.rwrock);
        state = pEnc->thread.state;
        pthread_rwlock_unlock(&pEnc->thread.rwrock);

        if(state & DEMO_ERROR)
        {
            loge("intput thread receive other thread error. exit flag");
            goto intput_exit;
        }

        //convert 420P to 420SP
        /*
        if(Translate420PlanarTo420SemiPlanar(buf, pEnc->nWidth, pEnc->nHeight))
        {
            printf("YUV translate failed\n");
            pthread_rwlock_wrlock(&pEnc->thread.rwrock);
            pEnc->thread.state |= DEMO_INTPUT_EXIT;
            pthread_rwlock_unlock(&pEnc->thread.rwrock);
            goto intput_exit;
        }*/

        pBuffer =  dequeneInputBuffer(pEnc->codecCtx);

        if (pBuffer == NULL)
            continue;

        pBuffer->nFilledLen = readSize;
        pBuffer->nOffset = 0;
        pBuffer->nFlags = 0;
        pBuffer->pBuffer = buf;
        pEnc->nFinishNum ++;

        queneInputBuffer(pEnc->codecCtx, pBuffer);

        logd("Read intput data!");
        nRet = fread(buf, 1, readSize, fInput);
    }

    pthread_rwlock_wrlock(&pEnc->thread.rwrock);
    pEnc->thread.nEndofStream = 1;
    pEnc->thread.state |= DEMO_INTPUT_EXIT;
    pthread_rwlock_unlock(&pEnc->thread.rwrock);
intput_exit:

    logd("intput frame Num %d",pEnc->nFinishNum);

    if(buf)
        free(buf);

    if (fInput)
    {
        fclose(fInput);
        fInput = NULL;
    }

    pthread_rwlock_wrlock(&pEnc->thread.rwrock);
    pEnc->thread.nEndofStream = 1;
    pEnc->thread.state |= DEMO_INTPUT_EXIT;
    pthread_rwlock_unlock(&pEnc->thread.rwrock);

    logd("intput thread exit.....");
    pthread_exit(0);
    return 0;
}

void DemoHelpInfo(void)
{
    logd("==== demo help start =====");
    logd("input arg0 = executed file; arg1 = intfile; arg2 = outfile");
    logd("./demoOmxVenc /mnt/720p.yuv /mnt/save.h264");
    logd("==== demo help end ======");
}

int main(int argc, char** argv)
{
    int nRet = 0;
    pthread_t tparser, tdisplay;
    EecDemo  Encoder;

    DemoHelpInfo();

    if(argc < 3)
    {
        logd("arg error");
        return 0;
    }

    memset(&Encoder, 0, sizeof(EecDemo));
    nRet = initEncoder(argv[1],argv[2], &Encoder);

    if(nRet != 0)
    {
        loge("Encoder demom initEncoder error");
        return 0;
    }

    logd("Encoder file: %s", argv[1]);
    pthread_create(&tparser, NULL, InputThreadFunc, (void*)(&Encoder));
    pthread_create(&tdisplay, NULL, OutputThreadFunc, (void*)(&Encoder));

    pthread_join(tparser, (void**)&nRet);
    pthread_join(tdisplay, (void**)&nRet);
    OmxCodecStop(Encoder.codecCtx);

    logd("demo Encoder exit successful");
    return 0;
}
