#include <stdarg.h>
#include <syslog.h>
#include "fortify.h"

void __vsyslog(int, const char *, va_list);

void __vsyslog_chk(int priority, int flag, const char *message, va_list ap)
{
	return __vsyslog(priority, message, ap);
}