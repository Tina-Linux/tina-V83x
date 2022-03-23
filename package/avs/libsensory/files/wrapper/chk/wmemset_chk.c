#include <wchar.h>
#include "fortify.h"

wchar_t *__wmemset_chk(wchar_t *d, wchar_t c, size_t n, size_t dn)
{
	if(dn < n) __chk_fail();
	return wmemset(d, c, n);
}