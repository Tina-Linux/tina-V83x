#include <stdlib.h>
#include <limits.h>
#include "fortify.h"

char *__realpath_chk(const char *restrict filename, char *restrict resolved, size_t resolved_len)
{
	if(resolved_len < PATH_MAX) __chk_fail();
	return realpath(filename, resolved);
}