#include <stdarg.h>
#include <wchar.h>
#include "fortify.h"

int __vfwprintf_chk(FILE *restrict f, int flag, const wchar_t *restrict fmt, va_list ap)
{
	return vfwprintf(f, fmt, ap);
}