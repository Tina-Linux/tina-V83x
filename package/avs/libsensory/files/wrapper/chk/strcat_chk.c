#include <string.h>
#include "fortify.h"

char *__strcat_chk(char *restrict dest, const char *restrict src, size_t destlen)
{
	return strcat(dest, src);
}