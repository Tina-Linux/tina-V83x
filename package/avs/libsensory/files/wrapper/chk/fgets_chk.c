#include <stdio.h>
#include "fortify.h"

char *__fgets_chk(char *restrict s, size_t size, int strsize, FILE *restrict f)
{
	if(strsize < size) __chk_fail();
	return fgets(s, size, f);
}

weak_alias(__fgets_chk, __fgets_unlocked_chk);