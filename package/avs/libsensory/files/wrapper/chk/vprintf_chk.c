#include <stdio.h>
#include <stdarg.h>
#include "fortify.h"

int __vprintf_chk(int flag, const char *restrict fmt, va_list ap) {
	return vprintf(fmt, ap);
}