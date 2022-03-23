#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include "adpcm.h"
#include "wav.h"

#define TOTAL_FILE_NUM 1000
#define MAX_FILE_NAME_LEN 256
#define FILE_TYPE_NUM 2
#define FILE_TYPE_LEN 10

char  inputFile[TOTAL_FILE_NUM][MAX_FILE_NAME_LEN];

static int parseDirFiles(char* dirPath,int* realEncodedAndMuxFileNum){
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
            *realEncodedAndMuxFileNum = TOTAL_FILE_NUM;
        }else{
            *realEncodedAndMuxFileNum = count;
        }
        if((*realEncodedAndMuxFileNum) == 0){
            printf("there are no origin pcm files in %s,return -1\n",dirPath);
            ret = -1;
        }
    }
    return ret;
}

//* the main method.
int main(int argc, char** argv)
{
    printf("****************************************************************************\n");
    printf("* This program shows how to encode and mux pcm.wav to adpcm.wav\n");
    printf("***************************************************************************\n");

    printf("argc = %d\n",argc);
    if((argc != 2) && (argc != 3))
    {
        printf("one Usage:\n");
        printf("createAdpcmAndWavDemo argv[1] argv[2] \n");
        printf(" argv[1]: the pcm file which contains absolute path \n");
        printf(" argv[2]: the output adpcm file which contains absolute path \n");
        printf("for example:createAdpcmAndWavDemo /mnt/UDISK/pcm.wav /mnt/UDISK/adpcm.wav\n");
        printf("other Usage,which can encode all files in one folder:\n");
        printf("createAdpcmAndWavDemo argv[1] \n");
        printf(" argv[1]: the absolute path ,where the pcm files in \n");
        printf("for example:createAdpcmAndWavDemo /mnt/UDISK/ \n");
        return -1;
    }
    FILE *inputFileFp = NULL;
    FILE *outputFileFp = NULL;

    int readRet = 0;
    uint8_t highAdpcmSample;
    uint8_t lowAdpcmSample;
    int32_t readPcmSize = 4096;
    int16_t readPcmData[readPcmSize];
    uint8_t adpcmData[readPcmSize/2];

    if(argc == 2){
        printf("argc = %d,argv[0] = %s,argv[1] = %s\n",argc,argv[0],argv[1]);
        int realEncodedAndMuxFileNum = 0;
        int parseRet = parseDirFiles(argv[1],&realEncodedAndMuxFileNum);
        if(parseRet == -1){
            printf("parseDirFiles %s error\n",argv[1]);
            return -1;
        }
        for(int i = 0; i < realEncodedAndMuxFileNum;i++){
            printf("input file name:%s\n",inputFile[i]);
        }
        char inputFilePath[256];
        char outputFilePath[256];
        for(int encodeCount = 0; encodeCount < realEncodedAndMuxFileNum;encodeCount++){
            strcpy(inputFilePath,argv[1]);
            if(inputFilePath[strlen(inputFilePath)-1] != '/'){
                strcat(inputFilePath,"/");
            }
            strcat(inputFilePath,&inputFile[encodeCount]);
            strcpy(outputFilePath,argv[1]);
            if(outputFilePath[strlen(outputFilePath)-1] != '/'){
                strcat(outputFilePath,"/aw_");
            }else{
                strcat(outputFilePath,"aw_");
            }
            strcat(outputFilePath,&inputFile[encodeCount]);

            inputFileFp = fopen(inputFilePath,"rb");
            if(inputFileFp == NULL){
                printf("open %s fail\n",inputFilePath);
                return -1;
            }
            outputFileFp = fopen(outputFilePath,"wb");
            if(outputFileFp == NULL){
                printf("open %s fail\n",outputFilePath);
                fclose(inputFileFp);
                inputFileFp = NULL;
                return -1;
            }
            WavHeadObjectS pcmWavHead;
            ParseWavFile(inputFileFp,&pcmWavHead);
            int channel = pcmWavHead.formatChunk.channel;
            int sampleRate = pcmWavHead.formatChunk.sampleRate;
            int bitPerSample = pcmWavHead.formatChunk.bitPerSample;
            FORMAT_TYPE formatType = ADPCM_TYPE;
            int adpcmTotalDataLen = pcmWavHead.dataHead.size/4;
            printf("adpcmTotalDataLen = %d\n",adpcmTotalDataLen);
            MuxWavHead(outputFileFp,channel,sampleRate,bitPerSample,formatType,adpcmTotalDataLen);
            int initEncodeFlag = 0;
            while(1){
                readRet = fread(readPcmData,sizeof(int16_t),readPcmSize,inputFileFp);
                if(readRet <= 0){
                    printf("read to file end:readRet = %d,encoded file num = %d\n",readRet,encodeCount+1);
                    break;
                }
                int adpcmDataIndex = 0;
                for(int count = 0; count < readRet;count++){
                    if((count %2) == 0){
                        if(initEncodeFlag == 0){
                            highAdpcmSample = ADPCM_Encode(readPcmData[count],initEncodeFlag) << 4;
                            initEncodeFlag = 1;
                        }else{
                            highAdpcmSample = ADPCM_Encode(readPcmData[count],initEncodeFlag) << 4;
                        }
                    }else{
                        lowAdpcmSample = ADPCM_Encode(readPcmData[count],initEncodeFlag);
                        adpcmData[adpcmDataIndex] = highAdpcmSample | lowAdpcmSample;
                        adpcmDataIndex++;
                    }
                }
                //printf("adpcmDataIndex = %d\n",adpcmDataIndex);
                MuxWavData(adpcmData,adpcmDataIndex,outputFileFp);
            }

            if(inputFileFp){
                fclose(inputFileFp);
                inputFileFp = NULL;
            }
            if(outputFileFp){
                fclose(outputFileFp);
                outputFileFp = NULL;
            }
        }

    }else if(argc == 3){
        inputFileFp = fopen(argv[1],"rb");
        if(inputFileFp == NULL){
            printf("open %s fail\n",argv[1]);
            return -1;
        }
        outputFileFp = fopen(argv[2],"wb");
        if(outputFileFp == NULL){
            printf("open %s fail\n",argv[2]);
            fclose(inputFileFp);
            inputFileFp = NULL;
            return -1;
        }
        WavHeadObjectS pcmWavHead;
        ParseWavFile(inputFileFp,&pcmWavHead);
        int channel = pcmWavHead.formatChunk.channel;
        int sampleRate = pcmWavHead.formatChunk.sampleRate;
        int bitPerSample = pcmWavHead.formatChunk.bitPerSample;
        FORMAT_TYPE formatType = ADPCM_TYPE;
        int adpcmTotalDataLen = pcmWavHead.dataHead.size/4;
        printf("adpcmTotalDataLen = %d\n",adpcmTotalDataLen);
        MuxWavHead(outputFileFp,channel,sampleRate,bitPerSample,formatType,adpcmTotalDataLen);
        int initEncodeFlag = 0;
        while(1){
            readRet = fread(readPcmData,sizeof(int16_t),readPcmSize,inputFileFp);
            if(readRet <= 0){
                printf("read to file end:readRet = %d\n",readRet);
                break;
            }
            int adpcmDataIndex = 0;
            for(int count = 0; count < readRet;count++){
                if((count %2) == 0){
                    if(initEncodeFlag == 0){
                        highAdpcmSample = ADPCM_Encode(readPcmData[count],initEncodeFlag) << 4;
                        initEncodeFlag = 1;
                    }else{
                        highAdpcmSample = ADPCM_Encode(readPcmData[count],initEncodeFlag) << 4;
                    }
                }else{
                    lowAdpcmSample = ADPCM_Encode(readPcmData[count],initEncodeFlag);
                    adpcmData[adpcmDataIndex] = highAdpcmSample | lowAdpcmSample;
                    adpcmDataIndex++;
                }
            }
            //printf("adpcmDataIndex = %d\n",adpcmDataIndex);
            MuxWavData(adpcmData,adpcmDataIndex,outputFileFp);
        }

        if(inputFileFp){
            fclose(inputFileFp);
            inputFileFp = NULL;
        }
        if(outputFileFp){
            fclose(outputFileFp);
            outputFileFp = NULL;
        }
    }

    printf("\n");
    printf("*************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("********************************************************************\n");
    printf("\n");

    return 0;
}
