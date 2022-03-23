#include <stdio.h>
#include <stdarg.h>

#include "wfd_log.h"

enum WFD_LOG_LEVEL_TYPE GLOBAL_WFD_LOG_LEVEL = WFD_LOG_LEVEL_DEBUG;

#ifndef __ANDROID__
const char *WFD_LOG_LEVEL_NAME[] = {
    "",
    "",
    [WFD_LOG_LEVEL_VERBOSE] = "VERBOSE",
    [WFD_LOG_LEVEL_DEBUG]   = "DEBUG  ",
    [WFD_LOG_LEVEL_INFO]    = "INFO   ",
    [WFD_LOG_LEVEL_WARNING] = "WARNING",
    [WFD_LOG_LEVEL_ERROR]   = "ERROR  ",
};
#endif

void wfd_log_set_level(unsigned level)
{
    wfd_logw("Set log level to %u", level);
    GLOBAL_WFD_LOG_LEVEL = level;
}