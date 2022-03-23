#include <stdarg.h>
#include <wchar.h>
#include "fortify.h"

int __vwprintf_chk(int flag, const wchar_t *restrict fmt, va_list ap)
{
	return vwprintf(fmt, ap);
}