/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxLog.h
 * Description : Log
 * History :
 *
 */

#ifndef CDX_LOG_H
#define CDX_LOG_H

#include "cdx_config.h"
#include "log.h"

#include <CdxTypes.h>
#include <CdxDebug.h>

#define CDX_LOGV(fmt, arg...) logv(fmt, ##arg)
#define CDX_LOGD(fmt, arg...) logd(fmt, ##arg)
#define CDX_LOGI(fmt, arg...) logi(fmt, ##arg)
#define CDX_LOGW(fmt, arg...) logw(fmt, ##arg)
#define CDX_LOGE(fmt, arg...) loge(fmt, ##arg)

#ifdef __ANDROID__

#define CDX_TRACE() \
    CDX_LOGI("<%s:%u> tid(%d)", __FUNCTION__, __LINE__, gettid())

/*check when realease version*/
#define CDX_FORCE_CHECK(e) \
        LOG_ALWAYS_FATAL_IF(                        \
                !(e),                               \
                "<%s:%d>CDX_CHECK(%s) failed.",     \
                __FUNCTION__, __LINE__, #e)      \

#define CDX_TRESPASS() \
        LOG_ALWAYS_FATAL("Should not be here.")

#define CDX_LOG_FATAL(fmt, arg...)                          \
        LOG_ALWAYS_FATAL("<%s:%d>" fmt,                      \
            __FUNCTION__, __LINE__, ##arg)    \

#define CDX_LOG_CHECK(e, fmt, arg...)                           \
    LOG_ALWAYS_FATAL_IF(                                        \
            !(e),                                               \
            "<%s:%d>check (%s) failed:" fmt,                     \
            __FUNCTION__, __LINE__, #e, ##arg)    \

#ifdef AWP_DEBUG
#define CDX_CHECK(e)                                            \
    LOG_ALWAYS_FATAL_IF(                                        \
            !(e),                                               \
            "<%s:%d>CDX_CHECK(%s) failed.",                     \
            __FUNCTION__, __LINE__, #e)           \

#else
#define CDX_CHECK(e)
#endif

#elif CONFIG_OS == OPTION_OS_LINUX

#include <assert.h>

#define CDX_TRESPASS()

#define CDX_FORCE_CHECK(e) CDX_CHECK(e)

#define CDX_LOG_CHECK(e, fmt, arg...)                           \
    do {                                                        \
        if (!(e))                                                 \
        {                                                       \
            CDX_LOGE("check (%s) failed:"fmt, #e, ##arg);       \
            assert(0);                                          \
        }                                                       \
    } while (0)

#ifdef AWP_DEBUG
#define CDX_CHECK(e)                                            \
    do {                                                        \
        if (!(e))                                                 \
        {                                                       \
            CDX_LOGE("check (%s) failed.", #e);                 \
            assert(0);                                          \
        }                                                       \
    } while (0)
#else
#define CDX_CHECK(e)
#endif

#else
    #error "invalid configuration of os."
#endif

#define CDX_BUF_DUMP(buf, len) \
    do { \
        char *_buf = (char *)buf;\
        char str[1024] = {0};\
        unsigned int index = 0, _len;\
        _len = (unsigned int)len;\
        snprintf(str, 1024, ":%d:[", _len);\
        for (index = 0; index < _len; index++)\
        {\
            snprintf(str + strlen(str), 1024 - strlen(str), "%02hhx ", _buf[index]);\
        }\
        str[strlen(str) - 1] = ']';\
        CDX_LOGD("%s", str);\
    }while (0)

#define CDX_ITF_CHECK(base, itf)    \
    CDX_CHECK(base);                \
    CDX_CHECK(base->ops);           \
    CDX_CHECK(base->ops->itf)

#define CDX_UNUSE(param) (void)param
#endif
