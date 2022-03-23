#include <string.h>
#include "fortify.h"

char *__stpncpy_chk(char *restrict d, const char *restrict s, size_t n, size_t dn)
{
	if(dn < n) __chk_fail();
	return stpncpy(d, s, n);
}