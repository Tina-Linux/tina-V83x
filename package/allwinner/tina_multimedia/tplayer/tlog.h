
#ifndef LOG_H
#define LOG_H

#ifndef LOG_TAG
#define LOG_TAG "tplayer"
#endif

#include <stdio.h>
#include <string.h>

/**
 *  Debug Log Marco for tplayer.
 *  Log enable if define 'TPLAYER_DBG' in Makefile.
 */
#define TPLAYER_DBG 1

#define CHECK_ERROR_RETURN(e)                                            \
		do {														\
			if (!(e))												\
			{														\
				printf("check (%s) failed.", #e);		   \
				return -1;											\
			}														\
		} while (0)

#ifdef TPLAYER_DBG
#include <assert.h>

enum TP_LOG_LEVEL_TYPE {
    TLOG_LEVEL_VERBOSE = 2,
    TLOG_LEVEL_DEBUG = 3,
    TLOG_LEVEL_INFO = 4,
    TLOG_LEVEL_WARNING = 5,
    TLOG_LEVEL_ERROR = 6,
};

#define TLOG_LEVEL_VERBOSE "VERBOSE "
#define TLOG_LEVEL_DEBUG "DEBUG "
#define TLOG_LEVEL_INFO "INFO "
#define TLOG_LEVEL_WARNING "WARNING "
#define TLOG_LEVEL_ERROR "ERROR "

//enum TP_LOG_LEVEL_TYPE T_GLOBAL_LOG_LEVEL = TLOG_LEVEL_DEBUG;

#define TPLOG(level, fmt, arg...)  \
	do { \
		printf("%s: %s <%s:%d>: " fmt "\n", \
				level, LOG_TAG, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define TLOGE(fmt, arg...) TPLOG(TLOG_LEVEL_ERROR, "\033[40;31m" fmt "\033[0m", ##arg)
#define TLOGW(fmt, arg...) TPLOG(TLOG_LEVEL_WARNING, fmt, ##arg)
#define TLOGI(fmt, arg...) TPLOG(TLOG_LEVEL_INFO, fmt, ##arg)
#define TLOGD(fmt, arg...) TPLOG(TLOG_LEVEL_DEBUG, fmt, ##arg)
#define TLOGV(fmt, arg...) TPLOG(TLOG_LEVEL_VERBOSE, fmt, ##arg)
#define TP_LOGE(fmt, arg...) TLOGE(fmt, ##arg)

#define TP_CHECK(e)                                            \
    do {                                                        \
        if (!(e))                                               \
        {                                                       \
            TP_LOGE("check (%s) failed.", #e);         \
            assert(0);                                          \
        }                                                       \
    } while (0)

#define TP_LOG_CHECK(e, fmt, arg...)                           \
    do {                                                        \
        if (!(e))                                               \
        {                                                       \
            TP_LOGE("check (%s) failed:"fmt, #e, ##arg); \
            assert(0);                                          \
        }                                                       \
    } while (0)
#else
#define TP_CHECK(e)
#define TP_LOG_CHECK(e, fmt, arg...)
#define TLOGE(fmt, arg...)
#define TLOGW(fmt, arg...)
#define TLOGI(fmt, arg...)
#define TLOGD(fmt, arg...)
#define TLOGV(fmt, arg...)
#endif

#endif
