#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#define    RIFX    0X58464952
#define    RIFF    0X46464952
#define    WAVE    0X45564157
#define    fmt     0x20746d66
#define    fact    0x74636166
#define    dataF   0x61746164

#define  EachReadDataSize 4096

#define TOTAL_FILE_NUM 1000
#define MAX_FILE_NAME_LEN 256
#define FILE_TYPE_NUM 2
#define FILE_TYPE_LEN 10

char  inputFile[TOTAL_FILE_NUM][MAX_FILE_NAME_LEN];

typedef struct wavObject{//8 bytes
    int id;
    int size;
}wavObjectS;

typedef struct RIFFChunk{//12 bytes
    wavObjectS head;
    int   type;
}RIFFChunkS;

typedef struct FormatChunk{//24 bytes
    wavObjectS head;
    short int formatType;
    short int channel;
    int sampleRate;
    int avgBytePerSec;
    short int blockAlign;
    short int bitPerSample;
}FormatChunkS;

typedef struct WavHeadObject{
    RIFFChunkS riffChunk;
    FormatChunkS formatChunk;
    wavObjectS dataHead;
}WavHeadObjectS;

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


static int muxWav(FILE* inputFp,FILE* outputFp,WavHeadObjectS* wavHead,int channel,int sampleRate,int bitPerSample,short int formatType){
    int ret = 0;
    if(inputFp == NULL || outputFp == NULL || wavHead == NULL){
        printf("input or ouput fp or wavHead is null,inputFp = %p,outputFp = %p,wavHead = %p",inputFp,outputFp,wavHead);
        ret = -1;
    }else{
        memset(wavHead,0,sizeof(WavHeadObjectS));
        fseek(inputFp,0,SEEK_END);
        wavHead->dataHead.size = ftell(inputFp);
        printf("wavHead->dataHead.size = %d\n",wavHead->dataHead.size);
        wavHead->riffChunk.head.id = RIFF;
        wavHead->riffChunk.head.size = wavHead->dataHead.size+sizeof(WavHeadObjectS)-8;
        wavHead->riffChunk.type = WAVE;
        wavHead->formatChunk.head.id = fmt;
        wavHead->formatChunk.head.size = 16;
        wavHead->formatChunk.formatType = formatType;
        wavHead->formatChunk.channel = channel;
        wavHead->formatChunk.sampleRate = sampleRate;
        wavHead->formatChunk.avgBytePerSec = sampleRate*channel*bitPerSample/8;
        wavHead->formatChunk.blockAlign = channel*bitPerSample/8;//this is needed
        wavHead->formatChunk.bitPerSample = bitPerSample;
        wavHead->dataHead.id = dataF;
        fseek(inputFp,0,SEEK_SET);
        printf("wavHead->riffChunk.head.id = 0x%x,WavHeadObjectS size = %d\n",wavHead->riffChunk.head.id,sizeof(WavHeadObjectS));
        writeRIFFChunk(outputFp,wavHead);
        writeFormatChunk(outputFp,wavHead);
        writeDataChunk(outputFp, wavHead);
        char  data[EachReadDataSize];
        int hadReadDataSize = 0;
        int readRet = 0;
        while(hadReadDataSize < wavHead->dataHead.size){
            readRet = fread(data,sizeof(char),EachReadDataSize,inputFp);
            if(readRet > 0){
                fwrite(data,sizeof(char),readRet,outputFp);
                hadReadDataSize += readRet;
            }
        }
    }
    return ret;
}

static int parseDirFiles(char* dirPath,int* realMuxFileNum){
    int ret = 0;
    if(dirPath == NULL || inputFile == NULL){
        printf("dirPath or storeFiles is null,dirPath = %p,storeFiles = %p\n",dirPath,inputFile);
        ret = -1;
    }else{
        char fileType[FILE_TYPE_NUM][FILE_TYPE_LEN] = {".wav",".pcm"};
        DIR *dir = opendir(dirPath);
        if (dir == NULL)
        {
            printf("opendir %s fail\n",dirPath);
            return -1;
        }
        struct dirent *entry;
        int count= 0;
        while ((entry = readdir(dir)) != NULL)
        {
            //printf("file record length = %d,type = %d, name = %s\n",entry->d_reclen,entry->d_type,entry->d_name);
            if(entry->d_type == 8){
                char* strpos;
                if((strpos = strrchr(entry->d_name,'.')) != NULL){
                    int i = 0;
                    //printf("cut down suffix is:%s\n",strpos);
                    for(i = 0;i < FILE_TYPE_NUM;i++){
                        if(!strncasecmp(strpos,&(fileType[i]),strlen(&(fileType[i])))){
                            printf("find the matched type:%s\n",&(fileType[i]));
                            break;
                        }
                    }

                    if(i < FILE_TYPE_NUM){
                        if(count < TOTAL_FILE_NUM){
                            strncpy(&inputFile[count],entry->d_name,strlen(entry->d_name));
                            printf("pcm file name = %s\n",&inputFile[count]);
                            count++;
                        }else{
                            count++;
                            printf("warning:the origin pcm file in %s is %d,which is larger than %d,we only support %d\n",dirPath,count,TOTAL_FILE_NUM,TOTAL_FILE_NUM);
                        }
                    }
                }
            }
        }
        closedir(dir);
        if(count >  TOTAL_FILE_NUM){
            *realMuxFileNum = TOTAL_FILE_NUM;
        }else{
            *realMuxFileNum = count;
        }
        if((*realMuxFileNum) == 0){
            printf("there are no origin pcm files in %s,return -1\n",dirPath);
            ret = -1;
        }
    }
    return ret;
}

