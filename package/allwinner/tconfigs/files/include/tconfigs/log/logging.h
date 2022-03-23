#ifndef __TCONFIGS_LOG_LOGGING_H__
#define __TCONFIGS_LOG_LOGGING_H__

#ifndef __FILE_BASENAME__
#include <string.h>
#define __FILE_BASENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#include "tconfigs/log/logger.h"

#ifdef TCONFIGS_LOG_SYSLOG
#define TCONFIGS_LOG_COLOR_NONE
#define TCONFIGS_LOG_COLOR_RED
#define TCONFIGS_LOG_COLOR_GREEN
#define TCONFIGS_LOG_COLOR_YELLOW
#define TCONFIGS_LOG_COLOR_BLUE
#define TCONFIGS_LOG_COLOR_MAGENTA
#define TCONFIGS_LOG_COLOR_CYAN
#else
#define TCONFIGS_LOG_COLOR_NONE     "\e[0m"
#define TCONFIGS_LOG_COLOR_RED      "\e[31m"
#define TCONFIGS_LOG_COLOR_GREEN    "\e[32m"
#define TCONFIGS_LOG_COLOR_YELLOW   "\e[33m"
#define TCONFIGS_LOG_COLOR_BLUE     "\e[34m"
#define TCONFIGS_LOG_COLOR_MAGENTA  "\e[35m"
#define TCONFIGS_LOG_COLOR_CYAN     "\e[36m"
#endif

#define TCLOG_DEBUG(format, ...) \
    do { \
        log::Logger::Instance().Log(log::Logger::Level::kDebug, \
                TCONFIGS_LOG_COLOR_BLUE "[tconfigs][DEBUG] (%s:%d:%s) " format \
                TCONFIGS_LOG_COLOR_NONE "\n", \
                __FILE_BASENAME__, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#define TCLOG_INFO(format, ...) \
    do { \
        log::Logger::Instance().Log(log::Logger::Level::kInfo, \
                TCONFIGS_LOG_COLOR_GREEN "[tconfigs][INFO] " format \
                TCONFIGS_LOG_COLOR_NONE "\n", ##__VA_ARGS__); \
    } while (0)

#define TCLOG_WARNING(format, ...) \
    do { \
        log::Logger::Instance().Log(log::Logger::Level::kWarning, \
                TCONFIGS_LOG_COLOR_YELLOW "[tconfigs][WARNING] " format \
                TCONFIGS_LOG_COLOR_NONE "\n", ##__VA_ARGS__); \
    } while (0)

#define TCLOG_ERROR(format, ...) \
    do { \
        log::Logger::Instance().Log(log::Logger::Level::kError, \
                TCONFIGS_LOG_COLOR_RED "[tconfigs][ERROR] (%s:%d:%s) " format \
                TCONFIGS_LOG_COLOR_NONE "\n", \
                __FILE_BASENAME__, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#endif /* ifndef __TCONFIGS_LOG_LOGGING_H__ */
