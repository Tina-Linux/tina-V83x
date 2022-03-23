#ifndef __DEBUG_H_
#define __DEBUG_H_
#include <syslog.h>
#include <stdbool.h>

/** @brief Used to output messages.
   *The messages will include the filename and line number, and will be sent to syslog if so configured in the config file
    */

#define LOG_LEVEL_EMERG   0   /* system is unusable */
#define LOG_LEVEL_ALERT   1   /* action must be taken immediately */
#define LOG_LEVEL_CRIT    2   /* critical conditions */
#define LOG_LEVEL_ERROR	   3   /* error conditions */
#define LOG_LEVEL_WARNING 4   /* warning conditions */
#define LOG_LEVEL_NOTICE  5   /* normal but significant condition */
#define LOG_LEVEL_INFO    6   /* informational */
#define LOG_LEVEL_DEBUG   7   /* debug-level messages */

#define LOG_LEVEL_FATAL   LOG_LEVEL_CRIT


//#define debug(level, format...) _debug("AICLOUD", __FUNCTION__, __LINE__, level, format)
#define debug_local(level, format...) _debug_local("AILocal", __FUNCTION__, __LINE__, level, format)

/** @internal */
//void _debug(const char *module, const char *filename, int line, int level, const char *format, ...);

void _debug_local(const char *module, const char *filename, int line, int level, const char *format, ...);

void debug_levelset(int level);

void debug_enableset(bool enable);


#endif /* _DEBUG_H_ */

