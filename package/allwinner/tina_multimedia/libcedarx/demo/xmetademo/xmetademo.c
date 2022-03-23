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

#include "cdx_config.h"
#include <cdx_log.h>
#include "xmetadataretriever.h"
#include "CdxTypes.h"
#include "CdxTime.h"

//* the main method.
int main(int argc, char** argv)
{
    CEDARX_UNUSE(argc);
    CEDARX_UNUSE(argv);
    int ret;

    printf("\n");
    printf("****************************************************************************\n");
    printf("* This program implements a simple player,");
    printf("* you can type commands to control the player.\n");
    printf("* To show what commands supported, type 'help'.\n");
    printf("* Inplemented by Allwinner ALD-AL3 department.\n");
    printf("***************************************************************************\n");

    if(argc < 2)
    {
        printf("Usage:\n");
        printf("demoretriver filename \n");
        return -1;
    }

    cdx_int64 start = CdxMonoTimeUs();

    XRetriever* demoRetriver;
    demoRetriver = XRetrieverCreate();

    if(NULL == demoRetriver)
    {
        printf("create failed\n");
        return -1;
    }

    ret = XRetrieverSetDataSource(demoRetriver, argv[1], NULL);
    if(ret < 0)
    {
        printf("set datasource failed\n");
        return -1;
    }
    printf("XRetrieverSetDataSource end");

    int width;
    XRetrieverGetMetaData(demoRetriver, METADATA_VIDEO_WIDTH, &width);

    int height;
    XRetrieverGetMetaData(demoRetriver, METADATA_VIDEO_HEIGHT, &height);

      int duration;
    XRetrieverGetMetaData(demoRetriver, METADATA_DURATION, &duration);

    printf("get metadata: w(%d), h(%d), duration(%d)\n", width, height, duration);

    XVideoFrame* videoFrame = NULL;
    videoFrame = XRetrieverGetFrameAtTime(demoRetriver, 0, 0);

    (void)videoFrame;
    XRetrieverDestory(demoRetriver);

    cdx_int64 end = CdxMonoTimeUs();
    printf("total need cost time is %lldms\n",(end - start)/1000);

    printf("\n");
    printf("*************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("********************************************************************\n");
    printf("\n");

    return 0;
}
