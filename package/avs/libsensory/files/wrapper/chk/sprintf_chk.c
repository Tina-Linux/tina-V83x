#include <stdio.h>
#include <stdarg.h>
#include "fortify.h"

int __sprintf_chk(char *restrict s, int flag, size_t sn, const char *restrict fmt, ...)
{
	if(sn == 0) __chk_fail();
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vsprintf(s, fmt, ap);
	va_end(ap);
	return ret;
}