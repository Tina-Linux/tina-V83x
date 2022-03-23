#include <stdio.h>
#include <stdarg.h>
#include "fortify.h"

int __vsprintf_chk(char *restrict s, int flag, size_t sn, const char *restrict fmt, va_list ap) {
  return vsprintf(s, fmt, ap);
}