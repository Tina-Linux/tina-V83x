#ifndef WFD_LOG_H
#define WFD_LOG_H

#ifndef LOG_TAG
#define LOG_TAG "WFD_LOG"
#endif

enum WFD_LOG_LEVEL_TYPE {
    WFD_LOG_LEVEL_VERBOSE = 2,
    WFD_LOG_LEVEL_DEBUG = 3,
    WFD_LOG_LEVEL_INFO = 4,
    WFD_LOG_LEVEL_WARNING = 5,
    WFD_LOG_LEVEL_ERROR = 6,
};

extern enum WFD_LOG_LEVEL_TYPE GLOBAL_WFD_LOG_LEVEL;

#ifndef CONFIG_WFD_LOG_LEVEL
#define CONFIG_WFD_LOG_LEVEL    (0xFFFF)
#endif

#ifdef __ANDROID__
#include <cutils/log.h>

#define WFD_LOG_ORDER \
    ((unsigned)WFD_LOG_LEVEL_ERROR   ==  (unsigned)ANDROID_LOG_ERROR) && \
    ((unsigned)WFD_LOG_LEVEL_WARNING ==  (unsigned)ANDROID_LOG_WARN) && \
    ((unsigned)WFD_LOG_LEVEL_INFO    ==  (unsigned)ANDROID_LOG_INFO) && \
    ((unsigned)WFD_LOG_LEVEL_DEBUG   ==  (unsigned)ANDROID_LOG_DEBUG) && \
    ((unsigned)WFD_LOG_LEVEL_VERBOSE ==  (unsigned)ANDROID_LOG_VERBOSE)

typedef char CHECK_WFD_LOG_LEVEL_EQUAL_TO_ANDROID[WFD_LOG_ORDER > 0 ? 1 : -1];

#define WFDLOG(level, fmt, arg...)  \
    do { \
        if (level >= GLOBAL_WFD_LOG_LEVEL || level >= CONFIG_WFD_LOG_LEVEL) \
            LOG_PRI(level, LOG_TAG, "<%s:%u>: " fmt, __FUNCTION__, __LINE__, ##arg); \
    } while (0)

#define WFD_TRACE() \
    WFD_LOGI("<%s:%u> tid(%d)", __FUNCTION__, __LINE__, gettid())

/*check when realease version*/
#define WFD_FORCE_CHECK(e) \
        LOG_ALWAYS_FATAL_IF(                        \
                !(e),                               \
                "<%s:%d>WFD_CHECK(%s) failed.",     \
                __FUNCTION__, __LINE__, #e)

#define WFD_TRESPASS() \
        LOG_ALWAYS_FATAL("Should not be here.")

#define WFD_LOG_FATAL(fmt, arg...)                          \
        LOG_ALWAYS_FATAL("<%s:%d>" fmt,                      \
            __FUNCTION__, __LINE__, ##arg)

#define WFD_LOG_CHECK(e, fmt, arg...)                           \
    LOG_ALWAYS_FATAL_IF(                                        \
            !(e),                                               \
            "<%s:%d>check (%s) failed:" fmt,                    \
            __FUNCTION__, __LINE__, #e, ##arg)

#ifdef WFDP_DEBUG
#define WFD_CHECK(e)                                            \
    LOG_ALWAYS_FATAL_IF(                                        \
            !(e),                                               \
            "<%s:%d>WFD_CHECK(%s) failed.",                     \
            __FUNCTION__, __LINE__, #e)

#else
#define WFD_CHECK(e)
#endif

#else

#include <stdio.h>
#include <string.h>
#include <assert.h>

extern const char *WFD_LOG_LEVEL_NAME[];
#define WFDLOG(level, fmt, arg...)  \
    do { \
        if ((level >= GLOBAL_WFD_LOG_LEVEL || level >= CONFIG_WFD_LOG_LEVEL) && \
                level <= WFD_LOG_LEVEL_ERROR) \
            printf("%s: %s <%s:%u>: " fmt "\n", \
                    WFD_LOG_LEVEL_NAME[level], LOG_TAG, __FUNCTION__, __LINE__, ##arg); \
    } while (0)

#define WFD_TRESPASS()

#define WFD_FORCE_CHECK(e) WFD_CHECK(e)

#define WFD_LOG_CHECK(e, fmt, arg...)                           \
    do {                                                        \
        if (!(e))                                               \
        {                                                       \
            WFD_LOGE("check (%s) failed:"fmt, #e, ##arg);       \
            assert(0);                                          \
        }                                                       \
    } while (0)

#ifdef WFD_DEBUG
#define WFD_CHECK(e)                                            \
    do {                                                        \
        if (!(e))                                               \
        {                                                       \
            WFD_LOGE("check (%s) failed.", #e);                 \
            assert(0);                                          \
        }                                                       \
    } while (0)
#else
#define WFD_CHECK(e)
#endif

#endif

#define WFD_LOGV(fmt, arg...) wfd_logv(fmt, ##arg)
#define WFD_LOGD(fmt, arg...) wfd_logd(fmt, ##arg)
#define WFD_LOGI(fmt, arg...) wfd_logi(fmt, ##arg)
#define WFD_LOGW(fmt, arg...) wfd_logw(fmt, ##arg)
#define WFD_LOGE(fmt, arg...) wfd_loge(fmt, ##arg)

#define WFD_BUF_DUMP(buf, len) \
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
        WFD_LOGD("%s", str);\
    }while (0)

#define WFD_ITF_CHECK(base, itf)    \
    WFD_CHECK(base);                \
    WFD_CHECK(base->ops);           \
    WFD_CHECK(base->ops->itf)

#define WFD_UNUSE(param) (void)param

#define wfd_logd(fmt, arg...) WFDLOG(WFD_LOG_LEVEL_DEBUG, fmt, ##arg)
#define wfd_loge(fmt, arg...) WFDLOG(WFD_LOG_LEVEL_ERROR, "\033[40;31m" fmt "\033[0m", ##arg)
#define wfd_logw(fmt, arg...) WFDLOG(WFD_LOG_LEVEL_WARNING, fmt, ##arg)
#define wfd_logi(fmt, arg...) WFDLOG(WFD_LOG_LEVEL_INFO, fmt, ##arg)
#define wfd_logv(fmt, arg...) WFDLOG(WFD_LOG_LEVEL_VERBOSE, fmt, ##arg)

#ifdef __cplusplus
extern "C"
{
#endif

void wfd_log_set_level(unsigned level);

#ifdef __cplusplus
}
#endif

#endif //WFD_LOG_H