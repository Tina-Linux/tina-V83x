#include <stdio.h>
#include <stdarg.h>
#include "fortify.h"

int __vfprintf_chk(FILE *restrict f, int flag, const char *restrict fmt, va_list ap)
{
	return vfprintf(f, fmt, ap);
}