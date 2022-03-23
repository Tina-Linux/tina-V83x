#include <wchar.h>
#include "fortify.h"

wchar_t *__wcsncpy_chk(wchar_t *restrict d, const wchar_t *restrict s, size_t n, size_t dn)
{
	if(dn < n) __chk_fail();
	return wcsncpy(d, s, n);
}