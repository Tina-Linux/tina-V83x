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

#define TOTAL_FILE_NUM 1000
#define MAX_FILE_NAME_LEN 256
#define FILE_TYPE_NUM 2
#define FILE_TYPE_LEN 10

typedef struct ChunkHead{
    int id;
    int size;
}chunkHead;

typedef struct FormatChunk{
    short int formatType;
    short int channel;
    int sampleRate;
    int avgBytePerSec;
    short int blockAlign;
    short int bitPerSample;
}formatChunkS;

static int parserWavFile(FILE* inputFp,FILE* outputFp){
    int ret = 0;
    if(inputFp == NULL || outputFp == NULL){
        printf("the input or out put file is null,inputFp = %p,outputFp = %p\n",inputFp,outputFp);
        ret = -1;
    }else{
        int readRet = 0;
        chunkHead head;
        formatChunkS formatChunk;
        int findDataChunkFlag = 0;
        int tmpWaveUid;
        readRet = fread(&head,sizeof(chunkHead),1,inputFp);
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
                    readRet = fread(&head,sizeof(chunkHead),1,inputFp);
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
                            readRet = fread(&formatChunk,sizeof(formatChunkS),1,inputFp);
                            if(readRet != 1){
                                printf("read err:readRet = %d\n",readRet);
                                return -1;
                            }
                            if(formatChunkSize > 16){
                                printf("format chunk has extra msg,skip it\n");
                                fseek(inputFp,formatChunkSize-16, SEEK_CUR);
                            }
                            printf("formatChunk.formatType = %d (0x%x) \n",formatChunk.formatType,formatChunk.formatType);
                            printf("formatChunk.channel = %d (0x%x) \n",formatChunk.channel,formatChunk.channel);
                            printf("formatChunk.sampleRate = %d (0x%x) \n",formatChunk.sampleRate,formatChunk.sampleRate);
                            printf("formatChunk.avgBytePerSec = %d (0x%x) \n",formatChunk.avgBytePerSec,formatChunk.avgBytePerSec);
                            printf("formatChunk.blockAlign = %d (0x%x) \n",formatChunk.blockAlign,formatChunk.blockAlign);
                            printf("formatChunk.bitPerSample = %d (0x%x) \n",formatChunk.bitPerSample,formatChunk.bitPerSample);
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
                            int dataChunkSize = head.size;
                            printf("dataChunkSize = %d (0x%x) ,formatChunk.bitPerSample = %d\n",dataChunkSize,dataChunkSize,formatChunk.bitPerSample);
                            int eachReadDataSize = 1024;
                            if(formatChunk.bitPerSample == 8){
                                char readBuf[eachReadDataSize];
                                readRet = fread(&readBuf,sizeof(char),eachReadDataSize,inputFp);
                                printf("readRet = %d\n",readRet);
                                while(readRet > 0){
                                    fwrite(&readBuf,sizeof(char),readRet,outputFp);
                                    readRet = fread(&readBuf,sizeof(char),eachReadDataSize,inputFp);
                                    //printf("readRet = %d\n",readRet);
                                }
                            }else if(formatChunk.bitPerSample == 16){
                                short int readBuf[eachReadDataSize];
                                readRet = fread(&readBuf,sizeof(short int),eachReadDataSize,inputFp);
                                printf("readRet = %d\n",readRet);
                                while(readRet > 0){
                                    fwrite(&readBuf,sizeof(short int),readRet,outputFp);
                                    readRet = fread(&readBuf,sizeof(short int),eachReadDataSize,inputFp);
                                    //printf("readRet = %d\n",readRet);
                                }
                            }else if(formatChunk.bitPerSample == 32){
                                int readBuf[eachReadDataSize];
                                readRet = fread(&readBuf,sizeof(int),eachReadDataSize,inputFp);
                                printf("readRet = %d\n",readRet);
                                while(readRet > 0){
                                    fwrite(&readBuf,sizeof(int),readRet,outputFp);
                                    readRet = fread(&readBuf,sizeof(int),eachReadDataSize,inputFp);
                                    //printf("readRet = %d\n",readRet);
                                }
                            }else{
                                printf("err:the formatChunk.bitPerSample = %d,which is not support\n",formatChunk.bitPerSample);
                            }
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

char  inputFile[TOTAL_FILE_NUM][MAX_FILE_NAME_LEN];

static int parseDirFiles(char* dirPath,int* realParseFileNum){
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
            *realParseFileNum = TOTAL_FILE_NUM;
        }else{
            *realParseFileNum = count;
        }
        if((*realParseFileNum) == 0){
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
    printf("* This program shows how to parser wav file\n");
    printf("***************************************************************************\n");
    printf("argc = %d\n",argc);
    if((argc != 2) && (argc != 3))
    {
        printf("one Usage:\n");
        printf("parserWavDemo argv[1] argv[2] \n");
        printf(" argv[1]: the input wav file which contains absolute path \n");
        printf(" argv[2]: the output file which contains absolute path \n");
        printf("for example:parserWavDemo /mnt/UDISK/test.wav /mnt/UDISK/test.pcm \n");
        printf("other Usage,which can parser all files in one folder:\n");
        printf("parserWavDemo argv[1] \n");
        printf(" argv[1]: the absolute path ,where the wav files in \n");
        printf("for example:parserWavDemo /mnt/UDISK/ \n");
        return -1;
    }
    FILE *inputFileFP = NULL;
    FILE *outputFileFP = NULL;
    if(argc == 2){
        int realParseFileNum = 0;
        int parseRet = parseDirFiles(argv[1],&realParseFileNum);
        if(parseRet == -1){
            printf("parseDirFiles %s error\n",argv[1]);
            return -1;
        }
        char inputFilePath[256];
        char outputFilePath[256];
        int parseFileCount = 0;
        for(parseFileCount = 0; parseFileCount < realParseFileNum;parseFileCount++){
            strcpy(inputFilePath,argv[1]);
            if(inputFilePath[strlen(inputFilePath)-1] != '/'){
                strcat(inputFilePath,"/");
            }
            strcat(inputFilePath,&inputFile[parseFileCount]);
            strcpy(outputFilePath,argv[1]);
            if(outputFilePath[strlen(outputFilePath)-1] != '/'){
                strcat(outputFilePath,"/parsed_");
            }else{
                strcat(outputFilePath,"parsed_");
            }
            strcat(outputFilePath,&inputFile[parseFileCount]);

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
            int parserWavRet = parserWavFile(inputFileFP,outputFileFP);
            if(outputFileFP){
                fclose(outputFileFP);
                outputFileFP = NULL;
            }
            if(inputFileFP){
                fclose(inputFileFP);
                inputFileFP = NULL;
            }
            if(parserWavRet == -1){
                printf("parse wav err,parse file count = %d\n",parseFileCount);
            }
        }
    }else if(argc == 3){
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
            int parserWavRet = parserWavFile(inputFileFP,outputFileFP);
            if(outputFileFP){
                fclose(outputFileFP);
                outputFileFP = NULL;
            }
            if(inputFileFP){
                fclose(inputFileFP);
                inputFileFP = NULL;
            }
            if(parserWavRet == -1){
                printf("parse wav err\n");
            }
    }

    printf("\n");
    printf("*************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("********************************************************************\n");
    printf("\n");

    return 0;
}
