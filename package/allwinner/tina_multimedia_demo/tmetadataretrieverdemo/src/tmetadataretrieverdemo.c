/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : tmetadataretrieverdemo.c
 * Description : tmetadataretrieverdemo
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

#include <tmetadataretriever.h>

//* the main method.
int main(int argc, char** argv)
{
    printf("****************************************************************************\n");
    printf("* This program shows how to get the video thumbnail \n");
    printf("***************************************************************************\n");

    if(argc != 4)
    {
        printf("the argc = %d,is not equal 4 ,fail\n",argc);
        printf("Usage:\n");
        printf("tmetadataretrieverdemo argv[1] argv[2] argv[3] \n");
        printf(" argv[1]: the video file which contains absolute path\n");
        printf(" argv[2]: the scaledown mode,support :0,1,2,3;0 means no scaledown,1 means scaledown 1/2,2 means scaledown 1/4,3 means scaledown 1/8 \n");
        printf(" argv[3]: the output decoded data type,support:nv21 yv12 and rgb565  \n");
        printf("for example:tmetadataretrieverdemo /mnt/SDCARD/test.mp4 1 rgb565 \n");
        return -1;
    }

    TRetriever* metedataretriever;
    metedataretriever = TRetrieverCreate();

    if(NULL == metedataretriever)
    {
        printf("create metedataretriever failed\n");
        return -1;
    }

    TmetadataRetrieverScaleDownRatio scaleRatio;
    int inputScaledown = atoi(argv[2]);
    switch(inputScaledown)
    {
        case 0:
            scaleRatio = TMETADATA_RETRIEVER_SCALE_DOWN_1;
            break;
        case 1:
            scaleRatio = TMETADATA_RETRIEVER_SCALE_DOWN_2;
            break;
        case 2:
            scaleRatio = TMETADATA_RETRIEVER_SCALE_DOWN_4;
            break;
        case 3:
            scaleRatio = TMETADATA_RETRIEVER_SCALE_DOWN_8;
            break;
        default:
            printf("warnning:the input scaledown ratio:%d is not support,use the default 0\n",inputScaledown);
            scaleRatio = TMETADATA_RETRIEVER_SCALE_DOWN_1;
            break;
    }

    TmetadataRetrieverOutputDataType outputDataTpe;
        if(strcmp(argv[3],"nv21") == 0){
            outputDataTpe= TmetadataRetrieverOutputDataNV21;
        }else if(strcmp(argv[3],"yv12") == 0){
            outputDataTpe = TmetadataRetrieverOutputDataYV12;
        }else if(strcmp(argv[3],"rgb565") == 0){
            outputDataTpe = TmetadataRetrieverOutputDataRGB565;
        }else{
            printf("warnning:the %s is not support,use the default outputDataTpe:nv21\n",argv[3]);
            outputDataTpe= TmetadataRetrieverOutputDataNV21;
        }
    TRetrieverSetDataSource(metedataretriever, argv[1],scaleRatio,outputDataTpe);
    printf("TRetrieverSetDataSource end\n");

    int origin_width = 0;
    TRetrieverGetMetaData(metedataretriever, TMETADATA_VIDEO_WIDTH, &origin_width);
    int origin_height = 0;
    TRetrieverGetMetaData(metedataretriever, TMETADATA_VIDEO_HEIGHT, &origin_height);
    int duration = 0;
    TRetrieverGetMetaData(metedataretriever, TMETADATA_DURATION, &duration);
    printf("origin_width = %d,origin_height = %d,duration = %dms\n",origin_width,origin_height,duration);

    VideoFrame* videoFrame = TRetrieverGetFrameAtTime(metedataretriever,0);
    if(videoFrame == NULL){
        printf("TRetrieverGetFrameAtTime fail\n");
        TRetrieverDestory(metedataretriever);
        return -1;
    }else{
            printf("TRetrieverGetFrameAtTime successfully,videoFrame->mWidth = %d,videoFrame->mHeight = %d,videoFrame->mDisplayWidth = %d,videoFrame->mDisplayHeight = %d\n",
                videoFrame->mWidth,videoFrame->mHeight,videoFrame->mDisplayWidth,videoFrame->mDisplayHeight);
            printf("videoFrame->mYuvData = %p,videoFrame->mYuvSize = %d\n",videoFrame->mYuvData,videoFrame->mYuvSize);
            printf("videoFrame->mRGB565Data = %p,videoFrame->mRGB565Size = %d\n",videoFrame->mRGB565Data,videoFrame->mRGB565Size);
    }

    TRetrieverDestory(metedataretriever);

    printf("\n");
    printf("*************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("********************************************************************\n");
    printf("\n");

    return 0;
}
