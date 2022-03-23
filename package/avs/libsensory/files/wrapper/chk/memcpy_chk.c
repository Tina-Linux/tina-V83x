#include <string.h>
#include "fortify.h"

void *__memcpy_chk(void *restrict dest, const void *restrict src, size_t n, size_t dn)
{
	if(dn < n) __chk_fail();
	return memcpy(dest, src, n);
}