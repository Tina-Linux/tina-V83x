#include <stdarg.h>
#include <wchar.h>
#include "fortify.h"

int __vswprintf_chk(wchar_t *restrict s, size_t n, int flag, size_t sn, const wchar_t *restrict fmt, va_list ap)
{
	if(sn < n) __chk_fail();
	return vswprintf(s, n, fmt, ap);
}