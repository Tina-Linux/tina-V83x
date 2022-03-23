#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

typedef signed char     int8_t;
typedef short int         int16_t;
typedef int                int32_t;
typedef unsigned char      uint8_t;
typedef unsigned short int   uint16_t;
typedef unsigned int       uint32_t;

#define TOTAL_FILE_NUM 1000
#define MAX_FILE_NAME_LEN 256
#define FILE_TYPE_NUM 2
#define FILE_TYPE_LEN 10

/* Quantizer step size lookup table */
const uint16_t StepSizeTable[89]={7,8,9,10,11,12,13,14,16,17,
                            19,21,23,25,28,31,34,37,41,45,
                            50,55,60,66,73,80,88,97,107,118,
                            130,143,157,173,190,209,230,253,279,307,
                            337,371,408,449,494,544,598,658,724,796,
                            876,963,1060,1166,1282,1411,1552,1707,1878,2066,
                            2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,
                            5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,
                            15289,16818,18500,20350,22385,24623,27086,29794,32767};
/* Table of Decodeindex changes */
const int8_t IndexTable[16]={0xff,0xff,0xff,0xff,2,4,6,8,0xff,0xff,0xff,0xff,2,4,6,8};

/**
  * @brief  ADPCM_Encode.
  * @param sample: a 16-bit PCM sample
  * @retval : a 4-bit ADPCM sample
  */

int16_t  Encodeindex = 0;
int32_t Encodepredsample = 0;
uint8_t ADPCM_Encode(int32_t sample,int initFlag)
{
    uint8_t code=0;
    uint16_t tmpstep=0;
    int32_t diff=0;
    int32_t diffq=0;
    uint16_t step=0;
    if(initFlag == 0){
        Encodeindex = 0;
        Encodepredsample = 0;
    }
    step = StepSizeTable[Encodeindex];

    /* 2. compute diff and record sign and absolut value */
    diff = sample-Encodepredsample;
    if (diff < 0)
    {
      code=8;
      diff = -diff;
    }

    /* 3. quantize the diff into ADPCM code */
    /* 4. inverse quantize the code into a predicted diff */
    tmpstep = step;
    diffq = (step >> 3);

    if (diff >= tmpstep)
    {
      code |= 0x04;
      diff -= tmpstep;
      diffq += step;
    }

    tmpstep = tmpstep >> 1;

    if (diff >= tmpstep)
    {
      code |= 0x02;
      diff -= tmpstep;
      diffq+=(step >> 1);
    }

    tmpstep = tmpstep >> 1;

    if (diff >= tmpstep)
    {
      code |=0x01;
      diffq+=(step >> 2);
    }

    /* 5. fixed predictor to get new predicted sample*/
    if (code & 8)
    {
      Encodepredsample -= diffq;
    }
    else
    {
      Encodepredsample += diffq;
    }

    /* check for overflow*/
    if (Encodepredsample > 32767)
    {
      Encodepredsample = 32767;
    }
    else if (Encodepredsample < -32768)
    {
      Encodepredsample = -32768;
    }

    /* 6. find new stepsize Encodeindex */
    Encodeindex += IndexTable[code];
    /* check for overflow*/
    if (Encodeindex <0)
    {
      Encodeindex = 0;
    }
    else if (Encodeindex > 88)
    {
      Encodeindex = 88;
    }

    /* 8. return new ADPCM code*/
    return (code & 0x0f);
}


/**
  * @brief  ADPCM_Decode.
  * @param code: a byte containing a 4-bit ADPCM sample.
  * @retval : 16-bit ADPCM sample
  */
int16_t  Decodeindex = 0;
int32_t Decodepredsample = 0;

int16_t ADPCM_Decode(uint8_t code,int initFlag)
{
    if(initFlag == 0){
        Decodeindex = 0;
        Decodepredsample = 0;
    }
    uint16_t step=0;
    int32_t diffq=0;

    step = StepSizeTable[Decodeindex];

    /* 2. inverse code into diff */
    diffq = step>> 3;
    if (code&4)
    {
      diffq += step;
    }

    if (code&2)
    {
      diffq += step>>1;
    }

    if (code&1)
    {
      diffq += step>>2;
    }

    /* 3. add diff to predicted sample*/
    if (code&8)
    {
      Decodepredsample -= diffq;
    }
    else
    {
      Decodepredsample += diffq;
    }

    /* check for overflow*/
    if (Decodepredsample > 32767)
    {
      Decodepredsample = 32767;
    }
    else if (Decodepredsample < -32768)
    {
      Decodepredsample = -32768;
    }

    /* 4. find new quantizer step size */
    Decodeindex += IndexTable [code];
    /* check for overflow*/
    if (Decodeindex < 0)
    {
      Decodeindex = 0;
    }
    if (Decodeindex > 88)
    {
      Decodeindex = 88;
    }

    /* 5. save predict sample and Decodeindex for next iteration */
    /* done! static variables */

    /* 6. return new speech sample*/
    return ((int16_t)Decodepredsample);
}

char  inputFile[TOTAL_FILE_NUM][MAX_FILE_NAME_LEN];

