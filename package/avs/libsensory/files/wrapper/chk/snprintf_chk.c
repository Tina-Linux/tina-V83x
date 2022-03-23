#include <stdio.h>
#include <stdarg.h>
#include "fortify.h"

int __snprintf_chk(char *restrict s, size_t n, int flag, size_t sn, const char *restrict fmt, ...)
{
	if(sn < n) __chk_fail();
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vsnprintf(s, n, fmt, ap);
	va_end(ap);
	return ret;
}