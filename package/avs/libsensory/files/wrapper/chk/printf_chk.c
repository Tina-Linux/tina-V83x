#include <stdio.h>
#include <stdarg.h>
#include "fortify.h"

int __printf_chk(int flag, const char *restrict fmt, ...)
{
	//FIXME: fortify
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return ret;
}