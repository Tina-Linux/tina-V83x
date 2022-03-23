#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>

int __vasprintf_chk(char **s, int flag, const char *fmt, va_list ap)
{
	return vasprintf(s, fmt, ap);
}