#include <string.h>
#include "fortify.h"

char *__strcpy_chk(char *restrict dest, const char *restrict src, size_t destlen)
{
	if(destlen <= strlen(src)) __chk_fail();
	return strcpy(dest, src);
}