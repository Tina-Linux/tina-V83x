#include <string.h>
#include "fortify.h"

char *__stpcpy_chk(char *restrict d, const char *restrict s, size_t dn)
{
	if(dn <= strlen(s)) __chk_fail();
	return stpcpy(d, s);
}