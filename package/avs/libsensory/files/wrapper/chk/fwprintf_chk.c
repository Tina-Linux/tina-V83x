#include <wchar.h>
#include <stdarg.h>
#include "fortify.h"

int __fwprintf_chk(FILE *restrict f, int flag, const wchar_t *restrict fmt, ...)
{
	//FIXME: fortify
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vfwprintf(f, fmt, ap);
	va_end(ap);
	return ret;
}