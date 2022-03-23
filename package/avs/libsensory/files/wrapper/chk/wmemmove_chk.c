#include <wchar.h>
#include "fortify.h"

wchar_t *__wmemmove_chk(wchar_t *d, const wchar_t *s, size_t n, size_t dn)
{
	if(dn < n) __chk_fail();
	return wmemmove(d, s, n);
}