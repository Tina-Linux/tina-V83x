#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include "wav.h"

#define    RIFX    0X58464952
#define    RIFF    0X46464952
#define    WAVE    0X45564157
#define    fmt     0x20746d66
#define    fact    0x74636166
#define    dataF   0x61746164

static void writeRIFFChunk(FILE* outputFp,WavHeadObjectS* wavHead){
    fwrite(&(wavHead->riffChunk.head.id),sizeof(int),1,outputFp);
    fwrite(&(wavHead->riffChunk.head.size),sizeof(int),1,outputFp);
    fwrite(&(wavHead->riffChunk.type),sizeof(int),1,outputFp);
}

static void writeFormatChunk(FILE* outputFp,WavHeadObjectS* wavHead){
    fwrite(&(wavHead->formatChunk.head.id),sizeof(int),1,outputFp);
    fwrite(&(wavHead->formatChunk.head.size),sizeof(int),1,outputFp);
    fwrite(&(wavHead->formatChunk.formatType),sizeof(short int),1,outputFp);
    fwrite(&(wavHead->formatChunk.channel),sizeof(short int),1,outputFp);
    fwrite(&(wavHead->formatChunk.sampleRate),sizeof(int),1,outputFp);
    fwrite(&(wavHead->formatChunk.avgBytePerSec),sizeof(int),1,outputFp);
    fwrite(&(wavHead->formatChunk.blockAlign),sizeof(short int),1,outputFp);
    fwrite(&(wavHead->formatChunk.bitPerSample),sizeof(short int),1,outputFp);
}

static void writeDataChunk(FILE* outputFp,WavHeadObjectS* wavHead){
    fwrite(&(wavHead->dataHead.id),sizeof(int),1,outputFp);
    fwrite(&(wavHead->dataHead.size),sizeof(int),1,outputFp);
}

/**
  * @brief  parseWavFile.
  * @param inputFp: the input wav file,which we will read data from this file,and after call parseWavFile,inputFp will point to the real data
  * @param wavHeadP: we will parse wav head and store in wavHeadP
  * @retval : 0 means successful,-1 means fail
  */
int ParseWavFile(FILE* inputFp,WavHeadObjectS* wavHeadP){
    int ret = 0;
    if(inputFp == NULL || wavHeadP == NULL){
        printf("the input  file or wavHeadP is null,inputFp = %p,wavHeadP = %p\n",inputFp,wavHeadP);
        ret = -1;
    }else{
        int readRet = 0;
        wavObjectS head;
        FormatChunkS formatChunk;
        int findDataChunkFlag = 0;
        int tmpWaveUid;
        readRet = fread(&head,sizeof(wavObjectS),1,inputFp);
        if(readRet != 1){
            printf("read err:readRet = %d\n",readRet);
            return -1;
        }
        if(head.id == RIFF){
            readRet = fread(&tmpWaveUid,sizeof(int),1,inputFp);
            if(readRet != 1){
                printf("read err:readRet = %d\n",readRet);
                return -1;
            }
            if(tmpWaveUid == WAVE){
                while(!findDataChunkFlag){
                    readRet = fread(&head,sizeof(wavObjectS),1,inputFp);
                    if(readRet != 1){
                        printf("read err:readRet = %d\n",readRet);
                        return -1;
                    }
                    switch(head.id)
                    {
                        case fmt:
                        {
                            printf("it is format chunk\n");
                            int formatChunkSize = head.size;
                            printf("formatChunkSize = %d (0x%x) \n",formatChunkSize,formatChunkSize);
                            fseek(inputFp,-sizeof(wavObjectS),SEEK_CUR);
                            readRet = fread(&formatChunk,sizeof(FormatChunkS),1,inputFp);
                            if(readRet != 1){
                                printf("read err:readRet = %d\n",readRet);
                                return -1;
                            }
                            if(formatChunkSize > 16){
                                printf("format chunk has extra msg,skip it\n");
                                fseek(inputFp,formatChunkSize+sizeof(wavObjectS)-16, SEEK_CUR);
                            }
                            printf("formatChunk.formatType = %d (0x%x) \n",formatChunk.formatType,formatChunk.formatType);
                            printf("formatChunk.channel = %d (0x%x) \n",formatChunk.channel,formatChunk.channel);
                            printf("formatChunk.sampleRate = %d (0x%x) \n",formatChunk.sampleRate,formatChunk.sampleRate);
                            printf("formatChunk.avgBytePerSec = %d (0x%x) \n",formatChunk.avgBytePerSec,formatChunk.avgBytePerSec);
                            printf("formatChunk.blockAlign = %d (0x%x) \n",formatChunk.blockAlign,formatChunk.blockAlign);
                            printf("formatChunk.bitPerSample = %d (0x%x) \n",formatChunk.bitPerSample,formatChunk.bitPerSample);
                            wavHeadP->formatChunk.channel = formatChunk.channel;
                            wavHeadP->formatChunk.sampleRate = formatChunk.sampleRate;
                            wavHeadP->formatChunk.bitPerSample = formatChunk.bitPerSample;
                            break;
                        }
                        case fact:
                        {
                            printf("it is fact chunk\n");
                            int factChunkSize = head.size;
                            printf("factChunkSize = %d (0x%x) \n",factChunkSize,factChunkSize);
                            int factChunkData;
                            readRet = fread(&factChunkData,sizeof(int),1,inputFp);//read the factChunkData and skip it
                            if(readRet != 1){
                                printf("read err:readRet = %d\n",readRet);
                                return -1;
                            }
                            break;
                        }
                        case dataF:
                        {
                            printf("it is data chunk\n");
                            wavHeadP->dataHead.size = head.size;
                            printf("dataChunkSize = %d (0x%x) ,formatChunk.bitPerSample = %d\n",wavHeadP->dataHead.size,wavHeadP->dataHead.size,formatChunk.bitPerSample);
                            findDataChunkFlag = 1;//has found data chunk
                            break;
                        }
                        default:
                        {
                            printf("it is other chunk,this chunk size = %d,we skip it\n",head.size);
                            fseek(inputFp,head.size, SEEK_CUR);
                            break;
                        }
                     }
                }
            }
        }else if(head.id == RIFX){
            printf("it is RIFX file,to do\n");
        }else{
            printf("warning:it is not RIFF or RIFX file\n");
            ret = -1;
        }
    }
    return ret;
}

