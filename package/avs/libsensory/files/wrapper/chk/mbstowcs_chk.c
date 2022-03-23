#include <wchar.h>
#include "fortify.h"

size_t __mbstowcs_chk(wchar_t *restrict ws, const char *restrict s, size_t wn, size_t dn)
{
	if(dn < wn) __chk_fail();
	return mbstowcs(ws, s, wn);
}