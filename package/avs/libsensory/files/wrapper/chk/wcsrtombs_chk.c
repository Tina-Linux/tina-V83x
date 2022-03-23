#include <wchar.h>
#include "fortify.h"

size_t __wcsrtombs_chk(char *restrict s, const wchar_t **restrict ws, size_t n, mbstate_t *restrict st, size_t dn)
{
	if(dn < n) __chk_fail();
	return wcsrtombs(s, ws, n, st);
}