/**
  * @brief  muxWavHead.
  * @param inputData: the input pcm or adpcm data
  * @param channel: the channel
  * @param sampleRate: the sampleRate
  * @param bitPerSample: the bits per sample
  * @param formatType: the formatType,which support PCM_TYPE and ADPCM_TYPE
  * @retval : 0 means successful,-1 means fail
  */
int MuxWavHead(FILE* outputFp,int channel,int sampleRate,int bitPerSample,FORMAT_TYPE formatType,int totalDataLen){
    int ret = 0;
    if(outputFp == NULL){
        printf("the ouput fp is null,outputFp = %p\n",outputFp);
        ret = -1;
    }else{
        WavHeadObjectS wavHead;
        memset(&wavHead,0,sizeof(WavHeadObjectS));
        wavHead.riffChunk.head.id = RIFF;
        wavHead.riffChunk.head.size = totalDataLen+sizeof(WavHeadObjectS)-8;
        wavHead.riffChunk.type = WAVE;
        wavHead.formatChunk.head.id = fmt;
        wavHead.formatChunk.head.size = 16;
        wavHead.formatChunk.formatType = (short int)formatType;
        wavHead.formatChunk.channel = channel;
        wavHead.formatChunk.sampleRate = sampleRate;
        wavHead.formatChunk.avgBytePerSec = sampleRate*channel*bitPerSample/8;
        wavHead.formatChunk.blockAlign = channel*bitPerSample/8;//this is needed
        wavHead.formatChunk.bitPerSample = bitPerSample;
        wavHead.dataHead.id = dataF;
        wavHead.dataHead.size = totalDataLen;
        printf("wavHead->riffChunk.head.id = 0x%x,WavHeadObjectS size = %d\n",wavHead.riffChunk.head.id,sizeof(WavHeadObjectS));
        writeRIFFChunk(outputFp,&wavHead);
        writeFormatChunk(outputFp,&wavHead);
        writeDataChunk(outputFp, &wavHead);
    }
    return ret;
}

/**
  * @brief  muxWavData.
  * @param inputData: the input pcm or adpcm data
  * @param inputDataLen: the len of input pcm or adpcm data,the unit is Bytes
  * @param outputFp: the output file
  * @retval : 0 means successful,-1 means fail
  */
int MuxWavData(unsigned char* inputData,int inputDataLen,FILE* outputFp){
    int ret = 0;
    if(inputData == NULL || outputFp == NULL ){
        printf("inputData or ouput fp is null,inputData = %p,outputFp = %p\n",inputData,outputFp);
        ret = -1;
    }else{
        fwrite(inputData,sizeof(unsigned char),inputDataLen,outputFp);
    }
    return ret;
}
