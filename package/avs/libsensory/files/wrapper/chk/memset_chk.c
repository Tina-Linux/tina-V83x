#include <string.h>
#include "fortify.h"

void *__memset_chk(void *dest, int c, size_t n, size_t dn)
{
	if(dn < n) __chk_fail();
	return memset(dest, c, n);
}