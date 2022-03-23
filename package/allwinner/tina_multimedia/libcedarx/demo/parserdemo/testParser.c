/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : testParser.c
 * Description : Parser demo
 * History :
 *
 */

#include <stdio.h>
#include "CdxParser.h"
#include <string.h>
#include <stdlib.h>
#include "cdx_log.h"

int DemoParser(const char* url,const char* filePath)
{
    CdxParserT *parser = NULL;
    CdxStreamT *stream = NULL;
    CdxDataSourceT source;
    CdxMediaInfoT mediaInfo;
    cdx_bool forceExit = 0;
    char tmp[1024];
    char tmpPath[1024];
    int i=0;
    int j;
    int ret;
    int bufSize = 5*1024*1024;  // care: maybe the packet length is large than bufSize
    char *buf = (char*)malloc(bufSize);
    unsigned char sha[20] = {0};

    memset(&source,0,sizeof(source));
    memset(&mediaInfo,0,sizeof(mediaInfo));
    memset(tmp,0,sizeof(tmp));
    memset(tmpPath,0,sizeof(tmpPath));
    //
    if(strlen(url) > 1023)
    {
        printf("url is too long!\n");
        free(buf);
        return -1;
    }
    if(strlen(filePath) > (1023 - sizeof("video.h264")))
    {
        printf("file path is too long!\n");
        free(buf);
        return -1;
    }
    strcpy(tmp,url);
    strcpy(tmpPath,filePath);

    source.uri = tmp;

    printf("url :%s",source.uri);
    ret = CdxParserPrepare(&source, 0, NULL, &forceExit,
                        &parser, &stream, NULL, NULL);
    if(ret < 0 || !parser)
    {
        printf("we can not get the right parser, maybe we not support this file\n");
        free(buf);
        return -1;
    }

    if(0 != CdxParserGetMediaInfo(parser, &mediaInfo))
    {
        printf("get media info err\n");
        free(buf);
        return -1;
    }

    CdxPacketT packet;
    memset(&packet, 0, sizeof(packet));
    int prefetch_result, read_result;

    while(0 == CdxParserPrefetch(parser, &packet))
    {
        if(packet.length > 5*1024*1024)
        {
            printf("error: lenth(%d) > 1024*1024", packet.length);
            free(buf);
            return -1;
        }

        packet.buflen = packet.length;
        packet.buf = buf;

        if(0 == CdxParserRead(parser,&packet))
        {
            if(packet.type == CDX_MEDIA_VIDEO ||
                packet.type == CDX_MEDIA_AUDIO ||
                packet.type == CDX_MEDIA_SUBTITLE)
            {

            }
        }
        else
        {
            printf("parser read err!\n");
            break;
        }
    }

    CdxParserClose(parser);
    free(buf);

    return 0;
}

void printUseage()
{
    printf("********** useage *********\n");
    printf("param 1: the file path,like this :\"file:///mnt/sdcard/A098.mp4\" \n");
    printf("param 2: the dump file path,parser will store file in this path,"
            "like this:\"/data/camera\",if there is no param 2,the default path will be /\n");
    printf("********* end ************\n");
}

int main(int argc, char** argv)
{
    printf("parser test start!\n");
    if(argc < 2 || argc > 3)
    {
        printUseage();
        return 0;
    }

    if(argc < 3)
    {
        printf("exit main function!\n");
        return DemoParser(argv[1],"/mnt/sdcard/");
    }
    else
    {
        return DemoParser(argv[1],argv[2]);
    }
}
