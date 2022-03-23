/*
 * Copyright (c) 2008-2017 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : audio_exe.c
 * Description : demoAdecoder
 * History :
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "demo_utils.h"
#include "dtsSpeakerMap.h"

#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG "Audio_exe"
#endif

#define DEMO_DEBUG 2

#define DEMO_PARSER_ERROR (1 << 0)
#define DEMO_DECODER_ERROR (1 << 1)
#define DEMO_DSP_ERROR (1 << 2)
#define DEMO_PARSER_FINISH (1 << 3)
#define DEMO_DECODER_FINISH   (1 << 5)
#define DEMO_DSP_FINISH  (1 << 6)
#define DEMO_EXIT    ((DEMO_PARSER_ERROR | DEMO_DECODER_ERROR) | DEMO_DSP_ERROR)

CdxPlaybkCfg initPlybkCfg =
{
    .nRoutine = 0,
    .nNeedDirect = 0,
    .nChannels = 2,
    .nSamplerate = 44100,
    .nBitpersample = 16,
    .nDataType = AUDIO_RAW_DATA_PCM,
};

int demo_stop = 0;

static void demo_close(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
    demo_stop = 1;
}

static void declareMyWork(int print)
{
    if(print)
    {
        demo_print("*******************************************************************************\
                  ******************");
        demo_print("* Wellcome to The Demo of Allwinner Audio Codec");
        demo_print("*         Revision : %s", DEMO_VERSION);
        demo_print("*         Writer   : %s", DEMO_WRITER);
        demo_print("*         Owner    : %s", DEMO_OWNER);
        demo_print("* This Demo is supposed to Tell U to:");
        demo_print("*   a. Init parser and setup a parser thread...");
        demo_print("*   b. Init decoder and setup a decoder thread...");
        demo_print("*   c. Init render and setup a render thread...");
        demo_print("*   d. How to feed bitstream to Audio Mid Layer with its API...");
        demo_print("*   e. How to get a frame buffer from Audio Mid Layer with its API and decode a\
                  frame...");
        demo_print("*   f. How to get pcm or non-pcm-audio data from Audio Mid Layer with its API \
                  and write to Dsp...");
        demo_print("*   g. How to setup the Dsp through alsa driver codec...");
        demo_print("*******************************************************************************\
                  *******************");
    }
}

static void PrintDemoUsage()
{
    demo_print("");
    demo_print(" ^^^^ Demo Tips ^^^^ ");
    demo_print(" demoAdecoder pairs{-[options] -[values]}");
    demo_print(" Usage: ");
    demo_print("     -i/I -value    Indicate input file with directory{value}...");
    demo_print("     -o/O -value    Indicate generating a output file with directory{value}...");
    demo_print("     -h/H           For help...");
    demo_print("             ");
    demo_print(" For example:");
    demo_print("     demoAdecoder -i /mnt/sdcard/Music/fuck-the-police.mp3 -o /data/camera/demo_dec\
              ode.pcm");
}

#define DEMO_FILE_NAME_LEN 256

static void *parserThread(void* param)
{
    DecDemo *Demo;
    CdxParserT *parser;
    AudioDecoder *pAudioDec;
    int nRet, nStreamNum, state;
    int nValidSize;
    int nRequestDataSize, trytime;
    unsigned char *buf;
    AudioStreamDataInfo  dataInfo;
    CdxPacketT packet;

    Demo = (DecDemo *)param;
    pAudioDec = Demo->pAudioDec;
    parser = Demo->parser;
    memset(&packet, 0, sizeof(packet));
    demo_logd(" parserThread(), thread created !");
    state = 0;
    trytime = 0;

    while (0 == CdxParserPrefetch(parser, &packet) && !demo_stop)
    {
        int data_filled = 0;
        usleep(50);
        nValidSize = AudioStreamBufferSize() - AudioStreamDataSize(pAudioDec);
        nRequestDataSize = packet.length;

        pthread_rwlock_wrlock(&Demo->thread.rwrock);
        state = Demo->thread.state;
        pthread_rwlock_unlock(&Demo->thread.rwrock);

        if(state & DEMO_EXIT)
        {
            demo_loge(" parser receive other thread error. exit flag...");
            goto parser_exit;
        }

        if(trytime >= 2000)
        {
            demo_loge("  parser thread trytime >= 2000, maybe some error happen...");
            pthread_rwlock_wrlock(&Demo->thread.rwrock);
            Demo->thread.state |= DEMO_PARSER_ERROR;
            pthread_rwlock_unlock(&Demo->thread.rwrock);
            goto parser_exit;
        }

        if (packet.type == CDX_MEDIA_AUDIO)
        {
            data_filled = 0;

            if(nRequestDataSize > nValidSize)
            {
                usleep(50 * 1000);
                trytime++;
                continue;
            }
            nRet = ParserRequestBsBuffer(pAudioDec,
                                            nRequestDataSize,
                                            (unsigned char**)&packet.buf,
                                            &packet.buflen,
                                            (unsigned char**)&packet.ringBuf,
                                            &packet.ringBufLen,
                                            &data_filled);

            if(nRet != 0)
            {
                demo_logd(" ParserRequestBsBuffer fail. request size: %d, valid size: %d ",
                        nRequestDataSize, nValidSize);
                usleep(50*1000);
                continue;
            }
            if(packet.buflen + packet.ringBufLen < nRequestDataSize)
            {
                demo_loge(" ParserRequestBsBuffer fail, require size is too small ");
                pthread_rwlock_wrlock(&Demo->thread.rwrock);
                Demo->thread.state |= DEMO_PARSER_ERROR;
                pthread_rwlock_unlock(&Demo->thread.rwrock);
                goto parser_exit;
            }
        }

        trytime = 0;

        nRet = CdxParserRead(parser, &packet);

        if(nRet != 0)
        {
            if(CdxParserGetStatus(parser) == PSR_EOS){
                demo_logi(" parser thread read audio data eos ");
                pthread_rwlock_wrlock(&Demo->thread.rwrock);
                Demo->thread.state |= DEMO_PARSER_FINISH;
                pthread_rwlock_unlock(&Demo->thread.rwrock);
            }
            else{
                demo_logi(" parser thread read audio data error ");
                pthread_rwlock_wrlock(&Demo->thread.rwrock);
                Demo->thread.state |= DEMO_PARSER_ERROR;
                pthread_rwlock_unlock(&Demo->thread.rwrock);
            }
            goto parser_exit;
        }

        ParserUpdateBsBuffer(pAudioDec,
                             packet.buflen,
                             packet.pts,
                             data_filled);
        if(nRet != 0)
        {
            demo_loge(" parser thread  ParserUpdateBsBuffer() error ");
            pthread_rwlock_wrlock(&Demo->thread.rwrock);
            Demo->thread.state |= DEMO_PARSER_ERROR;
            pthread_rwlock_unlock(&Demo->thread.rwrock);
            goto parser_exit;
        }
    }

    pthread_rwlock_wrlock(&Demo->thread.rwrock);
    Demo->thread.nEndofStream = 1;
    Demo->thread.state |= DEMO_PARSER_FINISH;
    pthread_rwlock_unlock(&Demo->thread.rwrock);

parser_exit:
    demo_logi(" parser exit...");
    pthread_exit(NULL);
    return 0;
}

static void *decodeThread(void* param)
{
    DecDemo *pDemo;
    AudioDecoder *pAudioDec;
    int ret = 0, nPcmDataLen = 0;
    char* pOutputBuf = NULL;
    int state = 0;

    pDemo = (DecDemo *)param;
    pAudioDec = pDemo->pAudioDec;

    while(!(pDemo->thread.state & DEMO_DECODER_FINISH) && !demo_stop)
    {
        pthread_rwlock_wrlock(&pDemo->thread.rwrock);
        state = pDemo->thread.state;
        pthread_rwlock_unlock(&pDemo->thread.rwrock);
        if(state & DEMO_EXIT)
        {
            demo_loge(" audio decoder receive other thread error. exit flag ");
            goto decoding_exit;
        }

        if (DecRequestPcmBuffer(pAudioDec, &pOutputBuf) < 0)
        {
            //* no pcm buffer, wait for the pcm buffer semaphore.
            demo_logw("no pcm buffer, wait.");
            usleep(20*1000);    //* wait for frame buffer.
            continue;
        }

        ret = DecodeAudioStream(pAudioDec,
                                &pDemo->AudioInfo,
                                pOutputBuf,
                                &nPcmDataLen);
        //fprintf(stderr, "DecodeAudioStream, ret = %d",ret);
        if(ret == ERR_AUDIO_DEC_NONE)
        {
            if(pDemo->AudioInfo.nSampleRate != pDemo->bsInfo.out_samplerate ||
                pDemo->AudioInfo.nChannelNum != pDemo->bsInfo.out_channels)
            {
                pDemo->AudioInfo.nSampleRate = pDemo->bsInfo.out_samplerate;
                pDemo->AudioInfo.nChannelNum = pDemo->bsInfo.out_channels;
            }
            DecUpdatePcmBuffer(pAudioDec, nPcmDataLen);
            continue;
        }
        else if(ret == ERR_AUDIO_DEC_NO_BITSTREAM || ret == ERR_AUDIO_DEC_ABSEND)
        {
            if(pDemo->thread.state & DEMO_PARSER_FINISH)
            {
                demo_logi("audio decoder notify eos...");
                pthread_rwlock_wrlock(&pDemo->thread.rwrock);
                pDemo->thread.state |= DEMO_DECODER_FINISH;
                pthread_rwlock_unlock(&pDemo->thread.rwrock);
                continue;
            }
            else
            {
                usleep(10*1000);
                continue;
            }
        }
        else if(ret == ERR_AUDIO_DEC_EXIT || ret == ERR_AUDIO_DEC_ENDINGCHKFAIL)
        {
            pthread_rwlock_wrlock(&pDemo->thread.rwrock);
            pDemo->thread.state |= DEMO_DECODER_ERROR;
            pthread_rwlock_unlock(&pDemo->thread.rwrock);
            goto decoding_exit;
        }
        else
        {
            demo_logd("DecodeAudioStream() return %d, continue to decode", ret);
            continue;
        }
    }

decoding_exit:
    demo_logd(" decoding exit...");
    pthread_exit(NULL);
    return 0;

}

static void *renderThread(void* param)
{
    unsigned char *pPcmData = NULL;;
    int nPcmDataLen;
    DecDemo *pDemo;
    AudioDecoder *pAudioDec;
    int ret = 0, nWritten = 0;
    int state = 0;
    int raw_flag = 0;
    int nFrameSize = 0, nSampleRate = 0, nBitpersample = 0, nChannels = 0;
    int64_t pDts = 0;
    int tryopenpcmdev = 0;
    pDemo = (DecDemo *)param;
    pAudioDec = pDemo->pAudioDec;

    nSampleRate = pDemo->dsp->spr;
    nBitpersample = pDemo->dsp->bps;
    nChannels = pDemo->dsp->ch;
    nFrameSize  = nChannels * nBitpersample / 8;
    raw_flag  = pDemo->dsp->raw_flag;

    while(!(pDemo->thread.state & DEMO_DSP_FINISH) && !demo_stop){
        /* Part 1
              *    If the demo has occured exit flag, render module exit at once...
        */
        pthread_rwlock_wrlock(&pDemo->thread.rwrock);
        state = pDemo->thread.state;
        pthread_rwlock_unlock(&pDemo->thread.rwrock);
        if(state & DEMO_EXIT)
        {
            demo_loge(" audio decoder receive other thread error. exit flag ");
            goto rendering_exit;
        }

        /* Part 2
              *    This part below is to:
              *        1. Get the audio parameters such as sample per second and channel number
              *    Tips:
              *        pDemo->AudioInfo has been filled in the initialising of demo
              *        --- after parser prepared, before the dsp module setuped ---, for dsp's init-
              *        ialization need AudioInfo. The information of AudioInfo would be uncorrect -
              *        to the actially situation at the very beginning, but it will be refactored -
              *        later...
        */
        nSampleRate = pDemo->AudioInfo.nSampleRate;
        nChannels   = pDemo->AudioInfo.nChannelNum;

        /*  Part 3
        *    This part below is to:
              *        To request pcm data of 20ms first
        */
        nPcmDataLen = (nSampleRate * 20 / 1000) * nFrameSize;
        pPcmData    = NULL;

        /*  Part 4
           *    This part below is to:
              *        Call the Audio Mid Layer API to get the pts and decoded pcm buffer, if it
              *        exist...
              *        Even get the passthru infomation strcut in the passthru mode.
        */
        memset(&pDemo->cfg, 0x00, sizeof(CdxPlaybkCfg));
        pDts = PlybkRequestPcmPts(pAudioDec);
        memcpy(&pDemo->cfg,&(pDemo->AudioInfo.raw_data),sizeof(CdxPlaybkCfg));
        ret = PlybkRequestPcmBuffer(pAudioDec, (unsigned char **)&pPcmData, CdxInt32PtrTypeCheck(&nPcmDataLen));
        // 4.1 Check the para...
        if(ret < 0 || pPcmData == NULL || nPcmDataLen < nFrameSize)
        {
            demo_logd("err: audio render, ret = %d, pPcmData = %p, nPcmDataLen=%d, %d",
                ret, pPcmData, nPcmDataLen, nFrameSize);

            pthread_rwlock_wrlock(&pDemo->thread.rwrock);
            state = pDemo->thread.state;
            pthread_rwlock_unlock(&pDemo->thread.rwrock);
            if(state & DEMO_DECODER_FINISH)
            {
                demo_logi("audio render notify eos...");
                DspStop(pDemo->dsp);
                demo_logd("DspStop...");
                pthread_rwlock_wrlock(&pDemo->thread.rwrock);
                pDemo->thread.state |= DEMO_DSP_FINISH;
                pthread_rwlock_unlock(&pDemo->thread.rwrock);
                demo_logd("rwrock unlock...");
            }

            //* if no pcm data, wait some time.
            usleep(10*1000);
            continue;
        }

        // 4.2 If direct mode(non-pcm-audio) occured, setup the passthru configuration
        if(pDemo->cfg.nNeedDirect != 0)
        {
            //Direct out put start
            pDemo->AudioInfo.nBitsPerSample = nBitpersample = pDemo->cfg.nBitpersample;
            pDemo->AudioInfo.nChannelNum = nChannels = pDemo->cfg.nChannels;
            pDemo->AudioInfo.nSampleRate = nSampleRate = pDemo->cfg.nSamplerate;
            raw_flag = pDemo->cfg.nDataType;
        }

        /*  Part 5
             *  This part below is to:
             *      Switch the dsp configuration , if it changed...
             */
        if ((nSampleRate != pDemo->dsp->spr) ||
            (nChannels != pDemo->dsp->ch) ||
            (nBitpersample != pDemo->dsp->bps) ||
            (raw_flag != pDemo->dsp->raw_flag))
        {
            demo_logd("sample rate change from %d to %d.",
                    pDemo->dsp->spr, nSampleRate);
            demo_logd("channel num change from %d to %d.",
                    pDemo->dsp->ch, nChannels);
            demo_logd("bitPerSample num change from %d to %d.",
                    pDemo->dsp->bps, nBitpersample);
            demo_logd("data type change from %d to %d.",
                    pDemo->dsp->raw_flag, raw_flag);
            ret = DspStop(pDemo->dsp);

            pDemo->dsp->bps = nBitpersample;
            pDemo->dsp->spr = nSampleRate;
            pDemo->dsp->ch = nChannels;
            nFrameSize  = ((nChannels * nBitpersample)>>3);
            pDemo->dsp->raw_flag  = raw_flag;
        }

        /*  Part 6
             *  This part below is to:
             *    Write the pcm or non-pcm-audio to the dsp...
             *    Tips:
             *       If dsp write successfully, Call the Audio Mid Layer API to release the pcm buf-
             *    fer you required previously...
             *       If dsp write failed, try to give dsp few chances to write utils it has finished
             *    . Exit rendering thread while it exceed a setting times.
             */
        tryopenpcmdev = 0;
        while(nPcmDataLen > 0)
        {
            nWritten = DspWrite(pDemo->dsp, pPcmData, nPcmDataLen);
            if(nWritten > 0)
            {
                nPcmDataLen -= nWritten;
                pPcmData    += nWritten;
                PlybkUpdatePcmBuffer(pAudioDec, nWritten);
            }
            else
            {
                //* dsp write failed, wait some time.
                demo_loge("DspWrite error occurs...");
                usleep(10*1000);
                if(nWritten == 0)
                {
                    int waitms = nPcmDataLen * 8000/ (nBitpersample*nChannels)/nSampleRate;
                    demo_logd("dsp write return 0... dev is full, wait %d/4 ms", waitms / 4);
                    DspWaitForDevConsume(waitms / 4);
                    continue;
                }
                tryopenpcmdev++;
                if(tryopenpcmdev > 19){
                    demo_loge("error happens even we wait for as long as 200ms...");
                    pthread_rwlock_wrlock(&pDemo->thread.rwrock);
                    pDemo->thread.state |= DEMO_DSP_ERROR;
                    pthread_rwlock_unlock(&pDemo->thread.rwrock);
                    goto rendering_exit;
                }
            }
        }
    }