//* the main method.
int main(int argc, char** argv)
{
    int ret = 0;
    printf("****************************************************************************\n");
    printf("* This program shows how to mux the pcm or adpcm files to wav file\n");
    printf("***************************************************************************\n");
    printf("argc = %d\n",argc);
    if((argc != 6) && (argc != 7))
    {
        printf("one Usage:\n");
        printf("muxWavDemo argv[1] argv[2] argv[3] argv[4] argv[5] argv[6]\n");
        printf(" argv[1]: the input pcm or adpcm file which contains absolute path \n");
        printf(" argv[2]: the output file which contains absolute path \n");
        printf(" argv[3]: the channel num,for example 1 or 2 \n");
        printf(" argv[4]: the samplerate,for example 16000/44100/48000 and so on \n");
        printf(" argv[5]: the bit of per sample,for example 8 or 16 and so on \n");
        printf(" argv[6]: the file type,only support pcm and adpcm \n");
        printf("for example:muxWavDemo /mnt/UDISK/test.pcm /mnt/UDISK/test.wav 2 16000 16 pcm \n");
        printf("other Usage,which can mux all pcm or adpcm files in one folder:\n");
        printf(" argv[1]: the absolute path which contains the input pcm or adpcm files \n");
        printf(" argv[2]: the channel num,for example 1 or 2 \n");
        printf(" argv[3]: the samplerate,for example 16000/44100/48000 and so on \n");
        printf(" argv[4]: the bit of per sample,for example 8 or 16 and so on \n");
        printf(" argv[5]: the file type,only support pcm and adpcm \n");
        printf("for example:muxWavDemo /mnt/UDISK/ 2 16000 16 pcm \n");
        return -1;
    }
    FILE *inputFileFP = NULL;
    FILE *outputFileFP = NULL;
    WavHeadObjectS  wavHead;

    if(argc == 6){
        int realMuxFileNum = 0;
        int parseRet = parseDirFiles(argv[1],&realMuxFileNum);
        if(parseRet == -1){
            printf("parseDirFiles %s error\n",argv[1]);
            return -1;
        }
        char inputFilePath[256];
        char outputFilePath[256];
        int muxFileCount = 0;
        int channel = atoi(argv[2]);
        int sampleRate = atoi(argv[3]);
        int bitPerSample = atoi(argv[4]);
        short int formatType = 0;
        if(strcmp(argv[5],"pcm") == 0){
            formatType = 1;
        }else if(strcmp(argv[5],"adpcm") == 0){
            formatType = 2;
        }else{
            printf("err:the formatType(%s) is not support\n",argv[5]);
            formatType = 1;
        }
        for(muxFileCount = 0; muxFileCount < realMuxFileNum;muxFileCount++){
            strcpy(inputFilePath,argv[1]);
            if(inputFilePath[strlen(inputFilePath)-1] != '/'){
                strcat(inputFilePath,"/");
            }
            strcat(inputFilePath,&inputFile[muxFileCount]);
            strcpy(outputFilePath,argv[1]);
            if(outputFilePath[strlen(outputFilePath)-1] != '/'){
                strcat(outputFilePath,"/mux_");
            }else{
                strcat(outputFilePath,"mux_");
            }
            strcat(outputFilePath,&inputFile[muxFileCount]);

            inputFileFP = fopen(inputFilePath,"rb");
            if(inputFileFP == NULL){
                printf("open %s fail\n",inputFilePath);
                return -1;
            }
            outputFileFP = fopen(outputFilePath,"wb");
            if(outputFileFP == NULL){
                printf("open %s fail\n",outputFilePath);
                fclose(inputFileFP);
                inputFileFP = NULL;
                return -1;
            }
            int muxWavRet = muxWav(inputFileFP,outputFileFP,&wavHead,channel,sampleRate,bitPerSample,formatType);
            if(outputFileFP){
                fclose(outputFileFP);
                outputFileFP = NULL;
            }
            if(inputFileFP){
                fclose(inputFileFP);
                inputFileFP = NULL;
            }
            if(muxWavRet == -1){
                printf("mux wav err,mux file count = %d\n",muxFileCount);
                ret = -1;
            }
        }
    }else if(argc == 7){
            inputFileFP = fopen(argv[1],"rb");
            if(inputFileFP == NULL){
                printf("open %s fail\n",argv[1]);
                return -1;
            }
            outputFileFP = fopen(argv[2],"wb");
            if(outputFileFP == NULL){
                printf("open %s fail\n",argv[2]);
                fclose(inputFileFP);
                inputFileFP = NULL;
                return -1;
            }
            int channel = atoi(argv[3]);
            int sampleRate = atoi(argv[4]);
            int bitPerSample = atoi(argv[5]);
            short int formatType = 0;
            if(strcmp(argv[6],"pcm") == 0){
                formatType = 1;
            }else if(strcmp(argv[6],"adpcm") == 0){
                formatType = 2;
            }else{
                printf("err:the formatType(%s) is not support\n",argv[6]);
                formatType = 1;
            }
            int muxWavRet = muxWav(inputFileFP,outputFileFP,&wavHead,channel,sampleRate,bitPerSample,formatType);
            if(outputFileFP){
                fclose(outputFileFP);
                outputFileFP = NULL;
            }
            if(inputFileFP){
                fclose(inputFileFP);
                inputFileFP = NULL;
            }
            if(muxWavRet == -1){
                printf("mux wav err\n");
                ret = -1;
            }
    }

    printf("\n");
    printf("*************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("********************************************************************\n");
    printf("\n");
    return ret;
}
