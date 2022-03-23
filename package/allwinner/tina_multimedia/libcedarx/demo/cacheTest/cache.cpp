/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : cache.cpp
 *
 * Description : cache unit test
 * History :
 *   Author  : Zhao Zhili
 *   Date    : 2016/04/25
 *   Comment : first version
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>

#include <errno.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <malloc.h>
#include <pthread.h>

#include <log.h>
#include <cache.h>

#define err_exit() \
    do { \
        fprintf(stderr, "%s [%d]: errno is %s\n", \
                __func__, __LINE__, strerror(errno)); \
        exit(-1); \
    } while (0)

static StreamCache *cache = NULL;
static const int64_t second = 1000 * 1000;
static const int KB = 1024;
static const int MB = 1024 * 1024;
static int64_t pts = 0;
static const int64_t frameRate = 25;
static const int64_t frameDuration = second / frameRate;
static const int keyFrameInterval = 2 * second;

static int fastTest = 1;

int64_t PlayerGetPosition(Player *p)
{
    (void)p;
    if (cache && cache->pHead)
        return cache->pHead->nPts;
    return 0;
}

static int writeNodeToFile(FILE *fp, CacheNode *node)
{
    if (node->nLength && 0 == (node->nPts % second)) {
        fprintf(fp, "PTS: %f sec -------------------------------------\n",
                node->nPts / 1000000.0);
        char *p = (char *)node->pData;
        int i = 0;
        while (p && *p) {
            putc(*p, fp);
            if (*p == '\n' && ++i > 30)
                break;
            p++;
        }
    }

    return 0;
}

static int getNode(CacheNode *node)
{
    if (pts > 180 * second)
        return -1;

    bzero(node, sizeof(*node));
    node->nLength = sizeof(bigBunny);
    node->pData = (unsigned char *)malloc(node->nLength);
    strcpy((char *)node->pData, bigBunny);

    node->nPts = pts;
    node->eMediaType = CDX_MEDIA_VIDEO;
    node->nFlags = pts % keyFrameInterval ? 0 : KEY_FRAME;
    pts += frameDuration;
    return 0;
}

static void *cacheThread(void *arg)
{
    FILE *fp = (FILE *)arg;
    CacheNode node;
    for (;;) {
        if (StreamCacheOverflow(cache)) {
            fprintf(stderr, "overflow\n");
            usleep(1000 * 10);
            continue;
        }

        if (getNode(&node) != 0)
        {
            fprintf(stdout, "eof\n");
            break;
        }

        if (StreamCacheAddOneFrame(cache, &node))
            err_exit();

        if (!fastTest)
            usleep(frameDuration);
    }

    return NULL;
}

static void normalRun(FILE *fp)
{
    (void)fp;
    cache = StreamCacheCreate();
    if (cache == NULL)
        err_exit();

    int nStartPlaySize = 256 * KB;
    int nMaxBufferSize = 20 * MB;
    StreamCacheSetSize(cache, nStartPlaySize, nMaxBufferSize);

    pthread_t thread;
    pthread_create(&thread, NULL, cacheThread, stdin);
    int i = 0;
    for (;;) {
        if (StreamCacheUnderflow(cache)) {
            if (++i > 10)
                break;

            usleep(1000 * 10);
            continue;
        }

        CacheNode *node = StreamCacheNextFrame(cache);
        if (node == NULL) {
            fprintf(stderr, "%s [%d]: not underflow but head == NULL, why?",
                    __func__, __LINE__);
            continue;
        }

        writeNodeToFile(stdout, node);
        StreamCacheFlushOneFrame(cache);
        if (!fastTest)
            usleep(frameDuration);
    }

    pthread_join(thread, NULL);
    StreamCacheDestroy(cache);
}

struct mediaFormat {
    CdxParserTypeT eContainer;
    enum EVIDEOCODECFORMAT eVideoCodec;
    int nBitrate;
};

