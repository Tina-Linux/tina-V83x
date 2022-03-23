#ifndef __TCONFIGS_LOG_LOGGER_H__
#define __TCONFIGS_LOG_LOGGER_H__

#include <mutex>

namespace tconfigs {
namespace log {

class Logger {
public:
    // These enum are in sequence, don't change their numbers
    enum class Level {
        kDebug      = 0,
        kInfo       = 1,
        kWarning    = 2,
        kError      = 3,
    };

    static Logger& Instance(void) {
        static Logger instance;
        return instance;
    }

    void SetLevel(Level level);

    void Log(Level level, const char* format, ...);

private:
    Logger(void) = default;
    Logger(const Logger& rhs) = default;
    Logger& operator=(const Logger& rhs) = default;
    ~Logger(void) = default;

    inline bool ShouldLog(Level level) { return level >= level_; }

    std::mutex mutex_;
    Level level_ = Level::kDebug;
};

} // namespace log
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_LOG_LOGGER_H__ */