static int parseDirFiles(char* dirPath,int* realDecodeFileNum){
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
            *realDecodeFileNum = TOTAL_FILE_NUM;
        }else{
            *realDecodeFileNum = count;
        }
        if((*realDecodeFileNum) == 0){
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
    printf("* This program shows how to decode adpcm to pcm\n");
    printf("***************************************************************************\n");

    if((argc != 2) && (argc != 3))
    {
        printf("one Usage:\n");
        printf("adpcmDecodeDemo argv[1] argv[2]\n");
        printf(" argv[1]: the adpcm file which contains absolute path\n");
        printf(" argv[2]: the output pcm file which contains absolute path \n");
        printf("for example:adpcmDecodeDemo /mnt/UDISK/adpcm.wav /mnt/UDISK/decoded_pcm.wav\n");
        printf("other Usage,which can decode all files in one folder:\n");
        printf("adpcmDecodeDemo argv[1] \n");
        printf(" argv[1]: the absolute path ,where the adpcm files in \n");
        printf("for example:adpcmDecodeDemo /mnt/UDISK/ \n");
        return -1;
    }

    FILE *inputFileFp;
    FILE *outputFileFp;

    uint8_t originHighsample;
    uint8_t originLowsample;
    int16_t decodedsample;
    int needReadDataSize = 1024;
    uint8_t readData[needReadDataSize];
    int16_t decodedData[needReadDataSize*2];
    int realReadSize = 0; //the real size each read from file,the unit is byte
    int realWriteSize;//the real size which write to sound card ,the unit is sizeof(int16_t)

    if(argc == 2){
        int realDecodeFileNum = 0;
        int parseRet = parseDirFiles(argv[1],&realDecodeFileNum);
        if(parseRet == -1){
            printf("parseDirFiles %s error\n",argv[1]);
            return -1;
        }
        for(int i = 0; i < realDecodeFileNum;i++){
            printf("input file name:%s\n",inputFile[i]);
        }
        char inputFilePath[256];
        char outputFilePath[256];
        for(int decodeCount = 0; decodeCount < realDecodeFileNum;decodeCount++){
            strcpy(inputFilePath,argv[1]);
            if(inputFilePath[strlen(inputFilePath)-1] != '/'){
                strcat(inputFilePath,"/");
            }
            strcat(inputFilePath,&inputFile[decodeCount]);
            strcpy(outputFilePath,argv[1]);
            if(outputFilePath[strlen(outputFilePath)-1] != '/'){
                strcat(outputFilePath,"/pcm_");
            }else{
                strcat(outputFilePath,"pcm_");
            }
            strcat(outputFilePath,&inputFile[decodeCount]);

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

            int initDecodeFlag = 0;
            while(1){
                int bufIndex = 0;
                realReadSize = fread(readData,sizeof(uint8_t),needReadDataSize,inputFileFp);
                if(realReadSize <= 0){
                    printf("reat to file end:realReadSize = %d\n",realReadSize);
                    break;
                }
                //printf("realReadSize = %d\n",realReadSize);
                for(int count = 0 ; count < realReadSize; count++){
                    originHighsample = readData[count] >> 4;
                    if(initDecodeFlag == 0){
                        decodedsample = ADPCM_Decode(originHighsample,initDecodeFlag);
                        initDecodeFlag = 1;
                    }else{
                        decodedsample = ADPCM_Decode(originHighsample,initDecodeFlag);
                    }
                    decodedData[bufIndex] = decodedsample;
                    bufIndex++;
                    originLowsample = readData[count] & 0x0F;
                    decodedsample = ADPCM_Decode(originLowsample,initDecodeFlag);
                    decodedData[bufIndex] = decodedsample;
                    bufIndex++;
                }
                realWriteSize = realReadSize*2;
                fwrite(decodedData,sizeof(int16_t),realWriteSize,outputFileFp);
            }
            if(outputFileFp){
                fclose(outputFileFp);
                outputFileFp = NULL;
            }
            if(inputFileFp){
                fclose(inputFileFp);
                inputFileFp = NULL;
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

        int initDecodeFlag = 0;
        while(1){

            int bufIndex = 0;
            realReadSize = fread(readData,sizeof(uint8_t),needReadDataSize,inputFileFp);
            if(realReadSize <= 0){
                printf("reat to file end:realReadSize = %d\n",realReadSize);
                break;
            }
            //printf("realReadSize = %d\n",realReadSize);
            for(int count = 0 ; count < realReadSize; count++){
                originHighsample = readData[count] >> 4;
                if(initDecodeFlag == 0){
                    decodedsample = ADPCM_Decode(originHighsample,initDecodeFlag);
                    initDecodeFlag = 1;
                }else{
                    decodedsample = ADPCM_Decode(originHighsample,initDecodeFlag);
                }
                decodedData[bufIndex] = decodedsample;
                bufIndex++;
                originLowsample = readData[count] & 0x0F;
                decodedsample = ADPCM_Decode(originLowsample,initDecodeFlag);
                decodedData[bufIndex] = decodedsample;
                bufIndex++;
            }
            realWriteSize = realReadSize*2;
            fwrite(decodedData,sizeof(int16_t),realWriteSize,outputFileFp);
        }
        if(outputFileFp){
            fclose(outputFileFp);
            outputFileFp = NULL;
        }
        if(inputFileFp){
            fclose(inputFileFp);
            inputFileFp = NULL;
        }
    }

    printf("\n");
    printf("*************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("********************************************************************\n");
    printf("\n");

    return 0;
}
