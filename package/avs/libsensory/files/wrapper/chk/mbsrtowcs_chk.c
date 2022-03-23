#include <wchar.h>
#include "fortify.h"

size_t __mbsrtowcs_chk(wchar_t *restrict ws, const char **restrict src, size_t wn, mbstate_t *restrict st, size_t dn)
{
	if(dn < wn) __chk_fail();
	return mbsrtowcs(ws, src, wn, st);
}