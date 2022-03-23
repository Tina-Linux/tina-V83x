#include "tconfigs/log/logger.h"

#ifdef TCONFIGS_LOG_SYSLOG
#include <syslog.h>
#include <stdarg.h>
#else
#include <stdarg.h>
#endif

namespace tconfigs {
namespace log {

void Logger::SetLevel(Level level)
{
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

#ifdef TCONFIGS_LOG_SYSLOG
void Logger::Log(Level level, const char* format, ...)
{
    if (!ShouldLog(level)) {
        return;
    }

    int priority;
    switch (level) {
    case Level::kDebug:
        priority = LOG_DEBUG;
        break;
    case Level::kInfo:
        priority = LOG_INFO;
        break;
    case Level::kWarning:
        priority = LOG_WARNING;
        break;
    case Level::kError:
        priority = LOG_ERR;
        break;
    default:
        return;
    }

    va_list va;
    va_start(va, format);
    vsyslog(priority, format, va);
    va_end(va);
}

#else // TCONFIGS_LOG_xxx

void Logger::Log(Level level, const char* format, ...)
{
    if (!ShouldLog(level)) {
        return;
    }
    va_list va;
    va_start(va, format);
    vprintf(format, va);
    va_end(va);
}
#endif // TCONFIGS_LOG_xxx

} // namespace log
} // namespace tconfigs
