#include <wchar.h>
#include "fortify.h"

size_t __wcsnrtombs_chk(char *restrict dst, const wchar_t **restrict wcs, size_t wn, size_t n, mbstate_t *restrict st, size_t dn)
{
	if(dn < n) __chk_fail();
	return wcsnrtombs(dst, wcs, wn, n, st);
}