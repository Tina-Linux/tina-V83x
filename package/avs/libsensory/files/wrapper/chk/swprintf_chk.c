#include <stdarg.h>
#include <wchar.h>
#include "fortify.h"

int __swprintf_chk(wchar_t *restrict s, size_t n, int flag, size_t sn, const wchar_t *restrict fmt, ...)
{
	if(sn < n) __chk_fail();
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vswprintf(s, n, fmt, ap);
	va_end(ap);
	return ret;
}