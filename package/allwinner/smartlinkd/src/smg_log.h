#ifndef __SMG_LOG_H
#define __SMG_LOG_H

#if __cplusplus
extern "C" {
#endif

extern int smg_debug_level;
extern int smg_debug_show_keys;
extern int smg_debug_timestap;

#ifdef __GNUC__
#define PRINTF_FORMAT(a,b) __attribute__ ((format (printf, (a), (b))))
#define STRUCT_PACKED __attribute__ ((packed))
#else
#define PRINTF_FORMAT(a,b)
#define STRUCT_PACKED
#endif

enum {
	SMG_ERROR=0, SMG_WARNING, SMG_INFO,SMG_DEBUG, SMG_MSGDUMP, SMG_EXCESSIVE
};

#ifdef CONFIG_NO_STDOUT_DEBUG

#define smg_printf(args...) do { } while (0)
#define smg_debug_open_file(p) do { } while (0)
#define smg_debug_close_file() do { } while (0)

#else
int smg_debug_open_file(const char *path);
void smg_debug_close_file(void);
void smg_debug_open_syslog(void);
void smg_debug_close_syslog(void);
void smg_set_debug_level(int level);
int smg_get_debug_level();

#define  CONFIG_DEBUG_FUNCTION_LINE 1

#ifdef CONFIG_DEBUG_FUNCTION_LINE
#define smg_printf(level,fmt,arg...) \
	smg_print(level,"[%s:%u]:" fmt "",__FUNCTION__,__LINE__,##arg)
#else
#define smg_printf(level,fmt,arg...) \
	smg_print(level,fmt,##arg)
#endif /*CONFIG_DEBUG_FUNCTION_LINE*/

void smg_print(int level, const char *fmt, ...)
PRINTF_FORMAT(2, 3);

#endif/* CONFIG_NO_STDOUT_DEBUG */

#if __cplusplus
};  // extern "C"
#endif

#endif
