/*
 * Copyright (c) 2008-2017 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : demo_utils.h
 * Description : demoAdecoder
 * History :
 *
 */

#ifndef DEMO_UTILS_H
#define DEMO_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "cdx_config.h"
#include "CdxParser.h"
#include "adecoder.h"
#include <iniparserapi.h>

#include "tinyplaydemo.h"

#define DEMO_VERSION "VERSION 1.0"
#define DEMO_WRITER  "KHAN"
#define DEMO_OWNER   "AllwinnerTech.Lto"

#define DEMO_SAFE_OPEN_FILE(file, dir, mode, name) if(file == NULL) { \
    file = fopen(dir, mode); \
    if(file == NULL) { \
        demo_loge("open %s fail!!", name); \
    } \
    else \
    { \
        demo_logd("open %s success", name); \
    } \
}

#define DEMO_SAFE_WRITE_FILE(buf, blksz, blknum, file) if(file != NULL) {\
    fwrite(buf, blksz, blknum, file); \
}

#define DEMO_SAFE_READ_FILE(buf, blksz, blknum, file) if(file != NULL) {\
    fread(buf, blksz, blknum, file); \
}

#define DEMO_SAFE_CLOSE_FILE(file, name) if(file != NULL) {\
    fclose(file); \
    demo_logd("Demo capture file %s(%p) has closed ...", name, file); \
    file = NULL; \
}

#define DEMO_SAFE_FREE(ptr, name, print) \
    if(ptr){\
        free(ptr);\
        if(print){ \
            demo_logd("Demo ptr %s(%p) safely freed ...", name, ptr);\
        } \
        ptr = NULL;\
    }

#define DEMO_VERBOS_SAFE_MALLOC(ptr, len, action_when_fail) \
    ptr = malloc(len); \
    if(ptr == NULL){ \
        action_when_fail \
    } \
    memset(ptr, 0x00, len);

#define demo_print(fmt, arg...) fprintf(stderr, fmt"\n", ##arg)

#define demo_logi(fmt, arg...) if(DEMO_DEBUG >= 0) \
    fprintf(stderr, "%s, line(%d), info:"fmt"\n", LOG_TAG, __LINE__, ##arg)

#define demo_loge(fmt, arg...) if(DEMO_DEBUG >= 1) \
    fprintf(stderr, "%s, line(%d), error:"fmt"\n", LOG_TAG, __LINE__, ##arg)

#define demo_logd(fmt, arg...) if(DEMO_DEBUG >= 2) \
    fprintf(stderr, "%s, line(%d), debug:"fmt"\n", LOG_TAG, __LINE__, ##arg)

#define demo_logw(fmt, arg...) if(DEMO_DEBUG >= 3) \
    fprintf(stderr, "%s, line(%d), warning:"fmt"\n", LOG_TAG, __LINE__, ##arg)

#define demo_logv(fmt, arg...) if(DEMO_DEBUG >= 4) \
    fprintf(stderr, "%s, line(%d), verbose:"fmt"\n", LOG_TAG, __LINE__, ##arg)

typedef struct MultiThreadCtx
{
    pthread_rwlock_t rwrock;
    int nEndofStream;
    int loop;
    int state;
}MultiThreadCtx;

typedef struct AUDIOSTREAMDATAINFO
{
    char*   pData;
    int     nLength;
    int64_t nPts;
    int64_t nPcr;
    int     bIsFirstPart;
    int     bIsLastPart;
}AudioStreamDataInfo;


typedef struct DecDemo
{
    AudioDecoder *pAudioDec;
    CdxParserT *parser;
    AudioDsp*   dsp;
    CdxDataSourceT source;
    CdxMediaInfoT   mediaInfo;
    BsInFor         bsInfo;
    AudioStreamInfo AudioInfo;
    CdxPlaybkCfg    cfg;
    MultiThreadCtx  thread;
    long long totalTime;
    long long DurationTime;
    char *pInputFile;
    char *pOutputDir;
    char file_base_name[256];
    pthread_mutex_t parserMutex;
}DecDemo;

#endif
