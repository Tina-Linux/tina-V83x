#include <wchar.h>
#include "fortify.h"

size_t __mbsnrtowcs_chk(wchar_t *restrict wcs, const char **restrict src, size_t n, size_t wn, mbstate_t *restrict st, size_t dn)
{
	if(dn < wn) __chk_fail();
	return mbsnrtowcs(wcs, src, n, wn, st);
}