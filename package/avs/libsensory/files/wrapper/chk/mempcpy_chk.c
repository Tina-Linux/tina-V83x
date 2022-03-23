#define _GNU_SOURCE
#include <string.h>
#include "fortify.h"

void *__mempcpy_chk(void *dest, const void *src, size_t n, size_t dn)
{
	if(dn < n) __chk_fail();
	return mempcpy(dest, src, n);
}