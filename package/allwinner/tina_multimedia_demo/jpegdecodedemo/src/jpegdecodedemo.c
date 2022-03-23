/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : xmetademo.c
 * Description : xmetademo
 * History :
 *
 */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>

#include <jpegdecode.h>

static char * readSrcData(char *path, int *pLen)
{
    FILE *fp = NULL;
    int ret = 0;
    char *data = NULL;

    fp = fopen(path, "rb");
    if(fp == NULL)
    {
        printf("read jpeg file error, errno(%d)\n", errno);
        return NULL;
    }

    fseek(fp,0,SEEK_END);
    *pLen = ftell(fp);
    rewind(fp);
    data = (char *) malloc (sizeof(char)*(*pLen));

    if(data == NULL)
      {
          printf("malloc memory fail\n");
          fclose(fp);
          return NULL;
      }

    ret = fread (data,1,*pLen,fp);
    if (ret != *pLen)
    {
        printf("read src file fail\n");
        fclose(fp);
        free(data);
        return NULL;
    }

    if(fp != NULL)
    {
        fclose(fp);
    }
    return data;
}


//* the main method.
int main(int argc, char** argv)
{
    printf("****************************************************************************\n");
    printf("* This program shows how to decode a jpeg picture to yuv or rgb data\n");
    printf("***************************************************************************\n");

    if(argc != 4)
    {
        printf("Usage:\n");
        printf("jpegdecodedemo argv[1] argv[2] argv[3] \n");
        printf(" argv[1]: the jpeg file which contains absolute path\n");
        printf(" argv[2]: the scaledown mode,support :0,1,2,3;0 means no scaledown,1 means scaledown 1/2,2 means scaledown 1/4,3 means scaledown 1/8 \n");
        printf(" argv[3]: the decoded data type,support:nv21 nv12 yu12 yv12 and rgb565 \n");
        printf("for example:jpegdecodedemo /mnt/SDCARD/test.jpg 1 yv12 \n");
        return -1;
    }

    JpegDecoder* jpegdecoder;
    jpegdecoder = JpegDecoderCreate();

    if(NULL == jpegdecoder)
    {
        printf("create jpegdecoder failed\n");
        return -1;
    }

    JpegDecodeScaleDownRatio scaleRatio;
    int inputScaledown = atoi(argv[2]);
    switch(inputScaledown)
    {
        case 0:
            scaleRatio = JPEG_DECODE_SCALE_DOWN_1;
            break;
        case 1:
            scaleRatio = JPEG_DECODE_SCALE_DOWN_2;
            break;
        case 2:
            scaleRatio = JPEG_DECODE_SCALE_DOWN_4;
            break;
        case 3:
            scaleRatio = JPEG_DECODE_SCALE_DOWN_8;
            break;
        default:
            printf("the input scaledown ratio:%d is not support,use the default 0\n",inputScaledown);
            scaleRatio = JPEG_DECODE_SCALE_DOWN_1;
            break;
    }
    JpegDecodeOutputDataType outputDataTpe;
    if(strcmp(argv[3],"nv21") == 0){
        outputDataTpe= JpegDecodeOutputDataNV21;
    }else if(strcmp(argv[3],"nv12") == 0){
        outputDataTpe = JpegDecodeOutputDataNV12;
    }else if(strcmp(argv[3],"yu12") == 0){
        outputDataTpe = JpegDecodeOutputDataYU12;
    }else if(strcmp(argv[3],"yv12") == 0){
        outputDataTpe = JpegDecodeOutputDataYV12;
    }else if(strcmp(argv[3],"rgb565") == 0){
        outputDataTpe = JpegDecodeOutputDataRGB565;
    }else{
        printf("the %s is not support,use the default outputDataTpe:nv21\n",argv[3]);
        outputDataTpe= JpegDecodeOutputDataNV21;
    }


    char* srcBuf;
    int srcBufLen;
    srcBuf = readSrcData(argv[1],&srcBufLen);
    JpegDecoderSetDataSourceBuf(jpegdecoder, srcBuf,srcBufLen,scaleRatio,outputDataTpe);
    printf("srcBuf = %p,srcBufLen = %d\n",srcBuf,srcBufLen);

    //JpegDecoderSetDataSource(jpegdecoder, argv[1],scaleRatio,outputDataTpe);
    printf("JpegDecoderSetDataSource end\n");

    ImgFrame* imgFrame = JpegDecoderGetFrame(jpegdecoder);
    if(imgFrame == NULL){
        printf("JpegDecoderGetFrame fail\n");
        JpegDecoderDestory(jpegdecoder);
        return -1;
    }else{
            printf("JpegDecoderGetFrame successfully,imgFrame->mWidth = %d,imgFrame->mHeight = %d,imgFrame->mYuvData = %p,imgFrame->mYuvSize = %d\n",
                imgFrame->mWidth,imgFrame->mHeight,imgFrame->mYuvData,imgFrame->mYuvSize);
            printf("imgFrame->mRGB565Data = %p,imgFrame->mRGB565Size = %d\n",imgFrame->mRGB565Data,imgFrame->mRGB565Size);
    }

    JpegDecoderDestory(jpegdecoder);

    printf("\n");
    printf("*************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("********************************************************************\n");
    printf("\n");

    return 0;
}
