
/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : log.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2015/04/13
*   Comment :
*
*
*/

#ifndef LOG_H
#define LOG_H

#include <cdc_config.h>

#ifndef LOG_TAG
#define LOG_TAG "awplayer"
#endif

#ifdef __ANDROID__
    #include <cutils/log.h>

    #define LOG_LEVEL_ERROR     ANDROID_LOG_ERROR
    #define LOG_LEVEL_WARNING   ANDROID_LOG_WARN
    #define LOG_LEVEL_INFO      ANDROID_LOG_INFO
    #define LOG_LEVEL_VERBOSE   ANDROID_LOG_VERBOSE
    #define LOG_LEVEL_DEBUG     ANDROID_LOG_DEBUG

    #define AWLOG(level, fmt, arg...)  \
        LOG_PRI(level, LOG_TAG, "<%s:%u>: " fmt, __FUNCTION__, __LINE__, ##arg)

#else

#include <stdio.h>
#include <string.h>

#define LOG_LEVEL_ERROR     "error  "
#define LOG_LEVEL_WARNING   "warning"
#define LOG_LEVEL_INFO      "info   "
#define LOG_LEVEL_VERBOSE   "verbose"
#define LOG_LEVEL_DEBUG     "debug  "

#define AWLOG(level, fmt, arg...)  \
    printf("%s: %s <%s:%u>: "fmt"\n", level, LOG_TAG, __FUNCTION__, __LINE__, ##arg)

#endif

#define logd(fmt, arg...) AWLOG(LOG_LEVEL_DEBUG, fmt, ##arg)

#if CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_CLOSE

#define loge(fmt, arg...)
#define logw(fmt, arg...)
#define logi(fmt, arg...)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_ERROR

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m" fmt "\033[0m", ##arg)
#define logw(fmt, arg...)
#define logi(fmt, arg...)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_WARNING

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m" fmt "\033[0m", ##arg)
#define logw(fmt, arg...) AWLOG(LOG_LEVEL_WARNING, fmt, ##arg)
#define logi(fmt, arg...)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_DEFAULT

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m" fmt "\033[0m", ##arg)
#define logw(fmt, arg...) AWLOG(LOG_LEVEL_WARNING, fmt, ##arg)
#define logi(fmt, arg...) AWLOG(LOG_LEVEL_INFO, fmt, ##arg)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_DETAIL

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m" fmt "\033[0m", ##arg)
#define logw(fmt, arg...) AWLOG(LOG_LEVEL_WARNING, fmt, ##arg)
#define logi(fmt, arg...) AWLOG(LOG_LEVEL_INFO, fmt, ##arg)
#define logv(fmt, arg...) AWLOG(LOG_LEVEL_VERBOSE, fmt, ##arg)

#else
    #error "invalid configuration of debug level."
#endif

#define CEDARC_UNUSE(param) (void)param

#endif
