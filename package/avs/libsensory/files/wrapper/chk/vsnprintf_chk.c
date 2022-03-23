#include <stdio.h>
#include <stdarg.h>
#include "fortify.h"

int __vsnprintf_chk(char *restrict s, size_t n, int flag, size_t sn, const char *restrict fmt, va_list ap)
{
	if(sn < n) __chk_fail();
	return vsnprintf(s, n, fmt, ap);
}