static int checkSeek(struct mediaFormat *mediaFormat)
{
    int ret = 0;
    cache = StreamCacheCreate();
    if (cache == NULL)
        err_exit();

    int nStartPlaySize = 256 * KB;
    int nMaxBufferSize = 20 * MB;
    StreamCacheSetSize(cache, nStartPlaySize, nMaxBufferSize);
    StreamCacheSetMediaFormat(cache, mediaFormat->eContainer,
                        mediaFormat->eVideoCodec, mediaFormat->nBitrate);

    pts = 0;
    CacheNode node;
    for (;;) {
        if (StreamCacheOverflow(cache))
            break;

        if (getNode(&node) != 0)
        {
            fprintf(stdout, "eof\n");
            break;
        }

        if (StreamCacheAddOneFrame(cache, &node))
            err_exit();
    }

    fprintf(stdout, "max pts: %f\n", (double)node.nPts / second);
    int i;
    for (i = 0; i < 100; i++) {
        int64_t seekTo = rand() % node.nPts;
        int interval = keyFrameInterval / second;
        fprintf(stdout, "seek to %f, keyframe %"PRId64"\n",
                (double)seekTo / second,
                ((seekTo + second - 1) / second) / interval * interval);

        int64_t ret = StreamCacheSeekTo(cache, seekTo);
        if (ret >= 0) {
            fprintf(stdout, "\t\tseek result %f, header pts %f\n",
                (double)ret / second,
                (double)cache->pHead->nPts / second);

            int64_t precision = keyFrameInterval / 2;
            int64_t error = abs(cache->pHead->nPts - seekTo);
            if (error > precision) {
                fprintf(stdout, "seek result not precise enough\n");
                ret = -1;
            }
        } else {
            fprintf(stdout, "\t\tseek failed\n");
            ret = -1;
        }
    }

    StreamCacheDestroy(cache);

    return ret;
}

static void checkSeekByPts(FILE *fp)
{
    int bitRate = sizeof(bigBunny) * 8 * frameRate;
    struct mediaFormat mediaFormat = {CDX_PARSER_MOV,
                                    VIDEO_CODEC_FORMAT_H264,
                                    bitRate};
    if (checkSeek(&mediaFormat))
        fprintf(fp, "seekbypts failed\n");
    else
        fprintf(fp, "seekbypts success\n");
}

static void checkSeekByOffset(FILE *fp)
{
    int bitRate = sizeof(bigBunny) * 8 * frameRate;
    struct mediaFormat mediaFormat = {CDX_PARSER_TS,
                                    VIDEO_CODEC_FORMAT_H264,
                                    bitRate};
    if (checkSeek(&mediaFormat))
        fprintf(fp, "seekbyoffset failed\n");
    else
        fprintf(fp, "seekbyoffset success\n");
}

static int getMemoryInfo()
{
    struct mallinfo meminfo = mallinfo();
    return meminfo.uordblks + meminfo.hblkhd;
}

static void checkMemoryLeak(void (*func)(FILE *))
{
    int i;
    int n_new;
    int n_old = getMemoryInfo();

    fprintf(stderr, "Before memory leak test, heap and mmaped size is %d M\n",
            n_old / (1 << 20));

    for (i = 0; i < 100; i++) {
        func(stdout);
        n_new = getMemoryInfo();
        fprintf(stdout, "heap and mmaped size is %d M\n", n_new / (1 << 20));

        if (n_new > 5LL * n_old) {
            fprintf(stderr, "After running %d loops, heap and mmaped size is "
                    "%d M, I'm very sure there is memory leak\n",
                    i + 1, n_new / (1 << 20));
            err_exit();
        }
    }

    fprintf(stderr, "After memory leak test, heap and mmaped size is %d M\n",
            n_new / (1 << 20));
}

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "-s") == 0)
        fastTest = 0;

    normalRun(stderr);
    fprintf(stderr, "normalRun test finish\n");

    checkSeekByOffset(stderr);
    checkSeekByPts(stderr);

    fastTest = 1;

    fprintf(stderr, "check if there is memory leak during normal run....\n");
    checkMemoryLeak(normalRun);
    fprintf(stderr, "normal run looks good\n");

    fprintf(stderr, "check if there is memory leak at SeekByOffset....\n");
    checkMemoryLeak(checkSeekByOffset);
    fprintf(stderr, "SeekByOffset looks good\n");

    fprintf(stderr, "check if there is memory leak at checkSeekByPts....\n");
    checkMemoryLeak(checkSeekByPts);
    fprintf(stderr, "SeekByPts looks good\n");

    fprintf(stderr, "Done\n");
    return 0;
}
