#include <stdlib.h>
#include "fortify.h"

size_t __wcstombs_chk(char *restrict s, const wchar_t *restrict ws, size_t n, size_t dn)
{
	if(dn < n) __chk_fail();
	return wcstombs(s, ws, n);
}