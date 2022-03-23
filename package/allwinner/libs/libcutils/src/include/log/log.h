#ifndef __LIBCUTILS_LOG_H__
#define __LIBCUTILS_LOG_H__
#include <stdio.h>

/*
 * Android log priority values, in ascending priority order.
 */
typedef enum android_LogPriority {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,    /* only for SetMinPriority() */
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT,     /* only for SetMinPriority(); must be last */
} android_LogPriority;

#define ALOGE printf
#define ALOGD printf
#define ALOGW printf
#define ALOGI printf
#define ALOGV printf

#define SLOGE printf
#define SLOGD printf
#define SLOGW printf
#define SLOGI printf
#define SLOGV printf

#define CONDITION(cond)     (__builtin_expect((cond)!=0, 0))
#ifndef LOG_ALWAYS_FATAL_IF
#define LOG_ALWAYS_FATAL_IF(cond, ...) \
     ( (CONDITION(cond)) \
    ? ((void)ALOGD(__VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef LOG_FATAL_IF
#define LOG_FATAL_IF(cond, ...) LOG_ALWAYS_FATAL_IF(cond, ## __VA_ARGS__)
#endif

#ifndef ALOG_ASSERT
#define ALOG_ASSERT(cond, ...) LOG_FATAL_IF(!(cond), ## __VA_ARGS__)
#endif

#ifndef LOG_ALWAYS_FATAL
#define LOG_ALWAYS_FATAL(...) \
     ( ((void)ALOGE(__VA_ARGS__)) )
#endif

#ifndef ALOGW_IF
#define ALOGW_IF(cond, ...) \
     ( (CONDITION(cond)) \
     ? ((void)ALOGW(__VA_ARGS__)) \
     : (void)0 )
#endif

#ifndef LOG_EVENT_INT
#define LOG_EVENT_INT(_tag, _value) 
#endif

#endif