rendering_exit:
    demo_logi(" rendering exit..... ");
    pthread_exit(NULL);
    return 0;

}

static int initDemo(DecDemo *Demo)
{
    int nRet, i;
    int bForceExit = 0;

    struct CdxProgramS *program;
    CdxStreamT *stream = NULL;

    memset(&Demo->AudioInfo, 0, sizeof(AudioStreamInfo));
    memset(&Demo->mediaInfo, 0, sizeof(CdxMediaInfoT));
    memset(&Demo->source, 0, sizeof(CdxDataSourceT));

    Demo->source.uri = Demo->pInputFile;
    demo_logd(" before CdxParserPrepare() %s", Demo->source.uri);
    pthread_mutex_init(&Demo->parserMutex, NULL);
    nRet = CdxParserPrepare(&Demo->source, 0, &Demo->parserMutex,
            &bForceExit, &Demo->parser, &stream, NULL, NULL);

    if(nRet < 0 || Demo->parser == NULL)
    {
        demo_loge(" decoder open parser error nRet = %d, Decoder->parser: %p", nRet, Demo->parser);
        return -1;
    }

    nRet = CdxParserGetMediaInfo(Demo->parser, &Demo->mediaInfo);

    if(nRet != 0)
    {
        return -1;
    }
    Demo->pAudioDec = CreateAudioDecoder();

    if(Demo->pAudioDec == NULL)
    {
        demo_loge(" decoder demom CreateAudioDecoder() error ");
        return -1;
    }

    program = &(Demo->mediaInfo.program[Demo->mediaInfo.programIndex]);

    for (i = 0; i < 1; i++)
    {
        AudioStreamInfo *ap = &Demo->AudioInfo;
        ap->eCodecFormat = program->audio[i].eCodecFormat;
        ap->eSubCodecFormat = program->audio[i].eSubCodecFormat;
        ap->nChannelNum = program->audio[i].nChannelNum;
        ap->nBitsPerSample = program->audio[i].nBitsPerSample;
        ap->nSampleRate = program->audio[i].nSampleRate;
        ap->nCodecSpecificDataLen = program->audio[i].nCodecSpecificDataLen;
        ap->pCodecSpecificData = program->audio[i].pCodecSpecificData;
    }

    memset(&Demo->bsInfo, 0, sizeof(BsInFor));
    nRet = InitializeAudioDecoder(Demo->pAudioDec, &Demo->AudioInfo, &Demo->bsInfo);
    if(nRet != 0)
    {
        demo_loge("decoder demo initialize audio decoder fail.");
        DestroyAudioDecoder(Demo->pAudioDec);
        Demo->pAudioDec = NULL;
        return -1;
    }

    SetRawPlayParam(Demo->pAudioDec,(void*)Demo);

    Demo->dsp = CreateAudioDsp();
    if(Demo->dsp == NULL)
    {
        demo_logd("decoder demo initialize audio dsp fail.");
        DeleteAudioDsp(Demo->dsp);
        Demo->dsp = NULL;
        return -1;
    }

    Demo->dsp->period_size = 1024;
    Demo->dsp->period_count = 4;

    Demo->cfg = initPlybkCfg;

    Demo->dsp->bps = Demo->cfg.nBitpersample;
    Demo->dsp->ch  = Demo->cfg.nChannels ;
    Demo->dsp->spr = Demo->cfg.nSamplerate;

    if(Demo->AudioInfo.nBitsPerSample != 0 && Demo->AudioInfo.nBitsPerSample != Demo->dsp->bps)
        Demo->dsp->bps = Demo->AudioInfo.nBitsPerSample;

    if(Demo->AudioInfo.nChannelNum != 0 && Demo->AudioInfo.nChannelNum != Demo->dsp->ch)
        Demo->dsp->ch  = Demo->AudioInfo.nChannelNum;

    if(Demo->AudioInfo.nSampleRate != 0 && Demo->AudioInfo.nSampleRate != Demo->dsp->spr)
        Demo->dsp->spr = Demo->AudioInfo.nSampleRate;

    Demo->dsp->raw_flag = 1;

    demo_logd("Init decoder --- bitpersample(%d), channel(%d), samples per second(%d)",
            Demo->dsp->bps, Demo->dsp->ch, Demo->dsp->spr);
    pthread_rwlock_init(&Demo->thread.rwrock, NULL);

#ifdef CAPTURE_DATA
    dtsCapture_getFileBaseName(Demo);
#endif
    return 0;
}

