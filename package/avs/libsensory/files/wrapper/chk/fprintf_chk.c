#include <stdio.h>
#include <stdarg.h>
#include "fortify.h"

int __fprintf_chk(FILE *restrict f, int flag, const char *restrict fmt, ...)
{
	//FIXME: fortify
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vfprintf(f, fmt, ap);
	va_end(ap);
	return ret;
}