#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>

int __asprintf_chk(char **s, int flag, const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vasprintf(s, fmt, ap);
	va_end(ap);
	return ret;
}