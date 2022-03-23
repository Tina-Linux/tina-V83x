#include <string.h>
#include "fortify.h"

void *__memmove_chk(void *dest, const void *src, size_t n, size_t dn)
{
	if(dn < n) __chk_fail();
	return memmove(dest, src, n);
}