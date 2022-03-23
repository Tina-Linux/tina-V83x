#include <wchar.h>
#include "fortify.h"

wchar_t *__fgetws_chk(wchar_t *restrict s, size_t size, int strsize, FILE *restrict f)
{
	if(strsize < size) __chk_fail();
	return fgetws(s, size, f);
}

weak_alias(__fgetws_chk, __fgetws_unlocked_chk);