int main(int args, char* argv[])
{
    int nRet = 0;
    int i, nDramCostThreadNum;
    char *pInputFile;
    char *pOutputDir;
    char *pSaveShaFile;
    pthread_t tdecoder, tparser, trender;
    DecDemo  Demo;
    long long endTime;
    FILE *command_src = NULL;
    DEMO_SAFE_OPEN_FILE(command_src, "/data/camera/command_src", "wb", "command_src")
    DEMO_SAFE_CLOSE_FILE(command_src, "command_src")

    memset(&Demo, 0x00, sizeof(DecDemo));

    pInputFile = malloc(DEMO_FILE_NAME_LEN);

    if(pInputFile == NULL)
    {
        demo_loge(" input file. calloc memory fail. ");
        return 0;
    }
    memset(pInputFile, 0x00, DEMO_FILE_NAME_LEN);

    pOutputDir = malloc(DEMO_FILE_NAME_LEN);

    if(pOutputDir == NULL)
    {
        demo_logd(" output file. calloc memory fail. ");
        free(pInputFile);
        return 0;
    }
    memset(pOutputDir, 0x00, DEMO_FILE_NAME_LEN);

    Demo.pInputFile  = pInputFile;
    Demo.pOutputDir = pOutputDir;

    argv++;

    declareMyWork(0);

    if(args >= 2)
    {
        while (*argv) {
            if (strcmp(*argv, "-i") == 0 || strcmp(*argv, "-I") == 0) {
                argv++;
                if (*argv){
                    sprintf(Demo.pInputFile, "file://");
                    sscanf(*argv, "%2048s", Demo.pInputFile + 7);
                    demo_logd(" get input file: %s ", Demo.pInputFile);
                }
            }
            else if (strcmp(*argv, "-o") == 0 || strcmp(*argv, "-O") == 0) {
                argv++;
                if (*argv){
                    sscanf(*argv, "%2048s", Demo.pOutputDir);
                    demo_logd(" get output dir: %s ", Demo.pOutputDir);
                }
            }
            else if (0 == strncmp(&(*argv)[0], "--", 2 )) {
                char* ptr = &(*argv)[2];
                int sl = strlen(ptr);
                char end = ';';
                demo_logi("command len : %d, %s", sl, ptr);
                DEMO_SAFE_OPEN_FILE(command_src, "/data/camera/command_src", "ab+", "command_src")
                for(i = 0; i < sl; i++)
                {
                    DEMO_SAFE_WRITE_FILE(&ptr[i], 1, 1, command_src)
                }
                DEMO_SAFE_WRITE_FILE(&end, 1, 1, command_src)
                DEMO_SAFE_CLOSE_FILE(command_src, "command_src")
                argv++;
                continue;
            }
            else if ((*argv)[0] == '-') {
                char* ptr = &(*argv)[1];
                int sl = strlen(ptr);
                char end = ';';
                demo_logi("command len : %d, %s", sl, ptr);
                DEMO_SAFE_OPEN_FILE(command_src, "/data/camera/command_src", "ab+", "command_src")
                for(i = 0; i < sl; i++)
                {
                    DEMO_SAFE_WRITE_FILE(&ptr[i], 1, 1, command_src)
                }
                DEMO_SAFE_WRITE_FILE(&end, 1, 1, command_src)
                DEMO_SAFE_CLOSE_FILE(command_src, "command_src")
                argv++;
                continue;
            }
            else if (strcmp(*argv, "-h") == 0 || strcmp(*argv, "-H") == 0) {
                PrintDemoUsage();
                return 0;
            }
            else {
                demo_logd("invalid para...");
                return -1;
            }

            if (*argv){
                argv++;
            }
        }

    }
    else
    {
        demo_loge(" we need more arguments ");
        PrintDemoUsage();
        return 0;
    }

    nRet = initDemo(&Demo);
    if(nRet != 0)
    {
        demo_loge(" decoder demo initDecoder error ");
        return -1;
    }

    /* catch ctrl-c to shutdown cleanly */
    signal(SIGINT, demo_close);

    pthread_create(&tparser, NULL, parserThread, (void*)(&Demo));
    pthread_create(&tdecoder, NULL, decodeThread, (void*)(&Demo));
    usleep(100*1000);//wait some time ...
    pthread_create(&trender, NULL, renderThread, (void*)(&Demo));

    pthread_join(tparser, (void**)&nRet);
    pthread_join(tdecoder, (void**)&nRet);
    pthread_join(trender, (void**)&nRet);

    pthread_mutex_destroy(&Demo.parserMutex);
    demo_logd("parserMutex destroyed ...");
    pthread_rwlock_destroy(&Demo.thread.rwrock);
    demo_logd("rwrock destroyed ...");

    CdxParserClose(Demo.parser);
    Demo.parser = NULL;
    demo_logd("CdxParser closed ...");

    DestroyAudioDecoder(Demo.pAudioDec);
    Demo.pAudioDec = NULL;
    demo_logd("AudioDecoder destroyed ...");

#ifdef CAPTURE_DATA
    dtsCapture_capturePcm2wave(&Demo);
#endif

    DeleteAudioDsp(Demo.dsp);
    Demo.dsp = NULL;
    demo_logd("AudioDsp deleted ...");

    DEMO_SAFE_FREE(pInputFile, "pInputFile", 1);
    DEMO_SAFE_FREE(pOutputDir, "pOutputDir", 1);
    demo_logd("wait some time ...");
    usleep(2000*1000);
    demo_logd("go ...");
    return 0;
}
